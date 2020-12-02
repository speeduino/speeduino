#if defined(CORE_TEENSY) && defined(__IMXRT1062__)
#include "board_teensy41.h"
#include "globals.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"


void initBoard()
{
    /*
    ***********************************************************************************************************
    * General
    */

   //TODO: Configure timers here

    /*
    ***********************************************************************************************************
    * Idle
    */
    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) )
    {
        //TODO: Configure timers here
    }

    /*
    ***********************************************************************************************************
    * Timers
    */
    //Uses the PIT timer on Teensy.
    lowResTimer.begin(oneMSInterval, 1000);

    //TODO: Configure timers here

    /*
    ***********************************************************************************************************
    * Auxilliaries
    */

    //2uS resolution Min 8Hz, Max 5KHz
    boost_pwm_max_count = 1000000L / (2 * configPage6.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (2 * configPage6.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle

    //TODO: Configure timers here

    /*
    ***********************************************************************************************************
    * Schedules
    */

    //TODO: Configure timers here
}

uint16_t freeRam()
{
    uint32_t stackTop;
    uint32_t heapTop;

    // current position of the stack.
    stackTop = (uint32_t) &stackTop;

    // current position of heap.
    void* hTop = malloc(1);
    heapTop = (uint32_t) hTop;
    free(hTop);

    // The difference is the free, available ram.
    return (uint16_t)stackTop - heapTop;
}

void doSystemReset() { return; }
void jumpToBootloader() { return; }

#endif