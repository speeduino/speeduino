#include "mazdaau.h"

namespace decoders {

namespace mazdaau {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state;

  state.triggerToothAngle = 108; //The number of degrees that passes from tooth to tooth (primary). This is the maximum gap
  state.toothCurrentCount = 99; //Fake tooth count represents no sync

  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.hasFixedCrankingTiming = true;

  state.toothAngles[0] = 348; //tooth #1
  state.toothAngles[1] = 96; //tooth #2
  state.toothAngles[2] = 168; //tooth #3
  state.toothAngles[3] = 276; //tooth #4

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  state.triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)

  return state;
}

}
}