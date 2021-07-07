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

#ifndef SPIFLASH_H
#define SPIFLASH_H

#include "SPIMemory.h"

class SPIFlash {
public:
  //------------------------------------ Constructor ------------------------------------//
  //New Constructor to Accept the PinNames as a Chip select Parameter - @boseji <salearj@hotmail.com> 02.03.17
  #if defined (ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32)
  SPIFlash(uint8_t cs = CS, SPIClass *spiinterface=&SPI);
  #elif defined (BOARD_RTL8195A)
  SPIFlash(PinName cs = CS);
  #else
  SPIFlash(uint8_t cs = CS);
  #endif
  //----------------------------- Initial / Chip Functions ------------------------------//
  bool     begin(uint32_t flashChipSize = 0);
  void     setClock(uint32_t clockSpeed);
  bool     libver(uint8_t *b1, uint8_t *b2, uint8_t *b3);
  bool     sfdpPresent(void);
  uint8_t  error(bool verbosity = false);
  uint16_t getManID(void);
  uint32_t getJEDECID(void);
  uint64_t getUniqueID(void);
  uint32_t getAddress(uint16_t size);
  uint16_t sizeofStr(String &inputStr);
  uint32_t getCapacity(void);
  uint32_t getMaxPage(void);
  float    functionRunTime(void);
  //-------------------------------- Write / Read Bytes ---------------------------------//
  bool     writeByte(uint32_t _addr, uint8_t data, bool errorCheck = true);
  uint8_t  readByte(uint32_t _addr, bool fastRead = false);
  //----------------------------- Write / Read Byte Arrays ------------------------------//
  bool     writeByteArray(uint32_t _addr, uint8_t *data_buffer, size_t bufferSize, bool errorCheck = true);
  bool     readByteArray(uint32_t _addr, uint8_t *data_buffer, size_t bufferSize, bool fastRead = false);
  //-------------------------------- Write / Read Chars ---------------------------------//
  bool     writeChar(uint32_t _addr, int8_t data, bool errorCheck = true);
  int8_t   readChar(uint32_t _addr, bool fastRead = false);
  //------------------------------ Write / Read Char Arrays -----------------------------//
  bool     writeCharArray(uint32_t _addr, char *data_buffer, size_t bufferSize, bool errorCheck = true);
  bool     readCharArray(uint32_t _addr, char *data_buffer, size_t buffer_size, bool fastRead = false);
  //-------------------------------- Write / Read Shorts --------------------------------//
  bool     writeShort(uint32_t _addr, int16_t data, bool errorCheck = true);
  int16_t  readShort(uint32_t _addr, bool fastRead = false);
  //-------------------------------- Write / Read Words ---------------------------------//
  bool     writeWord(uint32_t _addr, uint16_t data, bool errorCheck = true);
  uint16_t readWord(uint32_t _addr, bool fastRead = false);
  //-------------------------------- Write / Read Longs ---------------------------------//
  bool     writeLong(uint32_t _addr, int32_t data, bool errorCheck = true);
  int32_t  readLong(uint32_t _addr, bool fastRead = false);
  //--------------------------- Write / Read Unsigned Longs -----------------------------//
  bool     writeULong(uint32_t _addr, uint32_t data, bool errorCheck = true);
  uint32_t readULong(uint32_t _addr, bool fastRead = false);
  //-------------------------------- Write / Read Floats --------------------------------//
  bool     writeFloat(uint32_t _addr, float data, bool errorCheck = true);
  float    readFloat(uint32_t _addr, bool fastRead = false);
  //-------------------------------- Write / Read Strings -------------------------------//
  bool     writeStr(uint32_t _addr, String &data, bool errorCheck = true);
  bool     readStr(uint32_t _addr, String &data, bool fastRead = false);
  //------------------------------- Write / Read Anything -------------------------------//

  template <class T> bool writeAnything(uint32_t _addr, const T& data, bool errorCheck = true);
  template <class T> bool readAnything(uint32_t _addr, T& data, bool fastRead = false);
  //-------------------------------- Erase functions ------------------------------------//
  bool     eraseSection(uint32_t _addr, uint32_t _sz);
  bool     eraseSector(uint32_t _addr);
  bool     eraseBlock32K(uint32_t _addr);
  bool     eraseBlock64K(uint32_t _addr);
  bool     eraseChip(void);
  //-------------------------------- Power functions ------------------------------------//
  bool     suspendProg(void);
  bool     resumeProg(void);
  bool     powerDown(void);
  bool     powerUp(void);
  //-------------------------- Public Arduino Due Functions -----------------------------//
//#if defined (ARDUINO_ARCH_SAM)
  //uint32_t freeRAM(void);
//#endif
  //------------------------------- Public variables ------------------------------------//

private:
  //------------------------------- Private functions -----------------------------------//
  unsigned _createMask(unsigned a, unsigned b);
  void     _troubleshoot(uint8_t _code, bool printoverride = false);
  void     _endSPI(void);
  bool     _disableGlobalBlockProtect(void);
  bool     _isChipPoweredDown(void);
  bool     _prep(uint8_t opcode, uint32_t _addr, uint32_t size = 0);
  bool     _startSPIBus(void);
  bool     _beginSPI(uint8_t opcode);
  bool     _noSuspend(void);
  bool     _notBusy(uint32_t timeout = BUSY_TIMEOUT);
  bool     _notPrevWritten(uint32_t _addr, uint32_t size = 1);
  bool     _writeEnable(bool _troubleshootEnable = true);
  bool     _writeDisable(void);
  bool     _getJedecId(void);
  bool     _getManId(uint8_t *b1, uint8_t *b2);
  bool     _chipID(uint32_t flashChipSize = 0);
  bool     _transferAddress(void);
  bool     _addressCheck(uint32_t _addr, uint32_t size = 1);
  bool     _enable4ByteAddressing(void);
  bool     _disable4ByteAddressing(void);
  uint8_t  _nextByte(char IOType, uint8_t data = NULLBYTE);
  uint16_t _nextInt(uint16_t = NULLINT);
  void     _nextBuf(uint8_t opcode, uint8_t *data_buffer, uint32_t size);
  uint8_t  _readStat1(void);
  uint8_t  _readStat2(void);
  uint8_t  _readStat3(void);
  bool     _getSFDPTable(uint32_t _tableAddress, uint8_t *data_buffer, uint8_t numberOfDWords);
  bool     _getSFDPData(uint32_t _address, uint8_t *data_buffer, uint8_t numberOfBytes);
  uint32_t _getSFDPdword(uint32_t _tableAddress, uint8_t dWordNumber);
  uint16_t _getSFDPint(uint32_t _tableAddress, uint8_t dWordNumber, uint8_t startByte);
  uint8_t  _getSFDPbyte(uint32_t _tableAddress, uint8_t dWordNumber, uint8_t byteNumber);
  bool     _getSFDPbit(uint32_t _tableAddress, uint8_t dWordNumber, uint8_t bitNumber);
  uint32_t _getSFDPTableAddr(uint32_t paramHeaderNum);
  uint32_t _calcSFDPEraseTimeUnits(uint8_t _unitBits);
  bool     _checkForSFDP(void);
  void     _getSFDPEraseParam(void);
  void     _getSFDPProgramTimeParam(void);
  bool     _getSFDPFlashParam(void);
  template <class T> bool _write(uint32_t _addr, const T& value, uint32_t _sz, bool errorCheck, uint8_t _dataType);
  template <class T> bool _read(uint32_t _addr, T& value, uint32_t _sz, bool fastRead = false, uint8_t _dataType = 0x00);
  //template <class T> bool _writeErrorCheck(uint32_t _addr, const T& value);
  template <class T> bool _writeErrorCheck(uint32_t _addr, const T& value, uint32_t _sz, uint8_t _dataType = 0x00);
  //-------------------------------- Private variables ----------------------------------//
  #ifdef SPI_HAS_TRANSACTION
    SPISettings _settings;
  #endif
  //If multiple SPI ports are available this variable is used to choose between them (SPI, SPI1, SPI2 etc.)
  SPIClass *_spi;
  #if !defined (BOARD_RTL8195A)
  uint8_t     csPin;
  #else
  // Object declaration for the GPIO HAL type for csPin - @boseji <salearj@hotmail.com> 02.03.17
  gpio_t      csPin;
  #endif
  volatile uint8_t *cs_port;
  bool        pageOverflow, SPIBusState;
  bool        chipPoweredDown = false;
  bool        address4ByteEnabled = false;
  bool        _loopedOver = false;
  uint8_t     cs_mask, errorcode, stat1, stat2, stat3, _SPCR, _SPSR, _a0, _a1, _a2;
  char READ = 'R';
  char WRITE = 'W';
  float _spifuncruntime = 0;
  struct      chipID {
                bool supported;
                bool supportedMan;
                bool sfdpAvailable;
                uint8_t manufacturerID;
                uint8_t memoryTypeID;
                uint8_t capacityID;
                uint32_t capacity;
                uint32_t eraseTime;
              };
              chipID _chip;
  struct      eraseParam{
              bool supported;
              uint8_t opcode;
              uint32_t time;
            } kb4Erase, kb32Erase, kb64Erase, kb256Erase, chipErase;
  uint8_t     _noOfParamHeaders, _noOfBasicParamDwords;
  uint16_t    _eraseTimeMultiplier, _prgmTimeMultiplier, _pageSize;
  uint32_t    currentAddress, _currentAddress = 0;
  uint32_t    _addressOverflow = false;
  uint32_t    _BasicParamTableAddr, _SectorMapParamTableAddr, _byteFirstPrgmTime, _byteAddnlPrgmTime, _pagePrgmTime;
  uint8_t     _uniqueID[8];
  const uint8_t _capID[16]   =
  {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x43, 0x4B, 0x00, 0x01, 0x13, 0x37};

  const uint32_t _memSize[16]  =
  {KB(64), KB(128), KB(256), KB(512), MB(1), MB(2), MB(4), MB(8), MB(16), MB(32), MB(8), MB(8), KB(256), KB(512), MB(4), KB(512)};
  // To understand the _memSize definitions check defines.h

  const uint8_t _supportedManID[7] = {WINBOND_MANID, MICROCHIP_MANID, CYPRESS_MANID, ADESTO_MANID, MICRON_MANID, ON_MANID, AMIC_MANID};

  const uint8_t _altChipEraseReq[3] = {A25L512, M25P40, SST26};
};

//--------------------------------- Public Templates ------------------------------------//

// Writes any type of data to a specific location in the flash memory.
// Takes three arguments -
//  1. _addr --> Any address from 0 to maxAddress
//  2. T& value --> Variable to write
//  3. errorCheck --> Turned on by default. Checks for writing errors
// WARNING: You can only write to previously erased memory locations (see datasheet).
//      Use the eraseSector()/eraseBlock32K/eraseBlock64K commands to first clear memory (write 0xFFs)
template <class T> bool SPIFlash::writeAnything(uint32_t _addr, const T& data, bool errorCheck) {
  return _write(_addr, data, sizeof(data), errorCheck, _STRUCT_);
}

// Reads any type of data from a specific location in the flash memory.
// Takes three arguments -
//  1. _addr --> Any address from 0 to maxAddress
//  2. T& value --> Variable to return data into
//  3. fastRead --> defaults to false - executes _beginFastRead() if set to true
template <class T> bool SPIFlash::readAnything(uint32_t _addr, T& data, bool fastRead) {
  return _read(_addr, data, sizeof(data), fastRead);
}

//---------------------------------- Private Templates ----------------------------------//

template <class T> bool SPIFlash::_writeErrorCheck(uint32_t _addr, const T& value, uint32_t _sz, uint8_t _dataType) {
  if (_isChipPoweredDown() || !_addressCheck(_addr, _sz) || !_notBusy()) {
    return false;
  }
  const uint8_t* p = (const uint8_t*)(const void*)&value;
  /*if (_dataType == _STRUCT_) {
    uint8_t _inByte[_sz];
    _beginSPI(READDATA);
    _nextBuf(READDATA, &(*_inByte), _sz);
    _endSPI();
    for (uint16_t i = 0; i < _sz; i++) {
      if (*p++ != _inByte[i]) {
        _troubleshoot(0x0A); //0x0A is ERRORCHKFAIL
        return false;
      }
      else {
        return true;
      }
    }
  }
  else {*/
    CHIP_SELECT
    _nextByte(WRITE, READDATA);
    _transferAddress();
    for (uint16_t i = 0; i < _sz; i++) {
      if (*p++ != _nextByte(READ)) {
        _troubleshoot(0x0A); //0x0A is ERRORCHKFAIL
        _endSPI();
        return false;
      }
    }
    _endSPI();
  //}
  return true;
}

// Writes any type of data to a specific location in the flash memory.
// Takes four arguments -
//  1. _addr --> Any address from 0 to maxAddress
//  2. T& value --> Variable to write
//  3. _sz --> Size of variable in bytes (1 byte = 8 bits)
//  4. errorCheck --> Turned on by default. Checks for writing errors
// WARNING: You can only write to previously erased memory locations (see datasheet).
//      Use the eraseSector()/eraseBlock32K/eraseBlock64K commands to first clear memory (write 0xFFs)

template <class T> bool SPIFlash::_write(uint32_t _addr, const T& value, uint32_t _sz, bool errorCheck, uint8_t _dataType) {
  bool _retVal;
#ifdef RUNDIAGNOSTIC
  _spifuncruntime = micros();
#endif

  uint32_t _addrIn = _addr;
  if (!_prep(PAGEPROG, _addrIn, _sz)) {
    return false;
  }
  _addrIn = _currentAddress;
  //Serial.print("_addrIn: ");
  //Serial.println(_addrIn, HEX);
  const uint8_t* p = ((const uint8_t*)(const void*)&value);
  //Serial.print(F("Address being written to: "));
  //Serial.println(_addr);
  uint32_t length = _sz;
  uint16_t maxBytes = SPI_PAGESIZE-(_addrIn % SPI_PAGESIZE);  // Force the first set of bytes to stay within the first page

  if (!SPIBusState) {
    _startSPIBus();
  }
  CHIP_SELECT
  _nextByte(WRITE, PAGEPROG);
  _transferAddress();

  if (maxBytes > length) {
    for (uint16_t i = 0; i < length; ++i) {
      _nextByte(WRITE, *p++);
    }
    CHIP_DESELECT
  }
  else {
    uint32_t writeBufSz;
    uint16_t data_offset = 0;

    do {
      writeBufSz = (length<=maxBytes) ? length : maxBytes;

      for (uint16_t i = 0; i < writeBufSz; ++i) {
        _nextByte(WRITE, *p++);
      }
      CHIP_DESELECT
      if (!_addressOverflow) {
        _currentAddress += writeBufSz;
      }
      else {
        if (data_offset >= _addressOverflow) {
          _currentAddress = 0x00;
          _addressOverflow = false;
        }
      }
      data_offset += writeBufSz;
      length -= writeBufSz;
      maxBytes = SPI_PAGESIZE;   // Now we can do up to 256 bytes per loop
      if(!_notBusy() || !_writeEnable()) {
        return false;
      }
    } while (length > 0);
  }

  if (!errorCheck) {
    _endSPI();
    return true;
  }
  else {
    //Serial.print(F("Address sent to error check: "));
    //Serial.println(_addr);
    _retVal =  _writeErrorCheck(_addr, value, _sz, _dataType);
  }
#ifdef RUNDIAGNOSTIC
  _spifuncruntime = micros() - _spifuncruntime;
#endif
  return _retVal;
}

// Reads any type of data from a specific location in the flash memory.
// Takes four arguments -
//  1. _addr --> Any address from 0 to maxAddress
//  2. T& value --> Variable to return data into
//  3. _sz --> Size of the variable in bytes (1 byte = 8 bits)
//  4. fastRead --> defaults to false - executes _beginFastRead() if set to true
template <class T> bool SPIFlash::_read(uint32_t _addr, T& value, uint32_t _sz, bool fastRead, uint8_t _dataType) {
  #ifdef RUNDIAGNOSTIC
    _spifuncruntime = micros();
  #endif
  if (!_prep(READDATA, _addr, _sz)) {
    return false;
  }
  else {
    uint8_t* p = (uint8_t*)(void*)&value;

    if (_dataType == _STRING_) {
      char _inChar[_sz];
      _beginSPI(READDATA);
      _nextBuf(READDATA, (uint8_t*) &(*_inChar), _sz);
      _endSPI();
      for (uint16_t i = 0; i < _sz; i++) {
        *p++ = _inChar[i];
      }
    }
    else {
      CHIP_SELECT
      if (fastRead) {
        _nextByte(WRITE, FASTREAD);
      }
      else {
        _nextByte(WRITE, READDATA);
      }
      _transferAddress();
      for (uint16_t i = 0; i < _sz; i++) {
        *p++ =_nextByte(READ);
      }
      _endSPI();
    }
  }
  #ifdef RUNDIAGNOSTIC
    _spifuncruntime = micros() - _spifuncruntime;
  #endif
  return true;
}

#endif // _SPIFLASH_H_
