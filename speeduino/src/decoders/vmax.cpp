#include "vmax.h"

namespace decoders {

namespace vmax {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current)
{
  shared::state_t state = {};

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * 60U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  if(!current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time
  state.triggerFilterTime = 1500;
  state.decoderStatus.validTrigger = true; // We must start with a valid trigger or we cannot start measuring the lobe width. We only have a false trigger on the lobe up event when it doesn't pass the filter. Then, the lobe width will also not be beasured.
  state.toothAngles[1] = 0;      //tooth #1, these are the absolute tooth positions
  state.toothAngles[2] = 40;     //tooth #2
  state.toothAngles[3] = 110;    //tooth #3
  state.toothAngles[4] = 180;    //tooth #4
  state.toothAngles[5] = 220;    //tooth #5
  state.toothAngles[6] = 290;    //tooth #6

  return state;
}

}
}