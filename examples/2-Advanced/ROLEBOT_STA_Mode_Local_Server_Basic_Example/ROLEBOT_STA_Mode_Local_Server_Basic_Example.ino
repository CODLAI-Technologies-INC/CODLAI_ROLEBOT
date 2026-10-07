/*
 * TR: İSTASYON (STA) MODUNDA YEREL WEB SUNUCUSU. ROLEBOT evinizin WiFi ağına bağlanır ve
 * o ağda basit bir web sayfası yayınlar. Aynı ağdaki telefon/bilgisayardan açabilirsiniz.
 * WiFi'ye bağlanamazsa kendi ağını (AP modu) kurar.
 *  1) Aşağıya WiFi adınızı ve şifrenizi yazın, kodu yükleyin.
 *  2) Seri Port'ta yazan sayfa adresini (örn. http://192.168.1.45/demopage) aynı ağdaki
 *     bir tarayıcıda açın. AP moduna düştüyse "CODLAI Server" ağına bağlanıp
 *     http://192.168.4.1/demopage adresini açın.
 *  - Sunucu özelliklerini kullanmak için kütüphaneyi eklemeden ÖNCE "#define USE_SERVER"
 *    yazılmalıdır (aşağıda var).
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim  / help    -> komut listesi
 *      durum   / status  -> mod (STA/AP), IP ve sayfa adresi
 *      dil     / lang    -> dili değiştir (Türkçe <-> English)
 *
 * EN: LOCAL WEB SERVER IN STATION (STA) MODE. The ROLEBOT joins your home WiFi network
 * and serves a simple web page on it. Open it from a phone/computer on the same network.
 * If it cannot connect to WiFi, it creates its own network (AP mode).
 *  1) Enter your WiFi name and password below and upload the code.
 *  2) Open the page address printed on the Serial port (e.g. http://192.168.1.45/demopage)
 *     in a browser on the same network. If it fell back to AP mode, join the
 *     "CODLAI Server" network and open http://192.168.4.1/demopage.
 *  - To use the server features, "#define USE_SERVER" must be written BEFORE including
 *    the library (it is below).
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help    / yardim  -> command list
 *      status  / durum   -> mode (STA/AP), IP and page address
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

// WiFi ayarları - bağlanmak istediğiniz ağ / WiFi settings - the network to join
#define WIFI_SSID "YOUR_WIFI_SSID"     // WiFi adınız / your WiFi name
#define WIFI_PASS "YOUR_WIFI_PASSWORD" // WiFi şifreniz / your WiFi password

// WiFi'ye bağlanamazsa kurulacak erişim noktası (AP) / Access point (AP) used if WiFi fails
#define AP_SSID "CODLAI Server" // AP ağ adı / AP network name
#define AP_PASS "12345678"      // AP şifresi (en az 8 karakter) / AP password (at least 8 characters)

bool apMode = false; // true = WiFi'ye bağlanamadı, kendi ağını kurdu / true = WiFi failed, own network created

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
  rolebot.serialWrite(L("---- STA MODU WEB SUNUCUSU - Komutlar ----", "---- STA MODE WEB SERVER - Commands ----"));
  rolebot.serialWrite(L("  yardim  : bu liste", "  help    : this list"));
  rolebot.serialWrite(L("  durum   : mod, IP ve sayfa adresi", "  status  : mode, IP and page address"));
  rolebot.serialWrite(L("  dil     : English'e geç", "  lang    : switch to Turkish"));
}

void printStatus() {
  if (apMode) {
    rolebot.serialWrite(String(L("Mod: AP (kendi ağı) - ağ adı: \"", "Mode: AP (own network) - network: \"")) + AP_SSID +
                        L("\"  şifre: ", "\"  password: ") + AP_PASS);
    rolebot.serialWrite(String(L("Sayfa adresi: http://", "Page address: http://")) + WiFi.softAPIP().toString() + "/demopage");
  } else {
    rolebot.serialWrite(String(L("Mod: STA - bağlı ağ: \"", "Mode: STA - connected to: \"")) + WIFI_SSID + "\"  " +
                        (WiFi.status() == WL_CONNECTED ? L("(bağlı)", "(connected)") : L("(bağlantı koptu!)", "(connection lost!)")));
    rolebot.serialWrite(String(L("Sayfa adresi: http://", "Page address: http://")) + rolebot.wifiGetIPAddress() + "/demopage");
  }
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

  // Önce WiFi'yi dene; sunucuyu SADECE BİR KEZ başlat (iki kez başlatmak sayfaları iki kez kaydeder).
  // Try WiFi first; start the server ONLY ONCE (starting it twice registers the pages twice).
  WiFi.mode(WIFI_STA);
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS);
  if (WiFi.status() == WL_CONNECTED) {
    rolebot.serverStart("STA", WIFI_SSID, WIFI_PASS); // Zaten bağlı, hemen sunucuyu açar / already connected, opens the server
  } else {
    rolebot.serialWrite(L("WiFi'ye bağlanılamadı - kendi ağımızı (AP) kuruyoruz.", "Could not join WiFi - creating our own network (AP)."));
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    rolebot.serverStart("AP", AP_SSID, AP_PASS);
    apMode = true;
  }

  // Web sayfasını yayınla: http://<IP>/demopage / Publish the web page: http://<IP>/demopage
  rolebot.serverCreateLocalPage("demopage", WEBPageScript, WEBPageCSS, WEBPageHTML);

  printStatus();
  printHelp();
}

void loop() {
  rolebot.serverContinue(); // AP modundaysa DNS yönlendirmesini sürdür / keep DNS redirection running in AP mode

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
