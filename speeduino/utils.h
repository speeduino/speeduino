/*
These are some utility functions and variables used through the main code
*/
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

void setResetControlPinState();
byte pinTranslate(byte);
uint32_t calculateCRC32(byte);

#endif // UTILS_H
