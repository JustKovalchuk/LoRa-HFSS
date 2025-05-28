#include <AESLib.h>

AESLib aesLib;

// 256-бітний ключ (32 байти)
byte aes_key[32] = {
  0xa9, 0x22, 0x56, 0x65, 0x79, 0x71, 0x2d, 0x0b,
  0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
  0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
  0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
};

byte aes_iv[16] = {
  0x79, 0x4E, 0x98, 0x21, 0xAE, 0xD8, 0xA6, 0xAA,
  0xD7, 0x97, 0x44, 0x14, 0xAB, 0xDD, 0x9F, 0x2C
};

byte iv_enc[16];
byte iv_dec[16];

char buffer[64];
char ciphertext[64];
char decryptedtext[64];

int padLength(int len) {
  int pad = 16 - (len % 16);
  return pad;
}

void padBuffer(char* buf, int len) {
  int pad = padLength(len);
  for (int i = len; i < len + pad; i++) {
    buf[i] = (char)pad;
  }
  buf[len + pad] = '\0';
}

int unpadBuffer(char* buf, int len) {
  int pad = (int)buf[len - 1];
  if (pad < 1 || pad > 16) return len;
  return len - pad;
}

extern int __heap_start, *__brkval;
int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  char plaintext[] = "HelloSecureLoRa|data";
  int len = strlen(plaintext);
  int pad = padLength(len);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, plaintext, len);
  padBuffer(buffer, len);

  Serial.print("Original: ");
  Serial.println(plaintext);

  // === ШИФРУВАННЯ ===
  memcpy(iv_enc, aes_iv, 16);
  unsigned long start_enc = micros();
  uint16_t ciphertextLength = aesLib.encrypt64(buffer, len + pad, ciphertext, aes_key, sizeof(aes_key), iv_enc);
  unsigned long end_enc = micros();

  Serial.print("Time to encrypt (us): ");
  Serial.println(end_enc - start_enc);
  Serial.print("Encrypted (base64): ");
  Serial.println(ciphertext);

  // === ДЕШИФРУВАННЯ ===
  memcpy(iv_dec, aes_iv, 16);
  unsigned long start_dec = micros();
  uint16_t decryptedLength = aesLib.decrypt64(ciphertext, ciphertextLength, decryptedtext, aes_key, sizeof(aes_key), iv_dec);
  unsigned long end_dec = micros();

  Serial.print("Time to decrypt (us): ");
  Serial.println(end_dec - start_dec);

  int unpaddedLength = unpadBuffer(decryptedtext, decryptedLength);
  decryptedtext[unpaddedLength] = '\0';

  Serial.print("Decrypted text: ");
  Serial.println(decryptedtext);

  // === RAM ===
  Serial.print("Free memory (SRAM): ");
  Serial.println(freeMemory());
}

void loop() {}
