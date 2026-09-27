// TR: COCUKLAR ICIN BASIT KABLOSUZ MESAJLASMA. Bir sayi mesaji alindiginda
// (ornegin baska bir kart bir sensor degeri gonderdiginde) deger belirli
// bir esigi asarsa Role 1'i tetikler - "kablosuz uzaktan komut" fikrinin
// en basit hali. Ayni zamanda her aldigi metin mesajini Seri Port'a
// yazdirir.
// EN: SIMPLE WIRELESS MESSAGING FOR KIDS. When a number message is
// received (e.g. another board sends a sensor value) and it crosses a
// threshold, it triggers Relay 1 - the simplest form of a "wireless
// remote command" idea. It also prints every text message it receives
// to the Serial port.
//
// ROLEBOT'ta LCD ekran YOK; tum bilgiler Seri Port (USB) uzerinden verilir.
// / ROLEBOT has NO LCD screen; all feedback is given through Serial (USB).

#define USE_ESPNOW
#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

constexpr float kNumberThreshold = 50.0f;

void setup() {
  rolebot.serialStart(115200);
  rolebot.espNowBegin(1); // Kanal 1 - gonderen diger kartla AYNI kanal olmali / channel 1 - must match the sending board's channel
  rolebot.Relay1Write(false);
  rolebot.serialWrite(turkish ? "Basit ESP-NOW mesajlasma hazir. Mesaj bekleniyor..."
                              : "Simple ESP-NOW messaging ready. Waiting for a message...");
}

void loop() {
  if (rolebot.espNowAvailable()) {
    String text = rolebot.espNowReadText();
    if (text.length() > 0) {
      rolebot.serialWrite(turkish ? ("Metin alindi: " + text) : ("Text received: " + text));
    }

    String name = rolebot.espNowReadName();
    float value = rolebot.espNowReadNumber();
    if (name.length() > 0) {
      rolebot.serialWrite(turkish ? (name + " = " + String(value)) : (name + " = " + String(value)));
      bool shouldTrigger = value > kNumberThreshold;
      rolebot.Relay1Write(shouldTrigger);
    }
  }

  delay(20);
}
