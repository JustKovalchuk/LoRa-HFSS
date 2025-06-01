
#include "processor.h"

#include <Crypto.h>
#include <AES.h>
#include <CTR.h>

AES128 aes128;
CTR<AES128> ctr;

void printByteArray(const char* label, const byte* data, int length) {
  Serial.print(label);
  for (int i = 0; i < length; i++) {
    if (data[i] < 0x10) Serial.print("0");
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

const byte* getPlaintext() {
  static byte plaintext[] = {
    97, 74,                  // SpO2, Pulse
    0x79, 0xB2, 0x28, 0x12,  // Longitude
    0xF9, 0xA1, 0x01, 0x1E,  // Latitude
    0x45, 0xC3, 0x53, 0x66   // Timestamp
  };
  return plaintext;
}

uint32_t toUint32(byte* data) {
  return ((uint32_t)data[0]) |
         ((uint32_t)data[1] << 8) |
         ((uint32_t)data[2] << 16) |
         ((uint32_t)data[3] << 24);
}

// void encript(byte* key, byte* nonce, byte* plaintext, int dataLength) {
//     byte ciphertext[64];
//     ctr.setKey(key, sizeof(key));
//     ctr.setIV(nonce, sizeof(nonce));
//     ctr.encrypt(ciphertext, plaintext, dataLength);
// }

// void decript(byte* key, byte* nonce, byte* ciphertext, int dataLength) {
//     byte decrypted[64];
//     ctr.setKey(key, sizeof(key));
//     ctr.setIV(nonce, sizeof(nonce));
//     ctr.decrypt(decrypted, ciphertext, dataLength);
// }