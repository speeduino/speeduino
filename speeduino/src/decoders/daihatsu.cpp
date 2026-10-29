#include "daihatsu.h"


namespace decoders {

namespace daihatsu {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config2 &page2)
{
  shared::state_t state = {};

  state.triggerActualTeeth = page2.nCylinders + 1;
  state.triggerToothAngle = 720 / state.triggerActualTeeth; //The number of degrees that passes from tooth to tooth
  state.triggerFilterTime = MICROS_PER_MIN / MAX_RPM / page2.nCylinders; // Minimum time required between teeth
  state.triggerFilterTime = state.triggerFilterTime / 2; //Safety margin
  state.decoderFeatures.supportsSequential = true;

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/90U) * state.triggerToothAngle)*4U;//Minimum 90rpm. (1851uS is the time per degree at 90rpm). This uses 90rpm rather than 50rpm due to the potentially very high stall time on a 4 cylinder if we wait that long.

  if(page2.nCylinders == 3)
  {
    state.toothAngles[0] = 0; //tooth #1
    state.toothAngles[1] = 30; //tooth #2 (Extra tooth)
    state.toothAngles[2] = 240; //tooth #3
    state.toothAngles[3] = 480; //tooth #4
  }
  else
  {
    //Should be 4 cylinders here
    state.toothAngles[0] = 0; //tooth #1
    state.toothAngles[1] = 30; //tooth #2 (Extra tooth)
    state.toothAngles[2] = 180; //tooth #3
    state.toothAngles[3] = 360; //tooth #4
    state.toothAngles[4] = 540; //tooth #5
  }

  return state;
}

}
}