#ifndef STM32OFFICIAL_H
#define STM32OFFICIAL_H
#if defined(CORE_STM32_OFFICIAL)
#include <Arduino.h>
#include <HardwareTimer.h>
#include <HardwareSerial.h>

#if defined(STM32F1)
#include "stm32f1xx_ll_tim.h"
#elif defined(STM32F3)
#include "stm32f3xx_ll_tim.h"
#elif defined(STM32F4)
#include "stm32f4xx_ll_tim.h"
#else /*Default should be STM32F4*/
#include "stm32f4xx_ll_tim.h"
#endif
/*
***********************************************************************************************************
* General
*/
#define PORT_TYPE uint32_t
#define PINMASK_TYPE uint32_t
#define COMPARE_TYPE uint16_t
#define COUNTER_TYPE uint16_t
#define micros_safe() micros() //timer5 method is not used on anything but AVR, the micros_safe() macro is simply an alias for the normal micros()
#if defined(SRAM_AS_EEPROM)
    #define EEPROM_LIB_H "src/BackupSram/BackupSramAsEEPROM.h"
#elif defined(FRAM_AS_EEPROM) //https://github.com/VitorBoss/FRAM
    #define EEPROM_LIB_H <Fram.h>
#else
    #define EEPROM_LIB_H "src/SPIAsEEPROM/SPIAsEEPROM.h"
#endif

#ifndef LED_BUILTIN
  #define LED_BUILTIN PA7
#endif

#if defined(FRAM_AS_EEPROM)
  #include <Fram.h>
  #if defined(ARDUINO_BLACK_F407VE)
  FramClass EEPROM(PB5, PB4, PB3, PB0); /*(mosi, miso, sclk, ssel, clockspeed) 31/01/2020*/
  #else
  FramClass EEPROM(PB15, PB12, PB13, PB12); //Blue/Black Pills
  #endif
#endif

#define USE_SERIAL3
void initBoard();
uint16_t freeRam();
extern "C" char* sbrk(int incr);

#if defined(ARDUINO_BLUEPILL_F103C8) || defined(ARDUINO_BLUEPILL_F103CB) \
 || defined(ARDUINO_BLACKPILL_F401CC) || defined(ARDUINO_BLACKPILL_F411CE)
  #ifndef PB11 //Hack for F4 BlackPills
    #define PB11 PB10
  #endif
  //Hack to alow compile on small STM boards
  #ifndef A10
    #define A10  PA0
    #define A11  PA1
    #define A12  PA2
    #define A13  PA3
    #define A14  PA4
    #define A15  PA5
  #endif
#endif

/*
***********************************************************************************************************
* Schedules
*/
#define MAX_TIMER_PERIOD 65535*4 //The longest period of time (in uS) that the timer can permit (IN this case it is 65535 * 4, as each timer tick is 4uS)
#define uS_TO_TIMER_COMPARE(uS) (uS>>2) //Converts a given number of uS into the required number of timer ticks until that time has passed.

#define FUEL1_COUNTER (TIM3)->CNT
#define FUEL2_COUNTER (TIM3)->CNT
#define FUEL3_COUNTER (TIM3)->CNT
#define FUEL4_COUNTER (TIM3)->CNT

#define FUEL1_COMPARE (TIM3)->CCR1
#define FUEL2_COMPARE (TIM3)->CCR2
#define FUEL3_COMPARE (TIM3)->CCR3
#define FUEL4_COMPARE (TIM3)->CCR4

#define IGN1_COUNTER  (TIM2)->CNT
#define IGN2_COUNTER  (TIM2)->CNT
#define IGN3_COUNTER  (TIM2)->CNT
#define IGN4_COUNTER  (TIM2)->CNT

#define IGN1_COMPARE (TIM2)->CCR1
#define IGN2_COMPARE (TIM2)->CCR2
#define IGN3_COMPARE (TIM2)->CCR3
#define IGN4_COMPARE (TIM2)->CCR4

#define FUEL5_COUNTER (TIM5)->CNT
#define FUEL6_COUNTER (TIM5)->CNT
#define FUEL7_COUNTER (TIM5)->CNT
#define FUEL8_COUNTER (TIM5)->CNT

#define FUEL5_COMPARE (TIM5)->CCR1
#define FUEL6_COMPARE (TIM5)->CCR2
#define FUEL7_COMPARE (TIM5)->CCR3
#define FUEL8_COMPARE (TIM5)->CCR4

#define IGN5_COUNTER  (TIM4)->CNT
#define IGN6_COUNTER  (TIM4)->CNT
#define IGN7_COUNTER  (TIM4)->CNT
#define IGN8_COUNTER  (TIM4)->CNT

#define IGN5_COMPARE (TIM4)->CCR1
#define IGN6_COMPARE (TIM4)->CCR2
#define IGN7_COMPARE (TIM4)->CCR3
#define IGN8_COMPARE (TIM4)->CCR4

  
#define FUEL1_TIMER_ENABLE() (TIM3)->CCER |= TIM_CCER_CC1E
#define FUEL2_TIMER_ENABLE() (TIM3)->CCER |= TIM_CCER_CC2E
#define FUEL3_TIMER_ENABLE() (TIM3)->CCER |= TIM_CCER_CC3E
#define FUEL4_TIMER_ENABLE() (TIM3)->CCER |= TIM_CCER_CC4E

#define FUEL1_TIMER_DISABLE() (TIM3)->CCER &= ~TIM_CCER_CC1E
#define FUEL2_TIMER_DISABLE() (TIM3)->CCER &= ~TIM_CCER_CC2E
#define FUEL3_TIMER_DISABLE() (TIM3)->CCER &= ~TIM_CCER_CC3E
#define FUEL4_TIMER_DISABLE() (TIM3)->CCER &= ~TIM_CCER_CC4E

#define IGN1_TIMER_ENABLE() (TIM2)->CCER |= TIM_CCER_CC1E
#define IGN2_TIMER_ENABLE() (TIM2)->CCER |= TIM_CCER_CC2E
#define IGN3_TIMER_ENABLE() (TIM2)->CCER |= TIM_CCER_CC3E
#define IGN4_TIMER_ENABLE() (TIM2)->CCER |= TIM_CCER_CC4E

#define IGN1_TIMER_DISABLE() (TIM2)->CCER &= ~TIM_CCER_CC1E
#define IGN2_TIMER_DISABLE() (TIM2)->CCER &= ~TIM_CCER_CC2E
#define IGN3_TIMER_DISABLE() (TIM2)->CCER &= ~TIM_CCER_CC3E
#define IGN4_TIMER_DISABLE() (TIM2)->CCER &= ~TIM_CCER_CC4E


#define FUEL5_TIMER_ENABLE() (TIM5)->CCER |= TIM_CCER_CC1E
#define FUEL6_TIMER_ENABLE() (TIM5)->CCER |= TIM_CCER_CC2E
#define FUEL7_TIMER_ENABLE() (TIM5)->CCER |= TIM_CCER_CC3E
#define FUEL8_TIMER_ENABLE() (TIM5)->CCER |= TIM_CCER_CC4E

#define FUEL5_TIMER_DISABLE() (TIM5)->CCER &= ~TIM_CCER_CC1E
#define FUEL6_TIMER_DISABLE() (TIM5)->CCER &= ~TIM_CCER_CC2E
#define FUEL7_TIMER_DISABLE() (TIM5)->CCER &= ~TIM_CCER_CC3E
#define FUEL8_TIMER_DISABLE() (TIM5)->CCER &= ~TIM_CCER_CC4E

#define IGN5_TIMER_ENABLE() (TIM4)->CCER |= TIM_CCER_CC1E
#define IGN6_TIMER_ENABLE() (TIM4)->CCER |= TIM_CCER_CC2E
#define IGN7_TIMER_ENABLE() (TIM4)->CCER |= TIM_CCER_CC3E
#define IGN8_TIMER_ENABLE() (TIM4)->CCER |= TIM_CCER_CC4E

#define IGN5_TIMER_DISABLE() (TIM4)->CCER &= ~TIM_CCER_CC1E
#define IGN6_TIMER_DISABLE() (TIM4)->CCER &= ~TIM_CCER_CC2E
#define IGN7_TIMER_DISABLE() (TIM4)->CCER &= ~TIM_CCER_CC3E
#define IGN8_TIMER_DISABLE() (TIM4)->CCER &= ~TIM_CCER_CC4E

  


/*
***********************************************************************************************************
* Auxilliaries
*/
#define ENABLE_BOOST_TIMER()  (TIM1)->CCER |= TIM_CCER_CC2E
#define DISABLE_BOOST_TIMER() (TIM1)->CCER &= ~TIM_CCER_CC2E

#define ENABLE_VVT_TIMER()    (TIM1)->CCER |= TIM_CCER_CC3E
#define DISABLE_VVT_TIMER()   (TIM1)->CCER &= ~TIM_CCER_CC3E

#define BOOST_TIMER_COMPARE   (TIM1)->CCR2
#define BOOST_TIMER_COUNTER   (TIM1)->CNT
#define VVT_TIMER_COMPARE     (TIM1)->CCR3
#define VVT_TIMER_COUNTER     (TIM1)->CNT

/*
***********************************************************************************************************
* Idle
*/
#define IDLE_COUNTER   (TIM1)->CNT
#define IDLE_COMPARE   (TIM1)->CCR4

#define IDLE_TIMER_ENABLE()  (TIM1)->CCER |= TIM_CCER_CC4E
#define IDLE_TIMER_DISABLE() (TIM1)->CCER &= ~TIM_CCER_CC4E

/*
***********************************************************************************************************
* Timers
*/

HardwareTimer Timer1(TIM1);
HardwareTimer Timer2(TIM2);
HardwareTimer Timer3(TIM3);
HardwareTimer Timer4(TIM4);
#if !defined(ARDUINO_BLUEPILL_F103C8) && !defined(ARDUINO_BLUEPILL_F103CB) //F103 just have 4 timers
HardwareTimer Timer5(TIM5);
#if defined(TIM11)
HardwareTimer Timer11(TIM11);
#elif defined(TIM7)
HardwareTimer Timer11(TIM7);
#endif
#endif

#if ((STM32_CORE_VERSION_MINOR<=8) & (STM32_CORE_VERSION_MAJOR==1)) 
void oneMSInterval(HardwareTimer*);
void boostInterrupt(HardwareTimer*);
void fuelSchedule1Interrupt(HardwareTimer*);
void fuelSchedule2Interrupt(HardwareTimer*);
void fuelSchedule3Interrupt(HardwareTimer*);
void fuelSchedule4Interrupt(HardwareTimer*);
#if (INJ_CHANNELS >= 5)
void fuelSchedule5Interrupt(HardwareTimer*);
#endif
#if (INJ_CHANNELS >= 6)
void fuelSchedule6Interrupt(HardwareTimer*);
#endif
#if (INJ_CHANNELS >= 7)
void fuelSchedule7Interrupt(HardwareTimer*);
#endif
#if (INJ_CHANNELS >= 8)
void fuelSchedule8Interrupt(HardwareTimer*);
#endif
void idleInterrupt(HardwareTimer*);
void vvtInterrupt(HardwareTimer*);
void ignitionSchedule1Interrupt(HardwareTimer*);
void ignitionSchedule2Interrupt(HardwareTimer*);
void ignitionSchedule3Interrupt(HardwareTimer*);
void ignitionSchedule4Interrupt(HardwareTimer*);
#if (IGN_CHANNELS >= 5)
void ignitionSchedule5Interrupt(HardwareTimer*);
#endif
#if (IGN_CHANNELS >= 6)
void ignitionSchedule6Interrupt(HardwareTimer*);
#endif
#if (IGN_CHANNELS >= 7)
void ignitionSchedule7Interrupt(HardwareTimer*);
#endif
#if (IGN_CHANNELS >= 8)
void ignitionSchedule8Interrupt(HardwareTimer*);
#endif
#endif //End core<=1.8

/*
***********************************************************************************************************
* CAN / Second serial
*/
#if defined(ARDUINO_BLACK_F407VE)
//HardwareSerial CANSerial(PD6, PD5);
#endif

#endif //CORE_STM32
#endif //STM32_H
