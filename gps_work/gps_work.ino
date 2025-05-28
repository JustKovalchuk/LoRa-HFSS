#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 6, TXPin = 5;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;

SoftwareSerial gpsSerial(RXPin, TXPin);

void setup() {
  Serial.begin(9600);
  
  gpsSerial.begin(GPSBaud);
  Serial.println("Software Serial started at 9600 baud rate");
}
// void loop(){
//   while (gpsSerial.available() > 0){
//     byte gpsData = gpsSerial.read();
//     Serial.write(gpsData);
//   }
// }
void loop() {
  unsigned long start = millis();

  while (millis() - start < 2000) {
    while (gpsSerial.available() > 0) {
      char c = gpsSerial.read();
      gps.encode(c);
    }

    if (gps.location.isValid() && gps.location.isUpdated()) {
      Serial.print("LAT: ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("LONG: "); 
      Serial.println(gps.location.lng(), 6);
      Serial.println("---------------------------------");
    }

    if (gps.satellites.isValid() && gps.satellites.isUpdated()) {
      Serial.print("Satellites: ");
      Serial.println(gps.satellites.value());
      Serial.println("---------------------------------");
    }
    if (gps.time.isValid() && gps.time.isUpdated()) {
      Serial.print("Time in UTC: ");
      Serial.print(gps.date.year());
      Serial.print("/");
      Serial.print(gps.date.month());
      Serial.print("/");
      Serial.print(gps.date.day());
      Serial.print(", ");
      Serial.print(gps.time.hour());
      Serial.print(":");
      Serial.print(gps.time.minute());
      Serial.print(":");
      Serial.println(gps.time.second());
      Serial.println("---------------------------------");
    }
  }
}