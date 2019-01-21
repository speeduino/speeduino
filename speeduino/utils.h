/*
These are some utility functions and variables used through the main code
*/
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

uint16_t freeRam ();
void setResetControlPinState();
byte pinTranslate(byte);


#endif // UTILS_H
