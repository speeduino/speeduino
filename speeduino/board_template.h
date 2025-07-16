#ifndef TEMPLATE_H
#define TEMPLATE_H
#if defined(CORE_TEMPLATE)

/*
***********************************************************************************************************
* General
*/
  #define PORT_TYPE uint32_t //Size of the port variables (Eg inj1_pin_port). Most systems use a byte, but SAMD21 and possibly others are a 32-bit unsigned int
  #define PINMASK_TYPE uint32_t
  #define SERIAL_BUFFER_SIZE 517 //Size of the serial buffer used by new comms protocol. For SD transfers this must be at least 512 + 1 (flag) + 4 (sector)
  #define FPU_MAX_SIZE 0 //Size of the FPU buffer. 0 means no FPU.
  #define BOARD_MAX_IO_PINS  52 //digital pins + analog channels + 1
  #define BOARD_MAX_DIGITAL_PINS 52 //Pretty sure this isn't right
  #define EEPROM_LIB_H <EEPROM.h> //The name of the file that provides the EEPROM class
  typedef int eeprom_address_t;
  #define micros_safe() micros() //timer5 method is not used on anything but AVR, the micros_safe() macro is simply an alias for the normal micros()
  void initBoard();
  uint16_t freeRam();
  void doSystemReset();
  void jumpToBootloader();

  #define pinIsReserved(pin)  ( ((pin) == 0) ) //Forbidden pins like USB

/*
***********************************************************************************************************
* Schedules
*/
  
  #define FUEL1_COUNTER <register here>
  #define FUEL2_COUNTER <register here>
  #define FUEL3_COUNTER <register here>
  #define FUEL4_COUNTER <register here>
  //The below are optional, but recommended if there are sufficient timers/compares
  #define FUEL5_COUNTER <register here>
  #define FUEL6_COUNTER <register here>
  #define FUEL7_COUNTER <register here>
  #define FUEL8_COUNTER <register here>

  #define IGN1_COUNTER  <register here>
  #define IGN2_COUNTER  <register here>
  #define IGN3_COUNTER  <register here>
  #define IGN4_COUNTER  <register here>
  //The below are optional, but recommended if there are sufficient timers/compares
  #define IGN5_COUNTER  <register here>
  #define IGN6_COUNTER  <register here>
  #define IGN7_COUNTER  <register here>
  #define IGN8_COUNTER  <register here>

  #define FUEL1_COMPARE <register here>
  #define FUEL2_COMPARE <register here>
  #define FUEL3_COMPARE <register here>
  #define FUEL4_COMPARE <register here>
  //The below are optional, but recommended if there are sufficient timers/compares
  #define FUEL5_COMPARE <register here>
  #define FUEL6_COMPARE <register here>
  #define FUEL7_COMPARE <register here>
  #define FUEL8_COMPARE <register here>

  #define IGN1_COMPARE  <register here>
  #define IGN2_COMPARE  <register here>
  #define IGN3_COMPARE  <register here>
  #define IGN4_COMPARE  <register here>
  //The below are optional, but recommended if there are sufficient timers/compares
  #define IGN5_COMPARE  <register here>
  #define IGN6_COMPARE  <register here>
  #define IGN7_COMPARE  <register here>
  #define IGN8_COMPARE  <register here>

  static inline void FUEL1_TIMER_ENABLE(void)  {<macro here>;}
  static inline void FUEL2_TIMER_ENABLE(void)  {<macro here>;}
  static inline void FUEL3_TIMER_ENABLE(void)  {<macro here>;}
  static inline void FUEL4_TIMER_ENABLE(void)  {<macro here>;}
  //The below are optional, but recommended if there are sufficient timers/compares
  static inline void FUEL5_TIMER_ENABLE(void)  {<macro here>;}
  static inline void FUEL6_TIMER_ENABLE(void)  {<macro here>;}
  static inline void FUEL7_TIMER_ENABLE(void)  {<macro here>;}
  static inline void FUEL8_TIMER_ENABLE(void)  {<macro here>;}

  static inline void FUEL1_TIMER_DISABLE(void)  { <macro here>;}
  static inline void FUEL2_TIMER_DISABLE(void)  { <macro here>;}
  static inline void FUEL3_TIMER_DISABLE(void)  { <macro here>;}
  static inline void FUEL4_TIMER_DISABLE(void)  { <macro here>;}
  //The below are optional, but recommended if there are sufficient timers/compares
  static inline void FUEL5_TIMER_DISABLE(void)  { <macro here>;}
  static inline void FUEL6_TIMER_DISABLE(void)  { <macro here>;}
  static inline void FUEL7_TIMER_DISABLE(void)  { <macro here>;}
  static inline void FUEL8_TIMER_DISABLE(void)  { <macro here>;}

    static inline void IGN1_TIMER_ENABLE(void)  {<macro here>;}
    static inline void IGN2_TIMER_ENABLE(void)  {<macro here>;}
    static inline void IGN3_TIMER_ENABLE(void)  {<macro here>;}
    static inline void IGN4_TIMER_ENABLE(void)  {<macro here>;}
  //The below are optional, but recommended if there are sufficient timers/compares
    static inline void IGN5_TIMER_ENABLE(void)  {<macro here>;}
    static inline void IGN6_TIMER_ENABLE(void)  {<macro here>;}
    static inline void IGN7_TIMER_ENABLE(void)  {<macro here>;}
    static inline void IGN8_TIMER_ENABLE(void)  {<macro here>;}

    static inline void IGN1_TIMER_DISABLE(void)  {<macro here>;}
    static inline void IGN2_TIMER_DISABLE(void)  {<macro here>;}
    static inline void IGN3_TIMER_DISABLE(void)  {<macro here>;}
    static inline void IGN4_TIMER_DISABLE(void)  {<macro here>;}
  //The below are optional, but recommended if there are suffici;}ent timers/compares
    static inline void IGN5_TIMER_DISABLE(void)  {<macro here>;}
    static inline void IGN6_TIMER_DISABLE(void)  {<macro here>;}
    static inline void IGN7_TIMER_DISABLE(void)  {<macro here>;}
    static inline void IGN8_TIMER_DISABLE(void)  {<macro here>;}

  
#define MAX_TIMER_PERIOD 139808 //This is the maximum time, in uS, that the compare channels can run before overflowing. It is typically 65535 * <how long each tick represents>

/** @brief Convert a time in microseconds to the equivalent number of timer ticks */
static inline constexpr COMPARE_TYPE convertMicroSecToTicks(uint32_t uS) {
  return (COMPARE_TYPE)((uS * 75UL) >> 6);
}

/*
***********************************************************************************************************
* Auxiliaries
*/
  //macro functions for enabling and disabling timer interrupts for the boost and vvt functions
  #define ENABLE_BOOST_TIMER()  <macro here>
  #define DISABLE_BOOST_TIMER(void)  { <macro here>

  #define ENABLE_VVT_TIMER()    <macro here>
  #define DISABLE_VVT_TIMER()   <macro here>

  #define BOOST_TIMER_COMPARE   <register here>
  #define BOOST_TIMER_COUNTER   <register here>
  #define VVT_TIMER_COMPARE     <register here>
  #define VVT_TIMER_COUNTER     <register here>

/*
***********************************************************************************************************
* Idle
*/
  //Same as above, but for the timer controlling PWM idle
  #define IDLE_COUNTER          <register here>
  #define IDLE_COMPARE          <register here>

  #define IDLE_TIMER_ENABLE()   <macro here>
  #define IDLE_TIMER_DISABLE()  <macro here>

/*
***********************************************************************************************************
* CAN / Second serial
*/


#endif //CORE_TEMPLATE
#endif //TEMPLATE_H
