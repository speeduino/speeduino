#include "statuses.h"
#include "config_pages.h"
#include "scheduler_ignition_controller.h"
#include "decoder_state.h"

namespace decoders {

namespace shared {

void checkPerToothTiming(const statuses &current, const state_t &decoderState, const config4 &page4, int16_t crankAngle, uint16_t currentTooth)
{
  if ( !current.isFixedCrankingIgnitionTimingActive(page4) 
    && (current.rotationStatus!=EngineRotationStatus::Stopped))
  {
    #define ADJUST_IF_CURRENT(channelNum) \
      if ( (currentTooth == decoderState.ignitionEndTeeth[channelNum-1]) ) \
      { \
        adjustCrankAngle(current, ignitionSchedule ## channelNum, crankAngle); \
      } \
      else

      ADJUST_IF_CURRENT(1)
#if IGN_CHANNELS >= 2
      ADJUST_IF_CURRENT(2)
#endif
#if IGN_CHANNELS >= 3
      ADJUST_IF_CURRENT(3)
#endif
#if IGN_CHANNELS >= 4
      ADJUST_IF_CURRENT(4)
#endif
#if IGN_CHANNELS >= 5
      ADJUST_IF_CURRENT(5)
#endif
#if IGN_CHANNELS >= 6
      ADJUST_IF_CURRENT(6)
#endif
#if IGN_CHANNELS >= 7
      ADJUST_IF_CURRENT(7)
#endif
#if IGN_CHANNELS >= 8
      ADJUST_IF_CURRENT(8)
#endif
    {
      // Keep the code quality police happy...
    }
  }
}

}
}