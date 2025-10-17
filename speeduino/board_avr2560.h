#ifndef AVR2560_H
#define AVR2560_H

#include "globals.h"
#if defined(CORE_AVR)

#include <avr/interrupt.h>
#include <avr/io.h>

/*
***********************************************************************************************************
* General
*/
  #define PORT_TYPE uint8_t //Size of the port variables (Eg inj1_pin_port).
  #define PINMASK_TYPE uint8_t
  #define COMPARE_TYPE uint16_t
  #define COUNTER_TYPE uint16_t
  #define SERIAL_BUFFER_SIZE (256+7+1) //Size of the serial buffer used by new comms protocol. The largest single packet is the O2 calibration which is 256 bytes + 7 bytes of overhead
  #define FPU_MAX_SIZE 0 //Size of the FPU buffer. 0 means no FPU.
  #ifdef USE_SPI_EEPROM
    #define MAX_BLOCK_WRITE_BYTES 20
  #endif
  #ifdef PLATFORMIO
    #define RTC_LIB_H <TimeLib.h>
  #else
    #define RTC_LIB_H <Time.h>
  #endif
  void initialiseStorage(void);
  void initBoard(void);
  uint16_t freeRam(void);
  void doSystemReset(void);
  void jumpToBootloader(void);
  uint8_t getSystemTemp();

  #if defined(TIMER5_MICROS)
    /*#define micros() (((timer5_overflow_count << 16) + TCNT5) * 4) */ //Fast version of micros() that uses the 4uS tick of timer5. See timers.ino for the overflow ISR of timer5
    #define millis() (ms_counter) //Replaces the standard millis() function with this macro. It is both faster and more accurate. See timers.ino for its counter increment.
    static inline unsigned long micros_safe(); //A version of micros() that is interrupt safe
  #else
    #define micros_safe() micros() //If the timer5 method is not used, the micros_safe() macro is simply an alias for the normal micros()
  #endif
  #define pinIsReserved(pin)  ( ((pin) == 0) ) //Forbidden pins like USB on other boards

  #define USE_SERIAL3
  //Mega 2561 MCU does not have a serial3 available. 
  #if defined(__AVR_ATmega2561__)
  // Our MISRA checker is confused by #ifndef :-(
  #undef USE_SERIAL3
  #endif

/*
***********************************************************************************************************
* Schedules
*/
  //Refer to svn.savannah.nongnu.org/viewvc/trunk/avr-libc/include/avr/iomxx0_1.h?root=avr-libc&view=markup
  #define FUEL1_COUNTER TCNT3
  #define FUEL2_COUNTER TCNT3
  #define FUEL3_COUNTER TCNT3
  #define FUEL4_COUNTER TCNT4
  #define FUEL5_COUNTER TCNT4
  #define FUEL6_COUNTER TCNT4 //Replaces ignition 4
  #define FUEL7_COUNTER TCNT5 //Replaces ignition 3
  #define FUEL8_COUNTER TCNT5 //Replaces ignition 2

  #define IGN1_COUNTER  TCNT5
  #define IGN2_COUNTER  TCNT5
  #define IGN3_COUNTER  TCNT5
  #define IGN4_COUNTER  TCNT4
  #define IGN5_COUNTER  TCNT4
  #define IGN6_COUNTER  TCNT4 //Replaces injector 4
  #define IGN7_COUNTER  TCNT3 //Replaces injector 3
  #define IGN8_COUNTER  TCNT3 //Replaces injector 2

  #define FUEL1_COMPARE OCR3A
  #define FUEL2_COMPARE OCR3B
  #define FUEL3_COMPARE OCR3C
  #define FUEL4_COMPARE OCR4B //Replaces ignition 6
  #define FUEL5_COMPARE OCR4C //Replaces ignition 5
  #define FUEL6_COMPARE OCR4A //Replaces ignition 4
  #define FUEL7_COMPARE OCR5C //Replaces ignition 3
  #define FUEL8_COMPARE OCR5B //Replaces ignition 2

  #define IGN1_COMPARE  OCR5A
  #define IGN2_COMPARE  OCR5B
  #define IGN3_COMPARE  OCR5C
  #define IGN4_COMPARE  OCR4A //Replaces injector 6
  #define IGN5_COMPARE  OCR4C //Replaces injector 5
  #define IGN6_COMPARE  OCR4B //Replaces injector 4
  #define IGN7_COMPARE  OCR3C //Replaces injector 3
  #define IGN8_COMPARE  OCR3B //Replaces injector 2

  //Note that the interrupt flag is reset BEFORE the interrupt is enabled
static inline void FUEL1_TIMER_ENABLE(void) { TIFR3 |= (1<<OCF3A) ; TIMSK3 |= (1 << OCIE3A); } //Turn on the A compare unit (ie turn on the interrupt)
static inline void FUEL2_TIMER_ENABLE(void) { TIFR3 |= (1<<OCF3B); TIMSK3 |= (1 << OCIE3B); } //Turn on the B compare unit (ie turn on the interrupt)
static inline void FUEL3_TIMER_ENABLE(void) { TIFR3 |= (1<<OCF3C); TIMSK3 |= (1 << OCIE3C); } //Turn on the C compare unit (ie turn on the interrupt)
static inline void FUEL4_TIMER_ENABLE(void) { TIFR4 |= (1<<OCF4B); TIMSK4 |= (1 << OCIE4B); } //Turn on the B compare unit (ie turn on the interrupt)
static inline void FUEL5_TIMER_ENABLE(void) { TIFR4 |= (1<<OCF4C); TIMSK4 |= (1 << OCIE4C); } //Turn on the C compare unit (ie turn on the interrupt)
static inline void FUEL6_TIMER_ENABLE(void) { TIFR4 |= (1<<OCF4A); TIMSK4 |= (1 << OCIE4A); } //Turn on the A compare unit (ie turn on the interrupt)
static inline void FUEL7_TIMER_ENABLE(void) { TIFR5 |= (1<<OCF5C); TIMSK5 |= (1 << OCIE5C); } //
static inline void FUEL8_TIMER_ENABLE(void) { TIFR5 |= (1<<OCF5B); TIMSK5 |= (1 << OCIE5B); } //

static inline void FUEL1_TIMER_DISABLE(void) { TIMSK3 &= ~(1 << OCIE3A); } // //Turn off this output compare unit
static inline void FUEL2_TIMER_DISABLE(void) { TIMSK3 &= ~(1 << OCIE3B); } // //Turn off this output compare unit
static inline void FUEL3_TIMER_DISABLE(void) { TIMSK3 &= ~(1 << OCIE3C); } // //Turn off this output compare unit
static inline void FUEL4_TIMER_DISABLE(void) { TIMSK4 &= ~(1 << OCIE4B); } ////Turn off this output compare unit
static inline void FUEL5_TIMER_DISABLE(void) { TIMSK4 &= ~(1 << OCIE4C); } // //
static inline void FUEL6_TIMER_DISABLE(void) { TIMSK4 &= ~(1 << OCIE4A); } // //
static inline void FUEL7_TIMER_DISABLE(void) { TIMSK5 &= ~(1 << OCIE5C); } // //
static inline void FUEL8_TIMER_DISABLE(void) { TIMSK5 &= ~(1 << OCIE5B); } //

  //These have the TIFR5 bits set to 1 to clear the interrupt flag. This prevents a false interrupt being called the first time the channel is enabled.
static inline void IGN1_TIMER_ENABLE(void) { TIFR5 |= (1<<OCF5A); TIMSK5 |= (1 << OCIE5A); } //Turn on the A compare unit (ie turn on the interrupt)
static inline void IGN2_TIMER_ENABLE(void) { TIFR5 |= (1<<OCF5B); TIMSK5 |= (1 << OCIE5B); }//Turn on the B compare unit (ie turn on the interrupt)
static inline void IGN3_TIMER_ENABLE(void) { TIFR5 |= (1<<OCF5C); TIMSK5 |= (1 << OCIE5C); }//Turn on the C compare unit (ie turn on the interrupt)
static inline void IGN4_TIMER_ENABLE(void) { TIFR4 |= (1<<OCF4A); TIMSK4 |= (1 << OCIE4A); }//Turn on the A compare unit (ie turn on the interrupt)
static inline void IGN5_TIMER_ENABLE(void) { TIFR4 |= (1<<OCF4C); TIMSK4 |= (1 << OCIE4C); } //Turn on the A compare unit (ie turn on the interrupt)
static inline void IGN6_TIMER_ENABLE(void) { TIFR4 |= (1<<OCF4B); TIMSK4 |= (1 << OCIE4B); } //Replaces injector 4
static inline void IGN7_TIMER_ENABLE(void) { TIMSK3 |= (1 << OCIE3C); }//Replaces injector 3
static inline void IGN8_TIMER_ENABLE(void) { TIMSK3 |= (1 << OCIE3B); } //Replaces injector 2

static inline void IGN1_TIMER_DISABLE(void) { TIMSK5 &= ~(1 << OCIE5A); } //Turn off this output compare unit
static inline void IGN2_TIMER_DISABLE(void) { TIMSK5 &= ~(1 << OCIE5B); } //Turn off this output compare unit
static inline void IGN3_TIMER_DISABLE(void) { TIMSK5 &= ~(1 << OCIE5C); } //Turn off this output compare unit
static inline void IGN4_TIMER_DISABLE(void) { TIMSK4 &= ~(1 << OCIE4A); } //Turn off this output compare unit
static inline void IGN5_TIMER_DISABLE(void) { TIMSK4 &= ~(1 << OCIE4C); } //Turn off this output compare unit
static inline void IGN6_TIMER_DISABLE(void) { TIMSK4 &= ~(1 << OCIE4B); } //Replaces injector 4
static inline void IGN7_TIMER_DISABLE(void) { TIMSK3 &= ~(1 << OCIE3C); } //Replaces injector 3
static inline void IGN8_TIMER_DISABLE(void) { TIMSK3 &= ~(1 << OCIE3B); } //Replaces injector 2

  #define MAX_TIMER_PERIOD 262140UL //The longest period of time (in uS) that the timer can permit (IN this case it is 65535 * 4, as each timer tick is 4uS)
  #define uS_TO_TIMER_COMPARE(uS1) ((uS1) >> 2) //Converts a given number of uS into the required number of timer ticks until that time has passed

/*
***********************************************************************************************************
* Auxiliaries
*/
  #define ENABLE_BOOST_TIMER()  TIMSK1 |= (1 << OCIE1A)
  #define DISABLE_BOOST_TIMER() TIMSK1 &= ~(1 << OCIE1A)
  #define ENABLE_VVT_TIMER()    TIMSK1 |= (1 << OCIE1B)
  #define DISABLE_VVT_TIMER()   TIMSK1 &= ~(1 << OCIE1B)

  #define BOOST_TIMER_COMPARE   OCR1A
  #define BOOST_TIMER_COUNTER   TCNT1
  #define VVT_TIMER_COMPARE     OCR1B
  #define VVT_TIMER_COUNTER     TCNT1

/*
***********************************************************************************************************
* Idle
*/
  #define IDLE_COUNTER TCNT1
  #define IDLE_COMPARE OCR1C

  #define IDLE_TIMER_ENABLE() TIMSK1 |= (1 << OCIE1C)
  #define IDLE_TIMER_DISABLE() TIMSK1 &= ~(1 << OCIE1C)

/*
***********************************************************************************************************
* CAN / Second serial
*/
#define SECONDARY_SERIAL_T HardwareSerial

#endif //CORE_AVR
#endif //AVR2560_H
