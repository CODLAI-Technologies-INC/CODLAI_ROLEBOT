// TR: GERCEK PROJE - Tek Butonla Iki Role Kontrolu. ROLEBOT'ta tek bir
// buton var ama iki role var - bu ornek klasik bir gomulu sistem
// tekniginin ("tek tikla / cift tikla" ayrimi) nasil yapildigini
// gosterir: Butona TEK basip birakirsaniz Role 1 acilir/kapanir, HIZLI
// bir sekilde IKI kez basarsaniz (400ms icinde) Role 2 acilir/kapanir.
// EN: A REAL PROJECT - Controlling Two Relays with One Button. ROLEBOT
// only has one button but two relays - this example shows a classic
// embedded-systems technique (distinguishing a "single click" from a
// "double click"): a SINGLE press toggles Relay 1, a FAST double press
// (within 400ms) toggles Relay 2.

#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

namespace {
  constexpr uint32_t kDoubleClickWindowMs = 400;

  bool relay1On = false;
  bool relay2On = false;

  bool lastButtonState = true; // digitalRead: HIGH = birakilmis / released
  uint32_t lastReleaseMs = 0;
  bool waitingForSecondClick = false;
}

void setup() {
  rolebot.begin();
  rolebot.playIntro();
  rolebot.serialStart(115200);
  rolebot.Relay1Write(false);
  rolebot.Relay2Write(false);
  rolebot.serialWrite(turkish ? "Hazir: tek tikla Role 1, cift tikla Role 2."
                              : "Ready: single click = Relay 1, double click = Relay 2.");
}

void loop() {
  bool buttonState = rolebot.button1Read(); // false = basili / pressed

  // Butonun birakildigi ani yakala (kenar algila) / edge-detect the release
  if (lastButtonState == false && buttonState == true) {
    uint32_t now = millis();

    if (waitingForSecondClick && (now - lastReleaseMs) <= kDoubleClickWindowMs) {
      // Cift tik / double click
      waitingForSecondClick = false;
      relay2On = !relay2On;
      rolebot.Relay2Write(relay2On);
      rolebot.serialWrite(turkish ? ("Cift tik -> Role 2: " + String(relay2On ? "ACIK" : "KAPALI"))
                                  : ("Double click -> Relay 2: " + String(relay2On ? "ON" : "OFF")));
    } else {
      // Ilk tik - ikincisi gelecek mi diye bekle / first click - wait to see if a second one follows
      waitingForSecondClick = true;
      lastReleaseMs = now;
    }
  }

  // Bekleme suresi doldu ve ikinci tik gelmediyse: tek tik islemini uygula
  // The wait window elapsed with no second click: apply the single-click action
  if (waitingForSecondClick && (millis() - lastReleaseMs) > kDoubleClickWindowMs) {
    waitingForSecondClick = false;
    relay1On = !relay1On;
    rolebot.Relay1Write(relay1On);
    rolebot.serialWrite(turkish ? ("Tek tik -> Role 1: " + String(relay1On ? "ACIK" : "KAPALI"))
                                : ("Single click -> Relay 1: " + String(relay1On ? "ON" : "OFF")));
  }

  lastButtonState = buttonState;
  delay(10);
}
