/*
* This file implements a shim layer for the SPIMemory library (https://github.com/Marzogh/SPIMemory) to mimic a minimal
* subset of the standard Arduino EEPROM library
*/
//Working example on BLACK_STM32F407VET6 with official stm32 core from ST

#if defined(CORE_STM32_OFFICIAL) && defined(SPIFLASH_AS_EEPROM)

#include "SPIAsEEPROM.h"
#include "SPI.h"             

//winbondSPIFlash flash(SS1, &SPI);       //Use this constructor if using an SPI bus other than the default SPI. Only works with chips with more than one hardware SPI bus


SPIAsEEPROM::SPIAsEEPROM()
{
    pinMode(PB0, OUTPUT);
    magicbuf[0] = MAGICNUMBER1;
    magicbuf[1] = MAGICNUMBER2;
    magicbuf[2] = MAGICNUMBER3;
    magicbuf[3] = 0x00;

  }
  uint8_t SPIAsEEPROM::begin() {
      uint8_t SpiFlashAvialable = 0;
      SpiFlashAvialable = winbondSPIFlash.begin(_W25Q16,SPI,PB0);

      // winbondSPIFlash.WE();
      // winbondSPIFlash.eraseAll();
      // while(winbondSPIFlash.busy());

      //Debugging
      // if(val){Serial.println("OK");} 
      uint8_t formatted = 0;
      if(SpiFlashAvialable){
        //check for magic numbers
        formatted = 1;
        uint8_t buf[MAGICNUMBER_OFFSET];
        for(uint16_t i=0; i< FLASHSIZEUSED/SECTORSIZE; i++ ){
            winbondSPIFlash.read(sectorNumber*SECTORSIZE, buf, sizeof(buf) - 1);
            if(buf[0] != MAGICNUMBER1 | buf[1] != MAGICNUMBER2 | buf[2] != MAGICNUMBER3 ){
              formatted = 0;
            }
        }

        if(!formatted){
          formatFlashForUse();
        }

        //check if format succeeded
        formatted = 1;
        for(uint16_t i=0; i< FLASHSIZEUSED/SECTORSIZE; i++ ){
            winbondSPIFlash.read(sectorNumber*SECTORSIZE, buf, sizeof(buf) - 1);
            if(buf[0] != MAGICNUMBER1 | buf[1] != MAGICNUMBER2 | buf[2] != MAGICNUMBER3 ){
              formatted = 0;
            }
        }
        
        // Serial.print("formatted");
        // Serial.println(formatted, DEC);
      }
      if(formatted & SpiFlashAvialable){return true;}else{return false;}
  }

int8_t SPIAsEEPROM::write(uint16_t addressEEPROM, uint8_t writeValue){  
    if (addressEEPROM >= FLASHSIZEUSED/SECTORSIZE * (SECTORSIZE/FLASH_PAGESIZE - 2)){addressEEPROM = FLASHSIZEUSED/SECTORSIZE * (SECTORSIZE/FLASH_PAGESIZE - 2);}
    uint8_t readValue = read(addressEEPROM);

    // Serial.print("readValue");
    // Serial.println(readValue, DEC);
    // Serial.print("writeValue");
    // Serial.println(writeValue, DEC);

    //After reading the byte all global variables containing inforamtion about the address are set correctly. 

    //only write if value is changed. 
    if (readValue != writeValue){

      //Check if buffer is full
      if (nrOfOnes < 1){
        //Serial.println("A sector gets erased and hoppfully writen back");
        //now the buffer is full and an erase must be done

        //First read all the values in this sector that will get distroyed when erasing
        uint8_t tempBuf[14];
        for(uint8_t i = 0; i<14; i++){
            tempBuf[i] = read((sectorNumber*14) + i);
        }

        //now erase the sector
        writeMagicNumber(sectorNumber);

        //write all the values back
        // Serial.println("Start re-writing values");
        for(uint8_t i=0; i<14; i++){
            write((sectorNumber*14) + i, tempBuf[i]);
        }
        // Serial.println("End re-writing values");

        write(addressEEPROM, writeValue);

        // Serial.println("New value re-writing");
        return 0;
        
      }

      //determine the adress of the byte in the infoblock where one bit must be reset when writing new values 
      uint8_t RelativeAdressInInfoBlock = (nrOfOnes - 1)/8;

      // Serial.print("RelativeAdressInInfoBlock");
      // Serial.println(RelativeAdressInInfoBlock, DEC);

      //determine value of the infoblock byte after writing one more time.
      uint8_t ValueInInfoBlock = 0xFF << (8 - (nrOfOnes - 1 - ((RelativeAdressInInfoBlock) * 8)));  
      
      // Serial.print("ValueInInfoBlock");
      // Serial.println(ValueInInfoBlock, DEC);

      uint8_t ByteBuf[1];
      
      // Serial.print("dataFlashAddress");
      // Serial.println(dataFlashAddress, DEC);

      // Serial.print("sizeof(ByteBuf)");
      // Serial.println(sizeof(ByteBuf), DEC);

      //write the new value at the new location 
      ByteBuf[0] = writeValue;
      winbondSPIFlash.WE();
      winbondSPIFlash.writePage(dataFlashAddress - 1, ByteBuf, sizeof(ByteBuf));
      while(winbondSPIFlash.busy());

      // Serial.print("infoFlashAddress");
      // Serial.println(infoFlashAddress, DEC);

      //write where read can find the new value
      ByteBuf[0] = ValueInInfoBlock;
      winbondSPIFlash.WE();
      winbondSPIFlash.writePage(infoFlashAddress + RelativeAdressInInfoBlock, ByteBuf, sizeof(ByteBuf));
      while(winbondSPIFlash.busy());

      //read the infoblock and put into the buffer
      // uint8_t buf[256];
      // winbondSPIFlash.read(infoFlashAddress, buf, sizeof(buf) - 1);
      // while(winbondSPIFlash.busy());

      // Serial.print("Buffer: ");
      // for (uint16_t i = 0; i<sizeof(buf); i++){
      //   Serial.print(buf[i], HEX);
      // }
      // Serial.println(" END");

      // winbondSPIFlash.read(0, buf, sizeof(buf) - 1);
      // while(winbondSPIFlash.busy());
      // Serial.print("sizeof(buf)");
      // Serial.println(sizeof(buf), DEC);

      
      // Serial.print("Buffer: ");
      // for (uint16_t i = 0; i<sizeof(buf); i++){
      //   Serial.print(buf[i], HEX);
      // }
      // Serial.println(" END");

      return 0;
    }else{
      return 0;
    }
}

int8_t SPIAsEEPROM::update(uint16_t address, uint8_t val){
    //a write statement is already an update statement 
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
    // Serial.print("Sector number to erase");
    // Serial.println(sectorNumber);
    winbondSPIFlash.WE();
    winbondSPIFlash.eraseSector(sectorNumber*SECTORSIZE);
    while(winbondSPIFlash.busy());
    winbondSPIFlash.WE();
    winbondSPIFlash.writePage(sectorNumber*SECTORSIZE, magicbuf, MAGICNUMBER_OFFSET);
    while(winbondSPIFlash.busy());

    // Serial.print("sizeof(magicbuf)");
    // Serial.println(sizeof(magicbuf), DEC);


    // uint8_t buf[256];
    // winbondSPIFlash.read(sectorNumber*SECTORSIZE, buf, sizeof(buf) - 1);
    // Serial.print("Buffer: ");
    // for (uint16_t i = 0; i<sizeof(buf); i++){
    //   Serial.print(buf[i], HEX);
    // }
    // Serial.println(" END");

    return 0;
}

// uint8_t SPIAsEEPROM::readInfoBlocks(){
//     uint32_t address;
//     uint16_t n;
//     uint8_t buf[INFOBYTESSECTOROFFSET];
//     for (uint16_t i=0; i < FLASHSIZEUSED/SECTORSIZE; i++) {
//         n = winbondSPIFlash.read(i*SECTORSIZE,buf,INFOBYTESSECTOROFFSET);
//         for (uint16_t j=0; j<36; j++){
//           Serial.print(buf[j], HEX);
//         } 
//         Serial.println(" ");
//     }
//     //winbondSPIFlash.read(address,buf,len);
//     return 0;
// }

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

    //Check if address is outside of the maximum. limit to get inside maximum and continue as normal.
    //Should be changed so that it returns -1 (error)
    if (addressEEPROM >= FLASHSIZEUSED/SECTORSIZE * (SECTORSIZE/FLASH_PAGESIZE - 2)){addressEEPROM = FLASHSIZEUSED/SECTORSIZE * (SECTORSIZE/FLASH_PAGESIZE - 2);}
    
    // Serial.print("byte adress");
    // Serial.println(addressEEPROM, DEC);
    uint8_t buf[INFOBYTES_PER_BYTE];
    
    //Check at what sector number the adress resides. 14 bytes per sector
    sectorNumber = addressEEPROM/(SECTORSIZE/FLASH_PAGESIZE - 2);

    // Serial.print("sectorNumber");
    // Serial.println(sectorNumber, DEC);

    //Check at what page number in the sector the adress can be found (16 pages per sector, 14 used)
    pageNumber = addressEEPROM - (sectorNumber * ((SECTORSIZE/FLASH_PAGESIZE) - 2));

    // Serial.print("pageNumber");
    // Serial.println(pageNumber, DEC);

    //The absulute adress of the infoblock of the byte in flash adress
    infoFlashAddress = sectorNumber*SECTORSIZE + pageNumber * INFOBYTES_PER_BYTE + MAGICNUMBER_OFFSET;

     //read the infoblock and put into the buffer
    winbondSPIFlash.read(infoFlashAddress, buf, sizeof(buf));

    // Serial.print("InfoblockBuffer: ");
    // for (uint16_t i = 0; i<sizeof(buf); i++){
    //   Serial.print(buf[i], HEX);
    // }
    // Serial.println(" END");

    while(winbondSPIFlash.busy());




    //calculate actual flash address of the data
    //Count de number of set bits in the infoblock
    nrOfOnes = count(buf);

    // Serial.print("nrOfOnes");
    // Serial.println(nrOfOnes, DEC);

    //Calulate the adress from all previous information.
    dataFlashAddress = sectorNumber*SECTORSIZE  + (pageNumber * FLASH_PAGESIZE) + INFOBYTESSECTOROFFSET + nrOfOnes - 1;

    uint8_t ByteBuf[1];
    //read the actual byte 
    winbondSPIFlash.read(dataFlashAddress, ByteBuf, sizeof(ByteBuf));
    while(winbondSPIFlash.busy());

    // Serial.print("values from EEPROM");
    // Serial.println(ByteBuf[0], DEC);

    return ByteBuf[0];
}

uint16_t SPIAsEEPROM::count(uint8_t buf[FLASH_PAGESIZE/BITS_PER_BYTE]){
    uint16_t count=0;
    for(uint8_t j=0; j < 32; j++)
      for(uint8_t i=0; i<8; i++){
        if(buf[j] & 1 == 1){ //if current bit 1
          count++;//increase count
        }
        buf[j]=buf[j]>>1;//right shift
      }
    return count;
}

SPIAsEEPROM EEPROM;

#endif