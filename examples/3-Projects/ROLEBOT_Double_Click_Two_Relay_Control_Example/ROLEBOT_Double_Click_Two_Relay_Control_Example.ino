/*
 * TR: GERÇEK PROJE - Tek Butonla İki Röle Kontrolü. ROLEBOT'ta tek bir buton
 * var ama iki röle var - bu örnek klasik bir gömülü sistem tekniğinin
 * ("tek tık / çift tık" ayrımı) nasıl yapıldığını gösterir:
 *  - B1'e TEK basıp bırakırsanız Röle 1 açılır/kapanır.
 *  - HIZLI bir şekilde İKİ kez basarsanız (400 ms içinde) Röle 2 açılır/kapanır.
 *  - Mavi LED, rölelerden biri açıkken yanar.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim       / help        -> komut listesi
 *      role1 ac     / relay1 on   -> Röle 1'i aç
 *      role2 kapat  / relay2 off  -> Röle 2'yi kapat
 *      degistir 1   / toggle 1    -> Röle 1'i tersine çevir (tek tık gibi)
 *      degistir 2   / toggle 2    -> Röle 2'yi tersine çevir (çift tık gibi)
 *      hepsi kapat  / all off     -> iki röleyi de kapat
 *      sure 400     / window 400  -> çift tık süresi (150-1000 ms)
 *      durum        / status      -> röle durumları
 *      dil          / lang        -> dili değiştir (Türkçe <-> English)
 *
 * EN: A REAL PROJECT - Controlling Two Relays with One Button. ROLEBOT only has
 * one button but two relays - this example shows a classic embedded-systems
 * technique (telling a "single click" from a "double click"):
 *  - A SINGLE press of B1 toggles Relay 1.
 *  - A FAST double press (within 400 ms) toggles Relay 2.
 *  - The blue LED is on while any relay is on.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help         / yardim       -> command list
 *      relay1 on    / role1 ac     -> turn Relay 1 on
 *      relay2 off   / role2 kapat  -> turn Relay 2 off
 *      toggle 1     / degistir 1   -> invert Relay 1 (like a single click)
 *      toggle 2     / degistir 2   -> invert Relay 2 (like a double click)
 *      all off      / hepsi kapat  -> turn both relays off
 *      window 400   / sure 400     -> double-click window (150-1000 ms)
 *      status       / durum        -> relay states
 *      lang         / dil          -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Yükleri rölelerin COM + NO uçlarına bağlayın. Şebeke gerilimi
 * (220V) ile SADECE bir yetişkin çalışsın. / Wire the loads to the relays' COM + NO
 * terminals. Only an adult should work with mains voltage (220V).
 * ROLEBOT'ta ekran YOK; bilgiler Seri Port'tan verilir. / ROLEBOT has NO screen;
 * feedback is given through the Serial port.
 */

#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

uint32_t doubleClickWindowMs = 400; // Çift tık için en uzun ara / longest gap for a double click

bool relayOn[3] = {false, false, false}; // [1] = Röle 1, [2] = Röle 2 / [1] = Relay 1, [2] = Relay 2

bool lastButtonState = true;     // digitalRead: HIGH = bırakılmış / released
uint32_t lastEdgeMs = 0;         // Parazit süzgeci / debounce
uint32_t lastReleaseMs = 0;
bool waitingForSecondClick = false;

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
// Röleler / Relays
// ---------------------------------------------------------------------------
const char *onOffText(bool on) { return on ? L("AÇIK", "ON") : L("KAPALI", "OFF"); }

void setRelay(int r, bool on, const char *source) {
  relayOn[r] = on;
  if (r == 1) rolebot.Relay1Write(on);
  else rolebot.Relay2Write(on);
  rolebot.ledWrite(relayOn[1] || relayOn[2]); // Bir röle açıksa LED yanar / LED on while any relay is on
  char line[96];
  snprintf(line, sizeof(line), L("%s -> Röle %d: %s", "%s -> Relay %d: %s"), source, r, onOffText(on));
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

// ---------------------------------------------------------------------------
void printHelp() {
  rolebot.serialWrite(L("---- TEK BUTON, İKİ RÖLE - Komutlar ----", "---- ONE BUTTON, TWO RELAYS - Commands ----"));
  rolebot.serialWrite(L("  yardim          : bu liste", "  help            : this list"));
  rolebot.serialWrite(L("  role1 ac/kapat  : Röle 1", "  relay1 on/off   : Relay 1"));
  rolebot.serialWrite(L("  role2 ac/kapat  : Röle 2", "  relay2 on/off   : Relay 2"));
  rolebot.serialWrite(L("  degistir 1 / 2  : röleyi tersine çevir", "  toggle 1 / 2    : invert a relay"));
  rolebot.serialWrite(L("  hepsi kapat     : iki röleyi de kapat", "  all off         : turn both relays off"));
  rolebot.serialWrite(L("  sure 400        : çift tık süresi (ms)", "  window 400      : double-click window (ms)"));
  rolebot.serialWrite(L("  durum           : röle durumları", "  status          : relay states"));
  rolebot.serialWrite(L("  dil             : English'e geç", "  lang            : switch to Turkish"));
  rolebot.serialWrite(L("  B1 tek tık      : Röle 1,  B1 çift tık: Röle 2", "  B1 single click: Relay 1,  B1 double click: Relay 2"));
}

void printStatus() {
  char line[96];
  snprintf(line, sizeof(line), L("Röle 1: %s, Röle 2: %s, çift tık süresi: %lu ms", "Relay 1: %s, Relay 2: %s, double-click window: %lu ms"),
           onOffText(relayOn[1]), onOffText(relayOn[2]), (unsigned long)doubleClickWindowMs);
  rolebot.serialWrite(line);
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  bool hasValue = space > 0;
  int value = hasValue ? cmd.substring(space + 1).toInt() : 0;
  int relay, action;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "durum" || word == "status") {
    printStatus();
  } else if ((word == "sure" || word == "window") && hasValue) {
    doubleClickWindowMs = constrain(value, 150, 1000);
    rolebot.serialWrite(String(L("Çift tık süresi: ", "Double-click window: ")) + doubleClickWindowMs + " ms");
  } else if (word == "dil" || word == "lang" || word == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else if (parseRelayCommand(cmd, relay, action)) {
    for (int r = 1; r <= 2; r++) {
      if (relay != 0 && relay != r) continue;
      setRelay(r, action == 2 ? !relayOn[r] : action == 1, L("Seri komut", "Serial command"));
    }
  } else {
    rolebot.serialWrite(String(L("Bilinmeyen komut: ", "Unknown command: ")) + cmd + L("  (yardim yazın)", "  (type help)"));
  }
}

// ---------------------------------------------------------------------------
void setup() {
  rolebot.begin();
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI başlar / safety: relays start OFF
  rolebot.Relay2Write(false);
  rolebot.playIntro();
  rolebot.serialStart(115200);
  rolebot.serialWrite(L("Hazır: tek tık = Röle 1, çift tık = Röle 2.", "Ready: single click = Relay 1, double click = Relay 2."));
  printHelp();
}

void loop() {
  uint32_t now = millis();
  bool buttonState = rolebot.button1Read(); // false = basılı / pressed

  // Butonun bırakıldığı anı yakala (kenar algıla, 30 ms parazit süzgeci)
  // Edge-detect the release (30 ms debounce)
  if (buttonState != lastButtonState && now - lastEdgeMs >= 30) {
    lastEdgeMs = now;
    if (lastButtonState == false && buttonState == true) {
      if (waitingForSecondClick && (now - lastReleaseMs) <= doubleClickWindowMs) {
        // Çift tık / double click
        waitingForSecondClick = false;
        setRelay(2, !relayOn[2], L("Çift tık", "Double click"));
      } else {
        // İlk tık - ikincisi gelecek mi diye bekle / first click - wait to see if a second one follows
        waitingForSecondClick = true;
        lastReleaseMs = now;
      }
    }
    lastButtonState = buttonState;
  }

  // Bekleme süresi doldu ve ikinci tık gelmediyse: tek tık işlemini uygula
  // The wait window elapsed with no second click: apply the single-click action
  if (waitingForSecondClick && (now - lastReleaseMs) > doubleClickWindowMs) {
    waitingForSecondClick = false;
    setRelay(1, !relayOn[1], L("Tek tık", "Single click"));
  }

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
