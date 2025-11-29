/**
 * @file
 * @brief Pulse width calculations
 */
#pragma once

#include <stdint.h>
#include "config_pages.h"
#include "statuses.h"

/** @brief Result of pulse width calculation */
struct pulseWidths {
  /** @brief Primary pulse width in µS */
  uint16_t primary;

  /** @brief Secondary pulse width in µS. 
   * 
   * Will be zero if no secondary pulse width is required. 
   * E.g. staged injection is not turned on or the required
   * fuel can be applied using the primary injectors.
   */
  uint16_t secondary;
};

/**
 * @brief This function calculates the required pulsewidth time (in us) given the current system state
 * 
 * @param REQ_FUEL The required fuel value in uS, as calculated by TunerStudio
 * @param VE Lookup from the main fuel table. This can either have been MAP or TPS based, depending on the algorithm used
 * @param MAP In KPa, read from the sensor (This is used when performing a multiply of the map only. It is applicable in both Speed density and Alpha-N)
 * @param corrections Sum of Enrichment factors (Cold start, acceleration). This is a multiplication factor (Eg to add 10%, this should be 110)
 * @param injOpen Injector opening time. The time the injector take to open minus the time it takes to close (Both in uS)
 * @return uint16_t The injector pulse width in uS
 */
uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen, const config10 &page10, const statuses &current);
uint16_t calculateRequiredFuel(const config2 &page2, const statuses &current);
uint16_t calculatePWLimit(const config2 &page2, const statuses &current, uint32_t revTime);

// Apply the pwLimit if staging is disabled and engine is not cranking
static inline uint16_t applyPwLimits(uint16_t pw, uint16_t pwLimit, uint16_t injOpenTime, const config10 &page10, const statuses &current) {
  if (pw<=injOpenTime) {
    return 0U;
  }
  if( !current.engineIsCranking && (page10.stagingEnabled == false) ) { 
    return min(pw, pwLimit);
  }
  return pw;
}

static inline bool canApplyStaging(const config2 &page2, const config10 &page10) {
    //To run staged injection, the number of cylinders must be less than the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
 return  (page10.stagingEnabled == true) 
      && (page2.nCylinders <= INJ_CHANNELS || page2.injType == INJ_TYPE_TBODY); //Final check is to ensure that DFCO isn't active, which would cause an overflow below (See #267)  
}

pulseWidths calculateSecondaryPw(uint16_t primaryPw, uint16_t pwLimit, uint16_t injOpenTime, const config2 &page2, const config10 &page10, const statuses &current);
void applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, statuses &current);
