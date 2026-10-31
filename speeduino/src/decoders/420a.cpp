#include "420a.h"

namespace decoders {

namespace dsm420a {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state = {};

  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 360UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.toothCurrentCount = 1;
  state.triggerToothAngle = 20; //Is only correct for the 4 short pulses before each TDC
  state.decoderStatus.toothAngleIsCorrect = false;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * 93U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  state.toothAngles[0] = 711; //tooth #1, just before #1 TDC
  state.toothAngles[1] = 111;
  state.toothAngles[2] = 131;
  state.toothAngles[3] = 151;
  state.toothAngles[4] = 171; //Just before #3 TDC
  state.toothAngles[5] = state.toothAngles[1] + 180;
  state.toothAngles[6] = state.toothAngles[2] + 180;
  state.toothAngles[7] = state.toothAngles[3] + 180;
  state.toothAngles[8] = state.toothAngles[4] + 180; //Just before #4 TDC
  state.toothAngles[9]  = state.toothAngles[1] + 360;
  state.toothAngles[10] = state.toothAngles[2] + 360;
  state.toothAngles[11] = state.toothAngles[3] + 360;
  state.toothAngles[12] = state.toothAngles[4] + 360; //Just before #2 TDC
  state.toothAngles[13] = state.toothAngles[1] + 540;
  state.toothAngles[14] = state.toothAngles[2] + 540;
  state.toothAngles[15] = state.toothAngles[3] + 540;

  return state;
}

}
}