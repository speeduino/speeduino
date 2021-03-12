/* Optimized SD Library for Teensy 3.X
 * Copyright (c) 2015, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this SD library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing genuine Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#if defined(__MK64FX512__) || defined(__MK66FX1M0__) 
#if defined(__arm__)
#include "SD_t3.h"
#ifdef USE_TEENSY3_OPTIMIZED_CODE

uint8_t  SDClass::fat_type;
uint32_t SDClass::fat1_begin_lba;
uint32_t SDClass::fat2_begin_lba;
uint32_t SDClass::data_begin_lba;
uint32_t SDClass::max_cluster;
uint8_t  SDClass::sector2cluster;
File SDClass::rootDir;

static uint32_t unaligned_read32_align16(const void *p)
{
	#ifdef KINETISK
	return *(const uint32_t *)p;
	#else
	return *(const uint16_t *)p | (*(const uint16_t *)(p+1) << 16);
	#endif
}

static uint32_t unaligned_read16_align8(const void *p)
{
	#ifdef KINETISK
	return *(const uint16_t *)p;
	#else
	return *(const uint8_t *)p | (*(const uint8_t *)(p+1) << 8);
	#endif
}

#define BPB_BytsPerSec 11   // 2 bytes
#define BPB_SecPerClus 13   // 1 byte
#define BPB_NumFATs    16   // 1 byte
#define BPB_RootEntCnt 17   // 1 byte
#define BPB_TotSec16   19   // 2 bytes
#define BPB_FATSz16    22   // 2 bytes
#define BPB_TotSec32   32   // 4 bytes
#define BPB_FATSz32    36   // 4 bytes
#define BPB_RootClus   44   // 4 bytes

bool SDClass::begin(uint8_t csPin)
{
	uint8_t status;
	uint32_t cond, hcs, ocr;

	// set up the SPI hardware
	csreg = PIN_TO_BASEREG(csPin);
	csmask = PIN_TO_BITMASK(csPin);
	pinMode(csPin, OUTPUT);
	DIRECT_WRITE_HIGH(csreg, csmask);
	SPI.begin();
	// send clocks to initialize hardware
	SPI.beginTransaction(SD_SPI_SPEED);
	for (uint8_t i=0; i < 5; i++) SPI.transfer16(0xFFFF);
	// put the card into idle state
	elapsedMillis msec = 0;
	while (1) {
		status = sd_cmd0();
		//Serial.print("cmd0=");
		//Serial.println(status);
		if (status == 1) break;
		SPI.endTransaction();
		if (msec > 250) return false;
		SPI.beginTransaction(SD_SPI_SPEED);
	}
	// detect version 1 vs 2 cards
	cond = sd_cmd8();
	if (cond == 0x80000000) {
		// version 1 card
		card_type = 1;
		hcs = 0;
	} else if (cond == 0x1AA) {
		// version 2 card
		card_type = 2;
		hcs = (1<<30);
	} else {
		SPI.endTransaction();
		return false;
	}
	//Serial.println();
	// wait for the card to be ready
	msec = 0;
	while (1) {
		status = sd_acmd41(hcs);
		//Serial.println();
		if (status == 0) break;
		SPI.endTransaction();
		if (status > 1) return false;
		if (msec > 1500) return false;
		SPI.beginTransaction(SD_SPI_SPEED);
	}
	//Serial.println("card is ready");
	// detect high capacity cards
	if (card_type == 2) {
		ocr = sd_cmd58();
		//Serial.print("ocr =");
		//Serial.println(ocr, HEX);
		if ((ocr >> 30) == 3) card_type = 3;
	}
	SPI.endTransaction();
	//Serial.println("init ok");
	// read the MBR (partition table)
	SDCache s;
	sector_t * mbr = s.read(0);
	//Serial.printf(" mbr sig = %04X\n", mbr->u16[255]);
	if (mbr->u16[255] != 0xAA55) return false;
	uint32_t partition_lba = 0;
	uint32_t index = 446;
	do {
		uint8_t type = mbr->u8[index+4];
		//Serial.printf(" partition %d is type %d\n", (index-446)/16+1, type);
		if (type == 6 || type == 11 || type == 12) {
			partition_lba = unaligned_read32_align16(mbr->u8 + index + 8);
			//Serial.printf(" partition lba = %d\n", partition_lba);
			break;
		}
		index += 16;
	} while (index < 64);
	s.release();

	// read the FAT volume ID
	sector_t *vol = s.read(partition_lba);
	if (vol->u16[255] != 0xAA55) return false;
	// BPB_BytsPerSec must be 512 bytes per sector
	if (unaligned_read16_align8(vol->u8 + BPB_BytsPerSec) != 512) return false;
	// BPB_NumFATs must be 2 copies of the file allocation table
	if (vol->u8[BPB_NumFATs] != 2) return false;
	uint32_t reserved_sectors = vol->u16[14/2];
	if (reserved_sectors == 0) return false;
	//Serial.printf(" reserved_sectors = %d\n", reserved_sectors);
	uint32_t sectors_per_cluster = vol->u8[BPB_SecPerClus];
	//Serial.printf(" sectors_per_cluster = %d\n", sectors_per_cluster);
	uint32_t s2c = 31 - __builtin_clz(sectors_per_cluster);
	//Serial.printf(" s2c = %d\n", s2c);
	sector2cluster = s2c;
	uint32_t sectors_per_fat = vol->u16[BPB_FATSz16/2];
	if (sectors_per_fat == 0) sectors_per_fat = vol->u32[BPB_FATSz32/4];
	//Serial.printf(" sectors_per_fat = %d\n", sectors_per_fat);
	uint32_t root_dir_entries = unaligned_read16_align8(vol->u8 + BPB_RootEntCnt);
	//Serial.printf(" root_dir_entries = %d\n", root_dir_entries);
	uint32_t root_dir_sectors = (root_dir_entries + 15) >> 4;
	//Serial.printf(" root_dir_sectors = %d\n", root_dir_sectors);

	uint32_t total_sectors = unaligned_read16_align8(vol->u8 + BPB_TotSec16);
	if (total_sectors == 0) total_sectors = vol->u32[BPB_TotSec32/4];
	//Serial.printf(" total_sectors = %d\n", total_sectors);

	fat1_begin_lba = partition_lba + reserved_sectors;
	fat2_begin_lba = fat1_begin_lba + sectors_per_fat;
	data_begin_lba = fat2_begin_lba + sectors_per_fat + root_dir_sectors;

	uint32_t cluster_count = (total_sectors - reserved_sectors
		- root_dir_sectors - (sectors_per_fat << 1)) >> s2c;
	//Serial.printf(" cluster_count = %d\n", cluster_count);
	max_cluster = cluster_count + 1;
	if (cluster_count < 4085) {
		return false; // FAT12
	} else if (cluster_count < 65525) {
		fat_type = 16;
		rootDir.length = root_dir_entries << 5;
		rootDir.start_cluster = partition_lba + reserved_sectors + sectors_per_fat * 2;
		rootDir.type = FILE_DIR_ROOT16;
	} else {
		fat_type = 32;
		rootDir.length = 0;
		rootDir.start_cluster = vol->u32[BPB_RootClus/4];
		//Serial.printf(" root cluster = %d\n", rootDir.start_cluster);
		rootDir.type = FILE_DIR;
	}
	rootDir.current_cluster = rootDir.start_cluster;
	rootDir.offset = 0;
	s.release();
	//Serial.println(sizeof(fatdir_t));
	return true;
}

#endif
#endif
#endif