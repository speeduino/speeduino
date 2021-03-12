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

#define cache_t  SDCache::cache_t
#define sector_t SDClass::sector_t

cache_t *SDCache::cache_list = NULL;
cache_t SDCache::cache[SD_CACHE_SIZE];

#define CACHE_FLAG_HAS_DATA  1
#define CACHE_FLAG_IS_DIRTY  2
#define CACHE_FLAG_IS_FAT    4

//#define PRINT_SECTORS

#ifdef PRINT_SECTORS
static void print_sector(const void *data)
{
	const uint8_t *p = (const uint8_t *)data;
	for (int i=0; i < 512; i++) {
		Serial.printf(" %02X", *p++);
		if ((i & 31) == 31) Serial.println();
	}
}
#endif

void SDCache::print_cache(void)
{
#if 0
	const cache_t *end=cache+SD_CACHE_SIZE;
	for (cache_t *c = cache; c < end; c++) {
		Serial.printf("      cache index %u, lba= %u, ucount=%u, flags=%u\n",
			c - cache, c->lba, c->usagecount, c->flags);
	}
	Serial.print("      cache order:");
	for (cache_t *c = cache_list; c; c = c->next) {
		Serial.printf(" %u ->", c - cache);
	}
	Serial.println();
#endif
}

// Read a sector into the cache.  If the sector is already cached,
// of course no actual read occurs.  This is the primary function
// used to access the SD card.
//
sector_t * SDCache::read(uint32_t lba, bool is_fat)
{
	sector_t *ret = NULL;
	//uint32_t slot=0, ucount=0;

	// the entire read operation, including all cache manipulation,
	// needs to be protected with exclusive access to the hardware.
	//Serial.printf("cache read: lba = %d\n", lba);
	SPI.beginTransaction(SD_SPI_SPEED);
	// does the cache already have the sector?
	cache_t *c = get(lba);
	if (c) {
		if (c->flags & CACHE_FLAG_HAS_DATA) {
			 //Serial.printf("   cache hit,  lba=%u\n", lba);
			ret = &c->data;
		} else {
			if (SDClass::sd_read(lba, &c->data)) {
				c->flags = CACHE_FLAG_HAS_DATA;
				if (is_fat) c->flags |= CACHE_FLAG_IS_FAT;
				ret = &c->data;
				 //Serial.printf("   cache miss, lba=%u\n", lba);
			} else {
				 //Serial.printf("   cache miss: read error, lba=%u\n", lba);
			}
		}
	} else {
		//Serial.printf("   cache full & all in use\n", lba);
	}
	SPI.endTransaction();
	//print_cache();
	return ret;
}

// Read a whole 512 byte sector directly to memory.  If the sector is
// already cached, of course no actual read occurs and data is copied
// from the cache.  When the sector is not cached, it's transferred
// directly from SD card to memory, bypassing the cache.
//
bool SDCache::read(uint32_t lba, void *buffer)
{
	bool ret = true;

	SPI.beginTransaction(SD_SPI_SPEED);
	cache_t *c = get(lba, false);
	if (!c || !(c->flags & CACHE_FLAG_HAS_DATA)) {
		ret = SDClass::sd_read(lba, buffer);
	}
	SPI.endTransaction();
	if (c) {
		if ((c->flags & CACHE_FLAG_HAS_DATA)) {
			memcpy(buffer, &c->data, 512);
			release();
			return true;
		}
		release();
	}
	return ret;
}


// locate a sector in the cache.
cache_t * SDCache::get(uint32_t lba, bool allocate)
{
	cache_t *c, *p=NULL, *last=NULL, *plast=NULL;

	// TODO: move initialization to a function called when the SD card is initialized
	if (cache_list == NULL) init();
	// have we already acquired a cache entry?
	if (item) {
		// if it's the desired block, use it
		if (item->lba == lba) return item;
		// if not, release our hold on it
		release();
	}
	__disable_irq();
	c = cache_list;
	do {
		if (c->lba == lba) {
			if (p) {
				p->next = c->next;
				c->next = cache_list;
				cache_list = c;
			}
			c->usagecount++;
			__enable_irq();
			item = c;
			return item;
		}
		if (c->usagecount == 0) {
			plast = p;
			last = c;
		}
		p = c;
		c = c->next;
	} while (c);
	if (allocate && last) {
		if (plast) {
			plast->next = last->next;
			last->next = cache_list;
			cache_list = last;
		}
		last->usagecount = 1;
		// TODO: flush if dirty
		last->lba = lba;
		last->flags = 0;
		item = last;
	}
	__enable_irq();
	return item;
}


void SDCache::init(void)
{
	cache_t *c = cache;
	cache_t *end = c + SD_CACHE_SIZE;
	//Serial.println("cache init");
	__disable_irq();
	do {
		c->lba = 0xFFFFFFFF;
		c->usagecount = 0;
		c->flags = 0;
		c->next = c + 1;
		c = c + 1;
	} while (c < end);
	c--;
	c->next = NULL;
	cache_list = cache;
	__enable_irq();
}


void SDCache::dirty(void)
{
	__disable_irq();
	item->flags |= CACHE_FLAG_IS_DIRTY;
	__enable_irq();
}

void SDCache::release(void)
{
	//Serial.printf("cache release\n");
	if (item) {
		__disable_irq();
		item->usagecount--;
		//uint32_t ucount = item->usagecount;
		//uint32_t lba = item->lba;
		//Serial.printf("release %d, %d, slot %u\n", item->lba, item->usagecount, item-cache);
		__enable_irq();
		item = NULL;
	}
}







#endif
#endif
#endif