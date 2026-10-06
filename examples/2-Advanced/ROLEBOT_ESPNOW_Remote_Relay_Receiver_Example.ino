// TR: GERCEK PROJE - Kablosuz Role Alicisi. Bu ROLEBOT, IOTBOT kumanda
// panelinin ESP-NOW ile yayinladigi komutlari dinler: "role1" -> Role 1,
// "role2" -> Role 2, "led" -> mavi LED. Buton 1 ile Role 1'i elle de
// acip kapatabilirsiniz - panel kapali olsa bile calisir. Kural basit:
// "son komut kazanir" (panelde B3'e basilinca panel, buradaki butona
// basilinca buton). Once IOTBOT'a IOTBOT_ESPNOW_Remote_Control_Panel_
// Example.ino dosyasini yukleyin; istersen bir MINIBOT'a da
// MINIBOT_ESPNOW_Remote_Servo_Receiver_Example.ino dosyasini yukleyin.
// EN: A REAL PROJECT - Wireless Relay Receiver. This ROLEBOT listens to the
// commands the IOTBOT control panel broadcasts over ESP-NOW: "role1" ->
// Relay 1, "role2" -> Relay 2, "led" -> blue LED. Button 1 also toggles
// Relay 1 by hand - it works even when the panel is off. The rule is
// simple: "the last command wins" (the panel when B3 is pressed there, the
// button when it is pressed here). First upload IOTBOT_ESPNOW_Remote_Control_
// Panel_Example.ino to an IOTBOT; optionally upload
// MINIBOT_ESPNOW_Remote_Servo_Receiver_Example.ino to a MINIBOT.
//
// Baglanti / Wiring: Yukleri (lamba, fan...) rolelerin COM + NO uclarina
// baglayin: role cekince yuk calisir. Sebeke gerilimi (220V) ile SADECE bir
// yetiskin calissin. / Wire the loads (lamp, fan...) to the relays' COM + NO
// terminals: the load runs when the relay pulls in. Only an adult should
// work with mains voltage (220V).
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
  constexpr int kEspNowChannel = 1;   // IOTBOT ile AYNI kanal / SAME channel as the IOTBOT

  bool relay1 = false, relay2 = false, led = false;
  // Panel her 2 sn'de durumu TEKRAR gonderir. Sadece panel tarafinda bir
  // DEGISIKLIK olunca uyguluyoruz; yoksa tekrar mesaji butonla yaptiginiz
  // degisikligi hemen geri alirdi. (-1 = panelden henuz bir sey gelmedi)
  // The panel RESENDS its state every 2 s. We only act when the panel's value
  // CHANGES; otherwise a repeat would instantly undo what you did with the
  // button. (-1 = nothing received from the panel yet)
  int panelRelay1 = -1, panelRelay2 = -1, panelLed = -1;
  bool lastButton = true;             // HIGH = birakilmis / released
  uint32_t lastPressMs = 0;

  void report(const char *what, bool on, bool byButton) {
    String text = String(what) + ": " + (on ? (turkish ? "ACIK" : "ON") : (turkish ? "KAPALI" : "OFF"));
    text += byButton ? (turkish ? "  (Buton 1)" : "  (Button 1)") : "  (panel)";
    rolebot.serialWrite(text);
  }

  // Panelden gelen degeri, sadece panelin son degerinden FARKLIYSA uygular.
  // Applies the panel's value only if it DIFFERS from the panel's last value.
  bool panelChanged(int &lastPanel, float value, bool &state) {
    int v = value > 0.5f ? 1 : 0;
    if (v == lastPanel) return false;
    lastPanel = v;
    state = v;
    return true;
  }
}

void setup() {
  rolebot.begin();
  rolebot.serialStart(115200);
  rolebot.espNowBegin(kEspNowChannel);
  rolebot.Relay1Write(false);
  rolebot.Relay2Write(false);
  rolebot.ledWrite(false);
  rolebot.serialWrite(turkish ? "Role alicisi hazir. Panel komutu bekleniyor (Buton 1 = Role 1 elle)."
                              : "Relay receiver ready. Waiting for the panel (Button 1 = Relay 1 by hand).");
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

    if (name == "role1" && panelChanged(panelRelay1, value, relay1)) {
      rolebot.Relay1Write(relay1);
      report(turkish ? "Role 1" : "Relay 1", relay1, false);
    } else if (name == "role2" && panelChanged(panelRelay2, value, relay2)) {
      rolebot.Relay2Write(relay2);
      report(turkish ? "Role 2" : "Relay 2", relay2, false);
    } else if (name == "led" && panelChanged(panelLed, value, led)) {
      rolebot.ledWrite(led);
      report("LED", led, false);
    }
    // "servo" MINIBOT icindir, burada yok sayilir. / "servo" is for the MINIBOT and is ignored here.
  }

  // 2) Buton 1: Role 1'i elle ac/kapa (panel olmadan da calisir).
  // 2) Button 1: toggle Relay 1 by hand (works without the panel too).
  bool button = rolebot.button1Read(); // false = basili / pressed
  if (lastButton && !button && millis() - lastPressMs > 250) { // 250 ms debounce
    lastPressMs = millis();
    relay1 = !relay1;
    rolebot.Relay1Write(relay1);
    report(turkish ? "Role 1" : "Relay 1", relay1, true);
  }
  lastButton = button;

  delay(5);
}
