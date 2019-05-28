/* Arduino SPIMemory Library v.3.2.1
 * Copyright (C) 2017 by Prajwal Bhattaram
 * Created by Prajwal Bhattaram - 19/05/2015
 * Modified by @boseji <salearj@hotmail.com> - 02/03/2017
 * Modified by Prajwal Bhattaram - 21/05/2018
 *
 * This file is part of the Arduino SPIMemory Library. This library is for
 * Winbond NOR flash memory modules. In its current form it enables reading
 * and writing individual data variables, structs and arrays from and to various locations;
 * reading and writing pages; continuous read functions; sector, block and chip erase;
 * suspending and resuming programming/erase and powering down for low power operation.
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

 #include "SPIFlash.h"

 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
 //     Private functions used by read, write and erase operations     //
 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
 // Creates bit mask from bit x to bit y
 unsigned SPIFlash::_createMask(unsigned x, unsigned y) {
   unsigned r = 0;
   for (unsigned i=x; i<=y; i++) {
     r |= 1 << i;
   }
   return r;
 }

 //Checks to see if page overflow is permitted and assists with determining next address to read/write.
 //Sets the global address variable
 bool SPIFlash::_addressCheck(uint32_t _addr, uint32_t size) {
   uint32_t _submittedAddress = _addr;
   uint8_t _errorcode = error();
   if (_errorcode == UNKNOWNCAP || _errorcode == NORESPONSE) {
     return false;
   }
 	if (!_chip.capacity) {
     _troubleshoot(CALLBEGIN);
     return false;
 	}

   //Serial.print("_chip.capacity: ");
   //Serial.println(_chip.capacity, HEX);

   if (_submittedAddress + size >= _chip.capacity) {
     //Serial.print("_submittedAddress + size: ");
     //Serial.println(_submittedAddress + size, HEX);
   #ifdef DISABLEOVERFLOW
     _troubleshoot(OUTOFBOUNDS);
     return false;					// At end of memory - (!pageOverflow)
   #else
     _addressOverflow = ((_submittedAddress + size) - _chip.capacity);
     _currentAddress = _addr;
     //Serial.print("_addressOverflow: ");
     //Serial.println(_addressOverflow, HEX);
     return true;					// At end of memory - (pageOverflow)
   #endif
   }
   else {
     _addressOverflow = false;
     _currentAddress = _addr;
     return true;				// Not at end of memory if (address < _chip.capacity)
   }
   //Serial.print("_currentAddress: ");
   //Serial.println(_currentAddress, HEX);
 }

 // Checks to see if the block of memory has been previously written to
 bool SPIFlash::_notPrevWritten(uint32_t _addr, uint32_t size) {
   //uint8_t _dat;
   _beginSPI(READDATA);
   for (uint32_t i = 0; i < size; i++) {
     if (_nextByte(READ) != 0xFF) {
       CHIP_DESELECT;
       _troubleshoot(PREVWRITTEN);
       return false;
     }
   }
   CHIP_DESELECT
   return true;
 }

 //Double checks all parameters before calling a read or write. Comes in two variants
 //Takes address and returns the address if true, else returns false. Throws an error if there is a problem.
 bool SPIFlash::_prep(uint8_t opcode, uint32_t _addr, uint32_t size) {
   // If the flash memory is >= 256 MB enable 4-byte addressing
   if (_chip.manufacturerID == WINBOND_MANID && _addr >= MB(16)) {
     if (!_enable4ByteAddressing()) {    // If unable to enable 4-byte addressing
       return false;
     }          // TODO: Add SFDP compatibility here
   }
   switch (opcode) {
     case PAGEPROG:
     //Serial.print(F("Address being prepped: "));
     //Serial.println(_addr);
     #ifndef HIGHSPEED
       if(_isChipPoweredDown() || !_addressCheck(_addr, size) || !_notPrevWritten(_addr, size) || !_notBusy() || !_writeEnable()) {
         return false;
       }
     #else
       if (_isChipPoweredDown() || !_addressCheck(_addr, size) || !_notBusy() || !_writeEnable()) {
         return false;
       }
     #endif
     return true;
     break;

     case ERASEFUNC:
     if(_isChipPoweredDown() || !_addressCheck(_addr, size) || !_notBusy() || !_writeEnable()) {
       return false;
     }
     return true;
     break;

     default:
       if (_isChipPoweredDown() || !_addressCheck(_addr, size) || !_notBusy()) {
         return false;
       }
     #ifdef ENABLEZERODMA
       _delay_us(3500L);
     #endif
     return true;
     break;
   }
 }

 // Transfer Address.
 bool SPIFlash::_transferAddress(void) {
   if (address4ByteEnabled) {
     _nextByte(WRITE, Highest(_currentAddress));
   }
   _nextByte(WRITE, Higher(_currentAddress));
   _nextByte(WRITE, Hi(_currentAddress));
   _nextByte(WRITE, Lo(_currentAddress));
   return true;
 }

 bool SPIFlash::_startSPIBus(void) {
   #ifndef SPI_HAS_TRANSACTION
       noInterrupts();
   #endif

   #if defined (ARDUINO_ARCH_SAM)
     due.SPIInit(DUE_SPI_CLK);
   #elif defined (ARDUINO_ARCH_SAMD)
     #ifdef SPI_HAS_TRANSACTION
       _spi->beginTransaction(_settings);
     #else
       _spi->setClockDivider(SPI_CLOCK_DIV_4);
       _spi->setDataMode(SPI_MODE0);
       _spi->setBitOrder(MSBFIRST);
     #endif
     #if defined ENABLEZERODMA
       dma_init();
     #endif
   #else
     #if defined (ARDUINO_ARCH_AVR)
       //save current SPI settings
       _SPCR = SPCR;
       _SPSR = SPSR;
     #endif
     #ifdef SPI_HAS_TRANSACTION
       SPI.beginTransaction(_settings);
     #else
       SPI.setClockDivider(SPI_CLOCK_DIV4);
       SPI.setDataMode(SPI_MODE0);
       SPI.setBitOrder(MSBFIRST);
     #endif
   #endif
   SPIBusState = true;
   return true;
 }

 //Initiates SPI operation - but data is not transferred yet. Always call _prep() before this function (especially when it involves writing or reading to/from an address)
 bool SPIFlash::_beginSPI(uint8_t opcode) {
   if (!SPIBusState) {
     _startSPIBus();
   }
   CHIP_SELECT
   switch (opcode) {
     case READDATA:
     _nextByte(WRITE, opcode);
     _transferAddress();
     break;

     case PAGEPROG:
     _nextByte(WRITE, opcode);
     _transferAddress();
     break;

     case FASTREAD:
     _nextByte(WRITE, opcode);
     _nextByte(WRITE, DUMMYBYTE);
     _transferAddress();
     break;

     case SECTORERASE:
     _nextByte(WRITE, opcode);
     _transferAddress();
     break;

     case BLOCK32ERASE:
     _nextByte(WRITE, opcode);
     _transferAddress();
     break;

     case BLOCK64ERASE:
     _nextByte(WRITE, opcode);
     _transferAddress();
     break;

     default:
     _nextByte(WRITE, opcode);
     break;
   }
   return true;
 }
 //SPI data lines are left open until _endSPI() is called

 //Reads/Writes next byte. Call 'n' times to read/write 'n' number of bytes. Should be called after _beginSPI()
 uint8_t SPIFlash::_nextByte(char IOType, uint8_t data) {
 #if defined (ARDUINO_ARCH_SAMD)
   #ifdef ENABLEZERODMA
     union {
       uint8_t dataBuf[1];
       uint8_t val;
     } rxData, txData;
     txData.val = data;
     spi_transfer(txData.dataBuf, rxData.dataBuf, 1);
     return rxData.val;
   #else
     return xfer(data);
   #endif
 #else
   return xfer(data);
 #endif
 }

 //Reads/Writes next int. Call 'n' times to read/write 'n' number of integers. Should be called after _beginSPI()
 uint16_t SPIFlash::_nextInt(uint16_t data) {
 #if defined (ARDUINO_ARCH_SAMD)
   return _spi->transfer16(data);
 #else
   return SPI.transfer16(data);
 #endif
 }

 //Reads/Writes next data buffer. Should be called after _beginSPI()
 void SPIFlash::_nextBuf(uint8_t opcode, uint8_t *data_buffer, uint32_t size) {
   switch (opcode) {
     case READDATA:
     #if defined (ARDUINO_ARCH_SAM)
       due.SPIRecByte(&(*data_buffer), size);
     #elif defined (ARDUINO_ARCH_SAMD)
       #ifdef ENABLEZERODMA
         spi_read(&(*data_buffer), size);
       #else
         _spi->transfer(&data_buffer[0], size);
       #endif
     #elif defined (ARDUINO_ARCH_AVR)
       SPI.transfer(&(*data_buffer), size);
     #else
       while(size--) {
         *data_buffer = xfer(NULLBYTE);
         data_buffer++;
       }
     #endif
     break;

     case PAGEPROG:
     #if defined (ARDUINO_ARCH_SAM)
       due.SPISendByte(&(*data_buffer), size);
     #elif defined (ARDUINO_ARCH_SAMD)
       #ifdef ENABLEZERODMA
         spi_write(&(*data_buffer), size);
       #else
         _spi->transfer(&data_buffer[0], size);
       #endif
     #elif defined (ARDUINO_ARCH_AVR)
       SPI.transfer(&(*data_buffer), size);
     #else
       while(size--) {
         xfer(*data_buffer);
         data_buffer++;
       }
     #endif
     break;
   }
 }

 //Stops all operations. Should be called after all the required data is read/written from repeated _nextByte() calls
 void SPIFlash::_endSPI(void) {
   CHIP_DESELECT

   if (address4ByteEnabled) {          // If the previous operation enabled 4-byte addressing, disable it
     _disable4ByteAddressing();
   }

 #ifdef SPI_HAS_TRANSACTION
   #if defined (ARDUINO_ARCH_SAMD)
     _spi->endTransaction();
   #else
     SPI.endTransaction();
   #endif
 #else
   interrupts();
 #endif
 #if defined (ARDUINO_ARCH_AVR)
   SPCR = _SPCR;
   SPSR = _SPSR;
 #endif
   SPIBusState = false;
 }

 // Checks if status register 1 can be accessed - used to check chip status, during powerdown and power up and for debugging
 uint8_t SPIFlash::_readStat1(void) {
   _beginSPI(READSTAT1);
   stat1 = _nextByte(READ);
   CHIP_DESELECT
   return stat1;
 }

 // Checks if status register 2 can be accessed, if yes, reads and returns it
 uint8_t SPIFlash::_readStat2(void) {
   _beginSPI(READSTAT2);
   stat2 = _nextByte(READ);
   //stat2 = _nextByte(READ);
   CHIP_DESELECT
   return stat2;
 }

 // Checks if status register 3 can be accessed, if yes, reads and returns it
 uint8_t SPIFlash::_readStat3(void) {
   _beginSPI(READSTAT3);
   stat3 = _nextByte(READ);
   //stat2 = _nextByte(READ);
   CHIP_DESELECT
   return stat3;
 }

 // Checks to see if 4-byte addressing is already enabled and if not, enables it
 bool SPIFlash::_enable4ByteAddressing(void) {
   if (_readStat3() & ADS) {
     return true;
   }
   _beginSPI(ADDR4BYTE_EN);
   CHIP_DESELECT
   if (_readStat3() & ADS) {
     address4ByteEnabled = true;
     return true;
   }
   else {
     _troubleshoot(UNABLETO4BYTE);
     return false;
   }
 }

 // Checks to see if 4-byte addressing is already disabled and if not, disables it
 bool SPIFlash::_disable4ByteAddressing(void) {
   if (!(_readStat3() & ADS)) {      // If 4 byte addressing is disabled (default state)
     return true;
   }
   _beginSPI(ADDR4BYTE_DIS);
   CHIP_DESELECT
   if (_readStat3() & ADS) {
     _troubleshoot(UNABLETO3BYTE);
     return false;
   }
   address4ByteEnabled = false;
   return true;
 }

 // Checks the erase/program suspend flag before enabling/disabling a program/erase suspend operation
 bool SPIFlash::_noSuspend(void) {
   switch (_chip.manufacturerID) {
     case WINBOND_MANID:
     if(_readStat2() & SUS) {
       _troubleshoot(SYSSUSPEND);
   		return false;
     }
   	return true;
     break;

     case MICROCHIP_MANID:
     _readStat1();
     if(stat1 & WSE || stat1 & WSP) {
       _troubleshoot(SYSSUSPEND);
   		return false;
     }
   }
   return true;
 }

 // Checks to see if chip is powered down. If it is, retrns true. If not, returns false.
 bool SPIFlash::_isChipPoweredDown(void) {
   if (chipPoweredDown) {
     _troubleshoot(CHIPISPOWEREDDOWN);
     return true;
   }
   else {
     return false;
   }
 }

 // Polls the status register 1 until busy flag is cleared or timeout
 bool SPIFlash::_notBusy(uint32_t timeout) {
   _delay_us(WINBOND_WRITE_DELAY);
   uint32_t _time = micros();

   do {
     _readStat1();
     if (!(stat1 & BUSY))
     {
       return true;
     }

   } while ((micros() - _time) < timeout);
   if (timeout <= (micros() - _time)) {
     _troubleshoot(CHIPBUSY);
     return false;
   }
   return true;
 }

 //Enables writing to chip by setting the WRITEENABLE bit
 bool SPIFlash::_writeEnable(bool _troubleshootEnable) {
   _beginSPI(WRITEENABLE);
   CHIP_DESELECT
   if (!(_readStat1() & WRTEN)) {
     if (_troubleshootEnable) {
       _troubleshoot(CANTENWRITE);
     }
     return false;
   }
   return true;
 }

 //Disables writing to chip by setting the Write Enable Latch (WEL) bit in the Status Register to 0
 //_writeDisable() is not required under the following conditions because the Write Enable Latch (WEL) flag is cleared to 0
 // i.e. to write disable state:
 // Power-up, Write Disable, Page Program, Quad Page Program, Sector Erase, Block Erase, Chip Erase, Write Status Register,
 // Erase Security Register and Program Security register
 bool SPIFlash::_writeDisable(void) {
 	_beginSPI(WRITEDISABLE);
   CHIP_DESELECT
 	return true;
 }

 //Checks the device ID to establish storage parameters
 bool SPIFlash::_getManId(uint8_t *b1, uint8_t *b2) {
   if(!_notBusy()) {
     return false;
   }
   _beginSPI(MANID);
   _nextByte(READ);
   _nextByte(READ);
   _nextByte(READ);
   *b1 = _nextByte(READ);
   *b2 = _nextByte(READ);
   CHIP_DESELECT
   return true;
 }

 //Checks for presence of chip by requesting JEDEC ID
 bool SPIFlash::_getJedecId(void) {
   if(!_notBusy()) {
     return false;
   }
   _beginSPI(JEDECID);
 	_chip.manufacturerID = _nextByte(READ);		// manufacturer id
 	_chip.memoryTypeID = _nextByte(READ);		// memory type
 	_chip.capacityID = _nextByte(READ);		// capacity
   CHIP_DESELECT
   if (!_chip.manufacturerID) {
     _troubleshoot(NORESPONSE);
     return false;
   }
   else {
     return true;
   }
 }

 bool SPIFlash::_disableGlobalBlockProtect(void) {
   if (_chip.memoryTypeID == SST25) {
     _readStat1();
     uint8_t _tempStat1 = stat1 & 0xC3;
     _beginSPI(WRITESTATEN);
     CHIP_DESELECT
     _beginSPI(WRITESTAT1);
     _nextByte(WRITE, _tempStat1);
     CHIP_DESELECT
   }
   else if (_chip.memoryTypeID == SST26) {
     if(!_notBusy()) {
     	return false;
     }
     _writeEnable();
     _delay_us(10);
     _beginSPI(ULBPR);
     CHIP_DESELECT
     _delay_us(50);
     _writeDisable();
   }
   return true;
 }

 //Identifies the chip
 bool SPIFlash::_chipID(uint32_t flashChipSize) {
   //set some default values
   kb4Erase.supported = kb32Erase.supported = kb64Erase.supported = chipErase.supported = true;
   kb4Erase.opcode = SECTORERASE;
   kb32Erase.opcode = BLOCK32ERASE;
   kb64Erase.opcode = BLOCK64ERASE;
   kb4Erase.time = BUSY_TIMEOUT;
   kb32Erase.time = kb4Erase.time * 8;
   kb64Erase.time = kb32Erase.time * 4;
   kb256Erase.supported = false;
   chipErase.opcode = CHIPERASE;
   chipErase.time = kb64Erase.time * 100L;

   _getJedecId();

   for (uint8_t i = 0; i < sizeof(_supportedManID); i++) {
     if (_chip.manufacturerID == _supportedManID[i]) {
       _chip.supportedMan = true;
       break;
     }
   }

   for (uint8_t i = 0; i < sizeof(_altChipEraseReq); i++) {
     if (_chip.memoryTypeID == _altChipEraseReq[i]) {
       chipErase.opcode = ALT_CHIPERASE;
       break;
     }
   }


     //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Begin SFDP ID section ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    #ifdef USES_SFDP
     if (_checkForSFDP()) {
       _getSFDPFlashParam();
     }
    #endif
     //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End SFDP ID section ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

   if (_chip.supportedMan) {
     #ifdef RUNDIAGNOSTIC
       Serial.println("No Chip size defined by user. Automated identification initiated.");
     #endif
     //Identify capacity
     for (uint8_t j = 0; j < sizeof(_capID); j++) {
       if (_chip.capacityID == _capID[j]) {
         _chip.capacity = (_memSize[j]);
         _chip.supported = true;
         #ifdef RUNDIAGNOSTIC
           Serial.println("Chip identified. This chip is fully supported by the library.");
         #endif
         return true;
       }
     }
   }
   else {
     _troubleshoot(UNKNOWNCHIP); //Error code for unidentified capacity
     return false;
   }

   if (!_chip.capacity) {

     if (flashChipSize) {
       // If a custom chip size is defined
       #ifdef RUNDIAGNOSTIC
       Serial.println("Custom Chipsize defined");
       #endif
       _chip.capacity = flashChipSize;
       _chip.supported = false;
       return true;
     }

     else {
       _troubleshoot(UNKNOWNCAP);
       return false;
     }

   }
   return false; //Failsafe
 }

 //Troubleshooting function. Called when #ifdef RUNDIAGNOSTIC is uncommented at the top of this file.
 void SPIFlash::_troubleshoot(uint8_t _code, bool printoverride) {
   diagnostics.troubleshoot(_code, printoverride);
 }
