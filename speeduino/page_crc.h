#pragma once
#include <Arduino.h>

/*
 * Calculates and returns the CRC32 value of a given page of memory
 */
uint32_t calculatePageCRC32(byte pageNum /**< [in] The page number to compute CRC for. */);