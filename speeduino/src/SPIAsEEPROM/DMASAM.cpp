/* Arduino SPIMemory Library v.2.6.0
 * Copyright (C) 2017 by Prajwal Bhattaram
 * Created by Prajwal Bhattaram - 30/09/2016
 * Modified by Prajwal Bhattaram - 14/04/2017
 * Original code from @manitou48 <https://github.com/manitou48/DUEZoo/blob/master/dmaspi.ino>
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
#if defined (ARDUINO_ARCH_SAM)
#include "SPIMemory.h"

// Constructor
//DMASAM::DMASAM(){}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//        Private functions used by Arduino Due DMA operations        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Disable DMA Controller
void DMASAM::_dmac_disable() {
  DMAC->DMAC_EN &= (~DMAC_EN_ENABLE);
}
// Enable DMA Controller.
void DMASAM::_dmac_enable() {
  DMAC->DMAC_EN = DMAC_EN_ENABLE;
}
// Disable DMA Channel
void DMASAM::_dmac_channel_disable(uint32_t ul_num) {
  DMAC->DMAC_CHDR = DMAC_CHDR_DIS0 << ul_num;
}
// Enable DMA Channel
void DMASAM::_dmac_channel_enable(uint32_t ul_num) {
  DMAC->DMAC_CHER = DMAC_CHER_ENA0 << ul_num;
}
// Poll for transfer complete
bool DMASAM::_dmac_channel_transfer_done(uint32_t ul_num) {
  return (DMAC->DMAC_CHSR & (DMAC_CHSR_ENA0 << ul_num)) ? false : true;
}
// start RX DMA
void DMASAM::SPIDmaRX(uint8_t* dst, uint16_t count) {
  _dmac_channel_disable(SPI_DMAC_RX_CH);
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_SADDR = (uint32_t)&SPI0->SPI_RDR;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_DADDR = (uint32_t)dst;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_DSCR =  0;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_CTRLA = count |
    DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_CTRLB = DMAC_CTRLB_SRC_DSCR |
    DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_PER2MEM_DMA_FC |
    DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_CFG = DMAC_CFG_SRC_PER(SPI_RX_IDX) |
    DMAC_CFG_SRC_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ASAP_CFG;
  _dmac_channel_enable(SPI_DMAC_RX_CH);
}
void DMASAM::SPIDmaRX(char* dst, uint16_t count) {
  _dmac_channel_disable(SPI_DMAC_RX_CH);
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_SADDR = (uint32_t)&SPI0->SPI_RDR;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_DADDR = (uint32_t)dst;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_DSCR =  0;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_CTRLA = count |
    DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_CTRLB = DMAC_CTRLB_SRC_DSCR |
    DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_PER2MEM_DMA_FC |
    DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_CFG = DMAC_CFG_SRC_PER(SPI_RX_IDX) |
    DMAC_CFG_SRC_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ASAP_CFG;
  _dmac_channel_enable(SPI_DMAC_RX_CH);
}
// start TX DMA
void DMASAM::SPIDmaTX(const uint8_t* src, uint16_t count) {
  static uint8_t ff = 0XFF;
  uint32_t src_incr = DMAC_CTRLB_SRC_INCR_INCREMENTING;
  if (!src) {
    src = &ff;
    src_incr = DMAC_CTRLB_SRC_INCR_FIXED;
  }
  _dmac_channel_disable(SPI_DMAC_TX_CH);
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_SADDR = (uint32_t)src;
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_DADDR = (uint32_t)&SPI0->SPI_TDR;
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_DSCR =  0;
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_CTRLA = count |
    DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;

  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_CTRLB =  DMAC_CTRLB_SRC_DSCR |
    DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_MEM2PER_DMA_FC |
    src_incr | DMAC_CTRLB_DST_INCR_FIXED;

  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_CFG = DMAC_CFG_DST_PER(SPI_TX_IDX) |
      DMAC_CFG_DST_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;

  _dmac_channel_enable(SPI_DMAC_TX_CH);
}

void DMASAM::SPIDmaCharTX(const char* src, uint16_t count) {
  static char ff = 0XFF;
  uint32_t src_incr = DMAC_CTRLB_SRC_INCR_INCREMENTING;
  if (!src) {
    src = &ff;
    src_incr = DMAC_CTRLB_SRC_INCR_FIXED;
  }
  _dmac_channel_disable(SPI_DMAC_TX_CH);
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_SADDR = (uint32_t)src;
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_DADDR = (uint32_t)&SPI0->SPI_TDR;
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_DSCR =  0;
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_CTRLA = count |
    DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;

  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_CTRLB =  DMAC_CTRLB_SRC_DSCR |
    DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_MEM2PER_DMA_FC |
    src_incr | DMAC_CTRLB_DST_INCR_FIXED;

  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_CFG = DMAC_CFG_DST_PER(SPI_TX_IDX) |
      DMAC_CFG_DST_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;

  _dmac_channel_enable(SPI_DMAC_TX_CH);
}

void DMASAM::SPIBegin() {
  PIO_Configure(
      g_APinDescription[PIN_SPI_MOSI].pPort,
      g_APinDescription[PIN_SPI_MOSI].ulPinType,
      g_APinDescription[PIN_SPI_MOSI].ulPin,
      g_APinDescription[PIN_SPI_MOSI].ulPinConfiguration);
  PIO_Configure(
      g_APinDescription[PIN_SPI_MISO].pPort,
      g_APinDescription[PIN_SPI_MISO].ulPinType,
      g_APinDescription[PIN_SPI_MISO].ulPin,
      g_APinDescription[PIN_SPI_MISO].ulPinConfiguration);
  PIO_Configure(
      g_APinDescription[PIN_SPI_SCK].pPort,
      g_APinDescription[PIN_SPI_SCK].ulPinType,
      g_APinDescription[PIN_SPI_SCK].ulPin,
      g_APinDescription[PIN_SPI_SCK].ulPinConfiguration);
  pmc_enable_periph_clk(ID_SPI0);
#if USE_SAM3X_DMAC
  pmc_enable_periph_clk(ID_DMAC);
  _dmac_disable();
  DMAC->DMAC_GCFG = DMAC_GCFG_ARB_CFG_FIXED;
  _dmac_enable();
#if USE_SAM3X_BUS_MATRIX_FIX
  MATRIX->MATRIX_WPMR = 0x4d415400;
  MATRIX->MATRIX_MCFG[1] = 1;
  MATRIX->MATRIX_MCFG[2] = 1;
  MATRIX->MATRIX_SCFG[0] = 0x01000010;
  MATRIX->MATRIX_SCFG[1] = 0x01000010;
  MATRIX->MATRIX_SCFG[7] = 0x01000010;
#endif  // USE_SAM3X_BUS_MATRIX_FIX
#endif  // USE_SAM3X_DMAC
}
//  initialize SPI controller
void DMASAM::SPIInit(uint8_t dueSckDivisor) {
#if ENABLE_SPI_TRANSACTIONS
  SPI.beginTransaction(SPISettings());
#endif  // ENABLE_SPI_TRANSACTIONS
  uint8_t scbr = dueSckDivisor;
  Spi* pSpi = SPI0;
  //  disable SPI
  pSpi->SPI_CR = SPI_CR_SPIDIS;
  // reset SPI
  pSpi->SPI_CR = SPI_CR_SWRST;
  // no mode fault detection, set master mode
  pSpi->SPI_MR = SPI_PCS(SPI_CHIP_SEL) | SPI_MR_MODFDIS | SPI_MR_MSTR;
  // mode 0, 8-bit,
  pSpi->SPI_CSR[SPI_CHIP_SEL] = SPI_CSR_SCBR(scbr) | SPI_CSR_NCPHA;
  // enable SPI
  pSpi->SPI_CR |= SPI_CR_SPIEN;
}
uint8_t DMASAM::SPITransfer(uint8_t b) {
  Spi* pSpi = SPI0;

  pSpi->SPI_TDR = b;
  while ((pSpi->SPI_SR & SPI_SR_RDRF) == 0) {}
  b = pSpi->SPI_RDR;
  return b;
}
// SPI receive a byte
uint8_t DMASAM::SPIRecByte() {
  return SPITransfer(0XFF);
}
// SPI receive multiple bytes
uint8_t DMASAM::SPIRecByte(uint8_t* buf, size_t len) {
  Spi* pSpi = SPI0;
  int rtn = 0;
#if USE_SAM3X_DMAC
  // clear overrun error
  uint32_t s = pSpi->SPI_SR;

  SPIDmaRX(buf, len);
  SPIDmaTX(0, len);

  uint32_t m = millis();
  while (!_dmac_channel_transfer_done(SPI_DMAC_RX_CH)) {
    if ((millis() - m) > SAM3X_DMA_TIMEOUT)  {
      _dmac_channel_disable(SPI_DMAC_RX_CH);
      _dmac_channel_disable(SPI_DMAC_TX_CH);
      rtn = 2;
      break;
    }
  }
  if (pSpi->SPI_SR & SPI_SR_OVRES) rtn |= 1;
#else  // USE_SAM3X_DMAC
  for (size_t i = 0; i < len; i++) {
    pSpi->SPI_TDR = 0XFF;
    while ((pSpi->SPI_SR & SPI_SR_RDRF) == 0) {}
    buf[i] = pSpi->SPI_RDR;
  }
#endif  // USE_SAM3X_DMAC
  return rtn;
}
// SPI receive a char
int8_t DMASAM::SPIRecChar() {
  return SPITransfer(0XFF);
}
// SPI receive multiple chars
int8_t DMASAM::SPIRecChar(char* buf, size_t len) {
  Spi* pSpi = SPI0;
  char rtn = 0;
#if USE_SAM3X_DMAC
  // clear overrun error
  uint32_t s = pSpi->SPI_SR;

  SPIDmaRX(buf, len);
  SPIDmaTX(0, len);

  uint32_t m = millis();
  while (!_dmac_channel_transfer_done(SPI_DMAC_RX_CH)) {
    if ((millis() - m) > SAM3X_DMA_TIMEOUT)  {
      _dmac_channel_disable(SPI_DMAC_RX_CH);
      _dmac_channel_disable(SPI_DMAC_TX_CH);
      rtn = 2;
      break;
    }
  }
  if (pSpi->SPI_SR & SPI_SR_OVRES) rtn |= 1;
#else  // USE_SAM3X_DMAC
  for (size_t i = 0; i < len; i++) {
    pSpi->SPI_TDR = 0XFF;
    while ((pSpi->SPI_SR & SPI_SR_RDRF) == 0) {}
    buf[i] = pSpi->SPI_RDR;
  }
#endif  // USE_SAM3X_DMAC
  return rtn;
}
// SPI send a byte
void DMASAM::SPISendByte(uint8_t b) {
  SPITransfer(b);
}

void DMASAM::SPISendByte(const uint8_t* buf, size_t len) {
  Spi* pSpi = SPI0;
#if USE_SAM3X_DMAC
  SPIDmaTX(buf, len);
  while (!_dmac_channel_transfer_done(SPI_DMAC_TX_CH)) {}
#else  // #if USE_SAM3X_DMAC
  while ((pSpi->SPI_SR & SPI_SR_TXEMPTY) == 0) {}
  for (size_t i = 0; i < len; i++) {
    pSpi->SPI_TDR = buf[i];
    while ((pSpi->SPI_SR & SPI_SR_TDRE) == 0) {}
  }
#endif  // #if USE_SAM3X_DMAC
  while ((pSpi->SPI_SR & SPI_SR_TXEMPTY) == 0) {}
  // leave RDR empty
  uint8_t b = pSpi->SPI_RDR;
}
// SPI send a char
void DMASAM::SPISendChar(char b) {
  SPITransfer(b);
}
//SPI send multiple chars
void DMASAM::SPISendChar(const char* buf, size_t len) {
  Spi* pSpi = SPI0;
#if USE_SAM3X_DMAC
  SPIDmaCharTX(buf, len);
  while (!_dmac_channel_transfer_done(SPI_DMAC_TX_CH)) {}
#else  // #if USE_SAM3X_DMAC
  while ((pSpi->SPI_SR & SPI_SR_TXEMPTY) == 0) {}
  for (size_t i = 0; i < len; i++) {
    pSpi->SPI_TDR = buf[i];
    while ((pSpi->SPI_SR & SPI_SR_TDRE) == 0) {}
  }
#endif  // #if USE_SAM3X_DMAC
  while ((pSpi->SPI_SR & SPI_SR_TXEMPTY) == 0) {}
  // leave RDR empty
  char b = pSpi->SPI_RDR;
}

DMASAM due; // default instantiation of DMASAM object

#endif
