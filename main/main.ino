#include <SPI.h>
#include <LoRa.h>
#include "auth.h"
#include "init.h"
#include "deviceinfo.h"
#include "processor.h"

#include <Wire.h>
#include <RTClib.h>

bool isTx = true;
bool isRtcConnected = false;
DateTime baseRTCTime(2025, 6, 1, 0, 0, 0);

int syncHopIndex = 0;
unsigned long syncTime = 0;
bool synced = false;

RTC_DS3231 rtc;
DeviceInfo myDevice;

DeviceInfo trustedDevices[] = {
  {
    "ALPHA", 
    {  
      0x7D, 0x3A, 0xB1, 0xE8, 0x4F, 0x90, 0x23, 0x56,
      0x88, 0xC1, 0xA4, 0xF2, 0x6E, 0x9D, 0x37, 0xE3,
      0x5C, 0xFA, 0xB0, 0x48, 0x13, 0xAC, 0x1F, 0xD2,
      0x9A, 0x77, 0x3E, 0x65, 0xB4, 0xDC, 0x11, 0x09
    },
    0
  },
  {
    "BRAVO", 
    {
      0xC4, 0x1E, 0x28, 0x95, 0xAF, 0x62, 0xDB, 0x33,
      0x17, 0x4A, 0x60, 0xB3, 0x02, 0x8C, 0x7F, 0xD7,
      0xE1, 0x3D, 0xA9, 0xBC, 0xF4, 0x0A, 0x94, 0x71,
      0x5D, 0x08, 0x36, 0xC9, 0x6B, 0xD5, 0xEF, 0x00
    }, 
    0
  },
};

#pragma region RX_VARS

const int deviceCount = sizeof(trustedDevices) / sizeof(trustedDevices[0]);

int requestPerMinute = 60000 / hopInterval;

const int REPLAY_WINDOW = 10000;//5 * requestPerMinute;

#pragma endregion



#pragma region TX_VARS

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
  Serial.println("_____________________");
  Serial.print("Got RTC SYNC.");
  if (isRtcConnected){
    unsigned long timeSinceSync = getRTCTime();
    unsigned long hopsSinceSync = timeSinceSync/hopInterval;
    unsigned long int currentFreqIndex = hopsSinceSync / freqCount;
    lastHopTime = hopsSinceSync*hopInterval;
    hopIndex = hopsSinceSync % freqCount;
    Serial.print("hopIndex: ");
    Serial.println(hopIndex);
    Serial.print("freqCount: ");
    Serial.println(freqCount);
    Serial.print("hopsSinceSync: ");
    Serial.println(hopsSinceSync);
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
  delay(1000);

  Wire.begin();
  // Try to connect to RTC 10 times
  for (int attempts = 0; attempts < 10; attempts++) {
    if (rtc.begin()) {
      isRtcConnected = true;
      Serial.println("RTC connected.");
      break;
    } else {
      Serial.print("RTC not responding (attempt ");
      Serial.print(attempts + 1);
      Serial.println("/10)");
      delay(300);
    }
  }

  if (!isRtcConnected) {
    Serial.println("RTC not found. Continuing without RTC.");
  }

  LoRa.setPins(10, 9, 2); // NSS, RESET, DIO0
  if (!LoRa.begin(SYNC_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  // LoRa.setSpreadingFactor(9);     // Або те, що вам потрібно
  // LoRa.setSignalBandwidth(100E3); // Стандарт
  // LoRa.setCodingRate4(5);         // Покращення надійності
  // LoRa.setSyncWord(0x34); 

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
    Serial.print("RTC unixTime: ");
    Serial.println(getRTCTime());

    Serial.print("lastHopTime: ");
    Serial.println(lastHopTime);

    Serial.println("-----------------------------");
  }

  long currentFreq = getCurrentFreq(hopIndex);
  char payload[64];
  snprintf(payload, sizeof(payload), "MSG:FREQ:%lu:end", currentFreq);
  char packet[140];
  getSecureMessage(payload, myDevice.deviceID, frameCounter, myDevice.hmacKey, sizeof(myDevice.hmacKey), packet, sizeof(packet));
  
  LoRa.beginPacket();
  LoRa.print(packet);
  LoRa.endPacket();
  delay(1000);

  Serial.print("Sent: ");
  Serial.println(packet);

  frameCounter++;
  delay(50);
}

#pragma endregion



#pragma region RX

void processPacket(char* packet) {
  char* pos1 = strchr(packet, '|');
  if (!pos1) { 
    Serial.println("Invalid format"); 
    return; 
  }
  *pos1 = '\0';
  char* pos2 = strchr(pos1 + 1, '|');
  if (!pos2) { 
    Serial.println("Invalid format"); 
    return; 
  }
  *pos2 = '\0';
  char* pos3 = strchr(pos2 + 1, '|');
  if (!pos3) { 
    Serial.println("Invalid format"); 
    return; 
  }

  *pos3 = '\0';

  char* deviceIDStr = packet;
  char* frameStr = pos1 + 1;
  char* payload = pos2 + 1;
  char* receivedHMAC = pos3 + 1;

  DeviceInfo* device = findDevice(deviceIDStr, trustedDevices, deviceCount);
  if (device == nullptr) {
    Serial.print("Unknown device ID: ");
    Serial.println(deviceIDStr);
    return;
  }

  int32_t receivedFrameCounter = atoi(frameStr);
  Serial.print(F("receivedFrameCounter: "));
  Serial.print(receivedFrameCounter);
  Serial.print(F("; lastFrameCounter: "));
  Serial.println(device->lastFrameCounter);

  // Перевірка на повтор
  if (receivedFrameCounter <= device->lastFrameCounter) {
    Serial.println("⚠️ Replay attempt — frame too old");
    return;
  }

  if (receivedFrameCounter > device->lastFrameCounter + REPLAY_WINDOW) {
    Serial.println("⚠️ Frame too far ahead — possible sync issue");
    return;
  }

  char base[128];
  snprintf(base, sizeof(base), "%s|%s|%s", deviceIDStr, frameStr, payload);

 if (verifyHMAC(base, device->hmacKey, sizeof(device->hmacKey), receivedHMAC)) {
    device->lastFrameCounter = receivedFrameCounter;
    Serial.print(F("✅ Accepted from "));
    Serial.print(deviceIDStr);
    Serial.print(F(": "));
    Serial.print(payload);
    Serial.print(F(" ("));
    Serial.print(frameStr);
    Serial.println(F(")"));
  } 
  else {
    Serial.println(F("❌ Invalid HMAC — possibly spoofed"));
  }
}

void setupRx() {
  Serial.begin(115200);
  while (!Serial);
  delay(1000);
  
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
  // LoRa.setSpreadingFactor(9);     // Або те, що вам потрібно
  // LoRa.setSignalBandwidth(125E3); // Стандарт
  // LoRa.setCodingRate4(5);         // Покращення надійності
  // LoRa.setSyncWord(0x34); 

  String printString = "Receiver ready, waiting for SYNC...";
  Serial.println(printString);
}
void loopRx() {
  int packetSize;
  String message;

  if (!synced) {
    if (isRtcConnected) {
      setInitHopIndex();
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
          Serial.print("Got Not RTC SYNC. Index: ");
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
    char buffer[128];
    Serial.print(F("Received: "));
    Serial.println(message);
    message.toCharArray(buffer, sizeof(buffer));

    processPacket(buffer);
    Serial.println(F("Exited processPacket"));
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
    myDevice = trustedDevices[0];
    setupTx();
    Serial.print("Initialized with device: ");
    Serial.println(myDevice.deviceID);
    Serial.print("HMAC key: ");
    for (int i = 0; i < 32; i++) {
      if (myDevice.hmacKey[i] < 0x10) Serial.print("0");
      Serial.print(myDevice.hmacKey[i], HEX);
    }
    Serial.println();
  }
  else {
    myDevice = trustedDevices[1];
    setupRx();
    Serial.print("Initialized with device: ");
    Serial.println(myDevice.deviceID);
    Serial.print("HMAC key: ");
    for (int i = 0; i < 32; i++) {
      if (myDevice.hmacKey[i] < 0x10) Serial.print("0");
      Serial.print(myDevice.hmacKey[i], HEX);
    }
    Serial.println();
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
