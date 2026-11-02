#include "ngc.h"

namespace decoders {

namespace ngc {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config2 &page2, config4 &page4)
{
  shared::state_t state = {};
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  //Primary trigger
  page4.triggerTeeth = 36; //The number of teeth on the wheel incl missing teeth.
  state.setTriggerToothAngle(10);  //The number of degrees that passes from tooth to tooth
  state.triggerFilterTime = MICROS_PER_SEC / (MAX_RPM/60U) / (360U/state.triggerToothAngle); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle * 2U ); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  //Secondary trigger
  if (page2.nCylinders == 4) {
    state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM/60U) / (360U/36U) * 2U); //Two nearest edges are 36 degrees apart. Multiply by 2 for half cam speed.
  } else {
    state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM/60U) / (360U/21U) * 2U); //Two nearest edges are 21 degrees apart. Multiply by 2 for half cam speed.
  }

  //state.toothAngles is reused to store the cam pattern, only used for 6 and 8 cylinder pattern
  if (page2.nCylinders == 6) {
    state.toothAngles[0] = 1; // Pos 0 is required to be the same as group 6 for easier math
    state.toothAngles[1] = 3; // Group 1 ...
    state.toothAngles[2] = 1;
    state.toothAngles[3] = 2;
    state.toothAngles[4] = 3;
    state.toothAngles[5] = 2;
    state.toothAngles[6] = 1;
    state.toothAngles[7] = 3; // Pos 7 is required to be the same as group 1 for easier math
  }
  else if (page2.nCylinders == 8) {
    state.toothAngles[0] = 3; // Pos 0 is required to be the same as group 8 for easier math
    state.toothAngles[1] = 1; // Group 1 ...
    state.toothAngles[2] = 1;
    state.toothAngles[3] = 2;
    state.toothAngles[4] = 3;
    state.toothAngles[5] = 2;
    state.toothAngles[6] = 2;
    state.toothAngles[7] = 1;
    state.toothAngles[8] = 3;
    state.toothAngles[9] = 1; // Pos 9 is required to be the same as group 1 for easier math
  } else {
    // 4 cylinder - do nothing
  }

  return state;
}

}
}