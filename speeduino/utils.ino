/*
  Speeduino - Simple engine management for the Arduino Mega 2560 platform
  Copyright (C) Josh Stewart
  A full copy of the license may be found in the projects root directory
*/


/*
  Returns how much free dynamic memory exists (between heap and stack)
  This function is one big MISRA violation. MISRA advisories forbid directly poking at memory addresses, however there is no other way of determining heap size on embedded systems.
*/
#include "utils.h"

uint16_t freeRam ()
{
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  extern int __heap_start, *__brkval;
  uint16_t v;

  return (uint16_t) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);

#elif defined(CORE_TEENSY)
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
#elif defined(CORE_STM32)
  char top = 't';
  return &top - reinterpret_cast<char*>(sbrk(0));

#endif
}

byte pinTranslate(byte rawPin)
{
  byte outputPin = rawPin;
  if(rawPin > BOARD_DIGITAL_GPIO_PINS) { outputPin = A8 + (outputPin - BOARD_DIGITAL_GPIO_PINS - 1); }

  return outputPin;
}

void setResetControlPinState()
{
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);

  /* Setup reset control initial state */
  switch (resetControl)
  {
    case RESET_CONTROL_PREVENT_WHEN_RUNNING:
      /* Set the reset control pin LOW and change it to HIGH later when we get sync. */
      digitalWrite(pinResetControl, LOW);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      break;
    case RESET_CONTROL_PREVENT_ALWAYS:
      /* Set the reset control pin HIGH and never touch it again. */
      digitalWrite(pinResetControl, HIGH);
      BIT_SET(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      break;
    case RESET_CONTROL_SERIAL_COMMAND:
      /* Set the reset control pin HIGH. There currently isn't any practical difference
         between this and PREVENT_ALWAYS but it doesn't hurt anything to have them separate. */
      digitalWrite(pinResetControl, HIGH);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      break;
  }
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

      #if defined(CORE_TEENSY)
        pinTrigger = 23;
        pinStepperDir = 33;
        pinStepperStep = 34;
        pinCoil1 = 31;
        pinTachOut = 28;
        pinFan = 27;
        pinCoil4 = 21;
        pinCoil3 = 30;
        pinO2 = A22;
      #endif
    #endif
      break;

    case 3:
      //Pin mappings as per the v0.4 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
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

      #if defined(CORE_TEENSY)
        pinTrigger = 23;
        pinTrigger2 = 36;
        pinStepperDir = 34;
        pinStepperStep = 35;
        pinCoil1 = 31;
        pinTachOut = 28;
        pinFan = 27;
        pinCoil4 = 29;
        pinCoil3 = 30;
        pinO2 = A22;
      #elif defined(STM32F4)
        pinInjector1 = PE7; //Output pin injector 1 is on
        pinInjector2 = PE8; //Output pin injector 2 is on
        pinInjector3 = PE9; //Output pin injector 3 is on
        pinInjector4 = PE10; //Output pin injector 4 is on
        pinInjector5 = PE11; //Output pin injector 5 is on
        pinCoil1 = PB10; //Pin for coil 1
        pinCoil2 = PB11; //Pin for coil 2
        pinCoil3 = PB12; //Pin for coil 3
        pinCoil4 = PB13; //Pin for coil 4
        pinCoil5 = PB14; //Pin for coil 5
        pinTPS = A0; //TPS input pin
        pinMAP = A1; //MAP sensor pin
        pinIAT = A2; //IAT sensor pin
        pinCLT = A3; //CLS sensor pin
        pinO2 = A4; //O2 Sensor pin
        pinBat = A5; //Battery reference voltage pin
        pinBaro = A6;
        pinStepperDir = PD8; //Direction pin  for DRV8825 driver
        pinStepperStep = PB15; //Step pin for DRV8825 driver
        pinStepperEnable = PD9; //Enable pin for DRV8825
        pinDisplayReset = PE1; // OLED reset pin
        pinFan = PE2; //Pin for the fan output
        pinFuelPump = PA6; //Fuel pump output
        pinTachOut = PA7; //Tacho output pin
        //external interrupt enabled pins
        pinFlex = PC4; // Flex sensor (Must be external interrupt enabled)
        pinTrigger = PC5; //The CAS pin
        pinTrigger2 = PC6; //The Cam Sensor pin
        pinBoost = PE0; //Boost control
        pinVVT_1 = PE1; //Default VVT output
      #elif defined(CORE_STM32)
        //http://docs.leaflabs.com/static.leaflabs.com/pub/leaflabs/maple-docs/0.0.12/hardware/maple-mini.html#master-pin-map
        //pins 23, 24 and 33 couldn't be used
        pinInjector1 = 15; //Output pin injector 1 is on
        pinInjector2 = 16; //Output pin injector 2 is on
        pinInjector3 = 17; //Output pin injector 3 is on
        pinInjector4 = 18; //Output pin injector 4 is on
        pinCoil1 = 19; //Pin for coil 1
        pinCoil2 = 20; //Pin for coil 2
        pinCoil3 = 21; //Pin for coil 3
        pinCoil4 = 26; //Pin for coil 4
        pinCoil5 = 27; //Pin for coil 5
        pinTPS = A0; //TPS input pin
        pinMAP = A1; //MAP sensor pin
        pinIAT = A2; //IAT sensor pin
        pinCLT = A3; //CLS sensor pin
        pinO2 = A4; //O2 Sensor pin
        pinBat = A5; //Battery reference voltage pin
        pinStepperDir = 12; //Direction pin  for DRV8825 driver
        pinStepperStep = 13; //Step pin for DRV8825 driver
        pinStepperEnable = 14; //Enable pin for DRV8825
        pinDisplayReset = 2; // OLED reset pin
        pinFan = 1; //Pin for the fan output
        pinFuelPump = 0; //Fuel pump output
        pinTachOut = 31; //Tacho output pin
        //external interrupt enabled pins
        pinFlex = 32; // Flex sensor (Must be external interrupt enabled)
        pinTrigger = 25; //The CAS pin
        pinTrigger2 = 22; //The Cam Sensor pin
        pinBaro = pinMAP;
        pinBoost = 1; //Boost control
        pinVVT_1 = 0; //Default VVT output
      #endif
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

      #if defined(CORE_TEENSY)
        pinTrigger = 23;
        pinTrigger2 = 36;
        pinStepperDir = 34;
        pinStepperStep = 35;
        pinCoil1 = 33; //Done
        pinCoil2 = 24; //Done
        pinCoil3 = 51; //Won't work (No mapping for pin 32)
        pinCoil4 = 52; //Won't work (No mapping for pin 33)
        pinFuelPump = 26; //Requires PVT4 adapter or above
        pinFan = 50; //Won't work (No mapping for pin 35)
        pinTachOut = 28; //Done
      #endif
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
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
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
    #endif
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

    case 50:
      //Pin mappings as per the teensy rev A shield
      pinInjector1 = 2; //Output pin injector 1 is on
      pinInjector2 = 10; //Output pin injector 2 is on
      pinInjector3 = 6; //Output pin injector 3 is on - NOT USED
      pinInjector4 = 9; //Output pin injector 4 is on - NOT USED
      //pinInjector5 = 13; //Placeholder only - NOT USED
      pinCoil1 = 29; //Pin for coil 1
      pinCoil2 = 30; //Pin for coil 2
      pinCoil3 = 31; //Pin for coil 3 - ONLY WITH DB2
      pinCoil4 = 32; //Pin for coil 4 - ONLY WITH DB2
      //pinCoil5 = 46; //Placeholder only - NOT USED
      pinTrigger = 23; //The CAS pin
      pinTrigger2 = 36; //The Cam Sensor pin
      pinTPS = 16; //TPS input pin
      pinMAP = 17; //MAP sensor pin
      pinIAT = 14; //IAT sensor pin
      pinCLT = 15; //CLT sensor pin
      pinO2 = A22; //O2 sensor pin
      pinO2_2 = A21; //O2 sensor pin (second sensor)
      pinBat = 18; //Battery reference voltage pin
      //pinBaro = A6; //Baro sensor pin - ONLY WITH DB
      //pinSpareTemp1 = A7; //spare Analog input 1 - ONLY WITH DB
      //pinDisplayReset = 48; // OLED reset pin - NOT USED
      pinTachOut = 20; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      //pinIdle2 = 47; //2 wire idle control - NOT USED
      pinBoost = 11; //Boost control
      //pinVVT_1 = 6; //Default VVT output
      pinFuelPump = 38; //Fuel pump output
      pinStepperDir = 34; //Direction pin for DRV8825 driver
      pinStepperStep = 35; //Step pin for DRV8825 driver
      pinStepperEnable = 33; //Enable pin for DRV8825 driver
      pinLaunch = 26; //Can be overwritten below
      //pinFlex = 20; // Flex sensor (Must be external interrupt enabled) - ONLY WITH DB
      pinFan = 37; //Pin for the fan output - ONLY WITH DB
      //pinSpareLOut1 = 32; //low current output spare1 - ONLY WITH DB
      //pinSpareLOut2 = 34; //low current output spare2 - ONLY WITH DB
      //pinSpareLOut3 = 36; //low current output spare3 - ONLY WITH DB
      //pinResetControl = 26; //Reset control output
      pinSpareHOut1 = 8; // high current output spare1
      pinSpareHOut2 = 7; // high current output spare2
      pinSpareLOut1 = 21; //low current output spare1
      break;

    case 51:
      //Pin mappings as per the teensy revB board shield
 pinInjector1 = 2; //Output pin injector 1 is on
      pinInjector2 = 10; //Output pin injector 2 is on
      pinInjector3 = 6; //Output pin injector 3 is on - NOT USED
      pinInjector4 = 9; //Output pin injector 4 is on - NOT USED
      //pinInjector5 = 13; //Placeholder only - NOT USED
      pinCoil1 = 29; //Pin for coil 1
      pinCoil2 = 30; //Pin for coil 2
      pinCoil3 = 31; //Pin for coil 3 - ONLY WITH DB2
      pinCoil4 = 32; //Pin for coil 4 - ONLY WITH DB2
      //pinCoil5 = 46; //Placeholder only - NOT USED
      pinTrigger = 23; //The CAS pin
      pinTrigger2 = 36; //The Cam Sensor pin
      pinTPS = 16; //TPS input pin
      pinMAP = 17; //MAP sensor pin
      pinIAT = 14; //IAT sensor pin
      pinCLT = 15; //CLT sensor pin
      pinO2 = A22; //O2 sensor pin
      pinO2_2 = A21; //O2 sensor pin (second sensor)
      pinBat = 18; //Battery reference voltage pin
      //pinBaro = A6; //Baro sensor pin - ONLY WITH DB
      //pinSpareTemp1 = A7; //spare Analog input 1 - ONLY WITH DB
      //pinDisplayReset = 48; // OLED reset pin - NOT USED
      pinTachOut = 20; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      //pinIdle2 = 47; //2 wire idle control - NOT USED
      pinBoost = 11; //Boost control
      //pinVVT_1 = 6; //Default VVT output
      pinFuelPump = 38; //Fuel pump output
      pinStepperDir = 34; //Direction pin for DRV8825 driver
      pinStepperStep = 35; //Step pin for DRV8825 driver
      pinStepperEnable = 33; //Enable pin for DRV8825 driver
      pinLaunch = 26; //Can be overwritten below
      //pinFlex = 20; // Flex sensor (Must be external interrupt enabled) - ONLY WITH DB
      pinFan = 37; //Pin for the fan output - ONLY WITH DB
      //pinSpareLOut1 = 32; //low current output spare1 - ONLY WITH DB
      //pinSpareLOut2 = 34; //low current output spare2 - ONLY WITH DB
      //pinSpareLOut3 = 36; //low current output spare3 - ONLY WITH DB
      //pinResetControl = 26; //Reset control output
      pinSpareHOut1 = 8; // high current output spare1
      pinSpareHOut2 = 7; // high current output spare2
      pinSpareLOut1 = 21; //low current output spare1
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

  tach_pin_port = portOutputRegister(digitalPinToPort(pinTachOut));
  tach_pin_mask = digitalPinToBitMask(pinTachOut);
  pump_pin_port = portOutputRegister(digitalPinToPort(pinFuelPump));
  pump_pin_mask = digitalPinToBitMask(pinFuelPump);

  //And for inputs
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
  pinMode(pinFlex, INPUT_PULLUP); //Standard GM / Continental flex sensor requires pullup
  if (configPage6.lnchPullRes == true) {
    pinMode(pinLaunch, INPUT_PULLUP);
  }
  else {
    pinMode(pinLaunch, INPUT);  //If Launch Pull Resistor is not set make input float.
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

void initialiseTriggers()
{
  byte triggerInterrupt = 0; // By default, use the first interrupt
  byte triggerInterrupt2 = 1;

  #if defined(CORE_AVR)
    switch (pinTrigger) {
      //Arduino Mega 2560 mapping
      case 2:
        triggerInterrupt = 0; break;
      case 3:
        triggerInterrupt = 1; break;
      case 18:
        triggerInterrupt = 5; break;
      case 19:
        triggerInterrupt = 4; break;
      case 20:
        triggerInterrupt = 3; break;
      case 21:
        triggerInterrupt = 2; break;
      default:
        triggerInterrupt = 0; break; //This should NEVER happen
    }
  #else
    triggerInterrupt = pinTrigger;
  #endif

  #if defined(CORE_AVR)
    switch (pinTrigger2) {
      //Arduino Mega 2560 mapping
      case 2:
        triggerInterrupt2 = 0; break;
      case 3:
        triggerInterrupt2 = 1; break;
      case 18:
        triggerInterrupt2 = 5; break;
      case 19:
        triggerInterrupt2 = 4; break;
      case 20:
        triggerInterrupt2 = 3; break;
      case 21:
        triggerInterrupt2 = 2; break;
      default:
        triggerInterrupt2 = 0; break; //This should NEVER happen
    }
  #else
    triggerInterrupt2 = pinTrigger2;
  #endif

  pinMode(pinTrigger, INPUT);
  pinMode(pinTrigger2, INPUT);
  pinMode(pinTrigger3, INPUT);
  //digitalWrite(pinTrigger, HIGH);
  detachInterrupt(triggerInterrupt);
  detachInterrupt(triggerInterrupt2);

  //Set the trigger function based on the decoder in the config
  switch (configPage4.TrigPattern)
  {
    case 0:
      //Missing tooth decoder
      triggerSetup_missingTooth();
      trigger = triggerPri_missingTooth;
      triggerSecondary = triggerSec_missingTooth;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;
      triggerSetEndTeeth = triggerSetEndTeeth_missingTooth;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      if(configPage4.TrigEdgeSec == 0) { attachInterrupt(triggerInterrupt2, triggerSec_missingTooth, RISING); }
      else { attachInterrupt(triggerInterrupt2, triggerSec_missingTooth, FALLING); }
      break;

    case 1:
      // Basic distributor
      triggerSetup_BasicDistributor();
      trigger = triggerPri_BasicDistributor;
      getRPM = getRPM_BasicDistributor;
      getCrankAngle = getCrankAngle_BasicDistributor;
      triggerSetEndTeeth = triggerSetEndTeeth_BasicDistributor;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      break;

    case 2:
      triggerSetup_DualWheel();
      trigger = triggerPri_DualWheel;
      getRPM = getRPM_DualWheel;
      getCrankAngle = getCrankAngle_DualWheel;
      triggerSetEndTeeth = triggerSetEndTeeth_DualWheel;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      if(configPage4.TrigEdgeSec == 0) { attachInterrupt(triggerInterrupt2, triggerSec_DualWheel, RISING); }
      else { attachInterrupt(triggerInterrupt2, triggerSec_DualWheel, FALLING); }
      break;

    case 3:
      triggerSetup_GM7X();
      trigger = triggerPri_GM7X;
      getRPM = getRPM_GM7X;
      getCrankAngle = getCrankAngle_GM7X;
      triggerSetEndTeeth = triggerSetEndTeeth_GM7X;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      break;

    case 4:
      triggerSetup_4G63();
      trigger = triggerPri_4G63;
      getRPM = getRPM_4G63;
      getCrankAngle = getCrankAngle_4G63;
      triggerSetEndTeeth = triggerSetEndTeeth_4G63;

      //These may both need to change, not sure
      if(configPage4.TrigEdge == 0)
      {
        attachInterrupt(triggerInterrupt, trigger, CHANGE);  // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
        attachInterrupt(triggerInterrupt2, triggerSec_4G63, FALLING); //changed
      }
      else
      {
        attachInterrupt(triggerInterrupt, trigger, CHANGE); // Primary trigger connects to
        attachInterrupt(triggerInterrupt2, triggerSec_4G63, FALLING);
      }
      break;

    case 5:
      triggerSetup_24X();
      trigger = triggerPri_24X;
      getRPM = getRPM_24X;
      getCrankAngle = getCrankAngle_24X;
      triggerSetEndTeeth = triggerSetEndTeeth_24X;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); } // Primary trigger connects to
      attachInterrupt(triggerInterrupt2, triggerSec_24X, CHANGE);
      break;

    case 6:
      triggerSetup_Jeep2000();
      trigger = triggerPri_Jeep2000;
      getRPM = getRPM_Jeep2000;
      getCrankAngle = getCrankAngle_Jeep2000;
      triggerSetEndTeeth = triggerSetEndTeeth_Jeep2000;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); } // Primary trigger connects to
      attachInterrupt(triggerInterrupt2, triggerSec_Jeep2000, CHANGE);
      break;

    case 7:
      triggerSetup_Audi135();
      trigger = triggerPri_Audi135;
      getRPM = getRPM_Audi135;
      getCrankAngle = getCrankAngle_Audi135;
      triggerSetEndTeeth = triggerSetEndTeeth_Audi135;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      attachInterrupt(triggerInterrupt2, triggerSec_Audi135, RISING);
      break;

    case 8:
      triggerSetup_HondaD17();
      trigger = triggerPri_HondaD17;
      getRPM = getRPM_HondaD17;
      getCrankAngle = getCrankAngle_HondaD17;
      triggerSetEndTeeth = triggerSetEndTeeth_HondaD17;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); } // Primary trigger connects to
      attachInterrupt(triggerInterrupt2, triggerSec_HondaD17, CHANGE);
      break;

    case 9:
      triggerSetup_Miata9905();
      trigger = triggerPri_Miata9905;
      getRPM = getRPM_Miata9905;
      getCrankAngle = getCrankAngle_Miata9905;
      triggerSetEndTeeth = triggerSetEndTeeth_Miata9905;

      //These may both need to change, not sure
      // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); }
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }

      if(configPage4.TrigEdgeSec == 0) { attachInterrupt(triggerInterrupt2, triggerSec_Miata9905, RISING); }
      else { attachInterrupt(triggerInterrupt2, triggerSec_Miata9905, FALLING); }
      break;

    case 10:
      triggerSetup_MazdaAU();
      trigger = triggerPri_MazdaAU;
      getRPM = getRPM_MazdaAU;
      getCrankAngle = getCrankAngle_MazdaAU;
      triggerSetEndTeeth = triggerSetEndTeeth_MazdaAU;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); } // Primary trigger connects to
      attachInterrupt(triggerInterrupt2, triggerSec_MazdaAU, FALLING);
      break;

    case 11:
      triggerSetup_non360();
      trigger = triggerPri_DualWheel; //Is identical to the dual wheel decoder, so that is used. Same goes for the secondary below
      getRPM = getRPM_non360;
      getCrankAngle = getCrankAngle_non360;
      triggerSetEndTeeth = triggerSetEndTeeth_Non360;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      attachInterrupt(triggerInterrupt2, triggerSec_DualWheel, FALLING); //Note the use of the Dual Wheel trigger function here. No point in having the same code in twice.
      break;

    case 12:
        triggerSetup_Nissan360();
        trigger = triggerPri_Nissan360;
        getRPM = getRPM_Nissan360;
        getCrankAngle = getCrankAngle_Nissan360;
        triggerSetEndTeeth = triggerSetEndTeeth_Nissan360;

        if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
        else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
        attachInterrupt(triggerInterrupt2, triggerSec_Nissan360, CHANGE);
        break;

    case 13:
            triggerSetup_Subaru67();
            trigger = triggerPri_Subaru67;
            getRPM = getRPM_Subaru67;
            getCrankAngle = getCrankAngle_Subaru67;
            triggerSetEndTeeth = triggerSetEndTeeth_Subaru67;

            if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
            else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
            attachInterrupt(triggerInterrupt2, triggerSec_Subaru67, FALLING);
            break;

    case 14:
            triggerSetup_Daihatsu();
            trigger = triggerPri_Daihatsu;
            getRPM = getRPM_Daihatsu;
            getCrankAngle = getCrankAngle_Daihatsu;
            triggerSetEndTeeth = triggerSetEndTeeth_Daihatsu;

            if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
            else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
            //No secondary input required for this pattern
            break;

    case 15:
            triggerSetup_Harley();
            trigger = triggerPri_Harley;
            //triggerSecondary = triggerSec_Harley;
            getRPM = getRPM_Harley;
            getCrankAngle = getCrankAngle_Harley;
            triggerSetEndTeeth = triggerSetEndTeeth_Harley;
            attachInterrupt(triggerInterrupt, trigger, RISING);
            // attachInterrupt(triggerInterrupt2, triggerSec_Harley, FALLING);
            break;

    case 16:
            //36-2-2-2
            triggerSetup_ThirtySixMinus222();
            trigger = triggerPri_ThirtySixMinus222;
            triggerSecondary = triggerSec_ThirtySixMinus222;
            getRPM = getRPM_missingTooth; //This uses the same function as the missing tooth decoder, so no need to duplicate code
            getCrankAngle = getCrankAngle_missingTooth; //This uses the same function as the missing tooth decoder, so no need to duplicate code
            triggerSetEndTeeth = triggerSetEndTeeth_ThirtySixMinus222;

            if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
            else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
            if(configPage4.TrigEdgeSec == 0) { attachInterrupt(triggerInterrupt2, triggerSecondary, RISING); }
            else { attachInterrupt(triggerInterrupt2, triggerSecondary, FALLING); }
            break;

    default:
      trigger = triggerPri_missingTooth;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      break;
  }
}
