#if !defined(__BOARD_H__)
#define __BOARD_H__

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
  #define BOARD_DIGITAL_GPIO_PINS 54
  #define BOARD_NR_GPIO_PINS 62
  #define LED_BUILTIN 13
  #define CORE_AVR
  #include "board_avr2560.h"
  #define INJ_CHANNELS 4
  #define IGN_CHANNELS 5

  //#define TIMER5_MICROS

#elif defined(CORE_TEENSY)
  #if defined(__MK64FX512__) || defined(__MK66FX1M0__)
    #define CORE_TEENSY35
    #include "board_teensy35.h"
    #define SD_LOGGING //SD logging enabled by default for Teensy 3.5 as it has the slot built in
  #elif defined(__IMXRT1062__)
    #define CORE_TEENSY40
    #include "board_teensy40.h"
  #endif
  #define INJ_CHANNELS 8
  #define IGN_CHANNELS 8

#elif defined(STM32_MCU_SERIES) || defined(ARDUINO_ARCH_STM32) || defined(STM32)
  //These should be updated to 8 later, but there's bits missing currently
  #define INJ_CHANNELS 4
  #define IGN_CHANNELS 5

  #ifndef word
    #define word(h, l) ((h << 8) | l) //word() function not defined for this platform in the main library
  #endif
  
  #if defined(ARDUINO_BLUEPILL_F103C8) || defined(ARDUINO_BLUEPILL_F103CB) \
   || defined(ARDUINO_BLACKPILL_F401CC) || defined(ARDUINO_BLACKPILL_F411CE)
    //STM32 Pill boards
    #define BOARD_DIGITAL_GPIO_PINS 34
    #define BOARD_NR_GPIO_PINS 34
    #ifndef LED_BUILTIN
      #define LED_BUILTIN PB1 //Maple Mini
    #endif
  #elif defined(ARDUINO_BLACK_F407VE)
    #define BOARD_DIGITAL_GPIO_PINS 74
    #define BOARD_NR_GPIO_PINS 74
  #endif

  #if defined(STM32_CORE_VERSION)
    //Need to identify the official core better
    #define CORE_STM32_OFFICIAL
    #include "board_stm32_official.h"
  #else
    #define CORE_STM32_GENERIC
    #include "board_stm32_generic.h"
  #endif

  //Specific mode for Bluepill due to its small flash size. This disables a number of strings from being compiled into the flash
  #if defined(MCU_STM32F103C8) || defined(MCU_STM32F103CB)
    #define SMALL_FLASH_MODE
  #endif

  #if __GNUC__ < 7 //Already included on GCC 7
  extern "C" char* sbrk(int incr); //Used to freeRam
  #endif
  #ifndef digitalPinToInterrupt
  inline uint32_t  digitalPinToInterrupt(uint32_t Interrupt_pin) { return Interrupt_pin; } //This isn't included in the stm32duino libs (yet)
  #endif
#elif defined(__SAMD21G18A__)
  #include "board_samd21.h"
  #define CORE_SAMD21
#else
  #error Incorrect board selected. Please select the correct board (Usually Mega 2560) and upload again
#endif

#endif // __BOARD_H__