#if defined(CORE_STM32)
#include "board_stm32.h"
#include "globals.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"
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

extern void oneMSIntervalIRQ(stimer_t *Timer){oneMSInterval();}
extern void EmptyIRQCallback(stimer_t *Timer, uint32_t channel){}

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
    //Timer1.setMode(4, TIMER_OUTPUT_COMPARE);
    //timer_set_mode(TIMER1, 4, TIMER_OUTPUT_COMPARE;
    //if(idle_pwm_max_count > 0) { Timer1.attachInterrupt(4, idleInterrupt);} //on first flash the configPage4.iacAlgorithm is invalid
    //Timer1.resume();


    /*
    ***********************************************************************************************************
    * Timers
    */
    #if defined(ARDUINO_BLACK_F407VE) || defined(STM32F4) || defined(_STM32F4_)
        TimerHandleInit(&HardwareTimers_8, 1000, 168);
        attachIntHandle(&HardwareTimers_8, oneMSIntervalIRQ);
    #else
        Timer4.setPeriod(1000);  // Set up period
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
//    Timer1.setMode(2, TIMER_OUTPUT_COMPARE);
//    Timer1.setMode(3, TIMER_OUTPUT_COMPARE);
//    if(boost_pwm_max_count > 0) { Timer1.attachInterrupt(2, boostInterrupt);}
//    if(vvt_pwm_max_count > 0) { Timer1.attachInterrupt(3, vvtInterrupt);}
//    Timer1.resume();

    /*
    ***********************************************************************************************************
    * Schedules
    */
    #if defined(ARDUINO_ARCH_STM32) // STM32GENERIC core
        //see https://github.com/rogerclarkmelbourne/Arduino_STM32/blob/754bc2969921f1ef262bd69e7faca80b19db7524/STM32F1/system/libmaple/include/libmaple/timer.h#L444
//        Timer1.setPrescaleFactor((HAL_RCC_GetHCLKFreq() * 2U)-1);  //2us resolution
//        Timer2.setPrescaleFactor((HAL_RCC_GetHCLKFreq() * 2U)-1);  //2us resolution
//        Timer3.setPrescaleFactor((HAL_RCC_GetHCLKFreq() * 2U)-1);  //2us resolution
    #else //libmaple core aka STM32DUINO
        //see https://github.com/rogerclarkmelbourne/Arduino_STM32/blob/754bc2969921f1ef262bd69e7faca80b19db7524/STM32F1/system/libmaple/include/libmaple/timer.h#L444
        #if defined (STM32F1) || defined(__STM32F1__)
            //(CYCLES_PER_MICROSECOND == 72, APB2 at 72MHz, APB1 at 36MHz).
            //Timer2 to 4 is on APB1, Timer1 on APB2.   http://www.st.com/resource/en/datasheet/stm32f103cb.pdf sheet 12
            Timer1.setPrescaleFactor((72 * 2U)-1); //2us resolution
            Timer2.setPrescaleFactor((36 * 2U)-1); //2us resolution
            Timer3.setPrescaleFactor((36 * 2U)-1); //2us resolution
        #elif defined(STM32F4)
            //(CYCLES_PER_MICROSECOND == 168, APB2 at 84MHz, APB1 at 42MHz).
            //Timer2 to 14 is on APB1, Timers 1, 8, 9 and 10 on APB2.   http://www.st.com/resource/en/datasheet/stm32f407vg.pdf sheet 120
            Timer1.setPrescaleFactor((84 * 2U)-1); //2us resolution
            Timer2.setPrescaleFactor((42 * 2U)-1); //2us resolution
            Timer3.setPrescaleFactor((42 * 2U)-1); //2us resolution
        #endif
    #endif
       
    TimerPulseInit(&HardwareTimers_2, 0xFFFF, 500, EmptyIRQCallback);
    attachIntHandleOC(&HardwareTimers_2, fuelSchedule1Interrupt, 1, 0);
    attachIntHandleOC(&HardwareTimers_2, fuelSchedule2Interrupt, 2, 0);
    attachIntHandleOC(&HardwareTimers_2, fuelSchedule3Interrupt, 3, 0);
    attachIntHandleOC(&HardwareTimers_2, fuelSchedule4Interrupt, 4, 0);

    TimerPulseInit(&HardwareTimers_3, 0xFFFF, 500, EmptyIRQCallback);
    attachIntHandleOC(&HardwareTimers_3, ignitionSchedule1Interrupt, 1, 0);
    attachIntHandleOC(&HardwareTimers_3, ignitionSchedule2Interrupt, 2, 0);
    attachIntHandleOC(&HardwareTimers_3, ignitionSchedule3Interrupt, 3, 0);
    attachIntHandleOC(&HardwareTimers_3, ignitionSchedule4Interrupt, 4, 0);
    
//    Timer1.setMode(1, TIMER_OUTPUT_COMPARE);

    //Attach interupt functions
    //Injection

    TimerPulseInit(&HardwareTimers_5, 0xFFFF, 0, EmptyIRQCallback);
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

}

uint16_t freeRam()
{
    char top = 't';
    return &top - reinterpret_cast<char*>(sbrk(0));
}
void setPinMapping(byte boardID)
{
  switch (boardID)
  {
    case 0:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.1 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 11; //Output pin injector 3 is on
      pinInjector4 = 10; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 6; //Pin for coil 1
      pinCoil2 = 7; //Pin for coil 2
      pinCoil3 = 12; //Pin for coil 3
      pinCoil4 = 13; //Pin for coil 4
      pinCoil5 = 14; //Pin for coil 5
      pinTrigger = 2; //The CAS pin
      pinTrigger2 = 3; //The CAS pin
      pinTPS = A0; //TPS input pin
      pinMAP = A1; //MAP sensor pin
      pinIAT = A2; //IAT sensor pin
      pinCLT = A3; //CLS sensor pin
      pinO2 = A4; //O2 Sensor pin
      pinIdle1 = 46; //Single wire idle control
      pinIdle2 = 47; //2 wire idle control
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinFan = 47; //Pin for the fan output
      pinFuelPump = 4; //Fuel pump output
      pinTachOut = 49; //Tacho output pin
      pinFlex = 19; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 43; //Reset control output
    #endif
      break;
    case 1:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.2 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 28; //Pin for coil 1
      pinCoil2 = 24; //Pin for coil 2
      pinCoil3 = 40; //Pin for coil 3
      pinCoil4 = 36; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 20; //The CAS pin
      pinTrigger2 = 21; //The Cam Sensor pin
      pinTPS = A2; //TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A8; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin
      pinIdle1 = 30; //Single wire idle control
      pinIdle2 = 31; //2 wire idle control
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinFan = 47; //Pin for the fan output
      pinFuelPump = 4; //Fuel pump output
      pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 43; //Reset control output
      break;
    #endif
    case 2:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.3 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 28; //Pin for coil 1
      pinCoil2 = 24; //Pin for coil 2
      pinCoil3 = 40; //Pin for coil 3
      pinCoil4 = 36; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A8; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      pinIdle2 = 53; //2 wire idle control
      pinBoost = 7; //Boost control
      pinVVT_1 = 6; //Default VVT output
      pinFuelPump = 4; //Fuel pump output
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinStepperEnable = 26; //Enable pin for DRV8825
      pinFan = A13; //Pin for the fan output
      pinLaunch = 51; //Can be overwritten below
      pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 50; //Reset control output
    #endif
      break;

    case 3:
      //Pin mappings as per the v0.4 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinInjector6 = 50; //CAUTION: Uses the same as Coil 4 below. 
      pinCoil1 = 40; //Pin for coil 1
      pinCoil2 = 38; //Pin for coil 2
      pinCoil3 = 52; //Pin for coil 3
      pinCoil4 = 50; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A8; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
      pinIdle1 = 5; //Single wire idle control
      pinIdle2 = 6; //2 wire idle control
      pinBoost = 7; //Boost control
      pinVVT_1 = 4; //Default VVT output
      pinFuelPump = 45; //Fuel pump output  (Goes to ULN2803)
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinStepperEnable = 24; //Enable pin for DRV8825
      pinFan = 47; //Pin for the fan output (Goes to ULN2803)
      pinLaunch = 51; //Can be overwritten below
      pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 43; //Reset control output
      
      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio
      //PA13~PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PD5 & PD6 Serial2
      pinInjector1 = PE7; //Output pin injector 1 is on
      pinInjector2 = PE8; //Output pin injector 2 is on
      pinInjector3 = PE9; //Output pin injector 3 is on
      pinInjector4 = PE10; //Output pin injector 4 is on
      pinInjector5 = PE11; //Output pin injector 5 is on
      pinInjector6 = PE12; //Output pin injector 6 is on
      pinCoil1 = PB5; //Pin for coil 1
      pinCoil2 = PB6; //Pin for coil 2
      pinCoil3 = PB7; //Pin for coil 3
      pinCoil4 = PB8; //Pin for coil 4
      pinCoil5 = PB9; //Pin for coil 5
      pinTPS = A0; //TPS input pin
      pinMAP = A1; //MAP sensor pin
      pinIAT = A2; //IAT sensor pin
      pinCLT = A3; //CLT sensor pin
      pinO2 = A4; //O2 Sensor pin
      pinBat = A5; //Battery reference voltage pin
      pinBaro = A10;
      pinIdle1 = PB8; //Single wire idle control
      pinIdle2 = PB9; //2 wire idle control
      pinBoost = PE0; //Boost control
      pinVVT_1 = PE1; //Default VVT output
      pinStepperDir = PD8; //Direction pin  for DRV8825 driver
      pinStepperStep = PB15; //Step pin for DRV8825 driver
      pinStepperEnable = PD9; //Enable pin for DRV8825
      pinDisplayReset = PE1; // OLED reset pin
      pinFan = PE2; //Pin for the fan output
      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PA7; //Tacho output pin
      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PE2; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PE3; //The CAS pin
      pinTrigger2 = PE4; //The Cam Sensor pin

        
//      #if defined(CORE_STM32)
//        //blue pill http://wiki.stm32duino.com/index.php?title=Blue_Pill
//        //Maple mini http://wiki.stm32duino.com/index.php?title=Maple_Mini
//        //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
//        pinInjector1 = PB7; //Output pin injector 1 is on
//        pinInjector2 = PB6; //Output pin injector 2 is on
//        pinInjector3 = PB5; //Output pin injector 3 is on
//        pinInjector4 = PB4; //Output pin injector 4 is on
//        pinCoil1 = PB3; //Pin for coil 1
//        pinCoil2 = PA15; //Pin for coil 2
//        pinCoil3 = PA14; //Pin for coil 3
//        pinCoil4 = PA9; //Pin for coil 4
//        pinCoil5 = PA8; //Pin for coil 5
//        pinTPS = A0; //TPS input pin
//        pinMAP = A1; //MAP sensor pin
//        pinIAT = A2; //IAT sensor pin
//        pinCLT = A3; //CLS sensor pin
//        pinO2 = A4; //O2 Sensor pin
//        pinBat = A5; //Battery reference voltage pin
//        pinBaro = pinMAP;
//        pinIdle1 = PB2; //Single wire idle control
//        pinIdle2 = PA2; //2 wire idle control
//        pinBoost = PA1; //Boost control
//        pinVVT_1 = PA0; //Default VVT output
//        pinStepperDir = PC15; //Direction pin  for DRV8825 driver
//        pinStepperStep = PC14; //Step pin for DRV8825 driver
//        pinStepperEnable = PC13; //Enable pin for DRV8825
//        pinDisplayReset = PB2; // OLED reset pin
//        pinFan = PB1; //Pin for the fan output
//        pinFuelPump = PB11; //Fuel pump output
//        pinTachOut = PB10; //Tacho output pin
//        //external interrupt enabled pins
//        pinFlex = PB8; // Flex sensor (Must be external interrupt enabled)
//        pinTrigger = PA10; //The CAS pin
//        pinTrigger2 = PA13; //The Cam Sensor pin
//      #endif
      break;

    case 9:
      //Pin mappings as per the MX5 PNP shield
      pinInjector1 = 11; //Output pin injector 1 is on
      pinInjector2 = 10; //Output pin injector 2 is on
      pinInjector3 = 9; //Output pin injector 3 is on
      pinInjector4 = 8; //Output pin injector 4 is on
      pinInjector5 = 14; //Output pin injector 5 is on
      pinCoil1 = 39; //Pin for coil 1
      pinCoil2 = 41; //Pin for coil 2
      pinCoil3 = 32; //Pin for coil 3
      pinCoil4 = 33; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A5; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A3; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
      pinIdle1 = 2; //Single wire idle control
      pinBoost = 4;
      pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pinFuelPump = 37; //Fuel pump output
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinFan = 35; //Pin for the fan output
      pinLaunch = 12; //Can be overwritten below
      pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 44; //Reset control output

      break;

    case 10:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings for user turtanas PCB
      pinInjector1 = 4; //Output pin injector 1 is on
      pinInjector2 = 5; //Output pin injector 2 is on
      pinInjector3 = 6; //Output pin injector 3 is on
      pinInjector4 = 7; //Output pin injector 4 is on
      pinInjector5 = 8; //Placeholder only - NOT USED
      pinInjector6 = 9; //Placeholder only - NOT USED
      pinInjector7 = 10; //Placeholder only - NOT USED
      pinInjector8 = 11; //Placeholder only - NOT USED
      pinCoil1 = 24; //Pin for coil 1
      pinCoil2 = 28; //Pin for coil 2
      pinCoil3 = 36; //Pin for coil 3
      pinCoil4 = 40; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 18; //The CAS pin
      pinTrigger2 = 19; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinMAP2 = A8; //MAP2 sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A4; //O2 Sensor pin
      pinBat = A7; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinSpareTemp1 = A6;
      pinSpareTemp2 = A5;
      pinTachOut = 41; //Tacho output pin transistori puuttuu 2n2222 tähän ja 1k 12v
      pinFuelPump = 42; //Fuel pump output 2n2222
      pinFan = 47; //Pin for the fan output
      pinTachOut = 49; //Tacho output pin
      pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 26; //Reset control output

    #endif
      break;

    case 20:
//    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
//      //Pin mappings as per the Plazomat In/Out shields Rev 0.1
//      pinInjector1 = 8; //Output pin injector 1 is on
//      pinInjector2 = 9; //Output pin injector 2 is on
//      pinInjector3 = 10; //Output pin injector 3 is on
//      pinInjector4 = 11; //Output pin injector 4 is on
//      pinInjector5 = 12; //Output pin injector 5 is on
//      pinCoil1 = 28; //Pin for coil 1
//      pinCoil2 = 24; //Pin for coil 2
//      pinCoil3 = 40; //Pin for coil 3
//      pinCoil4 = 36; //Pin for coil 4
//      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
//      pinSpareOut1 = 4; //Spare LSD Output 1(PWM)
//      pinSpareOut2 = 5; //Spare LSD Output 2(PWM)
//      pinSpareOut3 = 6; //Spare LSD Output 3(PWM)
//      pinSpareOut4 = 7; //Spare LSD Output 4(PWM)
//      pinSpareOut5 = 50; //Spare LSD Output 5(digital)
//      pinSpareOut6 = 52; //Spare LSD Output 6(digital)
//      pinTrigger = 20; //The CAS pin
//      pinTrigger2 = 21; //The Cam Sensor pin
//      pinSpareTemp2 = A15; //spare Analog input 2
//      pinSpareTemp1 = A14; //spare Analog input 1
//      pinO2 = A8; //O2 Sensor pin
//      pinBat = A4; //Battery reference voltage pin
//      pinMAP = A3; //MAP sensor pin
//      pinTPS = A2;//TPS input pin
//      pinCLT = A1; //CLS sensor pin
//      pinIAT = A0; //IAT sensor pin
//      pinFan = 47; //Pin for the fan output
//      pinFuelPump = 4; //Fuel pump output
//      pinTachOut = 49; //Tacho output pin
//      pinResetControl = 26; //Reset control output
//    #endif
      break;

    case 30:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the dazv6 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 40; //Pin for coil 1
      pinCoil2 = 38; //Pin for coil 2
      pinCoil3 = 50; //Pin for coil 3
      pinCoil4 = 52; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTrigger3 = 17; // cam sensor 2 pin
      pinTPS = A2;//TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A8; //O2 Sensor pin
      pinO2_2 = A9; //O2 sensor pin (second sensor)
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      pinFuelPump = 45; //Fuel pump output
      pinStepperDir = 20; //Direction pin  for DRV8825 driver
      pinStepperStep = 21; //Step pin for DRV8825 driver
      pinSpareHOut1 = 4; // high current output spare1
      pinSpareHOut2 = 6; // high current output spare2
      pinBoost = 7;
      pinSpareLOut1 = 43; //low current output spare1
      pinSpareLOut2 = 47;
      pinSpareLOut3 = 49;
      pinSpareLOut4 = 51;
      pinSpareLOut5 = 53;
      pinFan = 47; //Pin for the fan output
    #endif
      break;

    case 40:
      //Pin mappings as per the NO2C shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 11; //Output pin injector 3 is on - NOT USED
      pinInjector4 = 12; //Output pin injector 4 is on - NOT USED
      pinInjector5 = 13; //Placeholder only - NOT USED
      pinCoil1 = 23; //Pin for coil 1
      pinCoil2 = 22; //Pin for coil 2
      pinCoil3 = 2; //Pin for coil 3 - ONLY WITH DB2
      pinCoil4 = 3; //Pin for coil 4 - ONLY WITH DB2
      pinCoil5 = 46; //Placeholder only - NOT USED
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A3; //TPS input pin
      pinMAP = A0; //MAP sensor pin
      pinIAT = A5; //IAT sensor pin
      pinCLT = A4; //CLT sensor pin
      pinO2 = A2; //O2 sensor pin
      pinBat = A1; //Battery reference voltage pin
      pinBaro = A6; //Baro sensor pin - ONLY WITH DB
      pinSpareTemp1 = A7; //spare Analog input 1 - ONLY WITH DB
      pinDisplayReset = 48; // OLED reset pin - NOT USED
      pinTachOut = 38; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      pinIdle2 = 47; //2 wire idle control - NOT USED
      pinBoost = 7; //Boost control
      pinVVT_1 = 6; //Default VVT output
      pinFuelPump = 4; //Fuel pump output
      pinStepperDir = 25; //Direction pin for DRV8825 driver
      pinStepperStep = 24; //Step pin for DRV8825 driver
      pinStepperEnable = 27; //Enable pin for DRV8825 driver
      pinLaunch = 10; //Can be overwritten below
      pinFlex = 20; // Flex sensor (Must be external interrupt enabled) - ONLY WITH DB
      pinFan = 30; //Pin for the fan output - ONLY WITH DB
      pinSpareLOut1 = 32; //low current output spare1 - ONLY WITH DB
      pinSpareLOut2 = 34; //low current output spare2 - ONLY WITH DB
      pinSpareLOut3 = 36; //low current output spare3 - ONLY WITH DB
      pinResetControl = 26; //Reset control output
      break;

    case 41:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the UA4C shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 7; //Output pin injector 2 is on
      pinInjector3 = 6; //Output pin injector 3 is on
      pinInjector4 = 5; //Output pin injector 4 is on
      pinInjector5 = 45; //Output pin injector 5 is on PLACEHOLDER value for now
      pinCoil1 = 35; //Pin for coil 1
      pinCoil2 = 36; //Pin for coil 2
      pinCoil3 = 33; //Pin for coil 3
      pinCoil4 = 34; //Pin for coil 4
      pinCoil5 = 44; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinFlex = 20; // Flex sensor
      pinTPS = A3; //TPS input pin
      pinMAP = A0; //MAP sensor pin
      pinBaro = A7; //Baro sensor pin
      pinIAT = A5; //IAT sensor pin
      pinCLT = A4; //CLS sensor pin
      pinO2 = A1; //O2 Sensor pin
      pinO2_2 = A9; //O2 sensor pin (second sensor)
      pinBat = A2; //Battery reference voltage pin
      pinSpareTemp1 = A8; //spare Analog input 1
      pinLaunch = 37; //Can be overwritten below
      pinDisplayReset = 48; // OLED reset pin PLACEHOLDER value for now
      pinTachOut = 22; //Tacho output pin
      pinIdle1 = 9; //Single wire idle control
      pinIdle2 = 10; //2 wire idle control
      pinFuelPump = 23; //Fuel pump output
      pinVVT_1 = 11; //Default VVT output
      pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pinStepperStep = 31; //Step pin for DRV8825 driver
      pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pinBoost = 12; //Boost control
      pinSpareLOut1 = 26; //low current output spare1
      pinSpareLOut2 = 27; //low current output spare2
      pinSpareLOut3 = 28; //low current output spare3
      pinSpareLOut4 = 29; //low current output spare4
      pinFan = 24; //Pin for the fan output
      pinResetControl = 46; //Reset control output PLACEHOLDER value for now
    #endif
      break;

    default:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.2 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 28; //Pin for coil 1
      pinCoil2 = 24; //Pin for coil 2
      pinCoil3 = 40; //Pin for coil 3
      pinCoil4 = 36; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 20; //The CAS pin
      pinTrigger2 = 21; //The Cam Sensor pin
      pinTPS = A2; //TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A8; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinDisplayReset = 48; // OLED reset pin
      pinFan = 47; //Pin for the fan output
      pinFuelPump = 4; //Fuel pump output
      pinTachOut = 49; //Tacho output pin
      pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pinBoost = 5;
      pinIdle1 = 6;
      pinResetControl = 43; //Reset control output
    #endif
      break;
  }

  //Setup any devices that are using selectable pins

  if ( (configPage6.launchPin != 0) && (configPage6.launchPin < BOARD_NR_GPIO_PINS) ) { pinLaunch = pinTranslate(configPage6.launchPin); }
  if ( (configPage4.ignBypassPin != 0) && (configPage4.ignBypassPin < BOARD_NR_GPIO_PINS) ) { pinIgnBypass = pinTranslate(configPage4.ignBypassPin); }
  if ( (configPage2.tachoPin != 0) && (configPage2.tachoPin < BOARD_NR_GPIO_PINS) ) { pinTachOut = pinTranslate(configPage2.tachoPin); }
  if ( (configPage4.fuelPumpPin != 0) && (configPage4.fuelPumpPin < BOARD_NR_GPIO_PINS) ) { pinFuelPump = pinTranslate(configPage4.fuelPumpPin); }
  if ( (configPage6.fanPin != 0) && (configPage6.fanPin < BOARD_NR_GPIO_PINS) ) { pinFan = pinTranslate(configPage6.fanPin); }
  if ( (configPage6.boostPin != 0) && (configPage6.boostPin < BOARD_NR_GPIO_PINS) ) { pinBoost = pinTranslate(configPage6.boostPin); }
  if ( (configPage6.vvtPin != 0) && (configPage6.vvtPin < BOARD_NR_GPIO_PINS) ) { pinVVT_1 = pinTranslate(configPage6.vvtPin); }
  if ( (configPage6.useExtBaro != 0) && (configPage6.baroPin < BOARD_NR_GPIO_PINS) ) { pinBaro = configPage6.baroPin + A0; }
  if ( (configPage6.useEMAP != 0) && (configPage10.EMAPPin < BOARD_NR_GPIO_PINS) ) { pinEMAP = configPage10.EMAPPin + A0; }

  //Currently there's no default pin for Idle Up
  pinIdleUp = pinTranslate(configPage2.idleUpPin);

  /* Reset control is a special case. If reset control is enabled, it needs its initial state set BEFORE its pinMode.
     If that doesn't happen and reset control is in "Serial Command" mode, the Arduino will end up in a reset loop
     because the control pin will go low as soon as the pinMode is set to OUTPUT. */
  if ( (configPage4.resetControl != 0) && (configPage4.resetControlPin < BOARD_NR_GPIO_PINS) )
  {
    resetControl = configPage4.resetControl;
    pinResetControl = pinTranslate(configPage4.resetControlPin);
    setResetControlPinState();
    pinMode(pinResetControl, OUTPUT);
  }

  //Finally, set the relevant pin modes for outputs
  pinMode(pinCoil1, OUTPUT);
  pinMode(pinCoil2, OUTPUT);
  pinMode(pinCoil3, OUTPUT);
  pinMode(pinCoil4, OUTPUT);
  pinMode(pinCoil5, OUTPUT);
  pinMode(pinInjector1, OUTPUT);
  pinMode(pinInjector2, OUTPUT);
  pinMode(pinInjector3, OUTPUT);
  pinMode(pinInjector4, OUTPUT);
  pinMode(pinInjector5, OUTPUT);
  pinMode(pinTachOut, OUTPUT);
  pinMode(pinIdle1, OUTPUT);
  pinMode(pinIdle2, OUTPUT);
  pinMode(pinFuelPump, OUTPUT);
  pinMode(pinIgnBypass, OUTPUT);
  pinMode(pinFan, OUTPUT);
  pinMode(pinStepperDir, OUTPUT);
  pinMode(pinStepperStep, OUTPUT);
  pinMode(pinStepperEnable, OUTPUT);
  pinMode(pinBoost, OUTPUT);
  pinMode(pinVVT_1, OUTPUT);

  inj1_pin_port = portOutputRegister(digitalPinToPort(pinInjector1));
  inj1_pin_mask = digitalPinToBitMask(pinInjector1);
  inj2_pin_port = portOutputRegister(digitalPinToPort(pinInjector2));
  inj2_pin_mask = digitalPinToBitMask(pinInjector2);
  inj3_pin_port = portOutputRegister(digitalPinToPort(pinInjector3));
  inj3_pin_mask = digitalPinToBitMask(pinInjector3);
  inj4_pin_port = portOutputRegister(digitalPinToPort(pinInjector4));
  inj4_pin_mask = digitalPinToBitMask(pinInjector4);
  inj5_pin_port = portOutputRegister(digitalPinToPort(pinInjector5));
  inj5_pin_mask = digitalPinToBitMask(pinInjector5);
  inj6_pin_port = portOutputRegister(digitalPinToPort(pinInjector6));
  inj6_pin_mask = digitalPinToBitMask(pinInjector6);
  inj7_pin_port = portOutputRegister(digitalPinToPort(pinInjector7));
  inj7_pin_mask = digitalPinToBitMask(pinInjector7);
  inj8_pin_port = portOutputRegister(digitalPinToPort(pinInjector8));
  inj8_pin_mask = digitalPinToBitMask(pinInjector8);

  ign1_pin_port = portOutputRegister(digitalPinToPort(pinCoil1));
  ign1_pin_mask = digitalPinToBitMask(pinCoil1);
  ign2_pin_port = portOutputRegister(digitalPinToPort(pinCoil2));
  ign2_pin_mask = digitalPinToBitMask(pinCoil2);
  ign3_pin_port = portOutputRegister(digitalPinToPort(pinCoil3));
  ign3_pin_mask = digitalPinToBitMask(pinCoil3);
  ign4_pin_port = portOutputRegister(digitalPinToPort(pinCoil4));
  ign4_pin_mask = digitalPinToBitMask(pinCoil4);
  ign5_pin_port = portOutputRegister(digitalPinToPort(pinCoil5));
  ign5_pin_mask = digitalPinToBitMask(pinCoil5);
  ign6_pin_port = portOutputRegister(digitalPinToPort(pinCoil6));
  ign6_pin_mask = digitalPinToBitMask(pinCoil6);
  ign7_pin_port = portOutputRegister(digitalPinToPort(pinCoil7));
  ign7_pin_mask = digitalPinToBitMask(pinCoil7);
  ign8_pin_port = portOutputRegister(digitalPinToPort(pinCoil8));
  ign8_pin_mask = digitalPinToBitMask(pinCoil8);

  tach_pin_port = portOutputRegister(digitalPinToPort(pinTachOut));
  tach_pin_mask = digitalPinToBitMask(pinTachOut);
  pump_pin_port = portOutputRegister(digitalPinToPort(pinFuelPump));
  pump_pin_mask = digitalPinToBitMask(pinFuelPump);

  //And for inputs
  #if defined(CORE_STM32)
    #ifndef ARDUINO_ARCH_STM32 //libmaple core aka STM32DUINO
      pinMode(pinMAP, INPUT_ANALOG);
      pinMode(pinO2, INPUT_ANALOG);
      pinMode(pinO2_2, INPUT_ANALOG);
      pinMode(pinTPS, INPUT_ANALOG);
      pinMode(pinIAT, INPUT_ANALOG);
      pinMode(pinCLT, INPUT_ANALOG);
      pinMode(pinBat, INPUT_ANALOG);
      pinMode(pinBaro, INPUT_ANALOG);
    #else
      pinMode(pinMAP, INPUT);
      pinMode(pinO2, INPUT);
      pinMode(pinO2_2, INPUT);
      pinMode(pinTPS, INPUT);
      pinMode(pinIAT, INPUT);
      pinMode(pinCLT, INPUT);
      pinMode(pinBat, INPUT);
      pinMode(pinBaro, INPUT);
    #endif
  #endif
  pinMode(pinTrigger, INPUT);
  pinMode(pinTrigger2, INPUT);
  pinMode(pinTrigger3, INPUT);

  //Each of the below are only set when their relevant function is enabled. This can help prevent pin conflicts that users aren't aware of with unused functions
  if(configPage2.flexEnabled > 0)
  {
    pinMode(pinFlex, INPUT); //Standard GM / Continental flex sensor requires pullup, but this should be onboard. The internal pullup will not work (Requires ~3.3k)!
  }
  if(configPage6.launchEnabled > 0)
  {
    if (configPage6.lnchPullRes == true) { pinMode(pinLaunch, INPUT_PULLUP); }
    else { pinMode(pinLaunch, INPUT); } //If Launch Pull Resistor is not set make input float.
  }
  if(configPage2.idleUpEnabled > 0)
  {
    if (configPage2.idleUpPolarity == 0) { pinMode(pinIdleUp, INPUT_PULLUP); } //Normal setting
    else { pinMode(pinIdleUp, INPUT); } //inverted setting
  }
  

  //These must come after the above pinMode statements
  triggerPri_pin_port = portInputRegister(digitalPinToPort(pinTrigger));
  triggerPri_pin_mask = digitalPinToBitMask(pinTrigger);
  triggerSec_pin_port = portInputRegister(digitalPinToPort(pinTrigger2));
  triggerSec_pin_mask = digitalPinToBitMask(pinTrigger2);

  #if defined(CORE_STM32)
  #else
    //Set default values
    digitalWrite(pinMAP, HIGH);
    //digitalWrite(pinO2, LOW);
    digitalWrite(pinTPS, LOW);
  #endif
}
#endif
