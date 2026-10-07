/*
 * TR: GERÇEK PROJE - Kablosuz Röle Alıcısı. Bu ROLEBOT, IOTBOT kumanda panelinin ESP-NOW
 * ile yayınladığı komutları dinler: "role1" -> Röle 1, "role2" -> Röle 2, "led" -> mavi LED.
 * B1 butonu ile Röle 1'i elle de açıp kapatabilirsiniz - panel kapalı olsa bile çalışır.
 * Seri porttan da kontrol edebilirsiniz. Kural basit: "son komut kazanır" (panelde bir
 * değişiklik olunca panel, buradaki butona basılınca buton, seri komut yazılınca seri port).
 * Önce IOTBOT'a IOTBOT_ESPNOW_Remote_Control_Panel_Example.ino dosyasını yükleyin; isterseniz
 * bir MINIBOT'a da MINIBOT_ESPNOW_Remote_Servo_Receiver_Example.ino dosyasını yükleyin.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim       / help        -> komut listesi
 *      role1 ac     / relay1 on   -> Röle 1'i aç
 *      role2 kapat  / relay2 off  -> Röle 2'yi kapat
 *      degistir 1   / toggle 1    -> Röle 1'i tersine çevir
 *      hepsi kapat  / all off     -> iki röleyi de kapat
 *      led ac       / led on      -> mavi LED'i aç ("led kapat" / "led off")
 *      durum        / status      -> röle ve LED durumları
 *      dil          / lang        -> dili değiştir (Türkçe <-> English)
 *
 * EN: A REAL PROJECT - Wireless Relay Receiver. This ROLEBOT listens to the commands the
 * IOTBOT control panel broadcasts over ESP-NOW: "role1" -> Relay 1, "role2" -> Relay 2,
 * "led" -> blue LED. The B1 button also toggles Relay 1 by hand - it works even when the
 * panel is off. You can control it from the Serial port too. The rule is simple: "the last
 * command wins" (the panel when something changes there, the button when it is pressed
 * here, the Serial port when a command is typed). First upload
 * IOTBOT_ESPNOW_Remote_Control_Panel_Example.ino to an IOTBOT; optionally upload
 * MINIBOT_ESPNOW_Remote_Servo_Receiver_Example.ino to a MINIBOT.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help         / yardim      -> command list
 *      relay1 on    / role1 ac    -> turn Relay 1 on
 *      relay2 off   / role2 kapat -> turn Relay 2 off
 *      toggle 1     / degistir 1  -> invert Relay 1
 *      all off      / hepsi kapat -> turn both relays off
 *      led on       / led ac      -> turn the blue LED on ("led off" / "led kapat")
 *      status       / durum       -> relay and LED states
 *      lang         / dil         -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Yükleri (lamba, fan...) rölelerin COM + NO uçlarına bağlayın: röle
 * çekince yük çalışır. Şebeke gerilimi (220V) ile SADECE bir yetişkin çalışsın. / Wire the
 * loads (lamp, fan...) to the relays' COM + NO terminals: the load runs when the relay pulls
 * in. Only an adult should work with mains voltage (220V).
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

namespace {
  constexpr int kEspNowChannel = 1;   // IOTBOT ile AYNI kanal / SAME channel as the IOTBOT

  bool relay1 = false, relay2 = false, led = false;
  // Panel her 2 sn'de durumu TEKRAR gönderir. Sadece panel tarafında bir DEĞİŞİKLİK
  // olunca uyguluyoruz; yoksa tekrar mesajı butonla yaptığınız değişikliği hemen geri
  // alırdı. (-1 = panelden henüz bir şey gelmedi)
  // The panel RESENDS its state every 2 s. We only act when the panel's value CHANGES;
  // otherwise a repeat would instantly undo what you did with the button.
  // (-1 = nothing received from the panel yet)
  int panelRelay1 = -1, panelRelay2 = -1, panelLed = -1;
  bool lastButton = true;             // HIGH = bırakılmış / released
  uint32_t lastPressMs = 0;

  const char *onOff(bool on) { return on ? L("AÇIK", "ON") : L("KAPALI", "OFF"); }

  void report(const char *what, bool on, const char *source) {
    rolebot.serialWrite(String(what) + ": " + onOff(on) + "  (" + source + ")");
  }

  // Panelden gelen değeri, sadece panelin son değerinden FARKLIYSA uygular.
  // Applies the panel's value only if it DIFFERS from the panel's last value.
  bool panelChanged(int &lastPanel, float value, bool &state) {
    int v = value > 0.5f ? 1 : 0;
    if (v == lastPanel) return false;
    lastPanel = v;
    state = v;
    return true;
  }

  void setRelay(int r, bool on, const char *source) {
    if (r == 1) { relay1 = on; rolebot.Relay1Write(on); report(L("Röle 1", "Relay 1"), on, source); }
    else        { relay2 = on; rolebot.Relay2Write(on); report(L("Röle 2", "Relay 2"), on, source); }
  }
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

// ---------------------------------------------------------------------------
// Röle komutları / Relay commands
// ---------------------------------------------------------------------------
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
  rolebot.serialWrite(L("---- KABLOSUZ RÖLE ALICISI - Komutlar ----", "---- WIRELESS RELAY RECEIVER - Commands ----"));
  rolebot.serialWrite(L("  yardim          : bu liste", "  help            : this list"));
  rolebot.serialWrite(L("  role1 ac/kapat  : Röle 1", "  relay1 on/off   : Relay 1"));
  rolebot.serialWrite(L("  role2 ac/kapat  : Röle 2", "  relay2 on/off   : Relay 2"));
  rolebot.serialWrite(L("  degistir 1 / 2  : röleyi tersine çevir", "  toggle 1 / 2    : invert a relay"));
  rolebot.serialWrite(L("  hepsi kapat     : iki röleyi de kapat", "  all off         : turn both relays off"));
  rolebot.serialWrite(L("  led ac/kapat    : mavi LED", "  led on/off      : blue LED"));
  rolebot.serialWrite(L("  durum           : röle ve LED durumları", "  status          : relay and LED states"));
  rolebot.serialWrite(L("  dil             : English'e geç", "  lang            : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu       : Röle 1 aç/kapat", "  B1 button       : Relay 1 on/off"));
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
    led = (a == 2) ? !led : (a == 1);
    rolebot.ledWrite(led);
    report("LED", led, L("seri komut", "serial command"));
  } else if (word == "durum" || word == "status") {
    rolebot.serialWrite(String(L("Röle 1: ", "Relay 1: ")) + onOff(relay1) + L(" | Röle 2: ", " | Relay 2: ") + onOff(relay2) +
                        " | LED: " + onOff(led));
  } else if (word == "dil" || word == "lang" || word == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else if (parseRelayCommand(cmd, relay, action)) {
    if (relay == 0 || relay == 1) setRelay(1, action == 2 ? !relay1 : action == 1, L("seri komut", "serial command"));
    if (relay == 0 || relay == 2) setRelay(2, action == 2 ? !relay2 : action == 1, L("seri komut", "serial command"));
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
  rolebot.espNowBegin(kEspNowChannel);
  rolebot.serialWrite(L("Röle alıcısı hazır. Panel komutu bekleniyor (B1 = Röle 1 elle).",
                        "Relay receiver ready. Waiting for the panel (B1 = Relay 1 by hand)."));
  printHelp();
}

void loop() {
  // 1) Gelen mesaj. NOT: espNowReadName() mesajı "okundu" işaretler; bu yüzden sayıyı
  // ÖNCE receivedData.value'dan alıyoruz, adı SONRA okuyoruz.
  // 1) Incoming message. NOTE: espNowReadName() marks the message as read, so we take
  // the number FIRST from receivedData.value and read the name AFTER.
  if (rolebot.espNowAvailable()) {
    rolebot.espNowReadText();                    // Metin mesajıysa at / drop it if it is a text message
    float value = rolebot.receivedData.value;
    String name = rolebot.espNowReadName();

    if (name == "role1" && panelChanged(panelRelay1, value, relay1)) {
      rolebot.Relay1Write(relay1);
      report(L("Röle 1", "Relay 1"), relay1, "panel");
    } else if (name == "role2" && panelChanged(panelRelay2, value, relay2)) {
      rolebot.Relay2Write(relay2);
      report(L("Röle 2", "Relay 2"), relay2, "panel");
    } else if (name == "led" && panelChanged(panelLed, value, led)) {
      rolebot.ledWrite(led);
      report("LED", led, "panel");
    }
    // "servo" MINIBOT içindir, burada yok sayılır. / "servo" is for the MINIBOT and is ignored here.
  }

  // 2) B1: Röle 1'i elle aç/kapa (panel olmadan da çalışır).
  // 2) B1: toggle Relay 1 by hand (works without the panel too).
  bool button = rolebot.button1Read(); // false = basılı / pressed
  if (lastButton && !button && millis() - lastPressMs > 250) { // 250 ms parazit süzgeci / debounce
    lastPressMs = millis();
    setRelay(1, !relay1, L("B1 butonu", "B1 button"));
  }
  lastButton = button;

  // 3) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
