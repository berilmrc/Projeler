# 3. Donanım Mimarisi

## 3.1 Elektronik

| Bileşen | Görev |
|---|---|
| Arduino Uno | Ana kontrol kartı |
| Referans basınç sensörü | Mutlak basınç ölçümü, RS485 çıkışlı |
| RS485 dönüştürücü | Arduino (TTL) ile sensör (RS485) arasında çevirici |
| H-köprü motor sürücü (2 kanal) | Pompaların hız ve yön kontrolü |
| 12 V DC güç kaynağı | Pompa ve sensör beslemesi |

### Pin bağlantıları

| Arduino pini | Bağlantı | Görev |
|---|---|---|
| D9 | Sürücü EN A | Pompa A hızı (PWM) |
| D8, D7 | Sürücü IN1, IN2 | Pompa A yönü |
| D3 | Sürücü EN B | Pompa B hızı (PWM) |
| D5, D4 | Sürücü IN3, IN4 | Pompa B yönü |
| D10 | RS485 RO | Sensörden gelen veri |
| D11 | RS485 DI | Sensöre giden sorgu |
| D2 | RS485 DE + RE | Gönderme / alma yönü |

### Güç ve topraklama

- Pompalar ve sensör 12 V kaynaktan beslenir.
- Arduino, motor sürücü, sensör ve kaynak **yıldız GND** ile tek noktada topraklanır (T-05). Bu bağlantı olmadan PWM sinyali sürücüye ulaşmaz.
- Sürücünün kendi 5 V regülatörü ile Arduino'nun 5 V çıkışı aynı anda bağlanmaz.

## 3.2 Pnömatik

| Bileşen | Görev |
|---|---|
| Vakum pompası A ve B | Hazneden hava çekerek basıncı düşürür |
| Ortak hat (T bağlantı) | İki pompayı hazneye bağlar |
| Test haznesi | Basıncın oluştuğu kapalı hacim |
| Ölçüm hattı | Hazneyi referans sensöre bağlar |
