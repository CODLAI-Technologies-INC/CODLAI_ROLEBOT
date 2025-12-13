/*
 * ROLEBOT IFTTT Webhook Example / ROLEBOT IFTTT Webhook Örneği
 *
 * Control home automations by triggering IFTTT Webhooks whenever ROLEBOT boots
 * or when Button 1 is pressed. Customize the WiFi credentials, event name,
 * and webhook key, then link the event to any IFTTT applet (Google Sheets,
 * Gmail, Discord, smart lights, etc.).
 *
 * Setup reminder / Kurulum özeti:
 * 1. Create an IFTTT applet that uses Webhooks -> "Receive a web request" and set the Event Name to match
 *    `iftttEventName` below.
 * 2. Select your desired action service and finish the applet.
 * 3. Grab the Maker Webhooks key from https://ifttt.com/maker_webhooks (Documentation tab) and paste it into
 *    `iftttWebhookKey`.
 * 4. The JSON payload uses the standard `value1/2/3` fields that you can reuse inside the action step.
 *
 * NOTE: With `USE_IFTTT` enabled the ROLEBOT library automatically enables Wi-Fi, so `#define USE_WIFI`
 * is no longer needed.
 */

#define USE_IFTTT
#include "ROLEBOT.h"

ROLEBOT rolebot;

const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";

String iftttEventName = "YOUR_EVENT_NAME";
String iftttWebhookKey = "YOUR_IFTTT_KEY";

unsigned long lastTrigger = 0;
const unsigned long triggerInterval = 4000;

void setup() {
  rolebot.serialStart(115200);
  rolebot.begin();
  rolebot.playIntro();

  Serial.println("ROLEBOT IFTTT Webhook Example");
  Serial.println("Connecting to WiFi...");
  rolebot.wifiStartAndConnect(ssid, password);

  if (rolebot.wifiConnectionControl()) {
    Serial.println("WiFi connected. Sending boot notification...");
    String payload = "{\"value1\":\"ROLEBOT\",\"value2\":\"Boot\",\"value3\":\"Relays Ready\"}";
    rolebot.triggerIFTTTEvent(iftttEventName, iftttWebhookKey, payload);
  } else {
    Serial.println("WiFi connection failed. Check SSID/PASS.");
  }
}

void loop() {
  bool buttonPressed = rolebot.button1Read();
  unsigned long now = millis();

  if (buttonPressed && (now - lastTrigger) > triggerInterval) {
    lastTrigger = now;

    // Optional feedback
    rolebot.ledWrite(true);
    rolebot.Relay1Write(true);

    String payload = "{\"value1\":\"ROLEBOT Button\",\"value2\":\"Pressed\",\"value3\":\"Relay1 ON\"}";
    bool ok = rolebot.triggerIFTTTEvent(iftttEventName, iftttWebhookKey, payload);
    Serial.println(ok ? "[IFTTT] Button event delivered" : "[IFTTT] Button event failed");

    delay(500);
    rolebot.ledWrite(false);
    rolebot.Relay1Write(false);
  }

  delay(50);
}
