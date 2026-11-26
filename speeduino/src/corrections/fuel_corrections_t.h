/**
 * @file 
 * @brief fuel_corrections_t & supporting declarations 
 */
#pragma once

#include "maths.h"

/// @cond
// Forward declarations
struct config2;
struct statuses; 
/// @endcond

// Constant that represents "no fuel correction"
static constexpr uint8_t NO_FUEL_CORRECTION = ONE_HUNDRED_PCT;

/** 
 * @brief The corrections that are/will be applied to the pulse width 
 * 
 * These are percentages: 
 *   1. 100% representing "no correction".
 *   2. <100% results in a lean correction
 *   3. >100% results in a rich coorection
 * 
 * Individual corrections are combined to produce a single correction 
 * percentage which is then applied to the base pulse width.  
 */
struct fuel_corrections_t
{
  uint8_t wue = NO_FUEL_CORRECTION; ///< Warm Up Enrichment
  uint8_t ase = NO_FUEL_CORRECTION; ///< After Start Enrichment
  uint16_t acceleration = NO_FUEL_CORRECTION; ///< The amount of acceleration enrichment currently being applied. 100=No change. Varies above 255
  uint8_t floodClear = NO_FUEL_CORRECTION; ///< Flood clear correction
  uint8_t closedLoopAFR = NO_FUEL_CORRECTION; ///< The amount of closed loop AFR enrichment currently being applied
  uint8_t iat = NO_FUEL_CORRECTION; ///< The amount of inlet air temperature adjustment currently being applied
  uint8_t baro = NO_FUEL_CORRECTION; ///< The amount of correction being applied for the current baro reading
  uint8_t flex = NO_FUEL_CORRECTION; ///< Amount of correction being applied to compensate for ethanol content
  uint8_t fuelTemp = NO_FUEL_CORRECTION; ///< Amount of correction being applied to compensate for fuel temperature
  uint8_t launch = NO_FUEL_CORRECTION; ///< The amount of correction being applied if launch control is active
  uint8_t dfco = NO_FUEL_CORRECTION; ///< Deceleration Fuel Cut Off
  uint16_t cranking = NO_FUEL_CORRECTION; ///< Cranking correction

  uint16_t calculatePWCorrection(const config2 &page2, const statuses &current) const;
  
  uint8_t battery = NO_FUEL_CORRECTION; ///< Voltage correction is applied to the injector opening time
};
