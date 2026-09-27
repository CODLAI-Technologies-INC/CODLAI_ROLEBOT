// TR: KABLOSUZ AKILLI EV FIKRI - OTOMATIK GECE LAMBASI. Bu ROLEBOT'un
// kendi isik sensoru YOK - bunun yerine, ayni odadaki bir IOTBOT'un
// yayinladigi (broadcast) isik sensoru verisini ESP-NOW ile dinler ve
// hava kararinca (isik degeri dusunce) roleye bagli GERCEK bir lambayi
// otomatik acar. Iki karti kablo OLMADAN, birbirinden bagimsiz ama
// birlikte calisir hale getiriyoruz. Once IOTBOT_ESPNOW_LightSensor_
// Broadcast_Example.ino dosyasini bir IOTBOT'a yukleyin, sonra bu kodu
// bir ROLEBOT'a yukleyin - IOTBOT'un isik sensorunu elinizle kapatinca
// ROLEBOT'un rolesi (ve baglıysa lambaniz) otomatik acilacak!
// EN: A WIRELESS SMART HOME IDEA - AUTOMATIC NIGHT LIGHT. This ROLEBOT
// has NO light sensor of its own - instead, it listens over ESP-NOW to
// the light sensor data broadcast by an IOTBOT in the same room, and
// automatically turns on a REAL lamp (wired to its relay) when it gets
// dark. We make two boards work together WITHOUT any wire between them.
// First upload IOTBOT_ESPNOW_LightSensor_Broadcast_Example.ino to an
// IOTBOT, then upload this code to a ROLEBOT - cover the IOTBOT's light
// sensor with your hand and watch the ROLEBOT's relay (and your lamp, if
// wired) turn on!
//
// ROLEBOT'ta LCD ekran YOK; tum bilgiler Seri Port (USB) uzerinden verilir.
// / ROLEBOT has NO LCD screen; all feedback is given through Serial (USB).

#define USE_ESPNOW
#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

// Bu esigin ALTINDAKI degerler "karanlik" sayilir - IOTBOT'unuzun ortam
// isigina gore ayarlayin. / Values BELOW this threshold count as "dark" -
// adjust to your IOTBOT's ambient light level.
constexpr int kDarkThreshold = 1500;

namespace {
  bool lampOn = false;

  void say(const char* turkishText, const char* englishText) {
    rolebot.serialWrite(turkish ? turkishText : englishText);
  }
}

void setup() {
  rolebot.begin();
  rolebot.serialStart(115200);
  rolebot.initESPNow();
  rolebot.startListening(); // Gelen IOTBOT yayinini rolebot.receivedData'ya yazar.
  rolebot.Relay1Write(false);

  say("Otomatik gece lambasi hazir - IOTBOT'tan isik verisi bekleniyor...",
      "Automatic night light ready - waiting for light data from IOTBOT...");
}

void loop() {
  if (rolebot.newData) {
    rolebot.newData = false;

    // Sadece IOTBOT'tan (deviceType 10) gelen veriyi isik sensoru olarak
    // kabul ediyoruz. / Only treat data from an IOTBOT (deviceType 10) as
    // a light sensor reading.
    if (rolebot.receivedData.deviceType == 10) {
      int lightValue = rolebot.receivedData.axis1;
      bool shouldBeOn = lightValue < kDarkThreshold;

      if (shouldBeOn != lampOn) {
        lampOn = shouldBeOn;
        rolebot.Relay1Write(lampOn);
        rolebot.ledWrite(lampOn);
        Serial.print(turkish ? "Isik degeri: " : "Light value: ");
        Serial.print(lightValue);
        Serial.print(" -> ");
        Serial.println(lampOn ? (turkish ? "LAMBA ACILDI (karanlik)" : "LAMP ON (dark)")
                               : (turkish ? "LAMBA KAPANDI (aydinlik)" : "LAMP OFF (bright)"));
      }
    }
  }
}
