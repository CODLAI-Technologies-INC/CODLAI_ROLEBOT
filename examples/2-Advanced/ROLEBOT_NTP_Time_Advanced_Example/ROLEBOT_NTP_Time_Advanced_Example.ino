/*
 * TR: İNTERNET SAATİ (NTP). ROLEBOT WiFi'ye bağlanır ve doğru saati internetten (NTP)
 * alır. Saat 10 saniyede bir Seri Port'a yazılır. WiFi bağlantısı kurulduktan SONRA
 * ntpBegin() çağrılmalıdır.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim    / help    -> komut listesi
 *      saat      / time    -> tarih, saat, haftanın günü ve epoch değerini şimdi yaz
 *      guncelle  / update  -> saati internetten hemen yeniden al
 *      dil       / lang    -> dili değiştir (Türkçe <-> English)
 *
 * EN: INTERNET TIME (NTP). The ROLEBOT connects to WiFi and gets the correct time from
 * the internet (NTP). The time is printed to the Serial port every 10 seconds. Call
 * ntpBegin() AFTER connecting to WiFi.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help      / yardim    -> command list
 *      time      / saat      -> print date, time, weekday and epoch now
 *      update    / guncelle  -> fetch the time from the internet now
 *      lang      / dil       -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. Aşağıya WiFi adınızı ve şifrenizi yazın.
 * / No wiring needed. Fill in your WiFi name and password below.
 */

#define USE_WIFI
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

// WiFi bilgileriniz / your WiFi credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

// Türkiye UTC+3, yaz saati uygulaması yok / Turkey is UTC+3, no DST
static const int TIMEZONE_HOURS = 3;

uint32_t lastPrintMs = 0;

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "GÜNCELLE" -> "guncelle"
// Lower-cases and simplifies Turkish letters: "GÜNCELLE" -> "guncelle"
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
void printTime() {
  if (!rolebot.ntpIsTimeValid()) {
    rolebot.serialWrite(L("Saat henüz yok (WiFi bağlı mı? \"guncelle\" yazıp tekrar deneyin).",
                          "No time yet (is WiFi connected? type \"update\" to try again)."));
    return;
  }
  const char *daysTr[] = {"Pazartesi", "Salı", "Çarşamba", "Perşembe", "Cuma", "Cumartesi", "Pazar"};
  const char *daysEn[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
  int wd = rolebot.ntpGetWeekday(); // 1 = Pazartesi/Monday ... 7 = Pazar/Sunday
  rolebot.serialWrite(String(L("Tarih: ", "Date: ")) + rolebot.ntpGetDateString() + "  " +
                      (turkish ? daysTr[wd - 1] : daysEn[wd - 1]) +
                      L("  Saat: ", "  Time: ") + rolebot.ntpGetTimeString() +
                      "  (epoch: " + String((unsigned long)rolebot.ntpGetEpoch()) + ")");
}

void printHelp() {
  rolebot.serialWrite(L("---- İNTERNET SAATİ - Komutlar ----", "---- INTERNET TIME - Commands ----"));
  rolebot.serialWrite(L("  yardim    : bu liste", "  help      : this list"));
  rolebot.serialWrite(L("  saat      : tarih ve saati yaz", "  time      : print date and time"));
  rolebot.serialWrite(L("  guncelle  : saati yeniden al", "  update    : re-sync the time"));
  rolebot.serialWrite(L("  dil       : English'e geç", "  lang      : switch to Turkish"));
}

void handleCommand(const String &cmd) {
  if (cmd == "yardim" || cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "saat" || cmd == "time" || cmd == "tarih" || cmd == "date") {
    printTime();
  } else if (cmd == "guncelle" || cmd == "update") {
    rolebot.serialWrite(L("Saat güncelleniyor...", "Updating the time..."));
    bool ok = rolebot.ntpUpdate(); // En fazla 10 sn / up to 10 s
    rolebot.serialWrite(ok ? L("[NTP] Eşitlendi", "[NTP] Synced") : L("[NTP] Eşitlenemedi", "[NTP] Sync failed"));
    printTime();
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
  rolebot.serialStart(115200);
  delay(200);

  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS);
  if (!rolebot.wifiConnectionControl()) {
    rolebot.serialWrite(L("[WiFi] Bağlı değil - WiFi gelince \"guncelle\" yazın.", "[WiFi] Not connected - type \"update\" once WiFi is back."));
  }

  // Tek satırda kurulum (önerilen) / single-call setup (recommended)
  bool ok = rolebot.ntpBegin(TIMEZONE_HOURS);
  rolebot.serialWrite(ok ? L("[NTP] Eşitlendi", "[NTP] Synced") : L("[NTP] Eşitlenemedi", "[NTP] Sync failed"));
  printTime();
  printHelp();
}

void loop() {
  // 10 saniyede bir saati yaz / print the time every 10 seconds
  if (millis() - lastPrintMs >= 10000) {
    lastPrintMs = millis();
    printTime();
  }

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
