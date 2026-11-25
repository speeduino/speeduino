#include "statuses.h"
#include "atomic.h"
#include "decoder_builder.h"
#include "config_pages.h"
#include "crankMaths.h"

statuses::statuses(void)
{
  (void)memset(this, 0, sizeof(*this));
  decoder = decoder_builder_t().build();
}

#if defined(UNIT_TEST)
void statuses::setRpm(uint16_t rpm)
{
  ATOMIC()
  {
    this->RPM = rpm;
    this->RPMdiv100 = div100(rpm);
  }
}
#endif

void statuses::setRevolutionTime(uint32_t revTime)
{
  // The divisions are relatively expensive, so only recalculate when the revolution time changes
  if (revTime!=this->revolutionTime)
  {
    // Do the divisions before entering the critical section, to keep the time with interrupts disabled short
    const uint16_t rpm = RpmFromRevolutionTimeUs(revTime);
    const uint8_t rpmDiv100 = (uint8_t)div100(rpm);
    const angle_converter_factors_t factors = calculateAngleConverterFactors(revTime);

    // An ISR must never see the new revolution time alongside the old RPM or conversion factors
    ATOMIC()
    {
      this->revolutionTime = revTime;
      this->RPM = rpm;
      this->RPMdiv100 = rpmDiv100;
      // Zero conversion factors would turn every angle into 0uS, so keep the last known factors while the speed is unknown
      if (revTime!=0U) { applyAngleConverterFactors(factors); }
    }
  }
}

bool statuses::isFixedCrankingIgnitionTimingActive(const config4 &page4) const
{
  return   (page4.ignCranklock) 
        && (this->rotationStatus==EngineRotationStatus::Cranking) 
        && (this->decoder.getFeatures().hasFixedCrankingTiming)
        ;
}

static inline uint32_t combineCorrections(uint32_t sumCorrections, uint16_t correction) {
  if (correction == ONE_HUNDRED_PCT) {
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
