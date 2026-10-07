/*
 * TR: E-POSTA GÖNDERİCİ. ROLEBOT WiFi'ye bağlanır ve açılışta bir deneme e-postası
 * gönderir. Sonra B1 butonuna basarak ya da Seri Port'tan "gonder" yazarak tekrar
 * gönderebilirsiniz (örneğin "kapı açıldı" gibi bir uyarı için).
 *  - Gmail için normal şifreniz ÇALIŞMAZ: Google hesabınızda 2 adımlı doğrulamayı açıp bir
 *    "Uygulama şifresi" oluşturun ve AUTHOR_PASSWORD'e onu yazın.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim  / help   -> komut listesi
 *      gonder  / send   -> deneme e-postası gönder (birkaç saniye sürer)
 *      durum   / status -> WiFi durumu ve gönderilen e-posta sayısı
 *      dil     / lang   -> dili değiştir (Türkçe <-> English; e-posta metni de)
 *
 * EN: EMAIL SENDER. The ROLEBOT connects to WiFi and sends a test email at startup. After
 * that, press the B1 button or type "send" on the Serial port to send again (e.g. for an
 * alert like "the door opened").
 *  - For Gmail your normal password does NOT work: turn on 2-step verification in your
 *    Google account, create an "App password" and put it in AUTHOR_PASSWORD.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help    / yardim -> command list
 *      send    / gonder -> send a test email (takes a few seconds)
 *      status  / durum  -> WiFi state and number of emails sent
 *      lang    / dil    -> switch language (Turkish <-> English; email text too)
 *
 * Bağlantı / Wiring: Kablo gerekmez. Aşağıdaki bilgileri kendi bilgilerinizle değiştirin.
 * / No wiring needed. Replace the settings below with your own.
 */
#define USE_EMAIL
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

// WiFi bilgileri / WiFi credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// E-posta bilgileri / Email credentials
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "YOUR_EMAIL@gmail.com"
#define AUTHOR_PASSWORD "YOUR_APP_PASSWORD"
#define RECIPIENT_EMAIL "RECIPIENT_EMAIL@example.com"

uint32_t emailCount = 0;
bool lastButton = true;       // HIGH = bırakılmış / released
uint32_t lastEdgeMs = 0;      // Parazit süzgeci / debounce
uint32_t lastSendMs = 0;
const uint32_t MIN_GAP_MS = 10000; // İki e-posta arası en az 10 sn (spam olmasın) / at least 10 s between emails (no spam)

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
void sendTestEmail(const char *source) {
  if (WiFi.status() != WL_CONNECTED) {
    rolebot.serialWrite(L("WiFi bağlı değil - e-posta gönderilemez.", "WiFi not connected - cannot send email."));
    return;
  }
  if (emailCount > 0 && millis() - lastSendMs < MIN_GAP_MS) {
    rolebot.serialWrite(L("Biraz bekleyin: iki e-posta arası en az 10 saniye.", "Please wait: at least 10 seconds between emails."));
    return;
  }
  lastSendMs = millis();
  emailCount++;
  rolebot.ledWrite(true);
  rolebot.serialWrite(String(L("E-posta gönderiliyor (", "Sending email (")) + source + ")...");
  String body = String(L("ROLEBOT'tan merhaba! Bu, ", "Hello from ROLEBOT! This is email number ")) + emailCount +
                L(". e-posta. Kaynak: ", ". Source: ") + source;
  rolebot.sendEmail(SMTP_HOST, SMTP_PORT, AUTHOR_EMAIL, AUTHOR_PASSWORD, RECIPIENT_EMAIL,
                    L("ROLEBOT Deneme", "ROLEBOT Test"), body);
  rolebot.ledWrite(false);
}

void printHelp() {
  rolebot.serialWrite(L("---- E-POSTA GÖNDERİCİ - Komutlar ----", "---- EMAIL SENDER - Commands ----"));
  rolebot.serialWrite(L("  yardim  : bu liste", "  help    : this list"));
  rolebot.serialWrite(L("  gonder  : deneme e-postası gönder", "  send    : send a test email"));
  rolebot.serialWrite(L("  durum   : WiFi ve e-posta sayısı", "  status  : WiFi and email count"));
  rolebot.serialWrite(L("  dil     : English'e geç", "  lang    : switch to Turkish"));
  rolebot.serialWrite(L("  B1      : e-posta gönder", "  B1      : send an email"));
}

void handleCommand(const String &cmd) {
  if (cmd == "yardim" || cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "gonder" || cmd == "send") {
    sendTestEmail(L("seri komut", "serial command"));
  } else if (cmd == "durum" || cmd == "status") {
    rolebot.serialWrite(String("WiFi: ") + (WiFi.status() == WL_CONNECTED ? L("BAĞLI", "CONNECTED") : L("BAĞLI DEĞİL", "NOT CONNECTED")) +
                        L(" | Gönderilen e-posta: ", " | Emails sent: ") + emailCount);
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

  rolebot.serialWrite(L("E-posta Gönderici Örneği", "Email Sender Example"));
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASSWORD);

  if (rolebot.wifiConnectionControl()) {
    sendTestEmail(L("açılış", "startup"));
  }
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // B1: basınca e-posta gönder (30 ms parazit süzgeci) / B1: send an email on press (30 ms debounce)
  bool button = rolebot.button1Read(); // false = basılı / pressed
  if (button != lastButton && now - lastEdgeMs >= 30) {
    lastEdgeMs = now;
    if (lastButton && !button) sendTestEmail(L("B1 butonu", "B1 button"));
    lastButton = button;
  }

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
