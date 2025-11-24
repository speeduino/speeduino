//
// Created by Ognjen Galić on 23. 11. 2025..
//

#ifndef FIRMWARE_BOARD_X86_H
#define FIRMWARE_BOARD_X86_H

// Here to aid in CLion header detection
#if defined(PLATFORM_X86) || defined(__x86_64__)

/*
 * Board-x86 stuff
 */

#include "Stream.h"
#include "HardwareSerial.h"
#include "SoftwareTimer.h"
#include "log.h"

#define BLOCKING_FACTOR       121
#define TABLE_BLOCKING_FACTOR 64

#define COMPARE_TYPE uint32_t

extern COMPARE_TYPE dummy_register;

#define SECONDARY_SERIAL_T HardwareSerial

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

#ifndef INJ_CHANNELS
#define INJ_CHANNELS 8
#endif
#ifndef IGN_CHANNELS
#define IGN_CHANNELS 8
#endif

/*
 * Timers
 */
extern SoftwareTimer Fuel1;
extern SoftwareTimer Fuel2;
extern SoftwareTimer Fuel3;
extern SoftwareTimer Fuel4;
extern SoftwareTimer Fuel5;
extern SoftwareTimer Fuel6;
extern SoftwareTimer Fuel7;
extern SoftwareTimer Fuel8;

extern SoftwareTimer Ignition1;
extern SoftwareTimer Ignition2;
extern SoftwareTimer Ignition3;
extern SoftwareTimer Ignition4;
extern SoftwareTimer Ignition5;
extern SoftwareTimer Ignition6;
extern SoftwareTimer Ignition7;
extern SoftwareTimer Ignition8;

/*
***********************************************************************************************************
* General
*/
  #define SERIAL_BUFFER_SIZE 517 //Size of the serial buffer used by new comms protocol. For SD transfers this must be at least 512 + 1 (flag) + 4 (sector)
  #define FPU_MAX_SIZE 0 //Size of the FPU buffer. 0 means no FPU.
  #define BOARD_MAX_IO_PINS  52 //digital pins + analog channels + 1
  #define BOARD_MAX_DIGITAL_PINS 52 //Pretty sure this isn't right
  #define EEPROM_LIB_H <EEPROM.h> //The name of the file that provides the EEPROM class
  typedef int eeprom_address_t;

  #define pinIsReserved(pin)  ( ((pin) == 0) ) //Forbidden pins like USB

/*
***********************************************************************************************************
* Schedules
*/

  #define FUEL1_COUNTER dummy_register
  #define FUEL2_COUNTER dummy_register
  #define FUEL3_COUNTER dummy_register
  #define FUEL4_COUNTER dummy_register
  //The below are optional, but recommended if there are sufficient timers/compares
  #define FUEL5_COUNTER dummy_register
  #define FUEL6_COUNTER dummy_register
  #define FUEL7_COUNTER dummy_register
  #define FUEL8_COUNTER dummy_register

  #define IGN1_COUNTER  dummy_register
  #define IGN2_COUNTER  dummy_register
  #define IGN3_COUNTER  dummy_register
  #define IGN4_COUNTER  dummy_register
  //The below are optional, but recommended if there are sufficient timers/compares
  #define IGN5_COUNTER  dummy_register
  #define IGN6_COUNTER  dummy_register
  #define IGN7_COUNTER  dummy_register
  #define IGN8_COUNTER  dummy_register

  #define FUEL1_COMPARE dummy_register
  #define FUEL2_COMPARE dummy_register
  #define FUEL3_COMPARE dummy_register
  #define FUEL4_COMPARE dummy_register
  //The below are optional, but recommended if there are sufficient timers/compares
  #define FUEL5_COMPARE dummy_register
  #define FUEL6_COMPARE dummy_register
  #define FUEL7_COMPARE dummy_register
  #define FUEL8_COMPARE dummy_register

  #define IGN1_COMPARE  dummy_register
  #define IGN2_COMPARE  dummy_register
  #define IGN3_COMPARE  dummy_register
  #define IGN4_COMPARE  dummy_register
  //The below are optional, but recommended if there are sufficient timers/compares
  #define IGN5_COMPARE  dummy_register
  #define IGN6_COMPARE  dummy_register
  #define IGN7_COMPARE  dummy_register
  #define IGN8_COMPARE  dummy_register

  static inline void FUEL1_TIMER_ENABLE(void)  { log(TIMER, "FUEL1_TIMER_ENABLE\n"); }
  static inline void FUEL2_TIMER_ENABLE(void)  {log(TIMER, "FUEL2_TIMER_ENABLE\n");}
  static inline void FUEL3_TIMER_ENABLE(void)  {log(TIMER, "FUEL3_TIMER_ENABLE\n");}
  static inline void FUEL4_TIMER_ENABLE(void)  {log(TIMER, "FUEL4_TIMER_ENABLE\n");}
  //The below are optional, but recommended if there are sufficient timers/compares
  static inline void FUEL5_TIMER_ENABLE(void)  {log(TIMER, "FUEL5_TIMER_ENABLE\n");}
  static inline void FUEL6_TIMER_ENABLE(void)  {log(TIMER, "FUEL6_TIMER_ENABLE\n");}
  static inline void FUEL7_TIMER_ENABLE(void)  {log(TIMER, "FUEL7_TIMER_ENABLE\n");}
  static inline void FUEL8_TIMER_ENABLE(void)  {log(TIMER, "FUEL8_TIMER_ENABLE\n");}

  static inline void FUEL1_TIMER_DISABLE(void)  { log(TIMER, "FUEL1_TIMER_DISABLE\n");}
  static inline void FUEL2_TIMER_DISABLE(void)  { log(TIMER, "FUEL2_TIMER_DISABLE\n");}
  static inline void FUEL3_TIMER_DISABLE(void)  { log(TIMER, "FUEL3_TIMER_DISABLE\n");}
  static inline void FUEL4_TIMER_DISABLE(void)  { log(TIMER, "FUEL4_TIMER_DISABLE\n");}
  //The below are optional, but recommended if there are sufficient timers/compares
  static inline void FUEL5_TIMER_DISABLE(void)  { log(TIMER, "FUEL5_TIMER_DISABLE\n");}
  static inline void FUEL6_TIMER_DISABLE(void)  { log(TIMER, "FUEL6_TIMER_DISABLE\n");}
  static inline void FUEL7_TIMER_DISABLE(void)  { log(TIMER, "FUEL7_TIMER_DISABLE\n");}
  static inline void FUEL8_TIMER_DISABLE(void)  { log(TIMER, "FUEL8_TIMER_DISABLE\n");}

    static inline void IGN1_TIMER_ENABLE(void)  {log(TIMER, "IGN1_TIMER_ENABLE\n");}
    static inline void IGN2_TIMER_ENABLE(void)  {log(TIMER, "IGN2_TIMER_ENABLE\n");}
    static inline void IGN3_TIMER_ENABLE(void)  {log(TIMER, "IGN3_TIMER_ENABLE\n");}
    static inline void IGN4_TIMER_ENABLE(void)  {log(TIMER, "IGN4_TIMER_ENABLE\n");}

    static inline void IGN5_TIMER_ENABLE(void)  {log(TIMER, "IGN5_TIMER_ENABLE\n");}
    static inline void IGN6_TIMER_ENABLE(void)  {log(TIMER, "IGN6_TIMER_ENABLE\n");}
    static inline void IGN7_TIMER_ENABLE(void)  {log(TIMER, "IGN7_TIMER_ENABLE\n");}
    static inline void IGN8_TIMER_ENABLE(void)  {log(TIMER, "IGN8_TIMER_ENABLE\n");}

    static inline void IGN1_TIMER_DISABLE(void)  {log(TIMER, "IGN1_TIMER_DISABLE\n");}
    static inline void IGN2_TIMER_DISABLE(void)  {log(TIMER, "IGN2_TIMER_DISABLE\n");}
    static inline void IGN3_TIMER_DISABLE(void)  {log(TIMER, "IGN3_TIMER_DISABLE\n");}
    static inline void IGN4_TIMER_DISABLE(void)  {log(TIMER, "IGN4_TIMER_DISABLE\n");}

    static inline void IGN5_TIMER_DISABLE(void)  {log(TIMER, "IGN5_TIMER_DISABLE\n");}
    static inline void IGN6_TIMER_DISABLE(void)  {log(TIMER, "IGN6_TIMER_DISABLE\n");}
    static inline void IGN7_TIMER_DISABLE(void)  {log(TIMER, "IGN7_TIMER_DISABLE\n");}
    static inline void IGN8_TIMER_DISABLE(void)  {log(TIMER, "IGN8_TIMER_DISABLE\n");}


  #define MAX_TIMER_PERIOD 139808 //This is the maximum time, in uS, that the compare channels can run before overflowing. It is typically 65535 * <how long each tick represents>
  #define uS_TO_TIMER_COMPARE(uS) ((uS * 15) >> 5) //Converts a given number of uS into the required number of timer ticks until that time has passed.

/*
***********************************************************************************************************
* Auxiliaries
*/
  //macro functions for enabling and disabling timer interrupts for the boost and vvt functions
  #define ENABLE_BOOST_TIMER()  log(TIMER, "%s:%d ENABLE_BOOST_TIMER\n", __FILE__, __LINE__);
  #define DISABLE_BOOST_TIMER(void)  log(TIMER, "%s:%d DISABLE_BOOST_TIMER\n", __FILE__, __LINE__);

  #define ENABLE_VVT_TIMER()    log(TIMER, "%s:%dENABLE_VVT_TIMER\n", __FILE__, __LINE__);
  #define DISABLE_VVT_TIMER()   log(TIMER, "%s:%dDISABLE_VVT_TIMER\n", __FILE__, __LINE__);

  #define BOOST_TIMER_COMPARE   dummy_register
  #define BOOST_TIMER_COUNTER   dummy_register
  #define VVT_TIMER_COMPARE     dummy_register
  #define VVT_TIMER_COUNTER     dummy_register

/*
***********************************************************************************************************
* Idle
*/
  //Same as above, but for the timer controlling PWM idle
  #define IDLE_COUNTER          dummy_register
  #define IDLE_COMPARE          dummy_register

  #define IDLE_TIMER_ENABLE()   log(TIMER, "IDLE_TIMER_ENABLE\n");
  #define IDLE_TIMER_DISABLE()  log(TIMER, "IDLE_TIMER_DISABLE\n");

/*
***********************************************************************************************************
* CAN / Second serial
*/



/*
 * Other stuff
 */
#define PORT_TYPE X86Port*
#define PINMASK_TYPE uint64_t

extern void *memcpy_P(void *, const void *, size_t);
#define RTC_LIB_H "stdio.h"

extern void tickTimersX86(uint64_t time);

#endif
#endif //FIRMWARE_BOARD_X86_H