#include <algorithm>
#include "fuel_corrections_t.h"
#include "config_pages.h"
#include "statuses.h"

static inline uint32_t combineCorrections(uint32_t sumCorrections, uint16_t correction) {
  if (correction == NO_FUEL_CORRECTION) {
    return sumCorrections;
  }
  return percentageApprox(correction, sumCorrections);
}

uint16_t fuel_corrections_t::calculatePWCorrection(const config2 &page2, const statuses &current) const
{
  uint32_t sumCorrections = wue;
  sumCorrections = combineCorrections(sumCorrections, ase);
  sumCorrections = combineCorrections(sumCorrections, cranking);
  // multiply by the AE amount in case of multiplier AE mode or Decel
  if ( (page2.aeApplyMode == AE_MODE_MULTIPLIER) || (current.isDeceleratingTPS) )
  {
    sumCorrections = combineCorrections(sumCorrections, acceleration);
  }
  sumCorrections = combineCorrections(sumCorrections, floodClear);
  sumCorrections = combineCorrections(sumCorrections, closedLoopAFR);
  sumCorrections = combineCorrections(sumCorrections, iat);
  sumCorrections = combineCorrections(sumCorrections, baro);
  sumCorrections = combineCorrections(sumCorrections, flex);
  sumCorrections = combineCorrections(sumCorrections, fuelTemp);
  sumCorrections = combineCorrections(sumCorrections, launch);
  sumCorrections = combineCorrections(sumCorrections, dfco);

  //This is the maximum allowable increase
  return (std::min)((uint16_t)1500U, (uint16_t)sumCorrections);
}