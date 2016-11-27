/*
These are some utility functions and variables used through the main code
*/ 
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#define MS_IN_MINUTE 60000
#define US_IN_MINUTE 60000000

short freeRam ();
void setPinMapping(byte boardID);
unsigned short PW();
unsigned short PW_SD();
unsigned short PW_AN();

#endif // UTILS_H
