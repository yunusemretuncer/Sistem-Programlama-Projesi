#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "sau_format.h"

/* `num_files` giriş dosyasını `output_path` adındaki SAU arşivine yazar.
 * Tüm girişlerin ASCII metin olmasını, toplam boyutun MAX_TOTAL_SIZE'ı
 * aşmamasını ve dosya sayısının MAX_FILES'ı aşmamasını denetler.
 *
 * Başarıda 0, hata durumunda 1 döner (program çıkış kodu olarak kullanılır).
 * Tüm hata mesajları stderr'e yazılır. */
int archive_create(const char *output_path,
                   const char **input_files,
                   int num_files);

#endif /* ARCHIVE_H */
