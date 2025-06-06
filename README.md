# 📘 LoRa FHSS

> Реалізація частотного хопінгу для LoRa-пристроїв для запобігання виявленню та глушінню даного роду пристроїв.

---

## 👤 Автор

- **ПІБ**: Ковальчук Олександр
- **Група**: ФЕІ-42
- **Керівник**: Каськун Олег, асистент
- **Дата виконання**: 05.06.2025

---

## 📌 Загальна інформація

- **Тип проєкту**: Прошивка LoRa-пристрою на базі модуля Arduino
- **Мова програмування**: C/C++
- **Фреймворки / Бібліотеки**: 
    - LoRa.h — для взаємодії з LoRa-модулями.
    - Crypto.h — для реалізації симетричного шифрування AES-128, AES-256 та ChaCha20.
    - RTClib.h — для роботи з модулем реального часу.
    - TinyGPS.h  — для роботи з модулем GPS.
- **Апаратура**:
    - Dragino LoRa IoT Kit з LoRa модулями на основі чіпів SX1276/SX1278
    - Dragino LPS8 Gateway
    - Модуль реального часу (RTC) ZS-042 (DS3231)
    - GPS модуль NEO-M8N-0-10
- **Інші технології та середовища розробки**:
    - Arduino IDE
    - TTN Server
    - GNU Radio

---

## 🧠 Опис функціоналу

- FHSS реалізація з псевдовипадковими вибором частот на базі seed (LFSR);
- 2 методи синхронізації годинників:
    - MCU годинник з періодичною синхронізацією;
    - RTC годинник + GPS корегування (за наявності сигналу);
- реалізація шифрування payload частини пакету методами AES-128, AES-256, ChaCha20
- FrameCounter для запобігання повторним атакам
- унікальні deviceID та trustedDevices для ідентифікації та контролю доступу
- HMAC перевірка цілісності повідомлення

---

## 🧱 Опис основних класів / файлів

| Клас / Файл     | Призначення |
|----------------|-------------|
| `main.ino`      | Точка входу програми для Rx та Tx пристроїв |
| `auth.cpp`    | алгоритми автентифікації HMAC, побудови пакету повідомлення |
| `deviceinfo.cpp` | Засоби керувавння та доступу trustedDevices |
| `processor.cpp` | Код для отримання даних пакету та його шифрування |
| `arduino_rtc/rtc_mills_drift.ino` | код виміру дріфту між MCU, RTC та GPS годинниками |
| `gps_work/gps_work.ino` | тестування роботи GPS модуля |
| `arduino_encryption_chacha/arduino_encryption_chacha.ino` | реалізація ChaCha20 шифрування |
| `arduino_encryption_aes/arduino_encryption_aes.ino` | реалізація AES шифрування |
| `gnuRadio/my_fhss.grc` | GNU Radio файл з CSS та FHSS моделюванням |
| `lorawan/lorawan_abp.ino` | код LoRaWAN Tx-пристрою з ABP автентифікацією |
| `lorawan/lorawan_otaa.ino` | код LoRaWAN Tx-пристрою з OTAA автентифікацією |

---

## ▶️ Як запустити проєкт "з нуля"

### 1. Необхідно мати внаявності відповідну апаратуру зазначену вище

- Два Arduino UNO
- Два пристрої LoRa (мінімальна необзіжна апаратура)
- З'єднання схеми

### 2. Встановлення середовища розробки та бібліотек

- Arduino IDE
- LoRa.h
- Crypto.h
- RTClib.h
- TinyGPS.h

### 3. Клонування репозиторію

```bash
git clone https://github.com/JustKovalchuk/LoRa-HFSS.git
```

### 4. Запуск

- прошивка двох LoRa пристроїв за допомогою Arduino IDE  (один запрогамований як приймач, інший як передавач через відповідну булеву змінну у main.ini ```bool isTx = true;```)