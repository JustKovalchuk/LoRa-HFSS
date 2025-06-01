// processor.h
#include <Arduino.h>

#ifndef PROCESSOR_H
#define PROCESSOR_H

const byte* getPlaintext();
uint32_t toUint32(byte* data);

#endif