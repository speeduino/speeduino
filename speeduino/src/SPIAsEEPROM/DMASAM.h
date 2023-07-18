/* Arduino SPIMemory Library v.3.2.0
 * Copyright (C) 2017 by Prajwal Bhattaram
 * Created by Prajwal Bhattaram - 19/04/2018
 * Modified by Prajwal Bhattaram - 20/04/2018
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

#ifndef SAM_DMASPI_H
#define SAM_DMASPI_H
#include "SPIMemory.h"
class DMASAM {
public:
  DMASAM(void){};
  ~DMASAM(void){};
  void     SPIDmaRX(uint8_t* dst, uint16_t count);
  void     SPIDmaRX(char* dst, uint16_t count);
  void     SPIDmaTX(const uint8_t* src, uint16_t count);
  void     SPIDmaCharTX(const char* src, uint16_t count);
  void     SPIBegin(void);
  void     SPIInit(uint8_t dueSckDivisor);
  uint8_t  SPITransfer(uint8_t b);
  uint8_t  SPIRecByte(void);
  uint8_t  SPIRecByte(uint8_t* buf, size_t len);
  int8_t   SPIRecChar(void);
  int8_t   SPIRecChar(char* buf, size_t len);
  void     SPISendByte(uint8_t b);
  void     SPISendByte(const uint8_t* buf, size_t len);
  void     SPISendChar(char b);
  void     SPISendChar(const char* buf, size_t len);
private:
  void     _dmac_disable(void);
  void     _dmac_enable(void);
  void     _dmac_channel_disable(uint32_t ul_num);
  void     _dmac_channel_enable(uint32_t ul_num);
  bool     _dmac_channel_transfer_done(uint32_t ul_num);
};

extern DMASAM due; ///< default DMASAM instance

#endif
