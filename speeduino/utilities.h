/*
These are some utility functions and variables used through the main code
*/
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

#define CONCATS(s1, s2) (s1" " s2) //needed for some reason. not defined correctly because of utils.h file of speeduino (same name as one in arduino core)

#define COMPARATOR_EQUAL 0
#define COMPARATOR_NOT_EQUAL 1
#define COMPARATOR_GREATER 2
#define COMPARATOR_GREATER_EQUAL 3
#define COMPARATOR_LESS 4
#define COMPARATOR_LESS_EQUAL 5
#define COMPARATOR_CHANGE 6

#define BITWISE_DISABLED 0
#define BITWISE_AND 1
#define BITWISE_OR 2
#define BITWISE_XOR 3

uint16_t ioDelay[sizeof(configPage13.outputPin)];
uint8_t pinIsValid = 0;
//uint8_t outputPin[sizeof(configPage13.outputPin)];

void setResetControlPinState();
byte pinTranslate(byte);
uint32_t calculateCRC32(byte);
void initialiseProgrammableIO();
void checkProgrammableIO();
int16_t ProgrammableIOGetData(uint16_t index);

#endif // UTILS_H
