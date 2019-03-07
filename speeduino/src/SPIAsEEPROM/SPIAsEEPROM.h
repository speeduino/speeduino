#ifndef SPI_AS_EEPROM_H
#define SPI_AS_EEPROM_H

#if defined(SPI_AS_EEPROM)

#include <stdint.h>

class SPIAsEEPROM {
  private: 
   
  public:
    SPIAsEEPROM();
    uint8_t read(uint16_t address);  
    int8_t write(uint16_t address, uint8_t val);
    int8_t update(uint16_t address, uint8_t val);
};

extern SPIAsEEPROM EEPROM;

#endif
#endif