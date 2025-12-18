/*
 * English: Wikipedia Search Example
 * Turkish: Wikipedia Arama Örneği
 */
#define USE_WIKIPEDIA
#include "ROLEBOT.h"

ROLEBOT rolebot;

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

void setup() {
  rolebot.serialStart(115200);
  rolebot.begin();
  
  Serial.println("Wikipedia Search Example");

  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASSWORD);
  
  if(rolebot.wifiConnectionControl()) {
      Serial.println("Searching for 'Robot'...");
      String summary = rolebot.getWikipedia("Robot", "en");
      Serial.println("Summary:");
      Serial.println(summary);
  }
}

void loop() {
  
}
