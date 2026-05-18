#!/bin/bash
# run_tests.sh — tarsau icin otomatik test scripti.
# Kullanim: ./tests/run_tests.sh   (proje kok dizininden)
#
# Cikis kodu: 0 = tum testler gecti, 1 = en az bir test basarisiz.

set -u

# Renkler
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Sayaclar
PASS=0
FAIL=0

# Test isi calistirma dizini
WORK_DIR="$(mktemp -d)"
trap "rm -rf '$WORK_DIR'" EXIT

# Proje kok dizinindeki tarsau ikilisi
TARSAU="$(pwd)/tarsau"
DATA_DIR="$(pwd)/tests/data"

if [ ! -x "$TARSAU" ]; then
    echo -e "${RED}HATA: '$TARSAU' bulunamadi. Once 'make' calistirin.${NC}"
    exit 1
fi

# Yardimci fonksiyonlar
report_pass() {
    PASS=$((PASS + 1))
    echo -e "  ${GREEN}✓${NC} $1"
}

report_fail() {
    FAIL=$((FAIL + 1))
    echo -e "  ${RED}✗${NC} $1"
    if [ -n "${2:-}" ]; then
        echo -e "    ${YELLOW}beklenen:${NC} $2"
    fi
    if [ -n "${3:-}" ]; then
        echo -e "    ${YELLOW}alinan:${NC}  $3"
    fi
}

# === TEST 1: Temel arsivleme ===
echo "TEST 1: Temel arsivleme (-b)"
cd "$WORK_DIR"
cp "$DATA_DIR/t1.txt" "$DATA_DIR/t2.sh" "$DATA_DIR/t3.dat" .
out=$("$TARSAU" -b t1.txt t2.sh t3.dat -o s1.sau 2>&1)
if [ "$out" = "Dosyalar birleştirildi." ] && [ -f s1.sau ]; then
    report_pass "arsiv olusturuldu, dogru mesaj basildi"
else
    report_fail "yanlis mesaj veya arsiv yok" "Dosyalar birleştirildi." "$out"
fi

# === TEST 2: Varsayilan a.sau ===
echo "TEST 2: -o belirtilmediginde a.sau"
rm -f a.sau
"$TARSAU" -b t1.txt t2.sh > /dev/null 2>&1
if [ -f a.sau ]; then
    report_pass "varsayilan a.sau olusturuldu"
else
    report_fail "a.sau olusturulmadi"
fi

# === TEST 3: Cikarma ve icerik dogrulugu ===
echo "TEST 3: Round-trip (arsivle, cikar, karsilastir)"
cd "$WORK_DIR"
out=$("$TARSAU" -a s1.sau d1 2>&1)
expected="d1 dizininde t1.txt, t2.sh ve t3.dat dosyaları açıldı."
if [ "$out" = "$expected" ]; then
    report_pass "spec mesaji birebir uyuyor"
else
    report_fail "spec mesaji uymuyor" "$expected" "$out"
fi

all_match=true
for f in t1.txt t2.sh t3.dat; do
    if ! diff -q "$f" "d1/$f" > /dev/null 2>&1; then
        all_match=false
        report_fail "icerik farkli: $f"
    fi
done
$all_match && report_pass "tum dosyalarin icerigi birebir aynı"

# === TEST 4: Izin koruma ===
echo "TEST 4: Izin koruma"
all_match=true
declare -A expected_perms=( [t1.txt]=644 [t2.sh]=755 [t3.dat]=600 )
for f in t1.txt t2.sh t3.dat; do
    orig="${expected_perms[$f]}"
    actual=$(stat -c '%a' "d1/$f")
    if [ "$orig" != "$actual" ]; then
        all_match=false
        report_fail "izin uymuyor: $f" "$orig" "$actual"
    fi
done
$all_match && report_pass "tum izinler korundu"

# === TEST 5: Mevcut dizine cikarma ===
echo "TEST 5: Mevcut dizine cikarma"
mkdir cur && cd cur
out=$("$TARSAU" -a ../s1.sau 2>&1)
expected="Mevcut dizinde t1.txt, t2.sh ve t3.dat dosyaları açıldı."
if [ "$out" = "$expected" ]; then
    report_pass "mevcut dizin mesaji dogru"
else
    report_fail "mesaj farkli" "$expected" "$out"
fi
cd ..

# === TEST 6: Binary dosya reddi ===
echo "TEST 6: ASCII olmayan dosya reddi"
cp "$DATA_DIR/binary.dat" .
out=$("$TARSAU" -b t1.txt binary.dat -o test.sau 2>&1)
expected="binary.dat giriş dosyasının formatı uyumsuzdur!"
if [ "$out" = "$expected" ]; then
    report_pass "spec mesaji birebir uyuyor"
else
    report_fail "mesaj farkli" "$expected" "$out"
fi

# === TEST 7: Bozuk arsiv ===
echo "TEST 7: Bozuk arsiv tespiti"
echo "GARBAGE" > bad.sau
out=$("$TARSAU" -a bad.sau out_bad 2>&1)
expected="Arşiv dosyası uygunsuz veya bozuk!"
if [ "$out" = "$expected" ]; then
    report_pass "spec mesaji birebir uyuyor"
else
    report_fail "mesaj farkli" "$expected" "$out"
fi

# === TEST 8: Yanlis uzanti ===
echo "TEST 8: .sau uzantili olmayan arsiv"
out=$("$TARSAU" -a s1.txt out_x 2>&1)
expected="Arşiv dosyası uygunsuz veya bozuk!"
if [ "$out" = "$expected" ]; then
    report_pass "yanlis uzanti reddedildi"
else
    report_fail "mesaj farkli" "$expected" "$out"
fi

# === TEST 9: Nested dizin olusturma ===
echo "TEST 9: Nested hedef dizin"
"$TARSAU" -a s1.sau "a/b/c" > /dev/null 2>&1
if [ -d "a/b/c" ] && [ -f "a/b/c/t1.txt" ]; then
    report_pass "nested dizin otomatik olusturuldu"
else
    report_fail "nested dizin olusturulmadi"
fi

# === TEST 10: Bos dosya ===
echo "TEST 10: Bos dosya arsivleme"
cp "$DATA_DIR/empty.txt" .
"$TARSAU" -b t1.txt empty.txt -o empty_test.sau > /dev/null 2>&1
"$TARSAU" -a empty_test.sau empty_out > /dev/null 2>&1
if [ -f "empty_out/empty.txt" ] && [ ! -s "empty_out/empty.txt" ]; then
    report_pass "bos dosya korundu (0 bayt)"
else
    report_fail "bos dosya bozuldu"
fi

# === OZET ===
echo
echo "============================================"
TOTAL=$((PASS + FAIL))
if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}Tum testler gecti: $PASS / $TOTAL${NC}"
    exit 0
else
    echo -e "${RED}$FAIL test basarisiz, $PASS test gecti ($TOTAL toplam)${NC}"
    exit 1
fi
