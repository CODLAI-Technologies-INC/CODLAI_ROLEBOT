/*
 * English: ESP-NOW Receiver Example
 * Turkish: ESP-NOW Alıcı Örneği
 */
#define USE_ESPNOW
#define USE_WIFI
#include "ROLEBOT.h"

ROLEBOT rolebot;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

struct_message myData;

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData.a);
  Serial.print("Int: ");
  Serial.println(myData.b);
  Serial.print("Float: ");
  Serial.println(myData.c);
  Serial.print("Bool: ");
  Serial.println(myData.d);
  Serial.println();
}

void setup() {
  rolebot.serialStart(115200);
  rolebot.begin();
  
  Serial.println("ESP-NOW Receiver Example");

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  rolebot.initESPNow();
  
  // Register for a callback function that will be called when data is received
  rolebot.registerOnRecv(OnDataRecv);
}

void loop() {
  // Nothing to do here
  delay(1000);
}
