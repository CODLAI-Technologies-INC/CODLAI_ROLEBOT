/*
 * TR: İNTERNET SAATİ - Saate Göre Röle. ROLEBOT WiFi'ye bağlanıp saati internetten
 * (NTP) çeker. OTOMATİK modda (açılışta):
 *  - "Saat 19:00 ile 23:30 ARASINDA İSE" Röle 1 (örneğin lamba) açık (ntpTimeIsBetween).
 *  - "Saat 07:00 İSE" Röle 2 (örneğin bahçe pompası) o DAKİKA boyunca açık (ntpTimeIs)
 *    - yani her sabah 1 dakika sulama.
 *  - "Saat 12:00 OLUNCA" LED 3 kez yanıp söner - BİR KEZ (ntpTimeReached).
 *  - Saat her 6 saatte bir GÜNCELLENİR (ntpUpdate).
 * Bu fonksiyonlar editördeki "İnternet saatini kullan / güncelle", "saat ... ise",
 * "saat ... olunca" bloklarının karşılığıdır. Daha gelişmiş bir haftalık program için:
 * ROLEBOT_NTP_Scheduled_Relay_Example.ino
 *  - B1 butonu: OTOMATİK modda basınca MANUEL moda geçer (röleler kapanır). MANUEL modda
 *    kısa basış sıradaki röle durumuna geçer (ikisi kapalı -> Röle 1 -> Röle 2 -> ikisi
 *    açık), 1 sn basılı tutmak OTOMATİK moda döndürür. MANUEL modda mavi LED sürekli yanar.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim       / help        -> komut listesi
 *      oto          / auto        -> otomatik mod (saate göre)
 *      manuel       / manual      -> manuel mod
 *      role1 ac     / relay1 on   -> Röle 1'i aç (manuel moda geçer)
 *      role2 kapat  / relay2 off  -> Röle 2'yi kapat
 *      degistir 1   / toggle 1    -> Röle 1'i tersine çevir
 *      hepsi kapat  / all off     -> iki röleyi de kapat
 *      saat         / time        -> saat ve röle durumları
 *      guncelle     / update      -> saati internetten hemen yeniden al
 *      dil          / lang        -> dili değiştir (Türkçe <-> English)
 *
 * EN: INTERNET TIME - Relays by the Clock. The ROLEBOT connects to WiFi and gets the
 * time from the internet (NTP). In AUTO mode (at startup):
 *  - "IF the time is BETWEEN 19:00 and 23:30" Relay 1 (e.g. a lamp) is on (ntpTimeIsBetween).
 *  - "IF it is 07:00" Relay 2 (e.g. a garden pump) is on for that MINUTE (ntpTimeIs)
 *    - i.e. 1 minute of watering every morning.
 *  - "WHEN it is 12:00" the LED blinks 3 times - ONCE (ntpTimeReached).
 *  - The time is UPDATED every 6 hours (ntpUpdate).
 * These functions are what the editor's "use / update internet time", "if time is ...",
 * "when time is ..." blocks call. For a richer weekly schedule see
 * ROLEBOT_NTP_Scheduled_Relay_Example.ino
 *  - B1 button: in AUTO mode a press switches to MANUAL mode (relays turn off). In MANUAL
 *    mode a short press goes to the next relay state (both off -> Relay 1 -> Relay 2 ->
 *    both on), holding it 1 s goes back to AUTO mode. In MANUAL mode the blue LED stays on.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help         / yardim      -> command list
 *      auto         / oto         -> auto mode (by the clock)
 *      manual       / manuel      -> manual mode
 *      relay1 on    / role1 ac    -> turn Relay 1 on (switches to manual)
 *      relay2 off   / role2 kapat -> turn Relay 2 off
 *      toggle 1     / degistir 1  -> invert Relay 1
 *      all off      / hepsi kapat -> turn both relays off
 *      time         / saat        -> time and relay states
 *      update       / guncelle    -> fetch the time from the internet now
 *      lang         / dil         -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Lamba Röle 1'e, pompa Röle 2'ye (COM + NO; 220V bağlantılarını bir
 * yetişkin yapmalıdır). Aşağıya WiFi adınızı ve şifrenizi yazın.
 * / Lamp on Relay 1, pump on Relay 2 (COM + NO; mains wiring must be done by an adult).
 * Fill in your WiFi name and password below.
 */

#define USE_WIFI
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

const int TIMEZONE_HOURS = 3;                                // Türkiye UTC+3 / Turkey UTC+3
const uint32_t AUTO_UPDATE_MS = 6UL * 60UL * 60UL * 1000UL;  // 6 saatte bir / every 6 hours
const uint32_t LONG_PRESS_MS = 1000;                         // Uzun basış süresi / long-press time

bool manualMode = false;                 // false = OTOMATİK, true = MANUEL / false = AUTO, true = MANUAL
bool relayOn[3] = {false, false, false}; // [1] = Röle 1, [2] = Röle 2 / [1] = Relay 1, [2] = Relay 2
uint32_t lastUpdateMs = 0;
uint32_t lastPrintMs = 0;
int blinkToggles = 0;                    // 12:00 yanıp sönmesi için kalan değişim / toggles left for the 12:00 blink
uint32_t lastBlinkMs = 0;
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
void printStatus() {
  char line[120];
  snprintf(line, sizeof(line), L("%s  [%s]  Röle 1: %s  Röle 2: %s", "%s  [%s]  Relay 1: %s  Relay 2: %s"),
           rolebot.ntpGetTimeString().c_str(), manualMode ? L("MANUEL", "MANUAL") : L("OTOMATİK", "AUTO"),
           onOffText(relayOn[1]), onOffText(relayOn[2]));
  rolebot.serialWrite(line);
}

void printHelp() {
  rolebot.serialWrite(L("---- İNTERNET SAATİ RÖLE - Komutlar ----", "---- INTERNET CLOCK RELAY - Commands ----"));
  rolebot.serialWrite(L("  yardim          : bu liste", "  help            : this list"));
  rolebot.serialWrite(L("  oto             : otomatik mod (saate göre)", "  auto            : auto mode (by the clock)"));
  rolebot.serialWrite(L("  manuel          : manuel mod", "  manual          : manual mode"));
  rolebot.serialWrite(L("  role1 ac/kapat  : Röle 1", "  relay1 on/off   : Relay 1"));
  rolebot.serialWrite(L("  role2 ac/kapat  : Röle 2", "  relay2 on/off   : Relay 2"));
  rolebot.serialWrite(L("  degistir 1 / 2  : röleyi tersine çevir", "  toggle 1 / 2    : invert a relay"));
  rolebot.serialWrite(L("  hepsi kapat     : iki röleyi de kapat", "  all off         : turn both relays off"));
  rolebot.serialWrite(L("  saat            : saat ve röleler", "  time            : time and relays"));
  rolebot.serialWrite(L("  guncelle        : saati yeniden al", "  update          : re-sync the time"));
  rolebot.serialWrite(L("  dil             : English'e geç", "  lang            : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu       : OTOMATİK'te bas = MANUEL", "  B1 button       : press in AUTO = MANUAL"));
  rolebot.serialWrite(L("                    MANUEL'de kısa bas = sıradaki durum, 1 sn bas = OTOMATİK",
                        "                    in MANUAL short press = next state, hold 1 s = AUTO"));
}

void setMode(bool manual) {
  manualMode = manual;
  if (manual) {
    setRelay(1, false); // Manuele geçince röleler kapanır / relays turn off when entering manual
    setRelay(2, false);
  }
  ledState = manual;
  rolebot.ledWrite(ledState);
  rolebot.serialWrite(manual ? L(">> MANUEL mod: röleleri B1 ya da seri komutlarla siz kontrol edin.",
                                 ">> MANUAL mode: you control the relays with B1 or serial commands.")
                             : L(">> OTOMATİK mod: röleler saate göre çalışıyor.",
                                 ">> AUTO mode: the relays follow the clock."));
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
  } else if (word == "saat" || word == "time" || word == "durum" || word == "status") {
    printStatus();
  } else if (word == "guncelle" || word == "update") {
    rolebot.serialWrite(L("Saat güncelleniyor...", "Updating the time..."));
    bool ok = rolebot.ntpUpdate(); // "İnternet saatini güncelle" - en fazla 10 sn / "update internet time" - up to 10 s
    lastUpdateMs = millis();
    rolebot.serialWrite(ok ? String(L("Saat alındı: ", "Time synced: ")) + rolebot.ntpGetDateTimeString()
                           : String(L("Saat alınamadı (WiFi bağlı mı?).", "Could not get the time (is WiFi connected?).")));
  } else if (word == "dil" || word == "lang" || word == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else if (parseRelayCommand(cmd, relay, action)) {
    if (!manualMode) setMode(true); // Röle komutu manuel moda geçirir / a relay command switches to manual
    for (int r = 1; r <= 2; r++) {
      if (relay == 0 || relay == r) setRelay(r, action == 2 ? !relayOn[r] : action == 1);
    }
    printStatus();
  } else {
    rolebot.serialWrite(String(L("Bilinmeyen komut: ", "Unknown command: ")) + cmd + L("  (yardim yazın)", "  (type help)"));
  }
}

// ---------------------------------------------------------------------------
void setup() {
  rolebot.begin();
  setRelay(1, false); // Güvenlik: röleler KAPALI başlar / safety: relays start OFF
  setRelay(2, false);
  rolebot.serialStart(115200);
  rolebot.serialWrite(L("WiFi'ye bağlanıyor...", "Connecting to WiFi..."));
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS);
  if (!rolebot.wifiConnectionControl()) {
    rolebot.serialWrite(L("WiFi YOK - saat gelene kadar röleler kapalı kalacak.", "NO WiFi - relays stay off until the time arrives."));
  }
  // WiFi olmasa da çağırıyoruz: WiFi sonradan bağlanınca saat kendiliğinden gelir.
  // Called even without WiFi: the time arrives by itself once WiFi connects later.
  bool ok = rolebot.ntpBegin(TIMEZONE_HOURS); // "İnternet saatini kullan" / "use internet time"
  rolebot.serialWrite(ok ? String(L("Saat alındı: ", "Time synced: ")) + rolebot.ntpGetDateTimeString()
                         : String(L("Saat henüz yok.", "No time yet.")));
  lastUpdateMs = millis();
  printHelp();
  setMode(false);
}

void loop() {
  uint32_t now = millis();

  // 1) B1 butonu / B1 button
  int b1 = readB1(now);
  if (b1 != 0) {
    if (!manualMode) {
      setMode(true);  // OTOMATİK'te herhangi bir basış -> MANUEL / any press in AUTO -> MANUAL
    } else if (b1 == 2) {
      setMode(false); // MANUEL'de uzun basış -> OTOMATİK / long press in MANUAL -> AUTO
    } else {
      // Kısa basış: sıradaki durum (Röle1 + 2 x Röle2 = 0..3) / short press: next state
      int next = ((relayOn[1] ? 1 : 0) + (relayOn[2] ? 2 : 0) + 1) % 4;
      setRelay(1, next == 1 || next == 3);
      setRelay(2, next == 2 || next == 3);
      printStatus();
    }
  }

  // 2) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);

  // 3) 6 saatte bir saati güncelle / update the time every 6 hours
  if (now - lastUpdateMs >= AUTO_UPDATE_MS) {
    rolebot.ntpUpdate(); // "İnternet saatini güncelle" / "update internet time"
    lastUpdateMs = millis();
  }

  // 4) OTOMATİK: saate göre röleler. Saat henüz gelmediyse bu fonksiyonlar false döner -> röleler güvenle KAPALI.
  // 4) AUTO: relays by the clock. While there is no valid time these return false -> relays safely OFF.
  if (!manualMode) {
    bool r1 = rolebot.ntpTimeIsBetween(19, 0, 23, 30);
    bool r2 = rolebot.ntpTimeIs(7, 0);
    if (r1 != relayOn[1] || r2 != relayOn[2]) {
      setRelay(1, r1);
      setRelay(2, r2);
      printStatus(); // Sadece değişince yaz / print only on change
    }
    // "Saat 12:00 olunca" LED 3 kez yanıp söner (6 değişim, beklemeden)
    // "When it is 12:00" the LED blinks 3 times (6 toggles, non-blocking)
    if (rolebot.ntpTimeReached(12, 0)) {
      blinkToggles = 6;
      rolebot.serialWrite(L("Saat 12:00 - öğle vakti!", "It is 12:00 - noon!"));
    }
    if (blinkToggles > 0 && now - lastBlinkMs >= 200) {
      lastBlinkMs = now;
      blinkToggles--;
      ledState = !ledState;
      rolebot.ledWrite(ledState);
    }
  }

  // 5) 10 saniyede bir saat ve durum / time and status every 10 seconds
  if (now - lastPrintMs >= 10000) {
    lastPrintMs = now;
    printStatus();
  }
}
