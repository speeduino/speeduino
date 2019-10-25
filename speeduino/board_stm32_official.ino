#if defined(CORE_STM32_OFFICIAL)
#include "board_stm32_official.h"
#include "globals.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"
#include "HardwareTimer.h"




  void initBoard()
  {
    /*
    ***********************************************************************************************************
    * General
    */
    #ifndef FLASH_LENGTH
      #define FLASH_LENGTH 8192
    #endif
    delay(10);
    /*
    ***********************************************************************************************************
    * Idle
    */
    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) )
    {
        idle_pwm_max_count = 1000000L / (configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 5KHz
    } 

    //This must happen at the end of the idle init
    Timer1.setMode(4, TIMER_OUTPUT_COMPARE);
    //timer_set_mode(TIMER1, 4, TIMER_OUTPUT_COMPARE;
    if(idle_pwm_max_count > 0) { Timer1.attachInterrupt(4, idleInterrupt); } //on first flash the configPage4.iacAlgorithm is invalid


    /*
    ***********************************************************************************************************
    * Timers
    */
    #if defined(ARDUINO_BLACK_F407VE) || defined(STM32F4) || defined(_STM32F4_)
        Timer8.setOverflow(1000, MICROSEC_FORMAT);  // Set up period
        Timer8.setMode(1, TIMER_OUTPUT_COMPARE);
        Timer8.attachInterrupt(1, oneMSInterval);
        Timer8.resume(); //Start Timer
    #else
        Timer4.setOverflow(1000, MICROSEC_FORMAT);  // Set up period
        Timer4.setMode(1, TIMER_OUTPUT_COMPARE);
        Timer4.attachInterrupt(1, oneMSInterval);
        Timer4.resume(); //Start Timer
    #endif
    pinMode(LED_BUILTIN, OUTPUT); //Visual WDT

    /*
    ***********************************************************************************************************
    * Auxilliaries
    */
    //2uS resolution Min 8Hz, Max 5KHz
    boost_pwm_max_count = 1000000L / (2 * configPage6.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (2 * configPage6.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle

    //Need to be initialised last due to instant interrupt
    Timer1.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer1.setMode(3, TIMER_OUTPUT_COMPARE);
    if(boost_pwm_max_count > 0) { Timer1.attachInterrupt(2, boostInterrupt);}
    if(vvt_pwm_max_count > 0) { Timer1.attachInterrupt(3, vvtInterrupt);}

    /*
    ***********************************************************************************************************
    * Schedules
    */
    Timer1.setOverflow(MAX_TIMER_PERIOD, MICROSEC_FORMAT);
    Timer2.setOverflow(MAX_TIMER_PERIOD, MICROSEC_FORMAT);
    Timer3.setOverflow(MAX_TIMER_PERIOD, MICROSEC_FORMAT);


    Timer2.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(4, TIMER_OUTPUT_COMPARE);

    Timer3.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(4, TIMER_OUTPUT_COMPARE);
    Timer1.setMode(1, TIMER_OUTPUT_COMPARE);

    //Attach interupt functions
    //Injection
    Timer3.attachInterrupt(1, fuelSchedule1Interrupt);
    Timer3.attachInterrupt(2, fuelSchedule2Interrupt);
    Timer3.attachInterrupt(3, fuelSchedule3Interrupt);
    Timer3.attachInterrupt(4, fuelSchedule4Interrupt);
    #if (INJ_CHANNELS >= 5)
    Timer5.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer5.attachInterrupt(1, fuelSchedule5Interrupt);
    #endif
    #if (INJ_CHANNELS >= 6)
    Timer5.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer5.attachInterrupt(2, fuelSchedule6Interrupt);
    #endif
    #if (INJ_CHANNELS >= 7)
    Timer5.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer5.attachInterrupt(3, fuelSchedule7Interrupt);
    #endif
    #if (INJ_CHANNELS >= 8)
    Timer5.setMode(4, TIMER_OUTPUT_COMPARE);
    Timer5.attachInterrupt(4, fuelSchedule8Interrupt);
    #endif

    //Ignition
    Timer2.attachInterrupt(1, ignitionSchedule1Interrupt); 
    Timer2.attachInterrupt(2, ignitionSchedule2Interrupt);
    Timer2.attachInterrupt(3, ignitionSchedule3Interrupt);
    Timer2.attachInterrupt(4, ignitionSchedule4Interrupt);
    #if (IGN_CHANNELS >= 5)
    Timer4.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer4.attachInterrupt(1, ignitionSchedule5Interrupt);
    #endif
    #if (IGN_CHANNELS >= 6)
    Timer4.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer4.attachInterrupt(2, ignitionSchedule6Interrupt);
    #endif
    #if (IGN_CHANNELS >= 7)
    Timer4.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer4.attachInterrupt(3, ignitionSchedule7Interrupt);
    #endif
    #if (IGN_CHANNELS >= 8)
    Timer4.setMode(4, TIMER_OUTPUT_COMPARE);
    Timer4.attachInterrupt(4, ignitionSchedule8Interrupt);
    #endif

    Timer1.resume();
    Timer2.resume();
    Timer3.resume();
    #if (IGN_CHANNELS >= 5)
    Timer4.resume();
    #endif
    #if (INJ_CHANNELS >= 5)
    Timer5.resume();
    #endif
  }

  uint16_t freeRam()
  {
      char top = 't';
      return &top - reinterpret_cast<char*>(sbrk(0));
  }

    /*
  ***********************************************************************************************************
  * Interrupt callback functions
  */
  void oneMSInterval(HardwareTimer*){oneMSInterval();}
  void boostInterrupt(HardwareTimer*){boostInterrupt();}
  void fuelSchedule1Interrupt(HardwareTimer*){fuelSchedule1Interrupt();}
  void fuelSchedule2Interrupt(HardwareTimer*){fuelSchedule2Interrupt();}
  void fuelSchedule3Interrupt(HardwareTimer*){fuelSchedule3Interrupt();}
  void fuelSchedule4Interrupt(HardwareTimer*){fuelSchedule4Interrupt();}
  void idleInterrupt(HardwareTimer*){idleInterrupt();}
  void vvtInterrupt(HardwareTimer*){vvtInterrupt();}
  void ignitionSchedule1Interrupt(HardwareTimer*){ignitionSchedule1Interrupt();}
  void ignitionSchedule2Interrupt(HardwareTimer*){ignitionSchedule2Interrupt();}
  void ignitionSchedule3Interrupt(HardwareTimer*){ignitionSchedule3Interrupt();}
  void ignitionSchedule4Interrupt(HardwareTimer*){ignitionSchedule4Interrupt();}
  void ignitionSchedule5Interrupt(HardwareTimer*){ignitionSchedule5Interrupt();}

#endif
