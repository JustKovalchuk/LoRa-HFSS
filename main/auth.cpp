#include <Crypto.h>
#include <SHA256.h>
#include "auth.h"

SHA256 sha256;

void getHMAC(const char* message, const byte* hmacKey, size_t keyLength, char* hmacOut) {
  sha256.resetHMAC(hmacKey, keyLength);
  sha256.update((const byte*)message, strlen(message));

  byte digest[32];
  sha256.finalizeHMAC(hmacKey, keyLength, digest, sizeof(digest));

  // Повертаємо перші 4 байти HMAC як HEX рядок
  for (int i = 0; i < 4; i++) {
    sprintf(&hmacOut[i * 2], "%02x", digest[i]);
  }
  hmacOut[8] = '\0';
}
void getSecureMessage(const char* payload, char* deviceID, int32_t frameCounter, const byte* hmacKey, size_t keyLength, char* fullPacketOut, size_t maxLength) {
  char base[128];
  snprintf(base, sizeof(base), "%s|%ld|%s", deviceID, frameCounter, payload);

  char hmac[9];
  getHMAC(base, hmacKey, keyLength, hmac);

  snprintf(fullPacketOut, maxLength, "%s|%s", base, hmac);
}

bool verifyHMAC(const char* base, const byte* hmacKey, size_t keyLength, const char* receivedHMAC) {
  char expectedHMAC[9];
  getHMAC(base, hmacKey, keyLength, expectedHMAC);
  return strcmp(expectedHMAC, receivedHMAC) == 0;
}