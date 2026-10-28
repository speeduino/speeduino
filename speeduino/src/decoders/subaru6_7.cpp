#include "subaru6_7.h"

namespace decoders {

namespace subaru67 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state = {};

  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 360UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = 0;
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.toothCurrentCount = 1;
  state.triggerToothAngle = 2;
  state.decoderStatus.toothAngleIsCorrect = false;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * 93U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  state.toothAngles[0] = 710; //tooth #1
  state.toothAngles[1] = 83; //tooth #2
  state.toothAngles[2] = 115; //tooth #3
  state.toothAngles[3] = 170; //tooth #4
  state.toothAngles[4] = state.toothAngles[1] + 180;
  state.toothAngles[5] = state.toothAngles[2] + 180;
  state.toothAngles[6] = state.toothAngles[3] + 180;
  state.toothAngles[7] = state.toothAngles[1] + 360;
  state.toothAngles[8] = state.toothAngles[2] + 360;
  state.toothAngles[9] = state.toothAngles[3] + 360;
  state.toothAngles[10] = state.toothAngles[1] + 540;
  state.toothAngles[11] = state.toothAngles[2] + 540;

  return state;
}

}
}