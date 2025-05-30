// deviceinfo.h
#include <Arduino.h>
#include <Crypto.h> // if needed for key sizes

#ifndef DEVICEINFO_H
#define DEVICEINFO_H
#include <Arduino.h>

struct DeviceInfo {
  String deviceID;
  byte hmacKey[32];
  uint32_t lastFrameCounter;
};

DeviceInfo* findDevice(const String& deviceID, DeviceInfo* trustedDevices, int deviceCount);

#endif