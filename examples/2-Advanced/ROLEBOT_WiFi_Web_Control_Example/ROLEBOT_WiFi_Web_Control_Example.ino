/*
 * TR: WEB'DEN RÖLE KONTROLÜ. Telefon/tarayıcıdan (ROLEBOT'un kendi WiFi ağına bağlanarak)
 * iki röleyi ve LED'i açıp kapatabildiğiniz bir uzaktan kontrol paneli. `serverOnRequest()`
 * fonksiyonu kullanılır - `serverCreateLocalPage()`'in aksine, bir adrese (örneğin
 * "/relay1-on") istek geldiğinde GERÇEKTEN kod çalıştırmanıza (bir röleyi tetiklemenize)
 * izin verir. Aynı röleleri Seri Port'tan da kontrol edebilirsiniz; web sayfası durumu
 * her saniye yeniler.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim       / help        -> komut listesi
 *      role1 ac     / relay1 on   -> Röle 1'i aç
 *      role2 kapat  / relay2 off  -> Röle 2'yi kapat
 *      degistir 1   / toggle 1    -> Röle 1'i tersine çevir
 *      hepsi kapat  / all off     -> iki röleyi de kapat
 *      led ac       / led on      -> LED'i aç ("led kapat" / "led off")
 *      durum        / status      -> röle/LED durumu ve panel adresi
 *      dil          / lang        -> dili değiştir (Türkçe <-> English; web durum yazısı da)
 *
 * Kurulum:
 * 1) Bu kodu ROLEBOT'a yükleyin.
 * 2) Telefonunuzun WiFi ayarlarından "CODLAI_ROLEBOT" ağına bağlanın, şifre: 12345678
 * 3) Tarayıcıda http://192.168.4.1/panel adresini açın.
 *
 * EN: RELAY CONTROL FROM THE WEB. A remote-control panel you open from your phone/browser
 * (by joining ROLEBOT's own WiFi network) to turn two relays and the LED on/off. Uses the
 * `serverOnRequest()` function - unlike `serverCreateLocalPage()`, it lets code actually
 * RUN (trigger a relay) when a URL (e.g. "/relay1-on") is requested. You can control the
 * same relays from the Serial port too; the web page refreshes the state every second.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help         / yardim      -> command list
 *      relay1 on    / role1 ac    -> turn Relay 1 on
 *      relay2 off   / role2 kapat -> turn Relay 2 off
 *      toggle 1     / degistir 1  -> invert Relay 1
 *      all off      / hepsi kapat -> turn both relays off
 *      led on       / led ac      -> turn the LED on ("led off" / "led kapat")
 *      status       / durum       -> relay/LED state and the panel address
 *      lang         / dil         -> switch language (Turkish <-> English; web status text too)
 *
 * Setup:
 * 1) Upload this to your ROLEBOT.
 * 2) On your phone, join the "CODLAI_ROLEBOT" WiFi network, password: 12345678
 * 3) Open http://192.168.4.1/panel in a browser.
 *
 * Bağlantı / Wiring: Yükleri rölelerin COM + NO uçlarına bağlayın. Şebeke gerilimi (220V)
 * ile SADECE bir yetişkin çalışsın. / Wire the loads to the relays' COM + NO terminals.
 * Only an adult should work with mains voltage (220V).
 */

#define USE_SERVER
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

#define AP_SSID "CODLAI_ROLEBOT"
#define AP_PASS "12345678"

// Panel sayfasının adresi. NOT: "/" (ana sayfa) adresini kütüphanenin serverStart()'ı
// kendisi kullanır ("CODLAI Server is Running!"), bu yüzden panel "/panel" adresindedir.
// Address of the panel page. NOTE: the library's serverStart() already uses "/" (root)
// for itself ("CODLAI Server is Running!"), so the panel lives at "/panel".
#define PANEL_PAGE "panel"

bool relay1On = false;
bool relay2On = false;
bool ledOn = false;

// NOT: Bu metinler snprintf ile birleştirilir; HTML içinde tek başına '%' kullanmayın.
// NOTE: These texts are combined with snprintf; do not use a lone '%' inside the HTML.
const char WEBPageScript[] PROGMEM = R"rawliteral(
<script>
  function callAction(url) {
    fetch(url).then(() => refreshStatus());
  }
  function refreshStatus() {
    fetch('/status').then(r => r.text()).then(text => {
      document.getElementById('status').innerText = text;
    });
  }
  setInterval(refreshStatus, 1000);
  window.onload = refreshStatus;
</script>
)rawliteral";

const char WEBPageCSS[] PROGMEM = R"rawliteral(
<style>
  body { text-align: center; font-family: Arial, sans-serif; background: #101418; color: #eee; }
  h1 { color: #f6ad55; }
  button { font-size: 18px; padding: 12px 20px; margin: 10px; border-radius: 8px; border: none; }
  .on { background: #38a169; color: white; }
  .off { background: #e53e3e; color: white; }
  #status { white-space: pre-line; font-size: 16px; margin-top: 20px; }
</style>
)rawliteral";

const char WEBPageHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ROLEBOT Kontrol Paneli / Control Panel</title>
  %s
  %s
</head>
<body>
  <h1>ROLEBOT</h1>
  <div>
    <button class="on" onclick="callAction('/relay1-on')">RÖLE 1 AÇ / RELAY 1 ON</button>
    <button class="off" onclick="callAction('/relay1-off')">RÖLE 1 KAPAT / RELAY 1 OFF</button>
  </div>
  <div>
    <button class="on" onclick="callAction('/relay2-on')">RÖLE 2 AÇ / RELAY 2 ON</button>
    <button class="off" onclick="callAction('/relay2-off')">RÖLE 2 KAPAT / RELAY 2 OFF</button>
  </div>
  <div>
    <button class="on" onclick="callAction('/led-on')">LED AÇ / LED ON</button>
    <button class="off" onclick="callAction('/led-off')">LED KAPAT / LED OFF</button>
  </div>
  <pre id="status">...</pre>
</body>
</html>
)rawliteral";

// ---------------------------------------------------------------------------
// Röleler / Relays
// ---------------------------------------------------------------------------
const char *onOffText(bool on) { return on ? L("AÇIK", "ON") : L("KAPALI", "OFF"); }

void setRelay(int r, bool on, const char *source) {
  if (r == 1) { relay1On = on; rolebot.Relay1Write(on); }
  else        { relay2On = on; rolebot.Relay2Write(on); }
  char line[80];
  snprintf(line, sizeof(line), L("%s -> Röle %d: %s", "%s -> Relay %d: %s"), source, r, onOffText(on));
  rolebot.serialWrite(line);
}

void setLed(bool on, const char *source) {
  ledOn = on;
  rolebot.ledWrite(on);
  rolebot.serialWrite(String(source) + " -> LED: " + onOffText(on));
}

String statusText() {
  String s;
  s += L("Röle 1: ", "Relay 1: ");
  s += onOffText(relay1On);
  s += "\n";
  s += L("Röle 2: ", "Relay 2: ");
  s += onOffText(relay2On);
  s += "\n";
  s += "LED: ";
  s += onOffText(ledOn);
  return s;
}

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

void printHelp() {
  rolebot.serialWrite(L("---- WEB RÖLE PANELİ - Komutlar ----", "---- WEB RELAY PANEL - Commands ----"));
  rolebot.serialWrite(L("  yardim          : bu liste", "  help            : this list"));
  rolebot.serialWrite(L("  role1 ac/kapat  : Röle 1", "  relay1 on/off   : Relay 1"));
  rolebot.serialWrite(L("  role2 ac/kapat  : Röle 2", "  relay2 on/off   : Relay 2"));
  rolebot.serialWrite(L("  degistir 1 / 2  : röleyi tersine çevir", "  toggle 1 / 2    : invert a relay"));
  rolebot.serialWrite(L("  hepsi kapat     : iki röleyi de kapat", "  all off         : turn both relays off"));
  rolebot.serialWrite(L("  led ac/kapat    : mavi LED", "  led on/off      : blue LED"));
  rolebot.serialWrite(L("  durum           : durum ve panel adresi", "  status          : state and panel address"));
  rolebot.serialWrite(L("  dil             : English'e geç", "  lang            : switch to Turkish"));
  rolebot.serialWrite(String(L("Panel: \"", "Panel: join \"")) + AP_SSID + L("\" ağına bağlanın (şifre ", "\" (password ") + AP_PASS +
                      L("), sonra http://192.168.4.1/" PANEL_PAGE " adresini açın.", "), then open http://192.168.4.1/" PANEL_PAGE));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  String arg = (space < 0) ? String("") : cmd.substring(space + 1);
  arg.trim();
  int relay, action;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "led" && actionFromWord(arg) >= 0) {
    int a = actionFromWord(arg);
    setLed(a == 2 ? !ledOn : a == 1, L("Seri komut", "Serial command"));
  } else if (word == "durum" || word == "status") {
    String s = statusText();
    s.replace("\n", " | ");
    rolebot.serialWrite(s + L(" | Panel: http://192.168.4.1/" PANEL_PAGE, " | Panel: http://192.168.4.1/" PANEL_PAGE));
  } else if (word == "dil" || word == "lang" || word == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else if (parseRelayCommand(cmd, relay, action)) {
    if (relay == 0 || relay == 1) setRelay(1, action == 2 ? !relay1On : action == 1, L("Seri komut", "Serial command"));
    if (relay == 0 || relay == 2) setRelay(2, action == 2 ? !relay2On : action == 1, L("Seri komut", "Serial command"));
  } else {
    rolebot.serialWrite(String(L("Bilinmeyen komut: ", "Unknown command: ")) + cmd + L("  (yardim yazın)", "  (type help)"));
  }
}

// ---------------------------------------------------------------------------
void setup() {
  rolebot.begin();
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI başlar / safety: relays start OFF
  rolebot.Relay2Write(false);
  rolebot.ledWrite(false);
  rolebot.serialStart(115200);

  rolebot.serverStart("AP", AP_SSID, AP_PASS);
  rolebot.serverCreateLocalPage(PANEL_PAGE, WEBPageScript, WEBPageCSS, WEBPageHTML);

  // Web sayfasındaki butonlar bu adresleri çağırır / the buttons on the web page call these addresses
  rolebot.serverOnRequest("/relay1-on", []() -> String {
    setRelay(1, true, "Web");
    return "RELAY1 ON";
  });
  rolebot.serverOnRequest("/relay1-off", []() -> String {
    setRelay(1, false, "Web");
    return "RELAY1 OFF";
  });
  rolebot.serverOnRequest("/relay2-on", []() -> String {
    setRelay(2, true, "Web");
    return "RELAY2 ON";
  });
  rolebot.serverOnRequest("/relay2-off", []() -> String {
    setRelay(2, false, "Web");
    return "RELAY2 OFF";
  });
  rolebot.serverOnRequest("/led-on", []() -> String {
    setLed(true, "Web");
    return "LED ON";
  });
  rolebot.serverOnRequest("/led-off", []() -> String {
    setLed(false, "Web");
    return "LED OFF";
  });
  rolebot.serverOnRequest("/status", []() -> String {
    return statusText(); // Seçili dilde / in the selected language
  });

  rolebot.serialWrite(L("Kontrol paneli hazır: http://192.168.4.1/" PANEL_PAGE, "Control panel ready: http://192.168.4.1/" PANEL_PAGE));
  printHelp();
}

void loop() {
  rolebot.serverContinue(); // AP modunda DNS yönlendirmesini sürdür / keep DNS redirection running in AP mode

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
