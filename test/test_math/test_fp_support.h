#pragma once
#include <stdint.h>

void assert_rounded_div(int32_t a, int32_t b, int32_t actual, uint8_t delta = 0);
void assert_rounded_div(int16_t a, int16_t b, int16_t actual, uint8_t delta = 0);
void assert_rounded_div(uint32_t a, uint32_t b, uint32_t actual, uint8_t delta = 0);
void assert_rounded_div(uint16_t a, uint16_t b, uint16_t actual, uint8_t delta = 0);
