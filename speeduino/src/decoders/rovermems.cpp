#include "rovermems.h"

namespace decoders {

namespace rovermems {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(config4 &page4)
{
  shared::state_t state = {};

  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 36U)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U)); // only 1 tooth on the wheel not 36

  page4.triggerTeeth = 36;
  state.triggerToothAngle = 360 / page4.triggerTeeth; //The number of degrees that passes from tooth to tooth 360 / 36 theortical teeth
  state.triggerActualTeeth = 36; //The number of physical teeth on the wheel. Need to fix now so we can identify the wheel on the first rotation and not risk a  type 1 wheel not being spotted

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle * 2U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  state.decoderFeatures.supportsPerToothIgnition = true;

   return state;
}


}
}