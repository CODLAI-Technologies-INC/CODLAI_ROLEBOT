# CODLAI ROLEBOT Library Reference / Kütüphane Referansı

## Introduction / Giriş
**EN:** The ROLEBOT library is designed for ESP8266-based relay control and home automation. It simplifies controlling relays, connecting to WiFi, and interacting with cloud services.
**TR:** ROLEBOT kütüphanesi, ESP8266 tabanlı röle kontrolü ve ev otomasyonu için tasarlanmıştır. Röleleri kontrol etmeyi, WiFi'ye bağlanmayı ve bulut hizmetleriyle etkileşimi basitleştirir.

## Functions / Fonksiyonlar

### begin
**EN:** Initializes relay pins, buttons, and LED.
**TR:** Röle pinlerini, butonları ve LED'i başlatır.
**Syntax:** `void begin()`

### playIntro
**EN:** Plays a startup LED sequence.
**TR:** Başlangıç LED sekansını oynatır.
**Syntax:** `void playIntro()`

### Relay1Write
**EN:** Turns Relay 1 ON or OFF.
**TR:** 1. Röleyi AÇAR veya KAPATIR.
**Syntax:** `void Relay1Write(bool status)`

### Relay2Write
**EN:** Turns Relay 2 ON or OFF.
**TR:** 2. Röleyi AÇAR veya KAPATIR.
**Syntax:** `void Relay2Write(bool status)`

### ledWrite
**EN:** Controls the onboard LED.
**TR:** Dahili LED'i kontrol eder.
**Syntax:** `void ledWrite(bool status)`

### button1Read
**EN:** Reads the state of the onboard button.
**TR:** Dahili butonun durumunu okur.
**Syntax:** `bool button1Read()`

### wifiStartAndConnect
**EN:** Connects to a WiFi network.
**TR:** Bir WiFi ağına bağlanır.
**Syntax:** `void wifiStartAndConnect(const char *ssid, const char *pass)`

### serverStart
**EN:** Starts a local web server.
**TR:** Yerel bir web sunucusu başlatır.
**Syntax:** `void serverStart(const char *mode, const char *ssid, const char *password)`

### fbServerSet... / fbServerGet...
**EN:** Functions to read/write data to Firebase Realtime Database.
**TR:** Firebase Gerçek Zamanlı Veritabanına veri okuma/yazma fonksiyonları.
**Syntax:** `void fbServerSetInt(const char *path, int value)`, `int fbServerGetInt(const char *path)`

### sendTelegram
**EN:** Sends a message via Telegram Bot API.
**TR:** Telegram Bot API üzerinden mesaj gönderir.
**Syntax:** `void sendTelegram(String token, String chatId, String message)`

### sendEmail
**EN:** Sends an email via SMTP.
**TR:** SMTP üzerinden e-posta gönderir.
**Syntax:** `void sendEmail(...)`

### getWeather
**EN:** Gets weather information for a city.
**TR:** Bir şehir için hava durumu bilgisini alır.
**Syntax:** `String getWeather(String city, String apiKey)`

### getWikipedia
**EN:** Gets a summary from Wikipedia.
**TR:** Wikipedia'dan bir özet alır.
**Syntax:** `String getWikipedia(String query, String lang)`

### initESPNow
**EN:** Initializes ESP-NOW protocol.
**TR:** ESP-NOW protokolünü başlatır.
**Syntax:** `void initESPNow()`

### serialStart
**EN:** Starts serial communication at the specified baud rate.
**TR:** Seri haberleşmeyi belirtilen baud hızında başlatır.
**Syntax:** `void serialStart(int baudrate)`

### serialWrite
**EN:** Writes data to the serial port (Overloaded for String, int, float, bool).
**TR:** Seri porta veri yazar (String, int, float, bool için aşırı yüklenmiştir).
**Syntax:** `void serialWrite(data)`
