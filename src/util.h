#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <sys/types.h>

/* Dosyanın yalnızca ASCII karakterleri (her bayt < 128) içerip içermediğini
 * kontrol eder. ASCII ise 1, değilse 0, dosya açılamazsa -1 döner. */
int is_ascii_text_file(const char *path);

/* Dosyanın boyutunu bayt cinsinden döner. Hata halinde (size_t)-1 döner. */
size_t get_file_size(const char *path);

/* Dosyanın POSIX izin bitlerini (mode & 0777) döner.
 * Hata halinde (mode_t)-1 döner. */
mode_t get_file_permissions(const char *path);

#endif /* UTIL_H */
