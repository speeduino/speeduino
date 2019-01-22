
#include "globals.h"
#if defined(CORE_AVR)

void initBoard()
{
    /*
    ***********************************************************************************************************
    * General
    */
    configPage9.intcan_available = 0;   // AVR devices do NOT have internal canbus
}

uint16_t freeRam()
{
    extern int __heap_start, *__brkval;
    uint16_t v;

    return (uint16_t) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
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