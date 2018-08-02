/*
These are some utility functions and variables used through the main code
*/
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

uint16_t freeRam ();
void setPinMapping(byte boardID);
void initialiseTriggers();

//This is dumb, but it'll do for now to get things compiling
#if defined(CORE_STM32)
  //STM32F1/variants/.../board.cpp
  #if defined (STM32F4)
    #define A0  PA0
    #define A1  PA1
    #define A2  PA2
    #define A3  PA3
    #define A4  PA4
    #define A5  PA5
    #define A6  PA6
    #define A7  PA7
    #define A8  PB0
    #define A9  PB1
    #define A10  PC0
    #define A11  PC1
    #define A12  PC2
    #define A13  PC3
    #define A14  PC4
    #define A15  PC5
  #else
    #define A0  PB0
    #define A1  PA7
    #define A2  PA6
    #define A3  PA5
    #define A4  PA4
    #define A5  PA3
    #define A6  PA2
    #define A7  PA1
    #define A8  PA0
    //STM32F1 have only 9 12bit adc
    #define A9  PB0
    #define A10  PA7
    #define A11  PA6
    #define A12  PA5
    #define A13  PA4
    #define A14  PA3
    #define A15  PA2
  #endif
#endif

#endif // UTILS_H
