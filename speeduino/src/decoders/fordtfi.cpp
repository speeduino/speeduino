#include "fordtfi.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"

namespace decoders {

namespace ford_tfi {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config2 &page2)
{
  shared::state_t state = {};

  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  state.decoderStatus.toothAngleIsCorrect = true; //This is always true for this pattern

  state.triggerActualTeeth = page2.nCylinders;
  if(state.triggerActualTeeth == 0) { state.triggerActualTeeth = 1; }

  state.setTriggerToothAngle(720U / state.triggerActualTeeth); //The number of degrees that passes from tooth to tooth, half cylinder count
  state.toothCurrentCount = 0; //Default value
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 30U * state.triggerActualTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = state.triggerFilterTime * 4U /5u; //Same as above, but slightly about lower due to signature trigger (about 80%)
  state.lastSyncRevolution = 0;
  if(page2.nCylinders <= 4U) { state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/90U) * state.triggerToothAngle); }//Minimum 90rpm. (1851uS is the time per degree at 90rpm). This uses 90rpm rather than 50rpm due to the potentially very high stall time on a 4 cylinder if we wait that long.
  else { state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); } //Minimum 50rpm. (3200uS is the time per degree at 50rpm).

  return state;
}

}
}