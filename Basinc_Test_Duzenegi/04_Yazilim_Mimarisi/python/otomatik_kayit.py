"""Otomatik testi baslatir ve seri ciktinin tamamini dosyaya kaydeder.

Kullanim:  python otomatik_kayit.py COM5
Gerekli:   pip install pyserial
Cikti:     kayit_YYYYMMDD_HHMMSS.txt (betigin calistigi klasorde)

Ctrl+C ile durdurursan test de durdurulur.
"""
import sys
import time
from datetime import datetime
from pathlib import Path

import serial

BAUD = 115200
BITIS = ("# test bitti", "# test durduruldu")


def main():
    if len(sys.argv) < 2:
        print("Kullanim: python otomatik_kayit.py COM5")
        return 1

    port = sys.argv[1]
    dosya = Path.cwd() / f"kayit_{datetime.now():%Y%m%d_%H%M%S}.txt"

    with serial.Serial(port, BAUD, timeout=2) as seri, open(dosya, "w", encoding="utf-8") as f:
        time.sleep(2)  # Uno, port acilinca resetlenir. Acilisin bitmesini bekle.
        seri.reset_input_buffer()
        seri.write(b"basla\n")
        print(f"Test basladi. Kayit: {dosya}")

        try:
            while True:
                satir = seri.readline().decode("ascii", errors="replace").strip()
                if not satir:
                    continue
                f.write(satir + "\n")
                f.flush()
                if satir.startswith("# adim"):
                    print(satir)
                if satir in BITIS:
                    print(satir)
                    break
        except KeyboardInterrupt:
            seri.write(b"dur\n")
            f.write("# kullanici durdurdu\n")
            print("Durduruldu.")
            return 1

    print("Kayit tamamlandi.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
