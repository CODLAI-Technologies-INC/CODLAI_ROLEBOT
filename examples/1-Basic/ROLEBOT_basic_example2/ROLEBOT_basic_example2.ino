#include <ROLEBOT.h> // ROLEBOT kütüphanesi / ROLEBOT library

// 📌 ROLEBOT nesnesi oluşturuluyor / Create a ROLEBOT object
ROLEBOT rolebot;

void setup()
{
  rolebot.begin();             // ROLEBOT başlatılıyor / Initialize ROLEBOT
  rolebot.playIntro();         // Giriş müziği çalınıyor / Play startup melody
  rolebot.serialStart(115200); // Seri haberleşmeyi başlat / Start serial communication

  // 📌 Hoşgeldiniz mesajını seri porta yazdır / Display welcome message
  rolebot.serialWrite("🚀 ROLEBOT Röle Testi Başladı! / ROLEBOT Relay Test Started!");
}

void loop()
{
  // 📌 **Buton durumunu oku / Read button state**
  bool buttonState = rolebot.button1Read(); // Buton 1'in durumunu oku / Read Button 1 state

  if (!buttonState) // Eğer butona basıldıysa / If the button is pressed
  {
    rolebot.serialWrite("🔴 Butona Basıldı! Röle 1 Açık, Röle 2 Kapalı / Button Pressed! Relay 1 ON, Relay 2 OFF");

    rolebot.Relay1Write(HIGH); // Röle 1'i aç / Turn on Relay 1
    rolebot.Relay2Write(LOW);  // Röle 2'yi kapat / Turn off Relay 2

    delay(200); // Kısa bekleme / Short delay
  }
  else // Eğer butona basılmadıysa / If button is not pressed
  {
    rolebot.serialWrite("🟢 Buton Serbest! Röle 1 Kapalı, Röle 2 Açık / Button Free! Relay 1 OFF, Relay 2 ON");

    rolebot.Relay1Write(LOW);  // Röle 1'i kapat / Turn off Relay 1
    rolebot.Relay2Write(HIGH); // Röle 2'yi aç / Turn on Relay 2

    delay(200); // Kısa bekleme / Short delay
  }
}
