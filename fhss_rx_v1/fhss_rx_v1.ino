#include <SPI.h>
#include <LoRa.h>

const long freqList[] = {868100000, 868300000, 868500000};
const int freqCount = sizeof(freqList) / sizeof(freqList[0]);
const long SYNC_FREQ = 868200000;

const uint32_t seed = 123456;

unsigned long hopInterval = 10000;
unsigned long lastHopTime = 0;
int hopIndex = 0;

bool synced = false;

// Обираємо нову частоту для хопу
long UpdateHopFreq() {
  hopIndex = (hopIndex + 1) % freqCount;
  long currentFreq = freqList[hopIndex];
  LoRa.setFrequency(currentFreq);
  lastHopTime = millis();
  return currentFreq;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  LoRa.setPins(10, 9, 2); // NSS, RESET, DIO0
  if (!LoRa.begin(freqList[hopIndex])) {
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
        hopIndex = message.substring(5).toInt();
        Serial.print("Got SYNC. Index: ");
        Serial.println(hopIndex);
        synced = true;
        LoRa.setFrequency(freqList[hopIndex]);
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
  if (millis() - lastHopTime > hopInterval) {
    Serial.println("-----------------------------");
    long currentFreq = UpdateHopFreq();
    Serial.print("Hopped to: ");
    Serial.println(currentFreq);
  }
}
