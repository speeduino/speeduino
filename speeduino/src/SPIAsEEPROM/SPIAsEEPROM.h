/* Speeduino SPIAsEEPROM Library v.1.0.0
 * Copyright (C) 2019 by Tjeerd Hoogendijk
 * Created by Tjeerd Hoogendijk - 21/09/2019
 *
 * This file is part of the Speeduino project. This library is for
 * Winbond SPI flash memory modules. In its current form it enables reading
 * and writing individual bytes as if it where an AVR EEPROM. It uses some 
 * wear leveling (256x). When the begin() fuction is called for the first time 
 * it will "format" the flash chip. 
 * !!!!THIS DISTROYS ANY EXISTING DATA ON THE SPI FLASH!!!
 * 
 * 1757184 bytes used of the SPI flash resulting in 6006 bytes of usable EEPROM
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v3.0
 * along with the Arduino SPIMemory Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef SPI_AS_EEPROM_H
#define SPI_AS_EEPROM_H

#if defined(CORE_STM32_OFFICIAL) && defined(SPIFLASH_AS_EEPROM)

#define FLASHSIZEUSED 1757184 //must be a multiple of sectorsize //1757184 = 6006 bytes of EEPROM
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

class SPIAsEEPROM {
  private: 
    winbondFlashSPI winbondSPIFlash;
    uint8_t SpiFlashAvialable = 0;
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

extern SPIAsEEPROM EEPROM;

#endif
#endif


