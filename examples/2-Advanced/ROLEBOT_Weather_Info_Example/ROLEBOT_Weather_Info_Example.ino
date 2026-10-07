/*
 * TR: HAVA DURUMU. ROLEBOT WiFi'ye bağlanır ve seçilen şehrin hava durumunu internetten
 * alıp dakikada bir Seri Port'a yazar. API anahtarı boş bırakılırsa ücretsiz wttr.in
 * servisi kullanılır; isterseniz OpenWeatherMap anahtarınızı API_KEY'e yazın.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim         / help          -> komut listesi
 *      hava           / weather       -> hava durumunu şimdi al
 *      sehir Ankara   / city Ankara   -> şehri değiştir
 *      dil            / lang          -> dili değiştir (Türkçe <-> English)
 *
 * EN: WEATHER. The ROLEBOT connects to WiFi, gets the weather of the chosen city from the
 * internet and prints it to the Serial port once a minute. If the API key is left empty,
 * the free wttr.in service is used; optionally put your OpenWeatherMap key in API_KEY.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help           / yardim        -> command list
 *      weather        / hava          -> get the weather now
 *      city Ankara    / sehir Ankara  -> change the city
 *      lang           / dil           -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 */
#define USE_WEATHER
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// OpenWeatherMap API anahtarı (isteğe bağlı; boşsa wttr.in kullanılır)
// OpenWeatherMap API Key (optional, leave empty to use wttr.in)
#define API_KEY ""
String city = "Istanbul"; // Şehir ("sehir Ankara" ile değişir) / city (changed with "city Ankara")

const uint32_t UPDATE_EVERY_MS = 60000; // Dakikada bir / once a minute
uint32_t lastUpdateMs = 0;
bool firstUpdate = true;

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
String rawCommand; // Komutun orijinal hali (şehir adı harfleriyle kalsın diye) / original text (keeps the city's letters)
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "ŞEHİR" -> "sehir"
// Lower-cases and simplifies Turkish letters: "ŞEHİR" -> "sehir"
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
// Şehir adını olduğu gibi verin: getWeather() boşluk ve Türkçe harfleri adres (URL)
// için kendisi kodlar ("İzmir", "New York"). / Pass the city name as it is: getWeather()
// encodes spaces and Turkish letters for the address (URL) by itself.
void updateWeather() {
  lastUpdateMs = millis();
  if (WiFi.status() != WL_CONNECTED) {
    rolebot.serialWrite(L("WiFi bağlı değil - hava durumu alınamadı.", "WiFi not connected - could not get the weather."));
    return;
  }
  rolebot.ledWrite(true);
  String weather = rolebot.getWeather(city, API_KEY); // Birkaç saniye sürebilir / may take a few seconds
  rolebot.ledWrite(false);
  rolebot.serialWrite(String(L("Hava durumu (", "Weather in ")) + city + L("): ", ": ") + weather);
}

void printHelp() {
  rolebot.serialWrite(L("---- HAVA DURUMU - Komutlar ----", "---- WEATHER - Commands ----"));
  rolebot.serialWrite(L("  yardim        : bu liste", "  help          : this list"));
  rolebot.serialWrite(L("  hava          : hava durumunu şimdi al", "  weather       : get the weather now"));
  rolebot.serialWrite(L("  sehir Ankara  : şehri değiştir", "  city Ankara   : change the city"));
  rolebot.serialWrite(L("  dil           : English'e geç", "  lang          : switch to Turkish"));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "hava" || word == "weather") {
    updateWeather();
  } else if ((word == "sehir" || word == "city") && space > 0) {
    city = rawCommand.substring(rawCommand.indexOf(' ') + 1);
    city.trim();
    rolebot.serialWrite(String(L("Yeni şehir: ", "New city: ")) + city);
    updateWeather();
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

  rolebot.serialWrite(L("Hava Durumu Örneği", "Weather Info Example"));
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASSWORD);
  printHelp();
}

void loop() {
  // Dakikada bir güncelle (ilk seferde hemen); loop() beklemez, komutlar hemen çalışır.
  // Update once a minute (right away the first time); loop() never waits, commands work instantly.
  if (firstUpdate || millis() - lastUpdateMs >= UPDATE_EVERY_MS) {
    firstUpdate = false;
    updateWeather();
  }

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
