// TR: Telefon/tarayicidan (ROLEBOT'un kendi WiFi agina baglanarak) iki
// roleyi ve LED'i acip kapatabildiginiz bir uzaktan kontrol paneli. Yeni
// `serverOnRequest()` fonksiyonu kullanilir - `serverCreateLocalPage()`'in
// aksine, bir adrese (ornegin "/relay1-on") istek geldiginde GERCEKTEN
// kod calistirmaniza (bir rolyeyi tetiklemenize) izin verir.
// EN: A remote-control panel you open from your phone/browser (by joining
// ROLEBOT's own WiFi network) to turn two relays and the LED on/off. Uses
// the new `serverOnRequest()` function - unlike `serverCreateLocalPage()`,
// it lets code actually RUN (trigger a relay) when a URL (e.g.
// "/relay1-on") is requested.
//
// Kurulum / Setup:
// 1) Bu kodu ROLEBOT'a yukleyin / Upload this to your ROLEBOT.
// 2) Telefonunuzun WiFi ayarlarindan "CODLAI_ROLEBOT" agina baglanin,
//    sifre: 12345678 / On your phone, join the "CODLAI_ROLEBOT" WiFi
//    network, password: 12345678.
// 3) Tarayicida 192.168.4.1 adresini acin / Open 192.168.4.1 in a browser.

#define USE_SERVER
#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

#define AP_SSID "CODLAI_ROLEBOT"
#define AP_PASS "12345678"

namespace {
  bool relay1On = false;
  bool relay2On = false;
  bool ledOn = false;
}

const char WEBPageScript[] PROGMEM = R"rawliteral(
<script>
  function callAction(url) {
    fetch(url).then(() => refreshStatus());
  }
  function refreshStatus() {
    fetch('/status').then(r => r.text()).then(text => {
      document.getElementById('status').innerText = text;
    });
  }
  setInterval(refreshStatus, 1000);
  window.onload = refreshStatus;
</script>
)rawliteral";

const char WEBPageCSS[] PROGMEM = R"rawliteral(
<style>
  body { text-align: center; font-family: Arial, sans-serif; background: #101418; color: #eee; }
  h1 { color: #f6ad55; }
  button { font-size: 18px; padding: 12px 20px; margin: 10px; border-radius: 8px; border: none; }
  .on { background: #38a169; color: white; }
  .off { background: #e53e3e; color: white; }
  #status { white-space: pre-line; font-size: 16px; margin-top: 20px; }
</style>
)rawliteral";

const char WEBPageHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ROLEBOT Kontrol Paneli</title>
  %s
  %s
</head>
<body>
  <h1>ROLEBOT</h1>
  <div>
    <button class="on" onclick="callAction('/relay1-on')">RELAY 1 ON / ROLE 1 AC</button>
    <button class="off" onclick="callAction('/relay1-off')">RELAY 1 OFF / ROLE 1 KAPA</button>
  </div>
  <div>
    <button class="on" onclick="callAction('/relay2-on')">RELAY 2 ON / ROLE 2 AC</button>
    <button class="off" onclick="callAction('/relay2-off')">RELAY 2 OFF / ROLE 2 KAPA</button>
  </div>
  <div>
    <button class="on" onclick="callAction('/led-on')">LED ON / ACIK</button>
    <button class="off" onclick="callAction('/led-off')">LED OFF / KAPALI</button>
  </div>
  <pre id="status">...</pre>
</body>
</html>
)rawliteral";

void setup() {
  rolebot.begin();
  rolebot.serialStart(115200);
  rolebot.Relay1Write(false);
  rolebot.Relay2Write(false);
  rolebot.ledWrite(false);

  rolebot.serverStart("AP", AP_SSID, AP_PASS);
  rolebot.serverCreateLocalPage("/", WEBPageScript, WEBPageCSS, WEBPageHTML);

  rolebot.serverOnRequest("/relay1-on", []() -> String {
    relay1On = true;
    rolebot.Relay1Write(true);
    return "RELAY1 ON";
  });
  rolebot.serverOnRequest("/relay1-off", []() -> String {
    relay1On = false;
    rolebot.Relay1Write(false);
    return "RELAY1 OFF";
  });
  rolebot.serverOnRequest("/relay2-on", []() -> String {
    relay2On = true;
    rolebot.Relay2Write(true);
    return "RELAY2 ON";
  });
  rolebot.serverOnRequest("/relay2-off", []() -> String {
    relay2On = false;
    rolebot.Relay2Write(false);
    return "RELAY2 OFF";
  });
  rolebot.serverOnRequest("/led-on", []() -> String {
    ledOn = true;
    rolebot.ledWrite(true);
    return "LED ON";
  });
  rolebot.serverOnRequest("/led-off", []() -> String {
    ledOn = false;
    rolebot.ledWrite(false);
    return "LED OFF";
  });
  rolebot.serverOnRequest("/status", []() -> String {
    String s;
    s += turkish ? "Role 1: " : "Relay 1: ";
    s += relay1On ? (turkish ? "ACIK" : "ON") : (turkish ? "KAPALI" : "OFF");
    s += "\n";
    s += turkish ? "Role 2: " : "Relay 2: ";
    s += relay2On ? (turkish ? "ACIK" : "ON") : (turkish ? "KAPALI" : "OFF");
    s += "\n";
    s += turkish ? "LED: " : "LED: ";
    s += ledOn ? (turkish ? "ACIK" : "ON") : (turkish ? "KAPALI" : "OFF");
    return s;
  });

  rolebot.serialWrite(turkish ? "Kontrol paneli hazir: http://192.168.4.1"
                              : "Control panel ready: http://192.168.4.1");
}

void loop() {
  rolebot.serverContinue();
}
