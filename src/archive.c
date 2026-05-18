/* archive.c — -b modu: birden cok ASCII metin dosyasini tek bir SAU
 * arsivinde birlestirir. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "archive.h"
#include "sau_format.h"
#include "util.h"

#define COPY_BUFFER_SIZE 8192

/* SAU formati kayit alanlari icin ',' ve '|' ayriclari kullaniyor.
 * Bu karakterleri iceren dosya adlari formati bozar -> reddet. */
static int filename_has_invalid_chars(const char *name) {
    for (const char *p = name; *p; p++) {
        if (*p == ',' || *p == '|') {
            return 1;
        }
    }
    return 0;
}

/* Bir dosyanin tum icerigini `out`'a kopyalar. */
static int copy_file_contents(FILE *out, const char *input_path,
                              size_t expected_size) {
    FILE *in = fopen(input_path, "rb");
    if (!in) {
        fprintf(stderr, "Hata: '%s' okunamiyor: %s\n",
                input_path, strerror(errno));
        return -1;
    }

    unsigned char buf[COPY_BUFFER_SIZE];
    size_t total = 0;
    size_t n;

    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fprintf(stderr, "Hata: '%s' icerigi arsive yazilamadi.\n",
                    input_path);
            fclose(in);
            return -1;
        }
        total += n;
    }

    int err = ferror(in);
    fclose(in);

    if (err) {
        fprintf(stderr, "Hata: '%s' okurken hata olustu.\n", input_path);
        return -1;
    }
    if (total != expected_size) {
        fprintf(stderr,
                "Uyari: '%s' boyutu degisti (beklenen %zu, okunan %zu).\n",
                input_path, expected_size, total);
    }
    return 0;
}

int archive_create(const char *output_path,
                   const char **input_files,
                   int num_files) {
    if (num_files <= 0 || num_files > MAX_FILES) {
        fprintf(stderr, "Hata: Gecersiz giris dosyasi sayisi: %d\n", num_files);
        return 1;
    }

    sau_entry_t entries[MAX_FILES];
    size_t total_size = 0;

    /* 1. Asama: tum giris dosyalarini dogrula ve meta veri topla. */
    for (int i = 0; i < num_files; i++) {
        const char *path = input_files[i];

        /* Arsivde sadece taban adi saklayalim (spec ornegi de boyle). */
        const char *basename = strrchr(path, '/');
        basename = basename ? basename + 1 : path;

        if (strlen(basename) >= MAX_FILENAME_LEN) {
            fprintf(stderr, "Hata: '%s' dosya adi cok uzun.\n", basename);
            return 1;
        }
        if (filename_has_invalid_chars(basename)) {
            fprintf(stderr,
                    "Hata: '%s' dosya adinda ',' veya '|' kullanilamaz.\n",
                    basename);
            return 1;
        }

        size_t size = get_file_size(path);
        if (size == (size_t)-1) {
            fprintf(stderr, "Hata: '%s' acilamadi: %s\n",
                    path, strerror(errno));
            return 1;
        }

        /* ASCII format kontrolu — spec'in en kritik gerekliligi. */
        int ascii_result = is_ascii_text_file(path);
        if (ascii_result == -1) {
            fprintf(stderr, "Hata: '%s' okunamadi.\n", path);
            return 1;
        }
        if (ascii_result == 0) {
            /* Spec'in tam metni: "<dosya> giris dosyasinin formati uyumsuzdur!"
             * Program "sorunsuz bir sekilde" cikmali, yani exit 0. */
            printf("%s giriş dosyasının formatı uyumsuzdur!\n", path);
            return 0;
        }

        mode_t perms = get_file_permissions(path);
        if (perms == (mode_t)-1) {
            fprintf(stderr, "Hata: '%s' izinleri alinamadi.\n", path);
            return 1;
        }

        strncpy(entries[i].filename, basename, MAX_FILENAME_LEN - 1);
        entries[i].filename[MAX_FILENAME_LEN - 1] = '\0';
        entries[i].permissions = perms;
        entries[i].size = size;

        total_size += size;
        if (total_size > MAX_TOTAL_SIZE) {
            fprintf(stderr,
                    "Hata: Toplam giris boyutu %ld MB sinirini asti.\n",
                    MAX_TOTAL_SIZE / (1024 * 1024));
            return 1;
        }
    }

    /* 2. Asama: arsiv dosyasini ac ve organizasyon bolumunu yaz. */
    FILE *out = fopen(output_path, "wb");
    if (!out) {
        fprintf(stderr, "Hata: Cikis dosyasi '%s' olusturulamadi: %s\n",
                output_path, strerror(errno));
        return 1;
    }

    if (sau_write_organization(out, entries, num_files) != 0) {
        fprintf(stderr, "Hata: Organizasyon bolumu yazilamadi.\n");
        fclose(out);
        return 1;
    }

    /* 3. Asama: dosya iceriklerini arka arkaya ekle. */
    for (int i = 0; i < num_files; i++) {
        if (copy_file_contents(out, input_files[i], entries[i].size) != 0) {
            fclose(out);
            return 1;
        }
    }

    if (fclose(out) != 0) {
        fprintf(stderr, "Hata: Arsiv dosyasi kapatilirken hata olustu.\n");
        return 1;
    }

    printf("Dosyalar birleştirildi.\n");
    return 0;
}
