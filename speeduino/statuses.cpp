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

void statuses::setRpm(uint16_t rpm)
{
  ATOMIC()
  {
    this->RPM = rpm;
    this->RPMdiv100 = div100(rpm);
  }
}

static inline uint16_t RpmFromRevolutionTimeUs(uint32_t revTime)
{
  if (revTime==0U) { return 0U; }
  return clamp(fast_div_closest(MICROS_PER_MIN, revTime), (uint32_t)0UL, (uint32_t)MAX_RPM); //Calc RPM based on last full revolution time
}

void statuses::setRevolutionTime(uint32_t revTime)
{
  uint16_t rpm;
  uint8_t rpmDiv100;
  angle_converter_factors_t factors = {0U, 0U};
  bool updateFactors;

  /*
  The divisions below are relatively expensive, so only recalculate when the revolution time changes.
  Compute first, then publish revolution time, RPM and the angle factors together. An ISR must not
  observe a new period beside the previous RPM, or run while a division is in progress.
  */
  if (revTime!=this->revolutionTime)
  {
    rpm = RpmFromRevolutionTimeUs(revTime);
    rpmDiv100 = (uint8_t)div100(rpm);
    /* Keep the last known conversion factors if the speed is unknown: they may still be in use (E.g. per tooth ignition timing adjustments) */
    updateFactors = (revTime!=0U);
    if (updateFactors == true) { factors = calculateAngleConverterFactors(revTime); }

    ATOMIC()
    {
      this->revolutionTime = revTime;
      this->RPM = rpm;
      this->RPMdiv100 = rpmDiv100;
      if (updateFactors == true) { applyAngleConverterFactors(factors); }
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
