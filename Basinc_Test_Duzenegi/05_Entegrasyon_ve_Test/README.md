# 5. Entegrasyon ve Test

## 5.1 Entegrasyon sırası

Sistem parça parça kuruldu, her adım bir öncekine eklenmeden önce doğrulandı:

1. Referans sensör bilgisayardan okundu.
2. Sensör Arduino'ya bağlandı ve okundu.
3. Pompalar motor sürücüyle tek tek çalıştırıldı.
4. Pompa kontrolü ve sensör okuma tek programda birleştirildi.
5. Otomatik test yazılımı ve kayıt aracı eklendi.

## 5.2 Karşılaşılan sorunlar ve çözümler

| Sorun | Neden | Çözüm |
|---|---|---|
| Seri port erişim hatası | Port başka bir programca açıktı | Diğer program kapatıldı |
| Pompalar dönmedi | Arduino ile sürücü arasında ortak toprak yoktu | GND hatları birleştirildi |
| Sürücü regülatörü ısındı | İki 5 V kaynak aynı hatta bağlıydı | Tek 5 V kaynak kullanma kuralı konuldu |
| PWM etkisiz kaldı | Sürücüdeki hız kapakları takılıydı | Kapaklar çıkarıldı |
| Düşük hızda basınç salınımı | Pompanın başlatma eşiğine yakın çalışma | Otomatik testle ölçülüp belgelendi |

## 5.3 Test yöntemi

- Pompa hızı (PWM) adım adım artırıldı, her adımda basınç kaydedildi.
- Her adımda ortalama, en düşük, en yüksek ve tepe-tepe değerleri hesaplandı.
- Uzun süreli çalışmada sürücü ısınması ve ikinci pompanın basınca katkısı gözlendi.
