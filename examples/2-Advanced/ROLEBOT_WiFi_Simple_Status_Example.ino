// TR: KABLOSUZ ILETISIME ILK ADIM - En basit WiFi ornegi. ROLEBOT'u evinizin
// WiFi agina baglar, baglanti basarili olursa aldigi IP adresini Seri
// Port'ta gosterir ve LED'i yakar. ROLEBOT'ta LCD ekran olmadigi icin tum
// bilgiler Seri Port (USB) uzerinden verilir. Sunucu YOK, web sayfasi YOK -
// sadece "ag'a katilmak" ne demek onu ogretir.
// EN: FIRST STEP INTO WIRELESS COMMUNICATION - the simplest WiFi example.
// Connects ROLEBOT to your home WiFi network and, once connected, shows
// the IP address on Serial and turns the LED on. ROLEBOT has no LCD
// screen, so all feedback is given through the Serial (USB) monitor.
// NO server, NO web page - just teaches what "joining a network" means.

#define USE_WIFI
#include <ROLEBOT.h>

ROLEBOT rolebot;

// TR/EN: Bu degeri false yapip yeniden yukleyerek dili degistirebilirsiniz.
// Change this to false and re-upload to switch the language.
bool turkish = true;

// ONEMLI: Kendi WiFi agranizin adini ve sifresini yazin.
// IMPORTANT: Fill in your own WiFi network's name and password.
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASS "WIFI_PASSWORD"

void setup() {
  rolebot.begin();
  rolebot.serialStart(115200);
  rolebot.serialWrite(turkish ? "WiFi'ye baglaniliyor..." : "Connecting to WiFi...");

  rolebot.wifiStartAndConnect(WIFI_SSID, WIFI_PASS);

  if (rolebot.wifiConnectionControl()) {
    String ip = rolebot.wifiGetIPAddress();
    rolebot.serialWrite(turkish ? "Baglandi! IP adresi: " + ip : "Connected! IP address: " + ip);
    rolebot.ledWrite(true); // Baglanti basarili -> LED yanik kalir / Connected -> LED stays on
  } else {
    rolebot.serialWrite(turkish ? "Baglanti basarisiz! SSID/sifreyi kontrol edin."
                                 : "Connection failed! Check SSID/password.");
    rolebot.ledWrite(false);
  }
}

void loop() {
  // Bu basit ornekte yapilacak baska bir sey yok - baglanti bilgisi zaten
  // setup()'ta gosterildi. / Nothing else to do in this simple example -
  // connection info was already shown in setup().
  delay(1000);
}
