#ifndef STM32F407VE_H
#define STM32F407VE_H
#if defined(CORE_STM32_OFFICIAL)
#include <Arduino.h>
#include <timer.h>
#include "stm32f4xx_ll_tim.h"
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
#elif defined(SPI_AS_EEPROM)
    #define EEPROM_LIB_H "src/SPIAsEEPROM/SPIAsEEPROM.h"
#else
    #define EEPROM_LIB_H <EEPROM.h>
#endif

#ifndef LED_BUILTIN
  #define LED_BUILTIN PA7
#endif

#define USE_SERIAL3
void initBoard();
uint16_t freeRam();
extern void oneMSIntervalIRQ(stimer_t *Timer);
extern void EmptyIRQCallback(stimer_t *Timer, uint32_t channel);
extern "C" char* sbrk(int incr);

/*
***********************************************************************************************************
* Schedules
*/
#define MAX_TIMER_PERIOD 65535*4 //The longest period of time (in uS) that the timer can permit (IN this case it is 65535 * 2, as each timer tick is 2uS)
#define MAX_TIMER_PERIOD_SLOW 65535*4//The longest period of time (in uS) that the timer can permit (IN this case it is 65535 * 2, as each timer tick is 2uS)
#define uS_TO_TIMER_COMPARE(uS) (uS>>2) //Converts a given number of uS into the required number of timer ticks until that time has passed.
#define uS_TO_TIMER_COMPARE_SLOW(uS) (uS>>2) //Converts a given number of uS into the required number of timer ticks until that time has passed.

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


/*
***********************************************************************************************************
* CAN / Second serial
*/
HardwareSerial CANSerial(PD6,PD5);

#endif //CORE_STM32
#endif //STM32_H
