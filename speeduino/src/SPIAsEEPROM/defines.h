/* Arduino SPIMemory Library v.3.2.1
 * Copyright (C) 2017 by Prajwal Bhattaram
 * Created by Prajwal Bhattaram - 19/05/2015
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

 // Defines and variables specific to SAM architecture
 #if defined (ARDUINO_ARCH_SAM)
   #define CHIP_SELECT   digitalWrite(csPin, LOW);
   #define CHIP_DESELECT digitalWrite(csPin, HIGH);
   #define xfer   due.SPITransfer
   #define BEGIN_SPI due.SPIBegin();
   extern char _end;
   extern "C" char *sbrk(int i);
   //char *ramstart=(char *)0x20070000;
   //char *ramend=(char *)0x20088000;

 // Specific access configuration for Chip select pin. Includes specific to RTL8195A to access GPIO HAL - @boseji <salearj@hotmail.com> 02.03.17
 #elif defined (BOARD_RTL8195A)
   #define CHIP_SELECT   gpio_write(&csPin, 0);
   #define CHIP_DESELECT gpio_write(&csPin, 1);
   #define xfer(n)   SPI.transfer(n)
   #define BEGIN_SPI SPI.begin();

 // Defines and variables specific to SAMD architecture
 #elif defined (ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32)
   #define CHIP_SELECT   digitalWrite(csPin, LOW);
   #define CHIP_DESELECT digitalWrite(csPin, HIGH);
   #define xfer(n)   _spi->transfer(n)
   #define BEGIN_SPI _spi->begin();
 #else
   #define CHIP_SELECT   digitalWrite(csPin, LOW);
   #define CHIP_DESELECT digitalWrite(csPin, HIGH);
   #define xfer(n)   SPI.transfer(n)
   #define BEGIN_SPI SPI.begin();
 #endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//						Common Instructions 						  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#define	MANID         0x90
#define PAGEPROG      0x02
#define READDATA      0x03
#define FASTREAD      0x0B
#define WRITEDISABLE  0x04
#define READSTAT1     0x05
#define READSTAT2     0x35
#define READSTAT3     0x15
#define WRITESTATEN   0x50
#define WRITESTAT1    0x01
#define WRITESTAT2    0x31
#define WRITESTAT3    0x11
#define WRITEENABLE   0x06
#define ADDR4BYTE_EN  0xB7
#define ADDR4BYTE_DIS 0xE9
#define SECTORERASE   0x20
#define BLOCK32ERASE  0x52
#define BLOCK64ERASE  0xD8
#define CHIPERASE     0x60
#define ALT_CHIPERASE 0xC7    // Some flash chips use a different chip erase command
#define SUSPEND       0x75
#define ID            0x90
#define RESUME        0x7A
#define JEDECID       0x9F
#define POWERDOWN     0xB9
#define RELEASE       0xAB
#define READSFDP      0x5A
#define UNIQUEID      0x4B

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                     General size definitions                       //
//            B = Bytes; KiB = Kilo Bytes; MiB = Mega Bytes           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define B(x)          uint32_t(x*BYTE)
#define KB(x)         uint32_t(x*KiB)
#define MB(x)         uint32_t(x*MiB)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//					SFDP related defines 						  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define DWORD(x) x
#define FIRSTBYTE 0x01
#define SFDPSIGNATURE 0x50444653
#define ADDRESSOFSFDPDWORD(x,y) x+((y - 1) * 4)
#define ADDRESSOFSFDPBYTE(x,y,z) x+((y - 1) * 4)+(z - 1)
#define KB4ERASE_TYPE 0x0C
#define KB32ERASE_TYPE 0x0F
#define KB64ERASE_TYPE 0x10
#define KB256ERASE_TYPE 0x12
#define MS1 0b00000000
#define MS16 0b00000001
#define MS128 0b00000010
#define S1    0b00000011

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//					Fixed SFDP addresses 						  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define SFDP_HEADER_ADDR 0x00
#define SFDP_SIGNATURE_DWORD 0x01
#define SFDP_NPH_DWORD 0x02
#define SFDP_NPH_BYTE 0x03
#define SFDP_PARAM_TABLE_LENGTH_DWORD 0x01
#define SFDP_PARAM_TABLE_LENGTH_BYTE 0x04
#define SFDP_BASIC_PARAM_TABLE_HDR_ADDR 0x08
#define SFDP_BASIC_PARAM_TABLE_NO 0x01
#define SFDP_MEMORY_DENSITY_DWORD 0x02
#define SFDP_SECTOR_MAP_PARAM_TABLE_NO 0x02
#define SFDP_ERASE1_BYTE 0x01
#define SFDP_ERASE1_INSTRUCTION_DWORD 0x08
#define SFDP_ERASE2_INSTRUCTION_DWORD 0x09
#define SFDP_SECTOR_ERASE_TIME_DWORD 0x0A
#define SFDP_CHIP_ERASE_TIME_DWORD 0x0B
#define SFDP_PROGRAM_TIME_DWORD 0x0B

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//					Chip specific instructions 						  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~~~~ Winbond ~~~~~~~~~~~~~~~~~~~~~~~~~//
  #define WINBOND_MANID         0xEF
  #define SPI_PAGESIZE          0x100
  #define WINBOND_WRITE_DELAY   0x02
  #define WINBOND_WREN_TIMEOUT  10L

//~~~~~~~~~~~~~~~~~~~~~~~~ Microchip ~~~~~~~~~~~~~~~~~~~~~~~~//
  #define MICROCHIP_MANID       0xBF
  #define SST25                 0x25
  #define SST26                 0x26
  #define ULBPR                 0x98    //Global Block Protection Unlock (Ref sections 4.1.1 & 5.37 of datasheet)

//~~~~~~~~~~~~~~~~~~~~~~~~ Cypress ~~~~~~~~~~~~~~~~~~~~~~~~//
  #define CYPRESS_MANID         0x01

//~~~~~~~~~~~~~~~~~~~~~~~~ Adesto ~~~~~~~~~~~~~~~~~~~~~~~~//
  #define ADESTO_MANID         0x1F

//~~~~~~~~~~~~~~~~~~~~~~~~ Micron ~~~~~~~~~~~~~~~~~~~~~~~~//
  #define MICRON_MANID         0x20
  #define M25P40               0x20

//~~~~~~~~~~~~~~~~~~~~~~~~ ON ~~~~~~~~~~~~~~~~~~~~~~~~//
  #define ON_MANID             0x62

//~~~~~~~~~~~~~~~~~~~~~~~~ AMIC ~~~~~~~~~~~~~~~~~~~~~~~~//
  #define AMIC_MANID           0x37
  #define A25L512              0x30
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//							Definitions 							  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#define BUSY          0x01
#if defined (ARDUINO_ARCH_ESP32)
#define SPI_CLK       20000000        //Hz equivalent of 20MHz
#else
#define SPI_CLK       104000000       //Hz equivalent of 104MHz
#endif
#define WRTEN         0x02
#define SUS           0x80
#define WSE           0x04
#define WSP           0x08
#define ADS           0x01            // Current Address mode in Status register 3
#define DUMMYBYTE     0xEE
#define NULLBYTE      0x00
#define NULLINT       0x0000
#define NO_CONTINUE   0x00
#define PASS          0x01
#define FAIL          0x00
#define NOOVERFLOW    false
#define NOERRCHK      false
#define VERBOSE       true
#define PRINTOVERRIDE true
#define ERASEFUNC     0xEF
#define BUSY_TIMEOUT  1000000000L
#define arrayLen(x)   (sizeof(x) / sizeof(*x))
#define lengthOf(x)   (sizeof(x))/sizeof(byte)
#define BYTE          1L
#define KiB           1024L
#define MiB           KiB * KiB
#define S             1000L
#define TIME_TO_PROGRAM(x) (_byteFirstPrgmTime + (_byteAddnlPrgmTime * (x - 1)) )

#if defined (ARDUINO_ARCH_ESP8266)
#define CS 15
#elif defined (ARDUINO_ARCH_SAMD)
#define CS 10
/*********************************************************************************************
// Declaration of the Default Chip select pin name for RTL8195A
// Note: This has been shifted due to a bug identified in the HAL layer SPI driver
// @ref http://www.amebaiot.com/en/questions/forum/facing-issues-with-spi-interface-to-w25q32/
// Note: Please use any pin other than GPIOC_0 which is the D10 marked in the kit
// Original edit by @boseji <salearj@hotmail.com> 02.03.17
// Modified by Prajwal Bhattaram <marzogh@icloud.com> 14.4.17
**********************************************************************************************/
#elif defined (BOARD_RTL8195A)
#define CS PC_4
#else
#define CS SS
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                    Arduino Due DMA definitions                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Use SAM3X DMAC if nonzero
#define USE_SAM3X_DMAC 1
// Use extra Bus Matrix arbitration fix if nonzero
#define USE_SAM3X_BUS_MATRIX_FIX 0
// Time in ms for DMA receive timeout
#define SAM3X_DMA_TIMEOUT 100
// chip select register number
#define SPI_CHIP_SEL 3
// DMAC receive channel
#define SPI_DMAC_RX_CH  1
// DMAC transmit channel
#define SPI_DMAC_TX_CH  0
// DMAC Channel HW Interface Number for SPI TX.
#define SPI_TX_IDX  1
// DMAC Channel HW Interface Number for SPI RX.
#define SPI_RX_IDX  2
// Set DUE SPI clock div (any integer from 2 - 255)
#define DUE_SPI_CLK 2

 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
 //     					   List of Supported data types						  //
 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

  #define _BYTE_              0x01
  #define _CHAR_              0x02
  #define _WORD_              0x03
  #define _SHORT_             0x04
  #define _ULONG_             0x05
  #define _LONG_              0x06
  #define _FLOAT_             0x07
  #define _STRING_            0x08
  #define _BYTEARRAY_         0x09
  #define _CHARARRAY_         0x0A
  #define _STRUCT_            0x0B

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                        Bit shift macros                            //
//                      Thanks to @VitorBoss                          //
//          https://github.com/Marzogh/SPIMemory/issues/76             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define Lo(param) ((char *)&param)[0] //0x000y
#define Hi(param) ((char *)&param)[1] //0x00y0
#define Higher(param) ((char *)&param)[2] //0x0y00
#define Highest(param) ((char *)&param)[3] //0xy000
#define Low(param) ((int *)&param)[0] //0x00yy
#define Top(param) ((int *)&param)[1] //0xyy00

// Set bit and clear bit
// x -> byte, y -> bit
#define setBit(x, y) x |= (1 << y)
#define clearBit(x, y) x &= ~(1 << y)
#define toggleBit(x, y) x ^= (1 << y)

// Query to see if bit is set or cleared.
// x -> byte, y -> bit
#define bitIsSet(x, y) x & (1 << y)
#define bitIsClear(x, y) !(x & (1 << y))

//Set nibbles
// x -> byte, y -> value to set
#define setLowerNibble(x, y) x &= 0xF0; x |= (y & 0x0F) // Clear out the lower nibble // OR in the desired mask
#define setUpperNibble(x, y) x &= 0x0F; x |= (y & 0xF0) // Clear out the lower nibble // OR in the desired mask
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef LED_BUILTIN //fix for boards without that definition
  #define LED_BUILTIN 13
#endif
