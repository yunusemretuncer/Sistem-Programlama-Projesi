/* sau_format.c — Ortak SAU format okuma/yazma kodu.
 * TODO: archive.c ile birlikte 3. adimda doldurulacak. */

#include <stdio.h>

#include "sau_format.h"

int sau_write_organization(FILE *out, const sau_entry_t *entries, int count) {
    (void)out; (void)entries; (void)count;
    return 0;
}

int sau_read_organization(FILE *in, sau_entry_t *entries, int *count) {
    (void)in; (void)entries; (void)count;
    return 0;
}
