/*
 * TR: GERÇEK PROJE - Kablosuz Deprem Uyarı Ağı (ROLEBOT ALICI). Bu ROLEBOT, IOTBOT'un
 * ESP-NOW ile yayınladığı "deprem" mesajını dinler. "deprem = 1" gelince: Röle 1 ACİL
 * DURUM LAMBASINI yakar, Röle 2 doğalgaz vanasını / ana elektriği KESER ve LED yanıp
 * söner. "deprem = 0" gelince her şey eski haline döner. B1 butonu = yerel sıfırlama
 * (IOTBOT'a ulaşılamasa bile burada alarmı bitirir). Önce IOTBOT'a
 * IOTBOT_ESPNOW_Earthquake_Alert_Sender_Example.ino, istersen bir MINIBOT'a da
 * MINIBOT_ESPNOW_Earthquake_Alert_Receiver_Example.ino dosyasını yükleyin.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim    / help     -> komut listesi
 *      sifirla   / reset    -> alarmı yerel olarak bitir (B1 ile aynı)
 *      test      / test     -> alarmı DENEME için başlat (lamba yanar, vana kesilir)
 *      durum     / status   -> alarm ve röle durumu
 *      dil       / lang     -> dili değiştir (Türkçe <-> English)
 *
 * EN: A REAL PROJECT - Wireless Earthquake Alert Network (ROLEBOT RECEIVER). This ROLEBOT
 * listens for the "deprem" message the IOTBOT broadcasts over ESP-NOW. On "deprem = 1":
 * Relay 1 turns the EMERGENCY LIGHT on, Relay 2 CUTS the gas valve / main power and the
 * LED blinks. On "deprem = 0" everything goes back to normal. B1 button = local reset
 * (ends the alarm here even if the IOTBOT cannot be reached). First upload
 * IOTBOT_ESPNOW_Earthquake_Alert_Sender_Example.ino to an IOTBOT, and optionally
 * MINIBOT_ESPNOW_Earthquake_Alert_Receiver_Example.ino to a MINIBOT.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help      / yardim   -> command list
 *      reset     / sifirla  -> end the alarm locally (same as B1)
 *      test      / test     -> start the alarm as a TEST (light on, valve cut)
 *      status    / durum    -> alarm and relay state
 *      lang      / dil      -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Her rölenin 3 ucu vardır: COM (ortak), NO (normalde açık), NC
 * (normalde kapalı).
 * - Röle 1 (acil lamba): lambayı COM + NO üzerinden bağlayın -> röle çekmeyince lamba
 *   sönük, alarmda röle çekince yanar.
 * - Röle 2 (gaz vanası / ana hat): vananın beslemesini COM + NC üzerinden bağlayın ->
 *   normalde NC kapalı, vana enerjili ve AÇIK; alarmda röle çeker, NC açılır, vana
 *   enerjisiz kalıp KAPANIR ("enerji kesilince kapanan" tip vana kullanın).
 * EN: Each relay has 3 terminals: COM (common), NO (normally open), NC (normally closed).
 * - Relay 1 (emergency light): wire the lamp through COM + NO -> lamp is off while the
 *   relay is idle, it lights when the relay pulls in during alarm.
 * - Relay 2 (gas valve / main line): feed the valve through COM + NC -> normally NC is
 *   closed, the valve is powered and OPEN; during alarm the relay pulls in, NC opens and
 *   the valve loses power and CLOSES (use a "closes when unpowered" type valve).
 * DİKKAT / WARNING: Şebeke gerilimi (220V) ile SADECE bir yetişkin çalışsın; denemek için
 * 5V'luk bir LED veya küçük motor kullanın. / Only an adult should work with mains voltage
 * (220V); use a 5V LED or small motor to try it.
 *
 * ROLEBOT'ta LCD ekran YOK; tüm bilgiler Seri Port (USB) üzerinden verilir.
 * / ROLEBOT has NO LCD screen; all feedback is given through Serial (USB).
 */

#define USE_ESPNOW
#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

namespace {
  constexpr int kEspNowChannel = 1;     // IOTBOT ile AYNI kanal / SAME channel as the IOTBOT
  constexpr uint32_t kBlinkMs = 250;    // LED yanıp sönme hızı / LED blink speed

  bool alarmOn = false;
  bool ledState = false;
  uint32_t lastBlinkMs = 0;
  bool lastButton = true;               // HIGH = bırakılmış / released
  uint32_t lastEdgeMs = 0;              // Parazit süzgeci / debounce

  void say(const char *turkishText, const char *englishText) {
    rolebot.serialWrite(L(turkishText, englishText));
  }

  void setAlarm(bool on) {
    alarmOn = on;
    rolebot.Relay1Write(on); // Acil lamba (NO) / emergency light (NO)
    rolebot.Relay2Write(on); // Gaz vanası / ana hat KES (NC) / CUT gas valve / main line (NC)
    if (!on) rolebot.ledWrite(false);
    if (on) say("!!! DEPREM !!! Acil lamba AÇIK, gaz/elektrik KESİLDİ.",
                "!!! EARTHQUAKE !!! Emergency light ON, gas/power CUT.");
    else say("Normale dönüldü: acil lamba kapalı, gaz/elektrik geri verildi.",
             "Back to normal: emergency light off, gas/power restored.");
  }
}

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "SIFIRLA" -> "sifirla"
// Lower-cases and simplifies Turkish letters: "SIFIRLA" -> "sifirla"
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
  say("---- DEPREM ALICISI - Komutlar ----", "---- EARTHQUAKE RECEIVER - Commands ----");
  say("  yardim     : bu liste", "  help       : this list");
  say("  sifirla    : alarmı yerel olarak bitir", "  reset      : end the alarm locally");
  say("  test       : alarmı deneme için başlat", "  test       : start the alarm as a test");
  say("  durum      : alarm ve röle durumu", "  status     : alarm and relay state");
  say("  dil        : English'e geç", "  lang       : switch to Turkish");
  say("  B1 butonu  : yerel sıfırlama", "  B1 button  : local reset");
}

void handleCommand(const String &cmd) {
  if (cmd == "yardim" || cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "sifirla" || cmd == "reset") {
    if (alarmOn) {
      say("Yerel sıfırlama (seri komut).", "Local reset (serial command).");
      setAlarm(false);
    } else {
      say("Alarm zaten kapalı.", "The alarm is already off.");
    }
  } else if (cmd == "test" || cmd == "alarm test" || cmd == "deneme") {
    say("DENEME alarmı başlatılıyor ('sifirla' ile bitirin).", "Starting a TEST alarm (end it with 'reset').");
    if (!alarmOn) setAlarm(true);
  } else if (cmd == "durum" || cmd == "status") {
    rolebot.serialWrite(alarmOn ? L("Durum: ALARM! Röle 1 (acil lamba) AÇIK, Röle 2 (vana/hat) ÇEKİLİ = KESİK",
                                    "State: ALARM! Relay 1 (emergency light) ON, Relay 2 (valve/line) PULLED IN = CUT")
                                : L("Durum: normal. Röle 1 ve Röle 2 KAPALI (vana/hat açık).",
                                    "State: normal. Relay 1 and Relay 2 OFF (valve/line open)."));
  } else if (cmd == "dil" || cmd == "lang" || cmd == "language") {
    turkish = !turkish;
    say("Dil: Türkçe", "Language: English");
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
  rolebot.ledWrite(false);
  rolebot.serialStart(115200);
  rolebot.espNowBegin(kEspNowChannel);
  say("Deprem alıcısı (ROLEBOT) hazır - IOTBOT'tan uyarı bekleniyor...",
      "Earthquake receiver (ROLEBOT) ready - waiting for an alert from the IOTBOT...");
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // 1) Gelen mesaj. NOT: espNowReadName() mesajı "okundu" işaretler; bu yüzden sayıyı
  // ÖNCE receivedData.value'dan alıyoruz, adı SONRA okuyoruz.
  // 1) Incoming message. NOTE: espNowReadName() marks the message as read, so we take
  // the number FIRST from receivedData.value and read the name AFTER.
  if (rolebot.espNowAvailable()) {
    rolebot.espNowReadText();                    // Metin mesajıysa at / drop it if it is a text message
    float value = rolebot.receivedData.value;
    String name = rolebot.espNowReadName();
    if (name == "deprem") {
      bool danger = value > 0.5f;
      if (danger != alarmOn) setAlarm(danger);   // Tekrar eden mesajlar bir şey değiştirmez / repeats change nothing
    }
  }

  // 2) B1 = yerel sıfırlama (30 ms parazit süzgeci). / B1 = local reset (30 ms debounce).
  bool button = rolebot.button1Read(); // false = basılı / pressed
  if (button != lastButton && now - lastEdgeMs >= 30) {
    lastEdgeMs = now;
    if (lastButton && !button && alarmOn) {
      say("Yerel sıfırlama (B1 butonu).", "Local reset (B1 button).");
      setAlarm(false);
    }
    lastButton = button;
  }

  // 3) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);

  // 4) Alarmda LED yanıp söner (beklemeden). / LED blinks during alarm (non-blocking).
  if (alarmOn && now - lastBlinkMs >= kBlinkMs) {
    lastBlinkMs = now;
    ledState = !ledState;
    rolebot.ledWrite(ledState);
  }
}
