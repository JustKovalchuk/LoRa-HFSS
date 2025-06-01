// deviceinfo.h
#include <Arduino.h>
#include <Crypto.h> // if needed for key sizes

#ifndef DEVICEINFO_H
#define DEVICEINFO_H
#include <Arduino.h>

struct DeviceInfo {
  char* deviceID;
  byte hmacKey[32];
  uint32_t lastFrameCounter;
};

DeviceInfo* findDevice(const char* deviceID, DeviceInfo* trustedDevices, int deviceCount);

#endif