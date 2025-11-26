/*
 * English: Email Sender Example
 * Turkish: E-posta Gönderici Örneği
 */
#define USE_EMAIL
#define USE_WIFI
#include "ROLEBOT.h"

ROLEBOT rolebot;

// WiFi Credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Email Credentials
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "YOUR_EMAIL@gmail.com"
#define AUTHOR_PASSWORD "YOUR_APP_PASSWORD"
#define RECIPIENT_EMAIL "RECIPIENT_EMAIL@example.com"

void setup() {
  rolebot.serialStart(115200);
  rolebot.begin();
  
  rolebot.serialWrite("Email Sender Example");

  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASSWORD);
  
  if(rolebot.wifiConnectionControl()) {
      rolebot.serialWrite("Sending Email...");
      rolebot.sendEmail(SMTP_HOST, SMTP_PORT, AUTHOR_EMAIL, AUTHOR_PASSWORD, RECIPIENT_EMAIL, "Test Subject", "Hello from ROLEBOT!");
  }
}

void loop() {
  
}
