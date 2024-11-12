#pragma once

#include <stdint.h>
#include "globals.h"

/**
 * @file
 * @brief Pulse width calculations
 */

uint16_t calculateRequiredFuel(const config2 &page2, const statuses &current);
uint16_t calculatePWLimit(const config2 &page2, const statuses &current, uint32_t revTime);
uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen, const config10 &page10, const statuses &current);