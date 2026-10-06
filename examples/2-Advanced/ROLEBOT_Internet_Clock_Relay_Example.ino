// TR: INTERNET SAATI - Saate Gore Role. ROLEBOT WiFi'ye baglanip saati
// internetten (NTP) ceker:
//  - "Saat 19:00 ile 23:30 ARASINDA ISE" Role 1 (ornegin lamba) acik
//    (ntpTimeIsBetween).
//  - "Saat 07:00 ISE" Role 2 (ornegin bahce pompasi) o DAKIKA boyunca acik
//    (ntpTimeIs) - yani her sabah 1 dakika sulama.
//  - "Saat 12:00 OLUNCA" LED 3 kez yanip soner - BIR KEZ (ntpTimeReached).
//  - Saat her 6 saatte bir GUNCELLENIR (ntpUpdate).
// Bu fonksiyonlar editordeki "Internet saatini kullan / guncelle", "saat ...
// ise", "saat ... olunca" bloklarinin karsiligidir. Daha gelismis bir haftalik
// program icin: ROLEBOT_NTP_Scheduled_Relay_Example.ino
// EN: INTERNET TIME - Relays by the Clock. The ROLEBOT connects to WiFi and
// gets the time from the internet (NTP):
//  - "IF the time is BETWEEN 19:00 and 23:30" Relay 1 (e.g. a lamp) is on
//    (ntpTimeIsBetween).
//  - "IF it is 07:00" Relay 2 (e.g. a garden pump) is on for that MINUTE
//    (ntpTimeIs) - i.e. 1 minute of watering every morning.
//  - "WHEN it is 12:00" the LED blinks 3 times - ONCE (ntpTimeReached).
//  - The time is UPDATED every 6 hours (ntpUpdate).
// These functions are what the editor's "use / update internet time", "if
// time is ...", "when time is ..." blocks call. For a richer weekly schedule
// see ROLEBOT_NTP_Scheduled_Relay_Example.ino
//
// Baglanti / Wiring: Lamba Role 1'e, pompa Role 2'ye (220V baglantilarini bir
// yetiskin yapmalidir). Asagiya WiFi adinizi ve sifrenizi yazin.
// / Lamp on Relay 1, pump on Relay 2 (mains wiring must be done by an adult).
// Fill in your WiFi name and password below.

#define USE_WIFI
#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

namespace {
  constexpr int kTimezoneHours = 3;                              // Turkiye UTC+3 / Turkey UTC+3
  constexpr uint32_t kAutoUpdateMs = 6UL * 60UL * 60UL * 1000UL; // 6 saatte bir / every 6 hours
  uint32_t lastUpdateMs = 0;
  uint32_t lastPrintMs = 0;
}

void setup() {
  rolebot.begin();
  rolebot.serialStart(115200);
  rolebot.Relay1Write(false);
  rolebot.Relay2Write(false);
  Serial.println(turkish ? "WiFi'ye baglaniyor..." : "Connecting to WiFi...");
  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS);
  if (!rolebot.wifiConnectionControl()) {
    Serial.println(turkish ? "WiFi YOK - roleler kapali kalacak." : "NO WiFi - relays stay off.");
    return;
  }
  rolebot.ntpBegin(kTimezoneHours); // "Internet saatini kullan" / "use internet time"
  lastUpdateMs = millis();
}

void loop() {
  if (millis() - lastUpdateMs >= kAutoUpdateMs) {
    rolebot.ntpUpdate(); // "Internet saatini guncelle" / "update internet time"
    lastUpdateMs = millis();
  }

  // Saat henuz gelmediyse bu fonksiyonlar false doner -> roleler guvenle KAPALI kalir.
  // While there is no valid time these return false -> relays safely stay OFF.
  rolebot.Relay1Write(rolebot.ntpTimeIsBetween(19, 0, 23, 30));
  rolebot.Relay2Write(rolebot.ntpTimeIs(7, 0));

  if (rolebot.ntpTimeReached(12, 0)) {
    for (int i = 0; i < 3; i++) {
      rolebot.ledWrite(true);
      delay(200);
      rolebot.ledWrite(false);
      delay(200);
    }
  }

  if (millis() - lastPrintMs >= 10000) {
    lastPrintMs = millis();
    Serial.print(rolebot.ntpGetTimeString());
    Serial.print(turkish ? "  Role1: " : "  Relay1: ");
    Serial.print(rolebot.ntpTimeIsBetween(19, 0, 23, 30) ? "1" : "0");
    Serial.print(turkish ? "  Role2: " : "  Relay2: ");
    Serial.println(rolebot.ntpTimeIs(7, 0) ? "1" : "0");
  }
  delay(20);
}
