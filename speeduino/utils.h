/*
These are some utility functions and variables used through the main code
*/ 
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

byte launchDefault;
byte ignBypassPinDefault;
byte tachoDefault;
byte fuelPumpPinDefault;
byte fanPinDefault;
byte boostPinDefault;
byte vvtPinDefault;

int freeRam ();
void setPinMapping(byte boardID);
void setmcuMapping(byte mcuID);
unsigned int PW();
unsigned int PW_SD();
unsigned int PW_AN();

#endif // UTILS_H
