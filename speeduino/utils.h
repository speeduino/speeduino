/*
These are some utility functions and variables used through the main code
*/
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

#define CONCATS(s1, s2) (s1" " s2) //needed for some reason. not defined correctly because of utils.h file of speeduino (same name as one in arduino core)
void setResetControlPinState();
byte pinTranslate(byte);
uint32_t calculateCRC32(byte);

#endif // UTILS_H
