#ifndef SAU_FORMAT_H
#define SAU_FORMAT_H

#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>

/* SAU format sabitleri */
#define MAX_FILES         32
#define MAX_TOTAL_SIZE    (200L * 1024 * 1024)   /* 200 MB */
#define HEADER_SIZE_BYTES 10                     /* İlk 10 bayt: org. bölüm boyutu */
#define MAX_FILENAME_LEN  256

/* Tek bir arşiv kaydını temsil eder. */
typedef struct {
    char    filename[MAX_FILENAME_LEN];
    mode_t  permissions;   /* POSIX izin bitleri (örn. 0644) */
    size_t  size;          /* Dosya boyutu (bayt) */
} sau_entry_t;

/* Organizasyon (içerik) bölümünü `out` dosyasına yazar.
 * İlk 10 bayt (org. bölüm boyutu) + |dosya,izin,boyut|... kayıtları.
 * Başarıda 0, hatada negatif değer döner. */
int sau_write_organization(FILE *out, const sau_entry_t *entries, int count);

/* Organizasyon bölümünü `in` dosyasından okur.
 * `entries` en az MAX_FILES kapasiteli olmalıdır.
 * Okunan kayıt sayısı `*count` içine yazılır.
 * Başarıda 0, format bozuksa negatif değer döner. */
int sau_read_organization(FILE *in, sau_entry_t *entries, int *count);

#endif /* SAU_FORMAT_H */
