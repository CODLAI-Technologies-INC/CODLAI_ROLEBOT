/*
  CODLAI ROLEBOT - EEPROM Advanced Example

  TR:
  - Bu örnek, ROLEBOT kütüphanesindeki EEPROM yardımcı fonksiyonlarını gösterir.
  - EEPROM (Flash tabanlı) kalıcı hafızadır. Gereksiz sık yazma yapmayın.
  - Adres aralıklarını çakıştırmayın.

  EN:
  - This example demonstrates EEPROM helper functions in the ROLEBOT library.
  - EEPROM (Flash-backed) is persistent storage. Avoid excessive writes.
  - Do not overlap address ranges.
*/

#include <ROLEBOT.h>

ROLEBOT rolebot;

void setup()
{
  rolebot.serialStart(115200);
  delay(200);

  // TR/EN: Initialize EEPROM
  bool ok = rolebot.eepromBegin(512);
  rolebot.serialWrite(ok ? "[EEPROM] Ready" : "[EEPROM] Begin failed");

  // TR/EN: Legacy int16
  rolebot.eepromWriteInt(0, 2025);
  int v16 = rolebot.eepromReadInt(0);
  rolebot.serialWrite("v16 = ");
  rolebot.serialWrite(v16);

  // TR/EN: int32
  rolebot.eepromWriteInt32(10, -123456);
  int32_t v32 = rolebot.eepromReadInt32(10, 0);
  rolebot.serialWrite("v32 = ");
  rolebot.serialWrite((long)v32);

  // TR/EN: string
  rolebot.eepromWriteString(30, String("ROLEBOT EEPROM test"), 64);
  String s = rolebot.eepromReadString(30, 64);
  rolebot.serialWrite("str = ");
  rolebot.serialWrite(s);

  // TR/EN: Recommended (CRC + versioned record)
  struct ExampleConfig
  {
    uint32_t relayBootCount;
  };

  const int CONFIG_ADDR = 200;

  ExampleConfig cfgOut;
  cfgOut.relayBootCount = 1;
  bool wrec = rolebot.eepromWriteRecord(CONFIG_ADDR, (const uint8_t *)&cfgOut, (uint16_t)sizeof(cfgOut), 1);
  rolebot.serialWrite(wrec ? "record write = OK" : "record write = FAIL");

  ExampleConfig cfgIn;
  uint16_t outLen = 0;
  uint16_t outVer = 0;
  bool rrec = rolebot.eepromReadRecord(CONFIG_ADDR, (uint8_t *)&cfgIn, (uint16_t)sizeof(cfgIn), &outLen, &outVer);

  if (rrec)
  {
    Serial.print("record ver=");
    Serial.print(outVer);
    Serial.print(" len=");
    Serial.print(outLen);
    Serial.print(" relayBootCount=");
    Serial.println((unsigned long)cfgIn.relayBootCount);
  }
  else
  {
    Serial.println("record read = FAIL (magic/len/crc mismatch)");
  }
}

void loop()
{
}
