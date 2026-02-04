# CODLAI_ROLEBOT Library Documentation / Kütüphane Dokümantasyonu

**EN:** `ROLEBOT` is designed for relay control and home automation tasks using ESP8266.
**TR:** `ROLEBOT`, ESP8266 kullanarak röle kontrolü ve ev otomasyonu görevleri için tasarlanmıştır.

### Basic Control / Temel Kontrol
*   `void begin()`
    *   **EN:** Initializes relays and buttons.
    *   **TR:** Röleleri ve butonları başlatır.
*   `void playIntro()`
    *   **EN:** Performs startup with LED.
    *   **TR:** LED ile açılış yapar.
*   `bool button1Read()`
    *   **EN:** Reads the button.
    *   **TR:** Butonu okur.
*   `void ledWrite(bool status)`
    *   **EN:** Controls the onboard LED.
    *   **TR:** Kart üzerindeki LED'i kontrol eder.
*   `void Relay1Write(bool status)`
    *   **EN:** Controls Relay 1.
    *   **TR:** 1. Röleyi kontrol eder.
*   `void Relay2Write(bool status)`
    *   **EN:** Controls Relay 2.
    *   **TR:** 2. Röleyi kontrol eder.

### General and Communication / Genel ve İletişim
*   `void serialStart(int baudrate)`
    *   **EN:** Start serial port.
    *   **TR:** Seri port başlatma.
*   `void serialWrite(...)`
    *   **EN:** Write to serial port.
    *   **TR:** Seri port yazma.
*   **EEPROM / Kalıcı Hafıza**:
    *   `void eepromWriteInt(int address, int value)` / `int eepromReadInt(int address)`
        *   **EN:** Legacy 16-bit (2-byte) integer read/write.
        *   **TR:** Eski tip 16-bit (2 bayt) tam sayı okuma/yazma.
    *   `bool eepromBegin(size_t size = 512)` / `bool eepromCommit()` / `void eepromEnd()`
        *   **EN:** Initialize / commit / end EEPROM.
        *   **TR:** EEPROM başlat / commit / kapat.
    *   `bool eepromWriteByte(int address, uint8_t value)` / `uint8_t eepromReadByte(int address, uint8_t defaultValue = 0)`
        *   **EN:** Single byte read/write.
        *   **TR:** Tek bayt okuma/yazma.
    *   `bool eepromWriteInt32(int address, int32_t value)` / `int32_t eepromReadInt32(int address, int32_t defaultValue = 0)`
        *   **EN:** 32-bit integer read/write.
        *   **TR:** 32-bit tam sayı okuma/yazma.
    *   `bool eepromWriteFloat(int address, float value)` / `float eepromReadFloat(int address, float defaultValue = 0.0f)`
        *   **EN:** Float read/write.
        *   **TR:** Float okuma/yazma.
    *   `bool eepromWriteString(int address, const String &value, uint16_t maxLen = 128)` / `String eepromReadString(int address, uint16_t maxLen = 128)`
        *   **EN:** Stores as `[uint16 length][bytes...]`.
        *   **TR:** `[uint16 uzunluk][baytlar...]` formatında saklar.
    *   `bool eepromWriteBytes(int address, const uint8_t *data, size_t len)` / `bool eepromReadBytes(int address, uint8_t *data, size_t len)`
        *   **EN:** Raw bytes.
        *   **TR:** Ham bayt verisi.
    *   `bool eepromClear(int startAddress = 0, size_t length = 0, uint8_t fill = 0xFF)`
        *   **EN:** Fill a region (or whole EEPROM when length=0).
        *   **TR:** Bir bölgeyi (veya length=0 ise tüm EEPROM'u) doldurur.
    *   `uint32_t eepromCrc32(const uint8_t *data, size_t len, uint32_t seed = 0xFFFFFFFF)`
        *   **EN:** CRC32 for raw bytes.
        *   **TR:** Ham bayt verisi için CRC32.
    *   `bool eepromWriteRecord(int address, const uint8_t *data, uint16_t len, uint16_t version = 1)`
        *   **EN:** CRC-protected record write.
        *   **TR:** CRC korumalı record yazma.
    *   `bool eepromReadRecord(int address, uint8_t *out, uint16_t maxLen, uint16_t *outLen = nullptr, uint16_t *outVersion = nullptr)`
        *   **EN:** CRC-protected record read (validates magic/len/crc).
        *   **TR:** CRC korumalı record okuma (magic/len/crc kontrolü).
*   **WiFi**: `wifiStartAndConnect`, `wifiConnectionControl`, `wifiGetIPAddress`, `wifiGetMACAddress`.
*   **OTA (Over-The-Air)**: `otaBegin`, `otaHandle` (call after WiFi, keep `otaHandle()` in `loop()`).
*   **NTP Time / Saat Senkron**: `ntpBegin` (recommended), `ntpSync` (advanced), `ntpIsTimeValid`, `ntpGetEpoch`, `ntpGetDateTimeString`.
*   **ESP-NOW**: `initESPNow`, `setWiFiChannel`, `sendESPNow`, `registerOnRecv`, `startListening`.
*   **Server / Sunucu**: `serverStart`, `serverCreateLocalPage`, `serverHandleDNS`, `serverContinue`.
*   **Cloud / Bulut**: `fbServerSetandStartWithUser` (Firebase), `sendTelegram`, `sendEmail`, `getWeather`, `getWikipedia`.
    *   `bool triggerIFTTTEvent(const String &eventName, const String &webhookKey, const String &jsonPayload = "{}")`
        *   **EN:** Triggers an IFTTT Webhook event with optional JSON payload data and returns `true` when the request succeeds (HTTP 200).
        *   **TR:** Opsiyonel JSON verisiyle IFTTT Webhook olayını tetikler ve istek başarılı olduğunda (HTTP 200) `true` döndürür.
