# 4. Yazılım Mimarisi

Yazılım iki katmandan oluşur: Arduino üzerinde çalışan test yazılımı ve bilgisayarda çalışan Python kayıt aracı.
Bu bölümde, test sonuçlarının alındığı yazılım yer alır.

```
Bilgisayar (Python)  ──USB seri, 115200 baud──►  Arduino  ──Modbus RTU, 9600 baud──►  Referans sensör
                     ◄──────── ölçüm satırları ──           ──PWM──►  Motor sürücü
```

## 4.1 Arduino yazılımı

| Program | Görev |
|---|---|
| [`otomatik_test`](arduino/otomatik_test) | Test adımlarını kendisi çalıştırır, sonuçları CSV olarak yazar. |

**Sensör haberleşmesi:** Modbus RTU, fonksiyon 03 (holding register okuma), register `0x0002`, 32 bit kayan noktalı sayı (bar).

**Komutlar:** `basla` testi başlatır, `dur` testi durdurur.

**Otomatik test akışı:**
1. Gürültü ölçümü: pompalar kapalı, 60 sn
2. Basınç–PWM eğrisi: 0'dan 255'e 14 adım, her adımda 10 sn yerleşme + 20 sn ölçüm
3. Salınım ölçümü: seçili PWM değerlerinde 60 sn

Çıktı biçimi: `zaman_ms,etiket,pompa,pwm,bar,kpa`

## 4.2 Python kayıt aracı

| Araç | Görev |
|---|---|
| [`otomatik_kayit.py`](python/otomatik_kayit.py) | Otomatik testi başlatır, tüm çıktıyı zaman damgalı dosyaya kaydeder. |

```bash
pip install -r python/requirements.txt
python python/otomatik_kayit.py COM5
```
