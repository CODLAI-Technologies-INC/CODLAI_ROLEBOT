// TR: ESP-NOW'A ILK ADIM - en basit kablosuz haberlesme ornegi. Bir MAC
// adresi bilmenize gerek YOK: bu kod bir sayaci "yayin" (broadcast)
// olarak havaya gonderir, ve aninda ayni odada ESP-NOW ile dinleyen
// HERHANGI bir CODLAI karti (baska bir ROLEBOT, bir IOTBOT ya da bir
// MINIBOT - hepsi ayni veri yapisini kullanir) bunu duyabilir. Ayni anda
// hem gonderiyor hem dinliyoruz.
// EN: FIRST STEP INTO ESP-NOW - the simplest wireless example. You do
// NOT need to know any MAC address: this code broadcasts a counter into
// the air, and ANY nearby CODLAI board listening over ESP-NOW (another
// ROLEBOT, an IOTBOT, or a MINIBOT - they all share the same data
// structure) can hear it. We both send AND listen at the same time.
//
// ROLEBOT'ta LCD ekran YOK; tum bilgiler Seri Port (USB) uzerinden verilir.
// / ROLEBOT has NO LCD screen; all feedback is given through Serial (USB).
// Yayin alindiginda LED kisa bir sure yanip soner. / The LED blinks
// briefly whenever a broadcast is received.

#define USE_ESPNOW
#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

namespace {
  uint32_t counter = 0;
  uint32_t lastSendMs = 0;
  constexpr uint32_t kSendIntervalMs = 1000;

  void say(const char* turkishText, const char* englishText) {
    rolebot.serialWrite(turkish ? turkishText : englishText);
  }
}

void setup() {
  rolebot.begin();
  rolebot.serialStart(115200);
  say("ESP-NOW yayin modu baslatiliyor...", "Starting ESP-NOW broadcast mode...");

  rolebot.initESPNow();
  rolebot.startListening(); // Gelen HERHANGI bir yayini rolebot.receivedData'ya yazar.

  say("Yayin modu hazir - herkese aciyoruz!", "Broadcast mode ready - open to everyone!");
}

void loop() {
  // ---- Gonderim: her saniye sayaci yayinla / Sending: broadcast counter every second ----
  if (millis() - lastSendMs >= kSendIntervalMs) {
    lastSendMs = millis();
    counter++;
    CodlaiESPNowMessage outgoing;
    outgoing.deviceType = 30; // 30 = ROLEBOT (bu ornekte kullanilan kimlik / id used in this example)
    outgoing.axis1 = counter;
    outgoing.axis2 = 0;
    outgoing.axis3 = 0;
    outgoing.gripper = 0;
    outgoing.action = 0;
    rolebot.sendESPNow(broadcastAddress, (uint8_t *)&outgoing, sizeof(outgoing));

    Serial.print(turkish ? "Gonderilen: " : "Sent: ");
    Serial.println(counter);
  }

  // ---- Alis: baska bir karttan gelen HERHANGI bir yayin / Receiving: ANY broadcast from another board ----
  if (rolebot.newData) {
    rolebot.newData = false;
    const char* senderName = "?";
    switch (rolebot.receivedData.deviceType) {
      case 10: senderName = "IOTBOT"; break;
      case 20: senderName = "MINIBOT"; break;
      case 30: senderName = "ROLEBOT"; break;
      default: break;
    }
    Serial.print(turkish ? "Yayin alindi -> gonderen: " : "Broadcast received -> from: ");
    Serial.print(senderName);
    Serial.print(turkish ? ", deger: " : ", value: ");
    Serial.println(rolebot.receivedData.axis1);
    rolebot.ledWrite(true);
    delay(50);
    rolebot.ledWrite(false);
  }
}
