#include "globals.h"
#if defined(STM32_CORE_VERSION_MAJOR)
#include <Arduino.h>
#include "board_stm32_official.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"
#include "HardwareTimer.h"
#include "PinNames.h"

//ADC defines 
#define ADC_SAMPLINGTIME        ADC_SAMPLETIME_144CYCLES  //The longer the normal sampling time, the higher the acquired ADC accuracy.
#define ADC_CLOCK_DIV       ADC_CLOCK_SYNC_PCLK_DIV4
#ifndef ADC_REGULAR_RANK_1
#define ADC_REGULAR_RANK_1  1
#endif

ADC_HandleTypeDef AdcHandle = {};
ADC_ChannelConfTypeDef AdcChannelConf = {};


#if HAL_CAN_MODULE_ENABLED
//This activates CAN1 interface on STM32, but it's named as Can0, because that's how Teensy implementation is done
STM32_CAN Can0 (CAN1, ALT_2, RX_SIZE_256, TX_SIZE_16);
/*
These CAN interfaces and pins are available for use, depending on the chip/package:
Default CAN1 pins are PA11 and PA12. Alternative (ALT) pins are PB8 & PB9 and ALT_2 pins are PD0 & PD1.
Default CAN2 pins are PB12 & PB13. Alternative (ALT) pins are PB5 & PB6.
Default CAN3 pins are PA8 & PA15. Alternative (ALT) pins are PB3 & PB4.
*/
#endif

#if defined SD_LOGGING
    SPIClass SD_SPI(PC12, PC11, PC10); //SPI3_MOSI, SPI3_MISO, SPI3_SCK
#endif

#if defined(SRAM_AS_EEPROM)
    BackupSramAsEEPROM EEPROM;
#elif defined(USE_SPI_EEPROM)
    #if defined(STM32F407xx)
      SPIClass SPI_for_flash(PB5, PB4, PB3); //SPI1_MOSI, SPI1_MISO, SPI1_SCK
    #else //Blue/Black Pills
      SPIClass SPI_for_flash(PB15, PB14, PB13);
    #endif
 
    //windbond W25Q16 SPI flash EEPROM emulation
    EEPROM_Emulation_Config EmulatedEEPROMMconfig{255UL, 4096UL, 31, 0x00100000UL};
    Flash_SPI_Config SPIconfig{USE_SPI_EEPROM, SPI_for_flash};
    SPI_EEPROM_Class EEPROM(EmulatedEEPROMMconfig, SPIconfig);
#elif defined(FRAM_AS_EEPROM) //https://github.com/VitorBoss/FRAM
    #if defined(STM32F407xx)
      SPIClass SPI_for_FRAM(PB5, PB4, PB3); //SPI1_MOSI, SPI1_MISO, SPI1_SCK
      FramClass EEPROM(PB0, SPI_for_FRAM);
    #else //Blue/Black Pills
      SPIClass SPI_for_FRAM(PB15, PB14, PB13);
      FramClass EEPROM(PB12, SPI_for_FRAM);
    #endif
#elif defined(STM32F7xx)
  #if defined(DUAL_BANK)
    EEPROM_Emulation_Config EmulatedEEPROMMconfig{4UL, 131072UL, 2047UL, 0x08120000UL};
  #else
    EEPROM_Emulation_Config EmulatedEEPROMMconfig{2UL, 262144UL, 4095UL, 0x08180000UL};
  #endif
    InternalSTM32F7_EEPROM_Class EEPROM(EmulatedEEPROMMconfig);
#elif defined(STM32F401xC)
    EEPROM_Emulation_Config EmulatedEEPROMMconfig{1UL, 131072UL, 4095UL, 0x08020000UL};
    InternalSTM32F4_EEPROM_Class EEPROM(EmulatedEEPROMMconfig);
#elif defined(STM32F411xE)
    EEPROM_Emulation_Config EmulatedEEPROMMconfig{2UL, 131072UL, 4095UL, 0x08040000UL};
    InternalSTM32F4_EEPROM_Class EEPROM(EmulatedEEPROMMconfig);
#else //default case, internal flash as EEPROM for STM32F4
    EEPROM_Emulation_Config EmulatedEEPROMMconfig{4UL, 131072UL, 2047UL, 0x08080000UL};
    InternalSTM32F4_EEPROM_Class EEPROM(EmulatedEEPROMMconfig);
#endif


HardwareTimer Timer1(TIM1);
HardwareTimer Timer2(TIM2);
HardwareTimer Timer3(TIM3);
HardwareTimer Timer4(TIM4);
#if !defined(ARDUINO_BLUEPILL_F103C8) && !defined(ARDUINO_BLUEPILL_F103CB) //F103 just have 4 timers
HardwareTimer Timer5(TIM5);
#if defined(TIM11)
HardwareTimer Timer11(TIM11);
#elif defined(TIM7)
HardwareTimer Timer11(TIM7);
#endif
#endif

#ifdef RTC_ENABLED
STM32RTC& rtc = STM32RTC::getInstance();
#endif

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
    * Real Time clock for datalogging/time stamping
    */
    #ifdef RTC_ENABLED
      rtc.setClockSource(STM32RTC::LSE_CLOCK); //Initialize external clock for RTC. That is the only clock running of VBAT
      rtc.begin(); // initialize RTC 24H format
    #endif
    /*
    ***********************************************************************************************************
    * Idle
    */
    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OLCL))
    {
        idle_pwm_max_count = 1000000L / (TIMER_RESOLUTION * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 4uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 5KHz
    } 

    //This must happen at the end of the idle init
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer1.setMode(4, TIMER_OUTPUT_COMPARE);
    #else
    Timer1.setMode(4, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer1.attachInterrupt(4, idleInterrupt);  //on first flash the configPage4.iacAlgorithm is invalid


    /*
    ***********************************************************************************************************
    * Timers
    */
    #if defined(ARDUINO_BLUEPILL_F103C8) || defined(ARDUINO_BLUEPILL_F103CB)
      Timer4.setOverflow(1000, MICROSEC_FORMAT);  // Set up period
      #if ( STM32_CORE_VERSION_MAJOR < 2 )
      Timer4.setMode(1, TIMER_OUTPUT_COMPARE);
      Timer4.attachInterrupt(1, oneMSInterval);
      #else //2.0 forward
      Timer4.attachInterrupt(oneMSInterval);
      #endif
      Timer4.resume(); //Start Timer
    #else
      Timer11.setOverflow(1000, MICROSEC_FORMAT);  // Set up period
      #if ( STM32_CORE_VERSION_MAJOR < 2 )
      Timer11.setMode(1, TIMER_OUTPUT_COMPARE);
      Timer11.attachInterrupt(1, oneMSInterval);
      #else
      Timer11.attachInterrupt(oneMSInterval);
      #endif
      Timer11.resume(); //Start Timer
    #endif
    pinMode(LED_BUILTIN, OUTPUT); //Visual WDT

    /*
    ***********************************************************************************************************
    * Auxilliaries
    */
    //2uS resolution Min 8Hz, Max 5KHz

    boost_pwm_max_count = 1000000L / (TIMER_RESOLUTION * configPage6.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 4uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (TIMER_RESOLUTION * configPage6.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 4uS) it takes to complete 1 cycle
    fan_pwm_max_count = 1000000L / (TIMER_RESOLUTION * configPage6.fanFreq * 2); //Converts the frequency in Hz to the number of ticks (at 4uS) it takes to complete 1 cycle

    //Need to be initialised last due to instant interrupt
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer1.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer1.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer1.setMode(3, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
	Timer1.setMode(1, TIMER_OUTPUT_COMPARE_TOGGLE);
    Timer1.setMode(2, TIMER_OUTPUT_COMPARE_TOGGLE);
    Timer1.setMode(3, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer1.attachInterrupt(1, fanInterrupt);
    Timer1.attachInterrupt(2, boostInterrupt);
    Timer1.attachInterrupt(3, vvtInterrupt);

    /*
    ***********************************************************************************************************
    * Schedules
    */
    Timer1.setOverflow(0xFFFF, TICK_FORMAT);
    Timer2.setOverflow(0xFFFF, TICK_FORMAT);
    Timer3.setOverflow(0xFFFF, TICK_FORMAT);

    Timer1.setPrescaleFactor(((Timer1.getTimerClkFreq()/1000000) * TIMER_RESOLUTION)-1);   //4us resolution
    Timer2.setPrescaleFactor(((Timer2.getTimerClkFreq()/1000000) * TIMER_RESOLUTION)-1);   //4us resolution
    Timer3.setPrescaleFactor(((Timer3.getTimerClkFreq()/1000000) * TIMER_RESOLUTION)-1);   //4us resolution

    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer2.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(4, TIMER_OUTPUT_COMPARE);

    Timer3.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(4, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
    Timer2.setMode(1, TIMER_OUTPUT_COMPARE_TOGGLE);
    Timer2.setMode(2, TIMER_OUTPUT_COMPARE_TOGGLE);
    Timer2.setMode(3, TIMER_OUTPUT_COMPARE_TOGGLE);
    Timer2.setMode(4, TIMER_OUTPUT_COMPARE_TOGGLE);

    Timer3.setMode(1, TIMER_OUTPUT_COMPARE_TOGGLE);
    Timer3.setMode(2, TIMER_OUTPUT_COMPARE_TOGGLE);
    Timer3.setMode(3, TIMER_OUTPUT_COMPARE_TOGGLE);
    Timer3.setMode(4, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    //Attach interrupt functions
    //Injection
    Timer3.attachInterrupt(1, fuelSchedule1Interrupt);
    Timer3.attachInterrupt(2, fuelSchedule2Interrupt);
    Timer3.attachInterrupt(3, fuelSchedule3Interrupt);
    Timer3.attachInterrupt(4, fuelSchedule4Interrupt);
    #if (INJ_CHANNELS >= 5)
    Timer5.setOverflow(0xFFFF, TICK_FORMAT);
    Timer5.setPrescaleFactor(((Timer5.getTimerClkFreq()/1000000) * TIMER_RESOLUTION)-1);   //4us resolution
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer5.setMode(1, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
    Timer5.setMode(1, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer5.attachInterrupt(1, fuelSchedule5Interrupt);
    #endif
    #if (INJ_CHANNELS >= 6)
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer5.setMode(2, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
    Timer5.setMode(1, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer5.attachInterrupt(2, fuelSchedule6Interrupt);
    #endif
    #if (INJ_CHANNELS >= 7)
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer5.setMode(3, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
    Timer5.setMode(3, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer5.attachInterrupt(3, fuelSchedule7Interrupt);
    #endif
    #if (INJ_CHANNELS >= 8)
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer5.setMode(4, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
    Timer5.setMode(4, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer5.attachInterrupt(4, fuelSchedule8Interrupt);
    #endif

    //Ignition
    Timer2.attachInterrupt(1, ignitionSchedule1Interrupt); 
    Timer2.attachInterrupt(2, ignitionSchedule2Interrupt);
    Timer2.attachInterrupt(3, ignitionSchedule3Interrupt);
    Timer2.attachInterrupt(4, ignitionSchedule4Interrupt);
    #if (IGN_CHANNELS >= 5)
    Timer4.setOverflow(0xFFFF, TICK_FORMAT);
    Timer4.setPrescaleFactor(((Timer4.getTimerClkFreq()/1000000) * TIMER_RESOLUTION)-1);   //4us resolution
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer4.setMode(1, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
    Timer4.setMode(1, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer4.attachInterrupt(1, ignitionSchedule5Interrupt);
    #endif
    #if (IGN_CHANNELS >= 6)
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer4.setMode(2, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
    Timer4.setMode(2, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer4.attachInterrupt(2, ignitionSchedule6Interrupt);
    #endif
    #if (IGN_CHANNELS >= 7)
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer4.setMode(3, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
    Timer4.setMode(3, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer4.attachInterrupt(3, ignitionSchedule7Interrupt);
    #endif
    #if (IGN_CHANNELS >= 8)
    #if ( STM32_CORE_VERSION_MAJOR < 2 )
    Timer4.setMode(4, TIMER_OUTPUT_COMPARE);
    #else //2.0 forward
    Timer4.setMode(4, TIMER_OUTPUT_COMPARE_TOGGLE);
    #endif
    Timer4.attachInterrupt(4, ignitionSchedule8Interrupt);
    #endif

    Timer1.resume();
    DISABLE_BOOST_TIMER();  //Make sure it is disabled. It's is enabled by default on the library
    DISABLE_VVT_TIMER();    //Make sure it is disabled. It's is enabled by default on the library
    IDLE_TIMER_DISABLE();   //Make sure it is disabled. It's is enabled by default on the library
    Timer2.resume();
    IGN1_TIMER_DISABLE(); //Make sure it is disabled. It's is enabled by default on the library
    IGN2_TIMER_DISABLE(); //Make sure it is disabled. It's is enabled by default on the library
    IGN3_TIMER_DISABLE(); //Make sure it is disabled. It's is enabled by default on the library
    IGN4_TIMER_DISABLE(); //Make sure it is disabled. It's is enabled by default on the library
    Timer3.resume();
    FUEL1_TIMER_DISABLE();  //Make sure it is disabled. It's is enabled by default on the library
    FUEL2_TIMER_DISABLE();  //Make sure it is disabled. It's is enabled by default on the library
    FUEL3_TIMER_DISABLE();  //Make sure it is disabled. It's is enabled by default on the library
    FUEL4_TIMER_DISABLE();  //Make sure it is disabled. It's is enabled by default on the library
    #if (IGN_CHANNELS >= 5)
    Timer4.resume();
    IGN5_TIMER_DISABLE(); //Make sure it is disabled. It's is enabled by default on the library
    IGN6_TIMER_DISABLE(); //Make sure it is disabled. It's is enabled by default on the library
    IGN7_TIMER_DISABLE(); //Make sure it is disabled. It's is enabled by default on the library
    IGN8_TIMER_DISABLE(); //Make sure it is disabled. It's is enabled by default on the library
    #endif
    #if (INJ_CHANNELS >= 5)
    Timer5.resume();
    FUEL5_TIMER_DISABLE();  //Make sure it is disabled. It's is enabled by default on the library
    FUEL6_TIMER_DISABLE();  //Make sure it is disabled. It's is enabled by default on the library
    FUEL7_TIMER_DISABLE();  //Make sure it is disabled. It's is enabled by default on the library
    FUEL8_TIMER_DISABLE();  //Make sure it is disabled. It's is enabled by default on the library
    #endif
  }

  uint16_t freeRam()
  {
    uint32_t freeRam;
    uint32_t stackTop;
    uint32_t heapTop;

    // current position of the stack.
    stackTop = (uint32_t)&stackTop;

    // current position of heap.
    void *hTop = malloc(1);
    heapTop = (uint32_t)hTop;
    free(hTop);
    freeRam = stackTop - heapTop;

    if(freeRam>0xFFFF){return 0xFFFF;}
    else{return freeRam;}
  }

  void doSystemReset( void )
  {
    __disable_irq();
    NVIC_SystemReset();
  }

  void jumpToBootloader( void ) // https://github.com/3devo/Arduino_Core_STM32/blob/jumpSysBL/libraries/SrcWrapper/src/stm32/bootloader.c
  { // https://github.com/markusgritsch/SilF4ware/blob/master/SilF4ware/drv_reset.c
    #if !defined(STM32F103xB)
    HAL_RCC_DeInit();
    HAL_DeInit();
    SysTick->VAL = SysTick->LOAD = SysTick->CTRL = 0;
    SYSCFG->MEMRMP = 0x01;

    #if defined(STM32F7xx) || defined(STM32H7xx)
    const uint32_t DFU_addr = 0x1FF00000; // From AN2606
    #else
    const uint32_t DFU_addr = 0x1FFF0000; // Default for STM32F10xxx and STM32F40xxx/STM32F41xxx from AN2606
    #endif
    // This is assembly to prevent modifying the stack pointer after
    // loading it, and to ensure a jump (not call) to the bootloader.
    // Not sure if the barriers are really needed, they were taken from
    // https://github.com/GrumpyOldPizza/arduino-STM32L4/blob/ac659033eadd50cfe001ba1590a1362b2d87bb76/system/STM32L4xx/Source/boot_stm32l4xx.c#L159-L165
    asm volatile (
      "ldr r0, [%[DFU_addr], #0]   \n\t"  // get address of stack pointer
      "msr msp, r0            \n\t"  // set stack pointer
      "ldr r0, [%[DFU_addr], #4]   \n\t"  // get address of reset handler
      "dsb                    \n\t"  // data sync barrier
      "isb                    \n\t"  // instruction sync barrier
      "bx r0                  \n\t"  // branch to bootloader
      : : [DFU_addr] "l" (DFU_addr) : "r0"
    );
    __builtin_unreachable();
    #endif
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
  void fanInterrupt(HardwareTimer*){fanInterrupt();}
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

    /*
    ***********************************************************************************************************
    * ADC
    */
void ADCinit_STM32()
{
  AdcHandle.Instance                   = ADC1;         /* ADC1 can do all availavle channels on black_F407VE */
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_DIV;           /* (A)synchronous clock mode, input ADC clock divided */
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_10B;      /* resolution for converted data */
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;     /* Right-alignment for converted data */
  AdcHandle.Init.ScanConvMode          = DISABLE;                 /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
  AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;     /* EOC flag picked-up to indicate conversion end */
  AdcHandle.Init.ContinuousConvMode    = DISABLE;                 /* Continuous mode disabled to have only 1 conversion at each conversion trig */
  AdcHandle.Init.NbrOfConversion       = 1;                       /* Specifies the number of ranks that will be converted within the regular group sequencer. */
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                 /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.NbrOfDiscConversion   = 0;                       /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;      /* Software start to trig the 1st conversion manually, without external event */
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;/* Parameter discarded because software trigger chosen */
  AdcHandle.Init.DMAContinuousRequests = DISABLE;

  AdcHandle.State = HAL_ADC_STATE_RESET;
  AdcHandle.DMA_Handle = NULL;
  AdcHandle.Lock = HAL_UNLOCKED;
  /* Some other ADC_HandleTypeDef fields exists but not required */

  HAL_ADC_Init(&AdcHandle);
}

/*returns 1 when ADC succesfully started */
bool ADC_start_STM32(uint32_t ulPin) 
{
  uint32_t channel = 0;
  uint32_t bank = 0;
  PinName pin;
  
  pin = analogInputToPinName(ulPin);
  channel = get_adc_channel_STM32(pin);
  AdcChannelConf.Channel      = channel;                          /* Specifies the channel to configure into ADC */
  if (!IS_ADC_CHANNEL(AdcChannelConf.Channel)) {
    return 0; /* Channel Configuration Error */
  }
  AdcChannelConf.Rank         = ADC_REGULAR_RANK_1;               /* Specifies the rank in the regular group sequencer */
  AdcChannelConf.SamplingTime = ADC_SAMPLINGTIME;                     /* Sampling time value to be set for the selected channel */
  AdcChannelConf.Offset = 0;                                      /* Parameter discarded because offset correction is disabled */
  /* Configure ADC GPIO pin */
  if (!(pin & PADC_BASE)) {
    pinmap_pinout(pin, PinMap_ADC);
  }
  /*##-2- Configure ADC regular channel ######################################*/
  if (HAL_ADC_ConfigChannel(&AdcHandle, &AdcChannelConf) != HAL_OK) {
    /* Channel Configuration Error */
    return 0;
  } 
    /*##-3- Start the conversion process ####################*/
  if (HAL_ADC_Start(&AdcHandle) != HAL_OK) {
    /* Start Conversation Error */
    return 0;
  }
  return 1;  //success
}

bool ADC_CheckForConversionComplete_STM32(){
  ADC_HandleTypeDef* hadc=&AdcHandle;

  if (!(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_EOC))){
    return 0;
  }
    /* Clear regular group conversion flag */
  __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_STRT | ADC_FLAG_EOC);

  /* Update ADC state machine */
  SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC);

  /* Determine whether any further conversion upcoming on group regular       */
  /* by external trigger, continuous mode or scan sequence on going.          */
  /* Note: On STM32F4, there is no independent flag of end of sequence.       */
  /*       The test of scan sequence on going is done either with scan        */
  /*       sequence disabled or with end of conversion flag set to            */
  /*       of end of sequence.                                                */
  if(ADC_IS_SOFTWARE_START_REGULAR(hadc)                   &&
     (hadc->Init.ContinuousConvMode == DISABLE)            &&
     (HAL_IS_BIT_CLR(hadc->Instance->SQR1, ADC_SQR1_L) ||
      HAL_IS_BIT_CLR(hadc->Instance->CR2, ADC_CR2_EOCS)  )   )
  {
    /* Set ADC state */
    CLEAR_BIT(hadc->State, HAL_ADC_STATE_REG_BUSY);

    if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_INJ_BUSY))
    {
      SET_BIT(hadc->State, HAL_ADC_STATE_READY);
    }
  }
  return 1;
}

uint16_t ADC_get_value_STM32()
{
  uint16_t temp = 0U;
    /* Check if the continous conversion of regular channel is finished */
  if ((HAL_ADC_GetState(&AdcHandle) & HAL_ADC_STATE_REG_EOC) == HAL_ADC_STATE_REG_EOC) {
    /*##-5- Get the converted value of regular channel  ########################*/
    temp = HAL_ADC_GetValue(&AdcHandle);
  }
  //  temp = AdcHandle.Instance->DR; //basically this is all that it does
  return temp;
}
uint32_t get_adc_channel_STM32(PinName pin) //this really only needs to be here because it is made private in the HAL drivers
{
  uint32_t function = pinmap_function(pin, PinMap_ADC);
  uint32_t channel = 0;
  switch (STM_PIN_CHANNEL(function)) {
#ifdef ADC_CHANNEL_0
    case 0:
      channel = ADC_CHANNEL_0;
      break;
#endif
    case 1:
      channel = ADC_CHANNEL_1;
      break;
    case 2:
      channel = ADC_CHANNEL_2;
      break;
    case 3:
      channel = ADC_CHANNEL_3;
      break;
    case 4:
      channel = ADC_CHANNEL_4;
      break;
    case 5:
      channel = ADC_CHANNEL_5;
      break;
    case 6:
      channel = ADC_CHANNEL_6;
      break;
    case 7:
      channel = ADC_CHANNEL_7;
      break;
    case 8:
      channel = ADC_CHANNEL_8;
      break;
    case 9:
      channel = ADC_CHANNEL_9;
      break;
    case 10:
      channel = ADC_CHANNEL_10;
      break;
    case 11:
      channel = ADC_CHANNEL_11;
      break;
    case 12:
      channel = ADC_CHANNEL_12;
      break;
    case 13:
      channel = ADC_CHANNEL_13;
      break;
    case 14:
      channel = ADC_CHANNEL_14;
      break;
    case 15:
      channel = ADC_CHANNEL_15;
      break;
#ifdef ADC_CHANNEL_16
    case 16:
      channel = ADC_CHANNEL_16;
      break;
#endif
    case 17:
      channel = ADC_CHANNEL_17;
      break;
#ifdef ADC_CHANNEL_18
    case 18:
      channel = ADC_CHANNEL_18;
      break;
#endif
#ifdef ADC_CHANNEL_19
    case 19:
      channel = ADC_CHANNEL_19;
      break;
#endif
#ifdef ADC_CHANNEL_20
    case 20:
      channel = ADC_CHANNEL_20;
      break;
    case 21:
      channel = ADC_CHANNEL_21;
      break;
    case 22:
      channel = ADC_CHANNEL_22;
      break;
    case 23:
      channel = ADC_CHANNEL_23;
      break;
    case 24:
      channel = ADC_CHANNEL_24;
      break;
    case 25:
      channel = ADC_CHANNEL_25;
      break;
    case 26:
      channel = ADC_CHANNEL_26;
      break;
#ifdef ADC_CHANNEL_27
    case 27:
      channel = ADC_CHANNEL_27;
      break;
    case 28:
      channel = ADC_CHANNEL_28;
      break;
    case 29:
      channel = ADC_CHANNEL_29;
      break;
    case 30:
      channel = ADC_CHANNEL_30;
      break;
    case 31:
      channel = ADC_CHANNEL_31;
      break;
#endif
#endif
    default:
      channel = 0;
      break;
  }
  return channel;
}

#endif
