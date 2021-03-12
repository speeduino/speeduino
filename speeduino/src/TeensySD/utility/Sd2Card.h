/* Arduino Sd2Card Library
 * Copyright (C) 2009 by William Greiman
 *
 * This file is part of the Arduino Sd2Card Library
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
 * You should have received a copy of the GNU General Public License
 * along with the Arduino Sd2Card Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#if defined(__MK64FX512__) || defined(__MK66FX1M0__) 
#ifndef Sd2Card_h
#define Sd2Card_h
/**
 * \file
 * Sd2Card class
 */
#include "Sd2PinMap.h"
#include "SdInfo.h"
/** Set SCK to max rate of F_CPU/2. See Sd2Card::setSckRate(). */
uint8_t const SPI_FULL_SPEED = 0;
/** Set SCK rate to F_CPU/4. See Sd2Card::setSckRate(). */
uint8_t const SPI_HALF_SPEED = 1;
/** Set SCK rate to F_CPU/8. Sd2Card::setSckRate(). */
uint8_t const SPI_QUARTER_SPEED = 2;
//------------------------------------------------------------------------------
// SPI pin definitions
//
// hardware pin defs
/**
 * SD Chip Select pin
 *
 * Warning if this pin is redefined the hardware SS will pin will be enabled
 * as an output by init().  An avr processor will not function as an SPI
 * master unless SS is set to output mode.
 */
/** The default chip select pin for the SD card is SS. */
uint8_t const  SD_CHIP_SELECT_PIN = SS_PIN;
// The following three pins must not be redefined for hardware SPI.
/** SPI Master Out Slave In pin */
uint8_t const  SPI_MOSI_PIN = MOSI_PIN;
/** SPI Master In Slave Out pin */
uint8_t const  SPI_MISO_PIN = MISO_PIN;
/** SPI Clock pin */
uint8_t const  SPI_SCK_PIN = SCK_PIN;
/** optimize loops for hardware SPI */
#define OPTIMIZE_HARDWARE_SPI

//------------------------------------------------------------------------------
/** Protect block zero from write if nonzero */
#define SD_PROTECT_BLOCK_ZERO 1
/** init timeout ms */
const unsigned int SD_INIT_TIMEOUT = 2000;
/** erase timeout ms */
const unsigned int SD_ERASE_TIMEOUT = 10000;
/** read timeout ms */
const unsigned int SD_READ_TIMEOUT = 300;
/** write time out ms */
const unsigned int SD_WRITE_TIMEOUT = 600;
//------------------------------------------------------------------------------
// card types
/** Standard capacity V1 SD card */
uint8_t const SD_CARD_TYPE_SD1 = 1;
/** Standard capacity V2 SD card */
uint8_t const SD_CARD_TYPE_SD2 = 2;
/** High Capacity SD card */
uint8_t const SD_CARD_TYPE_SDHC = 3;
//------------------------------------------------------------------------------
#if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1052__) || defined(__IMXRT1062__)
	#include "NXP_SDHC.h"
	#define BUILTIN_SDCARD 254
#endif
//------------------------------------------------------------------------------
/**
 * \class Sd2Card
 * \brief Raw access to SD and SDHC flash memory cards.
 */
class Sd2Card {
 public:
  /** Construct an instance of Sd2Card. */
  Sd2Card(void) : type_(0) {}
  /* Initialize an SD flash memory card with the selected SPI clock rate
   * and the SD chip select pin.  */
  uint8_t init(uint8_t sckRateID, uint8_t chipSelectPin) {
    #if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1052__) || defined(__IMXRT1062__)
    if (chipSelectPin == BUILTIN_SDCARD) {
      chipSelectPin_ = BUILTIN_SDCARD;
      uint8_t ret = SDHC_CardInit();
      type_ = SDHC_CardGetType();
      return (ret == 0) ? true : false;
    }
    #endif
    return SD_init(sckRateID, chipSelectPin);
  }
  /* return the type of SD card detected during init() */
  uint8_t type(void) const {return type_;}
  /** Returns the current value, true or false, for partial block read. */
  uint8_t readBlock(uint32_t block, uint8_t* dst) {
    #if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1052__) || defined(__IMXRT1062__)
    if (chipSelectPin_ == BUILTIN_SDCARD) {
      return (SDHC_CardReadBlock(dst, block) == 0) ? true : false;
    }
    #endif
    return SD_readBlock(block, dst);
  }
  /** Return the card type: SD V1, SD V2 or SDHC */
  uint8_t writeBlock(uint32_t block, const uint8_t* src) {
    #if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1052__) || defined(__IMXRT1062__)
    if (chipSelectPin_ == BUILTIN_SDCARD) {
      return (SDHC_CardWriteBlock(src, block) == 0) ? true : false;
    }
    #endif
    return SD_writeBlock(block, src);
  }
 private:
  uint8_t chipSelectPin_;
  uint8_t status_;
  uint8_t type_;
  // private functions
  uint8_t SD_init(uint8_t sckRateID, uint8_t chipSelectPin);
  uint8_t SD_readBlock(uint32_t block, uint8_t* dst);
  uint8_t SD_writeBlock(uint32_t blockNumber, const uint8_t* src);
  uint8_t cardAcmd(uint8_t cmd, uint32_t arg) {
    cardCommand(CMD55, 0);
    return cardCommand(cmd, arg);
  }
  uint8_t cardCommand(uint8_t cmd, uint32_t arg);
  uint8_t sendWriteCommand(uint32_t blockNumber, uint32_t eraseCount);
  void chipSelectHigh(void);
  void chipSelectLow(void);
  uint8_t waitNotBusy(unsigned int timeoutMillis);
  uint8_t writeData(uint8_t token, const uint8_t* src);
  uint8_t waitStartBlock(void);
  uint8_t setSckRate(uint8_t sckRateID);
};
#endif  // Sd2Card_h
#endif