/*
 * TR: GERÇEK PROJE - Elektrik Kesintisinde Hafızalı Priz. Normal bir akıllı priz
 * elektrik kesilip geldiğinde hep KAPALI açılır. Bu örnekte Röle 1'in en son durumu
 * (açık/kapalı) her değiştiğinde EEPROM'a (kalıcı hafıza) yazılır; kart yeniden
 * başlatıldığında (örneğin elektrik kesintisinden sonra) rölenin son durumunu
 * hatırlayıp GERİ YÜKLER. (Bu yüzden bu örnekte Röle 1 bilerek son durumuyla başlar.)
 *  - B1 butonuna basınca Röle 1 açılır/kapanır ve durum hafızaya kaydedilir.
 *  - Mavi LED, Röle 1 açıkken yanar.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim             / help              -> komut listesi
 *      ac   (role1 ac)    / on  (relay1 on)   -> Röle 1'i aç ve kaydet
 *      kapat (role1 kapat)/ off (relay1 off)  -> Röle 1'i kapat ve kaydet
 *      degistir           / toggle            -> Röle 1'i tersine çevir ve kaydet
 *      oku                / read              -> hafızadaki değeri göster
 *      unut               / forget            -> hafızayı sil (sonraki açılış KAPALI)
 *      durum              / status            -> röle durumu
 *      dil                / lang              -> dili değiştir (Türkçe <-> English)
 *
 * EN: A REAL PROJECT - A Plug That Remembers Its State After a Power Cut. A normal
 * smart plug always comes back OFF after power returns. In this example, Relay 1's
 * last state (on/off) is written to EEPROM (persistent memory) every time it changes;
 * when the board restarts (e.g. after a power outage) it RESTORES the relay's last
 * state. (That is why Relay 1 deliberately starts in its last state in this example.)
 *  - Press B1 to toggle Relay 1; the state is saved to memory.
 *  - The blue LED is on while Relay 1 is on.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help               / yardim            -> command list
 *      on  (relay1 on)    / ac   (role1 ac)   -> turn Relay 1 on and save
 *      off (relay1 off)   / kapat(role1 kapat)-> turn Relay 1 off and save
 *      toggle             / degistir          -> invert Relay 1 and save
 *      read               / oku               -> show the value in memory
 *      forget             / unut              -> erase the memory (next boot = OFF)
 *      status             / durum             -> relay state
 *      lang               / dil               -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Yükü Röle 1'in COM + NO uçlarına bağlayın. Şebeke gerilimi (220V)
 * ile SADECE bir yetişkin çalışsın. / Wire the load to Relay 1's COM + NO terminals.
 * Only an adult should work with mains voltage (220V).
 * Not / Note: EEPROM'un yazma ömrü sınırlıdır; sadece durum DEĞİŞİNCE yazıyoruz.
 * / EEPROM has limited write cycles; we only write when the state CHANGES.
 */

#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

const int RELAY_STATE_ADDRESS = 0; // EEPROM'da rölenin durumunu tuttuğumuz adres / EEPROM address of the relay state

bool relayOn = false;
bool lastButtonState = true; // digitalRead: HIGH = bırakılmış / released
uint32_t lastEdgeMs = 0;     // Parazit süzgeci / debounce

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "AÇ" -> "ac"
// Lower-cases and simplifies Turkish letters: "AÇ" -> "ac"
String normalizeCommand(String s) {
  s.trim();
  s.replace("İ", "i"); s.replace("I", "i"); s.replace("ı", "i");
  s.replace("Ş", "s"); s.replace("ş", "s");
  s.replace("Ğ", "g"); s.replace("ğ", "g");
  s.replace("Ü", "u"); s.replace("ü", "u");
  s.replace("Ö", "o"); s.replace("ö", "o");
  s.replace("Ç", "c"); s.replace("ç", "c");
  s.toLowerCase();
  return s;
}

bool readCommand(String &cmd) {
  while (Serial.available() > 0) {
    char c = Serial.read();
    lastCharMs = millis();
    if (c == '\n' || c == '\r') {
      if (cmdBuffer.length() == 0) continue;
      cmd = normalizeCommand(cmdBuffer);
      cmdBuffer = "";
      return true;
    }
    if (cmdBuffer.length() < 40) cmdBuffer += c;
  }
  // "Satır sonu yok" seçiliyse: 150 ms sessizlikten sonra komutu kabul et.
  // "No line ending" selected: accept the command after 150 ms of silence.
  if (cmdBuffer.length() > 0 && millis() - lastCharMs > 150) {
    cmd = normalizeCommand(cmdBuffer);
    cmdBuffer = "";
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------------
const char *onOffText(bool on) { return on ? L("AÇIK", "ON") : L("KAPALI", "OFF"); }

// Röleyi ayarla ve durum DEĞİŞTİYSE hafızaya yaz / Set the relay and save it if the state CHANGED
void setRelay(bool on, const char *source) {
  bool changed = (on != relayOn);
  relayOn = on;
  rolebot.Relay1Write(relayOn);
  rolebot.ledWrite(relayOn);
  if (changed) {
    // eepromWriteByte kalıcı hafızaya hemen yazar (commit) / eepromWriteByte writes (commits) right away
    rolebot.eepromWriteByte(RELAY_STATE_ADDRESS, relayOn ? 1 : 0);
  }
  rolebot.serialWrite(String(source) + L(" -> Röle 1: ", " -> Relay 1: ") + onOffText(relayOn) +
                      (changed ? L(" (hafızaya kaydedildi)", " (saved to memory)") : L(" (zaten öyleydi)", " (already so)")));
}

void printHelp() {
  rolebot.serialWrite(L("---- HAFIZALI PRİZ - Komutlar ----", "---- MEMORY PLUG - Commands ----"));
  rolebot.serialWrite(L("  yardim     : bu liste", "  help       : this list"));
  rolebot.serialWrite(L("  ac         : Röle 1'i aç ve kaydet", "  on         : turn Relay 1 on and save"));
  rolebot.serialWrite(L("  kapat      : Röle 1'i kapat ve kaydet", "  off        : turn Relay 1 off and save"));
  rolebot.serialWrite(L("  degistir   : Röle 1'i tersine çevir", "  toggle     : invert Relay 1"));
  rolebot.serialWrite(L("  oku        : hafızadaki değer", "  read       : value in memory"));
  rolebot.serialWrite(L("  unut       : hafızayı sil", "  forget     : erase the memory"));
  rolebot.serialWrite(L("  durum      : röle durumu", "  status     : relay state"));
  rolebot.serialWrite(L("  dil        : English'e geç", "  lang       : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu  : Röle 1 aç/kapat", "  B1 button  : Relay 1 on/off"));
}

void handleCommand(const String &cmd) {
  // "role1 ac" / "relay 1 on" -> "ac" / "on" (bu örnekte sadece Röle 1 var / only Relay 1 here)
  String c = cmd;
  c.replace("role 1 ", ""); c.replace("relay 1 ", "");
  c.replace("role1 ", "");  c.replace("relay1 ", "");
  c.trim();

  if (c == "yardim" || c == "help" || c == "?") {
    printHelp();
  } else if (c == "ac" || c == "acik" || c == "on") {
    setRelay(true, L("Seri komut", "Serial command"));
  } else if (c == "kapat" || c == "kapa" || c == "kapali" || c == "off") {
    setRelay(false, L("Seri komut", "Serial command"));
  } else if (c == "degistir" || c == "toggle" || c == "role1" || c == "relay1") {
    setRelay(!relayOn, L("Seri komut", "Serial command"));
  } else if (c == "oku" || c == "read") {
    uint8_t saved = rolebot.eepromReadByte(RELAY_STATE_ADDRESS, 0);
    rolebot.serialWrite(String(L("Hafızadaki değer (adres 0): ", "Value in memory (address 0): ")) + saved +
                        L("  -> açılışta Röle 1 ", "  -> at boot Relay 1 is ") + onOffText(saved == 1));
  } else if (c == "unut" || c == "forget" || c == "sil" || c == "clear") {
    rolebot.eepromWriteByte(RELAY_STATE_ADDRESS, 0);
    rolebot.serialWrite(L("Hafıza silindi: bir sonraki açılışta Röle 1 KAPALI başlar.", "Memory erased: Relay 1 starts OFF at the next boot."));
  } else if (c == "durum" || c == "status") {
    rolebot.serialWrite(String(L("Röle 1: ", "Relay 1: ")) + onOffText(relayOn));
  } else if (c == "dil" || c == "lang" || c == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else {
    rolebot.serialWrite(String(L("Bilinmeyen komut: ", "Unknown command: ")) + cmd + L("  (yardim yazın)", "  (type help)"));
  }
}

// ---------------------------------------------------------------------------
void setup() {
  rolebot.begin();
  rolebot.Relay2Write(false); // Röle 2 bu örnekte kullanılmaz / Relay 2 is not used here
  rolebot.playIntro();
  rolebot.serialStart(115200);
  rolebot.eepromBegin(512);

  // Son kaydedilen durumu oku ve uygula / read and apply the last saved state
  uint8_t savedState = rolebot.eepromReadByte(RELAY_STATE_ADDRESS, 0);
  relayOn = (savedState == 1);
  rolebot.Relay1Write(relayOn);
  rolebot.ledWrite(relayOn);

  rolebot.serialWrite(String(L("Hafızadan geri yüklendi -> Röle 1: ", "Restored from memory -> Relay 1: ")) + onOffText(relayOn));
  rolebot.serialWrite(L("Şimdi elektriği kesip tekrar verin, rölenin aynı durumda kalacağını görün.",
                        "Now cut the power and reconnect it - watch the relay stay in the same state."));
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // B1: basıldığı an Röle 1'i değiştir (30 ms parazit süzgeci) / B1: toggle Relay 1 on press (30 ms debounce)
  bool buttonState = rolebot.button1Read(); // false = basılı / pressed
  if (buttonState != lastButtonState && now - lastEdgeMs >= 30) {
    lastEdgeMs = now;
    if (lastButtonState == true && buttonState == false) {
      setRelay(!relayOn, L("B1 butonu", "B1 button"));
    }
    lastButtonState = buttonState;
  }

  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
