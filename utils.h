/*
These are some utility functions and variables used through the main code
*/ 
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#define MS_IN_MINUTE 60000
#define US_IN_MINUTE 60000000

/*
 * Simple low pass IIR filter macro for the analog inputs
 * This is effectively implementing the smooth filter from http://playground.arduino.cc/Main/Smooth
 * But removes the use of floats and uses 8 bits of fixed precision. 
 */
#define ADC_FILTER(input, alpha, prior) (((long)input * (256 - alpha) + ((long)prior * alpha))) >> 8

int freeRam ();
void setPinMapping(byte boardID);
unsigned int PW();
unsigned int PW_SD();
unsigned int PW_AN();

#endif // UTILS_H
