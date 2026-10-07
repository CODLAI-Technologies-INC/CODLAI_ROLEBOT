/*
 * TR: ERİŞİM NOKTASI (AP) MODUNDA YEREL WEB SUNUCUSU. ROLEBOT kendi WiFi ağını kurar ve
 * içinde basit bir web sayfası yayınlar. İnternet gerekmez.
 *  1) Bu kodu yükleyin.
 *  2) Telefon/bilgisayardan "CODLAI Server" ağına bağlanın (şifre: 12345678).
 *  3) Tarayıcıda http://192.168.4.1/demopage adresini açın ve butona tıklayın.
 *  - Sunucu özelliklerini kullanmak için kütüphaneyi eklemeden ÖNCE "#define USE_SERVER"
 *    yazılmalıdır (aşağıda var).
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim  / help    -> komut listesi
 *      durum   / status  -> ağ adı, sayfa adresi ve bağlı cihaz sayısı
 *      dil     / lang    -> dili değiştir (Türkçe <-> English)
 *
 * EN: LOCAL WEB SERVER IN ACCESS POINT (AP) MODE. The ROLEBOT creates its own WiFi
 * network and serves a simple web page in it. No internet needed.
 *  1) Upload this code.
 *  2) From a phone/computer, join the "CODLAI Server" network (password: 12345678).
 *  3) Open http://192.168.4.1/demopage in a browser and click the button.
 *  - To use the server features, "#define USE_SERVER" must be written BEFORE including
 *    the library (it is below).
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help    / yardim  -> command list
 *      status  / durum   -> network name, page address and connected devices
 *      lang    / dil     -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 */

#define USE_SERVER
#include <ROLEBOT.h> // ROLEBOT kütüphanesi / ROLEBOT library

ROLEBOT rolebot; // ROLEBOT nesnesi / ROLEBOT object

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

// Erişim noktası (AP) WiFi bilgileri / Access point (AP) WiFi settings
#define AP_SSID "CODLAI Server" // AP ağ adı / AP network name
#define AP_PASS "12345678"      // AP şifresi (en az 8 karakter) / AP password (at least 8 characters)

// ---------------------------------------------------------------------------
// Web sayfası içeriği (HTML, CSS, JavaScript) / Web page content (HTML, CSS, JavaScript)
// NOT: HTML snprintf ile birleştirilir; HTML içinde tek başına '%' kullanmayın.
// NOTE: The HTML is combined with snprintf; do not use a lone '%' inside the HTML.
// ---------------------------------------------------------------------------
// JavaScript: butona tıklanınca bir mesaj gösterir / shows a message when the button is clicked
const char WEBPageScript[] PROGMEM = R"rawliteral(
<script>
  function sayHello() {
    alert("Merhaba ROLEBOT! / Hello ROLEBOT!");
  }
</script>
)rawliteral";

// CSS: sayfanın görünümü / the look of the page
const char WEBPageCSS[] PROGMEM = R"rawliteral(
<style>
  body { text-align: center; font-family: Arial, sans-serif; }
  button { font-size: 20px; padding: 10px; margin: 20px; }
</style>
)rawliteral";

// HTML: ROLEBOT'un sunacağı sayfa (ilk %s = JavaScript, ikinci %s = CSS)
// HTML: the page served by ROLEBOT (first %s = JavaScript, second %s = CSS)
const char WEBPageHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ROLEBOT Web Server</title>
  %s
  %s
</head>
<body>
  <h1>ROLEBOT Web Sayfası / Web Page</h1>
  <button onclick="sayHello()">Tıklayın / Click</button>
</body>
</html>
)rawliteral";

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "YARDIM" -> "yardim"
// Lower-cases and simplifies Turkish letters: "YARDIM" -> "yardim"
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
  rolebot.serialWrite(L("---- AP MODU WEB SUNUCUSU - Komutlar ----", "---- AP MODE WEB SERVER - Commands ----"));
  rolebot.serialWrite(L("  yardim  : bu liste", "  help    : this list"));
  rolebot.serialWrite(L("  durum   : ağ adı, sayfa adresi, bağlı cihazlar", "  status  : network name, page address, connected devices"));
  rolebot.serialWrite(L("  dil     : English'e geç", "  lang    : switch to Turkish"));
}

void printStatus() {
  rolebot.serialWrite(String(L("Ağ adı: \"", "Network: \"")) + AP_SSID + L("\"  şifre: ", "\"  password: ") + AP_PASS);
  rolebot.serialWrite(String(L("Sayfa adresi: http://", "Page address: http://")) + WiFi.softAPIP().toString() + "/demopage");
  rolebot.serialWrite(String(L("Bağlı cihaz sayısı: ", "Connected devices: ")) + WiFi.softAPgetStationNum());
}

void handleCommand(const String &cmd) {
  if (cmd == "yardim" || cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "durum" || cmd == "status") {
    printStatus();
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
  rolebot.begin();
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI / safety: relays OFF
  rolebot.Relay2Write(false);
  rolebot.serialStart(115200); // Seri haberleşme / Serial communication

  // ROLEBOT'u erişim noktası (AP) olarak başlat / Start the ROLEBOT as an access point (AP)
  rolebot.serverStart("AP", AP_SSID, AP_PASS);

  // Web sayfasını yayınla: http://192.168.4.1/demopage / Publish the web page: http://192.168.4.1/demopage
  rolebot.serverCreateLocalPage("demopage", WEBPageScript, WEBPageCSS, WEBPageHTML);

  printStatus();
  printHelp();
}

void loop() {
  rolebot.serverContinue(); // AP modunda DNS yönlendirmesini sürdür / keep DNS redirection running in AP mode

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
