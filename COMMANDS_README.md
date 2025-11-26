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
*   `void eepromWriteInt(...)`, `int eepromReadInt(...)`: EEPROM operations / işlemleri.
*   **WiFi**: `wifiStartAndConnect`, `wifiConnectionControl`, `wifiGetIPAddress`, `wifiGetMACAddress`.
*   **ESP-NOW**: `initESPNow`, `setWiFiChannel`, `sendESPNow`, `registerOnRecv`, `startListening`.
*   **Server / Sunucu**: `serverStart`, `serverCreateLocalPage`, `serverHandleDNS`, `serverContinue`.
*   **Cloud / Bulut**: `fbServerSetandStartWithUser` (Firebase), `sendTelegram`, `sendEmail`, `getWeather`, `getWikipedia`.
