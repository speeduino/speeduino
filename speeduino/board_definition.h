#pragma once

/** @file
 * @brief Inclusion of board specific header files and board related definitions.
 * 
 * @note This file should be named "board.h", but one of the STM32 Arduino implementations
 * has a <board.h> include. Which picks up *this file* instead of the intended file :-( 
*/

#include <stdint.h>
#include <Arduino.h>

/**
 * @brief Initialise the board, including USB comms
 * 
 * This is called after the tune is loaded from EEPROM, but before pins are assigned.
 * 
 * @param baudRate The Serial comms baud rate
 */
void initBoard(uint32_t baudRate);

/**
 * @brief Pin specific initialisation (optional - can be empty)
 * 
 * This is called *after* the pins are assigned and therefore after initBoard()
 */
void boardInitPins(void);

/** @brief Calculate free RAM for display in TunerStudio */
uint16_t freeRam(void);

/** @brief Reset the board (optional) */
void doSystemReset(void);

/** @brief Trigger the boot loader (optional) */
void jumpToBootloader(void);

/** @brief Get the board temp for display in TunerStudio (optional) */
uint8_t getSystemTemp(void);

// Include a specific header for a board.
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
  #include "board_avr2560.h"
#elif defined(CORE_TEENSY)
  #if defined(__MK64FX512__) || defined(__MK66FX1M0__)
    #include "board_teensy35.h"
  #elif defined(__IMXRT1062__)
    #include "board_teensy41.h"
  #endif
#elif defined(STM32_MCU_SERIES) || defined(ARDUINO_ARCH_STM32) || defined(STM32)
  #include "board_stm32_official.h"
#elif defined(__SAME51J19A__)
  #include "board_same51.h"
// Allow external injection of the board definition via compiler flags
#elif defined(EXTERNAL_BOARD_H)
  #include EXTERNAL_BOARD_H
#else
  #error Incorrect board selected. Please select the correct board (Usually Mega 2560) and upload again
#endif

#if defined(RTC_ENABLED)
/** @brief Board specific RTC system initialisation (optional) */
void boardInitRTC(void);
#endif

// It is important that we cast this to the actual overflow limit of the timer. 
// The compare variables type can be wider than the timer overflow.
#define SET_COMPARE(compare, value) (compare) = (COMPARE_TYPE)(value)
