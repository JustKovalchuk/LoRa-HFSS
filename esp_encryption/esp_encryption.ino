#include <Arduino.h>
#include "mbedtls/aes.h"
#include "mbedtls/platform.h"
#include "string.h"

// 256-бітний ключ (32 байт)
const uint8_t AES_KEY[32] = {
  0x60, 0x3d, 0xeb, 0x10,
  0x15, 0xca, 0x71, 0xbe,
  0x2b, 0x73, 0xae, 0xf0,
  0x85, 0x7d, 0x77, 0x81,
  0x1f, 0x35, 0x2c, 0x07,
  0x3b, 0x61, 0x08, 0xd7,
  0x2d, 0x98, 0x10, 0xa3,
  0x09, 0x14, 0xdf, 0xf4
};

// IV (ініціалізаційний вектор) — повинен бути унікальним!
uint8_t AES_IV[16] = {
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b,
  0x0c, 0x0d, 0x0e, 0x0f
};

// Паддінг PKCS#7
void padBuffer(uint8_t* buffer, size_t data_len, size_t total_len) {
  uint8_t pad_value = total_len - data_len;
  for (size_t i = data_len; i < total_len; i++) {
    buffer[i] = pad_value;
  }
}

// Видалення паддінгу
size_t unpadBuffer(uint8_t* buffer, size_t total_len) {
  uint8_t pad_value = buffer[total_len - 1];
  if (pad_value > 16) return total_len;
  return total_len - pad_value;
}

void aesEncryptCBC(const uint8_t* input, size_t input_len, uint8_t* output) {
  mbedtls_aes_context aes;
  uint8_t iv_copy[16];
  memcpy(iv_copy, AES_IV, 16); // IV буде змінений, тому копіюємо

  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, AES_KEY, 128);
  unsigned long start = micros();
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, input_len, iv_copy, input, output);
  unsigned long end = micros();
  Serial.print("Time to encrypt: ");
  Serial.println(end - start);
  mbedtls_aes_free(&aes);
}

void aesDecryptCBC(const uint8_t* input, size_t input_len, uint8_t* output) {
  mbedtls_aes_context aes;
  uint8_t iv_copy[16];
  memcpy(iv_copy, AES_IV, 16); // IV повинен бути тим самим!

  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, AES_KEY, 128);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, input_len, iv_copy, input, output);
  mbedtls_aes_free(&aes);
}

void printHex(const char* label, const uint8_t* data, size_t len) {
  Serial.print(label);
  for (size_t i = 0; i < len; i++) {
    Serial.printf("%02X ", data[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Дані для шифрування (менше 16 байт, буде доповнено паддінгом)
  const char* plaintext = "HelloSecureLoRa";
  size_t len = strlen(plaintext);
  size_t padded_len = ((len / 16) + 1) * 16;

  uint8_t padded[32];
  memset(padded, 0, 32);
  memcpy(padded, plaintext, len);
  padBuffer(padded, len, padded_len);

  uint8_t encrypted[32];
  uint8_t decrypted[32];

  aesEncryptCBC(padded, padded_len, encrypted);

  printHex("Encrypted: ", encrypted, padded_len);

  aesDecryptCBC(encrypted, padded_len, decrypted);
  size_t decrypted_len = unpadBuffer(decrypted, padded_len);

  Serial.print("Decrypted: ");
  for (size_t i = 0; i < decrypted_len; i++) {
    Serial.print((char)decrypted[i]);
  }
  Serial.println();
}

void loop() {
}
