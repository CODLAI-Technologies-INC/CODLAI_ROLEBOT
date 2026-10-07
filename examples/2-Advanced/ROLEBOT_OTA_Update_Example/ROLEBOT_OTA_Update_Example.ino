/*
 * TR: KABLOSUZ YAZILIM GÜNCELLEME (OTA - Over The Air). İlk yüklemeyi USB kablosuyla
 * yapın; sonra ROLEBOT aynı WiFi ağındayken yeni kodları KABLOSUZ yükleyebilirsiniz:
 * Arduino IDE'de "Araçlar > Port" menüsünde ağ portu olarak "ROLEBOT-OTA" görünür
 * (PlatformIO: upload_protocol = espota, upload_port = <IP adresi>). Şifre: 1234.
 *  - OTA için önce WiFi bağlantısı kurulmalı ve loop() içinde otaHandle() sürekli
 *    çağrılmalıdır (aşağıda var). Kütüphaneyi eklemeden ÖNCE "#define USE_OTA" yazılmalıdır.
 *  - Mavi LED, OTA hazırken yavaşça yanıp söner (kart çalışıyor mu görmek için).
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim  / help    -> komut listesi
 *      durum   / status  -> WiFi, IP adresi ve OTA cihaz adı
 *      dil     / lang    -> dili değiştir (Türkçe <-> English)
 *
 * EN: WIRELESS FIRMWARE UPDATE (OTA - Over The Air). Do the first upload with the USB
 * cable; after that, while the ROLEBOT is on the same WiFi network, you can upload new code
 * WIRELESSLY: in the Arduino IDE "Tools > Port" menu, "ROLEBOT-OTA" shows up as a network
 * port (PlatformIO: upload_protocol = espota, upload_port = <IP address>). Password: 1234.
 *  - For OTA, WiFi must be connected first and otaHandle() must be called continuously in
 *    loop() (done below). "#define USE_OTA" must be written BEFORE including the library.
 *  - The blue LED blinks slowly while OTA is ready (to see that the board is running).
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help    / yardim  -> command list
 *      status  / durum   -> WiFi, IP address and OTA device name
 *      lang    / dil     -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 */

#define USE_WIFI
#define USE_OTA

#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char *OTA_HOST = "ROLEBOT-OTA"; // Cihaz adı (ağ portunda görünür) / device name (shown as network port)
const char *OTA_PASS = "1234";        // OTA şifresi (varsayılan 1234) / OTA password (default 1234)

uint32_t lastBlinkMs = 0;
bool ledState = false;

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "YARDIM" -> "yardim"
// Lower-cases and simplifies Turkish letters: "YARDIM" -> "yardim"
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
void printHelp() {
  rolebot.serialWrite(L("---- OTA GÜNCELLEME - Komutlar ----", "---- OTA UPDATE - Commands ----"));
  rolebot.serialWrite(L("  yardim  : bu liste", "  help    : this list"));
  rolebot.serialWrite(L("  durum   : WiFi, IP, OTA cihaz adı", "  status  : WiFi, IP, OTA device name"));
  rolebot.serialWrite(L("  dil     : English'e geç", "  lang    : switch to Turkish"));
}

void printStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    rolebot.serialWrite(String(L("WiFi BAĞLI - IP: ", "WiFi CONNECTED - IP: ")) + rolebot.wifiGetIPAddress());
    rolebot.serialWrite(String(L("OTA cihaz adı: ", "OTA device name: ")) + OTA_HOST + L("  (port 8266, şifre: ", "  (port 8266, password: ") + OTA_PASS + ")");
  } else {
    rolebot.serialWrite(L("WiFi BAĞLI DEĞİL - OTA çalışmaz. SSID/şifreyi kontrol edin.", "WiFi NOT CONNECTED - OTA cannot work. Check SSID/password."));
  }
}

void handleCommand(const String &cmd) {
  if (cmd == "yardim" || cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "durum" || cmd == "status") {
    printStatus();
  } else if (cmd == "dil" || cmd == "lang" || cmd == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else {
    rolebot.serialWrite(String(L("Bilinmeyen komut: ", "Unknown command: ")) + cmd + L("  (yardim yazın)", "  (type help)"));
  }
}

// ---------------------------------------------------------------------------
void setup() {
  rolebot.serialStart(115200);
  rolebot.begin();
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI / safety: relays OFF
  rolebot.Relay2Write(false);

  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS); // Önce WiFi / WiFi first
  rolebot.otaBegin(OTA_HOST, OTA_PASS, 8266);        // Sonra OTA (8266 = ESP8266 OTA portu) / then OTA (8266 = ESP8266 OTA port)
  printStatus();
  printHelp();
}

void loop() {
  rolebot.otaHandle(); // OTA isteklerini dinle - sürekli çağrılmalı / listen for OTA requests - must be called continuously

  // Yavaş yanıp sönen LED = kart çalışıyor / slow blinking LED = the board is running
  if (millis() - lastBlinkMs >= 1000) {
    lastBlinkMs = millis();
    ledState = !ledState;
    rolebot.ledWrite(ledState);
  }

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
