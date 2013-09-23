/*
 * Copyright (c) 2013 Ambroz Bizjak
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AMBRO_AVR_ASM_DIV_32_32_LARGE
#define AMBRO_AVR_ASM_DIV_32_32_LARGE

#include <stdint.h>

#define DIVIDE_32_32_ITER_0_7(i) \
"    lsl %D[n]\n" \
"    rol __tmp_reg__\n" \
"    cp __tmp_reg__,%A[d]\n" \
"    cpc __zero_reg__,%B[d]\n" \
"    cpc __zero_reg__,%C[d]\n" \
"    cpc __zero_reg__,%D[d]\n" \
"    brcs zero_bit_" #i "__%=\n" \
"    sub __tmp_reg__,%A[d]\n" \
"    ori %D[q],1<<(7-" #i ")\n" \
"zero_bit_" #i "__%=:\n"

#define DIVIDE_32_32_ITER_8_15(i) \
"    lsl %C[n]\n" \
"    rol __tmp_reg__\n" \
"    rol %D[n]\n" \
"    cp __tmp_reg__,%A[d]\n" \
"    cpc %D[n],%B[d]\n" \
"    cpc __zero_reg__,%C[d]\n" \
"    cpc __zero_reg__,%D[d]\n" \
"    brcs zero_bit_" #i "__%=\n" \
"    sub __tmp_reg__,%A[d]\n" \
"    sbc %D[n],%B[d]\n" \
"    ori %C[q],1<<(15-" #i ")\n" \
"zero_bit_" #i "__%=:\n"

#define DIVIDE_32_32_ITER_16_23(i) \
"    lsl %B[n]\n" \
"    rol __tmp_reg__\n" \
"    rol %D[n]\n" \
"    rol %C[n]\n" \
"    cp __tmp_reg__,%A[d]\n" \
"    cpc %D[n],%B[d]\n" \
"    cpc %C[n],%C[d]\n" \
"    cpc __zero_reg__,%D[d]\n" \
"    brcs zero_bit_" #i "__%=\n" \
"    sub __tmp_reg__,%A[d]\n" \
"    sbc %D[n],%B[d]\n" \
"    sbc %C[n],%C[d]\n" \
"    ori %B[q],1<<(23-" #i ")\n" \
"zero_bit_" #i "__%=:\n"

#define DIVIDE_32_32_ITER_24_30(i) \
"    lsl %A[n]\n" \
"    rol __tmp_reg__\n" \
"    rol %D[n]\n" \
"    rol %C[n]\n" \
"    rol %B[n]\n" \
"    cp __tmp_reg__,%A[d]\n" \
"    cpc %D[n],%B[d]\n" \
"    cpc %C[n],%C[d]\n" \
"    cpc %B[n],%D[d]\n" \
"    brcs zero_bit_" #i "__%=\n" \
"    sub __tmp_reg__,%A[d]\n" \
"    sbc %D[n],%B[d]\n" \
"    sbc %C[n],%C[d]\n" \
"    sbc %B[n],%D[d]\n" \
"    ori %A[q],1<<(31-" #i ")\n" \
"zero_bit_" #i "__%=:\n"

/**
 * Division uint32_t/uint32_t.
 * 
 * Cycles in worst case: 384
 * = 5 + (8 * 9) + (8 * 11) + (8 * 13) + (7 * 15) + 10
 */
static inline uint32_t fastDivide32(uint32_t n, uint32_t d)
{
    uint32_t q;
    
    asm(
        "    clr %A[q]\n"
        "    clr %B[q]\n"
        "    clr %C[q]\n"
        "    clr %D[q]\n"
        "    clr __tmp_reg__\n"
        DIVIDE_32_32_ITER_0_7(0)
        DIVIDE_32_32_ITER_0_7(1)
        DIVIDE_32_32_ITER_0_7(2)
        DIVIDE_32_32_ITER_0_7(3)
        DIVIDE_32_32_ITER_0_7(4)
        DIVIDE_32_32_ITER_0_7(5)
        DIVIDE_32_32_ITER_0_7(6)
        DIVIDE_32_32_ITER_0_7(7)
        DIVIDE_32_32_ITER_8_15(8)
        DIVIDE_32_32_ITER_8_15(9)
        DIVIDE_32_32_ITER_8_15(10)
        DIVIDE_32_32_ITER_8_15(11)
        DIVIDE_32_32_ITER_8_15(12)
        DIVIDE_32_32_ITER_8_15(13)
        DIVIDE_32_32_ITER_8_15(14)
        DIVIDE_32_32_ITER_8_15(15)
        DIVIDE_32_32_ITER_16_23(16)
        DIVIDE_32_32_ITER_16_23(17)
        DIVIDE_32_32_ITER_16_23(18)
        DIVIDE_32_32_ITER_16_23(19)
        DIVIDE_32_32_ITER_16_23(20)
        DIVIDE_32_32_ITER_16_23(21)
        DIVIDE_32_32_ITER_16_23(22)
        DIVIDE_32_32_ITER_16_23(23)
        DIVIDE_32_32_ITER_24_30(24)
        DIVIDE_32_32_ITER_24_30(25)
        DIVIDE_32_32_ITER_24_30(26)
        DIVIDE_32_32_ITER_24_30(27)
        DIVIDE_32_32_ITER_24_30(28)
        DIVIDE_32_32_ITER_24_30(29)
        DIVIDE_32_32_ITER_24_30(30)
        "    lsl %A[n]\n"
        "    rol __tmp_reg__\n"
        "    rol %D[n]\n"
        "    rol %C[n]\n"
        "    rol %B[n]\n"
        "    cp __tmp_reg__,%A[d]\n"
        "    cpc %D[n],%B[d]\n"
        "    cpc %C[n],%C[d]\n"
        "    cpc %B[n],%D[d]\n"
        "    sbci %A[q],-1\n"
        
        : [q] "=&a" (q),
          [n] "=&r" (n)
        : "[n]" (n),
          [d] "r" (d)
    );
    
    return q;
}

//Replace the standard arduino map() function to use the above
long fastMap(long x, long in_min, long in_max, long out_min, long out_max)
{
  return fastDivide32( ((x - in_min) * (out_max - out_min)) , ((in_max - in_min) + out_min) );
}

#endif
