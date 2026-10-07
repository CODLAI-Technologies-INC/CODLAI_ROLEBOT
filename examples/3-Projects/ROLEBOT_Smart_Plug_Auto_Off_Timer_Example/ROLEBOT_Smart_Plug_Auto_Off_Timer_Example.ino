/*
 * TR: GERÇEK PROJE - Zaman Ayarlı Akıllı Priz. Butona bir kere basınca Röle 1
 * (örneğin bir lamba/şarj cihazı) açılır ve geri sayım başlar; süre dolunca cihaz
 * KENDİLİĞİNDEN kapanır - unutkanlık için idealdir (örneğin saç düzleştirici ya da
 * şarj cihazı için). Geri sayım sırasında tekrar butona basarsanız cihazı erken
 * kapatabilirsiniz. LED, röle açıkken yanıp söner (ne kadar hızlı yanıp sönüyorsa
 * kalan süre o kadar azalıyor demektir).
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim             / help             -> komut listesi
 *      baslat (ac)        / start (on)       -> cihazı aç, geri sayımı başlat
 *      durdur (kapat)     / stop (off)       -> cihazı hemen kapat
 *      sure 60            / time 60          -> açık kalma süresi (saniye, 5-3600)
 *      kalan              / left             -> kalan süre
 *      dil                / lang             -> dili değiştir (Türkçe <-> English)
 *
 * EN: A REAL PROJECT - Smart Plug with Auto-Off Timer. Press the button once and
 * Relay 1 (e.g. a lamp/charger) turns on and a countdown starts; when time runs out,
 * the device turns off AUTOMATICALLY - ideal for forgetful moments (e.g. a hair
 * straightener or a charger). Press the button again during the countdown to turn it
 * off early. The LED blinks while the relay is on (the faster it blinks, the less time
 * is left).
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help               / yardim           -> command list
 *      start (on)         / baslat (ac)      -> turn the device on, start the countdown
 *      stop (off)         / durdur (kapat)   -> turn the device off now
 *      time 60            / sure 60          -> on-time (seconds, 5-3600)
 *      left               / kalan            -> remaining time
 *      lang               / dil              -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Cihazı Röle 1'in COM + NO uçlarına bağlayın. Şebeke gerilimi
 * (220V) ile SADECE bir yetişkin çalışsın. / Wire the device to Relay 1's COM + NO
 * terminals. Only an adult should work with mains voltage (220V).
 */

#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

// Eğitim/test için kısa tutuldu (30 saniye) - gerçek kullanımda dakikaya çıkarabilirsiniz
// ("sure 600" = 10 dakika). / Kept short for teaching/testing (30 seconds) - for real
// use, scale it up to minutes ("time 600" = 10 minutes).
uint32_t onDurationMs = 30000;

bool relayOn = false;
uint32_t startMs = 0;        // Rölenin açıldığı an / when the relay turned on
bool lastButtonState = true; // digitalRead: HIGH = bırakılmış / released
uint32_t lastEdgeMs = 0;     // Parazit süzgeci / debounce
bool ledState = false;
uint32_t lastBlinkMs = 0;
uint32_t lastReportMs = 0;

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
uint32_t remainingSeconds() {
  uint32_t elapsed = millis() - startMs;
  return elapsed >= onDurationMs ? 0 : (onDurationMs - elapsed + 999) / 1000;
}

void turnOn(const char *source) {
  relayOn = true;
  startMs = millis();
  lastReportMs = startMs;
  rolebot.Relay1Write(true);
  rolebot.serialWrite(String(source) + L(" -> Cihaz AÇILDI. Otomatik kapanmaya ", " -> Device ON. Auto-off in ") +
                      (onDurationMs / 1000) + L(" saniye var.", " seconds."));
}

void turnOff(const char *message) {
  relayOn = false;
  rolebot.Relay1Write(false);
  ledState = false;
  rolebot.ledWrite(false);
  rolebot.serialWrite(message);
}

void printHelp() {
  rolebot.serialWrite(L("---- ZAMAN AYARLI PRİZ - Komutlar ----", "---- TIMER PLUG - Commands ----"));
  rolebot.serialWrite(L("  yardim     : bu liste", "  help       : this list"));
  rolebot.serialWrite(L("  baslat     : cihazı aç, geri sayımı başlat", "  start      : turn on, start the countdown"));
  rolebot.serialWrite(L("  durdur     : cihazı hemen kapat", "  stop       : turn off now"));
  rolebot.serialWrite(L("  sure 60    : açık kalma süresi (sn)", "  time 60    : on-time (s)"));
  rolebot.serialWrite(L("  kalan      : kalan süre", "  left       : remaining time"));
  rolebot.serialWrite(L("  dil        : English'e geç", "  lang       : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu  : başlat / erken kapat", "  B1 button  : start / turn off early"));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  bool hasValue = space > 0;
  long value = hasValue ? cmd.substring(space + 1).toInt() : 0;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "baslat" || word == "start" || word == "ac" || word == "on") {
    turnOn(L("Seri komut", "Serial command")); // Açıksa süreyi baştan başlatır / restarts the timer if already on
  } else if (word == "durdur" || word == "stop" || word == "kapat" || word == "off") {
    turnOff(L("Cihaz seri komutla KAPATILDI.", "Device turned OFF by serial command."));
  } else if ((word == "sure" || word == "time") && hasValue) {
    onDurationMs = (uint32_t)constrain(value, 5L, 3600L) * 1000UL;
    rolebot.serialWrite(String(L("Açık kalma süresi: ", "On-time: ")) + (onDurationMs / 1000) + L(" saniye", " seconds"));
  } else if (word == "kalan" || word == "left" || word == "durum" || word == "status") {
    if (relayOn) rolebot.serialWrite(String(L("Cihaz AÇIK, kalan süre: ", "Device ON, time left: ")) + remainingSeconds() + L(" sn", " s"));
    else rolebot.serialWrite(L("Cihaz KAPALI.", "Device OFF."));
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
  rolebot.begin();
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI başlar / safety: relays start OFF
  rolebot.Relay2Write(false);
  rolebot.playIntro();
  rolebot.serialStart(115200);
  rolebot.ledWrite(false);
  rolebot.serialWrite(L("Akıllı priz hazır. Başlatmak için B1'e basın (ya da \"baslat\" yazın).",
                        "Smart plug ready. Press B1 (or type \"start\") to begin."));
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // 1) B1: basıldığı an aç / kapat (30 ms parazit süzgeci) / B1: on / off on press (30 ms debounce)
  bool buttonState = rolebot.button1Read(); // false = basılı / pressed
  if (buttonState != lastButtonState && now - lastEdgeMs >= 30) {
    lastEdgeMs = now;
    if (lastButtonState == true && buttonState == false) {
      if (relayOn) turnOff(L("Cihaz elle KAPATILDI.", "Device turned OFF manually."));
      else turnOn(L("B1 butonu", "B1 button"));
    }
    lastButtonState = buttonState;
  }

  // 2) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);

  // 3) Geri sayım. "now - startMs" farkı millis() taşmasında da doğru çalışır.
  // 3) Countdown. The "now - startMs" difference works even when millis() overflows.
  if (relayOn) {
    uint32_t elapsed = now - startMs;
    if (elapsed >= onDurationMs) {
      // Süre doldu - otomatik kapat / time's up - auto shut off
      turnOff(L("Süre doldu, cihaz OTOMATİK kapatıldı.", "Time's up, device turned OFF automatically."));
    } else {
      // Kalan süreye göre LED yanıp sönme hızını ayarla / blink rate scales with remaining time
      uint32_t remainingMs = onDurationMs - elapsed;
      uint32_t blinkGapMs = map(remainingMs, 0, onDurationMs, 100, 800);
      if (now - lastBlinkMs >= blinkGapMs) {
        lastBlinkMs = now;
        ledState = !ledState;
        rolebot.ledWrite(ledState);
      }
      // Her 10 saniyede bir kalan süreyi yaz / print the remaining time every 10 seconds
      if (now - lastReportMs >= 10000) {
        lastReportMs = now;
        rolebot.serialWrite(String(L("Kalan süre: ", "Time left: ")) + remainingSeconds() + L(" sn", " s"));
      }
    }
  }
}
