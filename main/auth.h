// auth.h
#include <Arduino.h>

#ifndef AUTH_H
#define AUTH_H

String getHMAC(String message, byte hmacKey[]);
String getSecureMessage(String payload, char* deviceID, int32_t frameCounter, byte hmacKey[]);

bool verifyHMAC(String base, byte hmacKey[], String receivedHMAC);

#endif