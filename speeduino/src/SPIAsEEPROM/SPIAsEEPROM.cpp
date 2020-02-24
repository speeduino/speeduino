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
 * 1245184 bytes used of the SPI flash resulting in 4228 bytes of usable EEPROM
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

//#if defined(CORE_STM32_OFFICIAL) && defined(SPIFLASH_AS_EEPROM)
#if defined(USE_SPI_EEPROM)

#include "SPIAsEEPROM.h"
#include "SPI.h"        
//#include "globals.h"     

SPIAsEEPROM::SPIAsEEPROM()
{
    //pinMode(PB0, OUTPUT);
    magicbuf[0] = MAGICNUMBER1;
    magicbuf[1] = MAGICNUMBER2;
    magicbuf[2] = MAGICNUMBER3;
    magicbuf[3] = 0x00;

  }
  uint8_t SPIAsEEPROM::begin(uint8_t pinSPIFlash_CS=6)
  {
      pinMode(pinSPIFlash_CS, OUTPUT);
      SpiFlashAvialable = winbondSPIFlash.begin(_W25Q16,SPI, pinSPIFlash_CS);
      uint8_t formatted = 0;
      if(SpiFlashAvialable){
        //check for magic numbers
        formatted = 1;
        uint8_t buf[MAGICNUMBER_OFFSET];
        for(uint16_t i=0; i< FLASHSIZEUSED/SECTORSIZE; i++ ){
            winbondSPIFlash.read(sectorNumber*SECTORSIZE, buf, sizeof(buf) - 1);
            if((buf[0] != MAGICNUMBER1) | (buf[1] != MAGICNUMBER2) | (buf[2] != MAGICNUMBER3)){
              //if one of the SECTORS has no magic numbers it is not formatted
              formatted = 0;
            }
        }
        //If not formatted format flash. This takes 10 seconds or more!
        if(!formatted){
          formatFlashForUse();
        }

        //check if format succeeded
        formatted = 1;
        for(uint16_t i=0; i< FLASHSIZEUSED/SECTORSIZE; i++ ){
            winbondSPIFlash.read(sectorNumber*SECTORSIZE, buf, sizeof(buf) - 1);
            if((buf[0] != MAGICNUMBER1) | (buf[1] != MAGICNUMBER2) | (buf[2] != MAGICNUMBER3)){
              formatted = 0;
            }
        }
      }
      if(formatted & SpiFlashAvialable){return true;}else{return false;}
  }

int8_t SPIAsEEPROM::write(uint16_t addressEEPROM, uint8_t writeValue){ 
    uint8_t ByteBuf[1];

    //Check if adress is in the EEPROM space  
    if (addressEEPROM >= FLASHSIZEUSED/SECTORSIZE * (SECTORSIZE/FLASH_PAGESIZE - 2)){addressEEPROM = FLASHSIZEUSED/SECTORSIZE * (SECTORSIZE/FLASH_PAGESIZE - 2);}
    
    //read the current value
    uint8_t readValue = read(addressEEPROM);
    //After reading the current byte all global variables containing inforamtion about the address are set correctly. 

    //only write if value is changed. 
    if (readValue != writeValue){

      //Check if buffer is full and an erase must be performed.
      if (nrOfOnes < 1){

        //First read all the values in this sector that will get distroyed when erasing
        uint8_t tempBuf[14];
        for(uint8_t i = 0; i<14; i++){
            tempBuf[i] = read((sectorNumber*14) + i);
        }

        //now erase the sector
        writeMagicNumber(sectorNumber);

        //write all the values back
        for(uint8_t i=0; i<14; i++){
            write((sectorNumber*14) + i, tempBuf[i]);
        }

        //also do not forget to write the new value!
        write(addressEEPROM, writeValue);
        return 0;
        
      }

      //determine the adress of the byte in the infoblock where one bit must be reset when writing new values 
      uint8_t RelativeAdressInInfoBlock = (nrOfOnes - 1)/8;

      //determine value of the infoblock byte after writing one more time.
      uint8_t ValueInInfoBlock = 0xFF << (8 - (nrOfOnes - 1 - ((RelativeAdressInInfoBlock) * 8)));  
      
      //write the new value at the new location 
      ByteBuf[0] = writeValue;
      winbondSPIFlash.WE();
      winbondSPIFlash.writePage(dataFlashAddress - 1, ByteBuf, sizeof(ByteBuf));
      while(winbondSPIFlash.busy());

      //write where read can find the new value
      ByteBuf[0] = ValueInInfoBlock;
      winbondSPIFlash.WE();
      winbondSPIFlash.writePage(infoFlashAddress + RelativeAdressInInfoBlock, ByteBuf, sizeof(ByteBuf));
      while(winbondSPIFlash.busy());


      return 0;
    }else{
      return 0;
    }
}

int8_t SPIAsEEPROM::update(uint16_t address, uint8_t val){
    //a write function call is already an update. 
    write(address, val);
    return 0;
}

uint8_t SPIAsEEPROM::writeMagicNumber(uint16_t sectorNumber){
    //sector adress is for 0 to (FLASHSIZEUSED/SECTORSIZE -1)
    //Function is used to write a magic number at the start of each Sector.
    //This number can be used to identify if the flash is formated for use as EEPROM
    if (sectorNumber>=FLASHSIZEUSED/SECTORSIZE){
      sectorNumber = (FLASHSIZEUSED/SECTORSIZE - 1);
    }

    //First erase the sector (4KiB at the time)
    winbondSPIFlash.WE();
    winbondSPIFlash.eraseSector(sectorNumber*SECTORSIZE);
    while(winbondSPIFlash.busy()); //if no spi flash present or accessible this hangs forever!  

    //Write the magic numbers at the start of the sector for identification.
    winbondSPIFlash.WE();
    winbondSPIFlash.writePage(sectorNumber*SECTORSIZE, magicbuf, MAGICNUMBER_OFFSET);
    while(winbondSPIFlash.busy()); //if no spi flash present or accessible this hangs forever!  

    return 0;
}

uint8_t SPIAsEEPROM::formatFlashForUse(){
  //Format the flash for use by erasing all sectors used and 
  //write the magic number at the start of each erased sector
  for(uint16_t i = 0; i<FLASHSIZEUSED/SECTORSIZE; i++){
    writeMagicNumber(i);
  } 
  return 0;
}

uint8_t SPIAsEEPROM::read(uint16_t addressEEPROM){
    //The infoblock is at the start of each sector 
    //The first two pages will be used for the infoblock
    //The first 4 bytes of each page must have the magic number 
    //version 0.1 does not check magic number
    if(!SpiFlashAvialable){
      begin(USE_SPI_EEPROM);
    }


    uint8_t buf[INFOBYTES_PER_BYTE];

    //Check if address is outside of the maximum. limit to get inside maximum and continue as normal.
    //Should be changed so that it returns -1 (error)
    if (addressEEPROM >= FLASHSIZEUSED/SECTORSIZE * (SECTORSIZE/FLASH_PAGESIZE - 2)){addressEEPROM = FLASHSIZEUSED/SECTORSIZE * (SECTORSIZE/FLASH_PAGESIZE - 2);}  
    
    //Check at what sector number the adress resides. 14 bytes per sector
    sectorNumber = addressEEPROM/(SECTORSIZE/FLASH_PAGESIZE - 2);

    //Check at what page number in the sector the adress can be found (16 pages per sector, 14 used)
    pageNumber = addressEEPROM - (sectorNumber * ((SECTORSIZE/FLASH_PAGESIZE) - 2));

    //The absulute adress of the infoblock of the byte in flash adress
    infoFlashAddress = sectorNumber*SECTORSIZE + pageNumber * INFOBYTES_PER_BYTE + MAGICNUMBER_OFFSET;

    //read the infoblock and put into the buffer
    winbondSPIFlash.read(infoFlashAddress, buf, sizeof(buf));
    while(winbondSPIFlash.busy()); //if no spi flash present or accessible this hangs forever!  

    //calculate actual flash address of the data
    //Count de number of set bits in the infoblock
    nrOfOnes = count(buf);

    //Calulate the adress from all previous information.
    dataFlashAddress = sectorNumber*SECTORSIZE  + (pageNumber * FLASH_PAGESIZE) + INFOBYTESSECTOROFFSET + nrOfOnes - 1;

    uint8_t ByteBuf[1];
    //read the actual byte with information

    winbondSPIFlash.read(dataFlashAddress, ByteBuf, sizeof(ByteBuf));
    while(winbondSPIFlash.busy()); //if no spi flash present or accessible this hangs forever!  

    return ByteBuf[0];
}

uint16_t SPIAsEEPROM::count(uint8_t buf[FLASH_PAGESIZE/BITS_PER_BYTE]){
    uint16_t count=0;
    for(uint8_t j=0; j < 32; j++)
      for(uint8_t i=0; i<8; i++){
        if((buf[j] & 1) == 1){ //if current bit 1
          count++;//increase count
        }
        buf[j]=buf[j]>>1;//right shift
      }
    return count;
}

//SPIAsEEPROM EEPROM;

#endif