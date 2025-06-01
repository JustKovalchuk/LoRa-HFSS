// auth.h
#include <Arduino.h>

#ifndef AUTH_H
#define AUTH_H

void getHMAC(const char* message, const byte* hmacKey, size_t keyLength, char* hmacOut);
void getSecureMessage(const char* payload, char* deviceID, int32_t frameCounter, const byte* hmacKey, size_t keyLength, char* fullPacketOut, size_t maxLength);

bool verifyHMAC(const char* base, const byte* hmacKey, size_t keyLength, const char* receivedHMAC);

#endif