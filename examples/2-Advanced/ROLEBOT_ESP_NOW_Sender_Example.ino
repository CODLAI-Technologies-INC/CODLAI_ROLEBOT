/*
 * English: ESP-NOW Sender Example
 * Turkish: ESP-NOW Gönderici Örneği
 */
#define USE_ESPNOW
#define USE_WIFI
#include "ROLEBOT.h"

ROLEBOT rolebot;

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

struct_message myData;

void setup() {
  rolebot.serialStart(115200);
  rolebot.begin();
  
  Serial.println("ESP-NOW Sender Example");

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  rolebot.initESPNow();
}

void loop() {
  // Set values to send
  strcpy(myData.a, "Hello from ROLEBOT");
  myData.b = random(1, 20);
  myData.c = 1.2;
  myData.d = false;
  
  // Send message via ESP-NOW
  rolebot.sendESPNow(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  
  delay(2000);
}
