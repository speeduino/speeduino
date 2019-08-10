#if defined(CORE_STM32_OFFICIAL)
#include "board_stm32_official.h"
#include "globals.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"
#include <timer.h>

#if defined(STM32F4)
  #define NR_OFF_TIMERS 9
  //stimer_t HardwareTimers[NR_OFF_TIMERS + 1];
  stimer_t HardwareTimers_1;
  stimer_t HardwareTimers_2;
  stimer_t HardwareTimers_3;
  stimer_t HardwareTimers_4;
  stimer_t HardwareTimers_5;
  stimer_t HardwareTimers_8;
  //These should really be in the stm32GENERIC libs, but for somereason they only have timers 1-4
  //    #include <stm32_TIM_variant_11.h>
  //      #include "src/HardwareTimers/HardwareTimer.h"
  //    HardwareTimer Timer5(TIM5, chip_tim5, sizeof(chip_tim5) / sizeof(chip_tim5[0]));
  //    HardwareTimer Timer8(TIM8, chip_tim8, sizeof(chip_tim8) / sizeof(chip_tim8[0]));
#else
  #include "HardwareTimer.h"
#endif

  extern void oneMSIntervalIRQ(stimer_t *Timer)
  {
    oneMSInterval();
  }
  extern void EmptyIRQCallback(stimer_t *Timer, uint32_t channel)
  {
    
  }

  void initBoard()
  {
    /*
    * Initialize timers
    */

    HardwareTimers_1.timer = TIM1;
    HardwareTimers_2.timer = TIM2;
    HardwareTimers_3.timer = TIM3;
    HardwareTimers_4.timer = TIM4;

    HardwareTimers_5.timer = TIM5;
    HardwareTimers_8.timer = TIM8;
    

    /*
    ***********************************************************************************************************
    * General
    */
    #define FLASH_LENGTH 8192

    /*
    ***********************************************************************************************************
    * Idle
    */
    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) )
    {
        idle_pwm_max_count = 1000000L / (configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 5KHz
    } 

    //This must happen at the end of the idle init
    TimerPulseInit(&HardwareTimers_1, 0xFFFF, 0, EmptyIRQCallback);
    //setTimerPrescalerRegister(&HardwareTimers_1, (uint32_t)(getTimerClkFreq(HardwareTimers_1.timer) / (500000)) - 1);
    if(idle_pwm_max_count > 0) { attachIntHandleOC(&HardwareTimers_1, idleInterrupt, 4, 0);} //on first flash the configPage4.iacAlgorithm is invalid
    //Timer1.setMode(4, TIMER_OUTPUT_COMPARE);
    //timer_set_mode(TIMER1, 4, TIMER_OUTPUT_COMPARE;
    //on first flash the configPage4.iacAlgorithm is invalid:
    //if(idle_pwm_max_count > 0) { Timer1.attachInterrupt(4, idleInterrupt);} 
    //Timer1.resume();


    /*
    ***********************************************************************************************************
    * Timers
    */
    #if defined(ARDUINO_BLACK_F407VE) || defined(STM32F4) || defined(_STM32F4_)
      TimerHandleInit(&HardwareTimers_8, 1000, 168);
      attachIntHandle(&HardwareTimers_8, oneMSIntervalIRQ);
    #else
      //Should do something here for other boards
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
    //    Timer1.setMode(2, TIMER_OUTPUT_COMPARE);
    //    Timer1.setMode(3, TIMER_OUTPUT_COMPARE);
    //    if(boost_pwm_max_count > 0) { Timer1.attachInterrupt(2, boostInterrupt);}
    //    if(vvt_pwm_max_count > 0) { Timer1.attachInterrupt(3, vvtInterrupt);}
      if(idle_pwm_max_count > 0) { attachIntHandleOC(&HardwareTimers_1, boostInterrupt, 2, 0);}
      if(vvt_pwm_max_count > 0) { attachIntHandleOC(&HardwareTimers_1, vvtInterrupt, 3, 0);}
    //    Timer1.resume();
        
      TimerPulseInit(&HardwareTimers_3, 0xFFFF, 0, EmptyIRQCallback);
      attachIntHandleOC(&HardwareTimers_3, fuelSchedule1Interrupt, 1, 0);
      attachIntHandleOC(&HardwareTimers_3, fuelSchedule2Interrupt, 2, 0);
      attachIntHandleOC(&HardwareTimers_3, fuelSchedule3Interrupt, 3, 0);
      attachIntHandleOC(&HardwareTimers_3, fuelSchedule4Interrupt, 4, 0);
      

      TimerPulseInit(&HardwareTimers_2, 0xFFFF, 0, EmptyIRQCallback);
      attachIntHandleOC(&HardwareTimers_2, ignitionSchedule1Interrupt, 1, 0);
      attachIntHandleOC(&HardwareTimers_2, ignitionSchedule2Interrupt, 2, 0);
      attachIntHandleOC(&HardwareTimers_2, ignitionSchedule3Interrupt, 3, 0);
      attachIntHandleOC(&HardwareTimers_2, ignitionSchedule4Interrupt, 4, 0);
      
      //Attach interupt functions
      //Injection

      TimerPulseInit(&HardwareTimers_5, 0xFFFF, 0, EmptyIRQCallback);
      //setTimerPrescalerRegister(&HardwareTimers_5, (uint32_t)(getTimerClkFreq(HardwareTimers_5.timer) / (1000000)) - 1);
      #if (INJ_CHANNELS >= 5)
      attachIntHandleOC(&HardwareTimers_5, fuelSchedule5Interrupt, 1, 0);
      //Timer5.attachInterrupt(1, fuelSchedule5Interrupt);
      #endif
      #if (INJ_CHANNELS >= 6)
      attachIntHandleOC(&HardwareTimers_5, fuelSchedule6Interrupt, 2, 0);
      //Timer5.attachInterrupt(2, fuelSchedule6Interrupt);
      #endif
      #if (INJ_CHANNELS >= 7)
      attachIntHandleOC(&HardwareTimers_5, fuelSchedule7Interrupt, 3, 0);
      //Timer5.attachInterrupt(3, fuelSchedule7Interrupt);
      #endif
      #if (INJ_CHANNELS >= 8)
      attachIntHandleOC(&HardwareTimers_5, fuelSchedule8Interrupt, 4, 0);
      //Timer5.attachInterrupt(4, fuelSchedule8Interrupt);
      #endif

      TimerPulseInit(&HardwareTimers_4, 0xFFFF, 0, EmptyIRQCallback);
      //setTimerPrescalerRegister(&HardwareTimers_4, (uint32_t)(getTimerClkFreq(HardwareTimers_4.timer) / (1000000)) - 1);
      #if (IGN_CHANNELS >= 5)
      attachIntHandleOC(&HardwareTimers_4, ignitionSchedule5Interrupt, 1, 0);
      //Timer4.attachInterrupt(1, ignitionSchedule5Interrupt);
      #endif
      #if (IGN_CHANNELS >= 6)
      attachIntHandleOC(&HardwareTimers_4, ignitionSchedule6Interrupt, 2, 0);
      //Timer4.attachInterrupt(2, ignitionSchedule6Interrupt);
      #endif
      #if (IGN_CHANNELS >= 7)
      attachIntHandleOC(&HardwareTimers_4, ignitionSchedule7Interrupt, 3, 0);
      //Timer4.attachInterrupt(3, ignitionSchedule7Interrupt);
      #endif
      #if (IGN_CHANNELS >= 8)
      attachIntHandleOC(&HardwareTimers_4, ignitionSchedule8Interrupt, 4, 0);
      //Timer4.attachInterrupt(4, ignitionSchedule8Interrupt);
      #endif
      
      setTimerPrescalerRegister(&HardwareTimers_2, (uint32_t)(getTimerClkFreq(HardwareTimers_2.timer) / (250000)) - 1);
      setTimerPrescalerRegister(&HardwareTimers_3, (uint32_t)(getTimerClkFreq(HardwareTimers_3.timer) / (250000)) - 1);
      setTimerPrescalerRegister(&HardwareTimers_4, (uint32_t)(getTimerClkFreq(HardwareTimers_4.timer) / (250000)) - 1);
      setTimerPrescalerRegister(&HardwareTimers_5, (uint32_t)(getTimerClkFreq(HardwareTimers_5.timer) / (250000)) - 1);
  }

  uint16_t freeRam()
  {
      char top = 't';
      return &top - reinterpret_cast<char*>(sbrk(0));
  }

  //pinmapping the STM32F407 for different boards, at this moment no board is desgined.
  //All boards are set to the default just to be sure. 
  
#endif
