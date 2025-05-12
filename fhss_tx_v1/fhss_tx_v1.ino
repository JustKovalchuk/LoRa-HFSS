#include <SPI.h>
#include <LoRa.h>

const long freqList[] = {868100000, 868300000, 868500000};
const int freqCount = sizeof(freqList) / sizeof(freqList[0]);
const long SYNC_FREQ = 868200000;

const uint32_t seed = 123456;

unsigned long hopInterval = 1000;
unsigned long lastHopTime = 0;
int hopIndex = -1;

unsigned long syncDelay = 50;

bool IsNewHop(){
  return millis() - lastHopTime > hopInterval;
}

void UpdateLastHopTime() {
  unsigned long timeSinceSync = millis();
  int hopsSinceSync = timeSinceSync/hopInterval;
  int currentFreqIndex = hopsSinceSync / freqCount;
  lastHopTime = hopsSinceSync*hopInterval;
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

long getCurrentFreq(int hopIndex, bool isRandom=true) {
  int freqIdx = hopIndex;
  if (isRandom) {
    freqIdx = pseudoRandom(hopIndex, seed);
  }

  return freqList[freqIdx];
}

// Обираємо нову частоту для хопу
long nextHop(bool isRandom=true) {
  hopIndex = (hopIndex + 1) % freqCount;
  long currentFreq = getCurrentFreq(hopIndex, isRandom);
  LoRa.setFrequency(currentFreq);

  UpdateLastHopTime();

  return currentFreq;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  LoRa.setPins(10, 9, 2); // NSS, RESET, DIO0
  if (!LoRa.begin(SYNC_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Transmitter ready");
}

void loop() {
  if (IsNewHop()) {
    long currentFreq = nextHop();

    SendSyncPacket(currentFreq);
    delay(syncDelay);
    
    LoRa.beginPacket();
    LoRa.print("MSG: Hello on ");
    LoRa.print(currentFreq);
    LoRa.endPacket();

    Serial.print("Sent data on ");
    Serial.println(currentFreq);

    Serial.println("-----------------------------");
  }

  delay(50);
}

