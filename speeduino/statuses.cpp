#include "statuses.h"
#include "atomic.h"
#include "decoder_builder.h"
#include "config_pages.h"

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

bool statuses::isFixedCrankingIgnitionTimingActive(const config4 &page4) const
{
  return   (page4.ignCranklock) 
        && (this->rotationStatus==EngineRotationStatus::Cranking) 
        && (this->decoder.getFeatures().hasFixedCrankingTiming)
        ;
}
