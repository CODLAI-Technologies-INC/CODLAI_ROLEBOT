// TR: KABLOSUZ AKILLI EV FIKRI - OTOMATIK VANTILATOR. Bu ROLEBOT'un
// kendi sicaklik sensoru YOK - bunun yerine, ayni odadaki bir IOTBOT'un
// yayinladigi (broadcast) DHT sicaklik verisini ESP-NOW ile dinler ve
// sicaklik yukselince roleye bagli GERCEK bir vantilatoru/fani otomatik
// acar. Once IOTBOT_ESPNOW_Temperature_Broadcast_Example.ino dosyasini
// bir IOTBOT'a yukleyin, sonra bu kodu bir ROLEBOT'a yukleyin - IOTBOT'un
// DHT sensorunu elinizle isitinca ROLEBOT'un rolesi (ve baglıysa
// vantilatorunuz) otomatik acilacak!
// EN: A WIRELESS SMART HOME IDEA - AUTOMATIC FAN. This ROLEBOT has NO
// temperature sensor of its own - instead, it listens over ESP-NOW to
// the DHT temperature data broadcast by an IOTBOT in the same room, and
// automatically turns on a REAL fan (wired to its relay) when it gets
// hot. First upload IOTBOT_ESPNOW_Temperature_Broadcast_Example.ino to
// an IOTBOT, then upload this code to a ROLEBOT - warm up the IOTBOT's
// DHT sensor with your hand and watch the ROLEBOT's relay (and your fan,
// if wired) turn on!
//
// ROLEBOT'ta LCD ekran YOK; tum bilgiler Seri Port (USB) uzerinden verilir.
// / ROLEBOT has NO LCD screen; all feedback is given through Serial (USB).

#define USE_ESPNOW
#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

// Bu esigin UZERINDEKI degerler "sicak" sayilir - ortaminiza gore ayarlayin.
// Values ABOVE this threshold count as "hot" - adjust to your environment.
constexpr int kHotThresholdC = 28;

namespace {
  bool fanOn = false;

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

  say("Otomatik vantilator hazir - IOTBOT'tan sicaklik verisi bekleniyor...",
      "Automatic fan ready - waiting for temperature data from IOTBOT...");
}

void loop() {
  if (rolebot.newData) {
    rolebot.newData = false;

    // Sadece IOTBOT sicaklik yayinindan (deviceType 11) gelen veriyi kabul
    // ediyoruz. / Only treat data from an IOTBOT temperature broadcast
    // (deviceType 11) as a temperature reading.
    if (rolebot.receivedData.deviceType == 11) {
      int tempC = rolebot.receivedData.axis1;
      bool shouldBeOn = tempC > kHotThresholdC;

      if (shouldBeOn != fanOn) {
        fanOn = shouldBeOn;
        rolebot.Relay1Write(fanOn);
        rolebot.ledWrite(fanOn);
        Serial.print(turkish ? "Sicaklik: " : "Temperature: ");
        Serial.print(tempC);
        Serial.print(" C -> ");
        Serial.println(fanOn ? (turkish ? "VANTILATOR ACILDI (sicak)" : "FAN ON (hot)")
                              : (turkish ? "VANTILATOR KAPANDI (serin)" : "FAN OFF (cool)"));
      }
    }
  }
}
