#include "statuses.h"
#include "config_pages.h"
#include "scheduler_ignition_controller.h"
#include "decoder_state.h"

namespace decoders {

namespace detail {

void checkPerToothTiming(const statuses &current, const state_t &decoderState, const config4 &page4, int16_t crankAngle, uint16_t currentTooth)
{
  if ( !current.isFixedCrankingIgnitionTimingActive(page4) && (current.rotationStatus!=EngineRotationStatus::Stopped) )
  {
    if ( (currentTooth == decoderState.ignitionEndTeeth[0]) )
    {
      adjustCrankAngle(current, ignitionSchedule1, crankAngle);
    }
#if IGN_CHANNELS >= 2
    else if ( (currentTooth == decoderState.ignitionEndTeeth[1]) )
    {
      adjustCrankAngle(current, ignitionSchedule2, crankAngle);
    }
#endif
#if IGN_CHANNELS >= 3
    else if ( (currentTooth == decoderState.ignitionEndTeeth[2]) )
    {
      adjustCrankAngle(current, ignitionSchedule3, crankAngle);
    }
#endif
#if IGN_CHANNELS >= 4
    else if ( (currentTooth == decoderState.ignitionEndTeeth[3]) )
    {
      adjustCrankAngle(current, ignitionSchedule4, crankAngle);
    }
#endif
#if IGN_CHANNELS >= 5
    else if ( (currentTooth == decoderState.ignitionEndTeeth[4]) )
    {
      adjustCrankAngle(current, ignitionSchedule5, crankAngle);
    }
#endif
#if IGN_CHANNELS >= 6
    else if ( (currentTooth == decoderState.ignitionEndTeeth[5]) )
    {
      adjustCrankAngle(current, ignitionSchedule6, crankAngle);
    }
#endif
#if IGN_CHANNELS >= 7
    else if ( (currentTooth == decoderState.ignitionEndTeeth[6]) )
    {
      adjustCrankAngle(current, ignitionSchedule7, crankAngle);
    }
#endif
#if IGN_CHANNELS >= 8
    else if ( (currentTooth == decoderState.ignitionEndTeeth[7]) )
    {
      adjustCrankAngle(current, ignitionSchedule8, crankAngle);
    }
#endif
  }
}

}
}