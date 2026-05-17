#ifndef EXTRACT_H
#define EXTRACT_H

/* `archive_path` adındaki SAU arşivini açar ve `output_dir` dizinine
 * dosyaları çıkarır. `output_dir` NULL ise mevcut dizine açar.
 * Dizin yoksa oluşturur. Açılan dosyaların izinleri arşivlenirken
 * sahip oldukları izinlerle aynı olur.
 *
 * Başarıda 0, hata durumunda 1 döner. */
int archive_extract(const char *archive_path, const char *output_dir);

#endif /* EXTRACT_H */
