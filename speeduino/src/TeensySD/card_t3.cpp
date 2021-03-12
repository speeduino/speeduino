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

volatile uint8_t * SDClass::csreg;
uint8_t SDClass::csmask;
uint8_t SDClass::card_type;


#define CMD0_GO_IDLE_STATE        0x4095 // arg=0
#define CMD1_SEND_OP_COND         0x41FF
#define CMD8_SEND_IF_COND         0x4887 // arg=0x1AA
#define CMD55_APP_CMD             0x77FF
#define ACMD41_SD_SEND_OP_COND    0x69FF
#define CMD58_READ_OCR            0x7AFF
#define CMD17_READ_SINGLE_BLOCK   0x51FF
#define CMD24_WRITE_BLOCK         0x58FF


uint8_t SDClass::sd_cmd0()
{
	send_cmd(CMD0_GO_IDLE_STATE, 0);
	uint8_t r1 = recv_r1();
	end_cmd();
	return r1;
}

uint32_t SDClass::sd_cmd8()
{
	send_cmd(CMD8_SEND_IF_COND, 0x1AA);
	uint8_t r1 = recv_r1();
	uint32_t cond = 0x80000000;
	if (r1 == 1) {
		cond = recv_r3_or_r7();
		//Serial.print(cond, HEX);
	}
	end_cmd();
	return cond;
}

uint8_t SDClass::sd_acmd41(uint32_t hcs)
{
	//Serial.print("acmd41:");
	send_cmd(CMD55_APP_CMD, 0);
	uint8_t r1 = recv_r1();
	end_cmd();
	send_cmd(ACMD41_SD_SEND_OP_COND, hcs);
	r1 = recv_r1();
	end_cmd();
	return r1;
}

uint32_t SDClass::sd_cmd58(void)
{
	send_cmd(CMD58_READ_OCR, 0);
	uint8_t r1 = recv_r1();
	uint32_t ocr = 0;
	if (r1 == 0) ocr = recv_r3_or_r7();
	end_cmd();
	return ocr;
}

bool SDClass::sd_read(uint32_t addr, void * data)
{
	//Serial.printf("sd_read %ld\n", addr);
	if (card_type < 2) addr = addr << 9;
	send_cmd(CMD17_READ_SINGLE_BLOCK, addr);
	uint8_t r1 = recv_r1();
	if (r1 != 0) {
		end_cmd();
		//Serial.println("    sd_read fail r1");
		return false;
	}
	while (1) {
		uint8_t token = SPI.transfer(0xFF);
		//Serial.printf("t=%02X.", token);
		if (token == 0xFE) break;
		if (token != 0xFF) {
			end_cmd();
			//Serial.println("    sd_read fail token");
			return false;
		}
		// TODO: timeout
	}
	uint8_t *p = (uint8_t *)data;
	uint8_t *end = p + 510;
	SPI0_PUSHR = 0xFFFF | SPI_PUSHR_CTAS(1);
	SPI0_PUSHR = 0xFFFF | SPI_PUSHR_CTAS(1);
	while (p < end) {
		while (!(SPI0_SR & 0xF0)) ;
		SPI0_PUSHR = 0xFFFF | SPI_PUSHR_CTAS(1);
		uint32_t in = SPI0_POPR;
		*p++ = in >> 8;
		*p++ = in;
	}
	while (!(SPI0_SR & 0xF0)) ;
	uint32_t in = SPI0_POPR;
	*p++ = in >> 8;
	*p++ = in;
	while (!(SPI0_SR & 0xF0)) ;
	SPI0_POPR; // ignore crc
	DIRECT_WRITE_HIGH(csreg, csmask);
	SPI.transfer(0xFF);
	return true;
	// token = 0xFE
	// data, 512 bytes
	// crc, 2 bytes
}


void SDClass::send_cmd(uint16_t cmd, uint32_t arg)
{
	DIRECT_WRITE_LOW(csreg, csmask);
	SPI.transfer(cmd >> 8);
	SPI.transfer16(arg >> 16);
	SPI.transfer16(arg);
	SPI.transfer(cmd);
}

uint8_t SDClass::recv_r1(void)
{
	uint8_t ret, count=0;
	do {
		ret = SPI.transfer(0xFF);
		//Serial.print(ret);
		//Serial.print(".");
		if ((ret & 0x80) == 0) break;
	} while (++count < 9);
	return ret;
}

uint32_t SDClass::recv_r3_or_r7(void)
{
	uint32_t r;
	r = SPI.transfer16(0xFFFF) << 16;
	r |= SPI.transfer16(0xFFFF);
	return r;
}

void SDClass::end_cmd(void)
{
	DIRECT_WRITE_HIGH(csreg, csmask);
	SPI.transfer(0xFF);
}

#endif
#endif
#endif
