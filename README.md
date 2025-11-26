# ROLEBOT Library User Guide
This library is specially designed and produced by the CODLAI developer team to control the ROLEBOT product.
![alt text](<images/1.png>)

## Using with Arduino IDE

### Installation

1. Open the Arduino IDE.
2. Go to "Sketch" -> "Include Library" -> "Manage Libraries..." from the menu bar.
3. Type "ROLEBOT" in the search box.
4. Find the ROLEBOT library and click the "Install" button to complete the installation.

# ROLEBOT Kütüphanesi Kullanım Kılavuzu
Bu kütüphane CODLAI geliştirici ekibi tarafından ROLEBOT ürününü kontrol etmek için özel olarak tasarlanmış ve üretilmiştir.
![alt text](<images/1.png>)

## Arduino IDE ile Kullanım

### Kurulum

1. Arduino IDE'yi açın.
2. Menu çubuğundan "Sketch" -> "Include Library" -> "Manage Libraries..." seçeneğine gidin.
3. Arama kutusuna "ROLEBOT" yazın.
4. ROLEBOT kütüphanesini bulun ve "Install" düğmesine tıklayarak kurulumu tamamlayın.

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
Bu kütüphane 2024 Yılında Samed KAYA tarafından lisanslanmıştır. Detaylar için LICENSE dosyasına bakınız. / Copyright (c) 2024 Samed KAYA. All rights reserved.

See the LICENSE file for details.

![alt text](<images/2.png>)
![alt text](<images/3.png>)
![alt text](<images/4.png>)
![alt text](<images/5.png>)

