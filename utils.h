/*
These are some utility functions and variables used through the main code
*/ 
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#define MS_IN_MINUTE 60000
#define US_IN_MINUTE 60000000

int freeRam ();
void setPinMapping(byte boardID);
unsigned int PW();
unsigned int PW_SD();
unsigned int PW_AN();

#endif // UTILS_H
