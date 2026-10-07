/*
 * TR: IFTTT WEBHOOK. ROLEBOT açıldığında ve B1 butonuna basıldığında bir IFTTT Webhook
 * olayını tetikler; IFTTT de bunu istediğiniz bir servise iletir (Google E-Tablolar, Gmail,
 * Discord, akıllı lambalar...). Geri bildirim olarak LED ve Röle 1 yarım saniye açılır.
 * Kurulum özeti:
 *  1. IFTTT'de Webhooks -> "Receive a web request" tetikleyicili bir applet oluşturun ve
 *     Event Name'i aşağıdaki iftttEventName ile aynı yapın.
 *  2. İstediğiniz eylem servisini seçip applet'i bitirin.
 *  3. https://ifttt.com/maker_webhooks (Documentation) sayfasındaki anahtarı
 *     iftttWebhookKey'e yapıştırın.
 *  4. JSON verisi standart value1/2/3 alanlarını kullanır; bunları eylemde kullanabilirsiniz.
 *  - USE_IFTTT tanımlanınca kütüphane WiFi'yi kendisi açar; "#define USE_WIFI" gerekmez.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim  / help    -> komut listesi
 *      gonder  / send    -> olayı şimdi tetikle (B1 ile aynı)
 *      durum   / status  -> WiFi durumu ve gönderilen olay sayısı
 *      dil     / lang    -> dili değiştir (Türkçe <-> English)
 *
 * EN: IFTTT WEBHOOK. Triggers an IFTTT Webhook event when the ROLEBOT boots and when the
 * B1 button is pressed; IFTTT then forwards it to any service you like (Google Sheets,
 * Gmail, Discord, smart lights...). As feedback the LED and Relay 1 turn on for half a
 * second. Setup summary:
 *  1. Create an IFTTT applet that uses Webhooks -> "Receive a web request" and set the
 *     Event Name to match iftttEventName below.
 *  2. Select your desired action service and finish the applet.
 *  3. Grab the key from https://ifttt.com/maker_webhooks (Documentation tab) and paste it
 *     into iftttWebhookKey.
 *  4. The JSON payload uses the standard value1/2/3 fields that you can reuse in the action.
 *  - With USE_IFTTT the library enables WiFi automatically; "#define USE_WIFI" is not needed.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help    / yardim  -> command list
 *      send    / gonder  -> trigger the event now (same as B1)
 *      status  / durum   -> WiFi state and number of events sent
 *      lang    / dil     -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez; istersen Röle 1'e (COM + NO) küçük bir yük bağlayın.
 * / No wiring needed; optionally wire a small load to Relay 1 (COM + NO).
 */

#define USE_IFTTT
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";

String iftttEventName = "YOUR_EVENT_NAME";
String iftttWebhookKey = "YOUR_IFTTT_KEY";

const unsigned long triggerInterval = 4000; // İki olay arası en az 4 sn / at least 4 s between events
unsigned long lastTrigger = 0;
uint32_t eventCount = 0;
bool feedbackOn = false;     // LED + Röle 1 geri bildirimi açık mı? / LED + Relay 1 feedback on?
uint32_t feedbackOffAtMs = 0;
bool lastButton = true;      // HIGH = bırakılmış / released
uint32_t lastEdgeMs = 0;     // Parazit süzgeci / debounce

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
void triggerButtonEvent(const char *source) {
  unsigned long now = millis();
  if (eventCount > 0 && now - lastTrigger < triggerInterval) {
    rolebot.serialWrite(L("Biraz bekleyin: iki olay arası en az 4 saniye.", "Please wait: at least 4 seconds between events."));
    return;
  }
  lastTrigger = now;
  eventCount++;

  // Geri bildirim: LED + Röle 1 yarım saniye açık (beklemeden kapanır)
  // Feedback: LED + Relay 1 on for half a second (turned off without waiting)
  rolebot.ledWrite(true);
  rolebot.Relay1Write(true);
  feedbackOn = true;

  rolebot.serialWrite(String(L("IFTTT olayı gönderiliyor (", "Sending the IFTTT event (")) + source + ")...");
  String payload = "{\"value1\":\"ROLEBOT Button\",\"value2\":\"Pressed\",\"value3\":\"Relay1 ON\"}";
  bool ok = rolebot.triggerIFTTTEvent(iftttEventName, iftttWebhookKey, payload); // Birkaç saniye sürebilir / may take a few seconds
  rolebot.serialWrite(ok ? L("[IFTTT] Buton olayı iletildi", "[IFTTT] Button event delivered")
                         : L("[IFTTT] Buton olayı iletilemedi", "[IFTTT] Button event failed"));
  feedbackOffAtMs = millis() + 500;
}

void printHelp() {
  rolebot.serialWrite(L("---- IFTTT WEBHOOK - Komutlar ----", "---- IFTTT WEBHOOK - Commands ----"));
  rolebot.serialWrite(L("  yardim  : bu liste", "  help    : this list"));
  rolebot.serialWrite(L("  gonder  : olayı şimdi tetikle", "  send    : trigger the event now"));
  rolebot.serialWrite(L("  durum   : WiFi ve olay sayısı", "  status  : WiFi and event count"));
  rolebot.serialWrite(L("  dil     : English'e geç", "  lang    : switch to Turkish"));
  rolebot.serialWrite(L("  B1      : olayı tetikle", "  B1      : trigger the event"));
}

void handleCommand(const String &cmd) {
  if (cmd == "yardim" || cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "gonder" || cmd == "send" || cmd == "tetikle" || cmd == "trigger") {
    triggerButtonEvent(L("seri komut", "serial command"));
  } else if (cmd == "durum" || cmd == "status") {
    rolebot.serialWrite(String("WiFi: ") + (WiFi.status() == WL_CONNECTED ? L("BAĞLI", "CONNECTED") : L("BAĞLI DEĞİL", "NOT CONNECTED")) +
                        L(" | Gönderilen buton olayı: ", " | Button events sent: ") + eventCount);
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
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI başlar / safety: relays start OFF
  rolebot.Relay2Write(false);
  rolebot.playIntro();

  rolebot.serialWrite(L("ROLEBOT IFTTT Webhook Örneği", "ROLEBOT IFTTT Webhook Example"));
  rolebot.serialWrite(L("WiFi'ye bağlanılıyor...", "Connecting to WiFi..."));
  rolebot.wifiStartAndConnect(ssid, password);

  if (rolebot.wifiConnectionControl()) {
    rolebot.serialWrite(L("WiFi bağlandı. Açılış bildirimi gönderiliyor...", "WiFi connected. Sending boot notification..."));
    String payload = "{\"value1\":\"ROLEBOT\",\"value2\":\"Boot\",\"value3\":\"Relays Ready\"}";
    rolebot.triggerIFTTTEvent(iftttEventName, iftttWebhookKey, payload);
  } else {
    rolebot.serialWrite(L("WiFi bağlantısı başarısız. SSID/şifreyi kontrol edin.", "WiFi connection failed. Check SSID/PASS."));
  }
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // 1) B1: basınca olayı tetikle (30 ms parazit süzgeci) / B1: trigger the event on press (30 ms debounce)
  bool button = rolebot.button1Read(); // basılıyken LOW (false) / LOW (false) while pressed
  if (button != lastButton && now - lastEdgeMs >= 30) {
    lastEdgeMs = now;
    if (lastButton && !button) triggerButtonEvent(L("B1 butonu", "B1 button"));
    lastButton = button;
  }

  // 2) Yarım saniye sonra geri bildirimi kapat / turn the feedback off after half a second
  if (feedbackOn && (int32_t)(millis() - feedbackOffAtMs) >= 0) {
    feedbackOn = false;
    rolebot.ledWrite(false);
    rolebot.Relay1Write(false);
  }

  // 3) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
