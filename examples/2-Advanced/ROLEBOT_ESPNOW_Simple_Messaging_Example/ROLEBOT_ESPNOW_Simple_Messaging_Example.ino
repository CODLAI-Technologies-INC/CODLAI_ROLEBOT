/*
 * TR: ÇOCUKLAR İÇİN BASİT KABLOSUZ MESAJLAŞMA. Bir sayı mesajı alındığında (örneğin
 * başka bir kart bir sensör değeri gönderdiğinde) değer belirli bir eşiği aşarsa
 * Röle 1'i tetikler - "kablosuz uzaktan komut" fikrinin en basit hali. Aynı zamanda
 * her aldığı metin mesajını Seri Port'a yazdırır. Seri porttan siz de metin ve sayı
 * gönderebilirsiniz (iki ROLEBOT ile birbirinize mesaj atın!).
 *  - OTOMATİK mod (açılışta): gelen sayı eşiğin ÜSTÜNDEYSE Röle 1 açık, değilse kapalı.
 *  - B1 butonu: OTOMATİK modda basınca MANUEL moda geçer. MANUEL modda kısa basış
 *    Röle 1'i açar/kapatır, 1 sn basılı tutmak OTOMATİK moda döndürür.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim             / help              -> komut listesi
 *      metin Merhaba      / text Hello        -> herkese metin mesajı gönder
 *      sayi 75            / number 75         -> herkese "sayi" adlı bir sayı gönder
 *      sayi isik 75       / number light 75   -> herkese "isik" adlı bir sayı gönder
 *      oto                / auto              -> otomatik mod (gelen sayıya göre)
 *      manuel             / manual            -> manuel mod
 *      ac / kapat         / on / off          -> Röle 1 (manuel moda geçer)
 *      esik 50            / threshold 50      -> tetikleme eşiği
 *      durum              / status            -> son mesaj ve röle durumu
 *      dil                / lang              -> dili değiştir (Türkçe <-> English)
 *
 * EN: SIMPLE WIRELESS MESSAGING FOR KIDS. When a number message is received (e.g.
 * another board sends a sensor value) and it crosses a threshold, it triggers Relay 1 -
 * the simplest form of a "wireless remote command" idea. It also prints every text
 * message it receives to the Serial port. You can send text and numbers from the Serial
 * port too (message each other with two ROLEBOTs!).
 *  - AUTO mode (at startup): Relay 1 is on when the received number is ABOVE the threshold.
 *  - B1 button: in AUTO mode a press switches to MANUAL mode. In MANUAL mode a short press
 *    turns Relay 1 on/off, holding it 1 s goes back to AUTO mode.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help               / yardim            -> command list
 *      text Hello         / metin Merhaba     -> send a text message to everyone
 *      number 75          / sayi 75           -> send a number named "sayi" to everyone
 *      number light 75    / sayi isik 75      -> send a number named "light" to everyone
 *      auto               / oto               -> auto mode (by the received number)
 *      manual             / manuel            -> manual mode
 *      on / off           / ac / kapat        -> Relay 1 (switches to manual)
 *      threshold 50       / esik 50           -> trigger threshold
 *      status             / durum             -> last message and relay state
 *      lang               / dil               -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez; istersen Röle 1'e (COM + NO) bir yük bağlayın.
 * Diğer kart AYNI ESP-NOW kanalını (1) kullanmalı. / No wiring needed; optionally wire a
 * load to Relay 1 (COM + NO). The other board must use the SAME ESP-NOW channel (1).
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

float numberThreshold = 50.0f;       // Bu değerin ÜSTÜ röleyi açar / values ABOVE this turn the relay on
const uint32_t LONG_PRESS_MS = 1000; // Uzun basış süresi / long-press time

bool manualMode = false; // false = OTOMATİK, true = MANUEL / false = AUTO, true = MANUAL
bool relayOn = false;
bool haveNumber = false;
String lastName = "";
float lastValue = 0;

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
String rawCommand; // Komutun orijinal hali (mesaj metni büyük/küçük harfiyle gitsin diye) / original text (keeps letter case for messages)
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "AÇ" -> "ac"
// Lower-cases and simplifies Turkish letters: "AÇ" -> "ac"
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
// B1 butonu (GPIO0, basılıyken LOW) / B1 button (GPIO0, LOW while pressed)
// ---------------------------------------------------------------------------
bool b1Down = false;
bool b1LongDone = false;
uint32_t b1DownMs = 0;

// 0 = olay yok, 1 = kısa basış (bırakınca), 2 = uzun basış (1 sn basılı tutunca)
// 0 = no event, 1 = short press (on release), 2 = long press (after holding 1 s)
int readB1(uint32_t now) {
  bool down = !rolebot.button1Read();
  int event = 0;
  if (down && !b1Down) { b1DownMs = now; b1LongDone = false; }
  if (down && !b1LongDone && now - b1DownMs >= LONG_PRESS_MS) { b1LongDone = true; event = 2; }
  if (!down && b1Down && !b1LongDone && now - b1DownMs >= 30) event = 1; // 30 ms parazit süzgeci / debounce
  b1Down = down;
  return event;
}

// ---------------------------------------------------------------------------
void setRelay(bool on, const char *reason) {
  if (on == relayOn) return;
  relayOn = on;
  rolebot.Relay1Write(relayOn);
  rolebot.ledWrite(relayOn);
  rolebot.serialWrite(String(L("Röle 1: ", "Relay 1: ")) + (relayOn ? L("AÇIK", "ON") : L("KAPALI", "OFF")) + "  (" + reason + ")");
}

void applyAuto() {
  if (manualMode || !haveNumber) return;
  setRelay(lastValue > numberThreshold, L("gelen sayı", "received number"));
}

void printHelp() {
  rolebot.serialWrite(L("---- BASİT MESAJLAŞMA - Komutlar ----", "---- SIMPLE MESSAGING - Commands ----"));
  rolebot.serialWrite(L("  yardim          : bu liste", "  help            : this list"));
  rolebot.serialWrite(L("  metin Merhaba   : metin mesajı gönder", "  text Hello      : send a text message"));
  rolebot.serialWrite(L("  sayi 75         : \"sayi\" adlı sayı gönder", "  number 75       : send a number named \"sayi\""));
  rolebot.serialWrite(L("  sayi isik 75    : \"isik\" adlı sayı gönder", "  number light 75 : send a number named \"light\""));
  rolebot.serialWrite(L("  oto / manuel    : otomatik / manuel mod", "  auto / manual   : auto / manual mode"));
  rolebot.serialWrite(L("  ac / kapat      : Röle 1", "  on / off        : Relay 1"));
  rolebot.serialWrite(L("  esik 50         : tetikleme eşiği", "  threshold 50    : trigger threshold"));
  rolebot.serialWrite(L("  durum           : son mesaj ve röle", "  status          : last message and relay"));
  rolebot.serialWrite(L("  dil             : English'e geç", "  lang            : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu       : OTOMATİK'te bas = MANUEL; MANUEL'de kısa bas = Röle 1, 1 sn bas = OTOMATİK",
                        "  B1 button       : press in AUTO = MANUAL; in MANUAL short press = Relay 1, hold 1 s = AUTO"));
}

void setMode(bool manual) {
  manualMode = manual;
  rolebot.serialWrite(manual ? L(">> MANUEL mod: Röle 1'i B1 ya da \"ac\" / \"kapat\" ile siz kontrol edin.",
                                 ">> MANUAL mode: you control Relay 1 with B1 or \"on\" / \"off\".")
                             : L(">> OTOMATİK mod: Röle 1 gelen sayıya göre çalışıyor.",
                                 ">> AUTO mode: Relay 1 follows the received number."));
  applyAuto();
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  String rest = (space < 0) ? String("") : cmd.substring(space + 1);
  rest.trim();
  bool hasValue = rest.length() > 0;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if ((word == "metin" || word == "text" || word == "mesaj" || word == "msg") && hasValue) {
    // Orijinal yazımı gönder (büyük harfler korunur) / send the original spelling (keeps capitals)
    int rs = rawCommand.indexOf(' ');
    String text = rawCommand.substring(rs + 1);
    text.trim();
    rolebot.espNowSendText(text); // En fazla 31 karakter / at most 31 characters
    rolebot.serialWrite(String(L("Metin gönderildi: ", "Text sent: ")) + text);
  } else if ((word == "sayi" || word == "number") && hasValue) {
    // "sayi 75" ya da "sayi isik 75" / "number 75" or "number light 75"
    int sp2 = rest.indexOf(' ');
    String name = (sp2 < 0) ? String("sayi") : rest.substring(0, sp2);
    float value = (sp2 < 0) ? rest.toFloat() : rest.substring(sp2 + 1).toFloat();
    rolebot.espNowSendNumber(name, value);
    rolebot.serialWrite(String(L("Sayı gönderildi: ", "Number sent: ")) + name + " = " + String(value));
  } else if (word == "oto" || word == "otomatik" || word == "auto") {
    setMode(false);
  } else if (word == "manuel" || word == "manual") {
    setMode(true);
  } else if (word == "ac" || word == "on" || cmd == "role1 ac" || cmd == "relay1 on") {
    if (!manualMode) setMode(true);
    setRelay(true, L("seri komut", "serial command"));
  } else if (word == "kapat" || word == "off" || cmd == "role1 kapat" || cmd == "relay1 off") {
    if (!manualMode) setMode(true);
    setRelay(false, L("seri komut", "serial command"));
  } else if ((word == "esik" || word == "threshold") && hasValue) {
    numberThreshold = rest.toFloat();
    rolebot.serialWrite(String(L("Tetikleme eşiği: ", "Trigger threshold: ")) + String(numberThreshold));
    applyAuto();
  } else if (word == "durum" || word == "status") {
    String s = String(L("Mod: ", "Mode: ")) + (manualMode ? L("MANUEL", "MANUAL") : L("OTOMATİK", "AUTO")) +
               L(" | Röle 1: ", " | Relay 1: ") + (relayOn ? L("AÇIK", "ON") : L("KAPALI", "OFF")) +
               L(" | Eşik: ", " | Threshold: ") + String(numberThreshold);
    if (haveNumber) s += String(L(" | Son sayı: ", " | Last number: ")) + lastName + " = " + String(lastValue);
    rolebot.serialWrite(s);
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
  rolebot.begin();            // Röle ve LED pinlerini hazırlar / prepares the relay and LED pins
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI başlar / safety: relays start OFF
  rolebot.Relay2Write(false);
  rolebot.serialStart(115200);
  rolebot.espNowBegin(1); // Kanal 1 - gönderen diğer kartla AYNI kanal olmalı / channel 1 - must match the other board
  rolebot.serialWrite(L("Basit ESP-NOW mesajlaşma hazır. Mesaj bekleniyor...", "Simple ESP-NOW messaging ready. Waiting for a message..."));
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // 1) Gelen mesaj / Incoming message
  if (rolebot.espNowAvailable()) {
    String text = rolebot.espNowReadText();
    if (text.length() > 0) {
      rolebot.serialWrite(String(L("Metin alındı: ", "Text received: ")) + text);
    }

    String name = rolebot.espNowReadName();
    float value = rolebot.espNowReadNumber();
    if (name.length() > 0) {
      rolebot.serialWrite(String(L("Sayı alındı: ", "Number received: ")) + name + " = " + String(value));
      lastName = name;
      lastValue = value;
      haveNumber = true;
      applyAuto();
    }
  }

  // 2) B1 butonu / B1 button
  int b1 = readB1(now);
  if (b1 != 0) {
    if (!manualMode) setMode(true);                           // OTOMATİK'te bas -> MANUEL / press in AUTO -> MANUAL
    else if (b1 == 2) setMode(false);                         // 1 sn bas -> OTOMATİK / hold 1 s -> AUTO
    else setRelay(!relayOn, L("B1 butonu", "B1 button"));     // Kısa bas -> Röle 1 / short press -> Relay 1
  }

  // 3) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
