/*
These are some utility functions and variables used through the main code
*/ 
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

int16_t freeRam ();
void setPinMapping(byte boardID);
uint16_t PW();
uint16_t PW_SD();
uint16_t PW_AN();

#endif // UTILS_H
