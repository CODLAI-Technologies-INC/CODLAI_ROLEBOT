# Changelog

# CODLAI ERA (New Models)

## [Unreleased]
### Added
- NTP time helpers: `ntpSync`, `ntpIsTimeValid`, `ntpGetEpoch`, `ntpGetDateTimeString`.
- CRC-protected EEPROM record helpers: `eepromCrc32`, `eepromWriteRecord`, `eepromReadRecord`.
- New advanced example: `ROLEBOT_NTP_Time_Advanced_Example.ino` (TR/EN).

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

