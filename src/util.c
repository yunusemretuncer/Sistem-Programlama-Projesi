/* util.c — Dosya yardimcilari ve ASCII dogrulama. */

#include <stdio.h>
#include <sys/stat.h>

#include "util.h"

#define ASCII_CHECK_BUF_SIZE 8192

int is_ascii_text_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return -1;
    }

    unsigned char buf[ASCII_CHECK_BUF_SIZE];
    size_t n;

    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        for (size_t i = 0; i < n; i++) {
            /* Spec: "ASCII, karakter basina 1 bayt" -> her bayt < 128 olmali. */
            if (buf[i] >= 128) {
                fclose(fp);
                return 0;
            }
        }
    }

    int err = ferror(fp);
    fclose(fp);
    return err ? -1 : 1;
}

size_t get_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return (size_t)-1;
    }
    return (size_t)st.st_size;
}

mode_t get_file_permissions(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return (mode_t)-1;
    }
    return st.st_mode & 0777;
}
