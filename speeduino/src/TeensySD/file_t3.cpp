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

#define sector_t SDClass::sector_t

File::File()
{
	type = FILE_INVALID;
	namestr[0] = 0;
}

File::~File(void)
{
	close();
}

size_t File::write(uint8_t b)
{
	return write(&b, 1);
}

size_t File::write(const uint8_t *buf, size_t size)
{
	if (type != FILE_WRITE) {
		setWriteError();
		return 0;
	}
	// TODO: a lot of work....
	return 0;
}

int File::read()
{
	uint8_t b;
	int ret = read(&b, 1);
	if (ret != 1) return -1;
	return b;
}

int File::peek()
{
	uint32_t save_offset = offset;
	uint32_t save_cluster = current_cluster;
	uint8_t b;
	int ret = read(&b, 1);
	if (ret != 1) return -1;
	offset = save_offset;
	current_cluster = save_cluster;
	return b;
}

int File::available()
{
	if (type > FILE_WRITE) return 0;
	uint32_t maxsize = length - offset;
	if (maxsize > 0x7FFFFFFF) maxsize = 0x7FFFFFFF;
	return maxsize;
}

void File::flush()
{

}

int File::read(void *buf, uint32_t size)
{
	if (type > FILE_WRITE) return 0;
	uint32_t maxsize = length - offset;
	if (size > maxsize) size = maxsize;
	if (size == 0) return 0;
	uint32_t count = 0;
	uint8_t *dest = (uint8_t *)buf;
	uint32_t lba = custer_to_sector(current_cluster);
	uint32_t sindex = cluster_offset(offset);

	//Serial.printf(" read %u at %u  (%X)\n", size, offset, offset);

	lba += sindex >> 9;
	sindex &= 511;
	if (sindex) {
		// first read starts in the middle of a sector
		do {
			SDCache cache;
			sector_t *sector = cache.read(lba);
			if (!sector) {
				//Serial.println(" read err1, unable to read");
				return 0;
			}
			uint32_t n = 512 - sindex;
			if (size < n) {
				// read does not consume all of the sector
				memcpy(dest, sector->u8 + sindex, size);
				offset += size;
				//cache.priority(+1);
				return size;
			} else {
				// read fully consumes this sector
				memcpy(dest, sector->u8 + sindex, n);
				dest += n;
				count = n;
				offset += n;
				//cache.priority(-1);
			}
		} while (0);
		if (is_new_cluster(++lba)) {
			if (!next_cluster()) {
				//Serial.print(" read err1, next cluster");
				return count;
			}
		}
		if (count >= size) return count;
	}
	while (1) {
		// every read starts from the beginning of a sector
		do {
			SDCache cache;
			uint32_t n = size - count;
			if (n < 512) {
				// only part of a sector is needed
				sector_t *sector = cache.read(lba);
				if (!sector) {
					//Serial.println(" read err2, unable to read");
					return count;
				}
				memcpy(dest, sector->u8, n);
				offset += n;
				count += n;
				//cache.priority(+1);
				return count;
			} else {
				// a full sector is required
				if (!cache.read(lba, dest)) return count;
				dest += 512;
				offset += 512;
				count += 512;
			}
		} while (0);
		if (is_new_cluster(++lba)) {
			if (!next_cluster()) {
				//Serial.print(" read err2, next cluster");
				return count;
			}
		}
		if (count >= size) return count;
	}
}

bool File::seek(uint32_t pos)
{
	if (type > FILE_WRITE) return false;
	if (pos > length) return false;

	//Serial.printf(" seek to %u\n", pos);
	uint32_t save_cluster = current_cluster;
	uint32_t count;
	// TODO: if moving to a new lba, lower cache priority
	signed int diff = (int)cluster_number(pos) - (int)cluster_number(offset);
	if (diff >= 0) {
		// seek fowards, 0 or more clusters from current position
		count = diff;
	} else {
		// seek backwards, need to start from beginning of file
		current_cluster = start_cluster;
		count = cluster_number(pos);
	}
	while (count > 0) {
		if (!next_cluster()) {
			current_cluster = save_cluster;
			return false;
		}
		count--;
	}
	offset = pos;
	return true;
}

void File::close()
{
	type = FILE_INVALID;
	namestr[0] = 0;
}

#endif
#endif
#endif