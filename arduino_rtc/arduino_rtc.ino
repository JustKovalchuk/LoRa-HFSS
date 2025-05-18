#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void CalibrateTime() {
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("No RTC!");
    while (1);
  }

  CalibrateTime();
  if (rtc.lostPower()) {
    Serial.println("RTC lost power");
    // Встановлення поточного часу при першому запуску
    CalibrateTime();
  }
}

void loop() {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  delay(1000);
}
