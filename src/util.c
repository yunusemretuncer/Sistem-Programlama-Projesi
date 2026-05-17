/* util.c — Yardimci fonksiyonlar.
 * is_ascii_text_file: 3. adimda tam olarak yazilacak; simdilik stub.
 * Diger iki fonksiyon (stat tabanli) zaten kullanilabilir. */

#include <stdio.h>
#include <sys/stat.h>

#include "util.h"

int is_ascii_text_file(const char *path) {
    (void)path;
    return 1; /* TODO: 3. adimda gerçek kontrol */
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
