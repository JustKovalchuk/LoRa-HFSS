
#include "deviceinfo.h"

DeviceInfo* findDevice(const char* deviceID, DeviceInfo* trustedDevices, int deviceCount) {
  for (int i = 0; i < deviceCount; i++) {
    if (strcmp(trustedDevices[i].deviceID, deviceID) == 0) {
      return &trustedDevices[i];
    }
  }
  return nullptr;
}
