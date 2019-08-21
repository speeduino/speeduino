#ifndef TEENSY40_H
#define TEENSY40_H
#if defined(CORE_TEENSY)&& defined(__IMXRT1062__)

/*
***********************************************************************************************************
* General
*/
  void initBoard();
  uint16_t freeRam();
  #define PORT_TYPE uint32_t //Size of the port variables
  #define PINMASK_TYPE uint32_t
  #define COMPARE_TYPE uint32_t
  #define COUNTER_TYPE uint32_t
  #define BOARD_DIGITAL_GPIO_PINS 34
  #define BOARD_NR_GPIO_PINS 34
  #define USE_SERIAL3
  #define EEPROM_LIB_H <EEPROM.h>

  #define micros_safe() micros() //timer5 method is not used on anything but AVR, the micros_safe() macro is simply an alias for the normal micros()

/*
***********************************************************************************************************
* Schedules
*/
  //http://shawnhymel.com/661/learning-the-teensy-lc-interrupt-service-routines/
  #define FUEL1_COUNTER TMR1_CNTR0
  #define FUEL2_COUNTER TMR1_CNTR0
  #define FUEL3_COUNTER TMR1_CNTR1
  #define FUEL4_COUNTER TMR1_CNTR1
  #define FUEL5_COUNTER TMR1_CNTR2
  #define FUEL6_COUNTER TMR1_CNTR2
  #define FUEL7_COUNTER TMR1_CNTR3
  #define FUEL8_COUNTER TMR1_CNTR3

  #define IGN1_COUNTER  TMR2_CNTR0
  #define IGN2_COUNTER  TMR2_CNTR0
  #define IGN3_COUNTER  TMR2_CNTR1
  #define IGN4_COUNTER  TMR2_CNTR1
  #define IGN5_COUNTER  TMR2_CNTR2
  #define IGN6_COUNTER  TMR2_CNTR2
  #define IGN7_COUNTER  TMR2_CNTR3
  #define IGN8_COUNTER  TMR2_CNTR3

  #define FUEL1_COMPARE TMR1_COMP10
  #define FUEL2_COMPARE TMR1_COMP20
  #define FUEL3_COMPARE TMR1_COMP11
  #define FUEL4_COMPARE TMR1_COMP21
  #define FUEL5_COMPARE TMR1_COMP12
  #define FUEL6_COMPARE TMR1_COMP22
  #define FUEL7_COMPARE TMR1_COMP13
  #define FUEL8_COMPARE TMR1_COMP23

  #define IGN1_COMPARE  TMR2_COMP10
  #define IGN2_COMPARE  TMR2_COMP20
  #define IGN3_COMPARE  TMR2_COMP11
  #define IGN4_COMPARE  TMR2_COMP21
  #define IGN5_COMPARE  TMR2_COMP12
  #define IGN6_COMPARE  TMR2_COMP22
  #define IGN7_COMPARE  TMR2_COMP13
  #define IGN8_COMPARE  TMR2_COMP23

  #define FUEL1_TIMER_ENABLE() TMR1_CSCTRL0 |= TMR_CSCTRL_TCF1EN //Write 1 to the TCFIE (Channel Interrupt Enable) bit of channel 0 Status/Control
  #define FUEL2_TIMER_ENABLE() TMR1_CSCTRL0 |= TMR_CSCTRL_TCF2EN
  #define FUEL3_TIMER_ENABLE() TMR1_CSCTRL1 |= TMR_CSCTRL_TCF1EN
  #define FUEL4_TIMER_ENABLE() TMR1_CSCTRL1 |= TMR_CSCTRL_TCF2EN
  #define FUEL5_TIMER_ENABLE() TMR1_CSCTRL2 |= TMR_CSCTRL_TCF1EN
  #define FUEL6_TIMER_ENABLE() TMR1_CSCTRL2 |= TMR_CSCTRL_TCF2EN
  #define FUEL7_TIMER_ENABLE() TMR1_CSCTRL3 |= TMR_CSCTRL_TCF1EN
  #define FUEL8_TIMER_ENABLE() TMR1_CSCTRL3 |= TMR_CSCTRL_TCF2EN

  #define FUEL1_TIMER_DISABLE() TMR1_CSCTRL0 &= ~TMR_CSCTRL_TCF1EN //Write 0 to the CHIE (Channel Interrupt Enable) bit of channel 0 Status/Control
  #define FUEL2_TIMER_DISABLE() TMR1_CSCTRL0 &= ~TMR_CSCTRL_TCF2EN
  #define FUEL3_TIMER_DISABLE() TMR1_CSCTRL1 &= ~TMR_CSCTRL_TCF1EN
  #define FUEL4_TIMER_DISABLE() TMR1_CSCTRL1 &= ~TMR_CSCTRL_TCF2EN
  #define FUEL5_TIMER_DISABLE() TMR1_CSCTRL2 &= ~TMR_CSCTRL_TCF1EN
  #define FUEL6_TIMER_DISABLE() TMR1_CSCTRL2 &= ~TMR_CSCTRL_TCF2EN
  #define FUEL7_TIMER_DISABLE() TMR1_CSCTRL3 &= ~TMR_CSCTRL_TCF1EN
  #define FUEL8_TIMER_DISABLE() TMR1_CSCTRL3 &= ~TMR_CSCTRL_TCF2EN

  #define IGN1_TIMER_ENABLE() TMR2_CSCTRL0 |= TMR_CSCTRL_TCF1EN //Write 1 to the TCFIE (Channel Interrupt Enable) bit of channel 0 Status/Control
  #define IGN2_TIMER_ENABLE() TMR2_CSCTRL0 |= TMR_CSCTRL_TCF2EN
  #define IGN3_TIMER_ENABLE() TMR2_CSCTRL1 |= TMR_CSCTRL_TCF1EN
  #define IGN4_TIMER_ENABLE() TMR2_CSCTRL1 |= TMR_CSCTRL_TCF2EN
  #define IGN5_TIMER_ENABLE() TMR2_CSCTRL2 |= TMR_CSCTRL_TCF1EN
  #define IGN6_TIMER_ENABLE() TMR2_CSCTRL2 |= TMR_CSCTRL_TCF2EN
  #define IGN7_TIMER_ENABLE() TMR2_CSCTRL3 |= TMR_CSCTRL_TCF1EN
  #define IGN8_TIMER_ENABLE() TMR2_CSCTRL3 |= TMR_CSCTRL_TCF2EN

  #define IGN1_TIMER_DISABLE() TMR2_CSCTRL0 &= ~TMR_CSCTRL_TCF1EN //Write 0 to the CHIE (Channel Interrupt Enable) bit of channel 0 Status/Control
  #define IGN2_TIMER_DISABLE() TMR2_CSCTRL0 &= ~TMR_CSCTRL_TCF2EN
  #define IGN3_TIMER_DISABLE() TMR2_CSCTRL1 &= ~TMR_CSCTRL_TCF1EN
  #define IGN4_TIMER_DISABLE() TMR2_CSCTRL1 &= ~TMR_CSCTRL_TCF2EN
  #define IGN5_TIMER_DISABLE() TMR2_CSCTRL2 &= ~TMR_CSCTRL_TCF1EN
  #define IGN6_TIMER_DISABLE() TMR2_CSCTRL2 &= ~TMR_CSCTRL_TCF2EN
  #define IGN7_TIMER_DISABLE() TMR2_CSCTRL3 &= ~TMR_CSCTRL_TCF1EN
  #define IGN8_TIMER_DISABLE() TMR2_CSCTRL3 &= ~TMR_CSCTRL_TCF2EN

  #define MAX_TIMER_PERIOD 139808 // 2.13333333uS * 65535
  #define MAX_TIMER_PERIOD_SLOW 139808
  #define uS_TO_TIMER_COMPARE(uS) ((uS * 15) >> 5) //Converts a given number of uS into the required number of timer ticks until that time has passed.
  //Hack compatibility with AVR timers that run at different speeds
  #define uS_TO_TIMER_COMPARE_SLOW(uS) ((uS * 15) >> 5)

/*
***********************************************************************************************************
* Auxilliaries
*/
  #define ENABLE_BOOST_TIMER()  TMR3_CSCTRL0 |= TMR_CSCTRL_TCF1EN
  #define DISABLE_BOOST_TIMER() TMR3_CSCTRL0 &= ~TMR_CSCTRL_TCF1EN

  #define ENABLE_VVT_TIMER()    TMR3_CSCTRL0 |= TMR_CSCTRL_TCF2EN
  #define DISABLE_VVT_TIMER()   TMR3_CSCTRL0 &= ~TMR_CSCTRL_TCF2EN

  #define BOOST_TIMER_COMPARE   TMR3_COMP10
  #define BOOST_TIMER_COUNTER   TMR3_CNTR0
  #define VVT_TIMER_COMPARE     TMR3_COMP20
  #define VVT_TIMER_COUNTER     TMR3_CNTR0

  static inline void boostInterrupt();
  static inline void vvtInterrupt();

/*
***********************************************************************************************************
* Idle
*/
  #define IDLE_COUNTER TMR3_CNTR1
  #define IDLE_COMPARE TMR3_COMP21

  #define IDLE_TIMER_ENABLE() TMR3_CSCTRL1 |= TMR_CSCTRL_TCF1EN
  #define IDLE_TIMER_DISABLE() TMR3_CSCTRL1 &= ~TMR_CSCTRL_TCF1EN

  static inline void idleInterrupt();

/*
***********************************************************************************************************
* CAN / Second serial
*/
  //Uart CANSerial (&sercom3, 0, 1, SERCOM_RX_PAD_1, UART_TX_PAD_0);

#endif //CORE_TEENSY
#endif //TEENSY40_H