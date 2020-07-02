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
        idle_pwm_max_count = 1000000L / (4 * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 4uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 5KHz
    } 

    //This must happen at the end of the idle init
    Timer1.setMode(4, TIMER_OUTPUT_COMPARE);
    Timer1.attachInterrupt(4, idleInterrupt);  //on first flash the configPage4.iacAlgorithm is invalid


    /*
    ***********************************************************************************************************
    * Timers
    */
    #if defined(ARDUINO_BLUEPILL_F103C8) || defined(ARDUINO_BLUEPILL_F103CB)
      Timer4.setOverflow(1000, MICROSEC_FORMAT);  // Set up period
      Timer4.setMode(1, TIMER_OUTPUT_COMPARE);
      Timer4.attachInterrupt(1, oneMSInterval);
      Timer4.resume(); //Start Timer
    #else
      Timer11.setOverflow(1000, MICROSEC_FORMAT);  // Set up period
      Timer11.setMode(1, TIMER_OUTPUT_COMPARE);
      Timer11.attachInterrupt(1, oneMSInterval);
      Timer11.resume(); //Start Timer
    #endif
    pinMode(LED_BUILTIN, OUTPUT); //Visual WDT

    /*
    ***********************************************************************************************************
    * Auxilliaries
    */
    //2uS resolution Min 8Hz, Max 5KHz
    boost_pwm_max_count = 1000000L / (4 * configPage6.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 4uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (4 * configPage6.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 4uS) it takes to complete 1 cycle

    //Need to be initialised last due to instant interrupt
    Timer1.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer1.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer1.attachInterrupt(2, boostInterrupt);
    Timer1.attachInterrupt(3, vvtInterrupt);

    /*
    ***********************************************************************************************************
    * Schedules
    */
    Timer1.setOverflow(0xFFFF, TICK_FORMAT);
    Timer2.setOverflow(0xFFFF, TICK_FORMAT);
    Timer3.setOverflow(0xFFFF, TICK_FORMAT);

    Timer1.setPrescaleFactor(((Timer1.getTimerClkFreq()/1000000) * 4)-1);   //4us resolution
    Timer2.setPrescaleFactor(((Timer2.getTimerClkFreq()/1000000) * 4)-1);   //4us resolution
    Timer3.setPrescaleFactor(((Timer3.getTimerClkFreq()/1000000) * 4)-1);   //4us resolution

    Timer2.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(4, TIMER_OUTPUT_COMPARE);

    Timer3.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(4, TIMER_OUTPUT_COMPARE);
    Timer1.setMode(1, TIMER_OUTPUT_COMPARE);

    //Attach interrupt functions
    //Injection
    Timer3.attachInterrupt(1, fuelSchedule1Interrupt);
    Timer3.attachInterrupt(2, fuelSchedule2Interrupt);
    Timer3.attachInterrupt(3, fuelSchedule3Interrupt);
    Timer3.attachInterrupt(4, fuelSchedule4Interrupt);
    #if (INJ_CHANNELS >= 5)
    Timer5.setOverflow(0xFFFF, TICK_FORMAT);
    Timer5.setPrescaleFactor(((Timer5.getTimerClkFreq()/1000000) * 4)-1);   //4us resolution
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
    Timer4.setOverflow(0xFFFF, TICK_FORMAT);
    Timer4.setPrescaleFactor(((Timer4.getTimerClkFreq()/1000000) * 4)-1);   //4us resolution
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
  #if ((STM32_CORE_VERSION_MINOR<=8) & (STM32_CORE_VERSION_MAJOR==1)) 
  void oneMSInterval(HardwareTimer*){oneMSInterval();}
  void boostInterrupt(HardwareTimer*){boostInterrupt();}
  void fuelSchedule1Interrupt(HardwareTimer*){fuelSchedule1Interrupt();}
  void fuelSchedule2Interrupt(HardwareTimer*){fuelSchedule2Interrupt();}
  void fuelSchedule3Interrupt(HardwareTimer*){fuelSchedule3Interrupt();}
  void fuelSchedule4Interrupt(HardwareTimer*){fuelSchedule4Interrupt();}
  #if (INJ_CHANNELS >= 5)
  void fuelSchedule5Interrupt(HardwareTimer*){fuelSchedule5Interrupt();}
  #endif
  #if (INJ_CHANNELS >= 6)
  void fuelSchedule6Interrupt(HardwareTimer*){fuelSchedule6Interrupt();}
  #endif
  #if (INJ_CHANNELS >= 7)
  void fuelSchedule7Interrupt(HardwareTimer*){fuelSchedule7Interrupt();}
  #endif
  #if (INJ_CHANNELS >= 8)
  void fuelSchedule8Interrupt(HardwareTimer*){fuelSchedule8Interrupt();}
  #endif
  void idleInterrupt(HardwareTimer*){idleInterrupt();}
  void vvtInterrupt(HardwareTimer*){vvtInterrupt();}
  void ignitionSchedule1Interrupt(HardwareTimer*){ignitionSchedule1Interrupt();}
  void ignitionSchedule2Interrupt(HardwareTimer*){ignitionSchedule2Interrupt();}
  void ignitionSchedule3Interrupt(HardwareTimer*){ignitionSchedule3Interrupt();}
  void ignitionSchedule4Interrupt(HardwareTimer*){ignitionSchedule4Interrupt();}
  #if (IGN_CHANNELS >= 5)
  void ignitionSchedule5Interrupt(HardwareTimer*){ignitionSchedule5Interrupt();}
  #endif
  #if (IGN_CHANNELS >= 6)
  void ignitionSchedule6Interrupt(HardwareTimer*){ignitionSchedule6Interrupt();}
  #endif
  #if (IGN_CHANNELS >= 7)
  void ignitionSchedule7Interrupt(HardwareTimer*){ignitionSchedule7Interrupt();}
  #endif
  #if (IGN_CHANNELS >= 8)
  void ignitionSchedule8Interrupt(HardwareTimer*){ignitionSchedule8Interrupt();}
  #endif
  #endif //End core<=1.8
#endif
