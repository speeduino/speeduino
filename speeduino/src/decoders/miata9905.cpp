#include "miata9905.h"


namespace decoders {

namespace miata9905 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current)
{
  shared::state_t state = {};

  state.triggerToothAngle = 90; //The number of degrees that passes from tooth to tooth (primary)
  state.toothCurrentCount = 99; //Fake tooth count represents no sync
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.triggerActualTeeth = 8;

  if(current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time

  //Note that these angles are for every rising and falling edge
  state.toothAngles[0] = 710; //
  state.toothAngles[1] = 100; //First crank pulse after the SINGLE cam pulse
  state.toothAngles[2] = 170; //
  state.toothAngles[3] = 280; //
  state.toothAngles[4] = 350; //
  state.toothAngles[5] = 460; //First crank pulse AFTER the DOUBLE cam pulse
  state.toothAngles[6] = 530; //
  state.toothAngles[7] = 640; //

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  state.triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  state.triggerSecFilterTime = 0; //Need to figure out something better for this
  state.decoderFeatures.hasFixedCrankingTiming = true;
  state.decoderStatus.toothAngleIsCorrect = true;

  return state;
}

}
}