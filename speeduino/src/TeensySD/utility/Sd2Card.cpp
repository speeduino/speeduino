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
#include <Arduino.h>
#include <SPI.h>
#include "Sd2Card.h"

#ifdef SPI_HAS_TRANSACTION
static SPISettings settings;
#endif



#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
#define USE_TEENSY3_SPI

// Teensy 3.0 functions  (copied from sdfatlib20130629)
#include <kinetis.h>
// Limit initial fifo to three entries to avoid fifo overrun
#define SPI_INITIAL_FIFO_DEPTH 3
// define some symbols that are not in mk20dx128.h
#ifndef SPI_SR_RXCTR
#define SPI_SR_RXCTR 0XF0
#endif  // SPI_SR_RXCTR
#ifndef SPI_PUSHR_CONT
#define SPI_PUSHR_CONT 0X80000000
#endif   // SPI_PUSHR_CONT
#ifndef SPI_PUSHR_CTAS
#define SPI_PUSHR_CTAS(n) (((n) & 7) << 28)
#endif  // SPI_PUSHR_CTAS

static void spiBegin() {
  SIM_SCGC6 |= SIM_SCGC6_SPI0;
}

static void spiInit(uint8_t spiRate) {
  switch (spiRate) {
    // the top 2 speeds are set to 24 MHz, for the SD library defaults
    case 0:  settings = SPISettings(24000000, MSBFIRST, SPI_MODE0); break;
    case 1:  settings = SPISettings(24000000, MSBFIRST, SPI_MODE0); break;
    case 2:  settings = SPISettings(8000000, MSBFIRST, SPI_MODE0); break;
    case 3:  settings = SPISettings(4000000, MSBFIRST, SPI_MODE0); break;
    case 4:  settings = SPISettings(3000000, MSBFIRST, SPI_MODE0); break;
    case 5:  settings = SPISettings(2000000, MSBFIRST, SPI_MODE0); break;
    default: settings = SPISettings(400000, MSBFIRST, SPI_MODE0);
  }
  SPI.begin();
}

/** SPI receive a byte */
static  uint8_t spiRec() {
  SPI0_MCR |= SPI_MCR_CLR_RXF;
  SPI0_SR = SPI_SR_TCF;
  SPI0_PUSHR = 0xFF;
  while (!(SPI0_SR & SPI_SR_TCF)) {}
  return SPI0_POPR;
}
/** SPI receive multiple bytes */
static uint8_t spiRec(uint8_t* buf, size_t len) {
  // clear any data in RX FIFO
  SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);
  // use 16 bit frame to avoid TD delay between frames
  // get one byte if len is odd
  if (len & 1) {
    *buf++ = spiRec();
    len--;
  }
  // initial number of words to push into TX FIFO
  int nf = len/2 < SPI_INITIAL_FIFO_DEPTH ? len/2 : SPI_INITIAL_FIFO_DEPTH;
  for (int i = 0; i < nf; i++) {
    SPI0_PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
  }
  uint8_t* limit = buf + len - 2*nf;
  while (buf < limit) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    SPI0_PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
    uint16_t w = SPI0_POPR;
    *buf++ = w >> 8;
    *buf++ = w & 0XFF;
  }
  // limit for rest of RX data
  limit += 2*nf;
  while (buf < limit) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    uint16_t w = SPI0_POPR;
    *buf++ = w >> 8;
    *buf++ = w & 0XFF;
  }
  return 0;
}
static void spiRecIgnore(size_t len) {
  // clear any data in RX FIFO
  SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF | SPI_MCR_PCSIS(0x1F);
  // use 16 bit frame to avoid TD delay between frames
  // get one byte if len is odd
  if (len & 1) {
    spiRec();
    len--;
  }
  // initial number of words to push into TX FIFO
  int nf = len/2 < SPI_INITIAL_FIFO_DEPTH ? len/2 : SPI_INITIAL_FIFO_DEPTH;
  for (int i = 0; i < nf; i++) {
    SPI0_PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
    len -= 2;
  }
  //uint8_t* limit = buf + len - 2*nf;
  //while (buf < limit) {
  while (len > 0) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    SPI0_PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
    SPI0_POPR;
    len -= 2;
  }
  // limit for rest of RX data
  while (nf > 0) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    SPI0_POPR;
    nf--;
  }
}
/** SPI send a byte */
static void spiSend(uint8_t b) {
  SPI0_MCR |= SPI_MCR_CLR_RXF;
  SPI0_SR = SPI_SR_TCF;
  SPI0_PUSHR = b;
  while (!(SPI0_SR & SPI_SR_TCF)) {}
}
/** SPI send multiple bytes */

#elif defined(__IMXRT1052__)  || defined(__IMXRT1062__) || defined(__MKL26Z64__)
 #define USE_SPI_LIB
 
 static void spiInit(uint8_t spiRate) {
  switch (spiRate) {
#ifdef __MKL26Z64__
    // the top 2 speeds are set to 24 MHz, for the SD library defaults
    case 0:  settings = SPISettings(24000000, MSBFIRST, SPI_MODE0); break;
#else
    // the top 2 speeds are set to 24 MHz, for the SD library defaults
    case 0:  settings = SPISettings(25200000, MSBFIRST, SPI_MODE0); break;
#endif
    case 1:  settings = SPISettings(24000000, MSBFIRST, SPI_MODE0); break;
    case 2:  settings = SPISettings(8000000, MSBFIRST, SPI_MODE0); break;
    case 3:  settings = SPISettings(4000000, MSBFIRST, SPI_MODE0); break;
    case 4:  settings = SPISettings(3000000, MSBFIRST, SPI_MODE0); break;
    case 5:  settings = SPISettings(2000000, MSBFIRST, SPI_MODE0); break;
    default: settings = SPISettings(400000, MSBFIRST, SPI_MODE0);
  }
  SPI.begin();
} 

 static void spiSend(uint8_t b) {
	SPI.transfer(b);
 }
 
 static  uint8_t spiRec(void) {
	return SPI.transfer(0xff);
 }
 
 static void spiRec(uint8_t* buf, size_t len) {
	memset(buf, 0xFF, len);
	SPI.transfer(buf, len);	
 }
 
 static void spiRecIgnore(size_t len) {
	for (size_t i=0; i < len; i++) 
		SPI.transfer(0xff);
 }
 
//------------------------------------------------------------------------------
#else
// functions for hardware SPI
/** Send a byte to the card */
static void spiSend(uint8_t b) {
  SPDR = b;
  while (!(SPSR & (1 << SPIF)));
}
/** Receive a byte from the card */
static  uint8_t spiRec(void) {
  spiSend(0XFF);
  return SPDR;
}

#endif




//------------------------------------------------------------------------------
// send command and return error code.  Return zero for OK
uint8_t Sd2Card::cardCommand(uint8_t cmd, uint32_t arg)
{
  // wait up to 300 ms if busy
  waitNotBusy(300);

  // send command
  spiSend(cmd | 0x40);

  // send argument
  for (int8_t s = 24; s >= 0; s -= 8) spiSend(arg >> s);

  // send CRC
  uint8_t crc = 0XFF;
  if (cmd == CMD0) crc = 0X95;  // correct crc for CMD0 with arg 0
  if (cmd == CMD8) crc = 0X87;  // correct crc for CMD8 with arg 0X1AA
  spiSend(crc);

  // wait for response
  for (uint8_t i = 0; ((status_ = spiRec()) & 0X80) && i != 0XFF; i++);
  return status_;
}
//------------------------------------------------------------------------------
#ifdef SPI_HAS_TRANSACTION
static uint8_t chip_select_asserted = 0;
#endif
void Sd2Card::chipSelectHigh(void) {
  digitalWrite(chipSelectPin_, HIGH);
#ifdef SPI_HAS_TRANSACTION
  if (chip_select_asserted) {
    chip_select_asserted = 0;
    SPI.endTransaction();
  }
#endif
}
//------------------------------------------------------------------------------
void Sd2Card::chipSelectLow(void) {
#ifdef SPI_HAS_TRANSACTION
  if (!chip_select_asserted) {
    chip_select_asserted = 1;
    SPI.beginTransaction(settings);
  }
#endif
  digitalWrite(chipSelectPin_, LOW);
}
//------------------------------------------------------------------------------
/**
 * Initialize an SD flash memory card.
 *
 * \param[in] sckRateID SPI clock rate selector. See setSckRate().
 * \param[in] chipSelectPin SD chip select pin number.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t Sd2Card::SD_init(uint8_t sckRateID, uint8_t chipSelectPin) {
  type_ = 0;
  chipSelectPin_ = chipSelectPin;
  // 16-bit init start time allows over a minute
  unsigned int t0 = millis();
  uint32_t arg;

  digitalWrite(chipSelectPin_, HIGH);
  pinMode(chipSelectPin_, OUTPUT);
  digitalWrite(chipSelectPin_, HIGH);

#if defined(USE_TEENSY3_SPI)
  spiBegin();
  spiInit(6);
#elif defined(USE_SPI_LIB)
  spiInit(6);
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH); // disable any SPI device using hardware SS pin  
#else
  // set pin modes
  pinMode(SPI_MISO_PIN, INPUT);
  pinMode(SPI_MOSI_PIN, OUTPUT);
  pinMode(SPI_SCK_PIN, OUTPUT);
  // SS must be in output mode even it is not chip select
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH); // disable any SPI device using hardware SS pin
  // Enable SPI, Master, clock rate f_osc/128
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
  // clear double speed
  SPSR &= ~(1 << SPI2X);
#ifdef SPI_HAS_TRANSACTION
  settings = SPISettings(250000, MSBFIRST, SPI_MODE0);
#endif
#endif  // not USE_TEENSY3_SPI
  // must supply min of 74 clock cycles with CS high.
#ifdef SPI_HAS_TRANSACTION
  SPI.beginTransaction(settings);
#endif
  for (uint8_t i = 0; i < 10; i++) spiSend(0XFF);
#ifdef SPI_HAS_TRANSACTION
  SPI.endTransaction();
#endif
  chipSelectLow();
  // command to go idle in SPI mode
  while ((status_ = cardCommand(CMD0, 0)) != R1_IDLE_STATE) {
    unsigned int d = millis() - t0;
    if (d > SD_INIT_TIMEOUT) {
      goto fail; // SD_CARD_ERROR_CMD0
    }
  }
  // check SD version
  if ((cardCommand(CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) {
    type_ = SD_CARD_TYPE_SD1;
  } else {
    // only need last byte of r7 response
    for (uint8_t i = 0; i < 4; i++) status_ = spiRec();
    if (status_ != 0XAA) {
      goto fail; // SD_CARD_ERROR_CMD8
    }
    type_ = SD_CARD_TYPE_SD2;
  }
  // initialize card and send host supports SDHC if SD2
  arg = (type_ == SD_CARD_TYPE_SD2) ? 0X40000000 : 0;
  while ((status_ = cardAcmd(ACMD41, arg)) != R1_READY_STATE) {
    // check for timeout
    unsigned int d = millis() - t0;
    if (d > SD_INIT_TIMEOUT) {
      goto fail; // SD_CARD_ERROR_ACMD41
    }
  }
  // if SD2 read OCR register to check for SDHC card
  if (type_ == SD_CARD_TYPE_SD2) {
    if (cardCommand(CMD58, 0)) {
      goto fail; // SD_CARD_ERROR_CMD58
    }
    if ((spiRec() & 0XC0) == 0XC0) type_ = SD_CARD_TYPE_SDHC;
    // discard rest of ocr - contains allowed voltage range
    for (uint8_t i = 0; i < 3; i++) spiRec();
  }
  chipSelectHigh();
  return setSckRate(sckRateID);

fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/**
 * Read a 512 byte block from an SD card device.
 *
 * \param[in] block Logical block to be read.
 * \param[out] dst Pointer to the location that will receive the data.

 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t Sd2Card::SD_readBlock(uint32_t block, uint8_t* dst)
{
  // use address if not SDHC card
  if (type_ != SD_CARD_TYPE_SDHC) block <<= 9;
  chipSelectLow();
  if (cardCommand(CMD17, block)) {
    goto fail; // SD_CARD_ERROR_CMD17
  }
  if (!waitStartBlock()) {
    goto fail;
  }
#if defined(USE_TEENSY3_SPI) | defined(USE_SPI_LIB)
  spiRec(dst, 512);
  spiRecIgnore(2);
#else  // OPTIMIZE_HARDWARE_SPI
  // start first spi transfer
  SPDR = 0XFF;
  // transfer data
  for (uint16_t i = 0; i < 511; i++) {
    while (!(SPSR & (1 << SPIF)));
    dst[i] = SPDR;
    SPDR = 0XFF;
  }
  // wait for last byte
  while (!(SPSR & (1 << SPIF)));
  dst[511] = SPDR;
  // skip CRC bytes
  spiRec();
  spiRec();
#endif
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/**
 * Set the SPI clock rate.
 *
 * \param[in] sckRateID A value in the range [0, 6].
 *
 * 0 = 8 MHz
 * 1 = 4 MHz
 * 2 = 2 MHz
 * 3 = 1 MHz
 * 4 = 500 kHz
 * 5 = 125 kHz
 * 6 = 63 kHz
 *
 * The SPI clock will be set to F_CPU/pow(2, 1 + sckRateID). The maximum
 * SPI rate is F_CPU/2 for \a sckRateID = 0 and the minimum rate is F_CPU/128
 * for \a scsRateID = 6.
 *
 * \return The value one, true, is returned for success and the value zero,
 * false, is returned for an invalid value of \a sckRateID.
 */
uint8_t Sd2Card::setSckRate(uint8_t sckRateID) {
#if defined(USE_TEENSY3_SPI) || defined(USE_SPI_LIB)
  spiInit(sckRateID);
  return true;
#else
  if (sckRateID > 6) sckRateID = 6;
  // see avr processor datasheet for SPI register bit definitions
  if ((sckRateID & 1) || sckRateID == 6) {
    SPSR &= ~(1 << SPI2X);
  } else {
    SPSR |= (1 << SPI2X);
  }
  SPCR &= ~((1 <<SPR1) | (1 << SPR0));
  SPCR |= (sckRateID & 4 ? (1 << SPR1) : 0)
    | (sckRateID & 2 ? (1 << SPR0) : 0);
#ifdef SPI_HAS_TRANSACTION
  switch (sckRateID) {
    case 0:  settings = SPISettings(8000000, MSBFIRST, SPI_MODE0); break;
    case 1:  settings = SPISettings(4000000, MSBFIRST, SPI_MODE0); break;
    case 2:  settings = SPISettings(2000000, MSBFIRST, SPI_MODE0); break;
    case 3:  settings = SPISettings(1000000, MSBFIRST, SPI_MODE0); break;
    case 4:  settings = SPISettings(500000, MSBFIRST, SPI_MODE0); break;
    case 5:  settings = SPISettings(250000, MSBFIRST, SPI_MODE0); break;
    default: settings = SPISettings(125000, MSBFIRST, SPI_MODE0);
  }
#endif
  return true;
#endif
}
//------------------------------------------------------------------------------
// wait for card to go not busy
uint8_t Sd2Card::waitNotBusy(unsigned int timeoutMillis) {
  unsigned int t0 = millis();
  unsigned int d;
  do {
    if (spiRec() == 0XFF) return true;
    d = millis() - t0;
  }
  while (d < timeoutMillis);
  return false;
}
//------------------------------------------------------------------------------
/** Wait for start block token */
uint8_t Sd2Card::waitStartBlock(void) {
  unsigned int t0 = millis();
  while ((status_ = spiRec()) == 0XFF) {
    unsigned int d = millis() - t0;
    if (d > SD_READ_TIMEOUT) {
      return false; // SD_CARD_ERROR_READ_TIMEOUT
    }
  }
  if (status_ != DATA_START_BLOCK) {
    return false; // SD_CARD_ERROR_READ
  }
  return true;
}
//------------------------------------------------------------------------------
/**
 * Writes a 512 byte block to an SD card.
 *
 * \param[in] blockNumber Logical block to be written.
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t Sd2Card::SD_writeBlock(uint32_t blockNumber, const uint8_t* src) {
#if SD_PROTECT_BLOCK_ZERO
  // don't allow write to first block
  if (blockNumber == 0) {
    goto fail; // SD_CARD_ERROR_WRITE_BLOCK_ZERO
  }
#endif  // SD_PROTECT_BLOCK_ZERO

  // use address if not SDHC card
  if (type_ != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  chipSelectLow();
  if (cardCommand(CMD24, blockNumber)) {
    goto fail; // SD_CARD_ERROR_CMD24
  }
  if (!writeData(DATA_START_BLOCK, src)) goto fail;

  // wait for flash programming to complete
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) {
    goto fail; // SD_CARD_ERROR_WRITE_TIMEOUT
  }
  // response is r2 so get and check two bytes for nonzero
  if (cardCommand(CMD13, 0) || spiRec()) {
    goto fail; // SD_CARD_ERROR_WRITE_PROGRAMMING
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
// send one block of data for write block or write multiple blocks
uint8_t Sd2Card::writeData(uint8_t token, const uint8_t* src) {
#if defined(OPTIMIZE_HARDWARE_SPI) && !defined(USE_SPI_LIB)

  // send data - optimized loop
  SPDR = token;

  // send two byte per iteration
  for (uint16_t i = 0; i < 512; i += 2) {
    while (!(SPSR & (1 << SPIF)));
    SPDR = src[i];
    while (!(SPSR & (1 << SPIF)));
    SPDR = src[i+1];
  }

  // wait for last data byte
  while (!(SPSR & (1 << SPIF)));

#else  // OPTIMIZE_HARDWARE_SPI
  spiSend(token);
  for (uint16_t i = 0; i < 512; i++) {
    spiSend(src[i]); 
  }
#endif  // OPTIMIZE_HARDWARE_SPI
  spiSend(0xff);  // dummy crc
  spiSend(0xff);  // dummy crc

  status_ = spiRec();
  if ((status_ & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
    return false; // SD_CARD_ERROR_WRITE
  }
  return true;
}

#endif