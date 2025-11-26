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

#if defined(USE_SERVER)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#endif

#if defined(USE_FIREBASE)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
#endif

#if defined(USE_WIFI)
#include <ESP8266WiFi.h>
#endif

#if defined(USE_ESPNOW)
#include <espnow.h>
#endif

#if defined(USE_EMAIL)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#include <ESP_Mail_Client.h>
#endif

#if defined(USE_WEATHER) || defined(USE_WIKIPEDIA) || defined(USE_TELEGRAM)
#ifndef USE_WIFI
#define USE_WIFI
#endif
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#endif

// Structure to receive data via ESP-NOW
#ifndef CODLAI_ESPNOW_MESSAGE_DEFINED
#define CODLAI_ESPNOW_MESSAGE_DEFINED
typedef struct {
  uint8_t deviceType; // 1 = Armbot
  int axis1;
  int axis2;
  int axis3;
  int gripper;
  uint8_t action; // 0=None, 1=Horn, 2=Note
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
  bool button1Read();

  /*********************************** LED ***********************************/
  void ledWrite(bool status);

  /*********************************** Relays ***********************************
   */
  void Relay1Write(bool status);
  void Relay2Write(bool status);

  /*********************************** EEPROM  ***********************************
   */
  void eepromWriteInt(int address, int value);
  int eepromReadInt(int address);

  /*********************************** WiFi  ***********************************
   */
#if defined(USE_WIFI)
  void wifiStartAndConnect(const char *ssid, const char *pass);
  bool wifiConnectionControl();
  String wifiGetMACAddress();
  String wifiGetIPAddress();
#endif

  /*********************************** Server  ***********************************
   */
#if defined(USE_SERVER)
  void serverStart(const char *mode, const char *ssid, const char *password);
  void serverCreateLocalPage(const char *url, const char *WEBPageScript, const char *WEBPageCSS, const char *WEBPageHTML, size_t bufferSize = 4096);
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
          if (_instance && len == sizeof(CodlaiESPNowMessage)) {
              memcpy(&_instance->receivedData, incomingData, sizeof(CodlaiESPNowMessage));
              _instance->newData = true;
          }
      });
  }
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
#if defined(USE_SERVER)
  const IPAddress apIP = IPAddress(192, 168, 4, 1); // Sabit IP adresi tanımlanıyor / Define static IP address
  DNSServer dnsServer;                              // DNS sunucusu tanımlanıyor / Define DNS Server
  AsyncWebServer serverCODLAI{80};                  // Web server objesi
  AsyncWebSocket *serverCODLAIWebSocket;            // Pointer olarak tanımla
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
inline void ROLEBOT::eepromWriteInt(int address, int value) // EEPROM'a güvenli bir şekilde int türünde veri yazmak için fonksiyon
{
  byte highByte = highByte(value); // int'in yüksek baytını al
  byte lowByte = lowByte(value);   // int'in düşük baytını al

  EEPROM.write(address, highByte);    // İlk baytı EEPROM'a yaz
  EEPROM.write(address + 1, lowByte); // İkinci baytı EEPROM'a yaz
  EEPROM.commit();                    // Değişiklikleri kaydetmek için commit işlemi yapılmalıdır
}

inline int ROLEBOT::eepromReadInt(int address) // EEPROM'dan int türünde veri okumak için fonksiyon
{
  byte highByte = EEPROM.read(address);    // İlk baytı oku
  byte lowByte = EEPROM.read(address + 1); // İkinci baytı oku
  return word(highByte, lowByte);          // Yüksek ve düşük baytları birleştirerek int değeri oluştur
}

/*********************************** WiFi ***********************************/
#if defined(USE_WIFI)

inline void ROLEBOT::wifiStartAndConnect(const char *ssid, const char *pass)
{
  Serial.printf("[WiFi]: Connection Starting!\r\n[WiFi]: SSID: %s\r\n[WiFi]: Pass: %s\r\n", ssid, pass);

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
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("[WiFi]: Connection OK!");
    return true;
  }
  else
  {
    Serial.println("[WiFi]: Connection ERROR!");
    return false;
  }
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

/*********************************** Server ***********************************/
#if defined(USE_SERVER)

inline void ROLEBOT::serverStart(const char *mode, const char *ssid, const char *password)
{
  if (strcmp(mode, "STA") == 0)
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

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\n[STA Mode]: Connected!");
      Serial.printf("[STA Mode]: IP Address: http://%s\n", WiFi.localIP().toString().c_str());
    }
    else
    {
      Serial.println("\n[STA Mode]: Connection Failed! Switching to AP Mode...");
      serverStart("AP", ssid, password);
      return;
    }
  }
  else if (strcmp(mode, "AP") == 0)
  {
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

    Serial.printf("\n[AP Mode]: Access Point Started!\n");
    Serial.printf("[AP Mode]: SSID: \"%s\"\n", ssid);
    Serial.printf("[AP Mode]: Password: \"%s\"\n", password);
    Serial.printf("[AP Mode]: AP IP Address: http://%s\n", WiFi.softAPIP().toString().c_str());
  }

  // 📌 Sayfaları tanımla
  serverCODLAI.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
      Serial.println("[Local Server]: Root URL Accessed!");
      request->send(200, "text/plain", "CODLAI Server is Running!"); });

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
  // 📌 Sayfa içeriğini oluştur
  serverCODLAI.on(("/" + String(url)).c_str(), HTTP_GET, [WEBPageScript, WEBPageCSS, WEBPageHTML, bufferSize](AsyncWebServerRequest *request)
                  {
                    // Buffer boyutu kullanıcının belirttiği veya varsayılan değerle tanımlanır
                    char *buffer = new char[bufferSize];
                    int len = snprintf(buffer, bufferSize, WEBPageHTML, WEBPageScript, WEBPageCSS);

                    if (len >= bufferSize)
                    {
                      Serial.println("[ERROR]: Buffer size insufficient, content truncated!");
                    }

                    request->send(200, "text/html", buffer);
                    delete[] buffer; // Dinamik olarak ayrılan belleği serbest bırakın
                  });

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.printf("[Local Server]: Page created at: http://%s/%s\n", WiFi.localIP().toString().c_str(), url);
  }
  else
  {
    Serial.printf("[Local Server]: Page created at: http://%s/%s\n", apIP.toString().c_str(), url);
  }
}

inline void ROLEBOT::serverHandleDNS()
{
  dnsServer.processNextRequest();
}

inline void ROLEBOT::serverContinue()
{
  if (WiFi.getMode() == WIFI_AP)
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
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
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
    if (esp_now_add_peer(macAddr, ESP_NOW_ROLE_SLAVE, 1, NULL, 0) != 0)
    {
      Serial.println("Failed to add peer");
      return;
    }
  }

  int result = esp_now_send(macAddr, data, len);
  if (result == 0)
  {
    Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }
}

inline void ROLEBOT::registerOnRecv(esp_now_recv_cb_t cb)
{
  esp_now_register_recv_cb(cb);
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

  if (apiKey == "" || apiKey == "YOUR_API_KEY") {
      Serial.println("[Weather]: Using wttr.in (Free Service)...");
      
      client.setHandshakeTimeout(20000); 

      url = "https://wttr.in/" + city + "?format=%t+%C";
      
      Serial.println("[Weather]: Requesting URL: " + url);
      
      http.begin(client, url);
      http.setConnectTimeout(20000); 
      
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
      url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=metric";

      http.begin(client, url);
      int httpCode = http.GET();

      if (httpCode > 0)
      {
        String payload = http.getString();
        JsonDocument doc; 
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
  client.setHandshakeTimeout(20000); 

  HTTPClient http;
  String url = "https://" + lang + ".wikipedia.org/api/rest_v1/page/summary/" + query;

  Serial.println("[Wikipedia]: Requesting URL: " + url);
  
  http.begin(client, url);
  http.setConnectTimeout(20000); 
  http.setUserAgent("curl/7.68.0"); 

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();
    JsonDocument doc; 
    deserializeJson(doc, payload);
    
    if (doc.containsKey("extract")) {
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
  
  // URL Encode the message manually for basic characters or use a library if needed. 
  // For simplicity, we replace spaces with %20.
  message.replace(" ", "%20");
  
  String url = "https://api.telegram.org/bot" + token + "/sendMessage?chat_id=" + chatId + "&text=" + message;
  
  http.begin(client, url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.println("Telegram Message Sent!");
  } else {
    Serial.println("Error sending Telegram: " + http.errorToString(httpCode));
  }
  http.end();
}
#endif

#else
#error "ROLEBOT sadece ESP8266 için desteklenmektedir."
#endif

#endif
