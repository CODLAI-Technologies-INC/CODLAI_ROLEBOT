/*
 * TR: VİKİPEDİ ARAMA. ROLEBOT WiFi'ye bağlanır, Vikipedi'de bir konuyu arar ve kısa
 * özetini Seri Port'a yazar. Açılışta "Robot" konusunu arar; sonra Seri Port'tan
 * istediğiniz konuyu arayabilirsiniz. Arama dili, seçili dile göre Türkçe (tr) ya da
 * İngilizce (en) Vikipedi'dir.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim                 / help                 -> komut listesi
 *      ara Mustafa Kemal      / search Albert Einstein -> konuyu ara
 *      dil                    / lang                 -> dili değiştir (Türkçe <-> English;
 *                                                       Vikipedi dili de değişir)
 *  - İpucu: Konu adını Vikipedi'deki başlıkla aynı yazın (büyük harfler ve Türkçe harfler
 *    korunur), örn. "ara İstanbul".
 *
 * EN: WIKIPEDIA SEARCH. The ROLEBOT connects to WiFi, looks up a topic on Wikipedia and
 * prints a short summary to the Serial port. At startup it looks up "Robot"; after that you
 * can search any topic from the Serial port. The search uses the Turkish (tr) or English
 * (en) Wikipedia, following the selected language.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help                   / yardim               -> command list
 *      search Albert Einstein / ara Mustafa Kemal    -> look up the topic
 *      lang                   / dil                  -> switch language (Turkish <-> English;
 *                                                       the Wikipedia language changes too)
 *  - Tip: write the topic like the Wikipedia title (capitals and Turkish letters are kept),
 *    e.g. "search Istanbul".
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 */
#define USE_WIKIPEDIA
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
String rawCommand; // Komutun orijinal hali (aranan konu harfleriyle kalsın diye) / original text (keeps the topic's letters)
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
// Konuyu olduğu gibi verin: getWikipedia() boşlukları "_" yapar, Türkçe harfleri ve
// işaretleri adres (URL) için kendisi kodlar. / Pass the topic as it is: getWikipedia()
// turns spaces into "_" and encodes Turkish letters and symbols by itself.
void searchWiki(const String &topic) {
  if (WiFi.status() != WL_CONNECTED) {
    rolebot.serialWrite(L("WiFi bağlı değil - arama yapılamadı.", "WiFi not connected - could not search."));
    return;
  }
  const char *wikiLang = turkish ? "tr" : "en";
  rolebot.serialWrite(String(L("Aranıyor: ", "Searching for: ")) + topic + "  (" + wikiLang + ".wikipedia.org)...");
  rolebot.ledWrite(true);
  String summary = rolebot.getWikipedia(topic, wikiLang); // Birkaç saniye sürebilir / may take a few seconds
  rolebot.ledWrite(false);
  rolebot.serialWrite(L("Özet:", "Summary:"));
  rolebot.serialWrite(summary == "No Summary Found" ? String(L("Özet bulunamadı (başlığı Vikipedi'deki gibi yazın).", "No summary found (write the title like on Wikipedia).")) : summary);
}

void printHelp() {
  rolebot.serialWrite(L("---- VİKİPEDİ ARAMA - Komutlar ----", "---- WIKIPEDIA SEARCH - Commands ----"));
  rolebot.serialWrite(L("  yardim             : bu liste", "  help               : this list"));
  rolebot.serialWrite(L("  ara Mustafa Kemal  : konuyu ara (tr.wikipedia.org)", "  search Robot       : look up the topic (en.wikipedia.org)"));
  rolebot.serialWrite(L("  dil                : English'e geç (İngilizce Vikipedi)", "  lang               : switch to Turkish (Turkish Wikipedia)"));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if ((word == "ara" || word == "search" || word == "wiki") && space > 0) {
    searchWiki(rawCommand.substring(rawCommand.indexOf(' ') + 1));
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

  rolebot.serialWrite(L("Vikipedi Arama Örneği", "Wikipedia Search Example"));
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASSWORD);

  if (WiFi.status() == WL_CONNECTED) {
    searchWiki("Robot");
  }
  printHelp();
}

void loop() {
  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
