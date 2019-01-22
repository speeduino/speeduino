
#if defined(CORE_AVR)
#include "globals.h"
#include "auxiliaries.h"


void initBoard()
{
    /*
    ***********************************************************************************************************
    * General
    */
    configPage9.intcan_available = 0;   // AVR devices do NOT have internal canbus

    /*
    ***********************************************************************************************************
    * Auxilliaries
    */
    //PWM used by the Boost and VVT outputs
    TCCR1B = 0x00;          //Disbale Timer1 while we set it up
    TCNT1  = 0;             //Reset Timer Count
    TIFR1  = 0x00;          //Timer1 INT Flag Reg: Clear Timer Overflow Flag
    TCCR1A = 0x00;          //Timer1 Control Reg A: Wave Gen Mode normal (Simply counts up from 0 to 65535 (16-bit int)
    TCCR1B = (1 << CS12);   //Timer1 Control Reg B: Timer Prescaler set to 256. 1 tick = 16uS. Refer to http://www.instructables.com/files/orig/F3T/TIKL/H3WSA4V7/F3TTIKLH3WSA4V7.jpg

    boost_pwm_max_count = 1000000L / (16 * configPage6.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (16 * configPage6.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle

}

uint16_t freeRam()
{
    extern int __heap_start, *__brkval;
    int currentVal;
    uint16_t v;

    if(__brkval == 0) { currentVal = (int) &__heap_start; }
    else { currentVal = (int) __brkval; }

    //return (uint16_t) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); // cppcheck-suppress misra-c2012-12.1
    return (uint16_t) &v - currentVal; //cppcheck-suppress misra-c2012-11.4
}

#if defined(TIMER5_MICROS)
//This is used by the fast version of micros(). We just need to increment the timer overflow counter
ISR(TIMER5_OVF_vect)
{
    ++timer5_overflow_count;
}

static inline unsigned long micros_safe()
{
  unsigned long newMicros;
  noInterrupts();
  newMicros = (((timer5_overflow_count << 16) + TCNT5) * 4);
  interrupts();

  return newMicros;
} 
#endif

#endif