#ifndef SAMD21_H
#define SAMD21_H
#if defined(CORE_SAMD21)

#include "sam.h"

/*
***********************************************************************************************************
* General
*/
  #define PORT_TYPE uint32_t //Size of the port variables (Eg inj1_pin_port). Most systems use a byte, but SAMD21 is a 32-bit unsigned int
  #define BOARD_MAX_DIGITAL_PINS 54 //digital pins +1
  #define BOARD_MAX_IO_PINS  58 //digital pins + analog channels + 1

  //Additional analog pins (These won't work without other changes)
  #define PIN_A6               (8ul)
  #define PIN_A7               (9ul)
  #define PIN_A8               (10ul)
  #define PIN_A9               (11ul)
  #define PIN_A13               (9ul)
  #define PIN_A14               (9ul)
  #define PIN_A15               (9ul)

  static const uint8_t A6  = PIN_A6;
  static const uint8_t A7  = PIN_A7;
  static const uint8_t A8  = PIN_A8;
  static const uint8_t A9  = PIN_A9;
  static const uint8_t A13  = PIN_A13;
  static const uint8_t A14  = PIN_A14;
  static const uint8_t A15  = PIN_A15;

/*
***********************************************************************************************************
* Schedules
*/
  //See : https://electronics.stackexchange.com/questions/325159/the-value-of-the-tcc-counter-on-an-atsam-controller-always-reads-as-zero
  #define FUEL1_COUNTER TCC0->COUNT.reg
  #define FUEL2_COUNTER TCC0->COUNT.reg
  #define FUEL3_COUNTER TCC0->COUNT.reg
  #define FUEL4_COUNTER TCC0->COUNT.reg
  //The below are NOT YET RIGHT!
  #define FUEL5_COUNTER TCC0->COUNT.reg
  #define FUEL6_COUNTER TCC0->COUNT.reg
  #define FUEL7_COUNTER TCC0->COUNT.reg
  #define FUEL8_COUNTER TCC0->COUNT.reg

  #define IGN1_COUNTER  TCC1->COUNT.reg
  #define IGN2_COUNTER  TCC1->COUNT.reg
  #define IGN3_COUNTER  TCC2->COUNT.reg
  #define IGN4_COUNTER  TCC2->COUNT.reg
  //The below are NOT YET RIGHT!
  #define IGN5_COUNTER  TCC1->COUNT.reg
  #define IGN6_COUNTER  TCC1->COUNT.reg
  #define IGN7_COUNTER  TCC2->COUNT.reg
  #define IGN8_COUNTER  TCC2->COUNT.reg

  #define FUEL1_COMPARE TCC0->CC[0].bit.CC
  #define FUEL2_COMPARE TCC0->CC[1].bit.CC
  #define FUEL3_COMPARE TCC0->CC[2].bit.CC
  #define FUEL4_COMPARE TCC0->CC[3].bit.CC
  //The below are NOT YET RIGHT!
  #define FUEL5_COMPARE TCC0->CC0
  #define FUEL6_COMPARE TCC0->CC1
  #define FUEL7_COMPARE TCC0->CC2
  #define FUEL8_COMPARE TCC0->CC3

  #define IGN1_COMPARE  TCC1->CC[0].bit.CC
  #define IGN2_COMPARE  TCC1->CC[1].bit.CC
  #define IGN3_COMPARE  TCC2->CC[0].bit.CC
  #define IGN4_COMPARE  TCC2->CC[1].bit.CC
  //The below are NOT YET RIGHT!
  #define IGN5_COMPARE  TCC1->CC[0].bit.CC
  #define IGN6_COMPARE  TCC1->CC[1].bit.CC
  #define IGN7_COMPARE  TCC2->CC[0].bit.CC
  #define IGN8_COMPARE  TCC2->CC[1].bit.CC

  #define FUEL1_TIMER_ENABLE() TCC0->INTENSET.bit.MC0 = 0x1
  #define FUEL2_TIMER_ENABLE() TCC0->INTENSET.bit.MC1 = 0x1
  #define FUEL3_TIMER_ENABLE() TCC0->INTENSET.bit.MC2 = 0x1
  #define FUEL4_TIMER_ENABLE() TCC0->INTENSET.bit.MC3 = 0x1
  //The below are NOT YET RIGHT!
  #define FUEL5_TIMER_ENABLE() TCC0->INTENSET.bit.MC0 = 0x1
  #define FUEL6_TIMER_ENABLE() TCC0->INTENSET.bit.MC1 = 0x1
  #define FUEL7_TIMER_ENABLE() TCC0->INTENSET.bit.MC2 = 0x1
  #define FUEL8_TIMER_ENABLE() TCC0->INTENSET.bit.MC3 = 0x1

  #define FUEL1_TIMER_DISABLE() TCC0->INTENSET.bit.MC0 = 0x0
  #define FUEL2_TIMER_DISABLE() TCC0->INTENSET.bit.MC0 = 0x0
  #define FUEL3_TIMER_DISABLE() TCC0->INTENSET.bit.MC0 = 0x0
  #define FUEL4_TIMER_DISABLE() TCC0->INTENSET.bit.MC0 = 0x0
  //The below are NOT YET RIGHT!
  #define FUEL5_TIMER_DISABLE() TCC0->INTENSET.bit.MC0 = 0x0
  #define FUEL6_TIMER_DISABLE() TCC0->INTENSET.bit.MC0 = 0x0
  #define FUEL7_TIMER_DISABLE() TCC0->INTENSET.bit.MC0 = 0x0
  #define FUEL8_TIMER_DISABLE() TCC0->INTENSET.bit.MC0 = 0x0

  #define IGN1_TIMER_ENABLE() TCC1->INTENSET.bit.MC0 = 0x1
  #define IGN2_TIMER_ENABLE() TCC1->INTENSET.bit.MC1 = 0x1
  #define IGN3_TIMER_ENABLE() TCC2->INTENSET.bit.MC0 = 0x1
  #define IGN4_TIMER_ENABLE() TCC2->INTENSET.bit.MC1 = 0x1
  //The below are NOT YET RIGHT!
  #define IGN5_TIMER_ENABLE() TCC1->INTENSET.bit.MC0 = 0x1
  #define IGN6_TIMER_ENABLE() TCC1->INTENSET.bit.MC1 = 0x1
  #define IGN7_TIMER_ENABLE() TCC2->INTENSET.bit.MC0 = 0x1
  #define IGN8_TIMER_ENABLE() TCC2->INTENSET.bit.MC1 = 0x1

  #define IGN1_TIMER_DISABLE() TCC1->INTENSET.bit.MC0 = 0x0
  #define IGN2_TIMER_DISABLE() TCC1->INTENSET.bit.MC1 = 0x0
  #define IGN3_TIMER_DISABLE() TCC2->INTENSET.bit.MC0 = 0x0
  #define IGN4_TIMER_DISABLE() TCC2->INTENSET.bit.MC1 = 0x0
  //The below are NOT YET RIGHT!
  #define IGN5_TIMER_DISABLE() TCC1->INTENSET.bit.MC0 = 0x0
  #define IGN6_TIMER_DISABLE() TCC1->INTENSET.bit.MC1 = 0x0
  #define IGN7_TIMER_DISABLE() TCC2->INTENSET.bit.MC0 = 0x0
  #define IGN8_TIMER_DISABLE() TCC2->INTENSET.bit.MC1 = 0x0

  #define MAX_TIMER_PERIOD 139808 // 2.13333333uS * 65535
  #define MAX_TIMER_PERIOD_SLOW 139808
  #define uS_TO_TIMER_COMPARE(uS) ((uS * 15) >> 5) //Converts a given number of uS into the required number of timer ticks until that time has passed.
  //Hack compatibility with AVR timers that run at different speeds
  #define uS_TO_TIMER_COMPARE_SLOW(uS) ((uS * 15) >> 5)

/*
***********************************************************************************************************
* Auxilliaries
*/
  //Uses the 2nd TC
  //The 2nd TC is referred to as TC4
  #define ENABLE_BOOST_TIMER()  TC4->COUNT16.INTENSET.bit.MC0 = 0x1    // Enable match interrupts on compare channel 0 
  #define DISABLE_BOOST_TIMER() TC4->COUNT16.INTENSET.bit.MC0 = 0x0

  #define ENABLE_VVT_TIMER()    TC4->COUNT16.INTENSET.bit.MC1 = 0x1 
  #define DISABLE_VVT_TIMER()   TC4->COUNT16.INTENSET.bit.MC1 = 0x0

  #define BOOST_TIMER_COMPARE   TC4->COUNT16.CC[0].reg
  #define BOOST_TIMER_COUNTER   TC4->COUNT16.COUNT.bit.COUNT
  #define VVT_TIMER_COMPARE     TC4->COUNT16.CC[1].reg
  #define VVT_TIMER_COUNTER     TC4->COUNT16.COUNT.bit.COUNT

/*
***********************************************************************************************************
* Idle
*/
  //3rd TC is aliased as TC5
  #define IDLE_COUNTER TC5->COUNT16.COUNT.bit.COUNT
  #define IDLE_COMPARE TC5->COUNT16.CC[0].reg

  #define IDLE_TIMER_ENABLE() TC5->COUNT16.INTENSET.bit.MC0 = 0x1
  #define IDLE_TIMER_DISABLE() TC5->COUNT16.INTENSET.bit.MC0 = 0x0

/*
***********************************************************************************************************
* CAN / Second serial
*/
  Uart CANSerial (&sercom3, 0, 1, SERCOM_RX_PAD_1, UART_TX_PAD_0);

#endif //CORE_SAMD21
#endif //SAMD21_H