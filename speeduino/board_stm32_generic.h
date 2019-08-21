#ifndef STM32_H
#define STM32_H
#if defined(CORE_STM32_GENERIC)

/*
***********************************************************************************************************
* General
*/
  #define PORT_TYPE uint32_t
  #define PINMASK_TYPE uint32_t
  #define COMPARE_TYPE uint16_t
  #define COUNTER_TYPE uint16_t
  #define micros_safe() micros() //timer5 method is not used on anything but AVR, the micros_safe() macro is simply an alias for the normal micros()
  #ifndef USE_SERIAL3
  #define USE_SERIAL3
  #define EEPROM_LIB_H <Fram.h>
  #endif
  void initBoard();
  uint16_t freeRam();

  #ifndef Serial
    #define Serial Serial1
  #endif

  #if defined(USE_FRAM)
    #include <Fram.h>
    FramClass EEPROM(PB0, PB3, PB4, PB5, 15000000);
  #endif

  //Much of the below is not correct, but included to allow compilation
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


/*
***********************************************************************************************************
* Schedules
* Timers Table for STM32F1
*   TIMER1    TIMER2    TIMER3    TIMER4
* 1 -       1 - INJ1  1 - IGN1  1 - oneMSInterval
* 2 - BOOST 2 - INJ2  2 - IGN2  2 -
* 3 - VVT   3 - INJ3  3 - IGN3  3 -
* 4 - IDLE  4 - INJ4  4 - IGN4  4 -
*
* Timers Table for STM32F4
*   TIMER1    TIMER2    TIMER3    TIMER4    TIMER5    TIMER8
* 1 -       1 - INJ1  1 - IGN1  1 - IGN5  1 - INJ5  1 - oneMSInterval
* 2 - BOOST 2 - INJ2  2 - IGN2  2 - IGN6  2 - INJ6  2 - 
* 3 - VVT   3 - INJ3  3 - IGN3  3 - IGN7  3 - INJ7  3 - 
* 4 - IDLE  4 - INJ4  4 - IGN4  4 - IGN8  4 - INJ8  4 - 
*
*/
  #define MAX_TIMER_PERIOD 65535*2 //The longest period of time (in uS) that the timer can permit (IN this case it is 65535 * 2, as each timer tick is 2uS)
  #define MAX_TIMER_PERIOD_SLOW  65535*2 //The longest period of time (in uS) that the timer can permit (IN this case it is 65535 * 2, as each timer tick is 2uS)
  #define uS_TO_TIMER_COMPARE(uS) (uS >> 1) //Converts a given number of uS into the required number of timer ticks until that time has passed.
  #define uS_TO_TIMER_COMPARE_SLOW(uS) (uS >> 1) //Converts a given number of uS into the required number of timer ticks until that time has passed.
  #if defined(ARDUINO_ARCH_STM32) && !defined(_VARIANT_ARDUINO_STM32_) // STM32GENERIC core
    #define FUEL1_COUNTER (TIM2)->CNT
    #define FUEL2_COUNTER (TIM2)->CNT
    #define FUEL3_COUNTER (TIM2)->CNT
    #define FUEL4_COUNTER (TIM2)->CNT

    #define FUEL1_COMPARE (TIM2)->CCR1
    #define FUEL2_COMPARE (TIM2)->CCR2
    #define FUEL3_COMPARE (TIM2)->CCR3
    #define FUEL4_COMPARE (TIM2)->CCR4

    #define IGN1_COUNTER  (TIM3)->CNT
    #define IGN2_COUNTER  (TIM3)->CNT
    #define IGN3_COUNTER  (TIM3)->CNT
    #define IGN4_COUNTER  (TIM3)->CNT

    #define IGN1_COMPARE (TIM3)->CCR1
    #define IGN2_COMPARE (TIM3)->CCR2
    #define IGN3_COMPARE (TIM3)->CCR3
    #define IGN4_COMPARE (TIM3)->CCR4

    #ifndef SMALL_FLASH_MODE
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
    #endif
    //https://github.com/rogerclarkmelbourne/Arduino_STM32/blob/754bc2969921f1ef262bd69e7faca80b19db7524/STM32F1/system/libmaple/include/libmaple/timer.h#L444
    #define FUEL1_TIMER_ENABLE() (TIM2)->CCER |= TIM_CCER_CC1E
    #define FUEL2_TIMER_ENABLE() (TIM2)->CCER |= TIM_CCER_CC2E
    #define FUEL3_TIMER_ENABLE() (TIM2)->CCER |= TIM_CCER_CC3E
    #define FUEL4_TIMER_ENABLE() (TIM2)->CCER |= TIM_CCER_CC4E

    #define FUEL1_TIMER_DISABLE() (TIM2)->CCER &= ~TIM_CCER_CC1E
    #define FUEL2_TIMER_DISABLE() (TIM2)->CCER &= ~TIM_CCER_CC2E
    #define FUEL3_TIMER_DISABLE() (TIM2)->CCER &= ~TIM_CCER_CC3E
    #define FUEL4_TIMER_DISABLE() (TIM2)->CCER &= ~TIM_CCER_CC4E

    #define IGN1_TIMER_ENABLE() (TIM3)->CCER |= TIM_CCER_CC1E
    #define IGN2_TIMER_ENABLE() (TIM3)->CCER |= TIM_CCER_CC2E
    #define IGN3_TIMER_ENABLE() (TIM3)->CCER |= TIM_CCER_CC3E
    #define IGN4_TIMER_ENABLE() (TIM3)->CCER |= TIM_CCER_CC4E

    #define IGN1_TIMER_DISABLE() (TIM3)->CCER &= ~TIM_CCER_CC1E
    #define IGN2_TIMER_DISABLE() (TIM3)->CCER &= ~TIM_CCER_CC2E
    #define IGN3_TIMER_DISABLE() (TIM3)->CCER &= ~TIM_CCER_CC3E
    #define IGN4_TIMER_DISABLE() (TIM3)->CCER &= ~TIM_CCER_CC4E

    #ifndef SMALL_FLASH_MODE
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
    #endif
  #else //libmaple core aka STM32DUINO
    #define FUEL1_COUNTER (TIMER2->regs).gen->CNT
    #define FUEL2_COUNTER (TIMER2->regs).gen->CNT
    #define FUEL3_COUNTER (TIMER2->regs).gen->CNT
    #define FUEL4_COUNTER (TIMER2->regs).gen->CNT

    #define FUEL1_COMPARE (TIMER2->regs).gen->CCR1
    #define FUEL2_COMPARE (TIMER2->regs).gen->CCR2
    #define FUEL3_COMPARE (TIMER2->regs).gen->CCR3
    #define FUEL4_COMPARE (TIMER2->regs).gen->CCR4

    #define IGN1_COUNTER  (TIMER3->regs).gen->CNT
    #define IGN2_COUNTER  (TIMER3->regs).gen->CNT
    #define IGN3_COUNTER  (TIMER3->regs).gen->CNT
    #define IGN4_COUNTER  (TIMER3->regs).gen->CNT

    #define IGN1_COMPARE (TIMER3->regs).gen->CCR1
    #define IGN2_COMPARE (TIMER3->regs).gen->CCR2
    #define IGN3_COMPARE (TIMER3->regs).gen->CCR3
    #define IGN4_COMPARE (TIMER3->regs).gen->CCR4

    #ifndef SMALL_FLASH_MODE
        #define FUEL5_COUNTER (TIMER5->regs).gen->CNT
        #define FUEL6_COUNTER (TIMER5->regs).gen->CNT
        #define FUEL7_COUNTER (TIMER5->regs).gen->CNT
        #define FUEL8_COUNTER (TIMER5->regs).gen->CNT

        #define FUEL5_COMPARE (TIMER5->regs).gen->CCR1
        #define FUEL6_COMPARE (TIMER5->regs).gen->CCR2
        #define FUEL7_COMPARE (TIMER5->regs).gen->CCR3
        #define FUEL8_COMPARE (TIMER5->regs).gen->CCR4

        #define IGN5_COUNTER  (TIMER4->regs).gen->CNT
        #define IGN6_COUNTER  (TIMER4->regs).gen->CNT
        #define IGN7_COUNTER  (TIMER4->regs).gen->CNT
        #define IGN8_COUNTER  (TIMER4->regs).gen->CNT

        #define IGN5_COMPARE (TIMER4->regs).gen->CCR1
        #define IGN6_COMPARE (TIMER4->regs).gen->CCR2
        #define IGN7_COMPARE (TIMER4->regs).gen->CCR3
        #define IGN8_COMPARE (TIMER4->regs).gen->CCR4
    #endif
    //https://github.com/rogerclarkmelbourne/Arduino_STM32/blob/754bc2969921f1ef262bd69e7faca80b19db7524/STM32F1/system/libmaple/include/libmaple/timer.h#L444
    #define FUEL1_TIMER_ENABLE() (TIMER2->regs).gen->CCER |= TIMER_CCER_CC1E
    #define FUEL2_TIMER_ENABLE() (TIMER2->regs).gen->CCER |= TIMER_CCER_CC2E
    #define FUEL3_TIMER_ENABLE() (TIMER2->regs).gen->CCER |= TIMER_CCER_CC3E
    #define FUEL4_TIMER_ENABLE() (TIMER2->regs).gen->CCER |= TIMER_CCER_CC4E

    #define FUEL1_TIMER_DISABLE() (TIMER2->regs).gen->CCER &= ~TIMER_CCER_CC1E
    #define FUEL2_TIMER_DISABLE() (TIMER2->regs).gen->CCER &= ~TIMER_CCER_CC2E
    #define FUEL3_TIMER_DISABLE() (TIMER2->regs).gen->CCER &= ~TIMER_CCER_CC3E
    #define FUEL4_TIMER_DISABLE() (TIMER2->regs).gen->CCER &= ~TIMER_CCER_CC4E

    #define IGN1_TIMER_DISABLE() (TIMER3->regs).gen->CCER &= ~TIMER_CCER_CC1E
    #define IGN2_TIMER_DISABLE() (TIMER3->regs).gen->CCER &= ~TIMER_CCER_CC2E
    #define IGN3_TIMER_DISABLE() (TIMER3->regs).gen->CCER &= ~TIMER_CCER_CC3E
    #define IGN4_TIMER_DISABLE() (TIMER3->regs).gen->CCER &= ~TIMER_CCER_CC4E

    #define IGN1_TIMER_ENABLE() (TIMER3->regs).gen->CCER |= TIMER_CCER_CC1E
    #define IGN2_TIMER_ENABLE() (TIMER3->regs).gen->CCER |= TIMER_CCER_CC2E
    #define IGN3_TIMER_ENABLE() (TIMER3->regs).gen->CCER |= TIMER_CCER_CC3E
    #define IGN4_TIMER_ENABLE() (TIMER3->regs).gen->CCER |= TIMER_CCER_CC4E

    #ifndef SMALL_FLASH_MODE
        #define FUEL5_TIMER_ENABLE() (TIMER5->regs).gen->CCER |= TIMER_CCER_CC1E
        #define FUEL6_TIMER_ENABLE() (TIMER5->regs).gen->CCER |= TIMER_CCER_CC2E
        #define FUEL7_TIMER_ENABLE() (TIMER5->regs).gen->CCER |= TIMER_CCER_CC3E
        #define FUEL8_TIMER_ENABLE() (TIMER5->regs).gen->CCER |= TIMER_CCER_CC4E

        #define IGN5_TIMER_ENABLE() (TIMER4->regs).gen->CCER |= TIMER_CCER_CC1E
        #define IGN6_TIMER_ENABLE() (TIMER4->regs).gen->CCER |= TIMER_CCER_CC2E
        #define IGN7_TIMER_ENABLE() (TIMER4->regs).gen->CCER |= TIMER_CCER_CC3E
        #define IGN8_TIMER_ENABLE() (TIMER4->regs).gen->CCER |= TIMER_CCER_CC4E

        #define FUEL5_TIMER_DISABLE() (TIMER5->regs).gen->CCER &= ~TIMER_CCER_CC1E
        #define FUEL6_TIMER_DISABLE() (TIMER5->regs).gen->CCER &= ~TIMER_CCER_CC2E
        #define FUEL7_TIMER_DISABLE() (TIMER5->regs).gen->CCER &= ~TIMER_CCER_CC3E
        #define FUEL8_TIMER_DISABLE() (TIMER5->regs).gen->CCER &= ~TIMER_CCER_CC4E

        #define IGN5_TIMER_DISABLE() (TIMER4->regs).gen->CCER &= ~TIMER_CCER_CC1E
        #define IGN6_TIMER_DISABLE() (TIMER4->regs).gen->CCER &= ~TIMER_CCER_CC2E
        #define IGN7_TIMER_DISABLE() (TIMER4->regs).gen->CCER &= ~TIMER_CCER_CC3E
        #define IGN8_TIMER_DISABLE() (TIMER4->regs).gen->CCER &= ~TIMER_CCER_CC4E
    #endif
#endif  

/*
***********************************************************************************************************
* Auxilliaries
*/
    #if defined(ARDUINO_ARCH_STM32) && !defined(_VARIANT_ARDUINO_STM32_) // STM32GENERIC core
        #define ENABLE_BOOST_TIMER()  (TIM1)->CCER |= TIM_CCER_CC2E
        #define DISABLE_BOOST_TIMER() (TIM1)->CCER &= ~TIM_CCER_CC2E

        #define ENABLE_VVT_TIMER()    (TIM1)->CCER |= TIM_CCER_CC3E
        #define DISABLE_VVT_TIMER()   (TIM1)->CCER &= ~TIM_CCER_CC3E

        #define BOOST_TIMER_COMPARE   (TIM1)->CCR2
        #define BOOST_TIMER_COUNTER   (TIM1)->CNT
        #define VVT_TIMER_COMPARE     (TIM1)->CCR3
        #define VVT_TIMER_COUNTER     (TIM1)->CNT
    #else //libmaple core aka STM32DUINO
        #define ENABLE_BOOST_TIMER()  (TIMER1->regs).gen->CCER |= TIMER_CCER_CC2E
        #define DISABLE_BOOST_TIMER() (TIMER1->regs).gen->CCER &= ~TIMER_CCER_CC2E

        #define ENABLE_VVT_TIMER()    (TIMER1->regs).gen->CCER |= TIMER_CCER_CC3E
        #define DISABLE_VVT_TIMER()   (TIMER1->regs).gen->CCER &= ~TIMER_CCER_CC3E

        #define BOOST_TIMER_COMPARE   (TIMER1->regs).gen->CCR2
        #define BOOST_TIMER_COUNTER   (TIMER1->regs).gen->CNT
        #define VVT_TIMER_COMPARE     (TIMER1->regs).gen->CCR3
        #define VVT_TIMER_COUNTER     (TIMER1->regs).gen->CNT
    #endif

/*
***********************************************************************************************************
* Idle
*/
    #if defined(ARDUINO_ARCH_STM32) && !defined(_VARIANT_ARDUINO_STM32_) // STM32GENERIC core
        #define IDLE_COUNTER   (TIM1)->CNT
        #define IDLE_COMPARE   (TIM1)->CCR4

        #define IDLE_TIMER_ENABLE()  (TIM1)->CCER |= TIM_CCER_CC4E
        #define IDLE_TIMER_DISABLE() (TIM1)->CCER &= ~TIM_CCER_CC4E
    #else //libmaple core aka STM32DUINO
        #define IDLE_COUNTER   (TIMER1->regs).gen->CNT
        #define IDLE_COMPARE   (TIMER1->regs).gen->CCR4

        #define IDLE_TIMER_ENABLE()  (TIMER1->regs).gen->CCER |= TIMER_CCER_CC4E
        #define IDLE_TIMER_DISABLE() (TIMER1->regs).gen->CCER &= ~TIMER_CCER_CC4E
    #endif

/*
***********************************************************************************************************
* Timers
*/


/*
***********************************************************************************************************
* CAN / Second serial
*/

#endif //CORE_STM32
#endif //STM32_H