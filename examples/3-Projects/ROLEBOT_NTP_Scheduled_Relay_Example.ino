// TR: GERCEK PROJE - Saat Ayarli Akilli Role (Haftalik Program). ROLEBOT
// WiFi'ye baglanir, internetten (NTP) dogru saati alir ve rolelerini bir
// PROGRAM TABLOSUNA gore kendiliginden acar/kapatir: ornegin Role 1 (salon
// lambasi) her aksam 19:00-23:30, Role 2 (bahce pompasi) Pazartesi,
// Carsamba ve Cuma sabahlari 07:00-07:15. Buton 1 ile programi ELLE
// gecersiz kilabilirsiniz (kisa basma = Role 1, uzun basma = Role 2); bu
// elle ayar, programdaki bir sonraki degisime kadar gecerlidir. Saat
// bilinmiyorsa guvenlik icin program rolelerini KAPALI tutar.
// EN: A REAL PROJECT - Time-Scheduled Smart Relay (Weekly Program). The
// ROLEBOT connects to WiFi, gets the correct time from the internet (NTP)
// and switches its relays on/off BY ITSELF following a SCHEDULE TABLE: e.g.
// Relay 1 (living room lamp) every evening 19:00-23:30, Relay 2 (garden
// pump) Monday, Wednesday and Friday mornings 07:00-07:15. Button 1
// OVERRIDES the schedule by hand (short press = Relay 1, long press = Relay
// 2); the override lasts until the next change in the schedule. If the time
// is unknown, the schedule keeps the relays OFF for safety.
//
// Baglanti / Wiring: Yukleri rolelerin COM + NO uclarina baglayin. Sebeke
// gerilimi (220V) ile SADECE bir yetiskin calissin. Asagiya WiFi adinizi ve
// sifrenizi yazin. / Wire the loads to the relays' COM + NO terminals. Only
// an adult should work with mains voltage (220V). Enter your WiFi name and
// password below.
//
// ROLEBOT'ta LCD ekran YOK; tum bilgiler Seri Port (USB) uzerinden verilir.
// / ROLEBOT has NO LCD screen; all feedback is given through Serial (USB).

#define USE_WIFI
#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

#define WIFI_SSID "YOUR_WIFI_SSID"     // WiFi adiniz / your WiFi name
#define WIFI_PASS "YOUR_WIFI_PASSWORD" // WiFi sifreniz / your WiFi password

namespace {
  constexpr int kTimezoneHours = 3;        // Turkiye = UTC+3 (yaz saati yok) / Turkey = UTC+3 (no DST)
  constexpr uint32_t kStatusEveryMs = 10000;
  constexpr uint32_t kLongPressMs = 1000;  // Bundan uzun basma = Role 2 / longer press = Relay 2

  // Gun bitleri (tm_wday: 0 = Pazar). / Day bits (tm_wday: 0 = Sunday).
  constexpr uint8_t kSun = 1, kMon = 2, kTue = 4, kWed = 8, kThu = 16, kFri = 32, kSat = 64;
  constexpr uint8_t kEveryDay = 127;

  struct Rule { uint8_t relay; uint8_t days; uint8_t onH, onM, offH, offM; };

  // PROGRAM TABLOSU - satir ekleyip degistirerek kendi programinizi yapin.
  // SCHEDULE TABLE - add/change rows to build your own program.
  // Kapanis saati acilistan SONRA olmali; gece yarisini gecen bir program icin
  // iki satir yazin (orn. 22:00-24:00 ve 00:00-06:00). / The off time must be
  // AFTER the on time; for a program past midnight write two rows (e.g.
  // 22:00-24:00 and 00:00-06:00).
  const Rule kSchedule[] = {
    {1, kEveryDay,         19, 0, 23, 30}, // Role 1: lamba / Relay 1: lamp
    {2, kMon | kWed | kFri,  7, 0,  7, 15}, // Role 2: bahce pompasi / Relay 2: garden pump
  };

  bool planned[3], lastPlanned[3], overridden[3], overrideValue[3], relayOn[3]; // [1] ve [2] kullanilir / [1] and [2] are used
  bool lastButton = true;                  // HIGH = birakilmis / released
  uint32_t pressStartMs = 0, lastStatusMs = 0, lastBlinkMs = 0;
  bool ledState = false;

  // Bu role icin program su an "acik" diyor mu? / Does the schedule say "on" right now for this relay?
  bool scheduleSays(int relay, const struct tm &t) {
    int nowMin = t.tm_hour * 60 + t.tm_min;
    for (const Rule &r : kSchedule) {
      if (r.relay != relay || !(r.days & (1 << t.tm_wday))) continue;
      int onMin = r.onH * 60 + r.onM, offMin = r.offH * 60 + r.offM;
      if (nowMin >= onMin && nowMin < offMin) return true;
    }
    return false;
  }

  const char *onOff(bool on) { return on ? (turkish ? "ACIK" : "ON") : (turkish ? "KAPALI" : "OFF"); }

  void toggleOverride(int relay) {
    overridden[relay] = true;
    overrideValue[relay] = !relayOn[relay];
    rolebot.serialWrite(String(turkish ? "ELLE: Role " : "MANUAL: Relay ") + relay + " -> " + onOff(overrideValue[relay]) +
                        (turkish ? " (programdaki bir sonraki degisime kadar)" : " (until the next schedule change)"));
  }
}

void setup() {
  rolebot.begin();
  rolebot.serialStart(115200);
  rolebot.Relay1Write(false);
  rolebot.Relay2Write(false);
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS);
  // WiFi olmasa da cagiriyoruz: saat ayari arka planda kalir, WiFi gelince saat kendiliginden gelir.
  // Called even without WiFi: the time setup stays in the background and syncs once WiFi connects.
  bool ok = rolebot.ntpBegin(kTimezoneHours);
  rolebot.serialWrite(ok ? (turkish ? "Saat alindi: " : "Time synced: ") + rolebot.ntpGetDateTimeString()
                         : String(turkish ? "Saat henuz yok - roleler KAPALI (guvenli mod)." : "No time yet - relays OFF (safe mode)."));
}

void loop() {
  uint32_t now = millis();
  bool timeValid = rolebot.ntpIsTimeValid();
  struct tm t = {};
  if (timeValid) {
    time_t epoch = rolebot.ntpGetEpoch();
    localtime_r(&epoch, &t);
  }

  // 1) Program ne diyor? Program DEGISINCE elle ayar biter.
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

  // 2) Buton 1: kisa basma = Role 1, uzun basma (1 sn+) = Role 2 (birakinca karar verilir).
  // 2) Button 1: short press = Relay 1, long press (1 s+) = Relay 2 (decided on release).
  bool button = rolebot.button1Read(); // false = basili / pressed
  if (lastButton && !button) pressStartMs = now;
  if (!lastButton && button && now - pressStartMs > 40) { // 40 ms debounce
    toggleOverride(now - pressStartMs >= kLongPressMs ? 2 : 1);
  }
  lastButton = button;

  // 3) LED: saat varsa yavas, yoksa hizli yanip soner. / LED: slow blink with time, fast blink without.
  if (now - lastBlinkMs >= (timeValid ? 1000UL : 150UL)) {
    lastBlinkMs = now;
    ledState = !ledState;
    rolebot.ledWrite(ledState);
  }

  // 4) Her 10 saniyede bir durum raporu. / Status report every 10 seconds.
  if (now - lastStatusMs >= kStatusEveryMs) {
    lastStatusMs = now;
    String line = timeValid ? rolebot.ntpGetDateTimeString() : String(turkish ? "SAAT YOK (guvenli mod)" : "NO TIME (safe mode)");
    for (int relay = 1; relay <= 2; relay++) {
      line += String(turkish ? " | Role " : " | Relay ") + relay + ": " + onOff(relayOn[relay]) +
              (overridden[relay] ? (turkish ? " [elle]" : " [manual]") : (turkish ? " [program]" : " [schedule]"));
    }
    rolebot.serialWrite(line);
  }

  delay(10);
}
