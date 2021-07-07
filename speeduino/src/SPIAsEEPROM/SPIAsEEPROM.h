/* Speeduino SPIAsEEPROM Library v.2.0.4
 * Copyright (C) 2020 by Tjeerd Hoogendijk
 * Created by Tjeerd Hoogendijk - 21/09/2019
 * Updated by Tjeerd Hoogendijk - 19/04/2020
 * Updated by Tjeerd Hoogendijk - 21/07/2020 no new version number
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
 * 
 * ----------------- Explanation of the EEPROM emulation -------------------
 * This explanation is written for novice flash users. If you already know
 * about the details of programming flash memmory and EEPROM emulation in 
 * flash skip this part.
 * 
 * The important thing to rember for prgramming flash
 * 1. It has a limitted number of ERASE cycles. usually ~10k
 * 2. When erased all bits in flash are set so all flash is 0xFF
 * 3. An erase can only be done per flash sector of size X (X=4k or 128k or...) 
 * 4. Writing to flash can be done unlimited amounts of times, but 
 *    you can only write bits from 1 to 0, never from 0 to 1.
 * 
 * This library makes use of the fact it can reset bits but not set bits in 
 * flash. It uses X amount of bits for each emulated EEPROM byte that is 
 * available. For example 512kb of internal flash get 8188 of usable EEPROM bytes. 
 * The benefit of this is you can write X times more to the EMULATED eeprom than
 * writing directly to flash without wearing it down. 
 * 
-------------------------------- IMPLEMENTATION ----------------------------
 * 
 * For every EEPROM byte there are Y amount of flash bytes available. Every write 
 * to the same EEPROM address writes the new value to a +1 location in flash. Until
 * the the buffer of Y locations is full. Than a erase is performed and the whole 
 * cycle starts again. 
 * 
 * To know the location of the last written value there is an address translation part 
 * for each emulated EEPROM address. For every write one of the bits in the address 
 * translation part is reset. So when reading the emulated EEPROM it first reads the 
 * address translation part in flash. The number of ones tells the read function where
 * it can find the current byte of that emulated EEPROM address.
 * 
 * Each flash memmory is devided into erasable sectors. This is a property of the flash 
 * used. You can find the value in the datasheet of the chip. It is FLASH_SECTOR_SIZE macro
 * See table 1. The first sector used is offset by the EEPROM_FLASH_BASEADRESS. The last sector
 * is determiend by the FLASH_SECTORS_USED macro. 
 * 
 * table 1: Each flash memory
 * +-------------------------+----------------------+-------------------+-------------------------------------------------------------+
 * |     Flash Address       | Flash sector address |       Size        |                         Explanation                         |
 * +-------------------------+----------------------+-------------------+-------------------------------------------------------------+
 * | EEPROM_FLASH_BASEADRESS | Flash Sector[0]      | FLASH_SECTOR_SIZE | "Start address of EEPROM emulation in flash"                |
 * | 0000XXXXX               | Flash Sector[1]      | FLASH_SECTOR_SIZE | "Stores "EEPROM_BYTES_PER_SECTOR" bytes of emulated EEPROM" |
 * | 0000XXXXX               | Flash Sector[2]      | FLASH_SECTOR_SIZE | "Stores "EEPROM_BYTES_PER_SECTOR" bytes of emulated EEPROM" |
 * | 0000XXXXX               | .......              | .........         | ..........                                                  |
 * | 0000XXXXX               | Flash Sector[X]      | FLASH_SECTOR_SIZE | "Last Flash sector[FLASH_SECTORS_USED -1]                   |
 * +-------------------------+----------------------+-------------------+-------------------------------------------------------------+
 * 
 * Each sector is devided into equaly sized sections of FLASH_SECTOR_SIZE/(EEPROM_BYTES_PER_SECTOR+1)
 * The first section is used to store some identifiable numbers to show the sector is used
 * for EEPROM emulation. Every following section is used to store emulated EEPROM data per address.
 * See table 2
 * 
 * table 2: Each flash sector
 * +--------------------------+---------------------+-------------------------------------------------+-------------------------------------------------------------------------+
 * | Flash Address in sector  |   EEPROM address    |                      Size                       |                               Explanation                               |
 * +--------------------------+---------------------+-------------------------------------------------+-------------------------------------------------------------------------+
 * | 000000000                | Magic numbers       | FLASH_SECTOR_SIZE/(EEPROM_BYTES_PER_SECTOR + 1) | "Magic numbers and EEPROM version at the start of each flash sector"    |
 * | 0000XXXXX                | Emulated EEPROM [0] | FLASH_SECTOR_SIZE/(EEPROM_BYTES_PER_SECTOR + 1) | "EEPROM section[0], Address translation and value of eeprom address[0]" |
 * | 0000XXXXX                | Emulated EEPROM [1] | FLASH_SECTOR_SIZE/(EEPROM_BYTES_PER_SECTOR + 1) | "EEPROM section[1], Address translation and value of eeprom address[1]" |
 * | 0000XXXXX                | .......             | .........                                       | ..........                                                              |
 * | 0000XXXXX                | Emulated EEPROM [X] | FLASH_SECTOR_SIZE/(EEPROM_BYTES_PER_SECTOR + 1) | "Last EEPROM section[EEPROM_BYTES_PER_SECTOR -1]                        |
 * +--------------------------+---------------------+-------------------------------------------------+-------------------------------------------------------------------------+
 * 
 * Each section that is used for EEPROM emulation starts with the address translation part. This has minimum 
 * of Y bits to store the address translation. The rest is used for historic EEPROM written data for each EEPROM 
 * address. See 
 * 
 * table 3: Each section in the flash sector
 * +---------------------------+---------------------+---------------------------------------------------+------------------------------------------------------------------------------------------------------------+
 * | Address in sector section |     Description     |                       Size                        |                                                Explanation                                                 |
 * +---------------------------+---------------------+---------------------------------------------------+------------------------------------------------------------------------------------------------------------+
 * | 00000000X                 | Address translation | FLASH_SECTOR_SIZE/(EEPROM_BYTES_PER_SECTOR + 1)/8 | "Number of ones translates to address at what location the latest value is, a one is reset for each write" |
 * | 000000X+1                 | Emulated EEPROM [Y] | 1                                                 | "EEPROM value for the last write to emulated EEPROM at address Y. After this erase of flash sector"        |
 * | 000000X+2                 | Emulated EEPROM [Y] | 1                                                 | "Emulated EEPROM address Y byte write [last-1]"                                                            |
 * | 000000X+3                 | .......             | .........                                         | ..........                                                                                                 |
 * | 0000XXXXX                 | Emulated EEPROM [Y] | 1                                                 | "Location of a EEPROM value for the first write to emulated EEPROM at address Y"                           |
 * +---------------------------+---------------------+---------------------------------------------------+------------------------------------------------------------------------------------------------------------+
 */

#ifndef FLASH_AS_EEPROM_h
#define FLASH_AS_EEPROM_h

#include <Arduino.h>
#include "winbondflash.h"
#include <SPI.h>

// #elif defined(STM32F103xB) 
//   #include "stm32_def.h"
//   //Internal flash STM32F407 EEPROM emulation
//   #define FLASH_SECTORS_USED          9UL  //This can be any number from 1 to many. 
//   #define FLASH_SECTOR_SIZE           1024UL //Flash sector size this is determined by the physical device. This is the smallest block that can be erased at one time 
//   #define EEPROM_BYTES_PER_SECTOR     127UL //(FLASH_SECTOR_SIZE/EEPROM_BYTES_PER_SECTOR+1) Must be integer number and aligned with page size of flash used. 
//   #define EEPROM_FLASH_BASEADRESS     0x801D400UL //address to start from can be zero or any other place in flash. make sure EEPROM_FLASH_BASEADRESS+FLASH_SIZE_USED is not over end of flash
// #endif

#define MAGICNUMBER1                0xC0
#define MAGICNUMBER2                0xFF
#define MAGICNUMBER3                0xEE
#define EEPROM_VERSION              204 //V2.0.4
#define MAGICNUMBER_OFFSET          4

#define BITS_PER_BYTE 8 

typedef struct {
  uint32_t Flash_Sectors_Used;        //This the number of flash sectors used for EEPROM emulation can be any number from 1 to many. 
  uint32_t Flash_Sector_Size;         //Flash sector size: This is determined by the physical device. This is the smallest block that can be erased at one time 
  uint32_t EEPROM_Bytes_Per_Sector;   //EEPROM bytes per sector: (Flash sector size/EEPROM bytes per sector+1) -> Must be integer number and aligned with page size of flash used. 
  uint32_t EEPROM_Flash_BaseAddress;  //Flash address to start Emulation from, can be zero or any other place in flash. make sure EEPROM_FLASH_BASEADRESS+FLASH_SIZE_USED is not over end of flash
} EEPROM_Emulation_Config;

typedef struct {
  uint16_t pinChipSelect;
  SPIClass SPIport;
} Flash_SPI_Config;

//Base class for flash read and write. SPI and internal flash inherrit from this class. 
class FLASH_EEPROM_BaseClass 
{

  public:
    FLASH_EEPROM_BaseClass(EEPROM_Emulation_Config);

    /**
     * Initialize emulated EEPROM in flash
     * @param flashavailable
     * @return succes 
     */
    int8_t initialize(bool);

    /**
     * Read an eeprom cell
     * @param address
     * @return value
     */
    byte read(uint16_t);

    /**
     * Write value to an eeprom cell
     * @param address
     * @param value
     */
    int8_t write(uint16_t, byte);

    /**
     * Update a eeprom cell
     * @param address
     * @param value
     * @return number of bytes written to flash 
     */
    int8_t update(uint16_t, uint8_t);

    /**
     * Read AnyTypeOfData from eeprom 
     * @param address
     * @return AnyTypeOfData
     */
    template< typename T > T &get( int idx, T &t ){
        uint16_t e = idx;
        uint8_t *ptr = (uint8_t*) &t;
        for( int count = sizeof(T) ; count ; --count, ++e )  *ptr++ = read(e);
        return t;
    }

    /**
     * Write AnyTypeOfData to eeprom
     * @param address 
     * @param AnyTypeOfData 
     * @return number of bytes written to flash 
     */
    template< typename T > const T &put( int idx, const T &t ){        
        const uint8_t *ptr = (const uint8_t*) &t;
        uint16_t e = idx;
        for( int count = sizeof(T) ; count ; --count, ++e )  write(e, *ptr++);
        return t;
    }

    /**
     * Clear emulated eeprom sector
     * @return sectorsCleared 
     */
    int16_t clear();

    /**
     * Calculates emulated eeprom length in bytes
     * @return eeprom length in bytes
     */
    uint16_t length();

    //Class variable indicating if the emulated EEPROM flash is initialized  
    bool _EmulatedEEPROMAvailable=false;

    //Class variable storing number of ones counted in adres translation block
    uint32_t _nrOfOnes = 0;

    //Class variable storing what sector we are working in.
    uint32_t _sectorFlash = 0;

    //Class variable storing what flash address we are working in.
    uint32_t _addressFLASH = 0;

    //Class bool indicating if the flash is initialized and available for use
    bool _FlashAvailable=false;

    //Readwrite buffer used for flash access through the class. 
    byte _ReadWriteBuffer[128]; //make sure the FLASH_SECTOR_SIZE/EEPROM_BYTES_PER_SECTOR+1 < 128. Else increase this number.

    EEPROM_Emulation_Config _config;
    uint32_t _Flash_Size_Used;
    uint32_t _Flash_Size_Per_EEPROM_Byte;
    uint32_t _Addres_Translation_Size;
    uint32_t _EEPROM_Emulation_Size;

  private:

    /**
     * Checking for magic numbers on flash if numbers are there no erase is needed else do erase. True if magic numbers are there.
     * @return Succes. 
     */
    bool checkForMagicNumbers();

    /**
     * After an erase of a flash sector. New magic numbers must be written to that sector for use.
     * @param Sector 
     * @return Succes.
     */
    int8_t writeMagicNumbers(uint32_t);

    /**
     * For adress translation we need to know the first non 1 bit in the address translation block.
     * Read the buffer until length and count the numbers of ones in that buffer part. return the count
     * @param Buffer
     * @param Length
     * @return Count.
     */
    uint16_t count(byte*, uint32_t);

    //************************************************* START Implement for actual flash used ****************************************  
    /**
     * Read bytes from the flash storage
     * @param address
     * @param buffer
     * @param length
     * @return succes 
     */
    virtual int8_t readFlashBytes(uint32_t , byte*, uint32_t);

    /**
     * Write bytes to the flash storage
     * @param address
     * @param buffer
     * @param length
     * @return succes 
     */
    virtual int8_t writeFlashBytes(uint32_t, byte*, uint32_t);

    /**
     * Erase a flash sector. Adress determines the flash sector to erase. 
     * length is specified in number of bytes. if number of bytes > sector size, more than one sector is erased
     * @param address
     * @param length
     * @return succes 
     */
    virtual int8_t eraseFlashSector(uint32_t, uint32_t);

    //************************************************* END Implement for actual flash used ****************************************  
};

//SPI flash class for SPI flash EEPROM emulation. Inherrit most from the base class. 
class SPI_EEPROM_Class : public FLASH_EEPROM_BaseClass
{

  public:
    SPI_EEPROM_Class(EEPROM_Emulation_Config, Flash_SPI_Config);

    /**
     * begin emulated EEPROM in flash
     * @param Chip_select_pin
     * @param SPI_object
     * @return succes 
     */
    int8_t begin(SPIClass&, uint8_t);


    /**
     * Read an eeprom cell
     * @param address
     * @return value
     */
    byte read(uint16_t);


    /**
     * Read bytes from the flash storage
     * @param address
     * @param buffer
     * @param length
     * @return succes 
     */
    int8_t readFlashBytes(uint32_t , byte*, uint32_t);

    /**
     * Write bytes to the flash storage
     * @param address
     * @param buffer
     * @param length
     * @return succes 
     */
    int8_t writeFlashBytes(uint32_t, byte*, uint32_t);

    /**
     * Erase a flash sector. Adress determines the flash sector to erase. 
     * length is specified in number of bytes. if number of bytes > sector size, more than one sector is erased
     * @param address
     * @param length
     * @return succes 
     */
    int8_t eraseFlashSector(uint32_t, uint32_t);

    //winbond flash class instance for interacting with the spi flash chip
    winbondFlashSPI winbondSPIFlash;

    //SPI configuration struct. Now only the CS pins is used, future extension can be the use SPI object or MOSI/MISO/SCK pins
    Flash_SPI_Config _configSPI;
};

//Internal flash class for flash EEPROM emulation. Inherrit most from the base class. 
//Internal flash of the STM32F407VE6 is listed as 512kb total. But in reality is 1024kb
//The last 512kb flash is used for the EEPROM emulation 
class InternalSTM32F4_EEPROM_Class : public FLASH_EEPROM_BaseClass
{

  public:
    InternalSTM32F4_EEPROM_Class(EEPROM_Emulation_Config);

    /**
     * Read an eeprom cell
     * @param address
     * @return value
     */
    byte read(uint16_t);


    /**
     * Read bytes from the flash storage
     * @param address
     * @param buffer
     * @param length
     * @return succes 
     */
    int8_t readFlashBytes(uint32_t , byte*, uint32_t);

    /**
     * Write bytes to the flash storage
     * @param address
     * @param buffer
     * @param length
     * @return succes 
     */
    int8_t writeFlashBytes(uint32_t, byte*, uint32_t);

    /**
     * Erase a flash sector. Adress determines the flash sector to erase. 
     * length is specified in number of bytes. if number of bytes > sector size, more than one sector is erased
     * @param address
     * @param length
     * @return succes 
     */
    int8_t eraseFlashSector(uint32_t, uint32_t);
};


// class InternalSTM32F1_EEPROM_Class : public FLASH_EEPROM_BaseClass
// {

//   public:
//     InternalSTM32F1_EEPROM_Class(EEPROM_Emulation_Config);

//     /**
//      * Read an eeprom cell
//      * @param address
//      * @return value
//      */
//     byte read(uint16_t);


//     /**
//      * Read bytes from the flash storage
//      * @param address
//      * @param buffer
//      * @param length
//      * @return succes 
//      */
//     int8_t readFlashBytes(uint32_t , byte*, uint32_t);

//     /**
//      * Write bytes to the flash storage
//      * @param address
//      * @param buffer
//      * @param length
//      * @return succes 
//      */
//     int8_t writeFlashBytes(uint32_t, byte*, uint32_t);

//     /**
//      * Erase a flash sector. Adress determines the flash sector to erase. 
//      * length is specified in number of bytes. if number of bytes > sector size, more than one sector is erased
//      * @param address
//      * @param length
//      * @return succes 
//      */
//     int8_t eraseFlashSector(uint32_t, uint32_t);
// };

class InternalSTM32F7_EEPROM_Class : public FLASH_EEPROM_BaseClass
{
  public:
    InternalSTM32F7_EEPROM_Class(EEPROM_Emulation_Config);

    /**
     * Read an eeprom cell
     * @param address
     * @return value
     */
    byte read(uint16_t);


    /**
     * Read bytes from the flash storage
     * @param address
     * @param buffer
     * @param length
     * @return succes 
     */
    int8_t readFlashBytes(uint32_t , byte*, uint32_t);

    /**
     * Write bytes to the flash storage
     * @param address
     * @param buffer
     * @param length
     * @return succes 
     */
    int8_t writeFlashBytes(uint32_t, byte*, uint32_t);

    /**
     * Erase a flash sector. Adress determines the flash sector to erase. 
     * length is specified in number of bytes. if number of bytes > sector size, more than one sector is erased
     * @param address
     * @param length
     * @return succes 
     */
    int8_t eraseFlashSector(uint32_t, uint32_t);
};

#endif
