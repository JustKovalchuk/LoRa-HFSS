#include <SPI.h>
#include <LoRa.h>

const long freqList[] = {868100000, 868300000, 868500000};
const int freqCount = sizeof(freqList) / sizeof(freqList[0]);
const long SYNC_FREQ = 868200000;

unsigned long hopInterval = 10000;
unsigned long lastHopTime = 0;
int hopIndex = -1;

int isSyncronized = 0;

void SendSyncPacket() {
  LoRa.setFrequency(SYNC_FREQ);
  LoRa.beginPacket();
  LoRa.print("SYNC:");
  LoRa.print(hopIndex);
  LoRa.endPacket();
  Serial.print("Sent SYNC with index ");
  Serial.println(hopIndex);

  long currentFreq = freqList[hopIndex];
  LoRa.setFrequency(freqList[hopIndex]);
}

// Обираємо нову частоту для хопу
long UpdateHopFreq() {
  hopIndex = (hopIndex + 1) % freqCount;
  long currentFreq = freqList[hopIndex];
  LoRa.setFrequency(currentFreq);
  lastHopTime = millis();
  return currentFreq;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  LoRa.setPins(10, 9, 2); // NSS, RESET, DIO0
  if (!LoRa.begin(freqList[hopIndex])) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Transmitter ready");
}

void loop() {
  long currentFreq = UpdateHopFreq();

  SendSyncPacket();
  delay(200);

  LoRa.beginPacket();
  LoRa.print("MSG: Hello on ");
  LoRa.print(freqList[hopIndex]);
  LoRa.endPacket();

  Serial.print("Sent data on ");
  Serial.println(currentFreq);

  Serial.println("-----------------------------");
  delay(hopInterval);
}

