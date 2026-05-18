/* extract.c — -a modu: SAU arsivinden dosyalari geri cikarir,
 * orijinal izinlerini geri yukler, gerekirse hedef dizini olusturur. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "extract.h"
#include "sau_format.h"

#define COPY_BUFFER_SIZE 8192
#define MAX_PATH_LEN     (MAX_FILENAME_LEN * 2 + 2)

static const char *BAD_ARCHIVE_MSG = "Arşiv dosyası uygunsuz veya bozuk!\n";

/* mkdir -p benzeri: nested dizinleri yaratir.
 * '/' ayraciyla path'i parcalar, her seviyede mkdir cagirir.
 * Zaten varsa hata sayilmaz. */
static int mkdir_recursive(const char *path) {
    char tmp[MAX_PATH_LEN];
    size_t len = strlen(path);
    if (len == 0 || len >= sizeof(tmp)) {
        return -1;
    }
    strcpy(tmp, path);

    /* Sondaki '/'i kaldir. */
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    /* Her ara dizini olustur. */
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

/* Guvenlik: arsivde "../" gibi path traversal saldirilarini engelle. */
static int filename_is_safe(const char *name) {
    if (name[0] == '\0' || name[0] == '/') return 0;
    if (strstr(name, "..") != NULL) return 0;
    if (strchr(name, '/') != NULL) return 0;
    return 1;
}

/* `in`'den tam olarak `n` bayt okuyup `out`'a yazar.
 * Eksik okuma -> arsiv bozuk. */
static int copy_n_bytes(FILE *in, FILE *out, size_t n) {
    unsigned char buf[COPY_BUFFER_SIZE];
    size_t remaining = n;

    while (remaining > 0) {
        size_t want = remaining < sizeof(buf) ? remaining : sizeof(buf);
        size_t got = fread(buf, 1, want, in);
        if (got == 0) {
            return -1;  /* eksik veri */
        }
        if (fwrite(buf, 1, got, out) != got) {
            return -1;
        }
        remaining -= got;
    }
    return 0;
}

int archive_extract(const char *archive_path, const char *output_dir) {
    FILE *in = fopen(archive_path, "rb");
    if (!in) {
        fprintf(stderr, "Hata: Arsiv dosyasi '%s' acilamadi: %s\n",
                archive_path, strerror(errno));
        return 1;
    }

    /* 1. Organizasyon bolumunu oku. */
    sau_entry_t entries[MAX_FILES];
    int count = 0;
    if (sau_read_organization(in, entries, &count) != 0 || count == 0) {
        fprintf(stderr, "%s", BAD_ARCHIVE_MSG);
        fclose(in);
        return 1;
    }

    /* 2. Tum dosya adlarini guvenlik kontrolunden gecir. */
    for (int i = 0; i < count; i++) {
        if (!filename_is_safe(entries[i].filename)) {
            fprintf(stderr, "%s", BAD_ARCHIVE_MSG);
            fclose(in);
            return 1;
        }
    }

    /* 3. Cikis dizini varsa olustur. */
    if (output_dir != NULL && strcmp(output_dir, ".") != 0) {
        if (mkdir_recursive(output_dir) != 0) {
            fprintf(stderr,
                    "Hata: Cikis dizini '%s' olusturulamadi: %s\n",
                    output_dir, strerror(errno));
            fclose(in);
            return 1;
        }
    }

    /* 4. Her dosyayi sirayla cikar. */
    for (int i = 0; i < count; i++) {
        char fullpath[MAX_PATH_LEN];
        int n;
        if (output_dir != NULL) {
            n = snprintf(fullpath, sizeof(fullpath), "%s/%s",
                         output_dir, entries[i].filename);
        } else {
            n = snprintf(fullpath, sizeof(fullpath), "%s",
                         entries[i].filename);
        }
        if (n < 0 || (size_t)n >= sizeof(fullpath)) {
            fprintf(stderr, "Hata: Cikis yolu cok uzun: %s\n",
                    entries[i].filename);
            fclose(in);
            return 1;
        }

        FILE *out = fopen(fullpath, "wb");
        if (!out) {
            fprintf(stderr, "Hata: '%s' yazilamiyor: %s\n",
                    fullpath, strerror(errno));
            fclose(in);
            return 1;
        }

        if (copy_n_bytes(in, out, entries[i].size) != 0) {
            fprintf(stderr, "%s", BAD_ARCHIVE_MSG);
            fclose(out);
            fclose(in);
            return 1;
        }

        if (fclose(out) != 0) {
            fprintf(stderr, "Hata: '%s' kapatilirken hata.\n", fullpath);
            fclose(in);
            return 1;
        }

        /* Orijinal izinleri geri yukle. */
        if (chmod(fullpath, entries[i].permissions) != 0) {
            fprintf(stderr,
                    "Uyari: '%s' izinleri ayarlanamadi: %s\n",
                    fullpath, strerror(errno));
        }
    }

    /* Arsivde fazladan bayt var mi? Olmamali — bozuk olabilir.
     * Sadece uyari basariyoruz, hata degil (kullaniciya zarar vermez). */
    int extra = fgetc(in);
    if (extra != EOF) {
        fprintf(stderr,
                "Uyari: Arsivin sonunda beklenmeyen %d bayt var.\n", 1);
    }

    fclose(in);
    printf("Dosyalar başarıyla çıkarıldı.\n");
    return 0;
}
