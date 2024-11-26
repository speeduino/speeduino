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

// Apply the pwLimit if staging is disabled and engine is not cranking
static inline uint16_t applyPwLimits(uint16_t pw, uint16_t pwLimit, uint16_t injOpenTime, const config10 &page10, const statuses &current) {
  if (pw<=injOpenTime) {
    return 0U;
  }
  if( (!BIT_CHECK(current.engine, BIT_ENGINE_CRANK)) && (page10.stagingEnabled == false) ) { 
    return min(pw, pwLimit);
  }
  return pw;
}