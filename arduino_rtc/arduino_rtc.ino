#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SYNC_PIN 2

RTC_DS3231 rtc;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

  // if (rtc.lostPower()) {
  //   Serial.println("RTC lost power");
  //   // Встановлення поточного часу при першому запуску
  //   CalibrateTime();
  // }

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("display init.");
  display.display(); 
  delay(1000);
  display.setCursor(0, 0);
  display.println("display init..");
  display.display(); 
  delay(1000);
  display.setCursor(0, 0);
  display.println("display init...");
  display.display(); 
  delay(1000);
}

void loop() {
  if (digitalRead(SYNC_PIN) == HIGH) {
    Serial.println("Calibrate Time");
    CalibrateTime();
  }
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

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("ARDUINO2");
  display.display();

  // display.println("RTC TIME:");
  // display.print(now.year());
  // display.print("-");
  // display.print(now.month());
  // display.print("-");
  // display.print(now.day());
  // display.print(" ");
  // display.print(now.hour());
  // display.print(":");
  // display.print(now.minute());
  // display.print(":");
  // display.println(now.second());

  // // display.println("RTC" + String(now.year()) + String(now.month()) + String(now.day()) + String(now.hour()) + Strring(now.minute()) + String(now.second()));
  // display.display();
}
