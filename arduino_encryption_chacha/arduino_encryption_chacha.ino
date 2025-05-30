#include <Crypto.h>
#include <ChaCha.h>
#include <string.h>

ChaCha chacha;

int8_t numRounds = 20;
const uint8_t key[32] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

const uint8_t iv[12] = {
  0xAA, 0xBB, 0xCC, 0xDD,
  0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x07, 0x08
};

byte plaintext[] = {
  97, 74,                  // SpO2, Pulse
  0x79, 0xB2, 0x28, 0x12,  // Longitude
  0xF9, 0xA1, 0x01, 0x1E,  // Latitude
  0x45, 0xC3, 0x53, 0x66   // Timestamp
};
const int dataLength = sizeof(plaintext);  

// char plaintext[] = "SecureLoRaChaCha20";
// const int dataLength = strlen(plaintext); 

char buffer[256];

void printByteArray(const char* label, const byte* data, int length) {
  Serial.print(label);
  for (int i = 0; i < length; i++) {
    if (data[i] < 0x10) Serial.print("0");
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

uint32_t toUint32(byte* data) {
  return ((uint32_t)data[0]) |
         ((uint32_t)data[1] << 8) |
         ((uint32_t)data[2] << 16) |
         ((uint32_t)data[3] << 24);
}

void printHex(const char* label, uint8_t* data, size_t len) {
  Serial.print(label);
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 16) Serial.print("0");
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("ChaCha20 Example");

  printByteArray("Plaintext bytes:   ", plaintext, dataLength);

  size_t len = strlen(plaintext);

  memcpy(buffer, plaintext, dataLength);

  // Шифрування
  chacha.clear();
  chacha.setNumRounds(numRounds);
  chacha.setKey(key, sizeof(key));
  chacha.setIV(iv, sizeof(iv));
  chacha.encrypt(buffer, buffer, dataLength);
  printByteArray("Ciphertext bytes:  ", (uint8_t*)buffer, dataLength);

  // Дешифрування
  chacha.clear();
  chacha.setNumRounds(numRounds);
  chacha.setKey(key, sizeof(key));
  chacha.setIV(iv, sizeof(iv));
  chacha.decrypt(buffer, buffer, dataLength);

  printByteArray("Decrypted bytes:   ", (uint8_t*)buffer, dataLength);
  
  uint8_t spO2   = buffer[0];
  uint8_t pulse  = buffer[1];

  uint32_t longitude = toUint32((byte*)&buffer[2]);
  uint32_t latitude  = toUint32((byte*)&buffer[6]);
  uint32_t timestamp = toUint32((byte*)&buffer[10]);

  Serial.println();
  Serial.print("HEX to String data: ");

  Serial.print("SpO2: "); Serial.println(spO2);
  Serial.print("Pulse: "); Serial.println(pulse);
  Serial.print("Longitude (raw): "); Serial.println(longitude);
  Serial.print("Latitude (raw): "); Serial.println(latitude);
  Serial.print("Unix time: "); Serial.println(timestamp);

  Serial.print("Longitude (°): "); Serial.println((float)longitude / 1e7, 7);
  Serial.print("Latitude (°): ");  Serial.println((float)latitude / 1e7, 7);
  Serial.println();
}

void loop() {
}
