/*
 * TR: ESP-NOW ALICI ÖRNEĞİ. Kendi veri yapımızı (struct_message: metin, tam sayı, ondalıklı
 * sayı, doğru/yanlış) ESP-NOW ile alır ve Seri Port'a yazar. Gönderici olarak
 * ROLEBOT_ESP_NOW_Sender_Example.ino dosyasını başka bir karta yükleyin (veri yapısı
 * İKİ TARAFTA AYNI olmalı). Her paket geldiğinde mavi LED kısa bir süre yanar.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim   / help    -> komut listesi
 *      durum    / status  -> alınan paket sayısı ve bu kartın MAC adresi
 *      sifirla  / reset   -> sayacı sıfırla
 *      dil      / lang    -> dili değiştir (Türkçe <-> English)
 *
 * EN: ESP-NOW RECEIVER EXAMPLE. Receives our own data structure (struct_message: text,
 * integer, float, true/false) over ESP-NOW and prints it to the Serial port. Upload
 * ROLEBOT_ESP_NOW_Sender_Example.ino to another board as the sender (the data structure
 * must be THE SAME on both sides). The blue LED lights briefly for every packet.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help     / yardim   -> command list
 *      status   / durum    -> packets received and this board's MAC address
 *      reset    / sifirla  -> reset the counter
 *      lang     / dil      -> switch language (Turkish <-> English)
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

// Veri yapısı örneği - gönderici ile AYNI olmalı
// Example data structure - must match the sender's structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

struct_message myData;
volatile bool packetReady = false; // Geri çağırma yeni paket bıraktı mı? / did the callback leave a new packet?
volatile uint8_t lastLen = 0;
uint32_t packetCount = 0;
uint32_t ledOffAtMs = 0;
bool ledOn = false;

// Veri gelince çalışan geri çağırma fonksiyonu. Burada sadece veriyi kopyalıyoruz;
// yazdırma işini loop() yapar (geri çağırmanın içinde uzun iş yapılmamalı).
// Callback that runs when data arrives. We only copy the data here; loop() does the
// printing (a callback should not do long work).
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  // Gelen paketten fazlasını okumamak için en fazla "len" bayt kopyala
  // Copy at most "len" bytes so we never read past the incoming packet
  memset(&myData, 0, sizeof(myData));
  memcpy(&myData, incomingData, len < sizeof(myData) ? len : sizeof(myData));
  myData.a[sizeof(myData.a) - 1] = '\0';
  lastLen = len;
  packetReady = true;
}

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "SIFIRLA" -> "sifirla"
// Lower-cases and simplifies Turkish letters: "SIFIRLA" -> "sifirla"
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
  rolebot.serialWrite(L("---- ESP-NOW ALICI - Komutlar ----", "---- ESP-NOW RECEIVER - Commands ----"));
  rolebot.serialWrite(L("  yardim   : bu liste", "  help     : this list"));
  rolebot.serialWrite(L("  durum    : paket sayısı ve MAC adresi", "  status   : packet count and MAC address"));
  rolebot.serialWrite(L("  sifirla  : sayacı sıfırla", "  reset    : reset the counter"));
  rolebot.serialWrite(L("  dil      : English'e geç", "  lang     : switch to Turkish"));
}

void handleCommand(const String &cmd) {
  if (cmd == "yardim" || cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "durum" || cmd == "status") {
    rolebot.serialWrite(String(L("Alınan paket: ", "Packets received: ")) + packetCount +
                        L(" | Bu kartın MAC adresi: ", " | This board's MAC address: ") + rolebot.wifiGetMACAddress());
  } else if (cmd == "sifirla" || cmd == "reset") {
    packetCount = 0;
    rolebot.serialWrite(L("Sayaç sıfırlandı.", "Counter reset."));
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

  rolebot.serialWrite(L("ESP-NOW Alıcı Örneği", "ESP-NOW Receiver Example"));

  // Kartı WiFi istasyonu (STA) olarak ayarla / Set the device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // ESP-NOW'u başlat / Init ESP-NOW
  rolebot.initESPNow();

  // Veri gelince çağrılacak fonksiyonu kaydet / Register the callback called when data arrives
  rolebot.registerOnRecv(OnDataRecv);

  rolebot.serialWrite(String(L("Bu kartın MAC adresi: ", "This board's MAC address: ")) + rolebot.wifiGetMACAddress());
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // 1) Yeni paket varsa yazdır / Print a new packet if there is one
  if (packetReady) {
    struct_message copy;
    noInterrupts();
    copy = myData;
    uint8_t len = lastLen;
    packetReady = false;
    interrupts();

    packetCount++;
    rolebot.serialWrite(String(L("Alınan bayt: ", "Bytes received: ")) + len + L("  (paket #", "  (packet #") + packetCount + ")");
    rolebot.serialWrite(String(L("  Metin    : ", "  Char     : ")) + copy.a);
    rolebot.serialWrite(String(L("  Tam sayı : ", "  Int      : ")) + copy.b);
    rolebot.serialWrite(String(L("  Ondalık  : ", "  Float    : ")) + String(copy.c));
    rolebot.serialWrite(String(L("  Mantıksal: ", "  Bool     : ")) + (copy.d ? L("doğru", "true") : L("yanlış", "false")));
    rolebot.ledWrite(true);
    ledOn = true;
    ledOffAtMs = now + 50;
  }
  if (ledOn && (int32_t)(now - ledOffAtMs) >= 0) {
    ledOn = false;
    rolebot.ledWrite(false);
  }

  // 2) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
