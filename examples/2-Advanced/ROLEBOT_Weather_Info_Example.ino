/*
 * English: Weather Info Example
 * Turkish: Hava Durumu Bilgisi Örneği
 */
#define USE_WEATHER
#define USE_WIFI
#include "ROLEBOT.h"

ROLEBOT rolebot;

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// OpenWeatherMap API Key (Optional, leave empty to use wttr.in)
#define API_KEY "" 
#define CITY "Istanbul"

void setup() {
  rolebot.serialStart(115200);
  rolebot.begin();
  
  Serial.println("Weather Info Example");

  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  if(rolebot.wifiConnectionControl()) {
      String weather = rolebot.getWeather(CITY, API_KEY);
      Serial.println("Weather in " + String(CITY) + ": " + weather);
  }
  delay(60000); // Update every minute
}
