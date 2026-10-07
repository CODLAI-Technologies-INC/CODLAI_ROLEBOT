/*
 * TR: TELEGRAM BİLDİRİMİ. ROLEBOT WiFi'ye bağlanır; B1 butonuna basınca telefonunuza
 * Telegram mesajı gönderir (örneğin "kapı zili çaldı" uyarısı). Seri Port'tan kendi
 * mesajınızı da gönderebilirsiniz.
 *  - BOT_TOKEN: Telegram'da @BotFather ile bir bot oluşturun, verdiği anahtarı yazın.
 *  - CHAT_ID: @userinfobot'a yazarak kendi sohbet numaranızı öğrenin. Botunuza bir kez
 *    "merhaba" yazmayı unutmayın (yoksa bot size mesaj atamaz).
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim             / help             -> komut listesi
 *      gonder             / send             -> "Butona basıldı" deneme mesajı gönder
 *      mesaj Kapı açıldı  / msg Door opened  -> kendi mesajınızı gönder
 *      durum              / status           -> WiFi durumu ve gönderilen mesaj sayısı
 *      dil                / lang             -> dili değiştir (Türkçe <-> English; mesaj metni de)
 *
 * EN: TELEGRAM NOTIFICATION. The ROLEBOT connects to WiFi; pressing the B1 button sends a
 * Telegram message to your phone (e.g. a "doorbell rang" alert). You can also send your
 * own message from the Serial port.
 *  - BOT_TOKEN: create a bot with @BotFather in Telegram and paste the token it gives you.
 *  - CHAT_ID: message @userinfobot to learn your chat id. Remember to send "hello" to your
 *    bot once (otherwise the bot cannot message you).
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help               / yardim            -> command list
 *      send               / gonder            -> send a "Button pressed" test message
 *      msg Door opened    / mesaj Kapı açıldı -> send your own message
 *      status             / durum             -> WiFi state and number of messages sent
 *      lang               / dil               -> switch language (Turkish <-> English; message text too)
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 */
#define USE_TELEGRAM
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Telegram bot anahtarı (@BotFather'dan) / Telegram Bot Token (get it from @BotFather)
#define BOT_TOKEN "YOUR_BOT_TOKEN"
// Sohbet numaranız (@userinfobot'tan) / Your Chat ID (get it from @userinfobot)
#define CHAT_ID "YOUR_CHAT_ID"

const uint32_t MIN_GAP_MS = 5000; // İki mesaj arası en az 5 sn / at least 5 s between messages
uint32_t lastSendMs = 0;
uint32_t messageCount = 0;
bool lastButton = true;  // HIGH = bırakılmış / released
uint32_t lastEdgeMs = 0; // Parazit süzgeci / debounce

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
String rawCommand; // Komutun orijinal hali (mesaj büyük/küçük ve Türkçe harfleriyle gitsin diye) / original text (keeps case and Turkish letters)
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
// Mesajı olduğu gibi verin: sendTelegram() boşluk, &, ? ve Türkçe harfleri adres (URL)
// için kendisi kodlar. / Pass the message as it is: sendTelegram() encodes spaces, &, ?
// and Turkish letters for the address (URL) by itself.
void sendMessage(const String &text) {
  if (WiFi.status() != WL_CONNECTED) {
    rolebot.serialWrite(L("WiFi bağlı değil - mesaj gönderilemez.", "WiFi not connected - cannot send the message."));
    return;
  }
  if (messageCount > 0 && millis() - lastSendMs < MIN_GAP_MS) {
    rolebot.serialWrite(L("Biraz bekleyin: iki mesaj arası en az 5 saniye.", "Please wait: at least 5 seconds between messages."));
    return;
  }
  lastSendMs = millis();
  messageCount++;
  rolebot.ledWrite(true);
  rolebot.serialWrite(String(L("Telegram mesajı gönderiliyor: ", "Sending Telegram message: ")) + text);
  rolebot.sendTelegram(BOT_TOKEN, CHAT_ID, text); // Birkaç saniye sürebilir / may take a few seconds
  rolebot.ledWrite(false);
}

void printHelp() {
  rolebot.serialWrite(L("---- TELEGRAM BİLDİRİMİ - Komutlar ----", "---- TELEGRAM NOTIFICATION - Commands ----"));
  rolebot.serialWrite(L("  yardim             : bu liste", "  help               : this list"));
  rolebot.serialWrite(L("  gonder             : deneme mesajı gönder", "  send               : send a test message"));
  rolebot.serialWrite(L("  mesaj Kapı açıldı  : kendi mesajınızı gönder", "  msg Door opened    : send your own message"));
  rolebot.serialWrite(L("  durum              : WiFi ve mesaj sayısı", "  status             : WiFi and message count"));
  rolebot.serialWrite(L("  dil                : English'e geç", "  lang               : switch to Turkish"));
  rolebot.serialWrite(L("  B1                 : deneme mesajı gönder", "  B1                 : send a test message"));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "gonder" || word == "send") {
    sendMessage(L("ROLEBOT: Butona basıldı!", "ROLEBOT: Button pressed!"));
  } else if ((word == "mesaj" || word == "msg" || word == "message") && space > 0) {
    String text = rawCommand.substring(rawCommand.indexOf(' ') + 1);
    text.trim();
    sendMessage(String("ROLEBOT: ") + text);
  } else if (word == "durum" || word == "status") {
    rolebot.serialWrite(String("WiFi: ") + (WiFi.status() == WL_CONNECTED ? L("BAĞLI", "CONNECTED") : L("BAĞLI DEĞİL", "NOT CONNECTED")) +
                        L(" | Gönderilen mesaj: ", " | Messages sent: ") + messageCount);
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
  rolebot.serialStart(115200);
  rolebot.begin();
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI / safety: relays OFF
  rolebot.Relay2Write(false);

  rolebot.serialWrite(L("Telegram Bildirim Örneği", "Telegram Notification Example"));
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASSWORD);
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // B1: basınca mesaj gönder (30 ms parazit süzgeci; eskiden 5 sn delay() vardı)
  // B1: send a message on press (30 ms debounce; used to be a 5 s delay())
  bool button = rolebot.button1Read(); // false = basılı / pressed
  if (button != lastButton && now - lastEdgeMs >= 30) {
    lastEdgeMs = now;
    if (lastButton && !button) sendMessage(L("ROLEBOT: Butona basıldı!", "ROLEBOT: Button pressed!"));
    lastButton = button;
  }

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
