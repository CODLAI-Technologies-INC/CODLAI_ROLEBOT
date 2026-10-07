/*
 * TR: EEPROM (KALICI HAFIZA) - GELİŞMİŞ ÖRNEK. ROLEBOT kütüphanesindeki EEPROM yardımcı
 * fonksiyonlarını gösterir: 16-bit tam sayı, 32-bit tam sayı, metin (String) ve CRC
 * korumalı "kayıt" (record). Kayıt, kartın kaç kez açıldığını sayar - kartı birkaç kez
 * yeniden başlatın ve sayının arttığını görün!
 *  - EEPROM (Flash tabanlı) kalıcı hafızadır; gereksiz sık yazma yapmayın.
 *  - Adres aralıklarını çakıştırmayın. Bu örneğin adres haritası:
 *      0-1: int16 | 10-13: int32 | 30-95: metin/string | 200-213: kayıt/record
 *  - Seri port komutları (115200 baud). Türkçe veya İngilizce yazabilirsiniz:
 *      yardim       / help        -> komut listesi
 *      oku          / read        -> tüm değerleri hafızadan oku ve yaz
 *      demo         / demo        -> örnek değerleri yeniden yaz ve oku
 *      yaz 1234     / write 1234  -> int16 adresine (0) bir sayı yaz (0-32767)
 *      sil          / clear       -> bu örneğin kullandığı alanı sil (0-255)
 *      dil          / lang        -> dili değiştir (Türkçe <-> English)
 *
 * EN: EEPROM (PERSISTENT MEMORY) - ADVANCED EXAMPLE. Demonstrates the EEPROM helper
 * functions of the ROLEBOT library: 16-bit integer, 32-bit integer, text (String) and a
 * CRC-protected "record". The record counts how many times the board has booted - restart
 * the board a few times and watch the number grow!
 *  - EEPROM (Flash-backed) is persistent storage; avoid excessive writes.
 *  - Do not overlap address ranges. This example's address map:
 *      0-1: int16 | 10-13: int32 | 30-95: text/string | 200-213: record
 *  - Serial port commands (115200 baud). You can type Turkish or English:
 *      help         / yardim      -> command list
 *      read         / oku         -> read and print all values from memory
 *      demo         / demo        -> write the sample values again and read them
 *      write 1234   / yaz 1234    -> write a number to the int16 address (0) (0-32767)
 *      clear        / sil         -> erase the area used by this example (0-255)
 *      lang         / dil         -> switch language (Turkish <-> English)
 *
 * Bağlantı / Wiring: Kablo gerekmez. / No wiring needed.
 */

#include <ROLEBOT.h>

ROLEBOT rolebot;

// Dil seçimi: true = Türkçe, false = English. Seri porttan "dil" / "lang" ile de değişir.
// Language: true = Turkish, false = English. Can also be changed with "dil" / "lang".
bool turkish = true;
const char *L(const char *tr, const char *en) { return turkish ? tr : en; }

// Adres haritası / address map
const int INT16_ADDR = 0;    // 2 bayt / 2 bytes
const int INT32_ADDR = 10;   // 4 bayt / 4 bytes
const int STRING_ADDR = 30;  // 2 + en fazla 64 bayt / 2 + up to 64 bytes
const int CONFIG_ADDR = 200; // 10 bayt başlık + veri / 10-byte header + data

// CRC korumalı kayıt olarak saklanan ayar yapısı / settings struct stored as a CRC-protected record
struct ExampleConfig {
  uint32_t relayBootCount; // Kartın kaç kez açıldığı / how many times the board booted
};

// ---------------------------------------------------------------------------
// Seri komut okuyucu / Serial command reader
// Seri Monitör'ün satır sonu ayarı ne olursa olsun çalışır (NL, CR, ikisi, hiçbiri).
// Works with any Serial Monitor line-ending setting (NL, CR, both, none).
// ---------------------------------------------------------------------------
String cmdBuffer;
uint32_t lastCharMs = 0;

// Küçük harfe çevirir ve Türkçe harfleri sadeleştirir: "YARDIM" -> "yardim"
// Lower-cases and simplifies Turkish letters: "YARDIM" -> "yardim"
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
// Tüm değerleri hafızadan oku ve yaz / read all values from memory and print them
void readAll() {
  rolebot.serialWrite(String("int16  (0)  = ") + rolebot.eepromReadInt(INT16_ADDR));
  rolebot.serialWrite(String("int32  (10) = ") + (long)rolebot.eepromReadInt32(INT32_ADDR, 0));
  rolebot.serialWrite(String("string (30) = \"") + rolebot.eepromReadString(STRING_ADDR, 64) + "\"");

  ExampleConfig cfgIn;
  uint16_t outLen = 0;
  uint16_t outVer = 0;
  if (rolebot.eepromReadRecord(CONFIG_ADDR, (uint8_t *)&cfgIn, (uint16_t)sizeof(cfgIn), &outLen, &outVer)) {
    rolebot.serialWrite(String(L("kayıt  (200): sürüm=", "record (200): ver=")) + outVer + L(" uzunluk=", " len=") + outLen +
                        L(" açılış sayısı=", " boot count=") + (unsigned long)cfgIn.relayBootCount);
  } else {
    rolebot.serialWrite(L("kayıt  (200): OKUNAMADI (sihirli sayı/uzunluk/CRC uyuşmadı - henüz yazılmamış olabilir)",
                          "record (200): READ FAILED (magic/len/crc mismatch - maybe not written yet)"));
  }
}

// Örnek değerleri yaz / write the sample values
void writeDemoValues() {
  rolebot.eepromWriteInt(INT16_ADDR, 2025);                               // Eski tip int16 / legacy int16
  rolebot.eepromWriteInt32(INT32_ADDR, -123456);                          // int32
  rolebot.eepromWriteString(STRING_ADDR, String("ROLEBOT EEPROM test"), 64); // Metin / string
  rolebot.serialWrite(L("Örnek değerler yazıldı (2025, -123456, \"ROLEBOT EEPROM test\").",
                        "Sample values written (2025, -123456, \"ROLEBOT EEPROM test\")."));
}

// Önerilen yöntem: CRC + sürümlü kayıt. Her açılışta sayacı 1 artırır.
// Recommended: CRC + versioned record. Increments the counter on every boot.
void countBoot() {
  ExampleConfig cfg;
  if (!rolebot.eepromReadRecord(CONFIG_ADDR, (uint8_t *)&cfg, (uint16_t)sizeof(cfg))) {
    cfg.relayBootCount = 0; // Geçerli kayıt yok -> sıfırdan başla / no valid record -> start from zero
  }
  cfg.relayBootCount++;
  bool ok = rolebot.eepromWriteRecord(CONFIG_ADDR, (const uint8_t *)&cfg, (uint16_t)sizeof(cfg), 1);
  rolebot.serialWrite(ok ? String(L("Kayıt yazıldı - bu kart ", "Record written - this board has booted ")) + (unsigned long)cfg.relayBootCount + L(". kez açıldı.", " times.")
                         : String(L("Kayıt yazılamadı!", "Record write FAILED!")));
}

void printHelp() {
  rolebot.serialWrite(L("---- EEPROM - Komutlar ----", "---- EEPROM - Commands ----"));
  rolebot.serialWrite(L("  yardim     : bu liste", "  help       : this list"));
  rolebot.serialWrite(L("  oku        : tüm değerleri oku", "  read       : read all values"));
  rolebot.serialWrite(L("  demo       : örnek değerleri yeniden yaz", "  demo       : write the sample values again"));
  rolebot.serialWrite(L("  yaz 1234   : int16 adresine yaz", "  write 1234 : write to the int16 address"));
  rolebot.serialWrite(L("  sil        : 0-255 alanını sil", "  clear      : erase area 0-255"));
  rolebot.serialWrite(L("  dil        : English'e geç", "  lang       : switch to Turkish"));
}

void handleCommand(const String &cmd) {
  int space = cmd.indexOf(' ');
  String word = (space < 0) ? cmd : cmd.substring(0, space);
  bool hasValue = space > 0;
  int value = hasValue ? cmd.substring(space + 1).toInt() : 0;

  if (word == "yardim" || word == "help" || word == "?") {
    printHelp();
  } else if (word == "oku" || word == "read") {
    readAll();
  } else if (word == "demo") {
    writeDemoValues();
    readAll();
  } else if ((word == "yaz" || word == "write") && hasValue) {
    // eepromReadInt negatif sayıları geri okuyamaz (0-65535 döner), bu yüzden 0-32767 kullanıyoruz.
    // eepromReadInt cannot read negative numbers back (returns 0-65535), so we use 0-32767.
    rolebot.eepromWriteInt(INT16_ADDR, constrain(value, 0, 32767));
    rolebot.serialWrite(String(L("int16 adresine yazıldı, okunan: ", "Written to the int16 address, read back: ")) + rolebot.eepromReadInt(INT16_ADDR));
  } else if (word == "sil" || word == "clear") {
    bool ok = rolebot.eepromClear(0, 256, 0xFF);
    rolebot.serialWrite(ok ? L("0-255 alanı silindi (0xFF).", "Area 0-255 erased (0xFF).") : L("Silme başarısız!", "Erase FAILED!"));
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
  rolebot.Relay1Write(false); // Güvenlik: röleler KAPALI / safety: relays OFF
  rolebot.Relay2Write(false);
  rolebot.serialStart(115200);
  delay(200);

  // EEPROM'u başlat / Initialize EEPROM
  bool ok = rolebot.eepromBegin(512);
  rolebot.serialWrite(ok ? L("[EEPROM] Hazır", "[EEPROM] Ready") : L("[EEPROM] Başlatılamadı", "[EEPROM] Begin failed"));

  writeDemoValues();
  countBoot();
  readAll();
  printHelp();
}

void loop() {
  // Seri komutlar / Serial commands
  String cmd;
  if (readCommand(cmd)) handleCommand(cmd);
}
