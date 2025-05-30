// Об'єднаний приклад ChaCha20 та AES-256 CTR
// Використовує Crypto бібліотеку

#include <Crypto.h>
#include <ChaCha.h>
#include <AES.h>
#include <CTR.h>

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

ChaCha chacha;
AES256 aes256;
CTR<AES256> ctr;
AES128 aes128;
CTR<AES128> ctr128;

int8_t numRounds = 20;

const uint8_t key[32] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};
const uint8_t key128[16] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
};

const uint8_t iv[12] = {
  0xAA, 0xBB, 0xCC, 0xDD,
  0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x07, 0x08
};

byte plaintext[] = {
  97, 74,
  0x79, 0xB2, 0x28, 0x12,
  0xF9, 0xA1, 0x01, 0x1E,
  0x45, 0xC3, 0x53, 0x66
};
const int dataLength = sizeof(plaintext);

char buffer[256];
byte ciphertext[64];
byte decrypted[64];

void setup() {
  Serial.begin(9600);
  delay(1000);

  // ==== ChaCha20 ====
  Serial.println("ChaCha20 Encryption:");
  printByteArray("Plaintext:   ", plaintext, dataLength);

  memcpy(buffer, plaintext, dataLength);
  chacha.clear();
  chacha.setNumRounds(numRounds);
  chacha.setKey(key, sizeof(key));
  chacha.setIV(iv, sizeof(iv));
  chacha.encrypt(buffer, buffer, dataLength);
  printByteArray("Ciphertext:  ", (uint8_t*)buffer, dataLength);

  chacha.clear();
  chacha.setNumRounds(numRounds);
  chacha.setKey(key, sizeof(key));
  chacha.setIV(iv, sizeof(iv));
  chacha.decrypt(buffer, buffer, dataLength);
  printByteArray("Decrypted:   ", (uint8_t*)buffer, dataLength);

  uint8_t spO2   = buffer[0];
  uint8_t pulse  = buffer[1];
  uint32_t longitude = toUint32((byte*)&buffer[2]);
  uint32_t latitude  = toUint32((byte*)&buffer[6]);
  uint32_t timestamp = toUint32((byte*)&buffer[10]);

  Serial.println("Parsed ChaCha20:");
  Serial.print("SpO₂: "); Serial.println(spO2);
  Serial.print("Pulse: "); Serial.println(pulse);
  Serial.print("Longitude: "); Serial.println((float)longitude / 1e7, 7);
  Serial.print("Latitude: ");  Serial.println((float)latitude / 1e7, 7);
  Serial.print("Unix time: "); Serial.println(timestamp);
  Serial.println();

  // ==== AES-256 CTR ====
  Serial.println("AES-256 CTR Encryption:");
  printByteArray("Plaintext:   ", plaintext, dataLength);

  ctr.setKey(key, sizeof(key));
  ctr.setIV(iv, sizeof(iv));
  ctr.encrypt(ciphertext, plaintext, dataLength);
  printByteArray("Ciphertext:  ", ciphertext, dataLength);

  ctr.setKey(key, sizeof(key));
  ctr.setIV(iv, sizeof(iv));
  ctr.decrypt(decrypted, ciphertext, dataLength);
  printByteArray("Decrypted:   ", decrypted, dataLength);

  spO2   = decrypted[0];
  pulse  = decrypted[1];
  longitude = toUint32(&decrypted[2]);
  latitude  = toUint32(&decrypted[6]);
  timestamp = toUint32(&decrypted[10]);

  Serial.println("Parsed AES-256 CTR:");
  Serial.print("SpO₂: "); Serial.println(spO2);
  Serial.print("Pulse: "); Serial.println(pulse);
  Serial.print("Longitude: "); Serial.println((float)longitude / 1e7, 7);
  Serial.print("Latitude: ");  Serial.println((float)latitude / 1e7, 7);
  Serial.print("Unix time: "); Serial.println(timestamp);
  Serial.println();

  // ==== AES-128 CTR ====
  Serial.println("AES-128 CTR Encryption:");
  printByteArray("Plaintext:   ", plaintext, dataLength);

  ctr128.setKey(key128, sizeof(key128));
  ctr128.setIV(iv, sizeof(iv));
  ctr128.encrypt(ciphertext, plaintext, dataLength);
  printByteArray("Ciphertext:  ", ciphertext, dataLength);

  ctr128.setKey(key128, sizeof(key128));
  ctr128.setIV(iv, sizeof(iv));
  ctr128.decrypt(decrypted, ciphertext, dataLength);
  printByteArray("Decrypted:   ", decrypted, dataLength);

  spO2   = decrypted[0];
  pulse  = decrypted[1];
  longitude = toUint32(&decrypted[2]);
  latitude  = toUint32(&decrypted[6]);
  timestamp = toUint32(&decrypted[10]);

  Serial.println("Parsed AES-128 CTR:");
  Serial.print("SpO₂: "); Serial.println(spO2);
  Serial.print("Pulse: "); Serial.println(pulse);
  Serial.print("Longitude: "); Serial.println((float)longitude / 1e7, 7);
  Serial.print("Latitude: ");  Serial.println((float)latitude / 1e7, 7);
  Serial.print("Unix time: "); Serial.println(timestamp);
  Serial.println();
}

void loop() {}