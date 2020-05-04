// /* Speeduino SPIAsEEPROM Library v.2.0.4
//  * Copyright (C) 2020 by Tjeerd Hoogendijk
//  * Created by Tjeerd Hoogendijk - 21/09/2019
//  * Updated by Tjeerd Hoogendijk - 19/04/2020
//  *
//  * This file is part of the Speeduino project. This library is for
//  * Winbond SPI flash memory modules. In its current form it enables reading
//  * and writing individual bytes as if it where an AVR EEPROM. When the begin() 
//  * fuction is called for the first time it will "format" the flash chip. 
//  * !!!!THIS DISTROYS ANY EXISTING DATA ON THE SPI FLASH!!!
//  *  
//  * This Library is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This Library is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License v3.0
//  * along with the Arduino SPIAsEEPROM Library.  If not, see
//  * <http://www.gnu.org/licenses/>.
//  */

// #ifndef FLASH_AS_EEPROMBase_h
// #define FLASH_AS_EEPROMBase_h

// // // #include "FlashStorage.h"
// #include <Arduino.h>
// // #include "winbondflash.h"
// // #include <SPI.h>

// // #define FLASH_SECTORS_USED          255UL  //This can be any number from 1 to many. Depending on the Sector size flash size 
// // #define FLASH_SECTOR_SIZE           4096UL //Flashsector size this is determined by the physical device. This is the smallest block that can be erased at one time 
// // #define EEPROM_BYTES_PER_SECTOR     31 //(FLASH_SECTOR_SIZE/EEPROM_BYTES_PER_SECTOR+1) Must be integer number and aligned with page size of flash used. For windbond align with 256bytes. 
// // #define EEPROM_FLASH_BASEADRESS     0x00100000UL //address to start from can be zero or any other place in flash. make sure EEPROM_FLASH_BASEADRESS+FLASH_SIZE_USED is not over end of flash
// // #define FLASH_SIZE_USED             FLASH_SECTORS_USED*FLASH_SECTOR_SIZE
// // #define FLASH_SIZE_PER_EEPROM_BYTE  FLASH_SECTOR_SIZE/(EEPROM_BYTES_PER_SECTOR + 1)
// // #define ADDRESS_TRANSLATION_SIZE    FLASH_SECTOR_SIZE/(EEPROM_BYTES_PER_SECTOR + 1)/8
// // #define EEPROM_EMULATION_SIZE       FLASH_SECTORS_USED*EEPROM_BYTES_PER_SECTOR

// // #define MAGICNUMBER1                0xC0
// // #define MAGICNUMBER2                0xFF
// // #define MAGICNUMBER3                0xEE
// // #define EEPROM_VERSION              204
// // #define MAGICNUMBER_OFFSET          4

// // #define BITS_PER_BYTE 8 

// class FLASH_EEPROM_BaseClass 
// {

//   public:
//     FLASH_EEPROM_BaseClass(void);

//     /**
//      * Initialize emulated EEPROM in flash
//      * @param flashavailable
//      * @return succes 
//      */
//     int8_t initialize(bool);

//     /**
//      * Read an eeprom cell
//      * @param address
//      * @return value
//      */
//     byte read(uint16_t);

//     /**
//      * Write value to an eeprom cell
//      * @param address
//      * @param value
//      */
//     int8_t write(uint16_t, byte);

//     /**
//      * Update a eeprom cell
//      * @param address
//      * @param value
//      * @return number of bytes written to flash 
//      */
//     int8_t update(uint16_t, uint8_t);

//     /**
//      * Clear emulated eeprom sector
//      * @return sectorsCleared 
//      */
//     int16_t clear();

//     /**
//      * Calculates emulated eeprom length in bytes
//      * @return eeprom length in bytes
//      */
//     uint16_t length();

//     //Class variable indicating if the emulated EEPROM flash is initialized  
//     bool _EmulatedEEPROMAvailable=false;

//     //Class variable storing number of ones counted in adres translation block
//     uint16_t _nrOfOnes = 0;

//     //Class variable storing what sector we are working in.
//     uint16_t _sectorFlash = 0;

//     //Class variable storing what flash address we are working in.
//     uint32_t _addressFLASH = 0;

//     //Class bool indicating if the flash is initialized and available for use
//     bool _FlashAvailable=false;

//     //Readwrite buffer used for flash access through the class. 
//     byte _ReadWriteBuffer[FLASH_SIZE_PER_EEPROM_BYTE];

//   private:

//     /**
//      * Checking for magic numbers on flash if numbers are there no erase is needed else do erase. True if magic numbers are there.
//      * @return Succes. 
//      */
//     bool checkForMagicNumbers();

//     /**
//      * After an erase of a flash sector. New magic numbers must be written to that sector for use.
//      * @param Sector 
//      * @return Succes.
//      */
//     int8_t writeMagicNumbers(uint32_t);

//     /**
//      * For adress translation we need to know the first non 1 bit in the address translation block.
//      * Read the buffer until length and count the numbers of ones in that buffer part. return the count
//      * @param Buffer
//      * @param Length
//      * @return Count.
//      */
//     uint16_t count(byte*, uint16_t);

//     //************************************************* START Implement for actual flash used ****************************************  
//     /**
//      * Read bytes from the flash storage
//      * @param address
//      * @param buffer
//      * @param length
//      * @return succes 
//      */
//     virtual int8_t readFlashBytes(uint32_t , byte*, uint16_t);

//     /**
//      * Write bytes to the flash storage
//      * @param address
//      * @param buffer
//      * @param length
//      * @return succes 
//      */
//     virtual int8_t writeFlashBytes(uint32_t, byte*, uint16_t);

//     /**
//      * Erase a flash sector. Adress determines the flash sector to erase. 
//      * length is specified in number of bytes. if number of bytes > sector size, more than one sector is erased
//      * @param address
//      * @param length
//      * @return succes 
//      */
//     virtual int8_t eraseFlashSector(uint32_t, uint16_t);

//     //************************************************* END Implement for actual flash used ****************************************  
// };

// // class SPI_EEPROM_Class : public FLASH_EEPROM_BaseClass
// // {

// //   public:
// //     SPI_EEPROM_Class(void);

// //     /**
// //      * begin emulated EEPROM in flash
// //      * @param Chip_select_pin
// //      * @return succes 
// //      */
// //     int8_t begin(SPIClass&, uint8_t);


// //     /**
// //      * Read an eeprom cell
// //      * @param address
// //      * @return value
// //      */
// //     byte read(uint16_t);


// //     /**
// //      * Read bytes from the flash storage
// //      * @param address
// //      * @param buffer
// //      * @param length
// //      * @return succes 
// //      */
// //     int8_t readFlashBytes(uint32_t , byte*, uint16_t);

// //     /**
// //      * Write bytes to the flash storage
// //      * @param address
// //      * @param buffer
// //      * @param length
// //      * @return succes 
// //      */
// //     int8_t writeFlashBytes(uint32_t, byte*, uint16_t);

// //     /**
// //      * Erase a flash sector. Adress determines the flash sector to erase. 
// //      * length is specified in number of bytes. if number of bytes > sector size, more than one sector is erased
// //      * @param address
// //      * @param length
// //      * @return succes 
// //      */
// //     int8_t eraseFlashSector(uint32_t, uint16_t);

// //     //winbond flash class instance for interacting with the spi flash chip
// //     winbondFlashSPI winbondSPIFlash;
// // };

// // extern SPI_EEPROM_Class EEPROM;

// // class Internal_EEPROM_Class : public FLASH_EEPROM_BaseClass
// // {

// //   public:
// //     Internal_EEPROM_Class(void);

// //     /**
// //      * begin emulated EEPROM in flash
// //      * @param Chip_select_pin
// //      * @return succes 
// //      */
// //     int8_t begin(SPIClass&, uint8_t);


// //     /**
// //      * Read an eeprom cell
// //      * @param address
// //      * @return value
// //      */
// //     byte read(uint16_t);


// //     /**
// //      * Read bytes from the flash storage
// //      * @param address
// //      * @param buffer
// //      * @param length
// //      * @return succes 
// //      */
// //     int8_t readFlashBytes(uint32_t , byte*, uint16_t);

// //     /**
// //      * Write bytes to the flash storage
// //      * @param address
// //      * @param buffer
// //      * @param length
// //      * @return succes 
// //      */
// //     int8_t writeFlashBytes(uint32_t, byte*, uint16_t);

// //     /**
// //      * Erase a flash sector. Adress determines the flash sector to erase. 
// //      * length is specified in number of bytes. if number of bytes > sector size, more than one sector is erased
// //      * @param address
// //      * @param length
// //      * @return succes 
// //      */
// //     int8_t eraseFlashSector(uint32_t, uint16_t);

// // };

// #endif
