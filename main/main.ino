#include <SPI.h>
#include <LoRa.h>
#include "auth.h"
#include "init.h"
#include "deviceinfo.h"

#include <Wire.h>
#include <RTClib.h>

bool isTx = false;
bool isRtcConnected = false;
DateTime baseRTCTime(2025, 5, 23, 0, 0, 0);
// bool isDisplayConnected = false;

int syncHopIndex = 0;
unsigned long syncTime = 0;

RTC_DS3231 rtc;



// #define SCREEN_WIDTH 128 
// #define SCREEN_HEIGHT 8
// // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);



#pragma region RX_VARS

DeviceInfo trustedDevices[] = {
  {
    "NODE_ALPHA", 
    {  
      0x7D, 0x3A, 0xB1, 0xE8, 0x4F, 0x90, 0x23, 0x56,
      0x88, 0xC1, 0xA4, 0xF2, 0x6E, 0x9D, 0x37, 0xE3,
      0x5C, 0xFA, 0xB0, 0x48, 0x13, 0xAC, 0x1F, 0xD2,
      0x9A, 0x77, 0x3E, 0x65, 0xB4, 0xDC, 0x11, 0x09
    },
    0
  },
  {
    "NODE_BRAVO", 
    {
      0xC4, 0x1E, 0x28, 0x95, 0xAF, 0x62, 0xDB, 0x33,
      0x17, 0x4A, 0x60, 0xB3, 0x02, 0x8C, 0x7F, 0xD7,
      0xE1, 0x3D, 0xA9, 0xBC, 0xF4, 0x0A, 0x94, 0x71,
      0x5D, 0x08, 0x36, 0xC9, 0x6B, 0xD5, 0xEF, 0x00
    }, 
    0
  },
};

const int deviceCount = sizeof(trustedDevices) / sizeof(trustedDevices[0]);



bool synced = false;

int requestPerMinute = 60000 / hopInterval;

const int REPLAY_WINDOW = 5 * requestPerMinute;

#pragma endregion



#pragma region TX_VARS

DeviceInfo myDevice = trustedDevices[0]

unsigned long syncDelay = 100;

uint32_t frameCounter = 0; // оновлюється після кожного пакету

#pragma endregion



unsigned long getRTCTime() {
  return (rtc.now().unixtime()-baseRTCTime.unixtime())*1000;
}

bool IsNewHop(){
  if (isRtcConnected) {
    return getRTCTime() - lastHopTime >= hopInterval;
  }
  else {
    return millis() - lastHopTime >= hopInterval;
  }
}

void setInitHopIndex() {
  if (isRtcConnected){
    unsigned long timeSinceSync = getRTCTime();
    unsigned long hopsSinceSync = timeSinceSync/hopInterval;
    unsigned long int currentFreqIndex = hopsSinceSync / freqCount;
    lastHopTime = hopsSinceSync*hopInterval;
    hopIndex = hopsSinceSync % freqCount;
  }
  Serial.println("_____________________");
}

void UpdateLastHopTime() {
  if (isRtcConnected){
    unsigned long timeSinceSync = getRTCTime();
    unsigned long hopsSinceSync = timeSinceSync/hopInterval;
    unsigned long currentFreqIndex = hopsSinceSync / freqCount;
    lastHopTime = hopsSinceSync*hopInterval;
  }
  else {
    unsigned long timeSinceSync = millis()-syncTime;
    int hopsSinceSync = timeSinceSync/hopInterval;
    int currentFreqIndex = (syncHopIndex + hopsSinceSync) / freqCount;
    lastHopTime = syncTime + hopsSinceSync*hopInterval;
  }
}

int pseudoRandom(int index, uint32_t seed) {
  uint32_t lfsr = seed;

  // Прокручуємо LFSR index разів — генерація псевдовипадкового числа
  for (int i = 0; i < index; i++) {
    // Простий 32-бітний LFSR
    lfsr ^= lfsr << 13;
    lfsr ^= lfsr >> 17;
    lfsr ^= lfsr << 5;
  }

  return lfsr % freqCount; // повертаємо індекс із freqList
}

long getCurrentFreq(int hopIndex, bool isRandom=true) {
  int freqIdx = hopIndex;
  if (isRandom) {
    freqIdx = pseudoRandom(hopIndex, seed);
  }

  return freqList[freqIdx];
}

long nextHop(bool isRandom=true) {
  hopIndex = (hopIndex + 1) % freqCount;
  long currentFreq = getCurrentFreq(hopIndex, isRandom);
  LoRa.setFrequency(currentFreq);

  UpdateLastHopTime();

  return currentFreq;
}



#pragma region TX

void SendSyncPacket(long basicFreq) {
  LoRa.setFrequency(SYNC_FREQ);
  LoRa.beginPacket();
  LoRa.print("SYNC:");
  LoRa.print(hopIndex);
  LoRa.endPacket();
  Serial.print("Sent SYNC with index ");
  Serial.println(hopIndex);
  
  LoRa.setFrequency(basicFreq);
}

void setupTx(){
  Serial.begin(9600);
  while (!Serial);

  Wire.begin();
  // Try to connect to RTC 10 times
  for (int attempts = 0; attempts < 10; attempts++) {
    if (rtc.begin()) {
      isRtcConnected = true;
      Serial.println("✅ RTC connected.");
      break;
    } else {
      Serial.print("⏳ RTC not responding (attempt ");
      Serial.print(attempts + 1);
      Serial.println("/10)");
      delay(300);
    }
  }

  if (!isRtcConnected) {
    Serial.println("⚠️ RTC not found. Continuing without RTC.");
  }

  LoRa.setPins(10, 9, 2); // NSS, RESET, DIO0
  if (!LoRa.begin(SYNC_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Transmitter ready");
}
void loopTx() {
  if (!synced) {
    if (isRtcConnected) {
      setInitHopIndex();
    }
    synced = true;
  }

  if (IsNewHop()) {
    long currentFreq = nextHop();

    Serial.print("RTCtime: ");
    Serial.println(getRTCTime());
    Serial.print("lastHopTime: ");
    Serial.println(lastHopTime);
    Serial.print("hopInterval: ");
    Serial.println(hopInterval);

    if (!isRtcConnected){
      SendSyncPacket(currentFreq);
      delay(syncDelay);
    }
    
    String payload = "MSG: Hello on " + String(currentFreq);
    String fullPacket = getSecureMessage(payload, myDevice.deviceID, frameCounter, myDevice.hmacKey, sizeof(myDevice.hmacKey));
    
    LoRa.beginPacket();
    LoRa.print(fullPacket);
    LoRa.endPacket();

    Serial.println("Sent: " + fullPacket);

    frameCounter++;
    Serial.print("RTC unixTime: ");
    Serial.println(getRTCTime());

    Serial.print("lastHopTime: ");
    Serial.println(lastHopTime);

    Serial.println("-----------------------------");
  }

  delay(50);
}

#pragma endregion



#pragma region RX

void processPacket(String packet) {
  int pos1 = packet.indexOf('|');
  int pos2 = packet.indexOf('|', pos1 + 1);
  int pos3 = packet.indexOf('|', pos2 + 1);

  if (pos1 == -1 || pos2 == -1 || pos3 == -1) {
    Serial.println("❌ Invalid format");
    return;
  }

  String deviceIDStr = packet.substring(0, pos1);
  String frameStr = packet.substring(pos1 + 1, pos2);
  String payload = packet.substring(pos2 + 1, pos3);
  String receivedHMAC = packet.substring(pos3 + 1);

  DeviceInfo* device = findDevice(deviceIDStr, trustedDevices, deviceCount);
  if (device == nullptr) {
    Serial.println("🚫 Unknown device ID: " + deviceIDStr);
    return;
  }

  int32_t receivedFrameCounter = frameStr.toInt();
  Serial.println("receivedFrameCounter: " + String(receivedFrameCounter) + "; lastFrameCounter: " + String(device->lastFrameCounter));
  // Перевірка на повтор
  if (receivedFrameCounter <= device->lastFrameCounter) {
    Serial.println("⚠️ Replay attempt — frame too old");
    return;
  }

  if (receivedFrameCounter > device->lastFrameCounter + REPLAY_WINDOW) {
    Serial.println("⚠️ Frame too far ahead — possible sync issue");
    return;
  }

  String base = deviceIDStr + "|" + frameStr + "|" + payload;

  if (verifyHMAC(base, device->hmacKey, sizeof(device->hmacKey), receivedHMAC)) {
    device->lastFrameCounter = receivedFrameCounter;
    Serial.println("✅ Accepted from " + deviceIDStr + ": " + payload + "(" + frameStr + ")");
    // if (isDisplayConnected) {
    //   display.clearDisplay();
    //   display.setCursor(0, 0);

    //   display.println(deviceIDStr + ": " + payload + "(" + frameStr + ")");
    //   display.display(); 
    // }
  } 
  else {
    Serial.println("❌ Invalid HMAC — possibly spoofed");
  }
}

void setupRx() {
  Serial.begin(115200);
  while (!Serial);
  
  Wire.begin();
  // Try to connect to RTC 10 times
  for (int attempts = 0; attempts < 10; attempts++) {
    if (rtc.begin()) {
      isRtcConnected = true;
      Serial.println("✅ RTC connected.");
      break;
    } else {
      Serial.print("⏳ RTC not responding (attempt ");
      Serial.print(attempts + 1);
      Serial.println("/10)");
      delay(300);
    }
  }

  if (!isRtcConnected) {
    Serial.println("⚠️ RTC not found. Continuing without RTC.");
  }

  // if (isDisplayConnected) {
  //   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
  //     Serial.println(F("SSD1306 allocation failed"));
  //     for(;;);
  //   }
  // }

  LoRa.setPins(10, 9, 2); // NSS, RESET, DIO0
  if (!LoRa.begin(SYNC_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }


  String printString = "Receiver ready, waiting for SYNC...";
  Serial.println(printString);

  // if (isDisplayConnected) {
  //   display.clearDisplay();
  //   display.setTextSize(1);
  //   display.setTextColor(WHITE);
  //   display.setCursor(0, 0);

  //   display.println(printString);
  //   display.display(); 
  // }
}
void loopRx() {
  int packetSize;
  String message;

  if (!synced) {
    if (isRtcConnected) {
      setInitHopIndex();
      Serial.print("Got RTC SYNC. Index: ");
      Serial.println(hopIndex);
      synced = true;
      long currentFreq = getCurrentFreq(hopIndex);
      LoRa.setFrequency(currentFreq);
    }
    else {
      LoRa.setFrequency(SYNC_FREQ);
      packetSize = LoRa.parsePacket();
      if (packetSize) {
        message = "";
        while (LoRa.available()) {
          message += (char)LoRa.read();
        }
        if (message.startsWith("SYNC:")) {
          syncTime = millis();
          syncHopIndex = message.substring(5).toInt();
          hopIndex = syncHopIndex;
          Serial.print("Got SYNC. Index: ");
          Serial.println(hopIndex);
          synced = true;
          long currentFreq = getCurrentFreq(hopIndex);
          LoRa.setFrequency(currentFreq);
          lastHopTime = millis();
        }
      }
    }
    return;
  }
  
  // Отримання даних на синхронізованій частоті
  packetSize = LoRa.parsePacket();
  if (packetSize) {
    message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    processPacket(message);

    // Serial.print("Received: ");
    // Serial.println(message);
  }

  // Перехід на нову частоту
  if (IsNewHop()) {
    Serial.println("-----------------------------");

    Serial.print("RTCtime: ");
    Serial.println(getRTCTime());
    Serial.print("lastHopTime: ");
    Serial.println(lastHopTime);
    Serial.print("hopInterval: ");
    Serial.println(hopInterval);

    long currentFreq = nextHop();
    Serial.print("Hopped with hop index ");
    Serial.print(hopIndex);
    Serial.print(" to: ");
    Serial.println(currentFreq);
  }
}

#pragma endregion


void setup(){
  if (isTx){
    setupTx();
  }
  else {
    setupRx();
  }
}

void loop() {
  if (isTx){
    loopTx();
  }
  else {
    loopRx();
  }
}
