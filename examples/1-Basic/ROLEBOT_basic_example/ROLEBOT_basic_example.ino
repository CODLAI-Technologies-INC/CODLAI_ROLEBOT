#include <ROLEBOT.h> // ROLEBOT kütüphanesi / ROLEBOT library

// 📌 ROLEBOT nesnesi oluşturuluyor / Create a ROLEBOT object
ROLEBOT rolebot;

void setup()
{
  rolebot.begin();             // ROLEBOT başlatılıyor / Initialize ROLEBOT
  rolebot.playIntro();         // Giriş müziği çalınıyor / Play startup melody
  rolebot.serialStart(115200); // Seri haberleşmeyi başlat / Start serial communication

  // 📌 Hoşgeldiniz mesajını seri porta yazdır / Display welcome message
  rolebot.serialWrite("🚀 ROLEBOT Buton Testi Başladı! / ROLEBOT Button Test Started!");
}

void loop()
{
  // 📌 **Buton durumlarını oku / Read button states**
  bool buttonState = rolebot.button1Read(); // Buton 1'in durumunu oku / Read Button 1 state

  if (!buttonState) // Eğer butona basıldıysa / If the button is pressed
  {
    rolebot.serialWrite("🔴 Butona Basıldı! / Button Pressed!");
    rolebot.ledWrite(HIGH); // LED'i aç / Turn on LED
    delay(200);             // Kısa bekleme / Short delay
  }
  else // Butona basılmadıysa / If button is not pressed
  {
    rolebot.serialWrite("🟢 Buton Serbest / Button Free");
    rolebot.ledWrite(LOW); // LED'i kapat / Turn off LED
    delay(200);            // Kısa bekleme / Short delay
  }
}
