#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SYNC_PIN 2

RTC_DS3231 rtc;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DateTime lastRTC;
bool firstCheck = true;
DateTime launchRTCTime;
unsigned long launchMillisTime;

void CalibrateTime() {
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void setup() {
  pinMode(SYNC_PIN, INPUT);
  Serial.begin(9600);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("No RTC!");
    while (1);
  }

  CalibrateTime();
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("display init...");
  display.display();
  delay(1000);
}

void loop() {
  unsigned long currentMillis = millis();
  DateTime nowRTC = rtc.now();
  
  if (firstCheck) {
    launchMillisTime = currentMillis;
    lastRTC = nowRTC;
    launchRTCTime = nowRTC;
    firstCheck = false;
  }

  // Оновлення дисплея раз на секунду
  if (nowRTC.second() != lastRTC.second()) {
    float rtcElapsed = (nowRTC - launchRTCTime).totalseconds();
    float millisElapsed = (currentMillis - launchMillisTime)/1000.0;
    
    Serial.println("----------------------------------");
    Serial.print("RTC пройшло (сек): ");
    Serial.println(rtcElapsed, 3);
    Serial.print("MCU пройшло (сек): ");
    Serial.println(millisElapsed);
    Serial.print("Дрейф (сек): ");
    Serial.println(rtcElapsed - millisElapsed);
    Serial.println("----------------------------------");
    
    // Оновлення референсних значень
    lastRTC = nowRTC;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    char buf[20];
    display.print("RTC Time: ");
    sprintf(buf, "%02d:%02d:%02d", nowRTC.hour(), nowRTC.minute(), nowRTC.second());
    display.println(buf);

    display.print("RTC passed: ");
    display.print(rtcElapsed);
    display.println("s");
    display.print("MCU passed: ");
    display.print(millisElapsed);
    display.println("s");
    display.print("Drift: ");
    display.print(rtcElapsed - millisElapsed);
    display.println("s");

    // display.print("Drift: ");
    // display.print(drift, 3);
    // display.println(" s");

    // display.print("SUM Drift: ");
    // display.print(cumulativeDrift, 3);
    // display.println(" s");

    // display.print("SUM|Drift|: ");
    // display.print(cumulativeAbsDrift, 3);
    // display.println(" s");

    display.display();
  }

  // Необов’язково: коротка затримка для зменшення навантаження
  // delay(100);
}

// #include <TinyGPS++.h>
// #include <SoftwareSerial.h>

// static const int RXPin = 6, TXPin = 5;
// static const uint32_t GPSBaud = 9600;

// TinyGPSPlus gps;

// SoftwareSerial gpsSerial(RXPin, TXPin);

// void setup() {
//   Serial.begin(9600);
  
//   gpsSerial.begin(GPSBaud);
//   Serial.println("Software Serial started at 9600 baud rate");
// }
// // void loop(){
// //   while (gpsSerial.available() > 0){
// //     byte gpsData = gpsSerial.read();
// //     Serial.write(gpsData);
// //   }
// // }
// void loop() {
//   unsigned long start = millis();

//   while (millis() - start < 2000) {
//     while (gpsSerial.available() > 0) {
//       char c = gpsSerial.read();
//       gps.encode(c);
//     }

//     if (gps.location.isValid() && gps.location.isUpdated()) {
//       Serial.print("LAT: ");
//       Serial.println(gps.location.lat(), 6);
//       Serial.print("LONG: "); 
//       Serial.println(gps.location.lng(), 6);
//       Serial.println("---------------------------------");
//     }

//     if (gps.satellites.isValid() && gps.satellites.isUpdated()) {
//       Serial.print("Satellites: ");
//       Serial.println(gps.satellites.value());
//       Serial.println("---------------------------------");
//     }
//     if (gps.time.isValid() && gps.time.isUpdated()) {
//       Serial.print("Time in UTC: ");
//       Serial.print(gps.date.year());
//       Serial.print("/");
//       Serial.print(gps.date.month());
//       Serial.print("/");
//       Serial.print(gps.date.day());
//       Serial.print(", ");
//       Serial.print(gps.time.hour());
//       Serial.print(":");
//       Serial.print(gps.time.minute());
//       Serial.print(":");
//       Serial.println(gps.time.second());
//       Serial.println("---------------------------------");
//     }
//   }
// }
