/*
 * CODLAI ROLEBOT Library
 * 
 * Structure Information:
 * This library uses a modular structure to optimize memory usage and compilation time.
 * Features are enabled/disabled via definitions in the main sketch (e.g., #define USE_SERVER).
 * 
 * IMPORTANT: Define feature flags BEFORE including this library in your sketch.
 */

#ifndef ROLEBOT_H
#define ROLEBOT_H

#include <Arduino.h>

#if defined(ESP8266)
#include <EEPROM.h>
#include <time.h>

#if defined(USE_SERVER)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <functional> // serverOnRequest icin std::function
#endif

#if defined(USE_FIREBASE)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
#endif

#if defined(USE_WEATHER) || defined(USE_WIKIPEDIA) || defined(USE_TELEGRAM) || defined(USE_IFTTT)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#endif

#if defined(USE_OTA)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#include <ArduinoOTA.h>
#endif

#if defined(USE_ESPNOW)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#include <espnow.h>
extern "C" {
#include <user_interface.h> // wifi_set_channel() burada tanimli
}
#endif

#if defined(USE_EMAIL)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#include <ESP_Mail_Client.h>
#endif

#if defined(USE_WEATHER) || defined(USE_WIKIPEDIA) || defined(USE_TELEGRAM) || defined(USE_IFTTT)
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#endif

// NOT: <ESP8266WiFi.h> include'u kasitli olarak EN SONA alindi - USE_OTA/
// USE_ESPNOW/USE_EMAIL bayraklarinin HERHANGI biri USE_WIFI'yi KENDI
// blogu icinde otomatik tanimliyor; bu satir onlardan ONCE olursa (ornegin
// sadece USE_ESPNOW tanimliyken) USE_WIFI henuz tanimlanmamis olur ve
// WiFi.h hic include edilmez. Bkz. IOTBOT.h/MINIBOT.h'deki ayni duzeltme.
#if defined(USE_WIFI)
#include <ESP8266WiFi.h>
#endif

// Structure to receive data via ESP-NOW
#ifndef CODLAI_ESPNOW_MESSAGE_DEFINED
#define CODLAI_ESPNOW_MESSAGE_DEFINED
typedef struct {
  uint8_t deviceType; // 1=Armbot komutu, 2=Carbot komutu, 3=Carbot telemetrisi (axis3=mesafe cm, -1=bilinmiyor), 4=Armbot sinyali, 10=IOTBOT LDR yayini, 11=IOTBOT sicaklik yayini, 20=basit metin mesaji, 21=basit sayi mesaji, 22-29=REZERVE: editor.codlai.com ozel/eslesmeli mesajlasma bloklari (22 ozel metin, 23 ozel sayi, 24 eslesme teklifi, 25 eslesme kabulu; axis1=grup, axis2/axis3=hedef MAC, her zaman yayinla gonderilir, suzgec alicida), 30-39=REZERVE: CODLAI Robotlari Otonom projesi (30 eslesme teklifi, 31 eslesme kabul, 32 mod, 33 durum), 40-49=kutuphane ornek karti kimlikleri / library example board IDs (40 IOTBOT, 41 MINIBOT, 42 ROLEBOT; Broadcast_Simple / Pair / SmartLED_Remote ornekleri / examples)
  int axis1;
  int axis2;
  int axis3;
  int gripper;
  uint8_t action; // 0=None, 1=Horn, 2=Note
  char text[32];  // espNowSendText: metin icerigi / espNowSendNumber: sayinin adi (name)
  float value;    // espNowSendNumber: sayinin degeri (value)
} CodlaiESPNowMessage;
#endif

// Pins
#define RELAY_1 12
#define RELAY_2 13

#define B1_BUTTON_PIN 0
#define BLUE_LED 16

class ROLEBOT
{
public:
  ROLEBOT();
  void begin();
  void playIntro();

  /*********************************** Serial Port ***********************************
   */
  void serialStart(int baundrate);
  void serialWrite(const char *message);
  void serialWrite(String message);
  void serialWrite(long value);
  void serialWrite(int value);
  void serialWrite(float value);
  void serialWrite(bool value);

  /*********************************** BUTTONS ***********************************/
  // DIKKAT: Ham pin seviyesini dondurur (dahili pull-up) -> true = BIRAKILMIS,
  // false = BASILI. Basili mi diye bakmak icin: !rolebot.button1Read()
  // (Geriye uyumluluk icin boyle birakildi; ornekler degeri tersine cevirir.)
  // / NOTE: returns the raw pin level (internal pull-up) -> true = RELEASED,
  // false = PRESSED. To check "is pressed": !rolebot.button1Read()
  // (Kept this way for backward compatibility; the examples invert the value.)
  bool button1Read();

  /*********************************** LED ***********************************/
  void ledWrite(bool status);

  /*********************************** Relays ***********************************
   */
  void Relay1Write(bool status);
  void Relay2Write(bool status);

  /*********************************** EEPROM  ***********************************
   */
  bool eepromBegin(size_t size = 512);
  bool eepromCommit();
  void eepromEnd();

  bool eepromWriteByte(int address, uint8_t value);
  uint8_t eepromReadByte(int address, uint8_t defaultValue = 0);

  // NOTE: eepromWriteInt/eepromReadInt store 16-bit (2 bytes) for backward compatibility.
  // Aralik / range: -32768 ... 32767 (negatif sayilar da dogru okunur / negatives read back correctly)
  void eepromWriteInt(int address, int value);
  int eepromReadInt(int address);

  bool eepromWriteInt32(int address, int32_t value);
  int32_t eepromReadInt32(int address, int32_t defaultValue = 0);
  bool eepromWriteUInt32(int address, uint32_t value);
  uint32_t eepromReadUInt32(int address, uint32_t defaultValue = 0);
  bool eepromWriteFloat(int address, float value);
  float eepromReadFloat(int address, float defaultValue = 0.0f);

  // Stores: [uint16 length][bytes...]
  // Hic yazilmamis (silinmis, 0xFF) alan "" dondurur / never-written (erased, 0xFF) area returns ""
  bool eepromWriteString(int address, const String &value, uint16_t maxLen = 128);
  String eepromReadString(int address, uint16_t maxLen = 128);

  bool eepromWriteBytes(int address, const uint8_t *data, size_t len);
  bool eepromReadBytes(int address, uint8_t *data, size_t len);
  bool eepromClear(int startAddress = 0, size_t length = 0, uint8_t fill = 0xFF);

  // Record helpers (recommended): Stores a blob with header + CRC32
  // Layout: [uint16 magic][uint16 version][uint16 len][uint32 crc32][bytes...]
  uint32_t eepromCrc32(const uint8_t *data, size_t len, uint32_t seed = 0xFFFFFFFF);
  bool eepromWriteRecord(int address, const uint8_t *data, uint16_t len, uint16_t version = 1);
  bool eepromReadRecord(int address, uint8_t *out, uint16_t maxLen, uint16_t *outLen = nullptr, uint16_t *outVersion = nullptr);

  /*********************************** WiFi  ***********************************
   */
#if defined(USE_WIFI)
  void wifiStartAndConnect(const char *ssid, const char *pass); // Sifre seri porta yazilmaz (****) / password is not printed (****)
  bool wifiConnectionControl(); // Durumu dondurur; seri porta SADECE durum degisince yazar / returns the state; prints ONLY when it changes
  String wifiGetMACAddress();
  String wifiGetIPAddress();
#endif

  // Metni adres (URL) icinde guvenle kullanilacak hale getirir (UTF-8 yuzde
  // kodlama): harf/rakam ve - _ . ~ ayni kalir, diger her bayt (bosluk, &, ?,
  // Turkce harfler...) %XX olur. sendTelegram/getWeather/getWikipedia bunu
  // KENDILERI yapar - onlara ham metin verin.
  // / Makes text safe inside a web address (URL) (UTF-8 percent-encoding):
  // letters/digits and - _ . ~ stay, every other byte (space, &, ?, Turkish
  // letters...) becomes %XX. sendTelegram/getWeather/getWikipedia do this
  // THEMSELVES - pass them plain text.
  String urlEncode(const String &text);

  /*********************************** OTA (Over-The-Air) ***********************************
   * TR: WiFi baglantisindan SONRA cagirilmalidir. Varsayilan port 8266 (ESP8266
   * standardi; Arduino IDE / espota bu portu bekler).
   * EN: Must be called AFTER WiFi connection is established. Default port is 8266
   * (the ESP8266 standard; Arduino IDE / espota expect this port).
   */
#if defined(USE_OTA)
  void otaBegin(const char *hostname = "CODLAI-ROLEBOT", const char *password = "1234", uint16_t port = 8266);
  void otaHandle();
#endif

  /*********************************** NTP Time ***********************************
   * TR: WiFi baglantisindan SONRA cagirilmalidir.
   * EN: Must be called AFTER WiFi connection is established.
   */
  // TR/EN: Recommended simple setup for blocks.
  // timezoneHours: UTC offset in hours (e.g., Turkey is +3)
  bool ntpBegin(int timezoneHours = 0, const char *ntpServer = "pool.ntp.org", int daylightOffsetHours = 0, uint32_t timeoutMs = 10000);
  bool ntpSync(const char *ntpServer = "pool.ntp.org", long gmtOffsetSec = 0, int daylightOffsetSec = 0, uint32_t timeoutMs = 10000);
  bool ntpIsTimeValid(time_t minEpoch = 1609459200);
  time_t ntpGetEpoch();
  String ntpGetDateTimeString();
  // --- Blok dostu internet saati / Block-friendly internet time ---
  // TR: ntpBegin(3) ile baslat (Turkiye UTC+3; once WiFi'ye baglanin). Saat
  // gecerli degilse okuma fonksiyonlari -1 (metinler "--") dondurur.
  // EN: start with ntpBegin(3) (Turkey UTC+3; connect to WiFi first). While the
  // time is not valid, the getters return -1 (strings return "--").
  bool ntpUpdate();                  // Saati SIMDI yeniden cek (son ayarlarla) / re-sync NOW (last settings)
  int ntpGetHour();                  // 0-23
  int ntpGetMinute();                // 0-59
  int ntpGetSecond();                // 0-59
  int ntpGetDay();                   // 1-31
  int ntpGetMonth();                 // 1-12
  int ntpGetYear();                  // ornek / e.g. 2026
  int ntpGetWeekday();               // 1=Pazartesi/Monday ... 7=Pazar/Sunday
  String ntpGetTimeString();         // "14:05:09"
  String ntpGetDateString();         // "29.09.2026"
  bool ntpTimeIs(int hour, int minute);      // O dakika boyunca true / true during that whole minute
  bool ntpTimeReached(int hour, int minute); // O dakikaya girince SADECE BIR KEZ true / true only ONCE when that minute starts
  bool ntpTimeIsBetween(int startHour, int startMinute, int endHour, int endMinute); // [baslangic, bitis) gece yarisini asabilir / [start, end) may cross midnight

  /*********************************** Server  ***********************************
   */
#if defined(USE_SERVER)
  // mode "STA": ssid/password ile aga baglanir; 30 sn'de baglanamazsa kendi agini
  // kurar: ad "CODLAI-ROLEBOT", sifre = verilen sifre (8 karakterden kisaysa
  // "12345678"). mode "AP": ssid/password ile kendi agini kurar (bos ad ->
  // "CODLAI-ROLEBOT"; 1-7 karakterlik sifre -> "12345678", bos sifre = sifresiz ag).
  // / mode "STA": joins ssid/password; if it can't within 30 s it starts its own
  // network: name "CODLAI-ROLEBOT", password = the given one ("12345678" if it is
  // shorter than 8). mode "AP": starts its own network ssid/password (empty name ->
  // "CODLAI-ROLEBOT"; 1-7 char password -> "12345678", empty password = open network).
  void serverStart(const char *mode, const char *ssid, const char *password);
  // url: "demopage" ya da "/demopage" (ikisi ayni); "/" veya "" ana sayfa olur ve
  // serverStart'in varsayilan ana sayfasinin yerine gecer.
  // / url: "demopage" or "/demopage" (same thing); "/" or "" becomes the home page
  // and replaces serverStart's default home page.
  void serverCreateLocalPage(const char *url, const char *WEBPageScript, const char *WEBPageCSS, const char *WEBPageHTML, size_t bufferSize = 4096);
  // serverCreateLocalPage SADECE sabit/statik bir HTML sayfasi render eder;
  // butona basildiginda gercekten bir rolyeyi tetiklemek icin bu fonksiyon
  // kullanilir - bkz. IOTBOT.h'deki ayni fonksiyon.
  void serverOnRequest(const char *url, std::function<String()> callback);
  void serverHandleDNS();
  void serverContinue();
#endif

  /*********************************** ESP-NOW ***********************************
   */
#if defined(USE_ESPNOW)
  void initESPNow();
  void setWiFiChannel(int channel);
  void sendESPNow(uint8_t *macAddr, uint8_t *data, int len);
  void registerOnRecv(esp_now_recv_cb_t cb);

  // ESP-NOW Data Handling
  CodlaiESPNowMessage receivedData;
  volatile bool newData = false;
  static ROLEBOT* _instance;

  void startListening() {
      _instance = this;
      registerOnRecv([](uint8_t *mac, uint8_t *incomingData, uint8_t len) {
          // Eski (daha kucuk) CodlaiESPNowMessage boyutundaki paketleri de
          // kabul ediyoruz - bkz. IOTBOT.h/MINIBOT.h'deki ayni degisiklik.
          // Also accept packets sized for an older (smaller)
          // CodlaiESPNowMessage - see the same change in IOTBOT.h/MINIBOT.h.
          if (_instance && len > 0) {
              memset(&_instance->receivedData, 0, sizeof(_instance->receivedData));
              size_t copyLen = (size_t)len < sizeof(_instance->receivedData) ? (size_t)len : sizeof(_instance->receivedData);
              memcpy(&_instance->receivedData, incomingData, copyLen);
              _instance->newData = true;
          }
      });
  }

  // --- Basit ESP-NOW mesajlasma (cocuklar/blok kod icin) ---
  // Simple ESP-NOW messaging (for children / block-based code)
  bool espNowBegin(int channel = 1);
  void espNowSendText(const String &text);
  void espNowSendNumber(const String &name, float value);
  bool espNowAvailable();
  String espNowReadText();
  String espNowReadName();
  float espNowReadNumber();
#endif

  /*********************************** Email ***********************************
   */
#if defined(USE_EMAIL)
  void sendEmail(String smtpHost, int smtpPort, String authorEmail, String authorPassword, String recipientEmail, String subject, String message);
#endif

  /*********************************** Weather ***********************************
   */
#if defined(USE_WEATHER)
  String getWeather(String city, String apiKey);
#endif

  /*********************************** Wikipedia ***********************************
   */
#if defined(USE_WIKIPEDIA)
  String getWikipedia(String query, String lang = "en");
#endif

  /*********************************** Telegram ***********************************
   */
#if defined(USE_TELEGRAM)
  void sendTelegram(String token, String chatId, String message);
#endif

  /*********************************** IFTTT ***********************************
   */
#if defined(USE_IFTTT)
  bool triggerIFTTTEvent(const String &eventName, const String &webhookKey, const String &jsonPayload = "{}");
#endif

  /*********************************** Firebase Server  ***********************************
   */
#if defined(USE_FIREBASE)
  // 📡 Firebase Server Functions
  void fbServerSetandStartWithUser(const char *projectURL, const char *secretKey, const char *userMail, const char *mailPass); // projectURL: YOUR_FIREBASE_PROJECT_ID.firebaseio.com / secretKey: YOUR_FIREBASE_DATABASE_SECRET

  // 🔄 Firebase Database Write Functions
  void fbServerSetInt(const char *dataPath, int data);
  void fbServerSetFloat(const char *dataPath, float data);
  void fbServerSetString(const char *dataPath, String data);
  void fbServerSetDouble(const char *dataPath, double data);
  void fbServerSetBool(const char *dataPath, bool data);
  void fbServerSetJSON(const char *dataPath, String data);

  // 📥 Firebase Database Read Functions
  int fbServerGetInt(const char *dataPath);
  float fbServerGetFloat(const char *dataPath);
  String fbServerGetString(const char *dataPath);
  double fbServerGetDouble(const char *dataPath);
  bool fbServerGetBool(const char *dataPath);
  String fbServerGetJSON(const char *dataPath);
#endif

private:
  static constexpr uint16_t _EEPROM_RECORD_MAGIC = 0xCD1A;
  static constexpr time_t _NTP_VALID_EPOCH = 1609459200; // 2021-01-01
  // ntpUpdate() icin son NTP ayarlari / last NTP settings for ntpUpdate()
  String _ntpServer = "pool.ntp.org";
  long _ntpGmtOffsetSec = 0;
  int _ntpDaylightOffsetSec = 0;
  // ntpTimeReached() icin: her saat:dakika icin en son tetiklendigi dakika damgasi
  // / for ntpTimeReached(): last minute stamp each hour:minute fired at
  struct _NtpReachedSlot { int16_t key; int32_t stamp; };
  _NtpReachedSlot _ntpReached[8] = {};
  uint8_t _ntpReachedCount = 0;
  bool _ntpLocalTime(struct tm &out);

  bool _eepromReady = false;
  size_t _eepromSize = 0;

  bool _eepromEnsure(size_t minSize);

#if defined(USE_SERVER)
  const IPAddress apIP = IPAddress(192, 168, 4, 1); // Sabit IP adresi tanımlanıyor / Define static IP address
  DNSServer dnsServer;                              // DNS sunucusu tanımlanıyor / Define DNS Server
  AsyncWebServer serverCODLAI{80};                  // Web server objesi
  AsyncWebSocket *serverCODLAIWebSocket;            // Pointer olarak tanımla
  AsyncCallbackWebHandler *_serverRootHandler = nullptr; // serverStart'in varsayilan "/" sayfasi / serverStart's default "/" page
  bool _serverStarted = false;    // handler'lar + begin() sadece BIR kez / handlers + begin() only ONCE
  bool _serverUserRoot = false;   // kullanici "/" kaydetti mi / did the user register "/"
  bool _serverDnsStarted = false; // AP DNS yonlendirmesi calisiyor mu / is AP DNS redirection running
  String _serverPath(const char *url);
  void _serverClaimRoot(const String &path);
  void _serverStartAP(const char *ssid, const char *password);
#endif

#if defined(USE_WIFI)
  int8_t _wifiLastState = -1; // wifiConnectionControl: son yazilan durum (-1 = hic) / last printed state (-1 = never)
#endif

#if defined(USE_FIREBASE)
  FirebaseData firebaseData;     // Data object to handle Firebase communication
  FirebaseAuth firebaseAuth;     // Authentication credentials for user verification
  FirebaseConfig firebaseConfig; // Configuration settings for Firebase
  char uid[128] = "";            // User ID storage
#endif
};

#if defined(USE_ESPNOW)
// Initialize static member
inline ROLEBOT* ROLEBOT::_instance = nullptr;
#endif

/*********************************** IMPLEMENTATION ***********************************/

inline ROLEBOT::ROLEBOT()
{
#if defined(USE_SERVER)
  serverCODLAIWebSocket = new AsyncWebSocket("/serverCODLAIWebSocket");
#endif
}

inline void ROLEBOT::begin()
{
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(B1_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BLUE_LED, OUTPUT);
}

inline void ROLEBOT::playIntro()
{
  digitalWrite(BLUE_LED, HIGH);
  delay(100);
  digitalWrite(BLUE_LED, LOW);
  delay(100);
  digitalWrite(BLUE_LED, HIGH);
  delay(100);
  digitalWrite(BLUE_LED, LOW);
  delay(100);
  digitalWrite(BLUE_LED, HIGH);
  delay(100);
  digitalWrite(BLUE_LED, LOW);
  delay(100);
}

/*********************************** Serial Port ***********************************
 */

inline void ROLEBOT::serialStart(int baudrate)
{
  Serial.begin(baudrate);
}

// Overloaded function for const char* / `const char*` için fonksiyon
inline void ROLEBOT::serialWrite(const char *message)
{
  Serial.println(message);
}

// Overloaded function for String / `String` için özel fonksiyon
inline void ROLEBOT::serialWrite(String message)
{
  Serial.println(message.c_str()); // Convert String to const char*
}

// Overloaded function for long / `long` için özel fonksiyon
inline void ROLEBOT::serialWrite(long value)
{
  Serial.println(String(value).c_str());
}

// Overloaded function for int / `int` için fonksiyon
inline void ROLEBOT::serialWrite(int value)
{
  Serial.println(String(value).c_str());
}

// Overloaded function for float / `float` için fonksiyon
inline void ROLEBOT::serialWrite(float value)
{
  Serial.println(String(value).c_str());
}

// Overloaded function for bool / `bool` için fonksiyon
inline void ROLEBOT::serialWrite(bool value)
{
  Serial.println(value ? "true" : "false");
}

/*********************************** BUTTONS ***********************************
 */
inline bool ROLEBOT::button1Read()
{
  return digitalRead(B1_BUTTON_PIN);
}

/*********************************** LED ***********************************
 */
inline void ROLEBOT::ledWrite(bool status)
{
  return digitalWrite(BLUE_LED, status);
}

/*********************************** Relays  ***********************************
 */
inline void ROLEBOT::Relay1Write(bool status)
{
  digitalWrite(RELAY_1, status);
}

inline void ROLEBOT::Relay2Write(bool status)
{
  digitalWrite(RELAY_2, status);
}

/*********************************** EEPROM  ***********************************
 */
inline bool ROLEBOT::_eepromEnsure(size_t minSize)
{
  if (_eepromReady && _eepromSize >= minSize)
  {
    return true;
  }

  size_t targetSize = _eepromSize;
  if (targetSize == 0)
  {
    targetSize = 512;
  }
  if (targetSize < minSize)
  {
    targetSize = minSize;
  }

  return eepromBegin(targetSize);
}

inline bool ROLEBOT::eepromBegin(size_t size)
{
  if (size == 0)
  {
    size = 512;
  }

  if (_eepromReady && _eepromSize == size)
  {
    return true;
  }

  _eepromSize = size;
#if defined(ESP8266)
  EEPROM.begin(size);
  _eepromReady = true;
#else
  _eepromReady = EEPROM.begin(size);
#endif
  return _eepromReady;
}

inline bool ROLEBOT::eepromCommit()
{
  if (!_eepromEnsure(2))
  {
    return false;
  }

  return EEPROM.commit();
}

inline void ROLEBOT::eepromEnd()
{
  if (_eepromReady)
  {
    EEPROM.end();
  }
  _eepromReady = false;
  _eepromSize = 0;
}

inline bool ROLEBOT::eepromWriteByte(int address, uint8_t value)
{
  if (address < 0)
  {
    return false;
  }

  if (!_eepromEnsure((size_t)address + 1))
  {
    return false;
  }

  EEPROM.write(address, value);
  return EEPROM.commit();
}

inline uint8_t ROLEBOT::eepromReadByte(int address, uint8_t defaultValue)
{
  if (address < 0)
  {
    return defaultValue;
  }

  if (!_eepromEnsure((size_t)address + 1))
  {
    return defaultValue;
  }

  return EEPROM.read(address);
}

inline void ROLEBOT::eepromWriteInt(int address, int value) // EEPROM'a güvenli bir şekilde int türünde veri yazmak için fonksiyon
{
  if (address < 0)
  {
    return;
  }

  if (!_eepromEnsure((size_t)address + 2))
  {
    return;
  }

  uint8_t hi = highByte(value); // int'in yüksek baytını al
  uint8_t lo = lowByte(value);  // int'in düşük baytını al

  EEPROM.write(address, hi);      // İlk baytı EEPROM'a yaz
  EEPROM.write(address + 1, lo);  // İkinci baytı EEPROM'a yaz
  EEPROM.commit();                    // Değişiklikleri kaydetmek için commit işlemi yapılmalıdır
}

inline int ROLEBOT::eepromReadInt(int address) // EEPROM'dan int türünde veri okumak için fonksiyon
{
  if (address < 0)
  {
    return 0;
  }

  if (!_eepromEnsure((size_t)address + 2))
  {
    return 0;
  }

  uint8_t hi = EEPROM.read(address);     // İlk baytı oku
  uint8_t lo = EEPROM.read(address + 1); // İkinci baytı oku
  // word() isaretsizdir: -5 yazilip 65531 okunuyordu. int16_t'ye cevirmek isareti
  // geri getirir. / word() is unsigned: writing -5 read back 65531. Casting to
  // int16_t restores the sign.
  return (int16_t)word(hi, lo);
}

inline bool ROLEBOT::eepromWriteInt32(int address, int32_t value)
{
  if (address < 0)
  {
    return false;
  }
  if (!_eepromEnsure((size_t)address + sizeof(int32_t)))
  {
    return false;
  }

  EEPROM.put(address, value);
  return EEPROM.commit();
}

inline int32_t ROLEBOT::eepromReadInt32(int address, int32_t defaultValue)
{
  if (address < 0)
  {
    return defaultValue;
  }
  if (!_eepromEnsure((size_t)address + sizeof(int32_t)))
  {
    return defaultValue;
  }

  int32_t value;
  EEPROM.get(address, value);
  return value;
}

inline bool ROLEBOT::eepromWriteUInt32(int address, uint32_t value)
{
  if (address < 0)
  {
    return false;
  }
  if (!_eepromEnsure((size_t)address + sizeof(uint32_t)))
  {
    return false;
  }

  EEPROM.put(address, value);
  return EEPROM.commit();
}

inline uint32_t ROLEBOT::eepromReadUInt32(int address, uint32_t defaultValue)
{
  if (address < 0)
  {
    return defaultValue;
  }
  if (!_eepromEnsure((size_t)address + sizeof(uint32_t)))
  {
    return defaultValue;
  }

  uint32_t value;
  EEPROM.get(address, value);
  return value;
}

inline bool ROLEBOT::eepromWriteFloat(int address, float value)
{
  if (address < 0)
  {
    return false;
  }
  if (!_eepromEnsure((size_t)address + sizeof(float)))
  {
    return false;
  }

  EEPROM.put(address, value);
  return EEPROM.commit();
}

inline float ROLEBOT::eepromReadFloat(int address, float defaultValue)
{
  if (address < 0)
  {
    return defaultValue;
  }
  if (!_eepromEnsure((size_t)address + sizeof(float)))
  {
    return defaultValue;
  }

  float value;
  EEPROM.get(address, value);
  return value;
}

inline bool ROLEBOT::eepromWriteBytes(int address, const uint8_t *data, size_t len)
{
  if (address < 0 || data == nullptr)
  {
    return false;
  }

  if (len == 0)
  {
    return true;
  }

  if (!_eepromEnsure((size_t)address + len))
  {
    return false;
  }

  for (size_t i = 0; i < len; i++)
  {
    EEPROM.write(address + (int)i, data[i]);
  }

  return EEPROM.commit();
}

inline bool ROLEBOT::eepromReadBytes(int address, uint8_t *data, size_t len)
{
  if (address < 0 || data == nullptr)
  {
    return false;
  }

  if (len == 0)
  {
    return true;
  }

  if (!_eepromEnsure((size_t)address + len))
  {
    return false;
  }

  for (size_t i = 0; i < len; i++)
  {
    data[i] = EEPROM.read(address + (int)i);
  }

  return true;
}

inline bool ROLEBOT::eepromWriteString(int address, const String &value, uint16_t maxLen)
{
  if (address < 0)
  {
    return false;
  }

  if (maxLen == 0)
  {
    return false;
  }

  uint16_t len = (uint16_t)value.length();
  if (len > maxLen)
  {
    len = maxLen;
  }

  size_t total = sizeof(uint16_t) + (size_t)len;
  if (!_eepromEnsure((size_t)address + total))
  {
    return false;
  }

  EEPROM.put(address, len);
  for (uint16_t i = 0; i < len; i++)
  {
    EEPROM.write(address + (int)sizeof(uint16_t) + (int)i, (uint8_t)value[i]);
  }

  return EEPROM.commit();
}

inline String ROLEBOT::eepromReadString(int address, uint16_t maxLen)
{
  if (address < 0 || maxLen == 0)
  {
    return String("");
  }

  if (!_eepromEnsure((size_t)address + sizeof(uint16_t)))
  {
    return String("");
  }

  uint16_t len = 0;
  EEPROM.get(address, len);

  // Hic yazilmamis (silinmis) flash 0xFF okunur -> uzunluk 0xFFFF. Eskiden bu,
  // maxLen kadar 0xFF karakterli anlamsiz bir metin donduruyordu.
  // / Never-written (erased) flash reads 0xFF -> length 0xFFFF. This used to
  // return maxLen bytes of 0xFF garbage.
  if (len == 0xFFFF)
  {
    return String("");
  }

  if (len > maxLen)
  {
    len = maxLen;
  }

  if (!_eepromEnsure((size_t)address + sizeof(uint16_t) + (size_t)len))
  {
    return String("");
  }

  String out;
  out.reserve(len);
  for (uint16_t i = 0; i < len; i++)
  {
    out += (char)EEPROM.read(address + (int)sizeof(uint16_t) + (int)i);
  }
  return out;
}

inline bool ROLEBOT::eepromClear(int startAddress, size_t length, uint8_t fill)
{
  if (startAddress < 0)
  {
    return false;
  }

  if (length == 0)
  {
    length = (_eepromSize == 0) ? 512 : _eepromSize;
  }

  if (!_eepromEnsure((size_t)startAddress + length))
  {
    return false;
  }

  for (size_t i = 0; i < length; i++)
  {
    EEPROM.write(startAddress + (int)i, fill);
  }

  return EEPROM.commit();
}

inline uint32_t ROLEBOT::eepromCrc32(const uint8_t *data, size_t len, uint32_t seed)
{
  if (data == nullptr)
  {
    return 0;
  }

  uint32_t crc = seed;
  for (size_t i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; b++)
    {
      if (crc & 1)
      {
        crc = (crc >> 1) ^ 0xEDB88320UL;
      }
      else
      {
        crc >>= 1;
      }
    }
  }
  return crc ^ 0xFFFFFFFFUL;
}

inline bool ROLEBOT::eepromWriteRecord(int address, const uint8_t *data, uint16_t len, uint16_t version)
{
  if (address < 0 || data == nullptr)
  {
    return false;
  }

  const size_t headerSize = 2 + 2 + 2 + 4;
  if (!_eepromEnsure((size_t)address + headerSize + (size_t)len))
  {
    return false;
  }

  const uint16_t magic = _EEPROM_RECORD_MAGIC;
  const uint32_t crc = eepromCrc32(data, len);

  EEPROM.put(address, magic);
  EEPROM.put(address + 2, version);
  EEPROM.put(address + 4, len);
  EEPROM.put(address + 6, crc);

  for (uint16_t i = 0; i < len; i++)
  {
    EEPROM.write(address + (int)headerSize + (int)i, data[i]);
  }

  return EEPROM.commit();
}

inline bool ROLEBOT::eepromReadRecord(int address, uint8_t *out, uint16_t maxLen, uint16_t *outLen, uint16_t *outVersion)
{
  if (address < 0 || out == nullptr || maxLen == 0)
  {
    return false;
  }

  const size_t headerSize = 2 + 2 + 2 + 4;
  if (!_eepromEnsure((size_t)address + headerSize))
  {
    return false;
  }

  uint16_t magic = 0;
  uint16_t version = 0;
  uint16_t len = 0;
  uint32_t storedCrc = 0;

  EEPROM.get(address, magic);
  if (magic != _EEPROM_RECORD_MAGIC)
  {
    return false;
  }

  EEPROM.get(address + 2, version);
  EEPROM.get(address + 4, len);
  EEPROM.get(address + 6, storedCrc);

  if (len > maxLen)
  {
    return false;
  }

  if (!_eepromEnsure((size_t)address + headerSize + (size_t)len))
  {
    return false;
  }

  for (uint16_t i = 0; i < len; i++)
  {
    out[i] = EEPROM.read(address + (int)headerSize + (int)i);
  }

  const uint32_t calcCrc = eepromCrc32(out, len);
  if (calcCrc != storedCrc)
  {
    return false;
  }

  if (outLen)
  {
    *outLen = len;
  }
  if (outVersion)
  {
    *outVersion = version;
  }
  return true;
}

inline bool ROLEBOT::ntpSync(const char *ntpServer, long gmtOffsetSec, int daylightOffsetSec, uint32_t timeoutMs)
{
  if (ntpServer == nullptr || ntpServer[0] == '\0')
  {
    ntpServer = "pool.ntp.org";
  }

  // ntpUpdate() ayni ayarlarla tekrar cagirabilsin diye sakla.
  // / Remember the settings so ntpUpdate() can call again with them.
  _ntpServer = ntpServer;
  _ntpGmtOffsetSec = gmtOffsetSec;
  _ntpDaylightOffsetSec = daylightOffsetSec;

  configTime(gmtOffsetSec, daylightOffsetSec, _ntpServer.c_str());

  const uint32_t startMs = millis();
  while ((millis() - startMs) < timeoutMs)
  {
    time_t now = time(nullptr);
    if (now >= _NTP_VALID_EPOCH)
    {
      return true;
    }
    delay(50);
  }

  return false;
}

inline bool ROLEBOT::ntpBegin(int timezoneHours, const char *ntpServer, int daylightOffsetHours, uint32_t timeoutMs)
{
  const long gmtOffsetSec = (long)timezoneHours * 3600L;
  const int daylightOffsetSec = daylightOffsetHours * 3600;
  return ntpSync(ntpServer, gmtOffsetSec, daylightOffsetSec, timeoutMs);
}

inline bool ROLEBOT::ntpIsTimeValid(time_t minEpoch)
{
  if (minEpoch <= 0)
  {
    minEpoch = _NTP_VALID_EPOCH;
  }
  return time(nullptr) >= minEpoch;
}

inline time_t ROLEBOT::ntpGetEpoch()
{
  return time(nullptr);
}

inline String ROLEBOT::ntpGetDateTimeString()
{
  time_t now = time(nullptr);
  if (now < _NTP_VALID_EPOCH)
  {
    return String("");
  }

  struct tm tmInfo;
  localtime_r(&now, &tmInfo);

  char buf[80];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           tmInfo.tm_year + 1900,
           tmInfo.tm_mon + 1,
           tmInfo.tm_mday,
           tmInfo.tm_hour,
           tmInfo.tm_min,
           tmInfo.tm_sec);
  return String(buf);
}

/*********************************** NTP - Blok dostu saat / Block-friendly time ***********************************
 * TR: ESP cekirdegi saati arka planda zaten periyodik olarak (varsayilan ~1 saat) yeniden
 * esitler; ntpUpdate() bunu hemen yapmak icindir.
 * EN: The ESP core already re-syncs the clock periodically in the background (default
 * ~1 hour); ntpUpdate() does it right now.
 */
inline bool ROLEBOT::_ntpLocalTime(struct tm &out)
{
  time_t now = time(nullptr);
  if (now < _NTP_VALID_EPOCH)
  {
    return false;
  }
  localtime_r(&now, &out);
  return true;
}

inline bool ROLEBOT::ntpUpdate()
{
  return ntpSync(_ntpServer.c_str(), _ntpGmtOffsetSec, _ntpDaylightOffsetSec, 10000);
}

inline int ROLEBOT::ntpGetHour()
{
  struct tm t;
  return _ntpLocalTime(t) ? t.tm_hour : -1;
}

inline int ROLEBOT::ntpGetMinute()
{
  struct tm t;
  return _ntpLocalTime(t) ? t.tm_min : -1;
}

inline int ROLEBOT::ntpGetSecond()
{
  struct tm t;
  return _ntpLocalTime(t) ? t.tm_sec : -1;
}

inline int ROLEBOT::ntpGetDay()
{
  struct tm t;
  return _ntpLocalTime(t) ? t.tm_mday : -1;
}

inline int ROLEBOT::ntpGetMonth()
{
  struct tm t;
  return _ntpLocalTime(t) ? t.tm_mon + 1 : -1;
}

inline int ROLEBOT::ntpGetYear()
{
  struct tm t;
  return _ntpLocalTime(t) ? t.tm_year + 1900 : -1;
}

inline int ROLEBOT::ntpGetWeekday()
{
  struct tm t;
  if (!_ntpLocalTime(t))
  {
    return -1;
  }
  // tm_wday: 0=Pazar ... 6=Cumartesi -> 1=Pazartesi ... 7=Pazar
  // / tm_wday: 0=Sunday ... 6=Saturday -> 1=Monday ... 7=Sunday
  return (t.tm_wday == 0) ? 7 : t.tm_wday;
}

inline String ROLEBOT::ntpGetTimeString()
{
  struct tm t;
  if (!_ntpLocalTime(t))
  {
    return String("--:--:--");
  }
  char buf[12];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
  return String(buf);
}

inline String ROLEBOT::ntpGetDateString()
{
  struct tm t;
  if (!_ntpLocalTime(t))
  {
    return String("--.--.----");
  }
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d.%02d.%04d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
  return String(buf);
}

inline bool ROLEBOT::ntpTimeIs(int hour, int minute)
{
  struct tm t;
  return _ntpLocalTime(t) && t.tm_hour == hour && t.tm_min == minute;
}

inline bool ROLEBOT::ntpTimeReached(int hour, int minute)
{
  struct tm t;
  if (!_ntpLocalTime(t) || t.tm_hour != hour || t.tm_min != minute)
  {
    return false;
  }

  // Bu dakikayi benzersiz tanimlayan damga (yil + yilin gunu + dakika).
  // / A stamp that uniquely identifies this minute (year + day of year + minute).
  const int32_t stamp = ((int32_t)(t.tm_year % 100) * 366 + t.tm_yday) * 1440 + hour * 60 + minute;
  const int16_t key = (int16_t)(hour * 60 + minute);

  for (uint8_t i = 0; i < _ntpReachedCount; i++)
  {
    if (_ntpReached[i].key == key)
    {
      if (_ntpReached[i].stamp == stamp)
      {
        return false; // Bu dakikada zaten tetiklendi / already fired in this minute
      }
      _ntpReached[i].stamp = stamp;
      return true;
    }
  }

  // Yeni saat:dakika - bos yuvaya yaz (8 dolarsa en eskisinin yerine).
  // / New hour:minute - use a free slot (reuse the first one if all 8 are taken).
  uint8_t slot = (_ntpReachedCount < 8) ? _ntpReachedCount++ : 0;
  _ntpReached[slot].key = key;
  _ntpReached[slot].stamp = stamp;
  return true;
}

inline bool ROLEBOT::ntpTimeIsBetween(int startHour, int startMinute, int endHour, int endMinute)
{
  struct tm t;
  if (!_ntpLocalTime(t))
  {
    return false;
  }
  const int now = t.tm_hour * 60 + t.tm_min;
  const int start = startHour * 60 + startMinute;
  const int end = endHour * 60 + endMinute;
  if (start == end)
  {
    return false;
  }
  if (start < end)
  {
    return now >= start && now < end;
  }
  return now >= start || now < end; // Gece yarisini asan aralik (22:00-06:00) / range crossing midnight
}

/*********************************** WiFi ***********************************/
#if defined(USE_WIFI)

inline void ROLEBOT::wifiStartAndConnect(const char *ssid, const char *pass)
{
  // Sifre seri porta ACIK yazilmaz (sinifta ekran paylasiminda gorunmesin);
  // sadece uzunlugu kadar '*' basilir. / The password is NOT printed in clear
  // (so it doesn't show on a shared classroom screen); only one '*' per character.
  String masked;
  for (size_t i = 0; pass && pass[i]; i++)
    masked += '*';
  Serial.printf("[WiFi]: Connection Starting!\r\n[WiFi]: SSID: %s\r\n[WiFi]: Pass: %s\r\n", ssid, masked.c_str());

  WiFi.begin(ssid, pass);
  int count = 0;
  while (count < 30)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.printf("\n[WiFi]: Connected!\r\n[WiFi]: Local IP: %s\r\n", WiFi.localIP().toString().c_str());
      Serial.printf("[WiFi]: MAC Address: %s\r\n", WiFi.macAddress().c_str());
      return;
    }
    Serial.print(".");
    delay(500);
    count++;
  }
  Serial.println("[WiFi]: Connection Timeout!");
}

inline bool ROLEBOT::wifiConnectionControl()
{
  // loop() icinde sik cagrilinca her seferinde satir basip seri portu
  // bogmasin diye SADECE durum degisince (ve ilk cagrida) yazar.
  // / Prints ONLY when the state changes (and on the first call) so that
  // frequent calls from loop() don't flood the serial port.
  const bool connected = (WiFi.status() == WL_CONNECTED);
  if (_wifiLastState != (int8_t)connected)
  {
    _wifiLastState = (int8_t)connected;
    Serial.println(connected ? "[WiFi]: Connection OK!" : "[WiFi]: Connection ERROR!");
  }
  return connected;
}

inline String ROLEBOT::wifiGetMACAddress()
{
  return WiFi.macAddress();
}

inline String ROLEBOT::wifiGetIPAddress()
{
  return WiFi.localIP().toString();
}
#endif

/*********************************** URL encode ***********************************/
inline String ROLEBOT::urlEncode(const String &text)
{
  static const char hex[] = "0123456789ABCDEF";
  String out;
  out.reserve(text.length() * 3);
  for (size_t i = 0; i < text.length(); i++)
  {
    const uint8_t c = (uint8_t)text[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
        c == '-' || c == '_' || c == '.' || c == '~')
    {
      out += (char)c;
    }
    else
    {
      out += '%';
      out += hex[c >> 4];
      out += hex[c & 0x0F];
    }
  }
  return out;
}

/*********************************** OTA (Over-The-Air) ***********************************/
#if defined(USE_OTA)
inline void ROLEBOT::otaBegin(const char *hostname, const char *password, uint16_t port)
{
  static String otaHost;
  if (hostname && strlen(hostname) > 0)
  {
    otaHost = hostname;
  }
  else
  {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    otaHost = String("ROLEBOT-") + mac;
  }
  ArduinoOTA.setHostname(otaHost.c_str());

  if (password && strlen(password) > 0)
  {
    ArduinoOTA.setPassword(password);
  }
  else
  {
    ArduinoOTA.setPassword("1234");
  }

  ArduinoOTA.setPort(port);

  ArduinoOTA.onStart([]() {
    Serial.println("[OTA]: Start");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\n[OTA]: End");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (total > 0)
    {
      Serial.printf("[OTA]: Progress: %u%%\r", (progress * 100) / total);
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA]: Error[%u]\n", error);
  });

  ArduinoOTA.begin();
  Serial.println("[OTA]: Ready");
}

inline void ROLEBOT::otaHandle()
{
  ArduinoOTA.handle();
}
#endif

/*********************************** Server ***********************************/
#if defined(USE_SERVER)

// url'i "/..." bicimine getirir: "demopage" -> "/demopage", "/" ve "" -> "/".
// Eskiden her zaman basa "/" eklendigi icin "/" adresi "//" oluyordu.
// / Normalises url to "/...": "demopage" -> "/demopage", "/" and "" -> "/".
// A "/" was always prepended before, so "/" became "//".
inline String ROLEBOT::_serverPath(const char *url)
{
  String path = url ? String(url) : String("");
  path.trim();
  if (!path.startsWith("/"))
  {
    path = "/" + path;
  }
  return path;
}

// Kullanici "/" sayfasini kaydedince serverStart'in varsayilan "CODLAI Server is
// Running!" sayfasini kaldirir: once kaydedilen handler kazandigi icin kullanicinin
// ana sayfasi hic gorunmuyordu. / When the user registers "/", remove
// serverStart's default "CODLAI Server is Running!" page: the handler registered
// first wins, so the user's home page never showed up.
inline void ROLEBOT::_serverClaimRoot(const String &path)
{
  if (path != "/")
  {
    return;
  }
  _serverUserRoot = true;
  if (_serverRootHandler)
  {
    serverCODLAI.removeHandler(_serverRootHandler); // Kutuphane nesneyi siler / the library deletes the object
    _serverRootHandler = nullptr;
  }
}

// Kendi WiFi agini (AP) kurar ve ad / sifre / adresi seri porta yazar.
// / Starts its own WiFi network (AP) and prints name / password / address.
inline void ROLEBOT::_serverStartAP(const char *ssid, const char *password)
{
  const char *apSsid = (ssid && ssid[0]) ? ssid : "CODLAI-ROLEBOT";
  const char *apPass = password ? password : "";
  // softAP() 1-7 karakterlik sifreyi REDDEDER ve ag hic acilmaz; bos sifre ise
  // sifresiz (acik) ag demektir. / softAP() REJECTS a 1-7 character password and
  // no network appears at all; an empty password means an open network.
  if (strlen(apPass) > 0 && strlen(apPass) < 8)
  {
    Serial.println("[AP Mode]: Sifre 8 karakterden kisa, \"12345678\" kullaniliyor / Password shorter than 8 characters, using \"12345678\"");
    apPass = "12345678";
  }

  if (!WiFi.softAP(apSsid, apPass))
  {
    Serial.println("[AP Mode]: Erisim noktasi ACILAMADI! / Access point could NOT be started!");
  }
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  if (_serverDnsStarted)
  {
    dnsServer.stop(); // Ikinci cagrida eskisini kapat / close the old one on a second call
  }
  dnsServer.start(53, "*", apIP);
  _serverDnsStarted = true;

  Serial.println("\n[AP Mode]: Erisim noktasi acildi / Access point started");
  Serial.printf("[AP Mode]: Ag adi / Network name: \"%s\"\n", apSsid);
  Serial.printf("[AP Mode]: Sifre / Password: \"%s\"\n", apPass[0] ? apPass : "(sifresiz / open)");
  Serial.printf("[AP Mode]: Adres / Address: http://%s\n", WiFi.softAPIP().toString().c_str());
}

inline void ROLEBOT::serverStart(const char *mode, const char *ssid, const char *password)
{
  if (mode && strcmp(mode, "STA") == 0)
  {
    // Zaten bu aga bagliysa (ornegin wifiStartAndConnect ile) yeniden baglanma.
    // / Already joined to this network (e.g. via wifiStartAndConnect): don't reconnect.
    if (!(WiFi.status() == WL_CONNECTED && ssid && WiFi.SSID() == ssid))
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);

      Serial.printf("\n[STA Mode]: Connecting to WiFi: %s\n", ssid);

      int retries = 30;
      while (WiFi.status() != WL_CONNECTED && retries > 0)
      {
        delay(1000);
        Serial.print(".");
        retries--;
      }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\n[STA Mode]: Connected!");
      Serial.printf("[STA Mode]: IP Address: http://%s\n", WiFi.localIP().toString().c_str());
    }
    else
    {
      // Eskiden yedek AP modemin adi ve sifresiyle kuruluyordu: ayni adli sahte
      // bir ag cikiyor, modem sifresi 8 karakterden kisaysa ag hic acilmiyordu.
      // Artik sabit "CODLAI-ROLEBOT" adi kullanilir; STA kapatilir ki surekli yeniden
      // baglanma denemeleri AP'nin kanalini bozmasin.
      // / The fallback AP used to reuse the router's name and password: a fake
      // network with the same name appeared, and with a router password shorter
      // than 8 characters no network came up at all. Now the fixed name
      // "CODLAI-ROLEBOT" is used; STA is switched off so endless reconnect attempts
      // don't disturb the AP's channel.
      Serial.println("\n[STA Mode]: Baglanilamadi! Kendi agini (AP) kuruyor... / Connection Failed! Switching to AP Mode...");
      WiFi.mode(WIFI_AP);
      _serverStartAP("CODLAI-ROLEBOT", password);
    }
  }
  else if (mode && strcmp(mode, "AP") == 0)
  {
    _serverStartAP(ssid, password);
  }

  // Sayfalar, WebSocket ve begin() sadece ILK cagrida: serverStart ikinci kez
  // (ornegin STA basarisiz -> "AP") cagrilinca ayni handler'lar iki kez
  // ekleniyordu. / Pages, WebSocket and begin() only on the FIRST call: calling
  // serverStart again (e.g. STA failed -> "AP") used to add the same handlers twice.
  if (_serverStarted)
  {
    return;
  }
  _serverStarted = true;

  // 📌 Varsayilan ana sayfa (kullanici "/" kaydederse kaldirilir)
  // / Default home page (removed when the user registers "/")
  if (!_serverUserRoot)
  {
    _serverRootHandler = &serverCODLAI.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                                          {
        Serial.println("[Local Server]: Root URL Accessed!");
        request->send(200, "text/plain", "CODLAI Server is Running!"); });
  }

  // 📌 404 Hatası
  serverCODLAI.onNotFound([](AsyncWebServerRequest *request)
                          {
      Serial.println("[Local Server]: Received an Unknown Request!");
      request->send(404, "text/plain", "Not Found"); });

  // 📌 **WebSocket Olaylarını Bağla**
  serverCODLAIWebSocket->onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                                 {
      if (type == WS_EVT_CONNECT) {
          Serial.println("WebSocket Client Connected");
      } else if (type == WS_EVT_DISCONNECT) {
          Serial.println("WebSocket Client Disconnected");
      } });

  // 📌 WebSocket'i Sunucuya Bağla
  serverCODLAI.addHandler(serverCODLAIWebSocket);

  // 📌 **En son sunucuyu başlat!**
  serverCODLAI.begin();
  Serial.println("[Local Server]: Server Started! ✅");
}

inline void ROLEBOT::serverCreateLocalPage(const char *url, const char *WEBPageScript, const char *WEBPageCSS, const char *WEBPageHTML, size_t bufferSize)
{
  const String path = _serverPath(url);
  _serverClaimRoot(path);

  // 📌 Sayfa içeriğini oluştur
  serverCODLAI.on(path.c_str(), HTTP_GET, [WEBPageScript, WEBPageCSS, WEBPageHTML, bufferSize](AsyncWebServerRequest *request)
                  {
                    // Buffer boyutu kullanıcının belirttiği veya varsayılan değerle tanımlanır
                    char *buffer = new char[bufferSize];
                    int len = snprintf(buffer, bufferSize, WEBPageHTML, WEBPageScript, WEBPageCSS);

                    if ((size_t)len >= bufferSize)
                    {
                      Serial.println("[ERROR]: Buffer size insufficient, content truncated!");
                    }

                    request->send(200, "text/html", buffer);
                    delete[] buffer; // Dinamik olarak ayrılan belleği serbest bırakın
                  });

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.printf("[Local Server]: Page created at: http://%s%s\n", WiFi.localIP().toString().c_str(), path.c_str());
  }
  else
  {
    Serial.printf("[Local Server]: Page created at: http://%s%s\n", apIP.toString().c_str(), path.c_str());
  }
}

inline void ROLEBOT::serverOnRequest(const char *url, std::function<String()> callback)
{
  const String path = _serverPath(url); // "led/on" da "/led/on" da calisir / both "led/on" and "/led/on" work
  _serverClaimRoot(path);
  serverCODLAI.on(path.c_str(), HTTP_GET, [callback](AsyncWebServerRequest *request)
                  {
                    String response = callback(); // Donanim burada tetiklenir (role vb.)
                    request->send(200, "text/plain", response);
                  });
}

inline void ROLEBOT::serverHandleDNS()
{
  dnsServer.processNextRequest();
}

inline void ROLEBOT::serverContinue()
{
  // AP_STA modunda da (AP + aga bagli) DNS yonlendirmesi calissin; eskiden
  // sadece saf AP modunda calisiyordu. / Keep DNS redirection running in AP_STA
  // mode too (AP + joined network); it used to run only in pure AP mode.
  if (_serverDnsStarted && (WiFi.getMode() & WIFI_AP))
  {
    serverHandleDNS();
  }
}
#endif

/*********************************** Firebase Server Functions ***********************************/
#if defined(USE_FIREBASE)

// Initialize Firebase connection with SignUp Authentication
inline void ROLEBOT::fbServerSetandStartWithUser(const char *projectURL, const char *secretKey, const char *userMail, const char *mailPass)
{
  firebaseData.setResponseSize(1024); // Optimize memory usage

  // Firebase Configuration Settings
  firebaseConfig.api_key = secretKey;
  firebaseConfig.database_url = projectURL;
  firebaseAuth.user.email = userMail;
  firebaseAuth.user.password = mailPass;

  // Zaman aşımı ayarları
  firebaseConfig.timeout.socketConnection = 10 * 1000; // 10 saniye bağlantı zaman aşımı

  // Token durumu izleme ayarı
  // firebaseConfig.token_status_callback = tokenStatusCallback;
  firebaseConfig.max_token_generation_retry = 5; // Daha fazla token yenileme denemesi

  // Wi-Fi bağlantısı kaybolduğunda otomatik yeniden bağlanma
  Firebase.reconnectWiFi(true);

  // Firebase başlat
  Firebase.begin(&firebaseConfig, &firebaseAuth);

  Serial.println("[Firebase]: Verifying user credentials...");
  uint8_t id_count = 0;
  while (firebaseAuth.token.uid == "" && id_count < 50)
  {
    Serial.print('.');
    delay(500);
    id_count++;
  }
  if (firebaseAuth.token.uid == "")
  {
    Serial.println("\n[ERROR]: Authentication timeout.");
  }
  else
  {
    if (Firebase.ready())
    {
      strncpy(uid, firebaseAuth.token.uid.c_str(), 128 - 1); // UID'yi kopyala ve taşma kontrolü yap
      uid[128 - 1] = '\0';                                   // Diziyi null karakter ile sonlandır
      Serial.print("\n[Info]: Doğrulanan Kimlik ID: ");
      Serial.println(uid);
    }
    else
    {
      Serial.print("[ERROR]: Sign-up failed. Reason: ");
      Serial.println(firebaseData.errorReason());
    }
  }
}

/*********************************** Firebase Write Functions ***********************************/

inline void ROLEBOT::fbServerSetInt(const char *dataPath, int data)
{
  // Corrected function call
  if (Firebase.RTDB.setInt(&firebaseData, dataPath, data))
  {
    Serial.println("[SUCCESS]: Integer data sent successfully!");
  }
  else
  {
    Serial.print("[ERROR]: Failed to send integer data. ");
    Serial.printf("HTTP Code: %d\n", firebaseData.httpCode());
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

inline void ROLEBOT::fbServerSetFloat(const char *dataPath, float data)
{
  if (Firebase.RTDB.setFloat(&firebaseData, dataPath, data))
  {
    Serial.println("[SUCCESS]: Float data sent successfully!");
  }
  else
  {
    Serial.print("[ERROR]: Failed to send float data. ");
    Serial.printf("HTTP Code: %d\n", firebaseData.httpCode());
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

inline void ROLEBOT::fbServerSetString(const char *dataPath, String data)
{
  if (Firebase.RTDB.setString(&firebaseData, dataPath, data))
  {
    Serial.println("[SUCCESS]: String data sent successfully!");
  }
  else
  {
    Serial.print("[ERROR]: Failed to send string data. ");
    Serial.printf("HTTP Code: %d\n", firebaseData.httpCode());
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

inline void ROLEBOT::fbServerSetDouble(const char *dataPath, double data)
{
  if (Firebase.RTDB.setDouble(&firebaseData, dataPath, data))
  {
    Serial.println("[SUCCESS]: Double data sent successfully!");
  }
  else
  {
    Serial.print("[ERROR]: Failed to send double data. ");
    Serial.printf("HTTP Code: %d\n", firebaseData.httpCode());
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

inline void ROLEBOT::fbServerSetBool(const char *dataPath, bool data)
{
  if (Firebase.RTDB.setBool(&firebaseData, dataPath, data))
  {
    Serial.println("[SUCCESS]: Boolean data sent successfully!");
  }
  else
  {
    Serial.print("[ERROR]: Failed to send boolean data. ");
    Serial.printf("HTTP Code: %d\n", firebaseData.httpCode());
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

inline void ROLEBOT::fbServerSetJSON(const char *dataPath, String data)
{
  FirebaseJson json;
  json.set(dataPath, data);

  if (Firebase.RTDB.setJSON(&firebaseData, dataPath, &json))
  {
    Serial.println("[SUCCESS]: JSON data sent successfully!");
  }
  else
  {
    Serial.print("[ERROR]: Failed to send JSON data. ");
    Serial.printf("HTTP Code: %d\n", firebaseData.httpCode());
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

/*********************************** Firebase Read Functions ***********************************/

inline int ROLEBOT::fbServerGetInt(const char *dataPath)
{
  if (Firebase.RTDB.getInt(&firebaseData, dataPath))
  {
    Serial.println("[SUCCESS]: Integer data retrieved successfully!");
    return firebaseData.intData();
  }
  Serial.println("[ERROR]: Failed to retrieve integer data.");
  return -1;
}

inline float ROLEBOT::fbServerGetFloat(const char *dataPath)
{
  if (Firebase.RTDB.getFloat(&firebaseData, dataPath))
  {
    Serial.println("[SUCCESS]: Float data retrieved successfully!");
    return firebaseData.floatData();
  }
  Serial.println("[ERROR]: Failed to retrieve float data.");
  return -1.0;
}

inline String ROLEBOT::fbServerGetString(const char *dataPath)
{
  if (Firebase.RTDB.getString(&firebaseData, dataPath))
  {
    Serial.println("[SUCCESS]: String data retrieved successfully!");
    return firebaseData.stringData();
  }
  Serial.println("[ERROR]: Failed to retrieve string data.");
  return "";
}

inline double ROLEBOT::fbServerGetDouble(const char *dataPath)
{
  if (Firebase.RTDB.getDouble(&firebaseData, dataPath))
  {
    Serial.println("[SUCCESS]: Double data retrieved successfully!");
    return firebaseData.doubleData();
  }
  Serial.println("[ERROR]: Failed to retrieve double data.");
  return -1.0;
}

inline bool ROLEBOT::fbServerGetBool(const char *dataPath)
{
  if (Firebase.RTDB.getBool(&firebaseData, dataPath))
  {
    Serial.println("[SUCCESS]: Boolean data retrieved successfully!");
    return firebaseData.boolData();
  }
  Serial.println("[ERROR]: Failed to retrieve boolean data.");
  return false;
}

inline String ROLEBOT::fbServerGetJSON(const char *dataPath)
{
  if (Firebase.RTDB.getJSON(&firebaseData, dataPath))
  {
    Serial.println("[SUCCESS]: JSON data retrieved successfully!");
    return firebaseData.jsonString();
  }
  Serial.println("[ERROR]: Failed to retrieve JSON data.");
  return "{}";
}
#endif

/*********************************** ESP-NOW ***********************************/
#if defined(USE_ESPNOW)
inline void ROLEBOT::initESPNow()
{
  // Zaten AP ya da AP_STA modundaysa (ornegin ayni sketch'te bir web
  // sunucusu/OTA icin softAP() calisiyorsa) bu AP'yi DUSURMEDEN STA'yi
  // ekliyoruz. Kosulsuz WiFi.mode(WIFI_STA) AP'yi anlik olarak kapatirdi.
  // If already in AP or AP_STA mode (e.g. a web server/OTA in the same
  // sketch has called softAP()), add STA WITHOUT dropping that AP.
  // Unconditionally calling WiFi.mode(WIFI_STA) would have silently
  // dropped the AP.
  WiFiMode_t currentMode = WiFi.getMode();
  WiFi.mode((currentMode == WIFI_AP || currentMode == WIFI_AP_STA) ? WIFI_AP_STA : WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // ESP8266'nin klasik espnow.h API'si, esp_now_send()/esp_now_add_peer()
  // cagrilmadan ONCE bir "self role" belirlenmesini gerektirir; bu satir
  // eksikti ve esp_now_send() sessizce basarisiz oluyordu (bkz. MINIBOT.h'de
  // ayni hata gercek donanimda dogrulandi). COMBO, bu cihazin hem
  // gonderici hem alici olarak calismasina izin verir.
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  Serial.println("ESP-NOW Initialized");
}

inline void ROLEBOT::setWiFiChannel(int channel)
{
  wifi_set_channel(channel);
}

inline void ROLEBOT::sendESPNow(uint8_t *macAddr, uint8_t *data, int len)
{
  if (!esp_now_is_peer_exist(macAddr))
  {
    // Peer'i SABIT kanal 1 yerine o anki WiFi kanalina kaydet: setWiFiChannel(6)
    // kullanan projelerde peer yanlis kanalda kaliyordu.
    // / Register the peer on the CURRENT WiFi channel instead of a fixed 1:
    // projects using setWiFiChannel(6) ended up with the peer on the wrong channel.
    if (esp_now_add_peer(macAddr, ESP_NOW_ROLE_SLAVE, wifi_get_channel(), NULL, 0) != 0)
    {
      Serial.println("Failed to add peer");
      return;
    }
  }

  // Basarili gonderimde seri porta YAZMIYORUZ: yogun ESP-NOW trafiginde her
  // pakette satir basmak seri portu bogup loop()'u yavaslatiyordu. Sadece hata.
  // / Do NOT print on success: printing a line per packet flooded the serial
  // port and slowed loop() under heavy ESP-NOW traffic. Errors only.
  int result = esp_now_send(macAddr, data, len);
  if (result != 0)
  {
    Serial.println("Error sending the data");
  }
}

inline void ROLEBOT::registerOnRecv(esp_now_recv_cb_t cb)
{
  esp_now_register_recv_cb(cb);
}

/*********************************** Basit ESP-NOW Mesajlasma ***********************************/
inline bool ROLEBOT::espNowBegin(int channel)
{
  initESPNow();
  setWiFiChannel(channel);
  startListening();
  return true;
}

inline void ROLEBOT::espNowSendText(const String &text)
{
  CodlaiESPNowMessage msg = {};
  msg.deviceType = 20; // 20 = basit metin mesaji / simple text message
  strncpy(msg.text, text.c_str(), sizeof(msg.text) - 1);
  msg.text[sizeof(msg.text) - 1] = '\0';
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  sendESPNow(broadcastAddress, (uint8_t *)&msg, sizeof(msg));
}

inline void ROLEBOT::espNowSendNumber(const String &name, float value)
{
  CodlaiESPNowMessage msg = {};
  msg.deviceType = 21; // 21 = basit sayi mesaji / simple number message
  strncpy(msg.text, name.c_str(), sizeof(msg.text) - 1);
  msg.text[sizeof(msg.text) - 1] = '\0';
  msg.value = value;
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  sendESPNow(broadcastAddress, (uint8_t *)&msg, sizeof(msg));
}

inline bool ROLEBOT::espNowAvailable()
{
  return newData && (receivedData.deviceType == 20 || receivedData.deviceType == 21);
}

inline String ROLEBOT::espNowReadText()
{
  String result = "";
  if (newData && receivedData.deviceType == 20)
  {
    result = String(receivedData.text);
    newData = false;
  }
  return result;
}

inline String ROLEBOT::espNowReadName()
{
  String result = "";
  // newData'ya bakmadan son SAYI mesajini dondur: boylece espNowReadName()
  // ve espNowReadNumber() hangi sirayla cagrilirsa cagrilsin ikisi de dogru
  // degeri verir (eskiden ilki mesaji tuketip digerine 0/"" dondururdu).
  // / Return the last NUMBER message regardless of newData, so
  // espNowReadName() and espNowReadNumber() both give the right value in
  // any order (the first call used to consume the message and make the
  // other one return 0/"").
  if (receivedData.deviceType == 21)
  {
    result = String(receivedData.text);
    newData = false;
  }
  return result;
}

inline float ROLEBOT::espNowReadNumber()
{
  float result = 0.0f;
  // newData'ya bakmadan son SAYI mesajini dondur: boylece espNowReadName()
  // ve espNowReadNumber() hangi sirayla cagrilirsa cagrilsin ikisi de dogru
  // degeri verir (eskiden ilki mesaji tuketip digerine 0/"" dondururdu).
  // / Return the last NUMBER message regardless of newData, so
  // espNowReadName() and espNowReadNumber() both give the right value in
  // any order (the first call used to consume the message and make the
  // other one return 0/"").
  if (receivedData.deviceType == 21)
  {
    result = receivedData.value;
    newData = false;
  }
  return result;
}
#endif

/*********************************** Email ***********************************/
#if defined(USE_EMAIL)
inline void ROLEBOT::sendEmail(String smtpHost, int smtpPort, String authorEmail, String authorPassword, String recipientEmail, String subject, String messageStr)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected!");
    return;
  }

  SMTPSession smtp;
  Session_Config config;
  config.server.host_name = smtpHost;
  config.server.port = smtpPort;
  config.login.email = authorEmail;
  config.login.password = authorPassword;
  config.login.user_domain = "";

  SMTP_Message message;
  message.sender.name = "ROLEBOT";
  message.sender.email = authorEmail;
  message.subject = subject;
  message.addRecipient("User", recipientEmail);
  message.text.content = messageStr.c_str();

  smtp.connect(&config);
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
  else
    Serial.println("Email sent successfully!");
}
#endif

/*********************************** Weather ***********************************/
#if defined(USE_WEATHER)
#include <ArduinoJson.h>
inline String ROLEBOT::getWeather(String city, String apiKey)
{
  if (WiFi.status() != WL_CONNECTED)
    return "WiFi Error";

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  String url;

  // Sehir adi adreste guvenli olsun diye burada kodlanir ("New York" ->
  // "New%20York", "İzmir" -> "%C4%B0zmir"). Ham adi verin; onceden kodlarsaniz
  // iki kez kodlanir. / The city name is encoded here so it is safe in the
  // address ("New York" -> "New%20York", "İzmir" -> "%C4%B0zmir"). Pass the plain
  // name; pre-encoding it would encode it twice.
  city.trim();
  const String cityEnc = urlEncode(city);

  if (apiKey == "" || apiKey == "YOUR_API_KEY") {
      Serial.println("[Weather]: Using wttr.in (Free Service)...");
      
      #if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
      client.setHandshakeTimeout(20000); 
      #endif

      url = "https://wttr.in/" + cityEnc + "?format=%t+%C";
      
      Serial.println("[Weather]: Requesting URL: " + url);
      
      http.begin(client, url);
      #if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
      http.setConnectTimeout(20000); 
      #endif
      
      http.setUserAgent("curl/7.68.0"); 

      Serial.println("[Weather]: Sending GET request...");
      int httpCode = http.GET();
      Serial.println("[Weather]: GET request finished. HTTP Code: " + String(httpCode));

      if (httpCode > 0) {
          String payload = http.getString();
          http.end();
          payload.trim(); 
          Serial.println("[Weather]: Data received: " + payload);
          return payload;
      } else {
          String errorStr = http.errorToString(httpCode);
          http.end();
          Serial.println("[Weather]: Error: " + errorStr);
          return "Error: " + errorStr;
      }
  } 
  else {
      // https: istemci WiFiClientSecure (TLS); eski "http://" adresi 80 portunda
      // TLS denedigi icin OpenWeatherMap istegi hep basarisiz oluyordu.
      // / https: the client is WiFiClientSecure (TLS); the old "http://" address
      // tried TLS on port 80, so the OpenWeatherMap request always failed.
      url = "https://api.openweathermap.org/data/2.5/weather?q=" + cityEnc + "&appid=" + apiKey + "&units=metric";

      http.begin(client, url);
      int httpCode = http.GET();

      if (httpCode > 0)
      {
        String payload = http.getString();
        DynamicJsonDocument doc(1024); // ArduinoJson v6 VE v7 ile uyumlu (JsonDocument sadece v7'de var) / compatible with BOTH ArduinoJson v6 and v7 (JsonDocument only exists in v7)
        deserializeJson(doc, payload);
        float temp = doc["main"]["temp"];
        String weather = doc["weather"][0]["description"];
        http.end();
        return String(temp) + "C, " + weather;
      }
      else
      {
        http.end();
        return "Error (OWM)";
      }
  }
}
#endif

/*********************************** Wikipedia ***********************************/
#if defined(USE_WIKIPEDIA)
#include <ArduinoJson.h>
inline String ROLEBOT::getWikipedia(String query, String lang)
{
  if (WiFi.status() != WL_CONNECTED)
    return "WiFi Error";

  WiFiClientSecure client;
  client.setInsecure(); 
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  client.setHandshakeTimeout(20000); 
#endif

  HTTPClient http;
  // Baslik burada adrese uygun hale getirilir: bosluk -> "_" (Vikipedi basliklari
  // boyle), Turkce harfler ve isaretler -> %XX. Ham basligi verin ("Mustafa Kemal
  // Atatürk"); onceden kodlarsaniz iki kez kodlanir. / The title is prepared for
  // the address here: space -> "_" (how Wikipedia titles look), Turkish letters
  // and symbols -> %XX. Pass the plain title; pre-encoding it would encode twice.
  query.trim();
  query.replace(" ", "_");
  if (lang.length() == 0)
    lang = "en";
  String url = "https://" + lang + ".wikipedia.org/api/rest_v1/page/summary/" + urlEncode(query);

  Serial.println("[Wikipedia]: Requesting URL: " + url);
  
  http.begin(client, url);
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  http.setConnectTimeout(20000); 
#endif
  http.setUserAgent("curl/7.68.0"); 

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();
    DynamicJsonDocument doc(2048); // ArduinoJson v6 VE v7 ile uyumlu / compatible with BOTH ArduinoJson v6 and v7
    deserializeJson(doc, payload);

    // containsKey() ArduinoJson v7'de kaldirildi, v6/v7 ile calisan isNull()
    // kullaniliyor. / containsKey() was removed in ArduinoJson v7, using an
    // isNull() check that works on both v6 and v7.
    if (!doc["extract"].isNull()) {
        String extract = doc["extract"].as<String>();
        http.end();
        return extract;
    } else {
        http.end();
        return "No Summary Found";
    }
  }
  else
  {
    String errorStr = http.errorToString(httpCode);
    http.end();
    Serial.println("[Wikipedia]: Error: " + errorStr);
    return "Error: " + errorStr;
  }
}
#endif

/*********************************** Telegram ***********************************/
#if defined(USE_TELEGRAM)
inline void ROLEBOT::sendTelegram(String token, String chatId, String message)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected!");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  // Mesaj tam UTF-8 yuzde kodlamasiyla adrese eklenir; eskiden sadece bosluklar
  // kodlaniyordu: "&", "#", "+" ve Turkce harfler mesaji bozuyor/kesiyordu. Ham
  // metin verin (onceden kodlarsaniz mesajda "%20" gibi gorunur).
  // / The message goes into the address with full UTF-8 percent-encoding; only
  // spaces used to be encoded: "&", "#", "+" and Turkish letters broke/cut the
  // message. Pass plain text (pre-encoding it would show up as "%20" etc.).
  String url = "https://api.telegram.org/bot" + token + "/sendMessage?chat_id=" + urlEncode(chatId) + "&text=" + urlEncode(message);
  
  http.begin(client, url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Telegram Message Sent!");
  } else if (httpCode > 0) {
    // Sunucu cevap verdi ama reddetti (yanlis token / chat id...) / server answered but refused (wrong token / chat id...)
    Serial.println("Telegram Error: HTTP " + String(httpCode) + " (token / chat id?)");
  } else {
    Serial.println("Error sending Telegram: " + http.errorToString(httpCode));
  }
  http.end();
}
#endif

/*********************************** IFTTT ***********************************/
#if defined(USE_IFTTT)
inline bool ROLEBOT::triggerIFTTTEvent(const String &eventName, const String &webhookKey, const String &jsonPayload)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[IFTTT]: WiFi not connected!");
    return false;
  }

  if (eventName.length() == 0 || webhookKey.length() == 0)
  {
    Serial.println("[IFTTT]: Event name or key missing.");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = "https://maker.ifttt.com/trigger/" + eventName + "/with/key/" + webhookKey;

  Serial.println("[IFTTT]: Triggering '" + eventName + "'...");

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(jsonPayload);

  if (httpCode > 0)
  {
    Serial.println("[IFTTT]: HTTP " + String(httpCode));
    String payload = http.getString();
    Serial.println("[IFTTT]: Response => " + payload);
  }
  else
  {
    Serial.println("[IFTTT]: Error => " + http.errorToString(httpCode));
  }

  http.end();
  return httpCode == HTTP_CODE_OK;
}
#endif

#else
#error "ROLEBOT sadece ESP8266 için desteklenmektedir."
#endif

#endif
