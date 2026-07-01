#include "pinMapping.h"
#include "../../board_definition.h"
#include "../../preprocessor.h"
#include "../../unit_testing.h"

uint8_t pinTranslate(uint8_t rawPin)
{
  uint8_t outputPin = rawPin;
  if(rawPin > BOARD_MAX_DIGITAL_PINS) { outputPin = A8 + (outputPin - BOARD_MAX_DIGITAL_PINS - 1); }

  return outputPin;
}

uint8_t pinTranslateAnalog(uint8_t rawPin)
{
    if (rawPin<_countof(ANALOG_PINS))
    {
        return ANALOG_PINS[rawPin];
    }
    return rawPin;
}

// Pin mappings as per the v0.2 shield
static pinNumbers_t getV02ShieldMapping(void)
{
  pinNumbers_t pins;
  pins.setInjectorPin(0, 8);//Output pin injector 1 is on
  pins.setInjectorPin(1, 9);//Output pin injector 2 is on
  pins.setInjectorPin(2, 10);//Output pin injector 3 is on
  pins.setInjectorPin(3, 11);//Output pin injector 4 is on
  pins.setInjectorPin(4, 12);//Output pin injector 5 is on
  pins.setCoilPin(0, 28);//Pin for coil 1
  pins.setCoilPin(1, 24);//Pin for coil 2
  pins.setCoilPin(2, 40);//Pin for coil 3
  pins.setCoilPin(3, 36);//Pin for coil 4
  pins.setCoilPin(4, 34);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 20; //The CAS pin
  pins.pinTrigger2 = 21; //The Cam Sensor pin
  pins.pinTrigger3 = 3; //The Cam sensor 2 pin
  pins.pinTPS = A2; //TPS input pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A8; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin
  pins.pinIdle1 = 30; //Single wire idle control
  pins.pinIdle2 = 31; //2 wire idle control
  pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 17; //Step pin for DRV8825 driver
  pins.pinFan = 47; //Pin for the fan output
  pins.pinFuelPump = 4; //Fuel pump output
  pins.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 43; //Reset control output

  return pins;
}

TESTABLE_STATIC pinNumbers_t getDefaultPinMapping(void)
{
#if defined(STM32F407xx)
  pinNumbers_t pins;
  //Pin definitions for experimental board Tjeerd 
  //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407

  //******************************************
  //******** PORTA CONNECTIONS *************** 
  //******************************************
  /* = PA0 */ //Wakeup ADC123
  // = PA1;
  // = PA2;
  // = PA3;
  // = PA4;
  /* = PA5; */ //ADC12
  pins.pinFuelPump = PA6; //ADC12 LED_BUILTIN_1
  /* = PA7; */ //ADC12 LED_BUILTIN_2
  pins.setCoilPin(2, PA8);
  /* = PA9 */ //TXD1
  /* = PA10 */ //RXD1
  /* = PA11 */ //(DO NOT USE FOR SPEEDUINO) USB
  /* = PA12 */ //(DO NOT USE FOR SPEEDUINO) USB 
  /* = PA13 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA14 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA15 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK

  //******************************************
  //******** PORTB CONNECTIONS *************** 
  //******************************************
  /* = PB0; */ //(DO NOT USE FOR SPEEDUINO) ADC123 - SPI FLASH CHIP CS pin
  pins.pinBaro = PB1; //ADC12
  /* = PB2; */ //(DO NOT USE FOR SPEEDUINO) BOOT1 
  /* = PB3; */ //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
  /* = PB4; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
  /* = PB5; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
  /* = PB6; */ //NRF_CE
  /* = PB7; */ //NRF_CS
  /* = PB8; */ //NRF_IRQ
  pins.setCoilPin(1, PB9);//
  /* = PB9; */ //
  pins.setCoilPin(3, PB10);//TXD3
  pins.pinIdle1 = PB11; //RXD3
  pins.pinIdle2 = PB12; //
  /* pins.pinBoost = PB12; */ //
  /* = PB13; */ //SPI2_SCK
  /* = PB14; */ //SPI2_MISO
  /* = PB15; */ //SPI2_MOSI

  //******************************************
  //******** PORTC CONNECTIONS *************** 
  //******************************************
  pins.pinMAP = PC0; //ADC123 
  pins.pinTPS = PC1; //ADC123
  pins.pinIAT = PC2; //ADC123
  pins.pinCLT = PC3; //ADC123
  pins.pinO2 = PC4; //ADC12
  pins.pinBat = PC5; //ADC12
  /*pins.pinVVT_1 = PC6; */ //
  /* = PC8; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
  /* = PC9; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
  /* = PC10; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
  /* = PC11; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
  /* = PC12; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
  pins.pinTachOut = PC13; //
  /* = PC14; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
  /* = PC15; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

  //******************************************
  //******** PORTD CONNECTIONS *************** 
  //******************************************
  /* = PD0; */ //CANRX
  /* = PD1; */ //CANTX
  /* = PD2; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
  /* = PD3; */ //
  /* = PD4; */ //
  pins.pinFlex = PD4;
  /* = PD5;*/ //TXD2
  /* = PD6; */ //RXD2
  pins.setCoilPin(0, PD7);//
  /* = PD7; */ //
  /* = PD8; */ //
  pins.setCoilPin(4, PD9);//
  /* = PD10; */ //
  /* = PD11; */ //
  pins.setInjectorPin(0, PD12);//
  pins.setInjectorPin(1, PD13);//
  pins.setInjectorPin(2, PD14);//
  pins.setInjectorPin(3, PD15);//

  //******************************************
  //******** PORTE CONNECTIONS *************** 
  //******************************************
  pins.pinTrigger = PE0; //
  pins.pinTrigger2 = PE1; //
  pins.pinStepperEnable = PE2; //
  /* = PE3; */ //ONBOARD KEY1
  /* = PE4; */ //ONBOARD KEY2
  pins.pinStepperStep = PE5; //
  pins.pinFan = PE6; //
  pins.pinStepperDir = PE7; //
  /* = PE8; */ //
  /* = PE9; */ //
  /* = PE10; */ //
  pins.setInjectorPin(4, PE11); //
  pins.setInjectorPin(5, PE12); //
  /* = PE13; */ //
  /* = PE14; */ //
  /* = PE15; */ //

  return pins;
#else
  return getV02ShieldMapping();
#endif  
}

 //Pin mappings as per the v0.3 shield
static pinNumbers_t getV03ShieldMapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, 8);//Output pin injector 1 is on
  pins.setInjectorPin(1, 9);//Output pin injector 2 is on
  pins.setInjectorPin(2, 10);//Output pin injector 3 is on
  pins.setInjectorPin(3, 11);//Output pin injector 4 is on
  pins.setInjectorPin(4, 12);//Output pin injector 5 is on
  pins.setCoilPin(0, 28);//Pin for coil 1
  pins.setCoilPin(1, 24);//Pin for coil 2
  pins.setCoilPin(2, 40);//Pin for coil 3
  pins.setCoilPin(3, 36);//Pin for coil 4
  pins.setCoilPin(4, 34);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTrigger3 = 3; //The Cam sensor 2 pin
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A8; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin
  pins.pinIdle1 = 5; //Single wire idle control
  pins.pinIdle2 = 53; //2 wire idle control
  pins.pinBoost = 7; //Boost control
  pins.pinVVT_1 = 6; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.pinFuelPump = 4; //Fuel pump output
  pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 17; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 26; //Enable pin for DRV8825
  pins.pinFan = A13; //Pin for the fan output
  pins.pinLaunch = 51; //Can be overwritten below
  pins.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 50; //Reset control output
  pins.pinBaro = A5;
  pins.pinVSS = 20;

#if defined(CORE_TEENSY35)
  pins.pinTrigger = 23;
  pins.pinStepperDir = 33;
  pins.pinStepperStep = 34;
  pins.setCoilPin(0, 31);
  pins.pinTachOut = 28;
  pins.pinFan = 27;
  pins.setCoilPin(3, 21);
  pins.setCoilPin(2, 30);
  pins.pinO2 = A22;
#endif

  return pins;
}

// Pin mappings as per the v0.4 shield
static pinNumbers_t getV04ShieldMapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, 8);//Output pin injector 1 is on
  pins.setInjectorPin(1, 9);//Output pin injector 2 is on
  pins.setInjectorPin(2, 10);//Output pin injector 3 is on
  pins.setInjectorPin(3, 11);//Output pin injector 4 is on
  pins.setInjectorPin(4, 12);//Output pin injector 5 is on
  pins.setInjectorPin(5, 50);//CAUTION: Uses the same as Coil 4 below. 
  pins.setCoilPin(0, 40);//Pin for coil 1
  pins.setCoilPin(1, 38);//Pin for coil 2
  pins.setCoilPin(2, 52);//Pin for coil 3
  pins.setCoilPin(3, 50);//Pin for coil 4
  pins.setCoilPin(4, 34);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTrigger3 = 3; //The Cam sensor 2 pin
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A8; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
  pins.pinIdle1 = 5; //Single wire idle control
  pins.pinIdle2 = 6; //2 wire idle control
  pins.pinBoost = 7; //Boost control
  pins.pinVVT_1 = 4; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2803)
  pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 17; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 24; //Enable pin for DRV8825
  pins.pinFan = 47; //Pin for the fan output (Goes to ULN2803)
  pins.pinLaunch = 51; //Can be overwritten below
  pins.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 43; //Reset control output
  pins.pinBaro = A5;
  pins.pinVSS = 20;
  pins.pinWMIEmpty = 46;
  pins.pinWMIIndicator = 44;
  pins.pinWMIEnabled = 42;

#if defined(CORE_TEENSY35)
  pins.setInjectorPin(5, 51);

  pins.pinTrigger = 23;
  pins.pinTrigger2 = 36;
  pins.pinStepperDir = 34;
  pins.pinStepperStep = 35;
  pins.setCoilPin(0, 31);
  pins.setCoilPin(1, 32);
  pins.pinTachOut = 28;
  pins.pinFan = 27;
  pins.setCoilPin(3, 29);
  pins.setCoilPin(2, 30);
  pins.pinO2 = A22;

  //Make sure the CAN pins aren't overwritten
  pins.pinTrigger3 = 54;
  pins.pinVVT_1 = 55;

#elif defined(CORE_TEENSY41)
  //These are only to prevent lockups or weird behaviour on T4.1 when this board is used as the default
  pins.pinBaro = A4; 
  pins.pinMAP = A5;
  pins.pinTPS = A3; //TPS input pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A2; //O2 Sensor pin
  pins.pinBat = A15; //Battery reference voltage pin. Needs Alpha4+
  pins.pinLaunch = 34; //Can be overwritten below
  pins.pinVSS = 35;

  pins.pinTrigger = 20; //The CAS pin
  pins.pinTrigger2 = 21; //The Cam Sensor pin
  pins.pinTrigger3 = 24;

  pins.pinStepperDir = 34;
  pins.pinStepperStep = 35;
  
  pins.setCoilPin(0, 31);
  pins.setCoilPin(1, 32);
  pins.setCoilPin(3, 29);
  pins.setCoilPin(2, 30);

  pins.pinTachOut = 28;
  pins.pinFan = 27;
  pins.pinFuelPump = 33;
  pins.pinWMIEmpty = 34;
  pins.pinWMIIndicator = 35;
  pins.pinWMIEnabled = 36;
#elif defined(STM32F407xx)
//Pin definitions for experimental board Tjeerd 
  //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407

  //******************************************
  //******** PORTA CONNECTIONS *************** 
  //******************************************
  /* = PA0 */ //Wakeup ADC123
  // = PA1;
  // = PA2;
  // = PA3;
  // = PA4;
  /* = PA5; */ //ADC12
  /* = PA6; */ //ADC12 LED_BUILTIN_1
  pins.pinFuelPump = PA7; //ADC12 LED_BUILTIN_2
  pins.setCoilPin(2, PA8);
  /* = PA9 */ //TXD1
  /* = PA10 */ //RXD1
  /* = PA11 */ //(DO NOT USE FOR SPEEDUINO) USB
  /* = PA12 */ //(DO NOT USE FOR SPEEDUINO) USB 
  /* = PA13 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA14 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA15 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK

  //******************************************
  //******** PORTB CONNECTIONS *************** 
  //******************************************
  /* = PB0; */ //(DO NOT USE FOR SPEEDUINO) ADC123 - SPI FLASH CHIP CS pin
  pins.pinBaro = PB1; //ADC12
  /* = PB2; */ //(DO NOT USE FOR SPEEDUINO) BOOT1 
  /* = PB3; */ //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
  /* = PB4; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
  /* = PB5; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
  /* = PB6; */ //NRF_CE
  /* = PB7; */ //NRF_CS
  /* = PB8; */ //NRF_IRQ
  pins.setCoilPin(1, PB9);//
  /* = PB9; */ //
  pins.setCoilPin(3, PB10);//TXD3
  pins.pinIdle1 = PB11; //RXD3
  pins.pinIdle2 = PB12; //
  pins.pinBoost = PB12; //
  /* = PB13; */ //SPI2_SCK
  /* = PB14; */ //SPI2_MISO
  /* = PB15; */ //SPI2_MOSI

  //******************************************
  //******** PORTC CONNECTIONS *************** 
  //******************************************
  pins.pinMAP = PC0; //ADC123 
  pins.pinTPS = PC1; //ADC123
  pins.pinIAT = PC2; //ADC123
  pins.pinCLT = PC3; //ADC123
  pins.pinO2 = PC4;  //ADC12
  pins.pinBat = PC5; //ADC12
  pins.pinVVT_1 = PC6; //
  /* = PC8; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
  /* = PC9; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
  /* = PC10; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
  /* = PC11; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
  /* = PC12; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
  pins.pinTachOut = PC13; //
  /* = PC14; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
  /* = PC15; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

  //******************************************
  //******** PORTD CONNECTIONS *************** 
  //******************************************
  /* = PD0; */ //CANRX
  /* = PD1; */ //CANTX
  /* = PD2; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
  pins.pinVVT_2 = PD3; //
  pins.pinFlex = PD4;
  /* = PD5;*/ //TXD2
  /* = PD6; */ //RXD2
  pins.setCoilPin(0, PD7);//
  /* = PD8; */ //
  pins.setCoilPin(4, PD9);//
  /* = PD10; */ //
  /* = PD11; */ //
  pins.setInjectorPin(0, PD12);//
  pins.setInjectorPin(1, PD13);//
  pins.setInjectorPin(2, PD14);//
  pins.setInjectorPin(3, PD15);//

  //******************************************
  //******** PORTE CONNECTIONS *************** 
  //******************************************
  pins.pinTrigger = PE0; //
  pins.pinTrigger2 = PE1; //
  pins.pinStepperEnable = PE2; //
  /* = PE3; */ //ONBOARD KEY1
  /* = PE4; */ //ONBOARD KEY2
  pins.pinStepperStep = PE5; //
  pins.pinFan = PE6; //
  pins.pinStepperDir = PE7; //
  /* = PE8; */ //
  /* = PE9; */ //
  /* = PE10; */ //
  pins.setInjectorPin(4, PE11);//
  pins.setInjectorPin(5, PE12);//
  /* = PE13; */ //
  /* = PE14; */ //
  /* = PE15; */ //

#elif defined(CORE_STM32)
  //https://github.com/stm32duino/Arduino_Core_STM32/blob/master/variants/Generic_F411Cx/variant.h#L28
  //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
  //pins PB12, PB13, PB14 and PB15 are used to SPI FLASH
  //PB2 can't be used as input because it's the BOOT pin
  pins.setInjectorPin(0, PB7);//Output pin injector 1 is on
  pins.setInjectorPin(1, PB6);//Output pin injector 2 is on
  pins.setInjectorPin(2, PB5);//Output pin injector 3 is on
  pins.setInjectorPin(3, PB4);//Output pin injector 4 is on
  pins.setCoilPin(0, PB9);//Pin for coil 1
  pins.setCoilPin(1, PB8);//Pin for coil 2
  pins.setCoilPin(2, PB3);//Pin for coil 3
  pins.setCoilPin(3, PA15);//Pin for coil 4
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A8; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinBaro = pins.pinMAP;
  pins.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
  pins.pinIdle1 = PB2; //Single wire idle control
  pins.pinIdle2 = PB10; //2 wire idle control
  pins.pinBoost = PA6; //Boost control
  pins.pinStepperDir = PB10; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = PB2; //Step pin for DRV8825 driver
  pins.pinFuelPump = PA8; //Fuel pump output
  pins.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)
  //external interrupt enabled pins
  pins.pinFlex = PC14; // Flex sensor (Must be external interrupt enabled)
  pins.pinTrigger = PC13; //The CAS pin also led pin so bad idea
  pins.pinTrigger2 = PC15; //The Cam Sensor pin
#endif

  return pins;
}

 //Pin mappings as per the 2001-05 MX5 PNP shield
static pinNumbers_t getMiataNB2Mapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, 44);//Output pin injector 1 is on
  pins.setInjectorPin(1, 46);//Output pin injector 2 is on
  pins.setInjectorPin(2, 47);//Output pin injector 3 is on
  pins.setInjectorPin(3, 45);//Output pin injector 4 is on
  pins.setInjectorPin(4, 14);//Output pin injector 5 is on
  pins.setCoilPin(0, 42);//Pin for coil 1
  pins.setCoilPin(1, 43);//Pin for coil 2
  pins.setCoilPin(2, 32);//Pin for coil 3
  pins.setCoilPin(3, 33);//Pin for coil 4
  pins.setCoilPin(4, 34);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTrigger3 = 2; //The Cam sensor 2 pin
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A5; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A3; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinTachOut = 23; //Tacho output pin  (Goes to ULN2803)
  pins.pinIdle1 = 5; //Single wire idle control
  pins.pinBoost = 4;
  pins.pinVVT_1 = 11; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
  pins.pinFuelPump = 40; //Fuel pump output
  pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 17; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 24;
  pins.pinFan = 41; //Pin for the fan output
  pins.pinLaunch = 12; //Can be overwritten below
  pins.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 39; //Reset control output
  pins.pinVSS = 2;

  //This is NOT correct. It has not yet been tested with this board
#if defined(CORE_TEENSY35)
  pins.pinTrigger = 23;
  pins.pinTrigger2 = 36;
  pins.pinStepperDir = 34;
  pins.pinStepperStep = 35;
  pins.setCoilPin(0, 33);//Done
  pins.setCoilPin(1, 24);//Done
  pins.setCoilPin(2, 51);//Won't work (No mapping for pin 32)
  pins.setCoilPin(3, 52);//Won't work (No mapping for pin 33)
  pins.pinFuelPump = 26; //Requires PVT4 adapter or above
  pins.pinFan = 50; //Won't work (No mapping for pin 35)
  pins.pinTachOut = 28; //Done
#endif

  return pins;
}

// Pin mappings as per the 1996-97 MX5 PNP shield
static pinNumbers_t getMiataNA18Mapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, 11);//Output pin injector 1 is on
  pins.setInjectorPin(1, 10);//Output pin injector 2 is on
  pins.setInjectorPin(2, 9);//Output pin injector 3 is on
  pins.setInjectorPin(3, 8);//Output pin injector 4 is on
  pins.setInjectorPin(4, 14);//Output pin injector 5 is on
  pins.setCoilPin(0, 39);//Pin for coil 1
  pins.setCoilPin(1, 41);//Pin for coil 2
  pins.setCoilPin(2, 32);//Pin for coil 3
  pins.setCoilPin(3, 33);//Pin for coil 4
  pins.setCoilPin(4, 34);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A5; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A3; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinTachOut = A9; //Tacho output pin  (Goes to ULN2803)
  pins.pinIdle1 = 2; //Single wire idle control
  pins.pinBoost = 4;
  pins.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
  pins.pinFuelPump = 49; //Fuel pump output
  pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 17; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 24;
  pins.pinFan = 35; //Pin for the fan output
  pins.pinLaunch = 37; //Can be overwritten below
  pins.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 44; //Reset control output

  //This is NOT correct. It has not yet been tested with this board
#if defined(CORE_TEENSY35)
  pins.pinTrigger = 23;
  pins.pinTrigger2 = 36;
  pins.pinStepperDir = 34;
  pins.pinStepperStep = 35;
  pins.setCoilPin(0, 33);//Done
  pins.setCoilPin(1, 24);//Done
  pins.setCoilPin(2, 51);//Won't work (No mapping for pin 32)
  pins.setCoilPin(3, 52);//Won't work (No mapping for pin 33)
  pins.pinFuelPump = 26; //Requires PVT4 adapter or above
  pins.pinFan = 50; //Won't work (No mapping for pin 35)
  pins.pinTachOut = 28; //Done
#endif

  return pins;
}

// Pin mappings as per the 89-95 MX5 PNP shield
static pinNumbers_t getMiataNA16Mapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, 11);//Output pin injector 1 is on
  pins.setInjectorPin(1, 10);//Output pin injector 2 is on
  pins.setInjectorPin(2, 9);//Output pin injector 3 is on
  pins.setInjectorPin(3, 8);//Output pin injector 4 is on
  pins.setInjectorPin(4, 14);//Output pin injector 5 is on
  pins.setCoilPin(0, 39);//Pin for coil 1
  pins.setCoilPin(1, 41);//Pin for coil 2
  pins.setCoilPin(2, 32);//Pin for coil 3
  pins.setCoilPin(3, 33);//Pin for coil 4
  pins.setCoilPin(4, 34);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A5; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A3; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
  pins.pinIdle1 = 2; //Single wire idle control
  pins.pinBoost = 4;
  pins.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
  pins.pinFuelPump = 37; //Fuel pump output
  //Note that there is no stepper driver output on the PNP boards. These pins are unconnected and remain here just to prevent issues with random pin numbers occurring
  pins.pinStepperEnable = 15; //Enable pin for the DRV8825
  pins.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 17; //Step pin for DRV8825 driver
  pins.pinFan = 35; //Pin for the fan output
  pins.pinLaunch = 12; //Can be overwritten below
  pins.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 44; //Reset control output
  pins.pinVSS = 20;
  pins.pinIdleUp = 48;
  pins.pinCTPS = 47;
  
#if defined(CORE_TEENSY35)
  pins.pinTrigger = 23;
  pins.pinTrigger2 = 36;
  pins.pinStepperDir = 34;
  pins.pinStepperStep = 35;
  pins.setCoilPin(0, 33);//Done
  pins.setCoilPin(1, 24);//Done
  pins.setCoilPin(2, 51);//Won't work (No mapping for pin 32)
  pins.setCoilPin(3, 52);//Won't work (No mapping for pin 33)
  pins.pinFuelPump = 26; //Requires PVT4 adapter or above
  pins.pinFan = 50; //Won't work (No mapping for pin 35)
  pins.pinTachOut = 28; //Done
#endif

return pins;
}

// Pin mappings for user turtanas PCB
static pinNumbers_t getTurtanasPcbPinMapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, 4);//Output pin injector 1 is on
  pins.setInjectorPin(1, 5);//Output pin injector 2 is on
  pins.setInjectorPin(2, 6);//Output pin injector 3 is on
  pins.setInjectorPin(3, 7);//Output pin injector 4 is on
  pins.setInjectorPin(4, 8);//Placeholder only - NOT USED
  pins.setInjectorPin(5, 9);//Placeholder only - NOT USED
  pins.setInjectorPin(6, 10);//Placeholder only - NOT USED
  pins.setInjectorPin(7, 11);//Placeholder only - NOT USED
  pins.setCoilPin(0, 24);//Pin for coil 1
  pins.setCoilPin(1, 28);//Pin for coil 2
  pins.setCoilPin(2, 36);//Pin for coil 3
  pins.setCoilPin(3, 40);//Pin for coil 4
  pins.setCoilPin(4, 34);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 18; //The CAS pin
  pins.pinTrigger2 = 19; //The Cam Sensor pin
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A4; //O2 Sensor pin
  pins.pinBat = A7; //Battery reference voltage pin
  pins.pinTachOut = 41; //Tacho output pin transistor is missing 2n2222 for this and 1k for 12v
  pins.pinFuelPump = 42; //Fuel pump output 2n2222
  pins.pinFan = 47; //Pin for the fan output
  pins.pinTachOut = 49; //Tacho output pin
  pins.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
  pins.pinResetControl = 26; //Reset control output

  return pins;
}

// Pin mappings for the Levin board
#if defined(STM32F407xx)
#define LEVIN_BOARD_MAPPING
#endif

#if defined(LEVIN_BOARD_MAPPING)
static pinNumbers_t getLevinMapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, PB15);    // Output pin injector 1
  pins.setInjectorPin(1, PA8);     // Output pin injector 2
  pins.setInjectorPin(2, PB13);    // Output pin injector 3
  pins.setInjectorPin(3, PB14);    // Output pin injector 4
  pins.setInjectorPin(4, PE13);    // Output pin injector 5
  pins.setInjectorPin(5, PB12);    // Output pin injector 6
  pins.setInjectorPin(6, PE7);     // Output pin injector 7
  pins.setInjectorPin(7, PE10);    // Output pin injector 8
  pins.setCoilPin(0, PC13);        // Pin for coil 1
  pins.setCoilPin(1, PE6);         // Pin for coil 2
  pins.setCoilPin(2, PE5);         // Pin for coil 3
  pins.setCoilPin(3, PE4);         // Pin for coil 4
  pins.setCoilPin(4, PE3);         // Pin for coil 5
  pins.setCoilPin(5, PE2);         // Pin for coil 6
  pins.setCoilPin(6, PB9);         // Pin for coil 7
  pins.setCoilPin(7, PD12);        // Pin for coil 8
  pins.pinTrigger = PD3;        // The CAS pin
  pins.pinTrigger2 = PD4;       // The Cam Sensor pin
  pins.pinTPS = PA2;            // TPS input pin
  pins.pinMAP = PA3;            // MAP sensor pin
  pins.pinEMAP = PC5;           // EMAP sensor pin (placeholder)
  pins.pinIAT = PA0;            // IAT sensor pin
  pins.pinCLT = PA1;            // CLS sensor pin
  pins.pinO2 = PB0;             // O2 Sensor pin
  pins.pinBat = PA4;            // Battery reference voltage pin
  pins.pinBaro = PA5;           // Baro sensor pin
  pins.pinTachOut = PE8;        // Tacho output pin  (Goes to UNL2803)
  pins.pinIdle1 = PD10;         // ICV pin1  (Goes to UNL2803)
  pins.pinIdle2 = PD9;          // ICV pin3  (Goes to UNL2803)
  pins.pinBoost = PD8;          // Boost control
  pins.pinVVT_1 = PD11;         // VVT1 output (intake vanos)
  pins.pinVVT_2 = PC6;          // VVT2 output (exhaust vanos)
  pins.pinFuelPump = PE11;      // Fuel pump output  (Goes to UNL2803)
  pins.pinStepperDir = PB10;    // Stepper valve isn't used with these
  pins.pinStepperStep = PB11;   // Stepper valve isn't used with these
  pins.pinStepperEnable = PA15; // Stepper valve isn't used with these
  pins.pinFan = PE9;            // Pin for the fan output (Goes to UNL2803)
  pins.pinLaunch = PB8;         // Launch control pin
  pins.pinFlex = PD7;           // Flex sensor
  pins.pinResetControl = PB7;   // Reset control output
  pins.pinVSS = PB6;            // VSS input pin
  pins.pinWMIEmpty = PA6;       //(placeholder)
  pins.pinWMIIndicator = PC3;   //(placeholder)
  pins.pinWMIEnabled = PE15;    //(placeholder)
  pins.pinIdleUp = PC7;         //(placeholder)
  return pins;
}
#endif

#if defined(CORE_AVR) || defined(UNIT_TEST)
#define PLAZOMAT_V01_MAPPING
#endif

#if defined(PLAZOMAT_V01_MAPPING)
//Pin mappings as per the Plazomat In/Out shields Rev 0.1
static pinNumbers_t getPlazomatV01Mapping(void)
{
  pinNumbers_t pins;
  pins.setInjectorPin(0, 8);//Output pin injector 1 is on
  pins.setInjectorPin(1, 9);//Output pin injector 2 is on
  pins.setInjectorPin(2, 10);//Output pin injector 3 is on
  pins.setInjectorPin(3, 11);//Output pin injector 4 is on
  pins.setInjectorPin(4, 12);//Output pin injector 5 is on
  pins.setCoilPin(0, 28);//Pin for coil 1
  pins.setCoilPin(1, 24);//Pin for coil 2
  pins.setCoilPin(2, 40);//Pin for coil 3
  pins.setCoilPin(3, 36);//Pin for coil 4
  pins.setCoilPin(4, 34);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 20; //The CAS pin
  pins.pinTrigger2 = 21; //The Cam Sensor pin
  pins.pinO2 = A8; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinTPS = A2;//TPS input pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinFan = 47; //Pin for the fan output
  pins.pinFuelPump = 4; //Fuel pump output
  pins.pinTachOut = 49; //Tacho output pin
  pins.pinResetControl = 26; //Reset control output

  return pins;
}
#endif

// Pin mappings as per the dazv6 shield
static pinNumbers_t getDazV6V01Mapping(void)
{
  pinNumbers_t pins;
  pins.setInjectorPin(0, 8);//Output pin injector 1 is on
  pins.setInjectorPin(1, 9);//Output pin injector 2 is on
  pins.setInjectorPin(2, 10);//Output pin injector 3 is on
  pins.setInjectorPin(3, 11);//Output pin injector 4 is on
  pins.setInjectorPin(4, 12);//Output pin injector 5 is on
  pins.setCoilPin(0, 40);//Pin for coil 1
  pins.setCoilPin(1, 38);//Pin for coil 2
  pins.setCoilPin(2, 50);//Pin for coil 3
  pins.setCoilPin(3, 52);//Pin for coil 4
  pins.setCoilPin(4, 34);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTrigger3 = 17; // cam sensor 2 pin, pin17 isn't external trigger enabled in arduino mega??
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A8; //O2 Sensor pin
  pins.pinO2_2 = A9; //O2 sensor pin (second sensor)
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinTachOut = 49; //Tacho output pin
  pins.pinIdle1 = 5; //Single wire idle control
  pins.pinFuelPump = 45; //Fuel pump output
  pins.pinStepperDir = 20; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 21; //Step pin for DRV8825 driver
  pins.pinBoost = 7;
  pins.pinFan = 47; //Pin for the fan output

  return pins;
}

#if defined(CORE_AVR) || defined(STM32F407xx) || defined(UNIT_TEST)
#define PAZI_BMW_PNP_MAPPING
#endif

#if defined(PAZI_BMW_PNP_MAPPING)
// Pin mappings for the BMW PnP PCBs by pazi88.
static pinNumbers_t getBmwPnPMapping(void)
{
  pinNumbers_t pins;

#if defined(CORE_AVR) || defined(UNIT_TEST)
  //This is the regular MEGA2560 pin mapping
  pins.setInjectorPin(0, 8);//Output pin injector 1
  pins.setInjectorPin(1, 9);//Output pin injector 2
  pins.setInjectorPin(2, 10);//Output pin injector 3
  pins.setInjectorPin(3, 11);//Output pin injector 4
  pins.setInjectorPin(4, 12);//Output pin injector 5
  pins.setInjectorPin(5, 50);//Output pin injector 6
  pins.setInjectorPin(6, 39);//Output pin injector 7
  pins.setInjectorPin(7, 42);//Output pin injector 8
  pins.setCoilPin(0, 40);//Pin for coil 1
  pins.setCoilPin(1, 38);//Pin for coil 2
  pins.setCoilPin(2, 52);//Pin for coil 3
  pins.setCoilPin(3, 48);//Pin for coil 4
  pins.setCoilPin(4, 36);//Pin for coil 5
  pins.setCoilPin(5, 34);//Pin for coil 6
  pins.setCoilPin(6, 46);//Pin for coil 7
  pins.setCoilPin(7, 53);//Pin for coil 8
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTrigger3 = 20; //The Cam sensor 2 pin
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinEMAP = A15; //EMAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLT sensor pin
  pins.pinO2 = A8; //O2 Sensor pin
  pins.pinO2_2 = A12; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinBaro = A5; //Baro sensor pin
  pins.pinTachOut = 49; //Tacho output pin  (Goes to ULN2003)
  pins.pinIdle1 = 5; //ICV pin1
  pins.pinIdle2 = 6; //ICV pin3
  pins.pinBoost = 7; //Boost control
  pins.pinVVT_1 = 4; //VVT1 output (intake vanos)
  pins.pinVVT_2 = 26; //VVT2 output (exhaust vanos)
  pins.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2003)
  pins.pinStepperDir = 16; //Stepper valve isn't used with these
  pins.pinStepperStep = 17; //Stepper valve isn't used with these
  pins.pinStepperEnable = 24; //Stepper valve isn't used with these
  pins.pinFan = 47; //Pin for the fan output (Goes to ULN2003)
  pins.pinLaunch = 51; //Launch control pin
  pins.pinFlex = 2; // Flex sensor
  pins.pinResetControl = 43; //Reset control output
  pins.pinVSS = 3; //VSS input pin
  pins.pinWMIEmpty = 31; //(placeholder)
  pins.pinWMIIndicator = 33; //(placeholder)
  pins.pinWMIEnabled = 35; //(placeholder)
  pins.pinIdleUp = 37; //(placeholder)
  pins.pinIdleUpOutput = 41; //(placeholder)
  pins.pinCTPS = A6; //(placeholder)
#elif defined(STM32F407xx)
  pins.setInjectorPin(0, PB15);//Output pin injector 1
  pins.setInjectorPin(1, PB14);//Output pin injector 2
  pins.setInjectorPin(2, PB12);//Output pin injector 3
  pins.setInjectorPin(3, PB13);//Output pin injector 4
  pins.setInjectorPin(4, PA8);//Output pin injector 5
  pins.setInjectorPin(5, PE7);//Output pin injector 6
  pins.setInjectorPin(6, PE13);//Output pin injector 7
  pins.setInjectorPin(7, PE10);//Output pin injector 8
  pins.setCoilPin(0, PE2);//Pin for coil 1
  pins.setCoilPin(1, PE3);//Pin for coil 2
  pins.setCoilPin(2, PC13);//Pin for coil 3
  pins.setCoilPin(3, PE6);//Pin for coil 4
  pins.setCoilPin(4, PE4);//Pin for coil 5
  pins.setCoilPin(5, PE5);//Pin for coil 6
  pins.setCoilPin(6, PE0);//Pin for coil 7
  pins.setCoilPin(7, PB9);//Pin for coil 8
  pins.pinTrigger = PD3; //The CAS pin
  pins.pinTrigger2 = PD4; //The Cam Sensor pin
  pins.pinTPS = PA2;//TPS input pin
  pins.pinMAP = PA3; //MAP sensor pin
  pins.pinEMAP = PC5; //EMAP sensor pin
  pins.pinIAT = PA0; //IAT sensor pin
  pins.pinCLT = PA1; //CLS sensor pin
  pins.pinO2 = PB0; //O2 Sensor pin
  pins.pinO2_2 = PC2; //O2 Sensor pin
  pins.pinBat = PA4; //Battery reference voltage pin
  pins.pinBaro = PA5; //Baro sensor pin
  pins.pinTachOut = PE8; //Tacho output pin  (Goes to ULN2003)
  pins.pinIdle1 = PD10; //ICV pin1
  pins.pinIdle2 = PD9; //ICV pin3
  pins.pinBoost = PD8; //Boost control
  pins.pinVVT_1 = PD11; //VVT1 output (intake vanos)
  pins.pinVVT_2 = PC7; //VVT2 output (exhaust vanos)
  pins.pinFuelPump = PE11; //Fuel pump output  (Goes to ULN2003)
  pins.pinStepperDir = PB10; //Stepper valve isn't used with these
  pins.pinStepperStep = PB11; //Stepper valve isn't used with these
  pins.pinStepperEnable = PA15; //Stepper valve isn't used with these
  pins.pinFan = PE9; //Pin for the fan output (Goes to ULN2003)
  pins.pinLaunch = PB8; //Launch control pin
  pins.pinFlex = PD7; // Flex sensor
  pins.pinResetControl = PB7; //Reset control output
  pins.pinVSS = PB6; //VSS input pin
  pins.pinWMIEmpty = PD15; //(placeholder)
  pins.pinWMIIndicator = PD13; //(placeholder)
  pins.pinWMIEnabled = PE15; //(placeholder)
  pins.pinIdleUp = PE14; //(placeholder)
  pins.pinIdleUpOutput = PE12; //(placeholder)
  pins.pinCTPS = PA6; //(placeholder)
#endif
  return pins;
}
#endif

// Pin mappings as per the NO2C shield
static pinNumbers_t getNO2CMapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, 8);//Output pin injector 1 is on
  pins.setInjectorPin(1, 9);//Output pin injector 2 is on
  pins.setInjectorPin(2, 11);//Output pin injector 3 is on - NOT USED
  pins.setInjectorPin(3, 12);//Output pin injector 4 is on - NOT USED
  pins.setInjectorPin(4, 13);//Placeholder only - NOT USED
  pins.setCoilPin(0, 23);//Pin for coil 1
  pins.setCoilPin(1, 22);//Pin for coil 2
  pins.setCoilPin(2, 2);//Pin for coil 3 - ONLY WITH DB2
  pins.setCoilPin(3, 3);//Pin for coil 4 - ONLY WITH DB2
  pins.setCoilPin(4, 46);//Placeholder only - NOT USED
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTrigger3 = 21; //The Cam sensor 2 pin
  pins.pinTPS = A3; //TPS input pin
  pins.pinMAP = A0; //MAP sensor pin
  pins.pinIAT = A5; //IAT sensor pin
  pins.pinCLT = A4; //CLT sensor pin
  pins.pinO2 = A2; //O2 sensor pin
  pins.pinBat = A1; //Battery reference voltage pin
  pins.pinBaro = A6; //Baro sensor pin - ONLY WITH DB
  pins.pinTachOut = 38; //Tacho output pin
  pins.pinIdle1 = 5; //Single wire idle control
  pins.pinIdle2 = 47; //2 wire idle control - NOT USED
  pins.pinBoost = 7; //Boost control
  pins.pinVVT_1 = 6; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.pinFuelPump = 4; //Fuel pump output
  pins.pinStepperDir = 25; //Direction pin for DRV8825 driver
  pins.pinStepperStep = 24; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 27; //Enable pin for DRV8825 driver
  pins.pinLaunch = 10; //Can be overwritten below
  pins.pinFlex = 20; // Flex sensor (Must be external interrupt enabled) - ONLY WITH DB
  pins.pinFan = 30; //Pin for the fan output - ONLY WITH DB
  pins.pinResetControl = 26; //Reset control output
  return pins;
}

// Pin mappings as per the UA4C shield
static pinNumbers_t getUA4CMapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, 8);//Output pin injector 1 is on
  pins.setInjectorPin(1, 7);//Output pin injector 2 is on
  pins.setInjectorPin(2, 6);//Output pin injector 3 is on
  pins.setInjectorPin(3, 5);//Output pin injector 4 is on
  pins.setInjectorPin(4, 45);//Output pin injector 5 is on PLACEHOLDER value for now
  pins.setCoilPin(0, 35);//Pin for coil 1
  pins.setCoilPin(1, 36);//Pin for coil 2
  pins.setCoilPin(2, 33);//Pin for coil 3
  pins.setCoilPin(3, 34);//Pin for coil 4
  pins.setCoilPin(4, 44);//Pin for coil 5 PLACEHOLDER value for now
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTrigger3 = 3; //The Cam sensor 2 pin
  pins.pinFlex = 20; // Flex sensor
  pins.pinTPS = A3; //TPS input pin
  pins.pinMAP = A0; //MAP sensor pin
  pins.pinBaro = A7; //Baro sensor pin
  pins.pinIAT = A5; //IAT sensor pin
  pins.pinCLT = A4; //CLS sensor pin
  pins.pinO2 = A1; //O2 Sensor pin
  pins.pinO2_2 = A9; //O2 sensor pin (second sensor)
  pins.pinBat = A2; //Battery reference voltage pin
  pins.pinLaunch = 37; //Can be overwritten below
  pins.pinTachOut = 22; //Tacho output pin
  pins.pinIdle1 = 9; //Single wire idle control
  pins.pinIdle2 = 10; //2 wire idle control
  pins.pinFuelPump = 23; //Fuel pump output
  pins.pinVVT_1 = 11; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.pinStepperDir = 32; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 31; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 30; //Enable pin for DRV8825 driver
  pins.pinBoost = 12; //Boost control
  pins.pinFan = 24; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
  pins.pinVSS = 2;
  return pins;
}

// Pin mappings for all BlitzboxBL49sp variants
static pinNumbers_t getBlitzboxBL49spMapping(void)
{
  pinNumbers_t pins;

  pins.setInjectorPin(0, 6);//Output pin injector 1
  pins.setInjectorPin(1, 7);//Output pin injector 2
  pins.setInjectorPin(2, 8);//Output pin injector 3
  pins.setInjectorPin(3, 9);//Output pin injector 4
  pins.setCoilPin(0, 24);//Pin for coil 1
  pins.setCoilPin(1, 25);//Pin for coil 2
  pins.setCoilPin(2, 23);//Pin for coil 3
  pins.setCoilPin(3, 22);//Pin for coil 4
  pins.pinTrigger = 19; //The CRANK Sensor pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinFlex = 20; // Flex sensor PLACEHOLDER value for now
  pins.pinTPS = A0; //TPS input pin
  pins.pinO2 = A2; //O2 Sensor pin
  pins.pinIAT = A3; //IAT sensor pin
  pins.pinCLT = A4; //CLT sensor pin
  pins.pinMAP = A7; //internal MAP sensor
  pins.pinBat = A6; //Battery reference voltage pin
  pins.pinBaro = A5; //external MAP/Baro sensor pin
  pins.pinO2_2 = A9; //O2 sensor pin (second sensor) PLACEHOLDER value for now
  pins.pinLaunch = 2; //Can be overwritten below
  pins.pinTachOut = 10; //Tacho output pin
  pins.pinIdle1 = 11; //Single wire idle control
  pins.pinIdle2 = 14; //2 wire idle control PLACEHOLDER value for now
  pins.pinFuelPump = 3; //Fuel pump output
  pins.pinVVT_1 = 15; //Default VVT output PLACEHOLDER value for now
  pins.pinBoost = 5; //Boost control
  pins.pinFan = 12; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
  
  return pins;
}

#if defined(CORE_AVR) || defined(UNIT_TEST)
#define DIY_EFI_CORE4_MAPPING
#endif

#if defined(DIY_EFI_CORE4_MAPPING)
// Pin mappings for the DIY-EFI CORE4 Module. This is an AVR only module
static pinNumbers_t getDIYEFICore4Mapping(void)
{
  pinNumbers_t pins;
  pins.setInjectorPin(0, 10);//Output pin injector 1 is on
  pins.setInjectorPin(1, 11);//Output pin injector 2 is on
  pins.setInjectorPin(2, 12);//Output pin injector 3 is on
  pins.setInjectorPin(3, 9);//Output pin injector 4 is on
  pins.setCoilPin(0, 39);//Pin for coil 1
  pins.setCoilPin(1, 29);//Pin for coil 2
  pins.setCoilPin(2, 28);//Pin for coil 3
  pins.setCoilPin(3, 27);//Pin for coil 4
  pins.setCoilPin(4, 26);//Placeholder  for coil 5
  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTrigger3 = 21;// The Cam sensor 2 pin
  pins.pinFlex = 20; // Flex sensor
  pins.pinTPS = A3; //TPS input pin
  pins.pinMAP = A2; //MAP sensor pin
  pins.pinBaro = A15; //Baro sensor pin
  pins.pinIAT = A11; //IAT sensor pin
  pins.pinCLT = A4; //CLS sensor pin
  pins.pinO2 = A12; //O2 Sensor pin
  pins.pinO2_2 = A5; //O2 sensor pin (second sensor)
  pins.pinBat = A1; //Battery reference voltage pin
  pins.pinLaunch = 24; //Can be overwritten below
  pins.pinTachOut = 38; //Tacho output pin
  pins.pinIdle1 = 42; //Single wire idle control
  pins.pinIdle2 = 43; //2 wire idle control
  pins.pinFuelPump = 41; //Fuel pump output
  pins.pinVVT_1 = 44; //Default VVT output
  pins.pinVVT_2 = 48; //Default VVT2 output
  pins.pinStepperDir = 32; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 31; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 30; //Enable pin for DRV8825 driver
  pins.pinBoost = 45; //Boost control
  pins.setInjectorPin(4, 33);//Output pin injector 5 is on
  pins.setInjectorPin(5, 34);//Output pin injector 6 is on
  pins.pinFan = 40; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

  return pins;
}
#endif

#if defined(CORE_TEENSY35)
#define DVJ_CODEC_MAPPING
#endif

#if defined(DVJ_CODEC_MAPPING)
// Pin mappings as per the teensy rev A & B shield
static pinNumbers_t getDvjCodecMapping(void)
{
  pinNumbers_t pins;
  pins.setInjectorPin(0, 2);//Output pin injector 1 is on
  pins.setInjectorPin(1, 10);//Output pin injector 2 is on
  pins.setInjectorPin(2, 6);//Output pin injector 3 is on
  pins.setInjectorPin(3, 9);//Output pin injector 4 is on
  pins.setCoilPin(0, 29);//Pin for coil 1
  pins.setCoilPin(1, 30);//Pin for coil 2
  pins.setCoilPin(2, 31);//Pin for coil 3 - ONLY WITH DB2
  pins.setCoilPin(3, 32);//Pin for coil 4 - ONLY WITH DB2
  pins.pinTrigger = 23; //The CAS pin
  pins.pinTrigger2 = 36; //The Cam Sensor pin
  pins.pinTPS = 16; //TPS input pin
  pins.pinMAP = 17; //MAP sensor pin
  pins.pinIAT = 14; //IAT sensor pin
  pins.pinCLT = 15; //CLT sensor pin
  pins.pinO2 = A22; //O2 sensor pin
  pins.pinO2_2 = A21; //O2 sensor pin (second sensor)
  pins.pinBat = 18; //Battery reference voltage pin
  pins.pinTachOut = 20; //Tacho output pin
  pins.pinIdle1 = 5; //Single wire idle control
  pins.pinBoost = 11; //Boost control
  pins.pinFuelPump = 38; //Fuel pump output
  pins.pinStepperDir = 34; //Direction pin for DRV8825 driver
  pins.pinStepperStep = 35; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 33; //Enable pin for DRV8825 driver
  pins.pinLaunch = 26; //Can be overwritten below
  pins.pinFan = 37; //Pin for the fan output - ONLY WITH DB

  return pins;
}
#endif

#if defined(CORE_TEENSY35)
#define JUICE_BOX_MAPPING
#endif

#if defined(JUICE_BOX_MAPPING)
// Pin mappings for the Juice Box (ignition only board)
static pinNumbers_t getJuiceBoxMapping(void)
{
  pinNumbers_t pins;
  pins.setInjectorPin(0, 2);//Output pin injector 1 is on - NOT USED
  pins.setInjectorPin(1, 56);//Output pin injector 2 is on - NOT USED
  pins.setInjectorPin(2, 6);//Output pin injector 3 is on - NOT USED
  pins.setInjectorPin(3, 50);//Output pin injector 4 is on - NOT USED
  pins.setCoilPin(0, 29);//Pin for coil 1
  pins.setCoilPin(1, 30);//Pin for coil 2
  pins.setCoilPin(2, 31);//Pin for coil 3
  pins.setCoilPin(3, 32);//Pin for coil 4
  pins.pinTrigger = 37; //The CAS pin
  pins.pinTrigger2 = 38; //The Cam Sensor pin - NOT USED
  pins.pinTPS = A2; //TPS input pin
  pins.pinMAP = A7; //MAP sensor pin
  pins.pinIAT = A1; //IAT sensor pin
  pins.pinCLT = A5; //CLT sensor pin
  pins.pinO2 = A0; //O2 sensor pin
  pins.pinO2_2 = A21; //O2 sensor pin (second sensor) - NOT USED
  pins.pinBat = A6; //Battery reference voltage pin
  pins.pinTachOut = 28; //Tacho output pin
  pins.pinIdle1 = 5; //Single wire idle control - NOT USED
  pins.pinBoost = 11; //Boost control - NOT USED
  pins.pinFuelPump = 24; //Fuel pump output
  pins.pinStepperDir = 3; //Direction pin for DRV8825 driver - NOT USED
  pins.pinStepperStep = 4; //Step pin for DRV8825 driver - NOT USED
  pins.pinStepperEnable = 6; //Enable pin for DRV8825 driver - NOT USED
  pins.pinLaunch = 26; //Can be overwritten below
  pins.pinFan = 25; //Pin for the fan output

  return pins;
}
#endif

#if defined(CORE_TEENSY)
#define DROPBEAR_MAPPING
#endif

#if defined(DROPBEAR_MAPPING)
// Pin mappings for the DropBear
static pinNumbers_t getDropBearMapping(void)
{
  pinNumbers_t pins;

  //The injector pins below are not used directly as the control is via SPI through the MC33810s, however the pin numbers are set to be the SPI pins (SCLK, MOSI, MISO and CS) so that nothing else will set them as inputs
  pins.setInjectorPin(0, 13);//SCLK
  pins.setInjectorPin(1, 11);//MOSI
  pins.setInjectorPin(2, 12);//MISO
  pins.setInjectorPin(3, 10);//CS for MC33810 1
  pins.setInjectorPin(4, 9);//CS for MC33810 2
  pins.setInjectorPin(5, 9);//CS for MC33810 3

  //Dummy pins, without these pin 0 (Serial1 RX) gets overwritten
  pins.setCoilPin(0, 40);
  pins.setCoilPin(1, 41);

  pins.pinTrigger = 19; //The CAS pin
  pins.pinTrigger2 = 18; //The Cam Sensor pin
  pins.pinTrigger3 = 22; //Uses one of the protected spare digital inputs. This must be set or Serial1 (Pin 0) gets broken
  pins.pinFlex = A16; // Flex sensor
  pins.pinMAP = A1; //MAP sensor pin
  pins.pinBaro = A0; //Baro sensor pin
  pins.pinBat = A14; //Battery reference voltage pin
  pins.pinLaunch = A15; //Can be overwritten below
  pins.pinTachOut = 5; //Tacho output pin
  pins.pinIdle1 = 27; //Single wire idle control
  pins.pinIdle2 = 29; //2 wire idle control. Shared with Spare 1 output
  pins.pinFuelPump = 8; //Fuel pump output
  pins.pinVVT_1 = 28; //Default VVT output
  pins.pinStepperDir = 32; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 31; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 30; //Enable pin for DRV8825 driver
  pins.pinBoost = 24; //Boost control
  pins.pinFan = 25; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
  pins.pinVSS = 22;

  pins.pinWMIEmpty = 23; //Spare digital input
  pins.pinWMIIndicator = 26; //Spare output
  pins.pinWMIEnabled = 29; //Spare output

#if defined(CORE_TEENSY35)
  pins.pinTPS = A22; //TPS input pin
  pins.pinIAT = A19; //IAT sensor pin
  pins.pinCLT = A20; //CLS sensor pin
  pins.pinO2 = A21; //O2 Sensor pin
  pins.pinO2_2 = A18; //Spare 2
#endif

#if defined(CORE_TEENSY41)
  //New pins for the actual T4.1 version of the Dropbear
  pins.pinBaro = A4; 
  pins.pinMAP = A5;
  pins.pinTPS = A3; //TPS input pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A2; //O2 Sensor pin
  pins.pinBat = A15; //Battery reference voltage pin. Needs Alpha4+
  pins.pinLaunch = 36;
  pins.pinFlex = 37; // Flex sensor

  pins.pinTrigger = 20; //The CAS pin
  pins.pinTrigger2 = 21; //The Cam Sensor pin
  pins.pinTrigger3 = 34; //Uses one of the protected spare digital inputs.

  pins.pinFuelPump = 5; //Fuel pump output
  pins.pinTachOut = 0; //Tacho output pin

  pins.pinResetControl = 49; //PLaceholder only. Cannot use 42-47 as these are the SD card
  pins.pinWMIEmpty = 35; //Spare digital input
  pins.pinVSS = 34;
#endif

  pins.pinMC33810_1_CS = 10;
  pins.pinMC33810_2_CS = 9;

  //Pin alignment to the MC33810 outputs
  pins.mc33810InjBits[0] = 3;
  pins.mc33810InjBits[1] = 1;
  pins.mc33810InjBits[2] = 0;
  pins.mc33810InjBits[3] = 2;
  pins.mc33810IgnBits[0] = 4;
  pins.mc33810IgnBits[1] = 5;
  pins.mc33810IgnBits[2] = 6;
  pins.mc33810IgnBits[3] = 7;

  pins.mc33810InjBits[4] = 3;
  pins.mc33810InjBits[5] = 1;
  pins.mc33810InjBits[6] = 0;
  pins.mc33810InjBits[7] = 2;
  pins.mc33810IgnBits[4] = 4;
  pins.mc33810IgnBits[5] = 5;
  pins.mc33810IgnBits[6] = 6;
  pins.mc33810IgnBits[7] = 7;

  return pins;
}
#endif

#if defined(CORE_TEENSY) || defined(UNIT_TEST)
#define BEAR_CUB_MAPPING
#endif

#if defined(BEAR_CUB_MAPPING)
static pinNumbers_t getBearCubMapping(void)
{
  pinNumbers_t pins;

  //Pin mappings for the Bear Cub (Teensy 4.1)
  pins.setInjectorPin(0, 6);
  pins.setInjectorPin(1, 7);
  pins.setInjectorPin(2, 9);
  pins.setInjectorPin(3, 8);
  pins.setInjectorPin(4, 0); //Not used
  pins.setCoilPin(0, 2);
  pins.setCoilPin(1, 3);
  pins.setCoilPin(2, 4);
  pins.setCoilPin(3, 5);

  pins.pinTrigger = 20; //The CAS pin
  pins.pinTrigger2 = 21; //The Cam Sensor pin
  pins.pinFlex = 37; // Flex sensor
  pins.pinMAP = A5; //MAP sensor pin
  pins.pinBaro = A4; //Baro sensor pin
  pins.pinBat = A15; //Battery reference voltage pin
  pins.pinTPS = A3; //TPS input pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A2; //O2 Sensor pin
  pins.pinLaunch = 36;

  pins.pinTachOut = 38; //Tacho output pin
  pins.pinIdle1 = 27; //Single wire idle control
  pins.pinIdle2 = 26; //2 wire idle control. Shared with Spare 1 output
  pins.pinFuelPump = 10; //Fuel pump output
  pins.pinVVT_1 = 28; //Default VVT output
  pins.pinStepperDir = 32; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = 31; //Step pin for DRV8825 driver
  pins.pinStepperEnable = 30; //Enable pin for DRV8825 driver
  pins.pinBoost = 24; //Boost control
  pins.pinFan = 25; //Pin for the fan output
  pins.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

  return pins;
}
#endif

#if defined(STM32F407xx) || defined(STM32F411xE) || defined(STM32F401xC) || defined(CORE_STM32)
#define SPECTRE_V05_MAPPING
#endif

#if defined(SPECTRE_V05_MAPPING)
static pinNumbers_t getSpectreV05Mapping(void)
{
  pinNumbers_t pins;
#if defined(STM32F407xx)
  //Pin definitions for experimental board Tjeerd 
  //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407
  //https://github.com/Tjeerdie/SPECTRE/tree/master/SPECTRE_V0.5
  
  //******************************************
  //******** PORTA CONNECTIONS *************** 
  //******************************************
  // = PA0; //Wakeup ADC123
  // = PA1; //ADC123
  // = PA2; //ADC123
  // = PA3; //ADC123
  // = PA4; //ADC12
  // = PA5; //ADC12
  // = PA6; //ADC12 LED_BUILTIN_1
  // = PA7; //ADC12 LED_BUILTIN_2
  pins.setCoilPin(2, PA8);
  // = PA9;  //TXD1=Bluetooth module
  // = PA10; //RXD1=Bluetooth module
  // = PA11; //(DO NOT USE FOR SPEEDUINO) USB
  // = PA12; //(DO NOT USE FOR SPEEDUINO) USB 
  // = PA13;  //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  // = PA14;  //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  // = PA15;  //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK

  //******************************************
  //******** PORTB CONNECTIONS *************** 
  //******************************************
  // = PB0;  //(DO NOT USE FOR SPEEDUINO) ADC123 - SPI FLASH CHIP CS pin
  pins.pinBaro = PB1; //ADC12
  // = PB2;  //(DO NOT USE FOR SPEEDUINO) BOOT1 
  // = PB3;  //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
  // = PB4;  //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
  // = PB5;  //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
  // = PB6;  //NRF_CE
  pins.setCoilPin(5, PB7); //NRF_CS
  // = PB8;  //NRF_IRQ
  pins.setCoilPin(1, PB9);//
  // = PB9;  //
  // = PB10; //TXD3
  // = PB11; //RXD3
  // = PB12; //
  // = PB13;  //SPI2_SCK
  // = PB14;  //SPI2_MISO
  // = PB15;  //SPI2_MOSI

  //******************************************
  //******** PORTC CONNECTIONS *************** 
  //******************************************
  pins.pinIAT = PC0; //ADC123 
  pins.pinTPS = PC1; //ADC123
  pins.pinMAP = PC2; //ADC123 
  pins.pinCLT = PC3; //ADC123
  pins.pinO2 = PC4; //ADC12
  pins.pinBat = PC5;  //ADC12
  pins.pinBoost = PC6; //
  pins.pinIdle1 = PC7; //
  // = PC8;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
  // = PC9;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
  // = PC10;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
  // = PC11;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
  // = PC12;  //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
  pins.pinTachOut = PC13; //
  // = PC14;  //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
  // = PC15;  //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

  //******************************************
  //******** PORTD CONNECTIONS *************** 
  //******************************************
  // = PD0;  //CANRX
  // = PD1;  //CANTX
  // = PD2;  //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
  pins.pinIdle2 = PD3; //
  // = PD4;  //
  pins.pinFlex = PD4;
  // = PD5; //TXD2
  // = PD6;  //RXD2
  pins.setCoilPin(0, PD7);//
  // = PD7;  //
  // = PD8;  //
  pins.setCoilPin(4, PD9);//
  pins.setCoilPin(3, PD10);//
  // = PD11;  //
  pins.setInjectorPin(0, PD12);//
  pins.setInjectorPin(1, PD13);//
  pins.setInjectorPin(2, PD14);//
  pins.setInjectorPin(3, PD15);//

  //******************************************
  //******** PORTE CONNECTIONS *************** 
  //******************************************
  pins.pinTrigger = PE0; //
  pins.pinTrigger2 = PE1; //
  pins.pinStepperEnable = PE2; //
  pins.pinFuelPump = PE3; //ONBOARD KEY1
  // = PE4;  //ONBOARD KEY2
  pins.pinStepperStep = PE5; //
  pins.pinFan = PE6; //
  pins.pinStepperDir = PE7; //
  // = PE8;  //
  pins.setInjectorPin(4, PE9);//
  // = PE10;  //
  pins.setInjectorPin(5, PE11);//
  // = PE12; //
  pins.setInjectorPin(7, PE13);//
  pins.setInjectorPin(6, PE14);//
  // = PE15;  //
#elif defined(STM32F411xE) || defined(STM32F401xC)
  //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
  //PB2 can't be used as input because is BOOT pin
  pins.setInjectorPin(0, PB7);//Output pin injector 1 is on
  pins.setInjectorPin(1, PB6);//Output pin injector 2 is on
  pins.setInjectorPin(2, PB5);//Output pin injector 3 is on
  pins.setInjectorPin(3, PB4);//Output pin injector 4 is on
  pins.setCoilPin(0, PB9);//Pin for coil 1
  pins.setCoilPin(1, PB8);//Pin for coil 2
  pins.setCoilPin(2, PB3);//Pin for coil 3
  pins.setCoilPin(3, PA15);//Pin for coil 4
  pins.pinTPS = A2;//TPS input pin
  pins.pinMAP = A3; //MAP sensor pin
  pins.pinIAT = A0; //IAT sensor pin
  pins.pinCLT = A1; //CLS sensor pin
  pins.pinO2 = A8; //O2 Sensor pin
  pins.pinBat = A4; //Battery reference voltage pin
  pins.pinBaro = pins.pinMAP;
  pins.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
  pins.pinIdle1 = PB2; //Single wire idle control
  pins.pinIdle2 = PB10; //2 wire idle control
  pins.pinBoost = PA6; //Boost control
  pins.pinStepperDir = PB10; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = PB2; //Step pin for DRV8825 driver
  pins.pinFuelPump = PA8; //Fuel pump output
  pins.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)

  //external interrupt enabled pins
  pins.pinFlex = PC14; // Flex sensor (Must be external interrupt enabled)
  pins.pinTrigger = PC13; //The CAS pin also led pin so bad idea
  pins.pinTrigger2 = PC15; //The Cam Sensor pin
#elif defined(CORE_STM32)
  //blue pill wiki.stm32duino.com/index.php?title=Blue_Pill
  //Maple mini wiki.stm32duino.com/index.php?title=Maple_Mini
  //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
  //PB2 can't be used as input because is BOOT pin
  pins.setInjectorPin(0, PB7);//Output pin injector 1 is on
  pins.setInjectorPin(1, PB6);//Output pin injector 2 is on
  pins.setInjectorPin(2, PB5);//Output pin injector 3 is on
  pins.setInjectorPin(3, PB4);//Output pin injector 4 is on
  pins.setCoilPin(0, PB3);//Pin for coil 1
  pins.setCoilPin(1, PA15);//Pin for coil 2
  pins.setCoilPin(2, PA14);//Pin for coil 3
  pins.setCoilPin(3, PA9);//Pin for coil 4
  pins.setCoilPin(4, PA8);//Pin for coil 5
  pins.pinTPS = A0; //TPS input pin
  pins.pinMAP = A1; //MAP sensor pin
  pins.pinIAT = A2; //IAT sensor pin
  pins.pinCLT = A3; //CLS sensor pin
  pins.pinO2 = A4; //O2 Sensor pin
  pins.pinBat = A5; //Battery reference voltage pin
  pins.pinBaro = pins.pinMAP;
  pins.pinIdle1 = PB2; //Single wire idle control
  pins.pinIdle2 = PA2; //2 wire idle control
  pins.pinBoost = PA1; //Boost control
  pins.pinVVT_1 = PA0; //Default VVT output
  pins.pinVVT_2 = PA2; //Default VVT2 output
  pins.pinStepperDir = PC15; //Direction pin  for DRV8825 driver
  pins.pinStepperStep = PC14; //Step pin for DRV8825 driver
  pins.pinStepperEnable = PC13; //Enable pin for DRV8825
  pins.pinFan = PB1; //Pin for the fan output
  pins.pinFuelPump = PB11; //Fuel pump output
  pins.pinTachOut = PB10; //Tacho output pin
  //external interrupt enabled pins
  pins.pinFlex = PB8; // Flex sensor (Must be external interrupt enabled)
  pins.pinTrigger = PA10; //The CAS pin
  pins.pinTrigger2 = PA13; //The Cam Sensor pin

#endif

  return pins;
}
#endif


#if defined(BOARD_FCR_MICRO_F4)
static pinNumbers_t getFCRMicroF45Mapping(void)
{
  pinNumbers_t pins;
  pins.pinIAT       = PA0;
  pins.pinCLT       = PA1;
  pins.pinTPS       = PA2;
  pins.pinBat       = PA4;
  pins.pinMAP       = PA5;
  pins.pinBaro      = PA6;
  pins.pinO2        = PB0;
  pins.pinTrigger   = PB1;
  pins.pinTrigger2  = PA7;
  pins.setCoilPin(0, PE10);
  pins.setCoilPin(1, PE11);
  pins.setInjectorPin(0, PC6);
  pins.setInjectorPin(1, PC7);
  pins.pinIdle1     = PD2;
  pins.pinFan       = PD3;
  pins.pinFuelPump  = PD4;
  pins.pinTachOut   = PC9;
  return pins;
}
#endif

pinNumbers_t getPinMapping(uint8_t boardID)
{
#ifndef SMALL_FLASH_MODE
  switch (boardID)
  {
    //Note: Case 0 (Speeduino v0.1) was removed in Nov 2020 to handle default case for blank FRAM modules
    case 1: return getV02ShieldMapping(); break;
    case 2: return getV03ShieldMapping(); break;
    case 3: return getV04ShieldMapping(); break;
    case 6: return getMiataNB2Mapping(); break;
    case 8: return getMiataNA18Mapping(); break;
    case 9: return getMiataNA16Mapping(); break;
    case 10: return getTurtanasPcbPinMapping(); break;
#if defined(LEVIN_BOARD_MAPPING)
    case 14: return getLevinMapping(); break;
#endif
#if defined(PLAZOMAT_V01_MAPPING)
    case 20: return getPlazomatV01Mapping(); break;
#endif
    case 30: return getDazV6V01Mapping(); break;
#if defined(PAZI_BMW_PNP_MAPPING)
    case 31: return getBmwPnPMapping(); break;
#endif
    case 40: return getNO2CMapping(); break;
    case 41: return getUA4CMapping(); break;
    case 42: return getBlitzboxBL49spMapping(); break;
#if defined(DIY_EFI_CORE4_MAPPING)
    case 45: return getDIYEFICore4Mapping(); break;
#endif
#if defined(DVJ_CODEC_MAPPING)
    case 50: return getDvjCodecMapping(); break;
    case 51: return getDvjCodecMapping(); break;
#endif
#if defined(JUICE_BOX_MAPPING)
    case 53: return getJuiceBoxMapping(); break;
#endif
#if defined(DROPBEAR_MAPPING)
    case 55: return getDropBearMapping(); break;
#endif
#if defined(BEAR_CUB_MAPPING)
    case 56: return getBearCubMapping(); break;
#endif
#if defined(SPECTRE_V05_MAPPING)
    case 60: return getSpectreV05Mapping(); break;
#endif
#if defined(BOARD_FCR_MICRO_F4)
    case 61: return getFCRMicroF45Mapping(); break;
#endif
    default: break;
  }
#endif

  return getDefaultPinMapping();
}