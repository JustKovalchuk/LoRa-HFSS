
#include "deviceinfo.h"

DeviceInfo* findDevice(const String& deviceID, DeviceInfo* trustedDevices, int deviceCount) {
  for (int i = 0; i < deviceCount; i++) {
    if (trustedDevices[i].deviceID == deviceID) {
      return &trustedDevices[i];
    }
  }
  return nullptr;
}
