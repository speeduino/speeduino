#include "vmax.h"

namespace decoders {

namespace renix {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config2 &page2, config4 &page4)
{
  shared::state_t state = {};

  if( page2.nCylinders == 4)
  {
    state.setTriggerToothAngle(90); //The number of degrees that passes from tooth to tooth (primary) this changes between 41 and 49 degrees
    page4.triggerTeeth = 4; // wheel has 44 teeth but we use these to work out which tooth angle to use, therefore speeduino thinks we only have 8 teeth.
    page4.triggerMissingTeeth = 0;
    state.triggerActualTeeth = 4; //The number of teeth we're pretending physically existing on the wheel.
    state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 44U)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  }
  else if (page2.nCylinders == 6)
  {
    state.setTriggerToothAngle(60);
    page4.triggerTeeth = 6; // wheel has 44 teeth but we use these to work out which tooth angle to use, therefore speeduino thinks we only have 6 teeth.
    page4.triggerMissingTeeth = 0;
    state.triggerActualTeeth = 6; //The number of teeth we're pretending physically existing on the wheel.
    state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 66U)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  }
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm). Largest gap between teeth is 90 or 60 degrees depending on decoder.
  state.decoderFeatures.supportsPerToothIgnition = true;

  state.toothSystemCount = 1;
  state.toothCurrentCount = 1;

  return state;
}

}
}