/*
 * TR: ESP-NOW GÖNDERİCİ ÖRNEĞİ. Kendi veri yapımızı (struct_message: metin, tam sayı,
 * ondalıklı sayı, doğru/yanlış) 2 saniyede bir ESP-NOW ile gönderir. Alıcı olarak
 * ROLEBOT_ESP_NOW_Receiver_Example.ino dosyasını başka bir karta yükleyin (veri yapısı
 * İKİ TARAFTA AYNI olmalı). Adres FF:FF:FF:FF:FF:FF = herkese yayın; sadece tek bir karta
 * göndermek için alıcının MAC adresini yazın (alıcı açılışta MAC adresini yazar).
 * B1 butonuna basınca hemen bir paket gönderilir.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim           / help            -> komut listesi
 *      gonder           / send            -> hemen bir paket gönder
 *      metin Merhaba    / text Hello      -> gönderilecek metni değiştir
 *      dur              / pause           -> otomatik gönderimi durdur
 *      devam            / resume          -> otomatik gönderime devam et
 *      dil              / lang            -> dili değiştir (Türkçe <-> English)
 *
 * EN: ESP-NOW SENDER EXAMPLE. Sends our own data structure (struct_message: text,
 * integer, float, true/false) over ESP-NOW every 2 seconds. Upload
 * ROLEBOT_ESP_NOW_Receiver_Example.ino to another board as the receiver (the data
 * structure must be THE SAME on both sides). Address FF:FF:FF:FF:FF:FF = broadcast to
 * everyone; to send to a single board, write the receiver's MAC address (the receiver
 * prints its MAC address at startup). Press B1 to send a packet right away.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help             / yardim          -> command list
 *      send             / gonder          -> send a packet now
 *      text Hello       / metin Merhaba   -> change the text to send
 *      pause            / dur             -> stop automatic sending
 *      resume           / devam           -> resume automatic sending
 *      lang             / dil             -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 */
#define USE_ESPNOW
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

// ALICININ MAC ADRESİNİ YAZIN (FF:FF:FF:FF:FF:FF = herkese yayın)
// REPLACE WITH YOUR RECEIVER MAC Address (FF:FF:FF:FF:FF:FF = broadcast)
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Veri yapısı örneği - alıcı ile AYNI olmalı
// Example data structure - must match the receiver's structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

struct_message myData;

String textToSend = "Hello from ROLEBOT"; // Gönderilecek metin / text to send
bool autoSend = true;
uint32_t lastSendMs = 0;
const uint32_t SEND_INTERVAL_MS = 2000;
bool lastButton = true;  // HIGH = bırakılmış / released
uint32_t lastEdgeMs = 0; // Parazit süzgeci / debounce

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
String rawCommand; // Komutun orijinal hali (metin büyük/küçük harfiyle gitsin diye) / original text (keeps letter case)
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "GÖNDER" -> "gonder"
// Lower-cases and simplifies Turkish letters: "GÖNDER" -> "gonder"
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
      rawCommand = cmdBuffer; rawCommand.trim();
      cmd = normalizeCommand(cmdBuffer);
      cmdBuffer = "";
      return true;
    }
    if (cmdBuffer.length() < 40) cmdBuffer += c;
  }
  // "Satır sonu yok" seçiliyse: 150 ms sessizlikten sonra komutu kabul et.
  // "No line ending" selected: accept the command after 150 ms of silence.
  if (cmdBuffer.length() > 0 && millis() - lastCharMs > 150) {
    rawCommand = cmdBuffer; rawCommand.trim();
    cmd = normalizeCommand(cmdBuffer);
    cmdBuffer = "";
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------------
void sendPacket() {
  // Gönderilecek değerleri hazırla / Set values to send
  memset(&myData, 0, sizeof(myData));
  strncpy(myData.a, textToSend.c_str(), sizeof(myData.a) - 1); // Taşmasın diye en fazla 31 karakter / at most 31 chars (no overflow)
  myData.b = random(1, 20);
  myData.c = 1.2;
  myData.d = false;

  // ESP-NOW ile gönder / Send message via ESP-NOW
  rolebot.sendESPNow(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  rolebot.serialWrite(String(L("Gönderildi -> metin: \"", "Sent -> text: \"")) + myData.a + L("\", tam sayı: ", "\", int: ") + myData.b);
}

void printHelp() {
  rolebot.serialWrite(L("---- ESP-NOW GÖNDERİCİ - Komutlar ----", "---- ESP-NOW SENDER - Commands ----"));
  rolebot.serialWrite(L("  yardim         : bu liste", "  help           : this list"));
  rolebot.serialWrite(L("  gonder         : hemen gönder", "  send           : send now"));
  rolebot.serialWrite(L("  metin Merhaba  : gönderilecek metin", "  text Hello     : text to send"));
  rolebot.serialWrite(L("  dur / devam    : otomatik gönderim", "  pause / resume : automatic sending"));
  rolebot.serialWrite(L("  dil            : English'e geç", "  lang           : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu      : hemen gönder", "  B1 button      : send now"));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "gonder" || word == "send") {
    sendPacket();
  } else if ((word == "metin" || word == "text") && space > 0) {
    textToSend = rawCommand.substring(rawCommand.indexOf(' ') + 1);
    textToSend.trim();
    rolebot.serialWrite(String(L("Yeni metin: ", "New text: ")) + textToSend);
  } else if (word == "dur" || word == "durdur" || word == "pause" || word == "stop") {
    autoSend = false;
    rolebot.serialWrite(L("Otomatik gönderim durdu (gonder / B1 ile elle gönderin).", "Automatic sending paused (send by hand with send / B1)."));
  } else if (word == "devam" || word == "resume" || word == "start") {
    autoSend = true;
    rolebot.serialWrite(L("Otomatik gönderim devam ediyor.", "Automatic sending resumed."));
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
  rolebot.serialStart(115200);
  rolebot.begin();
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI / safety: relays OFF
  rolebot.Relay2Write(false);

  rolebot.serialWrite(L("ESP-NOW Gönderici Örneği", "ESP-NOW Sender Example"));

  // Kartı WiFi istasyonu (STA) olarak ayarla / Set the device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // ESP-NOW'u başlat / Init ESP-NOW
  rolebot.initESPNow();
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // 1) 2 saniyede bir otomatik gönder (beklemeden) / send every 2 seconds (non-blocking)
  if (autoSend && now - lastSendMs >= SEND_INTERVAL_MS) {
    lastSendMs = now;
    sendPacket();
  }

  // 2) B1: basınca hemen gönder (30 ms parazit süzgeci) / B1: send now on press (30 ms debounce)
  bool button = rolebot.button1Read(); // false = basılı / pressed
  if (button != lastButton && now - lastEdgeMs >= 30) {
    lastEdgeMs = now;
    if (lastButton && !button) sendPacket();
    lastButton = button;
  }

  // 3) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
