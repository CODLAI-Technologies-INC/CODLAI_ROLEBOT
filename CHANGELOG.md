# Changelog

# CODLAI ERA (New Models)

## [Unreleased]

## [1.6.0] - 2026-10-07
### Added
- **Ornekler bastan yazildi (28 ornek):** hepsi ayni kurala uyuyor - en ustte `bool turkish` ile TR/EN secimi, calisirken seri porttan `dil`/`lang` ile degisim, iki dilli ve bloklamayan seri komutlar (`yardim`/`help`). Bir seyi suren ornekler OTOMATIK gosteriyle baslar, B1 (GPIO0) ile MANUEL moda gecilir.
- Ornekler `Klasor/Klasor.ino` yapisina tasindi: Arduino IDE *Dosya > Ornekler* menusunde hepsi gorunur. `library.json` "examples" alani glob kullaniyor.
- `examples/examples.json`: her ornegin yolu, karti, gereken moduller/ayarlar, TR/EN ozeti ve seri komutlari (editor.codlai.com "Kutuphane Ornekleri" ekrani icin; `scripts/generate_examples_json.py` ile uretilir).
- ESP-NOW deviceType 40-49 kutuphane orneklerinin kart kimliklerine ayrildi (40 IOTBOT, 41 MINIBOT, 42 ROLEBOT); ornekler artik 10/20/30 tiplerini paylasmiyor.

### Fixed
- `serverStart`: STA baglanamazsa acilan yedek AP artik modemin adini/sifresini kopyalamiyor - sabit `CODLAI-ROLEBOT` adiyla acilir (sifre 8 karakterden kisaysa `12345678`). AP modunda 1-7 karakterlik sifre sessizce basarisiz oluyordu, artik `12345678` kullaniliyor. **Yedek agin adi degisti.**
- `serverCreateLocalPage` / `serverOnRequest`: "/" adresi "//" oluyordu; kullanici "/" tanimlayinca varsayilan "CODLAI Server is Running!" sayfasi kaldiriliyor (kullanicinin ana sayfasi gorunmuyordu). "sayfa" ve "/sayfa" ayni. DNS yonlendirmesi AP+STA modunda da calisiyor.
- `sendTelegram`: mesaj tam UTF-8 %XX kodlaniyor (eskiden sadece bosluk; Turkce harf, `&`, `#`, `+` mesaji bozuyordu). `getWeather` sehir adini, `getWikipedia` basligi kodluyor. Yeni `urlEncode()` yardimcisi. Sketch'te onceden kodlamayin.
- `getWeather` (OpenWeatherMap): `http://` yerine `https://` (anahtar acik gitmiyor / istek basarisiz oluyordu).
- `wifiStartAndConnect` WiFi sifresini seri porta acik yazmiyor; `wifiConnectionControl` seri portu sadece durum degisince yaziyor (loop icinde doldurmuyordu).
- `eepromReadInt` isaretli 16 bit donuyor: -5 yazilip 65531 okunuyordu; hic yazilmamis alan 65535 yerine -1. `eepromReadString` hic yazilmamis alanda cop yerine "" donuyor.
- **`otaBegin` varsayilan portu 3232 -> 8266** (ESP8266 standardi; Arduino IDE/espota bu portu bekler).

## [1.5.3] - 2026-09-29
### Fixed
- CodlaiESPNowMessage aciklamasi: deviceType 22-29 editor.codlai.com ozel/eslesmeli mesajlasma bloklarina rezerve edildi (22 ozel metin, 23 ozel sayi, 24 eslesme teklifi, 25 eslesme kabulu; 26-29 bos). Kutuphane davranisi degismedi - `espNowAvailable()` hala yalniz 20/21'i gorur.
- Ornekler: `ROLEBOT_Telegram_Notification_Example.ino` ve `ROLEBOT_IFTTT_Webhook_Example.ino` butonu ters okuyup buton basili DEGILKEN surekli mesaj/tetikleme gonderiyordu; duzeltildi.
- CodlaiESPNowMessage aciklamasi: deviceType 3/4 eklendi, 30-39 "CODLAI Robotlari Otonom" projesine rezerve edildi.
- Depoda eski bir PlatformIO kurulum kaydi (`.piopm`, surum 1.6.4) izleniyordu ve GitHub'a da gidiyordu; kutuphaneyi GitHub'dan ya da yerel klasorden (symlink) kuran projelerde bagimlilik agaci yanlis surum gosteriyordu. Dosya kaldirildi ve `.gitignore`'a eklendi. (Duvar projesi oturumunun bulgusu.)
- `sendESPNow()`: peer SABIT kanal 1'e kaydediliyordu; artik o anki WiFi kanali (`wifi_get_channel()`) kullaniliyor. Ayrica basarili her gonderimde basilan "Sent with success" satiri kaldirildi (sadece hata yaziliyor) - bkz. CODLAI_MINIBOT 1.5.3.
- `espNowReadName()` / `espNowReadNumber()`: ilk cagrilan fonksiyon mesaji tuketip digerinin 0/"" dondurmesine yol aciyordu - bkz. CODLAI_IOTBOT 1.7.3. Bu yuzden `ROLEBOT_ESPNOW_Simple_Messaging_Example.ino` gelen sayiyi hep 0 okuyordu; artik dogru calisiyor.

### Added
- **Blok dostu internet saati** (editor bloklari icin): `ntpUpdate()` (son ayarlarla hemen yeniden esitle), `ntpGetHour/Minute/Second/Day/Month/Year()`, `ntpGetWeekday()` (1=Pazartesi...7=Pazar), `ntpGetTimeString()` ("14:05:09"), `ntpGetDateString()` ("29.09.2026"), `ntpTimeIs(saat, dakika)` (o dakika boyunca true), `ntpTimeReached(saat, dakika)` (o dakikaya girince SADECE BIR KEZ true), `ntpTimeIsBetween(s1, d1, s2, d2)` (gece yarisini asan araliklar dahil). Saat gecerli degilse sayilar -1, metinler "--" doner; kosul fonksiyonlari false doner. Yeni ornek: `ROLEBOT_Internet_Clock_Relay_Example.ino`.
- Yeni proje ornegi: `ROLEBOT_NTP_Scheduled_Relay_Example.ino` (internet saatiyle haftalik role programi, elle gecici mudahale).
- Yeni kablosuz ornekler: `ROLEBOT_ESPNOW_Earthquake_Alert_Receiver_Example.ino`, `ROLEBOT_ESPNOW_Remote_Relay_Receiver_Example.ino` (IOTBOT gondericileriyle birlikte).

## [1.5.2] - 2026-09-28
### Fixed
- `getWeather()`/`getWikipedia()`: `JsonDocument` yerine v6/v7 ile de calisan `DynamicJsonDocument` kullaniliyor; `containsKey()` (ArduinoJson v7'de kaldirildi) yerine `!doc["extract"].isNull()` kullaniliyor. Bu, `USE_SERVER`+`USE_WEATHER` birlikte tanimlandiginda alinan "'JsonDocument'/'deserializeJson' was not declared" hatasinin kok nedeniydi. Bkz. CODLAI_IOTBOT 1.7.2'deki ayni degisiklik.
- `library.json`: eksik olan Firebase bagimliligi `dependencies` alanina eklendi.

## [1.5.1] - 2026-09-27
### Changed
- ESP-NOW alicisi (`startListening()`) artik eski (kutuphanenin onceki surumlerinde daha kucuk olan) `CodlaiESPNowMessage` boyutundaki paketleri de kabul ediyor - bkz. CODLAI_IOTBOT 1.7.1'deki ayni degisiklik.

## [1.5.0] - 2026-09-27
### Added
- **Basit ESP-NOW mesajlasma** (cocuklar/blok kod icin): `espNowBegin(channel=1)`, `espNowSendText(text)`, `espNowSendNumber(name, value)`, `espNowAvailable()`, `espNowReadText()`, `espNowReadName()`, `espNowReadNumber()`. `CodlaiESPNowMessage` yapisina `char text[32]` ve `float value` alanlari eklendi (Kol/Arac kontrolunu bozmadan) - ayni surumdeki tum CODLAI kartlari arasinda uyumlu.
- Yeni ornek: `ROLEBOT_ESPNOW_Simple_Messaging_Example.ino`.

### Not
- Melodi (`buzzerPlayNote`/`buzzerPlayMelody`/`buzzerSetTempo`) ve NeoPixel (`moduleSmartLEDFill` vb.) ozellikleri ROLEBOT'a EKLENMEDI - ROLEBOT'ta onboard buzzer ya da NeoPixel donanimi/API'si hic yok (sadece 2 role + 1 buton + 1 LED). Bu ozellikler sadece IOTBOT ve MINIBOT'ta mevcut.

## [1.4.1] - 2026-09-27
### Fixed
- `otaBegin()` icinde parola if/else zincirinden sonra fazladan bir `else { ArduinoOTA.setPassword("1234"); }` bloğu vardi - bu "else without a previous if" derleme hatasina yol acip `USE_OTA` tanimlayan HER sketch'in derlenmesini engelliyordu. Fazla blok kaldirildi. (Editor ajaninin derleme servisi testinde bulundu.)

## [1.4.0] - 2026-09-27
### Added
- Yeni "3-Projects" ornek klasoru: ROLEBOT'un sensorsuz, sadece 2 role + 1 buton + 1 LED donanimini kullanan, kablosuz gerektirmeyen basit proje ornekleri.
- `ROLEBOT_Double_Click_Two_Relay_Control_Example.ino` - tek butonla iki roleyi ayri ayri kontrol etme: tek tik Role 1'i, cift tik (400ms icinde) Role 2'yi degistirir.
- `ROLEBOT_Smart_Plug_Auto_Off_Timer_Example.ino` - butona basinca Role 1 acilir ve geri sayim baslar, sure dolunca kendiliginden kapanir (unutkanlik icin akilli priz).
- `ROLEBOT_EEPROM_Relay_Memory_Example.ino` - Role 1'in son durumu EEPROM'a yazilir; elektrik kesintisinden sonra kart yeniden basladiginda son durumu geri yukler.

- Yeni ornek: `ROLEBOT_ESPNOW_Fan_Control_Reactive_Example.ino` - bir IOTBOT'un yayinladigi DHT sicaklik verisine gore roleyi (vantilator) otomatik acar/kapatir ("kablosuz otomatik vantilator").

### Fixed
- `initESPNow()` icinde kosulsuz `WiFi.mode(WIFI_STA)` cagrisi, ayni sketch'te onceden acilmis bir AP'yi (ornegin bir web sunucusu/OTA icin `softAP()`) sessizce dusuruyordu. Artik mevcut mod AP ya da AP_STA ise `WIFI_AP_STA`'ya geciliyor, AP kapatilmiyor.

## [1.3.0] - 2026-09-26
### Added
- `serverOnRequest(url, callback)`: `serverCreateLocalPage` SADECE sabit/statik bir HTML sayfasi render eder; bu yeni fonksiyon bir adrese istek geldiginde GERCEKTEN kod calistirmaniza (bir rolyeyi tetiklemenize) izin verir.
- Yeni ornek: `ROLEBOT_WiFi_Web_Control_Example.ino` - telefon/tarayicidan iki role ve LED kontrolu (AP modu, `serverOnRequest` kullanir).
- Yeni baslangic seviyesi ornekler: `ROLEBOT_WiFi_Simple_Status_Example.ino` (MAC/sunucu gerekmeyen en basit WiFi baglanma ornegi), `ROLEBOT_ESPNOW_Broadcast_Simple_Example.ino` (MAC adresi bilmeden yayin/broadcast ile herhangi bir CODLAI kartina konusma) - egitim mufredati icin "once bunu dene" niteliginde.
- Yeni ornek: `ROLEBOT_ESPNOW_NightLight_Reactive_Example.ino` - bir IOTBOT'un yayinladigi isik sensoru verisine gore roleyi (gercek bir lambayi) otomatik acar/kapatir ("kablosuz gece lambasi").

### Fixed
- **ESP-NOW gonderme hatasi**: `initESPNow()` icinde `esp_now_set_self_role()` hic cagrilmiyordu; ESP8266'nin klasik `espnow.h` API'si bu olmadan `esp_now_send()`'i sessizce basarisiz kiliyordu ("Error sending the data" - MINIBOT'ta ayni hata gercek donanimda dogrulandi, ROLEBOT ayni kod deseninden mustesna degildi). `ESP_NOW_ROLE_COMBO` ile duzeltildi.
- `USE_ESPNOW` (tek basina, `USE_WIFI` olmadan) tanimlandiginda `WiFi.h`'in hic include edilmedigi bir sira sorunu duzeltildi - mevcut `ROLEBOT_ESP_NOW_Sender_Example.ino`/`Receiver_Example.ino` da bu hatadan etkileniyordu, artik derleniyor (bkz. CODLAI_IOTBOT v1.5.0'daki ayni duzeltme).

## [1.2.0] - 2026-09-25
### Added
- NTP time helpers: `ntpSync`, `ntpIsTimeValid`, `ntpGetEpoch`, `ntpGetDateTimeString`.
- CRC-protected EEPROM record helpers: `eepromCrc32`, `eepromWriteRecord`, `eepromReadRecord`.
- New advanced example: `ROLEBOT_NTP_Time_Advanced_Example.ino` (TR/EN).
- `examples/ROLEBOT_Musteri_Karsilama.cpp`'e Turkce/Ingilizce dil destegi eklendi: acilista B1 1.5sn basili tutulursa Ingilizce, birakilirsa (varsayilan) Turkce; secim LED yanip-sonmesiyle de teyit edilir.

### Fixed
- `library.json`'daki `dependencies` alani artik gercekte kullanilan kutuphaneleri gosteriyor (eski `ESPAsyncWebServer ^1.2.3` / `ESPAsyncTCP` / `AsyncTCP` uclusu yerine `mathieucarbou/ESPAsyncWebServer ^3.6.0` ve `bblanchon/ArduinoJson ^7.1.0`).

## [1.1.5] - 2026-02-04
### Added
- OTA helpers: `otaBegin`, `otaHandle` (requires `USE_OTA`).
- New advanced example: `ROLEBOT_OTA_Update_Example.ino` (TR/EN).

## [1.1.4] - 2025-12-20
### Added
- Extended EEPROM helpers: `eepromBegin/Commit/End`, byte/int32/uint32/float/string/bytes read-write and region clear.
- New advanced example: `ROLEBOT_EEPROM_Advanced_Example.ino` (TR/EN).

### Changed
- EEPROM int (legacy) helpers now lazy-initialize EEPROM to reduce common runtime issues.

## [1.1.3] - 2025-12-18
### Added
- Revived the ESP-NOW, email, Telegram, weather and Wikipedia advanced examples with bilingual guidance so their helper usage is aligned with the MINIBOT/IOTBOT libraries.

## [1.1.2] - 2025-03-09
### Fixed
- PlatformIO yeniden yayını için sürüm numarası artırıldı.

## [1.1.0] - 2025-03-09
### Added
- `triggerIFTTTEvent` helper for Maker Webhook automations.
- Advanced example: `ROLEBOT_IFTTT_Webhook_Example.ino`.

### Updated
- Documentation and metadata to highlight the new IFTTT workflow.

## [1.0.0] - 2025-03-04
### Added
- **Rebranding**: Transitioned from CODROB to CODLAI.
- Standardized library structure.
- Added `serialStart` and `serialWrite` wrappers.
- Updated examples to use library wrappers.
- Initial Release for PlatformIO and Arduino IDE.

---

# CODROB ERA (Legacy Models)

## [1.6.4] - 2025-02-28
### Added
- Tüm modüller için config dosyası kaldırıldı. Ortak kütüpahaneler devrede. 

## [1.6.3] - 2025-02-21
### Added
- Config dosyası eklendi. 

## [1.5.4] - 2025-02-19
### Added
- Arduino uyumluluğu için library.properties eklendi.
- esphome/ESPAsyncWebServer-esphome yerine mathieucarbou/ESPAsyncWebServer eklendi. 
- Gerekli uygulamalara define eklendi. uygulamaya gore kütüphane aktifleşecek hale getirildi. 

### Fixed
- CPP ve H dosyası arduıno ile uyumlu hale getirildi. 

## [1.5.1] - 2025-02-13
### Fixed
- Firebase ve Wifi örnek uygulamalrındaki eksiklikler düzeltildi. 

## [1.4.6] - 2025-02-11
### Fixed
- Firebase ve Wifi örnek uygulamalrındaki eksiklikler düzeltildi.  

## [1.3.0] - 2025-02-04
### Added
- Firebase kütüphaneleri eklendi. 
- Wifi Kütüphaneleri ve fonskiyonları ekleni 
- Local server fonksiyonaları eklendi. 
- EEPROM fonksiyonları ekledi.
- Örnek kütüphaneler eklendi. 

### Fixed
- Seriport fonksiyornları düzeltildi. 

## [1.2.0] - 2024-12-30
### Added
- ROLEBOT sınıfı ve temel sensör işlevleri eklendi.

---

## [1.0.0] - 2024-12-29
### Added
- İlk sürüm yayımlandı.

