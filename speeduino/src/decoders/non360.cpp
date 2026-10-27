#include "non360.h"

namespace decoders {

namespace non360 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config4 &page4)
{
  shared::state_t state = {};

  state.triggerToothAngle = (360U * page4.TrigAngMul) / page4.triggerTeeth; //The number of degrees that passes from tooth to tooth multiplied by the additional multiplier
  state.toothCurrentCount = UINT8_MAX; //Default value
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * page4.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  state.decoderFeatures.supportsSequential = true;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

}
}