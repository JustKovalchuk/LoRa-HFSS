#include <SPI.h>
#include <LoRa.h>
#include "auth.h"
#include "init.h"
#include "deviceinfo.h"

bool isTx = true;

int syncHopIndex = 0;
unsigned long syncTime = 0;


#pragma region TX_VARS

const char* deviceID = "NODE1";
const byte hmacKey[] = { 0x11, 0x22, 0x33, 0x44 }; // 128-бітовий ключ



unsigned long syncDelay = 100;

uint32_t frameCounter = 0; // оновлюється після кожного пакету

#pragma endregion



#pragma region RX_VARS

DeviceInfo trustedDevices[] = {
  {"NODE1", {0xAA, 0xBB, 0xCC, 0xDD}, 0},
  {"NODE2", {0x11, 0x22, 0x33, 0x44}, 0},
};

const int deviceCount = sizeof(trustedDevices) / sizeof(trustedDevices[0]);



bool synced = false;

int requestPerMinute = 60000 / hopInterval;

const int REPLAY_WINDOW = 5 * requestPerMinute;

#pragma endregion



bool IsNewHop(){
  return millis() - lastHopTime > hopInterval;
}

void UpdateLastHopTime() {
  unsigned long timeSinceSync = millis()-syncTime;
  int hopsSinceSync = timeSinceSync/hopInterval;
  int currentFreqIndex = (syncHopIndex + hopsSinceSync) / freqCount;
  lastHopTime = syncTime + hopsSinceSync*hopInterval;
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

  LoRa.setPins(10, 9, 2); // NSS, RESET, DIO0
  if (!LoRa.begin(SYNC_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Transmitter ready");
}
void loopTx() {
  if (IsNewHop()) {
    long currentFreq = nextHop();

    SendSyncPacket(currentFreq);
    delay(syncDelay);
    
    String payload = "MSG: Hello on " + String(currentFreq);
    String fullPacket = getSecureMessage(payload, deviceID, frameCounter, hmacKey);
    
    LoRa.beginPacket();
    LoRa.print(fullPacket);
    LoRa.endPacket();

    Serial.println("Sent: " + fullPacket);

    frameCounter++;
    // LoRa.beginPacket();
    // LoRa.print(payload);
    // LoRa.print(currentFreq);
    // LoRa.endPacket();

    // Serial.print("Sent data on ");
    // Serial.println(currentFreq);

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

  if (verifyHMAC(base, device->hmacKey, receivedHMAC)) {
    device->lastFrameCounter = receivedFrameCounter;
    Serial.println("✅ Accepted from " + deviceIDStr + ": " + payload + "(" + frameStr + ")");
  } 
  else {
    Serial.println("❌ Invalid HMAC — possibly spoofed");
  }
}

void setupRx() {
  Serial.begin(115200);
  while (!Serial);

  LoRa.setPins(10, 9, 2); // NSS, RESET, DIO0
  if (!LoRa.begin(SYNC_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Receiver ready, waiting for SYNC...");
  Serial.println("REPLAY_WINDOW" + String(REPLAY_WINDOW));
}
void loopRx() {
  int packetSize;
  String message;
  if (!synced) {
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
