// TR: GERCEK PROJE - Elektrik Kesintisinde Hafizali Priz. Normal bir
// akilli priz elektrik kesilip geldiginde hep KAPALI acilir. Bu ornekte
// Role 1'in en son durumu (acik/kapali) her degistiginde EEPROM'a
// (kalici hafiza) yazilir; kart yeniden baslatildiginda (ornegin elektrik
// kesintisinden sonra) rolenin son durumunu hatirlayip GERI YUKLER.
// Butona basarak durumu degistirebilirsiniz.
// EN: A REAL PROJECT - A Plug That Remembers Its State After a Power Cut.
// A normal smart plug always comes back OFF after power returns. In this
// example, Relay 1's last state (on/off) is written to EEPROM
// (persistent memory) every time it changes; when the board restarts
// (e.g. after a power outage) it RESTORES the relay's last state. Press
// the button to change the state.

#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

namespace {
  constexpr int kRelayStateAddress = 0; // EEPROM'da rolenin durumunu tuttugumuz adres
  bool relayOn = false;
  bool lastButtonState = true; // digitalRead: HIGH = birakilmis / released
}

void setup() {
  rolebot.begin();
  rolebot.playIntro();
  rolebot.serialStart(115200);
  rolebot.eepromBegin(512);

  // Son kaydedilen durumu oku ve uygula / read and apply the last saved state
  uint8_t savedState = rolebot.eepromReadByte(kRelayStateAddress, 0);
  relayOn = (savedState == 1);
  rolebot.Relay1Write(relayOn);

  rolebot.serialWrite(turkish ? ("Hafizadan geri yuklendi -> Role: " + String(relayOn ? "ACIK" : "KAPALI"))
                              : ("Restored from memory -> Relay: " + String(relayOn ? "ON" : "OFF")));
  rolebot.serialWrite(turkish ? "Simdi elektrigi kesip tekrar verin, rolenin ayni durumda kalacagini gorun."
                              : "Now cut the power and reconnect it - watch the relay stay in the same state.");
}

void loop() {
  bool buttonState = rolebot.button1Read(); // false = basili / pressed

  if (lastButtonState == true && buttonState == false) {
    relayOn = !relayOn;
    rolebot.Relay1Write(relayOn);

    // Yeni durumu kalici hafizaya yaz / write the new state to persistent memory
    rolebot.eepromWriteByte(kRelayStateAddress, relayOn ? 1 : 0);
    rolebot.eepromCommit();

    rolebot.serialWrite(turkish ? ("Role: " + String(relayOn ? "ACIK" : "KAPALI") + " (hafizaya kaydedildi)")
                                : ("Relay: " + String(relayOn ? "ON" : "OFF") + " (saved to memory)"));
    delay(300); // debounce
  }
  lastButtonState = buttonState;

  delay(20);
}
