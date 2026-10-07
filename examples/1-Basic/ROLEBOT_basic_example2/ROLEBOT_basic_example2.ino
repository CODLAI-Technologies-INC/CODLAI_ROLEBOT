/*
 * TR: RÖLE TESTİ - Otomatik demo + Manuel kontrol
 *  - Açılışta OTOMATİK mod çalışır: iki röle 2 saniyede bir sırayla değişir
 *    (ikisi kapalı -> Röle 1 -> Röle 2 -> ikisi açık -> ...). Mavi LED yavaşça yanıp söner.
 *  - B1 butonu (karttaki tek buton):
 *      OTOMATİK modda basınca          -> MANUEL moda geçer (röleler kapanır)
 *      MANUEL modda kısa basınca       -> sıradaki röle durumuna geçer
 *                                         (ikisi kapalı -> Röle 1 -> Röle 2 -> ikisi açık)
 *      MANUEL modda 1 sn basılı tutunca -> OTOMATİK moda döner
 *    MANUEL modda mavi LED sürekli yanar.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim       / help        -> komut listesi
 *      oto          / auto        -> otomatik mod
 *      manuel       / manual      -> manuel mod
 *      role1 ac     / relay1 on   -> Röle 1'i aç (manuel moda geçer)
 *      role2 kapat  / relay2 off  -> Röle 2'yi kapat
 *      degistir 1   / toggle 1    -> Röle 1'i tersine çevir
 *      hepsi ac     / all on      -> iki röleyi de aç
 *      hepsi kapat  / all off     -> iki röleyi de kapat
 *      durum        / status      -> rölelerin durumunu yaz
 *      dil          / lang        -> dili değiştir (Türkçe <-> English)
 *
 * EN: RELAY TEST - Automatic demo + Manual control
 *  - At startup AUTO mode runs: the two relays change in turn every 2 seconds
 *    (both off -> Relay 1 -> Relay 2 -> both on -> ...). The blue LED blinks slowly.
 *  - B1 button (the only button on the board):
 *      press in AUTO mode              -> switches to MANUAL mode (relays turn off)
 *      short press in MANUAL mode      -> goes to the next relay state
 *                                         (both off -> Relay 1 -> Relay 2 -> both on)
 *      hold 1 s in MANUAL mode         -> back to AUTO mode
 *    In MANUAL mode the blue LED stays on.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help         / yardim       -> command list
 *      auto         / oto          -> auto mode
 *      manual       / manuel       -> manual mode
 *      relay1 on    / role1 ac     -> turn Relay 1 on (switches to manual)
 *      relay2 off   / role2 kapat  -> turn Relay 2 off
 *      toggle 1     / degistir 1   -> invert Relay 1
 *      all on       / hepsi ac     -> turn both relays on
 *      all off      / hepsi kapat  -> turn both relays off
 *      status       / durum        -> print the relay states
 *      lang         / dil          -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Röleler kartın üzerindedir (Röle 1 = GPIO12, Röle 2 = GPIO13),
 * B1 = GPIO0, mavi LED = GPIO16. Yükü rölenin COM + NO uçlarına bağlayın.
 * Şebeke gerilimi (220V) ile SADECE bir yetişkin çalışsın.
 * / The relays are on the board (Relay 1 = GPIO12, Relay 2 = GPIO13), B1 = GPIO0,
 * blue LED = GPIO16. Wire the load to the relay's COM + NO terminals.
 * Only an adult should work with mains voltage (220V).
 */

#include <ROLEBOT.h> // ROLEBOT kütüphanesi / ROLEBOT library

ROLEBOT rolebot; // ROLEBOT nesnesi / ROLEBOT object

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

const uint32_t DEMO_STEP_MS = 2000;  // Otomatik modda adım süresi / step time in auto mode
const uint32_t LONG_PRESS_MS = 1000; // Uzun basış süresi / long-press time

bool manualMode = false;             // false = OTOMATİK, true = MANUEL / false = AUTO, true = MANUAL
bool relayOn[3] = {false, false, false}; // [1] = Röle 1, [2] = Röle 2 / [1] = Relay 1, [2] = Relay 2
int demoStep = 0;                    // 0 = ikisi kapalı, 1 = Röle 1, 2 = Röle 2, 3 = ikisi açık
uint32_t lastDemoMs = 0;
uint32_t lastLedMs = 0;
bool ledState = false;

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
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
// Röleler / Relays
// ---------------------------------------------------------------------------
const char *onOffText(bool on) { return on ? L("AÇIK", "ON") : L("KAPALI", "OFF"); }

void setRelay(int r, bool on) {
  relayOn[r] = on;
  if (r == 1) rolebot.Relay1Write(on);
  else rolebot.Relay2Write(on);
}

// 0 = ikisi kapalı, 1 = Röle 1, 2 = Röle 2, 3 = ikisi açık / 0 = both off, 1 = Relay 1, 2 = Relay 2, 3 = both on
void applyPattern(int step) {
  setRelay(1, step == 1 || step == 3);
  setRelay(2, step == 2 || step == 3);
}

void printRelays(const char *prefix) {
  char line[96];
  snprintf(line, sizeof(line), L("%sRöle 1: %s, Röle 2: %s", "%sRelay 1: %s, Relay 2: %s"),
           prefix, onOffText(relayOn[1]), onOffText(relayOn[2]));
  rolebot.serialWrite(line);
}

int relayFromWord(const String &w) {
  if (w == "role1" || w == "relay1" || w == "r1" || w == "1") return 1;
  if (w == "role2" || w == "relay2" || w == "r2" || w == "2") return 2;
  if (w == "hepsi" || w == "tumu" || w == "tum" || w == "ikisi" || w == "all" || w == "both") return 0;
  return -1;
}

int actionFromWord(const String &w) {
  if (w == "ac" || w == "acik" || w == "on") return 1;
  if (w == "kapat" || w == "kapa" || w == "kapali" || w == "off") return 0;
  if (w == "degistir" || w == "toggle" || w == "") return 2;
  return -1;
}

// Röle komutunu çözer: "role1 ac", "role 1 ac", "relay2 off", "hepsi kapat", "degistir 1", "toggle 2", "kapat"
// Parses a relay command. relay: 1, 2 or 0 (= both); action: 1 = on, 0 = off, 2 = toggle
bool parseRelayCommand(String c, int &relay, int &action) {
  c.replace("role ", "role");
  c.replace("relay ", "relay");
  int sp = c.indexOf(' ');
  String a = (sp < 0) ? c : c.substring(0, sp);
  String b = (sp < 0) ? String("") : c.substring(sp + 1);
  b.trim();
  // "degistir 1" / "ac hepsi" gibi ters sırayı da kabul et / also accept the reversed order
  if (relayFromWord(a) < 0 && actionFromWord(a) >= 0 && a.length() > 0) {
    String t = a; a = b; b = t;
    if (a.length() == 0) a = "hepsi"; // Sadece "kapat" = hepsini kapat / just "off" = all off
  }
  relay = relayFromWord(a);
  action = actionFromWord(b);
  return relay >= 0 && action >= 0;
}

void applyRelayCommand(int relay, int action) {
  for (int r = 1; r <= 2; r++) {
    if (relay != 0 && relay != r) continue;
    setRelay(r, action == 2 ? !relayOn[r] : action == 1);
  }
}

// ---------------------------------------------------------------------------
// Mesajlar ve modlar / Messages and modes
// ---------------------------------------------------------------------------
void printHelp() {
  rolebot.serialWrite(L("---- RÖLE TESTİ - Komutlar ----", "---- RELAY TEST - Commands ----"));
  rolebot.serialWrite(L("  yardim          : bu liste", "  help            : this list"));
  rolebot.serialWrite(L("  oto             : otomatik mod (demo)", "  auto            : auto mode (demo)"));
  rolebot.serialWrite(L("  manuel          : manuel mod", "  manual          : manual mode"));
  rolebot.serialWrite(L("  role1 ac        : Röle 1'i aç", "  relay1 on       : turn Relay 1 on"));
  rolebot.serialWrite(L("  role2 kapat     : Röle 2'yi kapat", "  relay2 off      : turn Relay 2 off"));
  rolebot.serialWrite(L("  degistir 1      : Röle 1'i tersine çevir", "  toggle 1        : invert Relay 1"));
  rolebot.serialWrite(L("  hepsi ac/kapat  : iki röle birden", "  all on/off      : both relays"));
  rolebot.serialWrite(L("  durum           : röle durumları", "  status          : relay states"));
  rolebot.serialWrite(L("  dil             : English'e geç", "  lang            : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu       : OTOMATİK'te bas = MANUEL", "  B1 button       : press in AUTO = MANUAL"));
  rolebot.serialWrite(L("                    MANUEL'de kısa bas = sıradaki durum, 1 sn bas = OTOMATİK",
                        "                    in MANUAL short press = next state, hold 1 s = AUTO"));
}

void setMode(bool manual) {
  manualMode = manual;
  demoStep = 0;
  applyPattern(0); // Mod değişince röleler güvenle kapanır / relays turn off safely on mode change
  lastDemoMs = millis();
  rolebot.serialWrite(manual ? L(">> MANUEL mod: B1 kısa bas = sıradaki durum, 1 sn basılı tut = otomatik. (role1 ac, hepsi kapat ...)",
                                 ">> MANUAL mode: B1 short press = next state, hold 1 s = auto. (relay1 on, all off ...)")
                             : L(">> OTOMATİK mod: röleler kendi kendine sırayla değişiyor.",
                                 ">> AUTO mode: the relays change in turn by themselves."));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  int relay, action;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "oto" || word == "otomatik" || word == "auto") {
    setMode(false);
  } else if (word == "manuel" || word == "manual") {
    setMode(true);
  } else if (word == "durum" || word == "status") {
    rolebot.serialWrite(String(L("Mod: ", "Mode: ")) + (manualMode ? L("MANUEL", "MANUAL") : L("OTOMATİK", "AUTO")));
    printRelays("");
  } else if (word == "dil" || word == "lang" || word == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else if (parseRelayCommand(cmd, relay, action)) {
    if (!manualMode) setMode(true); // Röle komutu manuel moda geçirir / a relay command switches to manual
    applyRelayCommand(relay, action);
    printRelays(L("Manuel -> ", "Manual -> "));
  } else {
    rolebot.serialWrite(String(L("Bilinmeyen komut: ", "Unknown command: ")) + cmd + L("  (yardim yazın)", "  (type help)"));
  }
}

// ---------------------------------------------------------------------------
void setup() {
  rolebot.begin();             // ROLEBOT başlatılıyor / Initialize ROLEBOT
  applyPattern(0);             // Güvenlik: röleler KAPALI başlar / safety: relays start OFF
  rolebot.playIntro();         // LED 3 kez yanıp söner / LED blinks 3 times
  rolebot.serialStart(115200); // Seri haberleşme / Serial communication
  rolebot.serialWrite(L("ROLEBOT röle testi başladı.", "ROLEBOT relay test started."));
  printHelp();
  setMode(false);
}

void loop() {
  uint32_t now = millis();

  // 1) B1 butonu / B1 button
  int b1 = readB1(now);
  if (b1 != 0) {
    if (!manualMode) {
      setMode(true);                 // OTOMATİK'te herhangi bir basış -> MANUEL / any press in AUTO -> MANUAL
    } else if (b1 == 2) {
      setMode(false);                // MANUEL'de uzun basış -> OTOMATİK / long press in MANUAL -> AUTO
    } else {
      // Kısa basış: sıradaki durum. Şu anki durum = Röle1 + 2 x Röle2 (0..3)
      // Short press: next state. Current state = Relay1 + 2 x Relay2 (0..3)
      int current = (relayOn[1] ? 1 : 0) + (relayOn[2] ? 2 : 0);
      applyPattern((current + 1) % 4);
      printRelays(L("B1 -> ", "B1 -> "));
    }
  }

  // 2) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);

  // 3) Otomatik demo (2 sn'de bir adım, loop hiç bloklanmaz) / Auto demo (one step every 2 s, never blocks)
  if (!manualMode && now - lastDemoMs >= DEMO_STEP_MS) {
    lastDemoMs = now;
    demoStep = (demoStep + 1) % 4;
    applyPattern(demoStep);
    printRelays(L("Otomatik -> ", "Auto -> "));
  }

  // 4) LED: OTOMATİK = yavaş yanıp söner, MANUEL = sürekli yanar / LED: AUTO = slow blink, MANUAL = steady on
  if (manualMode) {
    if (!ledState) { ledState = true; rolebot.ledWrite(true); }
  } else if (now - lastLedMs >= 500) {
    lastLedMs = now;
    ledState = !ledState;
    rolebot.ledWrite(ledState);
  }
}
