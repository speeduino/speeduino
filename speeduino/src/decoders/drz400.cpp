#include "drz400.h"
#include "scheduledIO_ign.h"
#include "shared/rev_time_calcs.h"

namespace decoders {

namespace drz400 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config4 &page4)
{
  shared::state_t state = {};

  state.decoderFeatures.supportsSequential = true;
  state.decoderStatus.toothAngleIsCorrect = true; //This is always true for this pattern
  state.decoderFeatures.supportsPerToothIgnition = true;  

  state.triggerToothAngle = 360 / page4.triggerTeeth; //The number of degrees that passes from tooth to tooth
  if(page4.TrigSpeed == 1) { state.triggerToothAngle = 720 / page4.triggerTeeth; } //Account for cam speed
  state.toothCurrentCount = UINT8_MAX; //Default value
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * page4.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 2U)); //Same as above, but fixed at 2 teeth on the secondary input
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}


}
}