#include <Crypto.h>
#include <SHA256.h>
#include "auth.h"

SHA256 sha256;

String getHMAC(String message, byte hmacKey[]) {
  sha256.resetHMAC(hmacKey, sizeof(hmacKey));
  sha256.update((const byte*)message.c_str(), message.length());

  byte digest[32];
  sha256.finalizeHMAC(hmacKey, sizeof(hmacKey), digest, sizeof(digest));

  // Повертаємо перші 4 байти HMAC як HEX рядок
  String hmacStr = "";
  for (int i = 0; i < 4; i++) {
    if (digest[i] < 16) hmacStr += "0";
    hmacStr += String(digest[i], HEX);
  }
  return hmacStr;
}

String getSecureMessage(String payload, char* deviceID, int32_t frameCounter, byte hmacKey[]) {
  String base = String(deviceID) + "|" + String(frameCounter) + "|" + payload;
  String hmac = getHMAC(base, hmacKey);
  String fullPacket = base + "|" + hmac;

  return fullPacket;
}

bool verifyHMAC(String base, byte hmacKey[], String receivedHMAC) {
  sha256.resetHMAC(hmacKey, sizeof(hmacKey));
  sha256.update((const byte*)base.c_str(), base.length());

  byte digest[32];
  sha256.finalizeHMAC(hmacKey, sizeof(hmacKey), digest, sizeof(digest));

  String expectedHMAC = "";
  for (int i = 0; i < 4; i++) {
    if (digest[i] < 16) expectedHMAC += "0";
    expectedHMAC += String(digest[i], HEX);
  }

  return (expectedHMAC == receivedHMAC);
}