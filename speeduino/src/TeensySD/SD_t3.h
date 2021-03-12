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

// This Teensy 3.x optimized version is a work-in-progress.
//
// Uncomment this line to use the Teensy version, which completely replaces
// all of the normal Arduino SD library code.  The optimized version is
// currently read-only.  It CAN NOT WRITE ANYTHING TO YOUR SD CARD.  However,
// it is *much* faster for reading more than 1 file at a time, especially for
// the Teensy Audio Library to play and mix multiple sound files.
// On Teensy 3.5 & 3.6, this optimization does NOT SUPPORT the built-in SD
// sockets.  It only works with SD cards connected to the SPI pins.
//
//#define USE_TEENSY3_OPTIMIZED_CODE

/* Why reinvent the SD library wheel...
 *   1: Allow reading files from within interrupts
 *   2: Cache more than one sector for improved performance
 *   3: General optimization for 32 bit ARM on Teensy 3.x & Teensy-LC
 *   4: Permissive MIT license
 */
#if defined(__MK64FX512__) || defined(__MK66FX1M0__) 
#if !defined(__SD_t3_H__) && defined(__arm__) && defined(USE_TEENSY3_OPTIMIZED_CODE)
#define __SD_t3_H__
#define __SD_H__

#include <Arduino.h>
#include <SPI.h>
#include "utility/ioreg.h"

#define SD_CACHE_SIZE   7  // each cache entry uses 520 bytes of RAM

#define SD_SPI_SPEED  SPISettings(25000000, MSBFIRST, SPI_MODE0)

#define FILE_READ	0
#define FILE_WRITE	1
#define FILE_DIR	2
#define FILE_DIR_ROOT16 3
#define FILE_INVALID	4

class File;

class SDClass
{
public:
	static bool begin(uint8_t csPin = SS);
	static File open(const char *path, uint8_t mode = FILE_READ);
	static bool exists(const char *path);
	static bool mkdir(const char *path);
	static bool remove(const char *path);
	static bool rmdir(const char *path);
private:
	static uint8_t sd_cmd0();
	static uint32_t sd_cmd8();
	static uint8_t sd_acmd41(uint32_t hcs);
	static uint32_t sd_cmd58();
	static bool sd_read(uint32_t addr, void * data);
	static void send_cmd(uint16_t cmd, uint32_t arg);
	static uint8_t recv_r1();
	static uint32_t recv_r3_or_r7();
	static void end_cmd();
	static volatile IO_REG_TYPE * csreg;
	static IO_REG_TYPE csmask;
	static uint8_t card_type; // 1=SDv1, 2=SDv2, 3=SDHC
	static File rootDir;
	static uint32_t fat1_begin_lba;
	static uint32_t fat2_begin_lba;
	static uint32_t data_begin_lba;
	static uint32_t max_cluster;
	static uint8_t sector2cluster;
	static uint8_t fat_type;
	friend class SDCache;
	friend class File;
	typedef struct {
		union {
			struct { // short 8.3 filename info
				char name[11];
				uint8_t attrib;
				uint8_t reserved;
				uint8_t ctime_tenth;
				uint16_t ctime;
				uint16_t cdate;
				uint16_t adate;
				uint16_t cluster_high;
				uint16_t wtime;
				uint16_t wdate;
				uint16_t cluster_low;
				uint32_t size;
			};
			struct { // long filename info
				uint8_t ord;
				uint8_t lname1[10];
				uint8_t lattrib;
				uint8_t type;
				uint8_t cksum;
				uint8_t lname2[12];
				uint16_t lcluster_low;
				uint8_t lname3[4];
			};
		};
	} fatdir_t;
	typedef union {
		uint8_t u8[512];
		uint16_t u16[256];
		uint32_t u32[128];
		fatdir_t dir[16];
	} sector_t;
};


#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LONG_NAME  0x0F

class File : public Stream
{
public:
	File();
	~File();
	// TODO: copy constructors, needs to be ISR safe
	virtual size_t write(uint8_t b);
	virtual size_t write(const uint8_t *buf, size_t size);
	virtual int read();
	virtual int peek();
	virtual int available();
	virtual void flush();
	int read(void *buf, uint32_t size);
	bool seek(uint32_t pos);
	uint32_t position() {
		if (type <= FILE_WRITE) return offset;
		return 0;
	}
	uint32_t size() {
		if (type <= FILE_WRITE) return length;
		return 0;
	}
	void close();
	operator bool() {
		return (type < FILE_INVALID);
	}
	char * name() {
		return namestr;
	}
	bool isDirectory() {
		return (type == FILE_DIR) || (type == FILE_DIR_ROOT16);
	}
	File openNextFile(uint8_t mode = FILE_READ);
	void rewindDirectory() {
		rewind();
	}
	using Print::write;
	void rewind() {
		offset = 0;
		current_cluster = start_cluster;
	};
private:
	bool find(const char *filename, File *found);
	void init(SDClass::fatdir_t *dirent);
	bool next_cluster();
	uint32_t offset;          // position within file (EOF = length)
	uint32_t length;          // total size of file
	uint32_t start_cluster;   // first cluster for the file
	uint32_t current_cluster; // position (must agree w/ offset)
	uint32_t dirent_lba;      // dir sector for this file
	uint8_t  dirent_index;    // dir index within sector (0 to 15)
	uint8_t type;             // file vs dir
	char namestr[13];
	friend class SDClass;
	static inline uint32_t cluster_number(uint32_t n) {
		return n >> (SDClass::sector2cluster + 9);
	}
	static inline uint32_t cluster_offset(uint32_t n) {
		return n & ((1 << (SDClass::sector2cluster + 9)) - 1);
	}
	static inline uint32_t custer_to_sector(uint32_t n) {
		return (n - 2) * (1 << SDClass::sector2cluster)
			+ SDClass::data_begin_lba;
	}
	static inline bool is_new_cluster(uint32_t lba) {
		return (lba & ((1 << SDClass::sector2cluster) - 1)) == 0;
	}
};

class SDCache
{
private:
	// SDCache objects should be created with local scope.
	// read(), get(), alloc() acquire temporary locks on
	// cache buffers, which are automatically released
	// by the destructor when the object goes out of scope.
	// Pointers returned by those functions must NEVER be
	// used after the SDCache object which returned them
	// no longer exists.
	SDCache(void) { item = NULL; }
	~SDCache(void) { release(); }
	typedef struct cache_struct {
		SDClass::sector_t data;
		uint32_t lba;
		cache_struct * next;
		uint8_t  usagecount;
		uint8_t  flags;
	} cache_t;
	SDClass::sector_t * read(uint32_t lba, bool is_fat=false);
	bool read(uint32_t lba, void *buffer);
	cache_t * get(uint32_t lba, bool allocate=true);
	void dirty(void);
	void flush(void);
	void release(void);
	cache_t * item;
	static cache_t *cache_list;
	static cache_t cache[SD_CACHE_SIZE];
	static void init(void);
	static void print_cache(void);
	friend class SDClass;
	friend class File;
};


extern SDClass SD;

#endif
#endif