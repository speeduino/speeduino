

/*
NOTE - This file and it's associated functions need a CLEARER NAME

//Purpose
We're implementing a lower frequency interrupt loop to perform calculations that are needed less often, some of which depend on time having passed (delta/time) to be meaningful.


//Technical
Timer2 is only 8bit so we are setting the prescaler to 128 to get the most out of it. This means that the counter increments every 0.008ms and the overflow at 256 will be after 2.048ms
Max Period = (Prescale)*(1/Frequency)*(2^8)
(See http://arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html)

We're after a 1ms interval so we'll need 131 intervals to reach this ( 1ms / 0.008ms per tick = 125).
Hence we will preload the timer with 131 cycles to leave 125 until overflow (1ms).

*/
#ifndef TIMERS_H
#define TIMERS_H

volatile byte loop33ms;
volatile byte loop66ms;
volatile byte loop100ms;
volatile byte loop250ms;
volatile int loopSec;

volatile unsigned int dwellLimit_uS;
volatile uint16_t lastRPM_100ms; //Need to record this for rpmDOT calculation
volatile uint16_t last250msLoopCount = 1000; //Set to effectively random number on startup. Just need this to be different to what mainLoopCount equals initially (Probably 0)

#if defined(TIMER5_MICROS)
  #define micros() (((timer5_overflow_count << 16) + TCNT5) * 4) //Fast version of micros() that uses the 4uS tick of timer5. See timers.ino for the overflow ISR of timer5
  #define millis() (ms_counter) //Replaces the standard millis() function with this macro. It is both faster and more accurate. See timers.ino for its counter increment.
  static inline unsigned long micros_safe(); //A version of micros() that is interrupt safe
#else
  #define micros_safe() micros() //If the timer5 method is not used, the micros_safe() macro is simply an alias for the normal micros()
#endif

#if defined (CORE_TEENSY)
  IntervalTimer lowResTimer;
  void oneMSInterval();
#elif defined(CORE_STM32)
  void oneMSInterval();
#endif
void initialiseTimers();

#endif // TIMERS_H
