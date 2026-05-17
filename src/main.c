/* main.c — Komut satırı ayrıştırma ve mod dağıtımı. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archive.h"
#include "extract.h"
#include "sau_format.h"

#define DEFAULT_ARCHIVE_NAME "a.sau"

static void print_usage(const char *prog) {
    fprintf(stderr, "Kullanim:\n");
    fprintf(stderr, "  %s -b <dosya1> [dosya2 ...] [-o <arsiv.sau>]\n", prog);
    fprintf(stderr, "  %s -a <arsiv.sau> [hedef_dizin]\n", prog);
}

/* -b modu: giris dosyalarini topla, opsiyonel -o cikis adini ayikla,
 * sonra archive_create() cagir. */
static int handle_archive_mode(int argc, char *argv[]) {
    const char *input_files[MAX_FILES];
    int         num_files   = 0;
    const char *output_name = DEFAULT_ARCHIVE_NAME;

    int i = 2;
    while (i < argc && strcmp(argv[i], "-o") != 0) {
        if (num_files >= MAX_FILES) {
            fprintf(stderr,
                    "Hata: En fazla %d giris dosyasi arsivlenebilir.\n",
                    MAX_FILES);
            return 1;
        }
        input_files[num_files++] = argv[i];
        i++;
    }

    if (num_files == 0) {
        fprintf(stderr, "Hata: En az bir giris dosyasi belirtilmelidir.\n");
        print_usage(argv[0]);
        return 1;
    }

    /* -o varsa, ardindan tam olarak bir argüman (cikis dosyasi adi) gelmeli. */
    if (i < argc && strcmp(argv[i], "-o") == 0) {
        if (i + 1 >= argc) {
            fprintf(stderr,
                    "Hata: -o parametresinden sonra cikis dosyasi adi belirtilmelidir.\n");
            return 1;
        }
        if (i + 2 < argc) {
            fprintf(stderr,
                    "Hata: -o parametresinden sonra fazladan argüman var: '%s'\n",
                    argv[i + 2]);
            return 1;
        }
        output_name = argv[i + 1];
    }

    return archive_create(output_name, input_files, num_files);
}

/* -a modu: arsiv.sau yolu (zorunlu), hedef dizin (opsiyonel). */
static int handle_extract_mode(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr,
                "Hata: -a parametresinden sonra arsiv dosyasi belirtilmelidir.\n");
        print_usage(argv[0]);
        return 1;
    }
    if (argc > 4) {
        fprintf(stderr, "Hata: -a en fazla 2 parametre alir.\n");
        return 1;
    }

    const char *archive_path = argv[2];
    const char *output_dir   = (argc >= 4) ? argv[3] : NULL;

    return archive_extract(archive_path, output_dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0) {
        return handle_archive_mode(argc, argv);
    }
    if (strcmp(argv[1], "-a") == 0) {
        return handle_extract_mode(argc, argv);
    }

    fprintf(stderr, "Hata: Bilinmeyen mod '%s'\n", argv[1]);
    print_usage(argv[0]);
    return 1;
}
