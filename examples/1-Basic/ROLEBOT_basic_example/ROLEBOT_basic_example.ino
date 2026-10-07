/*
 * TR: BUTON VE LED TESTİ
 *  - B1 butonuna (karttaki tek buton) bastığınızda mavi LED yanar, bıraktığınızda söner.
 *  - Buton her basıldığında / bırakıldığında Seri Port'a BİR KEZ yazılır (ekranı doldurmaz)
 *    ve basış sayısı sayılır.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim      / help      -> komut listesi
 *      oku         / read      -> butonun şu anki durumu ve basış sayısı
 *      led ac      / led on    -> LED'i aç (buton bırakılmışken yanık kalır)
 *      led kapat   / led off   -> LED'i kapat
 *      sifirla     / reset     -> basış sayacını sıfırla
 *      dil         / lang      -> dili değiştir (Türkçe <-> English)
 *
 * EN: BUTTON AND LED TEST
 *  - Press the B1 button (the only button on the board) and the blue LED lights up;
 *    release it and the LED goes off.
 *  - Every press / release is printed ONCE to the Serial port (no flooding) and the
 *    presses are counted.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help        / yardim     -> command list
 *      read        / oku        -> current button state and press count
 *      led on      / led ac     -> turn the LED on (stays on while the button is released)
 *      led off     / led kapat  -> turn the LED off
 *      reset       / sifirla    -> reset the press counter
 *      lang        / dil        -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. B1 = GPIO0 (basılıyken LOW), mavi LED = GPIO16.
 * / No wiring needed. B1 = GPIO0 (LOW while pressed), blue LED = GPIO16.
 */

#include <ROLEBOT.h> // ROLEBOT kütüphanesi / ROLEBOT library

ROLEBOT rolebot; // ROLEBOT nesnesi / ROLEBOT object

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

bool lastPressed = false;  // Butonun son durumu / last button state
uint32_t lastChangeMs = 0; // Son değişim zamanı (parazit süzgeci) / last change time (debounce)
uint32_t pressCount = 0;   // Basış sayısı / number of presses
bool ledByCommand = false; // Seri komutla açılan LED / LED turned on by a serial command

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
void printHelp() {
  rolebot.serialWrite(L("---- BUTON VE LED TESTİ - Komutlar ----", "---- BUTTON AND LED TEST - Commands ----"));
  rolebot.serialWrite(L("  yardim      : bu liste", "  help        : this list"));
  rolebot.serialWrite(L("  oku         : buton durumu ve basış sayısı", "  read        : button state and press count"));
  rolebot.serialWrite(L("  led ac      : LED'i aç", "  led on      : turn the LED on"));
  rolebot.serialWrite(L("  led kapat   : LED'i kapat", "  led off     : turn the LED off"));
  rolebot.serialWrite(L("  sifirla     : sayacı sıfırla", "  reset       : reset the counter"));
  rolebot.serialWrite(L("  dil         : English'e geç", "  lang        : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu   : basılıyken LED yanar", "  B1 button   : LED is on while pressed"));
}

void updateLed() {
  rolebot.ledWrite(lastPressed || ledByCommand); // Buton basılıysa ya da komutla açıldıysa yanar / on if pressed or turned on by command
}

void printState() {
  rolebot.serialWrite(String(L("Buton: ", "Button: ")) + (lastPressed ? L("BASILI", "PRESSED") : L("SERBEST", "RELEASED")) +
                      L("  | Basış sayısı: ", "  | Press count: ") + pressCount +
                      L("  | LED: ", "  | LED: ") + ((lastPressed || ledByCommand) ? L("AÇIK", "ON") : L("KAPALI", "OFF")));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  String arg = (space < 0) ? String("") : cmd.substring(space + 1);
  arg.trim();

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "oku" || word == "read" || word == "durum" || word == "status") {
    printState();
  } else if (word == "led" && (arg == "ac" || arg == "on")) {
    ledByCommand = true;
    updateLed();
    rolebot.serialWrite(L("LED açıldı.", "LED turned on."));
  } else if (word == "led" && (arg == "kapat" || arg == "off")) {
    ledByCommand = false;
    updateLed();
    rolebot.serialWrite(L("LED kapatıldı.", "LED turned off."));
  } else if (word == "sifirla" || word == "reset") {
    pressCount = 0;
    rolebot.serialWrite(L("Sayaç sıfırlandı.", "Counter reset."));
  } else if (word == "dil" || word == "lang" || word == "language") {
    turkish = !turkish;
    rolebot.serialWrite(L("Dil: Türkçe", "Language: English"));
    printHelp();
  } else {
    rolebot.serialWrite(String(L("Bilinmeyen komut: ", "Unknown command: ")) + cmd + L("  (yardim yazın)", "  (type help)"));
  }
}

// ---------------------------------------------------------------------------
void setup() {
  rolebot.begin();             // ROLEBOT başlatılıyor / Initialize ROLEBOT
  rolebot.Relay1Write(false);  // Güvenlik: röleler KAPALI / safety: relays OFF
  rolebot.Relay2Write(false);
  rolebot.playIntro();         // LED 3 kez yanıp söner / LED blinks 3 times
  rolebot.serialStart(115200); // Seri haberleşme / Serial communication
  rolebot.serialWrite(L("ROLEBOT buton testi başladı. B1'e basın!", "ROLEBOT button test started. Press B1!"));
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // 1) Buton: sadece durum DEĞİŞİNCE yaz (30 ms parazit süzgeci)
  // 1) Button: print only when the state CHANGES (30 ms debounce)
  bool pressed = !rolebot.button1Read(); // button1Read() basılıyken LOW (false) / LOW (false) while pressed
  if (pressed != lastPressed && now - lastChangeMs >= 30) {
    lastChangeMs = now;
    lastPressed = pressed;
    if (pressed) {
      pressCount++;
      rolebot.serialWrite(String(L("Butona basıldı! (", "Button pressed! (")) + pressCount + L(". kez)", " times)"));
    } else {
      rolebot.serialWrite(L("Buton serbest.", "Button released."));
    }
    updateLed();
  }

  // 2) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
