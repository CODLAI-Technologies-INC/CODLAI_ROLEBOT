# ROLEBOT Library User Guide
This library is specially designed and produced by the CODLAI developer team to control the ROLEBOT product.
![alt text](<images/1.png>)

## Using with Arduino IDE

### Installation

1. Open the Arduino IDE.
2. Go to "Sketch" -> "Include Library" -> "Manage Libraries..." from the menu bar.
3. Type "ROLEBOT" in the search box.
4. Find the ROLEBOT library and click the "Install" button to complete the installation.

## Using with PlatformIO

```ini
lib_deps = samed5497kaya/CODLAI_ROLEBOT
; Firebase / e-mail (USE_FIREBASE, USE_EMAIL) not used? Skip these two big
; libraries: faster builds and no "path too long" (260 char) errors on Windows.
lib_ignore = Firebase Arduino Client Library for ESP8266 and ESP32, ESP Mail Client
```

# ROLEBOT Kütüphanesi Kullanım Kılavuzu
Bu kütüphane CODLAI geliştirici ekibi tarafından ROLEBOT ürününü kontrol etmek için özel olarak tasarlanmış ve üretilmiştir.
![alt text](<images/1.png>)

## Arduino IDE ile Kullanım

### Kurulum

1. Arduino IDE'yi açın.
2. Menu çubuğundan "Sketch" -> "Include Library" -> "Manage Libraries..." seçeneğine gidin.
3. Arama kutusuna "ROLEBOT" yazın.
4. ROLEBOT kütüphanesini bulun ve "Install" düğmesine tıklayarak kurulumu tamamlayın.

## PlatformIO ile Kullanım

```ini
lib_deps = samed5497kaya/CODLAI_ROLEBOT
; Firebase / e-posta (USE_FIREBASE, USE_EMAIL) kullanmiyorsaniz bu iki buyuk
; kutuphaneyi atlayin: derleme hizlanir, Windows'ta "yol cok uzun" (260
; karakter) hatasi olmaz.
lib_ignore = Firebase Arduino Client Library for ESP8266 and ESP32, ESP Mail Client
```

# About ROLEBOT / ROLEBOT Hakkında
ROLEBOT is a development board that includes two programmable relays, buttons, and LED lights. It provides power through a Type-C port and allows programming in C++ & MicroPython via the CODLAI editor. With its built-in Wi-Fi connection, it enables internet-based applications and can be used either in remote control mode or as a local network.

ROLEBOT's dual relays can be used to control various electronic devices using a phone or other smart devices. For example, a coffee machine connected to ROLEBOT can be programmed to start at a specified wake-up time. Similarly, it can be used to control home lighting through a smartphone. The applications are limited only by imagination.

## Processor:
ESP8266EX - 160 MHz Tensilica L106 32-bit RISC microcontroller

## Wireless Connectivity:
2.4 GHz WiFi

## Programming Languages:
C++, MicroPython (Text and Block Based)

## Built-in Sensors:
- 2x Relay (5V DC 100mA)
- Digital Button
- Digital LED

## Peripheral Input/Output:
- 1 x Type-C Socket (Programming + Power)
- 2 x Programmable Relay Output

## Additional Features:
- Remote control of 220V AC devices via Wi-Fi
- Visual feedback LEDs for sensors
- Reset button
- Shock-resistant outer casing
- CODLAI editor support
- CODLAI platform support
- On-device information sections
- Open-source editor compatibility
- Comprehensive documentation for IoT applications
- Persistent storage helpers (EEPROM): int16/int32/float/string/bytes via `eeprom*` functions
- CRC-protected EEPROM records (versioned): `eepromWriteRecord/eepromReadRecord`
- NTP time sync helpers: `ntpSync/ntpGetDateTimeString` (requires WiFi)
- OTA firmware updates: `otaBegin/otaHandle` (requires WiFi)
- `triggerIFTTTEvent()` connects ROLEBOT to IFTTT services (Google Sheets, Gmail, Discord, smart lights, etc.). Start with `examples/2-Advanced/ROLEBOT_IFTTT_Webhook_Example/ROLEBOT_IFTTT_Webhook_Example.ino` to log relay events in the cloud.

## Security:
AES and SSL/TLS hardware accelerators

## Certifications:
CE, ROSH, EMC

## Operating Voltage:
5VDC

### Dependencies & Versions
- **Platform:** Espressif 8266 (ESP8266)
- **Framework:** Arduino
- **Libraries:**
  - ESPAsyncWebServer
  - Firebase Arduino Client Library for ESP8266 and ESP32
  - ArduinoJson
- **Offline Libraries:**
  - `other_libraries.zip`: Contains all required library dependencies for offline installation.

<!-- EXAMPLES:START -->
# Examples / Örnekler

**EN:** 28 examples - Basic (2), Advanced (22), Projects (4). Every example follows the same rules:
- **Turkish / English:** `bool turkish = true;` at the top picks the language. Type `lang` (or `dil`) in the Serial Monitor to switch while it runs. Serial, LCD and web texts follow it.
- **Serial port (115200 baud):** commands work in both languages (`help` = `yardim`, `angle 90` = `aci 90` = `açı 90`) and with any line-ending setting. Type `help` for the list.
- **Auto / manual:** 5 examples that drive something (motor, servo, relay, LED, buzzer, robot) start in **AUTO** mode with a demo. Press the **B1** button (GPIO0) to switch to **MANUAL**. In manual mode a short press toggles the relay, a 1 s hold returns to auto; serial commands work too. An actuator command sent from serial also switches to manual.
- 12 examples need your own settings (WiFi, tokens, keys): fill in the `YOUR_...` placeholders.
- Each example is in its own folder (`Folder/Folder.ino`), so it shows up under *File > Examples* in the Arduino IDE.
- `examples/examples.json` lists every example with its board, required modules, summary (TR/EN) and serial commands (used by editor.codlai.com).

**TR:** 28 örnek - Temel (2), İleri (22), Projeler (4). Tüm örnekler aynı kurallara uyar:
- **Türkçe / İngilizce:** En üstteki `bool turkish = true;` dili seçer. Çalışırken Seri Monitör'e `dil` (veya `lang`) yazarak değiştirebilirsiniz. Seri port, LCD ve web metinleri seçilen dili izler.
- **Seri port (115200 baud):** komutlar iki dilde de çalışır (`yardim` = `help`, `aci 90` = `açı 90` = `angle 90`) ve satır sonu ayarı ne olursa olsun algılanır. Komut listesi için `yardim` yazın.
- **Otomatik / manuel:** Bir şey süren 5 örnek (motor, servo, röle, LED, buzzer, robot) **OTOMATİK** modda bir gösteriyle başlar. **B1** butonu (GPIO0) ile **MANUEL** moda geçersiniz. Manuel modda kısa basış röleyi değiştirir, 1 sn basılı tutmak otomatiğe döndürür; seri komutlar da çalışır. Seri porttan gönderilen bir çalıştırma komutu da manuel moda geçirir.
- 12 örnek sizin ayarlarınızı ister (WiFi, token, anahtar): `YOUR_...` yer tutucularını doldurun.
- Her örnek kendi klasöründedir (`Klasör/Klasör.ino`); Arduino IDE'de *Dosya > Örnekler* menüsünde görünür.
- `examples/examples.json` her örneği kartı, gerektirdiği modüller, özeti (TR/EN) ve seri komutlarıyla listeler (editor.codlai.com kullanır).
<!-- EXAMPLES:END -->

# Library Structure & Contributing / Kütüphane Yapısı ve Katkıda Bulunma
This library follows a modular design pattern to ensure efficiency.
- **Configuration:** Use `ROLEBOT_Config.h` to enable/disable specific modules (e.g., WiFi, Firebase).
- **Extension:** To add new features, define a new flag in the config file and wrap your code in `#if defined(...)` blocks.

Bu kütüphane verimliliği sağlamak için modüler bir tasarım deseni izler.
- **Yapılandırma:** Belirli modülleri (örn. WiFi, Firebase) etkinleştirmek/devre dışı bırakmak için `ROLEBOT_Config.h` dosyasını kullanın.
- **Genişletme:** Yeni özellikler eklemek için yapılandırma dosyasında yeni bir bayrak tanımlayın ve kodunuzu `#if defined(...)` blokları içine alın.

# Katkıda Bulunma / Contributing
Katkıda bulunmak isterseniz, lütfen GitHub deposuna Pull Request gönderin. / If you'd like to contribute, please send a Pull Request to the GitHub repository.

# Lisans / License
Bu kütüphane 2024 Yılında CODLAI Teknoloji tarafından lisanslanmıştır. Detaylar için LICENSE dosyasına bakınız. / Copyright (c) 2024 CODLAI Teknoloji. All rights reserved.

See the LICENSE file for details.

![alt text](<images/2.png>)
![alt text](<images/3.png>)
![alt text](<images/4.png>)
![alt text](<images/5.png>)

