/*
 * TR: KABLOSUZ AKILLI EV FİKRİ - OTOMATİK GECE LAMBASI. Bu ROLEBOT'un kendi ışık
 * sensörü YOK - bunun yerine, aynı odadaki bir IOTBOT'un yayınladığı (broadcast) ışık
 * sensörü verisini ESP-NOW ile dinler ve hava kararınca (ışık değeri düşünce) Röle 1'e
 * bağlı GERÇEK bir lambayı otomatik açar. İki kartı kablo OLMADAN birlikte çalışır hale
 * getiriyoruz. Önce IOTBOT_ESPNOW_LightSensor_Broadcast_Example.ino dosyasını bir
 * IOTBOT'a yükleyin, sonra bu kodu bir ROLEBOT'a yükleyin - IOTBOT'un ışık sensörünü
 * elinizle kapatınca ROLEBOT'un rölesi (ve bağlıysa lambanız) açılır!
 *  - OTOMATİK mod (açılışta): ışık değeri eşiğin ALTINDAYSA (karanlık) lamba açık.
 *  - B1 butonu: OTOMATİK modda basınca MANUEL moda geçer. MANUEL modda kısa basış lambayı
 *    açar/kapatır, 1 sn basılı tutmak OTOMATİK moda döndürür.
 *  - Mavi LED, lamba açıkken yanar.
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim       / help            -> komut listesi
 *      oto          / auto            -> otomatik mod (ışığa göre)
 *      manuel       / manual          -> manuel mod
 *      ac           / on              -> lambayı aç (manuel moda geçer; "role1 ac" da olur)
 *      kapat        / off             -> lambayı kapat
 *      degistir     / toggle          -> lambayı tersine çevir
 *      esik 1500    / threshold 1500  -> karanlık eşiği (0-4095)
 *      durum        / status          -> son ışık değeri, eşik, lamba durumu
 *      dil          / lang            -> dili değiştir (Türkçe <-> English)
 *
 * EN: A WIRELESS SMART HOME IDEA - AUTOMATIC NIGHT LIGHT. This ROLEBOT has NO light
 * sensor of its own - instead, it listens over ESP-NOW to the light sensor data broadcast
 * by an IOTBOT in the same room, and automatically turns on a REAL lamp (wired to Relay 1)
 * when it gets dark. We make two boards work together WITHOUT any wire between them.
 * First upload IOTBOT_ESPNOW_LightSensor_Broadcast_Example.ino to an IOTBOT, then upload
 * this code to a ROLEBOT - cover the IOTBOT's light sensor with your hand and watch the
 * ROLEBOT's relay (and your lamp, if wired) turn on!
 *  - AUTO mode (at startup): lamp on when the light value is BELOW the threshold (dark).
 *  - B1 button: in AUTO mode a press switches to MANUAL mode. In MANUAL mode a short press
 *    turns the lamp on/off, holding it 1 s goes back to AUTO mode.
 *  - The blue LED is on while the lamp is on.
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help            / yardim     -> command list
 *      auto            / oto        -> auto mode (by light)
 *      manual          / manuel     -> manual mode
 *      on              / ac         -> lamp on (switches to manual; "relay1 on" works too)
 *      off             / kapat      -> lamp off
 *      toggle          / degistir   -> invert the lamp
 *      threshold 1500  / esik 1500  -> darkness threshold (0-4095)
 *      status          / durum      -> last light value, threshold, lamp state
 *      lang            / dil        -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Lambayı Röle 1'in COM + NO uçlarına bağlayın. Şebeke gerilimi (220V)
 * ile SADECE bir yetişkin çalışsın. / Wire the lamp to Relay 1's COM + NO terminals.
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

// Bu eşiğin ALTINDAKİ değerler "karanlık" sayılır - IOTBOT'unuzun ortam ışığına göre
// ayarlayın ("esik 1200"). / Values BELOW this threshold count as "dark" - adjust to your
// IOTBOT's ambient light level ("threshold 1200").
int darkThreshold = 1500;

const uint32_t LONG_PRESS_MS = 1000; // Uzun basış süresi / long-press time

bool manualMode = false; // false = OTOMATİK, true = MANUEL / false = AUTO, true = MANUAL
bool lampOn = false;
int lastLight = 0;
bool haveLight = false;  // IOTBOT'tan en az bir veri geldi mi? / any data from the IOTBOT yet?
uint32_t lastLightMs = 0;
uint32_t lastPrintMs = 0;

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
void setLamp(bool on, const char *reason) {
  lampOn = on;
  rolebot.Relay1Write(lampOn);
  rolebot.ledWrite(lampOn);
  rolebot.serialWrite(String(lampOn ? L("LAMBA AÇILDI", "LAMP ON") : L("LAMBA KAPANDI", "LAMP OFF")) + "  (" + reason + ")");
}

// OTOMATİK modda lambanın olması gereken durumu uygula / apply what the lamp should be in AUTO mode
void applyAuto() {
  if (manualMode || !haveLight) return;
  bool shouldBeOn = lastLight < darkThreshold;
  if (shouldBeOn != lampOn) {
    char reason[48];
    snprintf(reason, sizeof(reason), L("ışık %d, eşik %d -> %s", "light %d, threshold %d -> %s"),
             lastLight, darkThreshold, shouldBeOn ? L("karanlık", "dark") : L("aydınlık", "bright"));
    setLamp(shouldBeOn, reason);
  }
}

void printHelp() {
  rolebot.serialWrite(L("---- OTOMATİK GECE LAMBASI - Komutlar ----", "---- AUTOMATIC NIGHT LIGHT - Commands ----"));
  rolebot.serialWrite(L("  yardim     : bu liste", "  help       : this list"));
  rolebot.serialWrite(L("  oto        : otomatik mod (ışığa göre)", "  auto       : auto mode (by light)"));
  rolebot.serialWrite(L("  manuel     : manuel mod", "  manual     : manual mode"));
  rolebot.serialWrite(L("  ac / kapat : lambayı aç / kapat", "  on / off   : lamp on / off"));
  rolebot.serialWrite(L("  degistir   : lambayı tersine çevir", "  toggle     : invert the lamp"));
  rolebot.serialWrite(L("  esik 1500  : karanlık eşiği (0-4095)", "  threshold 1500 : darkness threshold (0-4095)"));
  rolebot.serialWrite(L("  durum      : ışık ve lamba durumu", "  status     : light and lamp state"));
  rolebot.serialWrite(L("  dil        : English'e geç", "  lang       : switch to Turkish"));
  rolebot.serialWrite(L("  B1 butonu  : OTOMATİK'te bas = MANUEL; MANUEL'de kısa bas = lamba aç/kapat, 1 sn bas = OTOMATİK",
                        "  B1 button  : press in AUTO = MANUAL; in MANUAL short press = lamp on/off, hold 1 s = AUTO"));
}

void printStatus() {
  String s = String(L("Mod: ", "Mode: ")) + (manualMode ? L("MANUEL", "MANUAL") : L("OTOMATİK", "AUTO")) +
             L(" | Lamba: ", " | Lamp: ") + (lampOn ? L("AÇIK", "ON") : L("KAPALI", "OFF")) +
             L(" | Eşik: ", " | Threshold: ") + darkThreshold;
  if (haveLight) s += String(L(" | Son ışık değeri: ", " | Last light value: ")) + lastLight + " (" + ((millis() - lastLightMs) / 1000) + L(" sn önce)", " s ago)");
  else s += L(" | IOTBOT'tan henüz veri yok", " | no data from the IOTBOT yet");
  rolebot.serialWrite(s);
}

void setMode(bool manual) {
  manualMode = manual;
  rolebot.serialWrite(manual ? L(">> MANUEL mod: lambayı B1 ya da \"ac\" / \"kapat\" ile siz kontrol edin.",
                                 ">> MANUAL mode: you control the lamp with B1 or \"on\" / \"off\".")
                             : L(">> OTOMATİK mod: lamba IOTBOT'un ışık sensörüne göre çalışıyor.",
                                 ">> AUTO mode: the lamp follows the IOTBOT's light sensor."));
  applyAuto();
}

void handleCommand(const String &cmd) {
  // "role1 ac" / "relay1 on" -> "ac" / "on" (lamba Röle 1'de / the lamp is on Relay 1)
  String c = cmd;
  c.replace("role 1 ", ""); c.replace("relay 1 ", "");
  c.replace("role1 ", "");  c.replace("relay1 ", "");
  c.replace("lamba ", "");  c.replace("lamp ", "");
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
    setLamp(true, L("seri komut", "serial command"));
  } else if (word == "kapat" || word == "kapa" || word == "kapali" || word == "off") {
    if (!manualMode) setMode(true);
    setLamp(false, L("seri komut", "serial command"));
  } else if (word == "degistir" || word == "toggle") {
    if (!manualMode) setMode(true);
    setLamp(!lampOn, L("seri komut", "serial command"));
  } else if ((word == "esik" || word == "threshold") && hasValue) {
    darkThreshold = constrain(value, 0, 4095);
    rolebot.serialWrite(String(L("Karanlık eşiği: ", "Darkness threshold: ")) + darkThreshold);
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

  rolebot.serialWrite(L("Otomatik gece lambası hazır - IOTBOT'tan ışık verisi bekleniyor...",
                        "Automatic night light ready - waiting for light data from the IOTBOT..."));
  printHelp();
}

void loop() {
  uint32_t now = millis();

  // 1) ESP-NOW verisi / ESP-NOW data
  if (rolebot.newData) {
    rolebot.newData = false;
    // Sadece IOTBOT'tan (deviceType 10) gelen veriyi ışık sensörü olarak kabul ediyoruz.
    // Only treat data from an IOTBOT (deviceType 10) as a light sensor reading.
    if (rolebot.receivedData.deviceType == 10) {
      lastLight = rolebot.receivedData.axis1;
      haveLight = true;
      lastLightMs = now;
      // Değeri en fazla 2 sn'de bir yaz (ekranı doldurmasın) / print at most every 2 s (no flooding)
      if (now - lastPrintMs >= 2000) {
        lastPrintMs = now;
        rolebot.serialWrite(String(L("Işık değeri: ", "Light value: ")) + lastLight);
      }
      applyAuto();
    }
  }

  // 2) B1 butonu / B1 button
  int b1 = readB1(now);
  if (b1 != 0) {
    if (!manualMode) setMode(true);                         // OTOMATİK'te bas -> MANUEL / press in AUTO -> MANUAL
    else if (b1 == 2) setMode(false);                       // 1 sn bas -> OTOMATİK / hold 1 s -> AUTO
    else setLamp(!lampOn, L("B1 butonu", "B1 button"));     // Kısa bas -> lamba aç/kapat / short press -> lamp on/off
  }

  // 3) Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
