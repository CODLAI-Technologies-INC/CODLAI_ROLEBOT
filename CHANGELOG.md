# Changelog

# CODLAI ERA (New Models)

## [Unreleased]

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

