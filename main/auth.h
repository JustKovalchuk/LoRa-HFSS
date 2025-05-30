// auth.h
#include <Arduino.h>

#ifndef AUTH_H
#define AUTH_H

String getHMAC(String message, const byte* hmacKey, size_t keyLength);
String getSecureMessage(String payload, char* deviceID, int32_t frameCounter, const byte* hmacKey, size_t keyLength);

bool verifyHMAC(String base, const byte* hmacKey, size_t keyLength, String receivedHMAC);

#endif