/*
 * TR: ESP-NOW'A İLK ADIM - en basit kablosuz haberleşme örneği. Bir MAC adresi bilmenize
 * gerek YOK: bu kod bir sayacı "yayın" (broadcast) olarak havaya gönderir ve aynı odada
 * ESP-NOW ile dinleyen HERHANGİ bir CODLAI kartı (başka bir ROLEBOT, bir IOTBOT ya da bir
 * MINIBOT - hepsi aynı veri yapısını kullanır) bunu duyabilir. Aynı anda hem gönderiyor
 * hem dinliyoruz. Yayın alındığında mavi LED kısa bir süre yanar.
 *  - Gönderen kart, paketteki kimlikten (deviceType) tanınır: 40 = IOTBOT,
 *    41 = MINIBOT, 42 = ROLEBOT (40-49 kütüphane örneklerine ayrılmıştır).
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim      / help       -> komut listesi
 *      dur         / pause      -> yayını durdur (dinlemeye devam eder)
 *      devam       / resume     -> yayına devam et
 *      gonder      / send       -> hemen bir yayın gönder
 *      aralik 500  / interval 500 -> yayın aralığı (ms, 200-10000)
 *      sifirla     / reset      -> sayacı sıfırla
 *      dil         / lang       -> dili değiştir (Türkçe <-> English)
 *
 * EN: FIRST STEP INTO ESP-NOW - the simplest wireless example. You do NOT need to know
 * any MAC address: this code broadcasts a counter into the air, and ANY nearby CODLAI
 * board listening over ESP-NOW (another ROLEBOT, an IOTBOT, or a MINIBOT - they all share
 * the same data structure) can hear it. We both send AND listen at the same time. The
 * blue LED lights briefly whenever a broadcast is received.
 *  - The sender is recognised by the ID in the packet (deviceType): 40 = IOTBOT,
 *    41 = MINIBOT, 42 = ROLEBOT (40-49 are reserved for the library examples).
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help         / yardim     -> command list
 *      pause        / dur        -> stop broadcasting (keeps listening)
 *      resume       / devam      -> resume broadcasting
 *      send         / gonder     -> send one broadcast now
 *      interval 500 / aralik 500 -> broadcast interval (ms, 200-10000)
 *      reset        / sifirla    -> reset the counter
 *      lang         / dil        -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 * ROLEBOT'ta LCD ekran YOK; tüm bilgiler Seri Port (USB) üzerinden verilir.
 * / ROLEBOT has NO LCD screen; all feedback is given through Serial (USB).
 */

#define USE_ESPNOW
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

namespace {
  uint32_t counter = 0;
  uint32_t lastSendMs = 0;
  uint32_t sendIntervalMs = 1000;
  bool sending = true;
  uint32_t ledOffAtMs = 0;
  bool ledOn = false;

  void say(const char *turkishText, const char *englishText) {
    rolebot.serialWrite(L(turkishText, englishText));
  }

  void sendCounter() {
    counter++;
    CodlaiESPNowMessage outgoing = {}; // Tüm alanlar sıfırla başlar / all fields start at zero
    outgoing.deviceType = 42; // 42 = ROLEBOT örnek kartı kimliği (40-49 örnekler için ayrıldı; 30-39 Otonom projesine ayrılmış) / ROLEBOT example board ID (40-49 reserved for examples; 30-39 belong to the Otonom project)
    outgoing.axis1 = counter;
    outgoing.axis2 = 0;
    outgoing.axis3 = 0;
    outgoing.gripper = 0;
    outgoing.action = 0;
    rolebot.sendESPNow(broadcastAddress, (uint8_t *)&outgoing, sizeof(outgoing));
    rolebot.serialWrite(String(L("Gönderilen: ", "Sent: ")) + counter);
  }
}

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
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
  say("---- ESP-NOW YAYIN - Komutlar ----", "---- ESP-NOW BROADCAST - Commands ----");
  say("  yardim       : bu liste", "  help         : this list");
  say("  dur          : yayını durdur", "  pause        : stop broadcasting");
  say("  devam        : yayına devam et", "  resume       : resume broadcasting");
  say("  gonder       : hemen bir yayın gönder", "  send         : send one broadcast now");
  say("  aralik 500   : yayın aralığı (ms)", "  interval 500 : broadcast interval (ms)");
  say("  sifirla      : sayacı sıfırla", "  reset        : reset the counter");
  say("  dil          : English'e geç", "  lang         : switch to Turkish");
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  bool hasValue = space > 0;
  long value = hasValue ? cmd.substring(space + 1).toInt() : 0;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "dur" || word == "durdur" || word == "pause" || word == "stop") {
    sending = false;
    say("Yayın durduruldu (dinlemeye devam ediyor).", "Broadcasting paused (still listening).");
  } else if (word == "devam" || word == "basla" || word == "resume" || word == "start") {
    sending = true;
    say("Yayına devam ediliyor.", "Broadcasting resumed.");
  } else if (word == "gonder" || word == "send") {
    sendCounter();
  } else if ((word == "aralik" || word == "interval") && hasValue) {
    sendIntervalMs = constrain(value, 200L, 10000L);
    rolebot.serialWrite(String(L("Yayın aralığı: ", "Broadcast interval: ")) + sendIntervalMs + " ms");
  } else if (word == "sifirla" || word == "reset") {
    counter = 0;
    say("Sayaç sıfırlandı.", "Counter reset.");
  } else if (word == "dil" || word == "lang" || word == "language") {
    turkish = !turkish;
    say("Dil: Türkçe", "Language: English");
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
  say("ESP-NOW yayın modu başlatılıyor...", "Starting ESP-NOW broadcast mode...");

  rolebot.initESPNow();
  rolebot.startListening(); // Gelen HERHANGİ bir yayını rolebot.receivedData'ya yazar / stores ANY incoming broadcast in rolebot.receivedData

  say("Yayın modu hazır - herkese açığız!", "Broadcast mode ready - open to everyone!");
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // ---- Gönderim: her saniye sayacı yayınla / Sending: broadcast the counter every second ----
  if (sending && now - lastSendMs >= sendIntervalMs) {
    lastSendMs = now;
    sendCounter();
  }

  // ---- Alış: başka bir karttan gelen HERHANGİ bir yayın / Receiving: ANY broadcast from another board ----
  if (rolebot.newData) {
    rolebot.newData = false;
    const char *senderName = "?";
    switch (rolebot.receivedData.deviceType) {
      // Örnek kartı kimlikleri 40-49 (bkz. CodlaiESPNowMessage) / example board IDs 40-49 (see CodlaiESPNowMessage)
      case 40: senderName = "IOTBOT"; break;
      case 41: senderName = "MINIBOT"; break;
      case 42: senderName = "ROLEBOT"; break;
      default: break;
    }
    char line[96];
    snprintf(line, sizeof(line), L("Yayın alındı -> gönderen: %s, değer: %d", "Broadcast received -> from: %s, value: %d"),
             senderName, rolebot.receivedData.axis1);
    rolebot.serialWrite(line);
    // LED'i 50 ms yak (beklemeden) / light the LED for 50 ms (non-blocking)
    rolebot.ledWrite(true);
    ledOn = true;
    ledOffAtMs = now + 50;
  }
  if (ledOn && (int32_t)(now - ledOffAtMs) >= 0) {
    ledOn = false;
    rolebot.ledWrite(false);
  }

  // ---- Seri komutlar / Serial commands ----
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
