# tarsau — Sıkıştırmasız Arşivleyici

Bilgisayar Mühendisliği Sistem Programlama 2025-2026 Bahar Dönemi Projesi.

`tar`/`zip` gibi çalışan, **sıkıştırma yapmayan** basit bir arşivleme programı.
Birden fazla ASCII metin dosyasını tek bir `.sau` dosyasında birleştirir ve
geri açar.

## Derleme

```bash
make
```

Temizleme:

```bash
make clean
```

## Test

Otomatik test scripti, 11 farklı senaryoyu (arşivleme, çıkarma, izin koruma,
ASCII reddi, bozuk arşiv tespiti, nested dizin oluşturma, boş dosya, vb.)
çalıştırıp her birinin spec'e uygun çıktı verdiğini doğrular:

```bash
make test
```

Tüm testler geçtiğinde çıktının sonunda `Tum testler gecti: 11 / 11` görünür.

## Kullanım

### Arşivleme (`-b`)

```bash
./tarsau -b dosya1 dosya2 dosya3 -o arsiv.sau
```

- `-o` belirtilmezse varsayılan olarak `a.sau` kullanılır.
- En fazla 32 dosya, toplam en fazla 200 MB.
- Giriş dosyalarının tamamı ASCII metin olmalıdır. Aksi halde
  `"<dosya> giriş dosyasının formatı uyumsuzdur!"` mesajı yazdırılır ve
  program sorunsuz çıkar.

### Çıkarma (`-a`)

```bash
./tarsau -a arsiv.sau hedef_dizin
```

- Hedef dizin belirtilmezse arşiv mevcut dizine açılır.
- Hedef dizin yoksa otomatik olarak oluşturulur.
- Açılan dosyalar **arşivlendikleri sırada sahip oldukları izinleri** korur.

## SAU Dosya Formatı

İki bölümden oluşur:

1. **Organizasyon bölümü**
   - İlk **10 bayt:** Bu bölümün boyutu (sıfır dolgulu ASCII sayı, örn. `0000000035`)
   - Sonrasında her kayıt `|dosya_adı,izinler,boyut|` formatında, ardışık.
2. **İçerik bölümü**
   - Dosyaların ham içeriği, hiçbir ayırıcı kullanılmadan peş peşe yerleştirilir.
   - Her dosyanın nerede bittiği, organizasyon bölümündeki `boyut` alanından okunur.

### Örnek

3 dosya (`t1`=6B, `t2`=7B, `t3.txt`=3B) için `.sau` içeriği:

```
0000000035|t1,644,6|t2,755,7|t3.txt,644,3|Hello\nWorld!\nabc
```

## Proje Yapısı

```
tarsau/
├── Makefile            # make / make clean / make test
├── README.md
├── src/
│   ├── main.c          # CLI ayrıştırma, -b / -a dağıtımı
│   ├── archive.c/.h    # -b arşivleme modu
│   ├── extract.c/.h    # -a çıkarma modu
│   ├── sau_format.c/.h # Ortak SAU format kodu
│   └── util.c/.h       # ASCII doğrulama, dosya yardımcıları
└── tests/
    ├── run_tests.sh    # Otomatik test scripti (11 senaryo)
    └── data/           # Test dosyaları
```

## Geliştiriciler

- Sena Kuzğu - G231210385
- Yunus Emre Tuncer - G231210045
