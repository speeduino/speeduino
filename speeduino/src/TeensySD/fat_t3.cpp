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

bool File::next_cluster()
{
	SDCache fat;
	uint32_t lba, cluster;

	lba = SDClass::fat1_begin_lba;
	cluster = current_cluster;
	//Serial.println();
	//Serial.println("****************************************");
	//Serial.println();
	//Serial.printf("   current_cluster = %d\n", cluster);

	if (SDClass::fat_type == 16) {
		SDClass::sector_t *s = fat.read(lba + (cluster >> 8), true);
		if (!s) return false;
		cluster = s->u16[cluster & 255];
	} else {
		SDClass::sector_t *s = fat.read(lba + (cluster >> 7), true);
		if (!s) return false;
		cluster = s->u32[cluster & 127];
	}

	//Serial.printf("    new_cluster = %d\n", cluster);
	//Serial.println();
	//Serial.println("****************************************");
	//Serial.println();
	current_cluster = cluster;
	if (cluster > SDClass::max_cluster) return false;
	return true;
}

#endif
#endif
#endif