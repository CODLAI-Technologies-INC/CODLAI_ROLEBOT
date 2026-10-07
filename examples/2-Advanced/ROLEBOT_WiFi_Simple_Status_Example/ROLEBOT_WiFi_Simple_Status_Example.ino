/*
 * TR: KABLOSUZ İLETİŞİME İLK ADIM - En basit WiFi örneği. ROLEBOT'u evinizin WiFi ağına
 * bağlar, bağlantı başarılı olursa aldığı IP adresini Seri Port'ta gösterir ve LED'i yakar.
 * Bağlantı koparsa LED söner ve Seri Port'a yazılır. ROLEBOT'ta LCD ekran olmadığı için
 * tüm bilgiler Seri Port (USB) üzerinden verilir. Sunucu YOK, web sayfası YOK - sadece
 * "ağa katılmak" ne demek onu öğretir.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim  / help     -> komut listesi
 *      durum   / status   -> ağ adı, IP, sinyal gücü, MAC adresi
 *      baglan  / connect  -> yeniden bağlanmayı dene (en fazla ~15 sn)
 *      dil     / lang     -> dili değiştir (Türkçe <-> English)
 *
 * EN: FIRST STEP INTO WIRELESS COMMUNICATION - the simplest WiFi example. Connects the
 * ROLEBOT to your home WiFi network and, once connected, shows the IP address on Serial
 * and turns the LED on. If the connection drops, the LED goes off and it is printed.
 * ROLEBOT has no LCD screen, so all feedback is given through the Serial (USB) monitor.
 * NO server, NO web page - just teaches what "joining a network" means.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help    / yardim   -> command list
 *      status  / durum    -> network name, IP, signal strength, MAC address
 *      connect / baglan   -> try to reconnect (up to ~15 s)
 *      lang    / dil      -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 */

#define USE_WIFI
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

// ÖNEMLİ: Kendi WiFi ağınızın adını ve şifresini yazın.
// IMPORTANT: Fill in your own WiFi network's name and password.
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

bool wasConnected = false;
uint32_t lastCheckMs = 0;

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "BAĞLAN" -> "baglan"
// Lower-cases and simplifies Turkish letters: "BAĞLAN" -> "baglan"
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
  rolebot.serialWrite(L("---- WiFi DURUMU - Komutlar ----", "---- WiFi STATUS - Commands ----"));
  rolebot.serialWrite(L("  yardim  : bu liste", "  help    : this list"));
  rolebot.serialWrite(L("  durum   : ağ, IP, sinyal, MAC", "  status  : network, IP, signal, MAC"));
  rolebot.serialWrite(L("  baglan  : yeniden bağlanmayı dene", "  connect : try to reconnect"));
  rolebot.serialWrite(L("  dil     : English'e geç", "  lang    : switch to Turkish"));
}

void printStatus() {
  bool connected = (WiFi.status() == WL_CONNECTED);
  rolebot.serialWrite(String(L("Ağ adı: ", "Network: ")) + WIFI_SSID + "  -> " + (connected ? L("BAĞLI", "CONNECTED") : L("BAĞLI DEĞİL", "NOT CONNECTED")));
  if (connected) {
    rolebot.serialWrite(String(L("IP adresi: ", "IP address: ")) + rolebot.wifiGetIPAddress());
    rolebot.serialWrite(String(L("Sinyal gücü: ", "Signal strength: ")) + WiFi.RSSI() + L(" dBm (0'a yakın = daha iyi)", " dBm (closer to 0 = better)"));
  }
  rolebot.serialWrite(String(L("MAC adresi: ", "MAC address: ")) + rolebot.wifiGetMACAddress());
}

void connectWiFi() {
  rolebot.serialWrite(L("WiFi'ye bağlanılıyor...", "Connecting to WiFi..."));
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS); // En fazla ~15 sn bekler / waits up to ~15 s
  wasConnected = (WiFi.status() == WL_CONNECTED);
  if (wasConnected) {
    rolebot.serialWrite(String(L("Bağlandı! IP adresi: ", "Connected! IP address: ")) + rolebot.wifiGetIPAddress());
  } else {
    rolebot.serialWrite(L("Bağlantı başarısız! SSID/şifreyi kontrol edin.", "Connection failed! Check SSID/password."));
  }
  rolebot.ledWrite(wasConnected); // Bağlıyken LED yanık kalır / LED stays on while connected
}

void handleCommand(const String &cmd) {
  if (cmd == "yardim" || cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "durum" || cmd == "status") {
    printStatus();
  } else if (cmd == "baglan" || cmd == "connect") {
    connectWiFi();
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
  rolebot.begin();
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI / safety: relays OFF
  rolebot.Relay2Write(false);
  rolebot.serialStart(115200);
  connectWiFi();
  printHelp();
}

void loop() {
  // Saniyede bir bağlantıyı kontrol et; değişince yaz ve LED'i güncelle
  // Check the connection once a second; print and update the LED when it changes
  if (millis() - lastCheckMs >= 1000) {
    lastCheckMs = millis();
    bool connected = (WiFi.status() == WL_CONNECTED);
    if (connected != wasConnected) {
      wasConnected = connected;
      rolebot.ledWrite(connected);
      if (connected) rolebot.serialWrite(String(L("WiFi tekrar bağlandı. IP: ", "WiFi reconnected. IP: ")) + rolebot.wifiGetIPAddress());
      else rolebot.serialWrite(L("WiFi bağlantısı koptu! (ESP kendisi yeniden deniyor)", "WiFi connection lost! (the ESP retries by itself)"));
    }
  }

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
