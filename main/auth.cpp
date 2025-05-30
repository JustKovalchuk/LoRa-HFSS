#include <Crypto.h>
#include <SHA256.h>
#include "auth.h"

SHA256 sha256;

String getHMAC(String message, const byte* hmacKey, size_t keyLength) {
  sha256.resetHMAC(hmacKey, keyLength);
  sha256.update((const byte*)message.c_str(), message.length());

  byte digest[32];
  sha256.finalizeHMAC(hmacKey, keyLength, digest, sizeof(digest));

  // Повертаємо перші 4 байти HMAC як HEX рядок
  String hmacStr = "";
  for (int i = 0; i < 4; i++) {
    if (digest[i] < 16) hmacStr += "0";
    hmacStr += String(digest[i], HEX);
  }
  return hmacStr;
}

String getSecureMessage(String payload, char* deviceID, int32_t frameCounter, const byte* hmacKey, size_t keyLength) {
  String base = String(deviceID) + "|" + String(frameCounter) + "|" + payload;
  String hmac = getHMAC(base, hmacKey, keyLength);
  String fullPacket = base + "|" + hmac;

  return fullPacket;
}

bool verifyHMAC(String base, const byte* hmacKey, size_t keyLength, String receivedHMAC) {
  sha256.resetHMAC(hmacKey, keyLength);
  sha256.update((const byte*)base.c_str(), base.length());

  byte digest[32];
  sha256.finalizeHMAC(hmacKey, keyLength, digest, sizeof(digest));

  String expectedHMAC = "";
  // Повертаємо перші 4 байти HMAC як HEX рядок
  for (int i = 0; i < 4; i++) {
    if (digest[i] < 16) expectedHMAC += "0";
    expectedHMAC += String(digest[i], HEX);
  }

  return (expectedHMAC == receivedHMAC);
}