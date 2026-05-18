/* sau_format.c — Ortak SAU format okuma/yazma.
 * Bu adimda sadece yazma kismi gercek; okuma 4. adimda yazilacak. */

#include <stdio.h>
#include <string.h>

#include "sau_format.h"

/* Tek kayit icin ust sinir: |dosya_adi,izinler(oktal 4 hane),boyut(20 hane)|
 *   2 pipe + filename + 3 virgul karakterleri + sayilar + emniyet pay
 * = MAX_FILENAME_LEN + 64 yeterli. */
#define MAX_RECORD_LEN     (MAX_FILENAME_LEN + 64)
#define MAX_RECORDS_BUFFER (MAX_FILES * MAX_RECORD_LEN)

int sau_write_organization(FILE *out, const sau_entry_t *entries, int count) {
    if (count <= 0) {
        return -1;
    }

    /* Once kayit dizisini bellekte olustur. Boylece kesin boyut bilinir
     * ve 10 baytlik basligi onune yazabiliriz. */
    static char records[MAX_RECORDS_BUFFER];
    size_t pos = 0;

    for (int i = 0; i < count; i++) {
        int written = snprintf(records + pos,
                               sizeof(records) - pos,
                               "|%s,%o,%zu",
                               entries[i].filename,
                               (unsigned int)(entries[i].permissions & 0777),
                               entries[i].size);
        if (written < 0 || (size_t)written >= sizeof(records) - pos) {
            return -1;
        }
        pos += (size_t)written;
    }

    /* Son kapanis pipe'i. */
    if (pos + 1 >= sizeof(records)) {
        return -1;
    }
    records[pos++] = '|';

    /* 10 baytlik sifir dolgulu ASCII boyut baligi.
     * %010zu uzunlugu pos'a gore dinamik degildir; pos < 10^10 oldugu surece
     * tam 10 karaktere padlenir. MAX_RECORDS_BUFFER ~ 10KB civari, sorun yok. */
    if (fprintf(out, "%0*zu", HEADER_SIZE_BYTES, pos) != HEADER_SIZE_BYTES) {
        return -1;
    }

    /* Sonra kayitlar. */
    if (fwrite(records, 1, pos, out) != pos) {
        return -1;
    }

    return 0;
}

int sau_read_organization(FILE *in, sau_entry_t *entries, int *count) {
    /* TODO: 4. adimda doldurulacak. */
    (void)in; (void)entries; (void)count;
    return 0;
}
