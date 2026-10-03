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

void statuses::setRevolutionTime(uint32_t revTime)
{
  // The divisions are relatively expensive, so only recalculate when the revolution time changes
  if (revTime!=this->revolutionTime)
  {
    const uint16_t rpm = RpmFromRevolutionTimeUs(revTime);
    const uint8_t rpmDiv100 = (uint8_t)div100(rpm);

    // An ISR must never see the new revolution time alongside the old RPM or conversion factors
    ATOMIC()
    {
      this->revolutionTime = revTime;
      this->RPM = rpm;
      this->RPMdiv100 = rpmDiv100;
      // Zero conversion factors would turn every angle into 0uS, so keep the last known factors while the speed is unknown
      if (revTime!=0U) { setAngleConverterRevolutionTime(revTime); }
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
