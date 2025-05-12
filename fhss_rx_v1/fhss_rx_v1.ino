#include <SPI.h>
#include <LoRa.h>

const long freqList[] = {868100000, 868300000, 868500000};
const int freqCount = sizeof(freqList) / sizeof(freqList[0]);
const long SYNC_FREQ = 868200000;

const uint32_t seed = 123456;

unsigned long hopInterval = 1000;
unsigned long lastHopTime = 0;
int hopIndex = 0;

int syncHopIndex = 0;
unsigned long syncTime = 0;
bool synced = false;

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

void setup() {
  Serial.begin(115200);
  while (!Serial);

  LoRa.setPins(10, 9, 2); // NSS, RESET, DIO0
  if (!LoRa.begin(SYNC_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Receiver ready, waiting for SYNC...");
}

void loop() {
  if (!synced) {
    LoRa.setFrequency(SYNC_FREQ);
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String message = "";
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
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }
    Serial.print("Received: ");
    Serial.println(message);
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
