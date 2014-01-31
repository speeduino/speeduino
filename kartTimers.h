

/*
NOTE - This file and it's associated functions need a CLEARER NAME

//Purpose
We're implementing a lower frequency interrupt loop to perform calculations that are needed less often, some of which depend on time having passed (delta/time) to be meaningful. 


//Technical
Timer2 is only 8bit so we are setting the prescaler to 1024 to get the most out of it. This means that the counter increments every 0.064ms and the overflow at 256 will be after ~16ms
Max Period = (Prescale)*(1/Frequency)*(2^8)
(See http://arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html)

We're looking for a 10ms interval so we'll need 156.25 intervals to reach this ( 10ms / 0.064ms per tick = 156.25). 
Hence we will preload the timer with 99 cycles to leave 156 until overflow (~10ms).

*/

volatile int loopGen;
volatile int loopSec;


void initialiseTimers();


