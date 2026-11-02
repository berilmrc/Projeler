// Otomatik test: pompalari ve sensor basincini test planina gore kendisi calistirir.
// Pin haritasi (bkz. 03_Donanim_Mimarisi):
//   Pompa A: ENA -> D9, IN1 -> D8, IN2 -> D7
//   Pompa B: ENB -> D3, IN3 -> D5, IN4 -> D4
//   Basinc sensoru (RS485 modulu): RO -> D10, DI -> D11, DE + RE -> D2
// Komutlar:
//   basla   testi baslatir (adimlar bitene kadar calisir)
//   dur     testi durdurur, pompalar kapanir
// Cikti (115200 baud): ilk satir baslik, sonra her saniye bir CSV satiri.
//   zaman_ms,etiket,pompa,pwm,bar,kpa
// "# " ile baslayan satirlar adim bilgisidir, analizde atlanir.
// Ciktiyi Serial Monitor'den kopyalayip .txt olarak kaydet.

#include <SoftwareSerial.h>
#include <string.h>

// --- Pompalar ---
const uint8_t PIN_ENA = 9;
const uint8_t PIN_ENB = 3;
const uint8_t PIN_IN1 = 8;
const uint8_t PIN_IN2 = 7;
const uint8_t PIN_IN3 = 5;
const uint8_t PIN_IN4 = 4;

uint8_t hizA = 0;
uint8_t hizB = 0;

// --- Basinc sensoru ---
const uint8_t PIN_RO = 10;
const uint8_t PIN_DI = 11;
const uint8_t PIN_DE_RE = 2;
const unsigned long CEVAP_ZAMAN_ASIMI_MS = 200;

SoftwareSerial sensor(PIN_RO, PIN_DI);

// adres 1, F3, register 0x0002, 2 register, CRC (65 CB)
const uint8_t SORGU[8] = {0x01, 0x03, 0x00, 0x02, 0x00, 0x02, 0x65, 0xCB};

// --- Test ayarlari (saniye) ---
const unsigned long GURULTU_S = 60;        // pompalar kapali, sensor gurultusu
const unsigned long BEKLEME_S = 10;        // her adimda yerlesme suresi
const unsigned long EGRI_OLCUM_S = 20;     // basinc-PWM egrisinde olcum suresi
const unsigned long SALINIM_OLCUM_S = 60;  // salinim logu suresi
const unsigned long DURUM_ARASI_S = 5;     // adimlar arasinda pompalar kapali
const bool ASAGI_DE = false;               // true ise egri asagi yonde de olculur

const uint8_t EGRI_PWM[] = {0, 50, 80, 100, 110, 120, 130, 140, 150, 160, 180, 200, 230, 255};
const uint8_t SALINIM_PWM[] = {110, 120, 130, 150, 180, 255};
const int EGRI_N = sizeof(EGRI_PWM);
const int SALINIM_N = sizeof(SALINIM_PWM);
const char* const POMPALAR[] = {"AB"};  // sadece iki pompa birlikte
const int POMPA_N = 1;

unsigned long testBaslangicMs = 0;
bool durduruldu = false;

// --- Pompa fonksiyonlari ---

void hizlariUygula() {
  analogWrite(PIN_ENA, hizA);
  analogWrite(PIN_ENB, hizB);
}

void durdur() {
  hizA = 0;
  hizB = 0;
  hizlariUygula();
}

// pompa: "A", "B", "AB" ya da "K" (kapali)
void pompaAyarla(const char* pompa, uint8_t pwm) {
  hizA = 0;
  hizB = 0;
  if (strcmp(pompa, "A") == 0) {
    hizA = pwm;
  } else if (strcmp(pompa, "B") == 0) {
    hizB = pwm;
  } else if (strcmp(pompa, "AB") == 0) {
    hizA = pwm;
    hizB = pwm;
  }
  hizlariUygula();
}

// --- Seri komut ve bekleme ---

// Seri porttan "dur" gelmis mi bakar. Geldiyse durduruldu bayragini kaldirir.
bool durmaIstendi() {
  while (Serial.available()) {
    String satir = Serial.readStringUntil('\n');
    satir.trim();
    satir.toLowerCase();
    if (satir == "dur") durduruldu = true;
  }
  return durduruldu;
}

// Belirtilen saniye kadar bekler. Dur istenirse false doner.
bool bekle(unsigned long saniye) {
  unsigned long t0 = millis();
  while (millis() - t0 < saniye * 1000UL) {
    if (durmaIstendi()) return false;
  }
  return true;
}

// --- Basinc sensoru fonksiyonlari ---

uint16_t crc16(const uint8_t *veri, uint8_t n) {
  uint16_t c = 0xFFFF;
  for (uint8_t i = 0; i < n; i++) {
    c ^= veri[i];
    for (uint8_t k = 0; k < 8; k++) {
      c = (c & 1) ? (c >> 1) ^ 0xA001 : (c >> 1);
    }
  }
  return c;
}

// Sorgu gonderir, cevabi en fazla 9 bayt olarak okur. Donen deger okunan bayt sayisi.
uint8_t sorgula(uint8_t *cevap) {
  while (sensor.available()) sensor.read();

  digitalWrite(PIN_DE_RE, HIGH);
  sensor.write(SORGU, sizeof(SORGU));
  delay(2);
  digitalWrite(PIN_DE_RE, LOW);

  uint8_t n = 0;
  unsigned long t0 = millis();
  while (n < 9 && millis() - t0 < CEVAP_ZAMAN_ASIMI_MS) {
    if (sensor.available()) cevap[n++] = sensor.read();
  }
  return n;
}

// Basinci okur. Basariliysa true doner ve bar degerini yazar.
bool sensorOku(float &bar) {
  uint8_t cevap[9];
  if (sorgula(cevap) != 9) return false;
  if (cevap[0] != 0x01 || cevap[1] != 0x03 || cevap[2] != 0x04) return false;
  if (crc16(cevap, 7) != (uint16_t)(cevap[7] | (cevap[8] << 8))) return false;

  uint32_t ham = ((uint32_t)cevap[3] << 24) | ((uint32_t)cevap[4] << 16) |
                 ((uint32_t)cevap[5] << 8) | cevap[6];
  memcpy(&bar, &ham, 4);
  return !isnan(bar) && !isinf(bar);
}

// --- Test akisi ---

void satirYaz(const char* etiket, const char* pompa, uint8_t pwm, bool okundu, float bar) {
  Serial.print(millis() - testBaslangicMs);
  Serial.print(",");
  Serial.print(etiket);
  Serial.print(",");
  Serial.print(pompa);
  Serial.print(",");
  Serial.print(pwm);
  Serial.print(",");
  if (okundu) {
    Serial.print(bar, 5);
    Serial.print(",");
    Serial.print(bar * 100.0, 3);
    Serial.println();
  } else {
    Serial.println("NA,NA");
  }
}

// Bir test adimi: pompayi ayarla, bekle, olc, pompalari kapat, dinlen.
// Dur istenirse false doner.
bool adimCalistir(const char* etiket, const char* pompa, uint8_t pwm,
                  unsigned long beklemeS, unsigned long olcumS) {
  Serial.print("# adim: ");
  Serial.print(etiket);
  Serial.print(" ");
  Serial.print(pompa);
  Serial.print(" ");
  Serial.println(pwm);

  pompaAyarla(pompa, pwm);
  if (!bekle(beklemeS)) {
    durdur();
    return false;
  }

  unsigned long t0 = millis();
  unsigned long sonraki = t0;
  while (millis() - t0 < olcumS * 1000UL) {
    if (durmaIstendi()) {
      durdur();
      return false;
    }
    if (millis() >= sonraki) {
      sonraki += 1000;
      float bar = 0;
      bool okundu = sensorOku(bar);
      satirYaz(etiket, pompa, pwm, okundu, bar);
    }
  }

  durdur();
  return bekle(DURUM_ARASI_S);
}

void iptal() {
  durdur();
  Serial.println("# test durduruldu");
}

void testiCalistir() {
  durduruldu = false;
  testBaslangicMs = millis();
  Serial.println("# test basladi");
  Serial.println("zaman_ms,etiket,pompa,pwm,bar,kpa");

  if (!adimCalistir("gurultu", "K", 0, 0, GURULTU_S)) { iptal(); return; }

  for (int i = 0; i < EGRI_N; i++) {
    for (int k = 0; k < POMPA_N; k++) {
      if (!adimCalistir("egri", POMPALAR[k], EGRI_PWM[i], BEKLEME_S, EGRI_OLCUM_S)) { iptal(); return; }
    }
  }

  if (ASAGI_DE) {
    for (int i = EGRI_N - 1; i >= 0; i--) {
      for (int k = 0; k < POMPA_N; k++) {
        if (!adimCalistir("egri_asagi", POMPALAR[k], EGRI_PWM[i], BEKLEME_S, EGRI_OLCUM_S)) { iptal(); return; }
      }
    }
  }

  for (int i = 0; i < SALINIM_N; i++) {
    for (int k = 0; k < POMPA_N; k++) {
      if (!adimCalistir("salinim", POMPALAR[k], SALINIM_PWM[i], BEKLEME_S, SALINIM_OLCUM_S)) { iptal(); return; }
    }
  }

  durdur();
  Serial.println("# test bitti");
}

// --- Arduino giris noktalari ---

void setup() {
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_DE_RE, OUTPUT);

  // Iki pompa ayni yonde: IN1 ve IN3 HIGH, IN2 ve IN4 LOW
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, HIGH);
  digitalWrite(PIN_IN4, LOW);

  digitalWrite(PIN_DE_RE, LOW);
  durdur();

  Serial.begin(115200);
  sensor.begin(9600);
  delay(300);

  Serial.println("# hazir. Komut: basla, dur");
}

void loop() {
  if (Serial.available()) {
    String satir = Serial.readStringUntil('\n');
    satir.trim();
    satir.toLowerCase();
    if (satir == "basla") {
      testiCalistir();
    } else if (satir == "dur") {
      durdur();
      Serial.println("# durduruldu");
    }
  }
}
