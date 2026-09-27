// TR: GERCEK PROJE - Zaman Ayarli Akilli Priz. Butona bir kere basinca
// Role 1 (ornegin bir lamba/sarj cihazi) acilir ve geri sayim baslar;
// sure dolunca cihaz KENDILIGINDEN kapanir - unutkanlik icin idealdir
// (ornegin sac duzlestirici ya da sarj cihazi icin). Geri sayim sirasinda
// tekrar butona basarsaniz cihazi erken kapatabilirsiniz. LED, role
// acikken yanip soner (ne kadar hizli yanip sonuyorsa suresi o kadar
// azaliyor demektir).
// EN: A REAL PROJECT - Smart Plug with Auto-Off Timer. Press the button
// once and Relay 1 (e.g. a lamp/charger) turns on and a countdown
// starts; when time runs out, the device turns off AUTOMATICALLY - ideal
// for forgetful moments (e.g. a hair straightener or a charger). Press
// the button again during the countdown to turn it off early. The LED
// blinks while the relay is on (the faster it blinks, the less time is
// left).

#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

namespace {
  // Egitim/test icin kisa tutuldu (30 saniye) - gercek kullanimda dakikaya
  // (60000UL * dakika) cikarabilirsiniz.
  // Kept short for teaching/testing (30 seconds) - for real use, scale it
  // up to minutes (60000UL * minutes).
  constexpr uint32_t kOnDurationMs = 30000;

  bool relayOn = false;
  uint32_t turnOffAtMs = 0;
  bool lastButtonState = true; // digitalRead: HIGH = birakilmis / released
  bool ledState = false;
  uint32_t lastBlinkMs = 0;
}

void setup() {
  rolebot.begin();
  rolebot.playIntro();
  rolebot.serialStart(115200);
  rolebot.Relay1Write(false);
  rolebot.ledWrite(false);
  rolebot.serialWrite(turkish ? "Akilli priz hazir. Baslatmak icin butona basin."
                              : "Smart plug ready. Press the button to start.");
}

void loop() {
  bool buttonState = rolebot.button1Read(); // false = basili / pressed

  if (lastButtonState == true && buttonState == false) {
    relayOn = !relayOn;
    rolebot.Relay1Write(relayOn);
    if (relayOn) {
      turnOffAtMs = millis() + kOnDurationMs;
      rolebot.serialWrite(turkish ? "Cihaz ACILDI. Otomatik kapanmaya " + String(kOnDurationMs / 1000) + " saniye var."
                                  : "Device ON. Auto-off in " + String(kOnDurationMs / 1000) + " seconds.");
    } else {
      rolebot.ledWrite(false);
      rolebot.serialWrite(turkish ? "Cihaz elle KAPATILDI." : "Device turned OFF manually.");
    }
    delay(300); // debounce
  }
  lastButtonState = buttonState;

  if (relayOn) {
    if (millis() >= turnOffAtMs) {
      // Sure doldu - otomatik kapat / time's up - auto shut off
      relayOn = false;
      rolebot.Relay1Write(false);
      rolebot.ledWrite(false);
      rolebot.serialWrite(turkish ? "Sure doldu, cihaz OTOMATIK kapatildi." : "Time's up, device turned OFF automatically.");
    } else {
      // Kalan sureye gore LED yanip sonme hizini ayarla / blink rate scales with remaining time
      uint32_t remainingMs = turnOffAtMs - millis();
      uint32_t blinkGapMs = map(remainingMs, 0, kOnDurationMs, 100, 800);
      if (millis() - lastBlinkMs >= blinkGapMs) {
        lastBlinkMs = millis();
        ledState = !ledState;
        rolebot.ledWrite(ledState);
      }
    }
  }

  delay(20);
}
