#include <Crypto.h>
#include <ChaCha.h>
#include <string.h>

ChaCha chacha;

const uint8_t key[32] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

const uint8_t nonce[12] = {
  0xAA, 0xBB, 0xCC, 0xDD,
  0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x07, 0x08
};

char plaintext[] = "SecureLoRaChaCha20";
char buffer[64]; // для шифрованого та дешифрованого тексту

void printHex(const char* label, uint8_t* data, size_t len) {
  Serial.print(label);
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 16) Serial.print("0");
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

extern int __heap_start, *__brkval;
int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.print("Plaintext: ");
  Serial.println(plaintext);

  size_t len = strlen(plaintext);

  // Копіюємо plaintext у buffer
  memcpy(buffer, plaintext, len);

  // Шифрування
  chacha.clear();
  chacha.setKey(key, sizeof(key));
  chacha.setIV(nonce, sizeof(nonce));
  unsigned long start = micros();
  chacha.encrypt(buffer, buffer, len);  // in-place
  unsigned long end = micros();
  Serial.print("Time to encrypt: ");
  Serial.println(end - start);

  printHex("Encrypted: ", (uint8_t*)buffer, len);

  // Дешифрування
  chacha.clear();
  chacha.setKey(key, sizeof(key));
  chacha.setIV(nonce, sizeof(nonce));
  chacha.encrypt(buffer, buffer, len); // in-place decrypt (same as encrypt)

  Serial.print("Decrypted: ");
  buffer[len] = '\0';
  Serial.println(buffer);

  Serial.print("Free memory: ");
  Serial.println(freeMemory());
}

void loop() {
  // порожньо
}
