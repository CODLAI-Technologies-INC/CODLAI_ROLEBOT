/*
 * TR: FIREBASE KULLANICI DOĞRULAMA VE VERİ OKUMA/YAZMA. ROLEBOT WiFi'ye bağlanır, bir
 * Firebase kullanıcısıyla (e-posta + şifre) giriş yapar, Realtime Database'e birkaç değer
 * yazar ve sonra dakikada bir geri okur.
 *  - Firebase özelliklerini kullanmak için kütüphaneyi eklemeden ÖNCE "#define USE_FIREBASE"
 *    yazılmalıdır (aşağıda var).
 *  - Firebase konsolunda: Authentication'da "E-posta/Şifre" ile bir kullanıcı oluşturun;
 *    Realtime Database adresini ve Web API anahtarını (Proje ayarları) aşağıya yazın.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim        / help       -> komut listesi
 *      oku           / read       -> değerleri şimdi oku
 *      sicaklik 30   / temp 30    -> /device/temperature değerini yaz
 *      dil           / lang       -> dili değiştir (Türkçe <-> English)
 *
 * EN: FIREBASE USER VERIFICATION AND DATA READ/WRITE. The ROLEBOT connects to WiFi, signs
 * in with a Firebase user (email + password), writes a few values to the Realtime Database
 * and then reads them back once a minute.
 *  - To use the Firebase features, "#define USE_FIREBASE" must be written BEFORE including
 *    the library (it is below).
 *  - In the Firebase console: create a user with "Email/Password" in Authentication; put the
 *    Realtime Database URL and the Web API key (Project settings) below.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help          / yardim       -> command list
 *      read          / oku          -> read the values now
 *      temp 30       / sicaklik 30  -> write the /device/temperature value
 *      lang          / dil          -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 */

#define USE_FIREBASE
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

// Firebase yapılandırması / Firebase configuration
#define FIREBASE_PROJECT_URL "https://YOUR_PROJECT_ID-default-rtdb.firebaseio.com/" // Firebase veritabanı adresi / Firebase database URL
#define FIREBASE_API_KEY "YOUR_FIREBASE_WEB_API_KEY"                                // Firebase Web API anahtarı / Firebase Web API key

// Firebase kullanıcı girişi / Firebase user authentication
#define USER_EMAIL "YOUR_FIREBASE_USER_EMAIL"       // Firebase'de oluşturduğunuz kullanıcının e-postası / email of the Firebase user
#define USER_PASSWORD "YOUR_FIREBASE_USER_PASSWORD" // O kullanıcının şifresi / that user's password

// WiFi ayarları / WiFi settings
#define WIFI_SSID "YOUR_WIFI_SSID"     // Bağlanılacak WiFi ağının adı / name of the WiFi network
#define WIFI_PASS "YOUR_WIFI_PASSWORD" // WiFi şifresi / WiFi password

const uint32_t READ_EVERY_MS = 60000; // Dakikada bir oku / read once a minute
uint32_t lastReadMs = 0;
bool firstRead = true;

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "SICAKLIK" -> "sicaklik"
// Lower-cases and simplifies Turkish letters: "SICAKLIK" -> "sicaklik"
String normalizeCommand(String s) {
  s.trim();
  s.replace("İ", "i"); s.replace("I", "i"); s.replace("ı", "i");
  s.replace("Ş", "s"); s.replace("ş", "s");
  s.replace("Ğ", "g"); s.replace("ğ", "g");
  s.replace("Ü", "u"); s.replace("ü", "u");
  s.replace("Ö", "o"); s.replace("ö", "o");
  s.replace("Ç", "c"); s.replace("ç", "c");
  s.toLowerCase();
  return s;
}

bool readCommand(String &cmd) {
  while (Serial.available() > 0) {
    char c = Serial.read();
    lastCharMs = millis();
    if (c == '\n' || c == '\r') {
      if (cmdBuffer.length() == 0) continue;
      cmd = normalizeCommand(cmdBuffer);
      cmdBuffer = "";
      return true;
    }
    if (cmdBuffer.length() < 40) cmdBuffer += c;
  }
  // "Satır sonu yok" seçiliyse: 150 ms sessizlikten sonra komutu kabul et.
  // "No line ending" selected: accept the command after 150 ms of silence.
  if (cmdBuffer.length() > 0 && millis() - lastCharMs > 150) {
    cmd = normalizeCommand(cmdBuffer);
    cmdBuffer = "";
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------------
// Adım 4: Firebase'den oku ve Seri Port'a yaz / Step 4: read from Firebase and print
void readValues() {
  lastReadMs = millis();
  int temp = rolebot.fbServerGetInt("/device/temperature");
  String status = rolebot.fbServerGetString("/device/status");
  bool active = rolebot.fbServerGetBool("/device/active");

  rolebot.serialWrite(String(L("Sıcaklık: ", "Temperature: ")) + temp);
  rolebot.serialWrite(String(L("Durum: ", "Status: ")) + status);
  rolebot.serialWrite(String(L("Aktif: ", "Active: ")) + (active ? L("Evet", "Yes") : L("Hayır", "No")));
}

void printHelp() {
  rolebot.serialWrite(L("---- FIREBASE - Komutlar ----", "---- FIREBASE - Commands ----"));
  rolebot.serialWrite(L("  yardim       : bu liste", "  help         : this list"));
  rolebot.serialWrite(L("  oku          : değerleri şimdi oku", "  read         : read the values now"));
  rolebot.serialWrite(L("  sicaklik 30  : sıcaklık değerini yaz", "  temp 30      : write the temperature value"));
  rolebot.serialWrite(L("  dil          : English'e geç", "  lang         : switch to Turkish"));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  bool hasValue = space > 0;
  int value = hasValue ? cmd.substring(space + 1).toInt() : 0;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "oku" || word == "read") {
    readValues();
  } else if ((word == "sicaklik" || word == "temp" || word == "temperature") && hasValue) {
    rolebot.fbServerSetInt("/device/temperature", value);
    rolebot.serialWrite(String(L("Yazılan sıcaklık: ", "Temperature written: ")) + value);
  } else if (word == "dil" || word == "lang" || word == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else {
    rolebot.serialWrite(String(L("Bilinmeyen komut: ", "Unknown command: ")) + cmd + L("  (yardim yazın)", "  (type help)"));
  }
}

// ---------------------------------------------------------------------------
void setup() {
  rolebot.serialStart(115200); // Seri haberleşme / Serial communication
  rolebot.begin();
  rolebot.Relay1Write(false);  // Güvenlik: röleler KAPALI / safety: relays OFF
  rolebot.Relay2Write(false);
  rolebot.serialWrite(L("ROLEBOT Firebase örneği başlıyor...", "ROLEBOT Firebase example starting..."));

  // Adım 1: WiFi'ye bağlan / Step 1: connect to WiFi
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS);

  if (WiFi.status() != WL_CONNECTED) {
    rolebot.serialWrite(L("WiFi yok - bağlantı bekleniyor...", "No WiFi - waiting for the connection..."));
    // Bağlantı gelene kadar bekle. delay() ESP8266'nın arka plan işlerine (WiFi, bekçi
    // köpeği zamanlayıcısı) zaman tanır; delay'siz bir döngü kartı yeniden başlatırdı.
    // Wait until connected. delay() gives the ESP8266 time for background work (WiFi,
    // watchdog); a loop without delay would reset the board.
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
    rolebot.serialWrite(L("Bağlantı başarılı! Devam ediliyor...", "Connection successful! Continuing..."));
  }

  // Adım 2: Firebase'i başlat ve kullanıcıyı doğrula / Step 2: start Firebase and verify the user
  rolebot.fbServerSetandStartWithUser(FIREBASE_PROJECT_URL, FIREBASE_API_KEY, USER_EMAIL, USER_PASSWORD);

  // Adım 3: Firebase'e veri gönder / Step 3: send data to Firebase
  rolebot.fbServerSetInt("/device/temperature", 25);
  rolebot.fbServerSetString("/device/status", "Online");
  rolebot.fbServerSetBool("/device/active", true);
  rolebot.serialWrite(L("Veriler Firebase'e gönderildi.", "Data sent to Firebase."));
  printHelp();
}

void loop() {
  // Dakikada bir oku (ilk seferde hemen); loop() beklemez, komutlar hemen çalışır.
  // Read once a minute (right away the first time); loop() never waits, commands work instantly.
  if (firstRead || millis() - lastReadMs >= READ_EVERY_MS) {
    firstRead = false;
    readValues();
  }

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
