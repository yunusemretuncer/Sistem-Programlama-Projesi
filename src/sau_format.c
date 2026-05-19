/* sau_format.c — Ortak SAU format okuma/yazma. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "sau_format.h"

#define MAX_RECORD_LEN     (MAX_FILENAME_LEN + 64)
#define MAX_RECORDS_BUFFER (MAX_FILES * MAX_RECORD_LEN)

/* ============================================================
 * YAZMA
 * ============================================================ */

int sau_write_organization(FILE *out, const sau_entry_t *entries, int count) {
    if (count <= 0) {
        return -1;
    }

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

    if (pos + 1 >= sizeof(records)) {
        return -1;
    }
    records[pos++] = '|';

    if (fprintf(out, "%0*zu", HEADER_SIZE_BYTES, pos) != HEADER_SIZE_BYTES) {
        return -1;
    }
    if (fwrite(records, 1, pos, out) != pos) {
        return -1;
    }

    return 0;
}

/* ============================================================
 * OKUMA
 * ============================================================ */

/* Strtoul wrapper'i — tam sayi mi yoksa cop mu kontrolu yapar. */
static int parse_uint(const char *s, int base, unsigned long *out) {
    if (!s || *s == '\0') return -1;
    char *end;
    errno = 0;
    unsigned long v = strtoul(s, &end, base);
    if (errno != 0 || *end != '\0') return -1;
    *out = v;
    return 0;
}

int sau_read_organization(FILE *in, sau_entry_t *entries, int *count) {
    *count = 0;

    /* 1. 10-bayt baslik. */
    char header[HEADER_SIZE_BYTES + 1];
    if (fread(header, 1, HEADER_SIZE_BYTES, in) != HEADER_SIZE_BYTES) {
        return -1;
    }
    header[HEADER_SIZE_BYTES] = '\0';

    /* Sifirla doldurulmus olabilir ama hepsi rakam olmali. */
    for (int i = 0; i < HEADER_SIZE_BYTES; i++) {
        if (!isdigit((unsigned char)header[i])) {
            return -1;
        }
    }

    unsigned long org_size;
    if (parse_uint(header, 10, &org_size) != 0) {
        return -1;
    }
    if (org_size == 0 || org_size > MAX_RECORDS_BUFFER) {
        return -1;
    }

    /* 2. Org bolumu butun olarak oku. */
    char *buf = malloc(org_size + 1);
    if (!buf) {
        return -1;
    }
    if (fread(buf, 1, org_size, in) != org_size) {
        free(buf);
        return -1;
    }
    buf[org_size] = '\0';

    /* 3. Format: |name,perm,size|name,perm,size|...|
     * Ilk karakter '|' olmali. */
    if (buf[0] != '|') {
        free(buf);
        return -1;
    }

    char *p = buf + 1;
    int n = 0;

    while (*p != '\0' && n < MAX_FILES) {
        /* Bu kaydin sonundaki '|'i bul. */
        char *end = strchr(p, '|');
        if (!end) {
            free(buf);
            return -1;
        }
        *end = '\0';

        /* 'name,perm,size' formatini parcala. */
        char *c1 = strchr(p, ',');
        if (!c1) { free(buf); return -1; }
        *c1++ = '\0';

        char *c2 = strchr(c1, ',');
        if (!c2) { free(buf); return -1; }
        *c2++ = '\0';

        /* p = name, c1 = perm, c2 = size */
        if (*p == '\0' || strlen(p) >= MAX_FILENAME_LEN) {
            free(buf);
            return -1;
        }

        unsigned long perm_val, size_val;
        if (parse_uint(c1, 8, &perm_val) != 0 || perm_val > 0777) {
            free(buf);
            return -1;
        }
        if (parse_uint(c2, 10, &size_val) != 0) {
            free(buf);
            return -1;
        }

        strncpy(entries[n].filename, p, MAX_FILENAME_LEN - 1);
        entries[n].filename[MAX_FILENAME_LEN - 1] = '\0';
        entries[n].permissions = (mode_t)perm_val;
        entries[n].size = (size_t)size_val;
        n++;

        p = end + 1;
    }

    /* Hala kayit varsa, MAX_FILES asilmis demek. */
    if (*p != '\0') {
        free(buf);
        return -1;
    }

    *count = n;
    free(buf);
    return 0;
}