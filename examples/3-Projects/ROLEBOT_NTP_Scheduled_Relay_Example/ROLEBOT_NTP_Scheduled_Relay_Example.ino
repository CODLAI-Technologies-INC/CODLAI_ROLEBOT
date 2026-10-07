/*
 * TR: GERÇEK PROJE - Saat Ayarlı Akıllı Röle (Haftalık Program). ROLEBOT WiFi'ye
 * bağlanır, internetten (NTP) doğru saati alır ve rölelerini bir PROGRAM TABLOSUNA
 * göre kendiliğinden açar/kapatır: örneğin Röle 1 (salon lambası) her akşam
 * 19:00-23:30, Röle 2 (bahçe pompası) Pazartesi, Çarşamba ve Cuma sabahları
 * 07:00-07:15. Saat bilinmiyorsa güvenlik için program röleleri KAPALI tutar.
 *  - ELLE müdahale: B1 kısa basış = Röle 1, uzun basış (1 sn+) = Röle 2 tersine döner.
 *    Elle ayar, programdaki bir sonraki değişime kadar (ya da "program" komutuna kadar)
 *    geçerlidir.
 *  - Mavi LED: saat varsa yavaş, yoksa hızlı yanıp söner.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim       / help        -> komut listesi
 *      role1 ac     / relay1 on   -> Röle 1'i elle aç (programı geçersiz kılar)
 *      role2 kapat  / relay2 off  -> Röle 2'yi elle kapat
 *      degistir 1   / toggle 1    -> Röle 1'i elle tersine çevir
 *      hepsi kapat  / all off     -> iki röleyi de elle kapat
 *      program      / schedule    -> elle ayarları bırak, programa dön
 *      tablo        / table       -> program tablosunu göster
 *      durum        / status      -> saat ve röle durumları
 *      guncelle     / update      -> saati internetten hemen yeniden al
 *      dil          / lang        -> dili değiştir (Türkçe <-> English)
 *
 * EN: A REAL PROJECT - Time-Scheduled Smart Relay (Weekly Program). The ROLEBOT
 * connects to WiFi, gets the correct time from the internet (NTP) and switches its
 * relays on/off BY ITSELF following a SCHEDULE TABLE: e.g. Relay 1 (living room lamp)
 * every evening 19:00-23:30, Relay 2 (garden pump) Monday, Wednesday and Friday
 * mornings 07:00-07:15. If the time is unknown, the schedule keeps the relays OFF for
 * safety.
 *  - MANUAL override: B1 short press = Relay 1, long press (1 s+) = Relay 2 is inverted.
 *    The override lasts until the next change in the schedule (or the "schedule" command).
 *  - Blue LED: slow blink with a valid time, fast blink without.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help         / yardim      -> command list
 *      relay1 on    / role1 ac    -> turn Relay 1 on by hand (overrides the schedule)
 *      relay2 off   / role2 kapat -> turn Relay 2 off by hand
 *      toggle 1     / degistir 1  -> invert Relay 1 by hand
 *      all off      / hepsi kapat -> turn both relays off by hand
 *      schedule     / program     -> drop the overrides, follow the schedule
 *      table        / tablo       -> show the schedule table
 *      status       / durum       -> time and relay states
 *      update       / guncelle    -> fetch the time from the internet now
 *      lang         / dil         -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Yükleri rölelerin COM + NO uçlarına bağlayın. Şebeke gerilimi
 * (220V) ile SADECE bir yetişkin çalışsın. Aşağıya WiFi adınızı ve şifrenizi yazın.
 * / Wire the loads to the relays' COM + NO terminals. Only an adult should work with
 * mains voltage (220V). Enter your WiFi name and password below.
 *
 * ROLEBOT'ta LCD ekran YOK; tüm bilgiler Seri Port (USB) üzerinden verilir.
 * / ROLEBOT has NO LCD screen; all feedback is given through Serial (USB).
 */

#define USE_WIFI
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

#define WIFI_SSID "YOUR_WIFI_SSID"     // WiFi adınız / your WiFi name
#define WIFI_PASS "YOUR_WIFI_PASSWORD" // WiFi şifreniz / your WiFi password

namespace {
  constexpr int kTimezoneHours = 3;        // Türkiye = UTC+3 (yaz saati yok) / Turkey = UTC+3 (no DST)
  constexpr uint32_t kStatusEveryMs = 10000;
  constexpr uint32_t kLongPressMs = 1000;  // Bundan uzun basma = Röle 2 / longer press = Relay 2

  // Gün bitleri (tm_wday: 0 = Pazar). / Day bits (tm_wday: 0 = Sunday).
  constexpr uint8_t kSun = 1, kMon = 2, kTue = 4, kWed = 8, kThu = 16, kFri = 32, kSat = 64;
  constexpr uint8_t kEveryDay = 127;

  struct Rule { uint8_t relay; uint8_t days; uint8_t onH, onM, offH, offM; };

  // PROGRAM TABLOSU - satır ekleyip değiştirerek kendi programınızı yapın.
  // SCHEDULE TABLE - add/change rows to build your own program.
  // Kapanış saati açılıştan SONRA olmalı; gece yarısını geçen bir program için
  // iki satır yazın (örn. 22:00-24:00 ve 00:00-06:00). / The off time must be
  // AFTER the on time; for a program past midnight write two rows (e.g.
  // 22:00-24:00 and 00:00-06:00).
  const Rule kSchedule[] = {
    {1, kEveryDay,         19, 0, 23, 30}, // Röle 1: lamba / Relay 1: lamp
    {2, kMon | kWed | kFri,  7, 0,  7, 15}, // Röle 2: bahçe pompası / Relay 2: garden pump
  };

  bool planned[3], lastPlanned[3], overridden[3], overrideValue[3], relayOn[3]; // [1] ve [2] kullanılır / [1] and [2] are used
  bool lastButton = true;                  // HIGH = bırakılmış / released
  uint32_t pressStartMs = 0, lastStatusMs = 0, lastBlinkMs = 0;
  bool ledState = false;

  // Bu röle için program şu an "açık" diyor mu? / Does the schedule say "on" right now for this relay?
  bool scheduleSays(int relay, const struct tm &t) {
    int nowMin = t.tm_hour * 60 + t.tm_min;
    for (const Rule &r : kSchedule) {
      if (r.relay != relay || !(r.days & (1 << t.tm_wday))) continue;
      int onMin = r.onH * 60 + r.onM, offMin = r.offH * 60 + r.offM;
      if (nowMin >= onMin && nowMin < offMin) return true;
    }
    return false;
  }

  const char *onOff(bool on) { return on ? L("AÇIK", "ON") : L("KAPALI", "OFF"); }

  // Elle ayar: röleyi istenen değere getirir (2 = tersine çevir)
  // Manual override: sets the relay to the wanted value (2 = invert)
  void setOverride(int relay, int action, const char *source) {
    overridden[relay] = true;
    overrideValue[relay] = (action == 2) ? !relayOn[relay] : (action == 1);
    char line[140];
    snprintf(line, sizeof(line), L("ELLE (%s): Röle %d -> %s (programdaki bir sonraki değişime kadar)",
                                   "MANUAL (%s): Relay %d -> %s (until the next schedule change)"),
             source, relay, onOff(overrideValue[relay]));
    rolebot.serialWrite(line);
  }

  void printStatus() {
    String line = rolebot.ntpIsTimeValid() ? rolebot.ntpGetDateTimeString() : String(L("SAAT YOK (güvenli mod)", "NO TIME (safe mode)"));
    for (int relay = 1; relay <= 2; relay++) {
      line += String(L(" | Röle ", " | Relay ")) + relay + ": " + onOff(relayOn[relay]) +
              (overridden[relay] ? L(" [elle]", " [manual]") : L(" [program]", " [schedule]"));
    }
    rolebot.serialWrite(line);
  }

  void printTable() {
    const char *daysTr[] = {"Paz", "Pzt", "Sal", "Çar", "Per", "Cum", "Cmt"};
    const char *daysEn[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    rolebot.serialWrite(L("---- Program tablosu ----", "---- Schedule table ----"));
    for (const Rule &r : kSchedule) {
      char line[80];
      snprintf(line, sizeof(line), L("Röle %d: %02d:%02d - %02d:%02d  ", "Relay %d: %02d:%02d - %02d:%02d  "),
               r.relay, r.onH, r.onM, r.offH, r.offM);
      String s = line;
      for (int d = 0; d < 7; d++) {
        if (r.days & (1 << d)) { s += turkish ? daysTr[d] : daysEn[d]; s += " "; }
      }
      rolebot.serialWrite(s);
    }
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
  rolebot.serialWrite(L("---- SAAT AYARLI RÖLE - Komutlar ----", "---- SCHEDULED RELAY - Commands ----"));
  rolebot.serialWrite(L("  yardim          : bu liste", "  help            : this list"));
  rolebot.serialWrite(L("  role1 ac/kapat  : Röle 1'i elle ayarla", "  relay1 on/off   : set Relay 1 by hand"));
  rolebot.serialWrite(L("  role2 ac/kapat  : Röle 2'yi elle ayarla", "  relay2 on/off   : set Relay 2 by hand"));
  rolebot.serialWrite(L("  degistir 1 / 2  : röleyi tersine çevir", "  toggle 1 / 2    : invert a relay"));
  rolebot.serialWrite(L("  hepsi kapat     : iki röleyi de kapat", "  all off         : turn both relays off"));
  rolebot.serialWrite(L("  program         : programa dön", "  schedule        : back to the schedule"));
  rolebot.serialWrite(L("  tablo           : program tablosu", "  table           : schedule table"));
  rolebot.serialWrite(L("  durum           : saat ve röleler", "  status          : time and relays"));
  rolebot.serialWrite(L("  guncelle        : saati yeniden al", "  update          : re-sync the time"));
  rolebot.serialWrite(L("  dil             : English'e geç", "  lang            : switch to Turkish"));
  rolebot.serialWrite(L("  B1 kısa bas     : Röle 1,  B1 1 sn bas: Röle 2", "  B1 short press  : Relay 1,  B1 hold 1 s: Relay 2"));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  int relay, action;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "program" || word == "schedule" || word == "oto" || word == "auto") {
    overridden[1] = overridden[2] = false;
    rolebot.serialWrite(L("Elle ayarlar bırakıldı, röleler programa göre çalışıyor.", "Overrides cleared, the relays follow the schedule."));
  } else if (word == "tablo" || word == "table") {
    printTable();
  } else if (word == "durum" || word == "status" || word == "saat" || word == "time") {
    printStatus();
  } else if (word == "guncelle" || word == "update") {
    rolebot.serialWrite(L("Saat güncelleniyor...", "Updating the time..."));
    bool ok = rolebot.ntpUpdate(); // En fazla 10 sn sürebilir / may take up to 10 s
    rolebot.serialWrite(ok ? String(L("Saat alındı: ", "Time synced: ")) + rolebot.ntpGetDateTimeString()
                           : String(L("Saat alınamadı (WiFi bağlı mı?).", "Could not get the time (is WiFi connected?).")));
  } else if (word == "dil" || word == "lang" || word == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else if (parseRelayCommand(cmd, relay, action)) {
    for (int r = 1; r <= 2; r++) {
      if (relay == 0 || relay == r) setOverride(r, action, L("seri komut", "serial command"));
    }
  } else {
    rolebot.serialWrite(String(L("Bilinmeyen komut: ", "Unknown command: ")) + cmd + L("  (yardim yazın)", "  (type help)"));
  }
}

// ---------------------------------------------------------------------------
void setup() {
  rolebot.begin();
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI başlar / safety: relays start OFF
  rolebot.Relay2Write(false);
  rolebot.serialStart(115200);
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS);
  // WiFi olmasa da çağırıyoruz: saat ayarı arka planda kalır, WiFi gelince saat kendiliğinden gelir.
  // Called even without WiFi: the time setup stays in the background and syncs once WiFi connects.
  bool ok = rolebot.ntpBegin(kTimezoneHours);
  rolebot.serialWrite(ok ? String(L("Saat alındı: ", "Time synced: ")) + rolebot.ntpGetDateTimeString()
                         : String(L("Saat henüz yok - röleler KAPALI (güvenli mod).", "No time yet - relays OFF (safe mode).")));
  printHelp();
  printTable();
}

void loop() {
  uint32_t now = millis();
  bool timeValid = rolebot.ntpIsTimeValid();
  struct tm t = {};
  if (timeValid) {
    time_t epoch = rolebot.ntpGetEpoch();
    localtime_r(&epoch, &t);
  }

  // 1) Program ne diyor? Program DEĞİŞİNCE elle ayar biter.
  // 1) What does the schedule say? When the schedule CHANGES, the manual override ends.
  for (int relay = 1; relay <= 2; relay++) {
    planned[relay] = timeValid && scheduleSays(relay, t); // Saat yoksa KAPALI / no time -> OFF
    if (planned[relay] != lastPlanned[relay]) {
      lastPlanned[relay] = planned[relay];
      overridden[relay] = false;
    }
    relayOn[relay] = overridden[relay] ? overrideValue[relay] : planned[relay];
  }
  rolebot.Relay1Write(relayOn[1]);
  rolebot.Relay2Write(relayOn[2]);

  // 2) B1: kısa basma = Röle 1, uzun basma (1 sn+) = Röle 2 (bırakınca karar verilir).
  // 2) B1: short press = Relay 1, long press (1 s+) = Relay 2 (decided on release).
  bool button = rolebot.button1Read(); // false = basılı / pressed
  if (lastButton && !button) pressStartMs = now;
  if (!lastButton && button && now - pressStartMs > 40) { // 40 ms parazit süzgeci / debounce
    setOverride(now - pressStartMs >= kLongPressMs ? 2 : 1, 2, L("B1 butonu", "B1 button"));
  }
  lastButton = button;

  // 3) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);

  // 4) LED: saat varsa yavaş, yoksa hızlı yanıp söner. / LED: slow blink with time, fast blink without.
  if (now - lastBlinkMs >= (timeValid ? 1000UL : 150UL)) {
    lastBlinkMs = now;
    ledState = !ledState;
    rolebot.ledWrite(ledState);
  }

  // 5) Her 10 saniyede bir durum raporu. / Status report every 10 seconds.
  if (now - lastStatusMs >= kStatusEveryMs) {
    lastStatusMs = now;
    printStatus();
  }
}
