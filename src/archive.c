/* archive.c — -b modu (SAU arsiv olusturma).
 * TODO: Bir sonraki adimda doldurulacak. */

#include <stdio.h>

#include "archive.h"

int archive_create(const char *output_path,
                   const char **input_files,
                   int num_files) {
    fprintf(stderr,
            "[stub] archive_create: %d dosya -> '%s'\n",
            num_files, output_path);
    for (int i = 0; i < num_files; i++) {
        fprintf(stderr, "  - %s\n", input_files[i]);
    }
    return 0;
}
