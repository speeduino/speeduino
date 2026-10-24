#include "gm7x.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"
#include "unit_testing.h"

namespace decoders {

namespace gm24x {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current)
{
    shared::state_t state;

  state.triggerToothAngle = 15; //The number of degrees that passes from tooth to tooth (primary)
  state.toothAngles[0] = 12;
  state.toothAngles[1] = 18;
  state.toothAngles[2] = 33;
  state.toothAngles[3] = 48;
  state.toothAngles[4] = 63;
  state.toothAngles[5] = 78;
  state.toothAngles[6] = 102;
  state.toothAngles[7] = 108;
  state.toothAngles[8] = 123;
  state.toothAngles[9] = 138;
  state.toothAngles[10] = 162;
  state.toothAngles[11] = 177;
  state.toothAngles[12] = 183;
  state.toothAngles[13] = 198;
  state.toothAngles[14] = 222;
  state.toothAngles[15] = 237;
  state.toothAngles[16] = 252;
  state.toothAngles[17] = 258;
  state.toothAngles[18] = 282;
  state.toothAngles[19] = 288;
  state.toothAngles[20] = 312;
  state.toothAngles[21] = 327;
  state.toothAngles[22] = 342;
  state.toothAngles[23] = 357;

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  if(current.initialisationComplete == false) { state.toothCurrentCount = 25; state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the init check to prevent the fuel pump just staying on all the time
  state.decoderFeatures.supportsSequential = true;
  state.decoderStatus.toothAngleIsCorrect = true;

  return state;

}

}
}