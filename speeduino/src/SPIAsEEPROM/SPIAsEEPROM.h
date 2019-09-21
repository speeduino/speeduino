#ifndef SPI_AS_EEPROM_H
#define SPI_AS_EEPROM_H

#if defined(CORE_STM32_OFFICIAL) && defined(SPIFLASH_AS_EEPROM)

// //UGLY HACK TO PREVENT EEPROM LIBRARY BEING IMPORTED FIRST!!
// //Use with black_F407VE board 
// #ifndef EEPROM_h
// #define EEPROM_h


#define FLASHSIZEUSED 1245184 //must be a multiple of sectorsize //1245184 = 4228 bytes of EEPROM
#define BYTESPERSECTOR 14 
#define SECTORSIZE 4096
#define INFOBYTESSECTOROFFSET 512
#define FLASH_PAGESIZE 256
#define MAGICNUMBER1 0xC0
#define MAGICNUMBER2 0xFF
#define MAGICNUMBER3 0xEE
#define MAGICNUMBER_OFFSET 4
#define BITS_PER_BYTE 8 

//Need one bit per byte to determine location actual data
//256bits/8 so 32 bytes
#define INFOBYTES_PER_BYTE FLASH_PAGESIZE/BITS_PER_BYTE

#include <stdint.h>
#include <SPI.h>
#include "winbondflash.h"

// winbondFlashSPI winbondSPIFlash; 

class SPIAsEEPROM {
  private: 
    winbondFlashSPI winbondSPIFlash;
    uint8_t ReadOutBuffer[BYTESPERSECTOR];
    uint8_t magicbuf[4];
    uint16_t sectorNumber;
    uint16_t pageNumber;
    uint16_t nrOfOnes;
    uint32_t dataFlashAddress;
    uint32_t infoFlashAddress;
    uint8_t writeMagicNumber(uint16_t sectorNumber);
    uint16_t count(uint8_t buf[FLASH_PAGESIZE/BITS_PER_BYTE]);
      
  public:
    
    SPIAsEEPROM();
    uint8_t begin();
    uint8_t formatFlashForUse();
    uint8_t read(uint16_t addressEEPROM);  
    int8_t write(uint16_t addressEEPROM, uint8_t val);
    int8_t update(uint16_t address, uint8_t val);
};

#endif
#endif
// #endif

extern SPIAsEEPROM EEPROM;
