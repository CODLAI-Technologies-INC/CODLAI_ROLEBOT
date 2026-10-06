// TR: GERCEK PROJE - Kablosuz Deprem Uyari Agi (ROLEBOT ALICI). Bu ROLEBOT,
// IOTBOT'un ESP-NOW ile yayinladigi "deprem" mesajini dinler. "deprem = 1"
// gelince: Role 1 ACIL DURUM LAMBASINI yakar, Role 2 dogalgaz vanasini /
// ana elektrigi KESER ve LED yanip soner. "deprem = 0" gelince her sey eski
// haline doner. Buton 1 = yerel sifirlama (IOTBOT'a ulasilamasa bile burada
// alarmi bitirir). Once IOTBOT'a IOTBOT_ESPNOW_Earthquake_Alert_Sender_
// Example.ino, istersen bir MINIBOT'a da MINIBOT_ESPNOW_Earthquake_Alert_
// Receiver_Example.ino dosyasini yukleyin.
// EN: A REAL PROJECT - Wireless Earthquake Alert Network (ROLEBOT RECEIVER).
// This ROLEBOT listens for the "deprem" message the IOTBOT broadcasts over
// ESP-NOW. On "deprem = 1": Relay 1 turns the EMERGENCY LIGHT on, Relay 2
// CUTS the gas valve / main power and the LED blinks. On "deprem = 0"
// everything goes back to normal. Button 1 = local reset (ends the alarm
// here even if the IOTBOT cannot be reached). First upload IOTBOT_ESPNOW_
// Earthquake_Alert_Sender_Example.ino to an IOTBOT, and optionally
// MINIBOT_ESPNOW_Earthquake_Alert_Receiver_Example.ino to a MINIBOT.
//
// Baglanti / Wiring: Her rolenin 3 ucu vardir: COM (ortak), NO (normalde
// acik), NC (normalde kapali).
// - Role 1 (acil lamba): lambayi COM + NO uzerinden baglayin -> role
//   cekmeyince lamba sonuk, alarmda role cekince yanar.
// - Role 2 (gaz vanasi / ana hat): vananin beslemesini COM + NC uzerinden
//   baglayin -> normalde NC kapali, vana enerjili ve ACIK; alarmda role
//   ceker, NC acilir, vana enerjisiz kalip KAPANIR ("enerji kesilince
//   kapanan" tip vana kullanin).
// EN: Each relay has 3 terminals: COM (common), NO (normally open), NC
// (normally closed).
// - Relay 1 (emergency light): wire the lamp through COM + NO -> lamp is off
//   while the relay is idle, it lights when the relay pulls in during alarm.
// - Relay 2 (gas valve / main line): feed the valve through COM + NC ->
//   normally NC is closed, the valve is powered and OPEN; during alarm the
//   relay pulls in, NC opens and the valve loses power and CLOSES (use a
//   "closes when unpowered" type valve).
// DIKKAT / WARNING: Sebeke gerilimi (220V) ile SADECE bir yetiskin calissin;
// denemek icin 5V'luk bir LED veya kucuk motor kullanin. / Only an adult
// should work with mains voltage (220V); use a 5V LED or small motor to try it.
//
// ROLEBOT'ta LCD ekran YOK; tum bilgiler Seri Port (USB) uzerinden verilir.
// / ROLEBOT has NO LCD screen; all feedback is given through Serial (USB).

#define USE_ESPNOW
#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

namespace {
  constexpr int kEspNowChannel = 1;     // IOTBOT ile AYNI kanal / SAME channel as the IOTBOT
  constexpr uint32_t kBlinkMs = 250;    // LED yanip sonme hizi / LED blink speed

  bool alarmOn = false;
  bool ledState = false;
  uint32_t lastBlinkMs = 0;
  bool lastButton = true;               // HIGH = birakilmis / released

  void say(const char *turkishText, const char *englishText) {
    rolebot.serialWrite(turkish ? turkishText : englishText);
  }

  void setAlarm(bool on) {
    alarmOn = on;
    rolebot.Relay1Write(on); // Acil lamba (NO) / emergency light (NO)
    rolebot.Relay2Write(on); // Gaz vanasi / ana hat KES (NC) / CUT gas valve / main line (NC)
    if (!on) rolebot.ledWrite(false);
    if (on) say("!!! DEPREM !!! Acil lamba ACIK, gaz/elektrik KESILDI.",
                "!!! EARTHQUAKE !!! Emergency light ON, gas/power CUT.");
    else say("Normale donuldu: acil lamba kapali, gaz/elektrik geri verildi.",
             "Back to normal: emergency light off, gas/power restored.");
  }
}

void setup() {
  rolebot.begin();
  rolebot.serialStart(115200);
  rolebot.espNowBegin(kEspNowChannel);
  rolebot.Relay1Write(false);
  rolebot.Relay2Write(false);
  rolebot.ledWrite(false);
  say("Deprem alicisi (ROLEBOT) hazir - IOTBOT'tan uyari bekleniyor...",
      "Earthquake receiver (ROLEBOT) ready - waiting for an alert from the IOTBOT...");
}

void loop() {
  // 1) Gelen mesaj. NOT: espNowReadName() mesaji "okundu" isaretler; bu yuzden
  // sayiyi ONCE receivedData.value'dan aliyoruz, adi SONRA okuyoruz.
  // 1) Incoming message. NOTE: espNowReadName() marks the message as read, so
  // we take the number FIRST from receivedData.value and read the name AFTER.
  if (rolebot.espNowAvailable()) {
    rolebot.espNowReadText();                    // Metin mesajiysa at / drop it if it is a text message
    float value = rolebot.receivedData.value;
    String name = rolebot.espNowReadName();
    if (name == "deprem") {
      bool danger = value > 0.5f;
      if (danger != alarmOn) setAlarm(danger);   // Tekrar eden mesajlar bir sey degistirmez / repeats change nothing
    }
  }

  // 2) Buton 1 = yerel sifirlama. / Button 1 = local reset.
  bool button = rolebot.button1Read(); // false = basili / pressed
  if (lastButton && !button && alarmOn) {
    say("Yerel sifirlama (Buton 1).", "Local reset (Button 1).");
    setAlarm(false);
  }
  lastButton = button;

  // 3) Alarmda LED yanip soner (beklemeden). / LED blinks during alarm (non-blocking).
  if (alarmOn && millis() - lastBlinkMs >= kBlinkMs) {
    lastBlinkMs = millis();
    ledState = !ledState;
    rolebot.ledWrite(ledState);
  }

  delay(10);
}
