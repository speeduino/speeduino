/* Speeduino SPIAsEEPROM Library v.2.0.4
 * Copyright (C) 2020 by Tjeerd Hoogendijk
 * Created by Tjeerd Hoogendijk - 21/09/2019
 * Updated by Tjeerd Hoogendijk - 19/04/2020
 *
 * This file is part of the Speeduino project. This library started out for
 * Winbond SPI flash memory modules. As of version 2.0 it also works with internal
 * flash memory of the STM32F407.  In its current form it enables reading
 * and writing individual bytes as if it where an AVR EEPROM. When the begin() 
 * fuction is called for the first time it will "format" the flash chip. 
 * !!!!THIS DISTROYS ANY EXISTING DATA ON THE FLASH!!!!
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
 * along with the Speeduino SPIAsEEPROM Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if defined(USE_SPI_EEPROM) | defined(STM32F407xx) | defined(STM32F103xB)

#include "SPIAsEEPROM.h"

static EEPROM_Emulation_Config EmulatedEEPROMMconfig{
    FLASH_SECTORS_USED,
    FLASH_SECTOR_SIZE,
    EEPROM_BYTES_PER_SECTOR,
    EEPROM_FLASH_BASEADRESS
};

// EmulatedEEPROMMconfig.Flash_Sectors_Used = FLASH_SECTORS_USED;

FLASH_EEPROM_BaseClass::FLASH_EEPROM_BaseClass(EEPROM_Emulation_Config config)
{
    //Class indicating if the emulated EEPROM flash is initialized  
   _EmulatedEEPROMAvailable=false;

    //Class variable storing number of ones counted in adres translation block
    _nrOfOnes = 0;

    //Class variable storing what sector we are working in.
    _sectorFlash = 0;

    //Class variable storing what flash address we are working in.
    _addressFLASH = 0;

    //Class bool indicating if the flash is initialized and available for use
    _FlashAvailable=false;

    //save configuration 
    _config = config;

    _Flash_Size_Used = _config.Flash_Sectors_Used*_config.Flash_Sector_Size;
    _Flash_Size_Per_EEPROM_Byte = _config.Flash_Sector_Size/(_config.EEPROM_Bytes_Per_Sector +1);
    _Addres_Translation_Size = _Flash_Size_Per_EEPROM_Byte/8;
    _EEPROM_Emulation_Size = _config.Flash_Sectors_Used*_config.EEPROM_Bytes_Per_Sector;
}

int8_t FLASH_EEPROM_BaseClass::initialize(bool flashavailable)
{
    bool formatted = false;
    _FlashAvailable = flashavailable;
    _EmulatedEEPROMAvailable = false;

    if(_FlashAvailable)
    {
      formatted = checkForMagicNumbers(); 

      //If not formatted format flash. This takes 10 seconds or more!
      if(!formatted){
        clear();
        //check if format succeeded
        formatted = checkForMagicNumbers();
      }

      if(formatted){_EmulatedEEPROMAvailable=true;}
    }
    return _EmulatedEEPROMAvailable;
}

byte FLASH_EEPROM_BaseClass::read(uint16_t addressEEPROM){
    //version 0.1 does not check magic number

    byte EEPROMbyte;
   
    //Check if address is outside of the maximum. return zero if address is out of range.
    if (addressEEPROM > _EEPROM_Emulation_Size){addressEEPROM = _EEPROM_Emulation_Size - 1; return 0;}  
    
    //Check at what flash sector the EEPROM byte information resides 
    _sectorFlash = addressEEPROM/_config.EEPROM_Bytes_Per_Sector;

    //Check at what flash address the EEPROM byte information resides 
    _addressFLASH = (_sectorFlash*_config.Flash_Sector_Size) + ((addressEEPROM % _config.EEPROM_Bytes_Per_Sector) + 1) * _Flash_Size_Per_EEPROM_Byte;

    readFlashBytes(_addressFLASH, _ReadWriteBuffer, _Flash_Size_Per_EEPROM_Byte);

    //calculate address of the valid data by couting the bits in the Address translation section
    _nrOfOnes = count(_ReadWriteBuffer, _Addres_Translation_Size);
    
    //Bring number of ones within specification of buffer size. 
    if(_nrOfOnes >=_Flash_Size_Per_EEPROM_Byte){_nrOfOnes =_Flash_Size_Per_EEPROM_Byte;}

    //The buffer is already filled with the latest information. Read the valid data at the correct address.

    //If it is the first read after clear (all ones still set), return 0xFF;
    if (_nrOfOnes==_Flash_Size_Per_EEPROM_Byte){
      EEPROMbyte = 0xFF;
    }else{
      EEPROMbyte = _ReadWriteBuffer[_nrOfOnes];
    }
    
    return EEPROMbyte;
}

int8_t FLASH_EEPROM_BaseClass::write(uint16_t addressEEPROM, byte val){    
    //Check if address is outside of the maximum. limit to get inside maximum and return an error.
    if (addressEEPROM > _EEPROM_Emulation_Size){addressEEPROM = _EEPROM_Emulation_Size - 1; return -1;}  
    
    //read the current value
    uint8_t readValue = read(addressEEPROM);

    //After reading the current byte all global variables containing information about the address are set correctly. 

    //only write if value is changed. 
    if (readValue != val){
      //Check if section is full and an erase must be performed.
      if (_nrOfOnes < _Addres_Translation_Size + 1){

        //First read all the values in this sector that will get distroyed when erasing
        byte tempBuf[_config.EEPROM_Bytes_Per_Sector];
        for(uint16_t i = 0; i<_config.EEPROM_Bytes_Per_Sector; i++){
            uint16_t TempEEPROMaddress = (_sectorFlash*_config.EEPROM_Bytes_Per_Sector) + i;
            tempBuf[i] = read(TempEEPROMaddress);
        }

        //Now erase the sector
        eraseFlashSector(_sectorFlash*_config.Flash_Sector_Size, _config.Flash_Sector_Size);

        //Write the magic numbers 
        writeMagicNumbers(_sectorFlash);

        //write all the values back
        for(uint16_t i=0; i<_config.EEPROM_Bytes_Per_Sector; i++){
            write((_sectorFlash*_config.EEPROM_Bytes_Per_Sector) + i, tempBuf[i]);
        }

        //Do not forget to write the new value!
        write(addressEEPROM, val);

        //Return we have writen a whole sector. 
        return 0xFF;
        
      }

      //determine the adress of the byte in the address translation section where one bit must be reset when writing new values 
      uint8_t AdressInAddressTranslation = (_nrOfOnes - 1)/8;

      //determine value of the infoblock byte after writing one more time.
      uint8_t ValueInAddressTranslation = 0xFF << (8 - (_nrOfOnes - 1 - ((AdressInAddressTranslation) * 8)));  
      
      //write the new adress translation value at the new location in buffer
      _ReadWriteBuffer[AdressInAddressTranslation] = ValueInAddressTranslation;

      //Write the new EEPROM value at the new location in the buffer.
      _nrOfOnes--; 
      _ReadWriteBuffer[_nrOfOnes] = val;

      //Write the buffer to the undelying flash storage. 
      // writeFlashBytes(_addressFLASH, _ReadWriteBuffer, _Flash_Size_Per_EEPROM_Byte);

      //Write actual value part of the buffer to flash   
      byte tempBuffer[4];
      _nrOfOnes &= ~(0x3); //align address with 2 byte (uint16_t) for write to flash for 32bit MCU
      memcpy(&tempBuffer, &_ReadWriteBuffer[_nrOfOnes], sizeof(uint32_t));
      writeFlashBytes(_addressFLASH +_nrOfOnes, tempBuffer, sizeof(uint32_t));

      //Write address translation part of the buffer to flash
      AdressInAddressTranslation &= ~(0x3); //align address with 4 byte for write to flash for 32bit MCU
      memcpy(&tempBuffer, &_ReadWriteBuffer[AdressInAddressTranslation], sizeof(uint32_t));
      writeFlashBytes(_addressFLASH+AdressInAddressTranslation, tempBuffer, sizeof(uint32_t));
      return 1;
    }  
  return 0;
}

int8_t FLASH_EEPROM_BaseClass::update(uint16_t addressEEPROM, uint8_t val){
  return write(addressEEPROM, val);
}

int16_t FLASH_EEPROM_BaseClass::clear(){
      uint32_t i;
      for(i=0; i< _config.Flash_Sectors_Used; i++ ){
          eraseFlashSector(i*_config.Flash_Sector_Size, _config.Flash_Sector_Size);
          writeMagicNumbers(i);
      }
      return i;
}

uint16_t FLASH_EEPROM_BaseClass::length(){ return _EEPROM_Emulation_Size; }


bool FLASH_EEPROM_BaseClass::checkForMagicNumbers(){
      bool magicnumbers = true;
      for(uint32_t i=0; i< _config.Flash_Sectors_Used; i++ ){
          readFlashBytes(i*_config.Flash_Sector_Size, _ReadWriteBuffer, _Flash_Size_Per_EEPROM_Byte);
          if((_ReadWriteBuffer[0] != MAGICNUMBER1) | (_ReadWriteBuffer[1] != MAGICNUMBER2) | (_ReadWriteBuffer[2] != MAGICNUMBER3) | (_ReadWriteBuffer[3] != EEPROM_VERSION)){magicnumbers=false;}
      }
      return magicnumbers;
}

int8_t FLASH_EEPROM_BaseClass::writeMagicNumbers(uint32_t sector){ 
    _ReadWriteBuffer[0] = MAGICNUMBER1;
    _ReadWriteBuffer[1] = MAGICNUMBER2;
    _ReadWriteBuffer[2] = MAGICNUMBER3;
    _ReadWriteBuffer[3] = EEPROM_VERSION;
    writeFlashBytes(sector*_config.Flash_Sector_Size, _ReadWriteBuffer, MAGICNUMBER_OFFSET); 
  return true;
}

uint16_t FLASH_EEPROM_BaseClass::count(byte* buffer, uint32_t length){
    byte tempBuffer[length];
    memcpy(&tempBuffer, buffer, length);
    uint16_t count=0;
    for(uint8_t j=0; j < length; j++)
      for(uint8_t i=0; i<8; i++){
        if((tempBuffer[j] & 1) == 1){ //if current bit 1
          count++;//increase count
        }
        tempBuffer[j]=tempBuffer[j]>>1;//right shift
      }
    return count;
}

int8_t FLASH_EEPROM_BaseClass::readFlashBytes(uint32_t address , byte* buffer, uint32_t length){return -1;}
int8_t FLASH_EEPROM_BaseClass::writeFlashBytes(uint32_t address, byte* buffer, uint32_t length){return -1;}
int8_t FLASH_EEPROM_BaseClass::eraseFlashSector(uint32_t address, uint32_t length){return -1;}

#endif

#if defined(USE_SPI_EEPROM)
SPI_EEPROM_Class::SPI_EEPROM_Class(EEPROM_Emulation_Config config):FLASH_EEPROM_BaseClass(config)
{

}

byte SPI_EEPROM_Class::read(uint16_t addressEEPROM){
    //Check if emulated EEPROM is available if not yet start it first.
    if(!_EmulatedEEPROMAvailable){
      //22.5Mhz is highest it could get with this. But should be ~45Mhz :-(. 
      SPISettings settings(22500000, MSBFIRST, SPI_MODE0);
      SPI.beginTransaction(settings);
      begin(SPI, USE_SPI_EEPROM);
    }
    
    return FLASH_EEPROM_BaseClass::read(addressEEPROM);
}

int8_t SPI_EEPROM_Class::begin(SPIClass &_spi, uint8_t pinSPIFlash_CS=6){
    pinMode(pinSPIFlash_CS, OUTPUT);
    bool flashavailable;
    flashavailable = winbondSPIFlash.begin(_W25Q16,_spi, pinSPIFlash_CS);
    return FLASH_EEPROM_BaseClass::initialize(flashavailable);
}    

int8_t SPI_EEPROM_Class::readFlashBytes(uint32_t address, byte *buf, uint32_t length){
  while(winbondSPIFlash.busy());
  return winbondSPIFlash.read(address+_config.EEPROM_Flash_BaseAddress, buf, length);
}

int8_t SPI_EEPROM_Class::writeFlashBytes(uint32_t address, byte *buf, uint32_t length){
  winbondSPIFlash.setWriteEnable(true);
  winbondSPIFlash.writePage(address+_config.EEPROM_Flash_BaseAddress, buf, length);
  while(winbondSPIFlash.busy());
  return 0;
}

int8_t SPI_EEPROM_Class::eraseFlashSector(uint32_t address, uint32_t length){
  winbondSPIFlash.setWriteEnable(true);
  winbondSPIFlash.eraseSector(address+_config.EEPROM_Flash_BaseAddress);
  while(winbondSPIFlash.busy());
  return 0;
}


//THIS IS NOT WORKING! FOR STM32F103 YOU CAN ONLY WRITE IN JUST ERASED HALFWORDS(UINT16_T). THE PHILISOPHY IS FLAWWED THERE.
// #elif defined(STM32F103xB)

// InternalSTM32F1_EEPROM_Class::InternalSTM32F1_EEPROM_Class(EEPROM_Emulation_Config config):FLASH_EEPROM_BaseClass(config)
// {
  
// }

// byte InternalSTM32F1_EEPROM_Class::read(uint16_t addressEEPROM){  
//   if(!_EmulatedEEPROMAvailable){
//       FLASH_EEPROM_BaseClass::initialize(true);
//   }
//   return FLASH_EEPROM_BaseClass::read(addressEEPROM);
// }

// int8_t InternalSTM32F1_EEPROM_Class::readFlashBytes(uint32_t address, byte *buf, uint32_t length){
//   memcpy(buf, (uint8_t *)(_config.EEPROM_Flash_BaseAddress + address), length);
//   return 0;
// }

// int8_t InternalSTM32F1_EEPROM_Class::writeFlashBytes(uint32_t flashAddress, byte *buf, uint32_t length){
//  {
//   uint32_t translatedAddress = flashAddress+_config.EEPROM_Flash_BaseAddress;
//   uint32_t data = 0;
//   uint32_t countaddress = translatedAddress; 
//   HAL_FLASH_Unlock();
//   while (countaddress < translatedAddress + length) {
//       memcpy(&data, buf, sizeof(uint32_t));
//       if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, countaddress, data) == HAL_OK) {
//         countaddress += 4;
//         offset += 4;
//       } else {
//         countaddress = translatedAddress + length + 1;
//       }
//     }
//   }
//   HAL_FLASH_Lock();
//   return 0;
// }

// int8_t InternalSTM32F1_EEPROM_Class::eraseFlashSector(uint32_t address, uint32_t length){
//   FLASH_EraseInitTypeDef EraseInitStruct;
//   uint32_t pageError = 0;
//   uint32_t realAddress = _config.EEPROM_Flash_BaseAddress+address;
//   bool EraseSucceed=false;

//   /* ERASING page */
//   EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
//   EraseInitStruct.Banks = 1;
//   EraseInitStruct.PageAddress = realAddress;
//   EraseInitStruct.NbPages = 1;

//   HAL_FLASH_Unlock();
//   if (HAL_FLASHEx_Erase(&EraseInitStruct, &pageError) == HAL_OK){EraseSucceed=true;}
//   HAL_FLASH_Lock();
//   return EraseSucceed;
// }

#elif defined(STM32F407xx)

InternalSTM32F4_EEPROM_Class::InternalSTM32F4_EEPROM_Class(EEPROM_Emulation_Config config) : FLASH_EEPROM_BaseClass(config)
{
  
}

byte InternalSTM32F4_EEPROM_Class::read(uint16_t addressEEPROM){  
  if(!_EmulatedEEPROMAvailable){
      FLASH_EEPROM_BaseClass::initialize(true);
  }
  return FLASH_EEPROM_BaseClass::read(addressEEPROM);
}

int8_t InternalSTM32F4_EEPROM_Class::readFlashBytes(uint32_t address, byte *buf, uint32_t length){
  memcpy(buf, (uint8_t *)(_config.EEPROM_Flash_BaseAddress + address), length);
  return 0;
}

int8_t InternalSTM32F4_EEPROM_Class::writeFlashBytes(uint32_t flashAddress, byte *buf, uint32_t length){
 {
  uint32_t translatedAddress = flashAddress+_config.EEPROM_Flash_BaseAddress;
  uint32_t data = 0;
  uint32_t offset = 0;
  uint32_t countaddress = translatedAddress; 
  HAL_FLASH_Unlock();
  while (countaddress < translatedAddress + length) {
      memcpy(&data, buf + offset, sizeof(uint32_t));
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, countaddress, data) == HAL_OK) {
        countaddress += 4;
        offset += 4;
      } else {
        countaddress = translatedAddress + length + 1;
      }
    }
  }
  HAL_FLASH_Lock();
  return 0;
}

int8_t InternalSTM32F4_EEPROM_Class::eraseFlashSector(uint32_t address, uint32_t length){
  FLASH_EraseInitTypeDef EraseInitStruct;
  // uint32_t offset = 0;
  uint32_t realAddress = _config.EEPROM_Flash_BaseAddress+address;
  // uint32_t address_end = FLASH_BASE_ADDRESS + E2END;
  bool EraseSucceed=false;
  uint32_t SectorError = 0;
  uint32_t _Sector = 11;

  //Look in the datasheet for more information about flash sectors and sizes
  //This is the correct sector allocation for the STM32F407 all types
  if ((realAddress>=0x08000000UL)&(realAddress<=0x08003FFFUL)){_Sector = 0;}
  if ((realAddress>=0x08004000UL)&(realAddress<=0x08007FFFUL)){_Sector = 1;}
  if ((realAddress>=0x08008000UL)&(realAddress<=0x0800BFFFUL)){_Sector = 2;}
  if ((realAddress>=0x0800C000UL)&(realAddress<=0x0800FFFFUL)){_Sector = 3;}
  if ((realAddress>=0x08010000UL)&(realAddress<=0x0801FFFFUL)){_Sector = 4;}
  if ((realAddress>=0x08020000UL)&(realAddress<=0x0803FFFFUL)){_Sector = 5;}
  if ((realAddress>=0x08040000UL)&(realAddress<=0x0805FFFFUL)){_Sector = 6;}
  if ((realAddress>=0x08050000UL)&(realAddress<=0x0807FFFFUL)){_Sector = 7;}
  if ((realAddress>=0x08080000UL)&(realAddress<=0x0809FFFFUL)){_Sector = 8;}
  if ((realAddress>=0x080A0000UL)&(realAddress<=0x080BFFFFUL)){_Sector = 9;}
  if ((realAddress>=0x080C0000UL)&(realAddress<=0x080DFFFFUL)){_Sector = 10;}
  if ((realAddress>=0x080E0000UL)&(realAddress<=0x080FFFFFUL)){_Sector = 11;}

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = _Sector;
  EraseInitStruct.NbSectors = 1;

  HAL_FLASH_Unlock();
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK){EraseSucceed=true;}
  HAL_FLASH_Lock();
  return EraseSucceed;
}
#endif

#if defined(USE_SPI_EEPROM)
  SPI_EEPROM_Class EEPROM(EmulatedEEPROMMconfig);
#elif defined(STM32F407xx) 
  InternalSTM32F4_EEPROM_Class EEPROM(EmulatedEEPROMMconfig);
#endif




