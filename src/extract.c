/* extract.c — -a modu (SAU arsiv acma).
 * TODO: 4. adimda doldurulacak. */

#include <stdio.h>

#include "extract.h"

int archive_extract(const char *archive_path, const char *output_dir) {
    fprintf(stderr,
            "[stub] archive_extract: '%s' -> '%s'\n",
            archive_path,
            output_dir ? output_dir : "(mevcut dizin)");
    return 0;
}
