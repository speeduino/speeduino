#include "36minus21.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"

namespace decoders {

namespace thirtysixminus21 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config4 &page4)
{
  shared::state_t state = {};

  state.triggerToothAngle = 10; //The number of degrees that passes from tooth to tooth
  state.triggerActualTeeth = 33; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt. Not Used
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 36)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.checkSyncToothCount = (page4.triggerTeeth) >> 1; //50% of the total teeth.
  state.toothLastMinusOneToothTime = 0;
  state.toothCurrentCount = 0;
  state.toothOneTime = 0;
  state.toothOneMinusOneTime = 0;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle * 2U ); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}


}
}