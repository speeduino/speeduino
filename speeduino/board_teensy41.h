#pragma once

/** DO NOT INCLUDE DIRECTLY - should be included via board_definition.h */

#include <Arduino.h>
#include <limits>
#include "src/pins/inputPin.h"
#include "src/pins/outputPin.h"

#define CORE_TEENSY41

/*
***********************************************************************************************************
* General
*/
bool pinIsSerial(uint8_t);

/** @brief The timer overflow type
 * 
 * On some boards timers can overflow at less than the timer register width
 */
using COMPARE_TYPE = uint16_t;

namespace
{
  /** @brief Tick resolution in µS */
  // Need to handle this dynamically in the future for other frequencies
#if F_CPU == 600000000  
  //Bus Clock is 150Mhz @ 600 Mhz CPU.
  constexpr auto BUS_CLOCK = 150U; // MHz
#elif F_CPU == 528000000
  //Bus Clock is 132Mhz @ 528 Mhz CPU.
  constexpr auto BUS_CLOCK = 132U; // MHz
#elif F_CPU == 450000000
  //Bus Clock is 150Mhz @ 450 Mhz CPU.
  constexpr auto BUS_CLOCK = 150U; // MHz
#elif F_CPU == 396000000
  //Bus Clock is 132Mhz @ 396 Mhz CPU.
  constexpr auto BUS_CLOCK = 132U; // MHz
#elif F_CPU == 150000000
  //Bus Clock is 75Mhz @ 150 Mhz CPU.
  constexpr auto BUS_CLOCK = 75U; // MHz
#else
  #error Unsupported CPU frequency. 
#endif

  /** @brief Tick resolution in µS */
  constexpr auto TICK_RESOLUTION = (1U<<7U)/(double)BUS_CLOCK;

  /** @brief µS<->tick conversion precision in decimal places for fixed point math */
  constexpr uint32_t TICK_CONVERTER_PRECISION = 6UL;
}

/** @brief Convert µS to timer ticks */
static constexpr COMPARE_TYPE uS_TO_TIMER_COMPARE(uint32_t micros)
{
  constexpr uint32_t MULTIPLIER = (uint32_t)((1UL<<TICK_CONVERTER_PRECISION)/TICK_RESOLUTION);
  return (COMPARE_TYPE)((micros * MULTIPLIER) >> TICK_CONVERTER_PRECISION);
}

/** @brief Convert timer ticks to µS */
static constexpr uint32_t ticksToMicros(COMPARE_TYPE ticks)
{
  constexpr uint32_t MULTIPLIER = (uint32_t)((1UL<<TICK_CONVERTER_PRECISION)*TICK_RESOLUTION);
  return (ticks * MULTIPLIER) >> TICK_CONVERTER_PRECISION;
}

#define TS_SERIAL_BUFFER_SIZE 517 //Size of the serial buffer used by new comms protocol. For SD transfers this must be at least 512 + 1 (flag) + 4 (sector)
#define FPU_MAX_SIZE 32 //Size of the FPU buffer. 0 means no FPU.
#define BOARD_MAX_DIGITAL_PINS 54
#define BOARD_MAX_IO_PINS 54
#define RTC_ENABLED
#define SD_LOGGING //SD logging enabled by default for Teensy 4.1 as it has the slot built in
#define RTC_LIB_H "TimeLib.h"
#define SD_CONFIG  SdioConfig(FIFO_SDIO) //Set Teensy to use SDIO in FIFO mode. This is the fastest SD mode on Teensy as it offloads most of the writes
constexpr uint16_t BLOCKING_FACTOR = 251;
constexpr uint16_t TABLE_BLOCKING_FACTOR = 256;

//#define PWM_FAN_AVAILABLE
static inline bool pinIsReserved(uint8_t pin) { 
  return (pin == 0U) 
      || (pin == 42U) 
      || (pin == 43U) 
      || (pin == 44U) 
      || (pin == 45U) 
      || (pin == 46U) 
      || (pin == 47U) 
      || pinIsSerial(pin) 
  ;
}

#define INJ_CHANNELS 8
#define IGN_CHANNELS 8

/*
***********************************************************************************************************
* Schedules
*/
/*
https://github.com/luni64/TeensyTimerTool/wiki/Supported-Timers#pit---periodic-timer
https://github.com/luni64/TeensyTimerTool/wiki/Configuration#clock-setting-for-the-gpt-and-pit-timers
The Quad timer (TMR) provides 4 timers each with 4 usable compare channels. The down compare and alternating compares are not usable
FUEL 1-4: TMR1
IGN 1-4 : TMR2
FUEL 5-8: TMR3
IGN 5-8 : TMR4
*/
#define FUEL1_COUNTER TMR1_CNTR0
#define FUEL2_COUNTER TMR1_CNTR1
#define FUEL3_COUNTER TMR1_CNTR2
#define FUEL4_COUNTER TMR1_CNTR3
#define FUEL5_COUNTER TMR3_CNTR0
#define FUEL6_COUNTER TMR3_CNTR1
#define FUEL7_COUNTER TMR3_CNTR2
#define FUEL8_COUNTER TMR3_CNTR3

#define IGN1_COUNTER  TMR2_CNTR0
#define IGN2_COUNTER  TMR2_CNTR1
#define IGN3_COUNTER  TMR2_CNTR2
#define IGN4_COUNTER  TMR2_CNTR3
#define IGN5_COUNTER  TMR4_CNTR0
#define IGN6_COUNTER  TMR4_CNTR1
#define IGN7_COUNTER  TMR4_CNTR2
#define IGN8_COUNTER  TMR4_CNTR3

#define FUEL1_COMPARE TMR1_COMP10
#define FUEL2_COMPARE TMR1_COMP11
#define FUEL3_COMPARE TMR1_COMP12
#define FUEL4_COMPARE TMR1_COMP13
#define FUEL5_COMPARE TMR3_COMP10
#define FUEL6_COMPARE TMR3_COMP11
#define FUEL7_COMPARE TMR3_COMP12
#define FUEL8_COMPARE TMR3_COMP13

#define IGN1_COMPARE  TMR2_COMP10
#define IGN2_COMPARE  TMR2_COMP11
#define IGN3_COMPARE  TMR2_COMP12
#define IGN4_COMPARE  TMR2_COMP13
#define IGN5_COMPARE  TMR4_COMP10
#define IGN6_COMPARE  TMR4_COMP11
#define IGN7_COMPARE  TMR4_COMP12
#define IGN8_COMPARE  TMR4_COMP13

static inline void FUEL1_TIMER_ENABLE(void)  {TMR1_CSCTRL0 &= ~TMR_CSCTRL_TCF1; TMR1_CSCTRL0 |= TMR_CSCTRL_TCF1EN;} //Write 1 to the TCFIEN (Channel Interrupt Enable) bit of channel 0 Status/Control
static inline void FUEL2_TIMER_ENABLE(void)  {TMR1_CSCTRL1 &= ~TMR_CSCTRL_TCF1; TMR1_CSCTRL1 |= TMR_CSCTRL_TCF1EN;}
static inline void FUEL3_TIMER_ENABLE(void)  {TMR1_CSCTRL2 &= ~TMR_CSCTRL_TCF1; TMR1_CSCTRL2 |= TMR_CSCTRL_TCF1EN;}
static inline void FUEL4_TIMER_ENABLE(void)  {TMR1_CSCTRL3 &= ~TMR_CSCTRL_TCF1; TMR1_CSCTRL3 |= TMR_CSCTRL_TCF1EN;}
static inline void FUEL5_TIMER_ENABLE(void)  {TMR3_CSCTRL0 &= ~TMR_CSCTRL_TCF1; TMR3_CSCTRL0 |= TMR_CSCTRL_TCF1EN;}
static inline void FUEL6_TIMER_ENABLE(void)  {TMR3_CSCTRL1 &= ~TMR_CSCTRL_TCF1; TMR3_CSCTRL1 |= TMR_CSCTRL_TCF1EN;}
static inline void FUEL7_TIMER_ENABLE(void)  {TMR3_CSCTRL2 &= ~TMR_CSCTRL_TCF1; TMR3_CSCTRL2 |= TMR_CSCTRL_TCF1EN;}
static inline void FUEL8_TIMER_ENABLE(void)  {TMR3_CSCTRL3 &= ~TMR_CSCTRL_TCF1; TMR3_CSCTRL3 |= TMR_CSCTRL_TCF1EN;}

static inline void FUEL1_TIMER_DISABLE(void)  {TMR1_CSCTRL0 &= ~TMR_CSCTRL_TCF1EN;} //Write 0 to the TCFIEN (Channel Interrupt Enable) bit of channel 0 Status/Control
static inline void FUEL2_TIMER_DISABLE(void)  {TMR1_CSCTRL1 &= ~TMR_CSCTRL_TCF1EN;}
static inline void FUEL3_TIMER_DISABLE(void)  {TMR1_CSCTRL2 &= ~TMR_CSCTRL_TCF1EN;}
static inline void FUEL4_TIMER_DISABLE(void)  {TMR1_CSCTRL3 &= ~TMR_CSCTRL_TCF1EN;}
static inline void FUEL5_TIMER_DISABLE(void)  {TMR3_CSCTRL0 &= ~TMR_CSCTRL_TCF1EN;}
static inline void FUEL6_TIMER_DISABLE(void)  {TMR3_CSCTRL1 &= ~TMR_CSCTRL_TCF1EN;}
static inline void FUEL7_TIMER_DISABLE(void)  {TMR3_CSCTRL2 &= ~TMR_CSCTRL_TCF1EN;}
static inline void FUEL8_TIMER_DISABLE(void)  {TMR3_CSCTRL3 &= ~TMR_CSCTRL_TCF1EN;}

static inline void IGN1_TIMER_ENABLE(void)  {TMR2_CSCTRL0 &= ~TMR_CSCTRL_TCF1; TMR2_CSCTRL0 |= TMR_CSCTRL_TCF1EN;}
static inline void IGN2_TIMER_ENABLE(void)  {TMR2_CSCTRL1 &= ~TMR_CSCTRL_TCF1; TMR2_CSCTRL1 |= TMR_CSCTRL_TCF1EN;}
static inline void IGN3_TIMER_ENABLE(void)  {TMR2_CSCTRL2 &= ~TMR_CSCTRL_TCF1; TMR2_CSCTRL2 |= TMR_CSCTRL_TCF1EN;}
static inline void IGN4_TIMER_ENABLE(void)  {TMR2_CSCTRL3 &= ~TMR_CSCTRL_TCF1; TMR2_CSCTRL3 |= TMR_CSCTRL_TCF1EN;}
static inline void IGN5_TIMER_ENABLE(void)  {TMR4_CSCTRL0 &= ~TMR_CSCTRL_TCF1; TMR4_CSCTRL0 |= TMR_CSCTRL_TCF1EN;}
static inline void IGN6_TIMER_ENABLE(void)  {TMR4_CSCTRL1 &= ~TMR_CSCTRL_TCF1; TMR4_CSCTRL1 |= TMR_CSCTRL_TCF1EN;}
static inline void IGN7_TIMER_ENABLE(void)  {TMR4_CSCTRL2 &= ~TMR_CSCTRL_TCF1; TMR4_CSCTRL2 |= TMR_CSCTRL_TCF1EN;}
static inline void IGN8_TIMER_ENABLE(void)  {TMR4_CSCTRL3 &= ~TMR_CSCTRL_TCF1; TMR4_CSCTRL3 |= TMR_CSCTRL_TCF1EN;}

static inline void IGN1_TIMER_DISABLE(void)  {TMR2_CSCTRL0 &= ~TMR_CSCTRL_TCF1EN;}
static inline void IGN2_TIMER_DISABLE(void)  {TMR2_CSCTRL1 &= ~TMR_CSCTRL_TCF1EN;}
static inline void IGN3_TIMER_DISABLE(void)  {TMR2_CSCTRL2 &= ~TMR_CSCTRL_TCF1EN;}
static inline void IGN4_TIMER_DISABLE(void)  {TMR2_CSCTRL3 &= ~TMR_CSCTRL_TCF1EN;}
static inline void IGN5_TIMER_DISABLE(void)  {TMR4_CSCTRL0 &= ~TMR_CSCTRL_TCF1EN;}
static inline void IGN6_TIMER_DISABLE(void)  {TMR4_CSCTRL1 &= ~TMR_CSCTRL_TCF1EN;}
static inline void IGN7_TIMER_DISABLE(void)  {TMR4_CSCTRL2 &= ~TMR_CSCTRL_TCF1EN;}
static inline void IGN8_TIMER_DISABLE(void)  {TMR4_CSCTRL3 &= ~TMR_CSCTRL_TCF1EN;}

/*
***********************************************************************************************************
* Auxiliaries
*/
#define ENABLE_BOOST_TIMER()  PIT_TCTRL1 |= PIT_TCTRL_TEN
#define DISABLE_BOOST_TIMER() PIT_TCTRL1 &= ~PIT_TCTRL_TEN

#define ENABLE_VVT_TIMER()    PIT_TCTRL2 |= PIT_TCTRL_TEN
#define DISABLE_VVT_TIMER()   PIT_TCTRL2 &= ~PIT_TCTRL_TEN

//Ran out of timers, this most likely won't work. This should be possible to implement with the GPT timer. 
#define ENABLE_FAN_TIMER()    TMR3_CSCTRL1 |= TMR_CSCTRL_TCF2EN
#define DISABLE_FAN_TIMER()   TMR3_CSCTRL1 &= ~TMR_CSCTRL_TCF2EN

#define BOOST_TIMER_COMPARE   PIT_LDVAL1
#define BOOST_TIMER_COUNTER   0
#define VVT_TIMER_COMPARE     PIT_LDVAL2
#define VVT_TIMER_COUNTER     0

//these probaply need to be PIT_LDVAL something???
#define FAN_TIMER_COMPARE     TMR3_COMP22
#define FAN_TIMER_COUNTER     TMR3_CNTR1

/*
***********************************************************************************************************
* Idle
*/
#define IDLE_COUNTER 0
#define IDLE_COMPARE PIT_LDVAL0

#define IDLE_TIMER_ENABLE() PIT_TCTRL0 |= PIT_TCTRL_TEN
#define IDLE_TIMER_DISABLE() PIT_TCTRL0 &= ~PIT_TCTRL_TEN

/*
***********************************************************************************************************
* CAN / Second serial
*/
#define SECONDARY_SERIAL_T HardwareSerial

#include <FlexCAN_T4.h>
#define NATIVE_CAN_AVAILABLE //Disable for now as it causes lockup 

using boardInputPin_t = inputPin_t;
using boardOutputPin_t = outputPin_t;

/** @brief Analog pin mapping */
constexpr uint8_t ANALOG_PINS[] = { _ANALOG_PINS_A0_A14, A15, A16 };