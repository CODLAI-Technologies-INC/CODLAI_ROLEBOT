/*
 * TR: KABLOSUZ AKILLI EV FİKRİ - OTOMATİK VANTİLATÖR. Bu ROLEBOT'un kendi sıcaklık
 * sensörü YOK - bunun yerine, aynı odadaki bir IOTBOT'un yayınladığı (broadcast) DHT
 * sıcaklık verisini ESP-NOW ile dinler ve sıcaklık yükselince Röle 1'e bağlı GERÇEK bir
 * vantilatörü/fanı otomatik açar. Önce IOTBOT_ESPNOW_Temperature_Broadcast_Example.ino
 * dosyasını bir IOTBOT'a yükleyin, sonra bu kodu bir ROLEBOT'a yükleyin - IOTBOT'un DHT
 * sensörünü elinizle ısıtınca ROLEBOT'un rölesi (ve bağlıysa vantilatörünüz) açılır!
 *  - OTOMATİK mod (açılışta): sıcaklık eşiğin ÜSTÜNDEYSE fan açık, değilse kapalı.
 *  - B1 butonu: OTOMATİK modda basınca MANUEL moda geçer. MANUEL modda kısa basış fanı
 *    açar/kapatır, 1 sn basılı tutmak OTOMATİK moda döndürür.
 *  - Mavi LED, fan açıkken yanar.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim     / help          -> komut listesi
 *      oto        / auto          -> otomatik mod (sıcaklığa göre)
 *      manuel     / manual        -> manuel mod
 *      ac         / on            -> fanı aç (manuel moda geçer; "role1 ac" da olur)
 *      kapat      / off           -> fanı kapat
 *      degistir   / toggle        -> fanı tersine çevir
 *      esik 28    / threshold 28  -> sıcaklık eşiği (°C)
 *      durum      / status        -> son sıcaklık, eşik, fan durumu
 *      dil        / lang          -> dili değiştir (Türkçe <-> English)
 *
 * EN: A WIRELESS SMART HOME IDEA - AUTOMATIC FAN. This ROLEBOT has NO temperature sensor
 * of its own - instead, it listens over ESP-NOW to the DHT temperature data broadcast by
 * an IOTBOT in the same room, and automatically turns on a REAL fan (wired to Relay 1)
 * when it gets hot. First upload IOTBOT_ESPNOW_Temperature_Broadcast_Example.ino to an
 * IOTBOT, then upload this code to a ROLEBOT - warm up the IOTBOT's DHT sensor with your
 * hand and watch the ROLEBOT's relay (and your fan, if wired) turn on!
 *  - AUTO mode (at startup): fan on when the temperature is ABOVE the threshold.
 *  - B1 button: in AUTO mode a press switches to MANUAL mode. In MANUAL mode a short press
 *    turns the fan on/off, holding it 1 s goes back to AUTO mode.
 *  - The blue LED is on while the fan is on.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help         / yardim     -> command list
 *      auto         / oto        -> auto mode (by temperature)
 *      manual       / manuel     -> manual mode
 *      on           / ac         -> fan on (switches to manual; "relay1 on" works too)
 *      off          / kapat      -> fan off
 *      toggle       / degistir   -> invert the fan
 *      threshold 28 / esik 28    -> temperature threshold (°C)
 *      status       / durum      -> last temperature, threshold, fan state
 *      lang         / dil        -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Fanı Röle 1'in COM + NO uçlarına bağlayın. Şebeke gerilimi (220V)
 * ile SADECE bir yetişkin çalışsın. / Wire the fan to Relay 1's COM + NO terminals.
 * Only an adult should work with mains voltage (220V).
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

// Bu eşiğin ÜZERİNDEKİ değerler "sıcak" sayılır - ortamınıza göre ayarlayın ("esik 30").
// Values ABOVE this threshold count as "hot" - adjust to your environment ("threshold 30").
int hotThresholdC = 28;

const uint32_t LONG_PRESS_MS = 1000; // Uzun basış süresi / long-press time

bool manualMode = false; // false = OTOMATİK, true = MANUEL / false = AUTO, true = MANUAL
bool fanOn = false;
int lastTempC = 0;
bool haveTemp = false;   // IOTBOT'tan en az bir veri geldi mi? / any data from the IOTBOT yet?
uint32_t lastTempMs = 0;

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
// B1 butonu (GPIO0, basılıyken LOW) / B1 button (GPIO0, LOW while pressed)
// ---------------------------------------------------------------------------
bool b1Down = false;
bool b1LongDone = false;
uint32_t b1DownMs = 0;

// 0 = olay yok, 1 = kısa basış (bırakınca), 2 = uzun basış (1 sn basılı tutunca)
// 0 = no event, 1 = short press (on release), 2 = long press (after holding 1 s)
int readB1(uint32_t now) {
  bool down = !rolebot.button1Read();
  int event = 0;
  if (down && !b1Down) { b1DownMs = now; b1LongDone = false; }
  if (down && !b1LongDone && now - b1DownMs >= LONG_PRESS_MS) { b1LongDone = true; event = 2; }
  if (!down && b1Down && !b1LongDone && now - b1DownMs >= 30) event = 1; // 30 ms parazit süzgeci / debounce
  b1Down = down;
  return event;
}

// ---------------------------------------------------------------------------
void setFan(bool on, const char *reason) {
  fanOn = on;
  rolebot.Relay1Write(fanOn);
  rolebot.ledWrite(fanOn);
  rolebot.serialWrite(String(fanOn ? L("VANTİLATÖR AÇILDI", "FAN ON") : L("VANTİLATÖR KAPANDI", "FAN OFF")) + "  (" + reason + ")");
}

// OTOMATİK modda fanın olması gereken durumu uygula / apply what the fan should be in AUTO mode
void applyAuto() {
  if (manualMode || !haveTemp) return;
  bool shouldBeOn = lastTempC > hotThresholdC;
  if (shouldBeOn != fanOn) {
    char reason[48];
    snprintf(reason, sizeof(reason), L("%d °C, eşik %d °C", "%d °C, threshold %d °C"), lastTempC, hotThresholdC);
    setFan(shouldBeOn, reason);
  }
}

void printHelp() {
  rolebot.serialWrite(L("---- OTOMATİK VANTİLATÖR - Komutlar ----", "---- AUTOMATIC FAN - Commands ----"));
  rolebot.serialWrite(L("  yardim     : bu liste", "  help       : this list"));
  rolebot.serialWrite(L("  oto        : otomatik mod (sıcaklığa göre)", "  auto       : auto mode (by temperature)"));
  rolebot.serialWrite(L("  manuel     : manuel mod", "  manual     : manual mode"));
  rolebot.serialWrite(L("  ac / kapat : fanı aç / kapat", "  on / off   : fan on / off"));
  rolebot.serialWrite(L("  degistir   : fanı tersine çevir", "  toggle     : invert the fan"));
  rolebot.serialWrite(L("  esik 28    : sıcaklık eşiği (°C)", "  threshold 28 : temperature threshold (°C)"));
  rolebot.serialWrite(L("  durum      : sıcaklık ve fan durumu", "  status     : temperature and fan state"));
  rolebot.serialWrite(L("  dil        : English'e geç", "  lang       : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu  : OTOMATİK'te bas = MANUEL; MANUEL'de kısa bas = fan aç/kapat, 1 sn bas = OTOMATİK",
                        "  B1 button  : press in AUTO = MANUAL; in MANUAL short press = fan on/off, hold 1 s = AUTO"));
}

void printStatus() {
  String s = String(L("Mod: ", "Mode: ")) + (manualMode ? L("MANUEL", "MANUAL") : L("OTOMATİK", "AUTO")) +
             L(" | Fan: ", " | Fan: ") + (fanOn ? L("AÇIK", "ON") : L("KAPALI", "OFF")) +
             L(" | Eşik: ", " | Threshold: ") + hotThresholdC + " °C";
  if (haveTemp) s += String(L(" | Son sıcaklık: ", " | Last temperature: ")) + lastTempC + " °C (" + ((millis() - lastTempMs) / 1000) + L(" sn önce)", " s ago)");
  else s += L(" | IOTBOT'tan henüz veri yok", " | no data from the IOTBOT yet");
  rolebot.serialWrite(s);
}

void setMode(bool manual) {
  manualMode = manual;
  rolebot.serialWrite(manual ? L(">> MANUEL mod: fanı B1 ya da \"ac\" / \"kapat\" ile siz kontrol edin.",
                                 ">> MANUAL mode: you control the fan with B1 or \"on\" / \"off\".")
                             : L(">> OTOMATİK mod: fan IOTBOT'un sıcaklığına göre çalışıyor.",
                                 ">> AUTO mode: the fan follows the IOTBOT's temperature."));
  applyAuto();
}

void handleCommand(const String &cmd) {
  // "role1 ac" / "relay1 on" -> "ac" / "on" (fan Röle 1'de / the fan is on Relay 1)
  String c = cmd;
  c.replace("role 1 ", ""); c.replace("relay 1 ", "");
  c.replace("role1 ", "");  c.replace("relay1 ", "");
  c.replace("fan ", "");
  c.trim();
  int space = c.indexOf(' ');
  String word = (space < 0) ? c : c.substring(0, space);
  bool hasValue = space > 0;
  int value = hasValue ? c.substring(space + 1).toInt() : 0;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "oto" || word == "otomatik" || word == "auto") {
    setMode(false);
  } else if (word == "manuel" || word == "manual") {
    setMode(true);
  } else if (word == "ac" || word == "acik" || word == "on") {
    if (!manualMode) setMode(true);
    setFan(true, L("seri komut", "serial command"));
  } else if (word == "kapat" || word == "kapa" || word == "kapali" || word == "off") {
    if (!manualMode) setMode(true);
    setFan(false, L("seri komut", "serial command"));
  } else if (word == "degistir" || word == "toggle") {
    if (!manualMode) setMode(true);
    setFan(!fanOn, L("seri komut", "serial command"));
  } else if ((word == "esik" || word == "threshold") && hasValue) {
    hotThresholdC = constrain(value, -20, 80);
    rolebot.serialWrite(String(L("Sıcaklık eşiği: ", "Temperature threshold: ")) + hotThresholdC + " °C");
    applyAuto();
  } else if (word == "durum" || word == "status" || word == "oku" || word == "read") {
    printStatus();
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
  rolebot.serialStart(115200);
  rolebot.initESPNow();
  rolebot.startListening(); // Gelen IOTBOT yayınını rolebot.receivedData'ya yazar / stores the IOTBOT broadcast in rolebot.receivedData

  rolebot.serialWrite(L("Otomatik vantilatör hazır - IOTBOT'tan sıcaklık verisi bekleniyor...",
                        "Automatic fan ready - waiting for temperature data from the IOTBOT..."));
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // 1) ESP-NOW verisi / ESP-NOW data
  if (rolebot.newData) {
    rolebot.newData = false;
    // Sadece IOTBOT sıcaklık yayınından (deviceType 11) gelen veriyi kabul ediyoruz.
    // Only treat data from an IOTBOT temperature broadcast (deviceType 11) as a temperature reading.
    if (rolebot.receivedData.deviceType == 11) {
      int tempC = rolebot.receivedData.axis1;
      if (!haveTemp || tempC != lastTempC) {
        rolebot.serialWrite(String(L("Sıcaklık: ", "Temperature: ")) + tempC + " °C");
      }
      lastTempC = tempC;
      haveTemp = true;
      lastTempMs = now;
      applyAuto();
    }
  }

  // 2) B1 butonu / B1 button
  int b1 = readB1(now);
  if (b1 != 0) {
    if (!manualMode) setMode(true);                       // OTOMATİK'te bas -> MANUEL / press in AUTO -> MANUAL
    else if (b1 == 2) setMode(false);                     // 1 sn bas -> OTOMATİK / hold 1 s -> AUTO
    else setFan(!fanOn, L("B1 butonu", "B1 button"));     // Kısa bas -> fan aç/kapat / short press -> fan on/off
  }

  // 3) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
