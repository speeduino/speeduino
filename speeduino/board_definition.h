#pragma once

/** @file
 * @brief Inclusion of board specific header files and board related definitions.
 * 
 * @note This file should be named "board.h", but one of the STM32 Arduino implementations
 * has a <board.h> include. Which picks up *this file* instead of the intended file :-( 
*/
#include <Arduino.h>

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
  #define BOARD_MAX_DIGITAL_PINS 54 //digital pins +1
  #define BOARD_MAX_IO_PINS 70 //digital pins + analog channels + 1
  #define BOARD_MAX_ADC_PINS  15 //Number of analog pins
  #ifndef LED_BUILTIN
    #define LED_BUILTIN 13
  #endif
  #define CORE_AVR
  #ifndef INJ_CHANNELS
    #define INJ_CHANNELS 4
  #endif
  #ifndef IGN_CHANNELS
    #define IGN_CHANNELS 5
  #endif
  #include "board_avr2560.h"
#elif defined(CORE_TEENSY)
  #define INJ_CHANNELS 8
  #define IGN_CHANNELS 8
  #if defined(__MK64FX512__) || defined(__MK66FX1M0__)
    #define CORE_TEENSY35
    #include "board_teensy35.h"
  #elif defined(__IMXRT1062__)
    #define CORE_TEENSY41
    #include "board_teensy41.h"
  #endif
#elif defined(STM32_MCU_SERIES) || defined(ARDUINO_ARCH_STM32) || defined(STM32)
  #define CORE_STM32

  #define BOARD_MAX_ADC_PINS  NUM_ANALOG_INPUTS-1 //Number of analog pins from core.
  #if defined(STM32F407xx) //F407 can do 8x8 STM32F401/STM32F411 don't
   #define INJ_CHANNELS 8
   #define IGN_CHANNELS 8
  #else
   #define INJ_CHANNELS 4
   #define IGN_CHANNELS 5
  #endif
  #include "board_stm32_official.h"
#elif defined(__SAMD21G18A__)
  #define CORE_SAMD21
  #define CORE_SAM
  #define INJ_CHANNELS 4
  #define IGN_CHANNELS 4
  #include "board_samd21.h"
#elif defined(__SAME51J19A__)
  #define CORE_SAME51
  #define CORE_SAM
  #define INJ_CHANNELS 8
  #define IGN_CHANNELS 8
  #include "board_same51.h"
// Allow external injection of the board definition via compiler flags
#elif defined(EXTERNAL_BOARD_H)
  #include EXTERNAL_BOARD_H
#else
  #error Incorrect board selected. Please select the correct board (Usually Mega 2560) and upload again
#endif
