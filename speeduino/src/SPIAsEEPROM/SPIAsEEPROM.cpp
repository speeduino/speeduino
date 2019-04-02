/*
* This file implements a shim layer for the SPIMemory library (https://github.com/Marzogh/SPIMemory) to mimic a minimal
* subset of the standard Arduino EEPROM library
*/
#if defined(SPI_AS_EEPROM)

#include "SPIAsEEPROM.h"
#include "SPIMemory.h"
#include <SPI.h>

//SPIFlash flash;                   
SPIFlash flash(SS1, &SPI);       //Use this constructor if using an SPI bus other than the default SPI. Only works with chips with more than one hardware SPI bus

SPIAsEEPROM::SPIAsEEPROM()
{
    //Do some init stuff here

    flash.begin();
    //To use a custom flash memory size (if using memory from manufacturers not officially supported by the library) - declare a size variable according to the list in defines.h
    //flash.begin(MB(1));

}

uint8_t SPIAsEEPROM::read(uint16_t address) {

    uint8_t val = 0;
    
    return val;
}

int8_t SPIAsEEPROM::write(uint16_t address, uint8_t val) 
{  
    return 0;
}

int8_t SPIAsEEPROM::update(uint16_t address, uint8_t val) 
{
    return 0;
}

SPIAsEEPROM EEPROM;

#endif