
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

    /*
    ***********************************************************************************************************
    * Timers
    */
    //Configure Timer2 for our low-freq interrupt code.
    TCCR2B = 0x00;          //Disbale Timer2 while we set it up
    TCNT2  = 131;           //Preload timer2 with 131 cycles, leaving 125 till overflow. As the timer runs at 125Khz, this causes overflow to occur at 1Khz = 1ms
    TIFR2  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
    TIMSK2 = 0x01;          //Timer2 Set Overflow Interrupt enabled.
    TCCR2A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
    /* Now configure the prescaler to CPU clock divided by 128 = 125Khz */
    TCCR2B |= (1<<CS22)  | (1<<CS20); // Set bits
    TCCR2B &= ~(1<<CS21);             // Clear bit

    //Enable the watchdog timer for 2 second resets (Good reference: https://tushev.org/articles/arduino/5/arduino-and-watchdog-timer)
    //Boooooooooo WDT is currently broken on Mega 2560 bootloaders :(
    //wdt_enable(WDTO_2S);

}

uint16_t freeRam()
{
    extern int __heap_start, *__brkval;
    int currentVal;
    uint16_t v;

    if(__brkval == 0) { currentVal = (int) &__heap_start; }
    else { currentVal = (int) __brkval; }

    //Old version:
    //return (uint16_t) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
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
void setPinMapping(byte boardID)
{
  switch (boardID)
  {
    case 0:
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
      break;
    case 1:
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
    case 2:
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


      break;

    case 20:
      //Pin mappings as per the Plazomat In/Out shields Rev 0.1
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
      pinSpareOut1 = 4; //Spare LSD Output 1(PWM)
      pinSpareOut2 = 5; //Spare LSD Output 2(PWM)
      pinSpareOut3 = 6; //Spare LSD Output 3(PWM)
      pinSpareOut4 = 7; //Spare LSD Output 4(PWM)
      pinSpareOut5 = 50; //Spare LSD Output 5(digital)
      pinSpareOut6 = 52; //Spare LSD Output 6(digital)
      pinTrigger = 20; //The CAS pin
      pinTrigger2 = 21; //The Cam Sensor pin
      pinSpareTemp2 = A15; //spare Analog input 2
      pinSpareTemp1 = A14; //spare Analog input 1
      pinO2 = A8; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinMAP = A3; //MAP sensor pin
      pinTPS = A2;//TPS input pin
      pinCLT = A1; //CLS sensor pin
      pinIAT = A0; //IAT sensor pin
      pinFan = 47; //Pin for the fan output
      pinFuelPump = 4; //Fuel pump output
      pinTachOut = 49; //Tacho output pin
      pinResetControl = 26; //Reset control output
      break;

    case 30:
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
      break;

    default:
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
  
  //Set default values
  digitalWrite(pinMAP, HIGH);
  //digitalWrite(pinO2, LOW);
  digitalWrite(pinTPS, LOW);

}
#endif
