#include "4g63.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"
#include "unit_testing.h"

namespace decoders {

namespace fourg63 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current, const config2 &page2)
{
    shared::state_t state;

  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.hasFixedCrankingTiming = true;
  state.decoderStatus.toothAngleIsCorrect = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  state.triggerToothAngle = 180; //The number of degrees that passes from tooth to tooth (primary)
  state.toothCurrentCount = 99; //Fake tooth count represents no sync
  state.MAX_STALL_TIME = 366667UL; //Minimum 50rpm based on the 110 degree tooth spacing
  if(!current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time

  //Note that these angles are for every rising and falling edge
  if(page2.nCylinders == 6)
  {
    //New values below
    state.toothAngles[0] = 715; //Rising edge of tooth #1
    state.toothAngles[1] = 45;  //Falling edge of tooth #1
    state.toothAngles[2] = 115; //Rising edge of tooth #2
    state.toothAngles[3] = 165; //Falling edge of tooth #2
    state.toothAngles[4] = 235; //Rising edge of tooth #3
    state.toothAngles[5] = 285; //Falling edge of tooth #3

    state.toothAngles[6] = 355; //Rising edge of tooth #4
    state.toothAngles[7] = 405; //Falling edge of tooth #4
    state.toothAngles[8] = 475; //Rising edge of tooth #5
    state.toothAngles[9] = 525; //Falling edge of tooth $5
    state.toothAngles[10] = 595; //Rising edge of tooth #6
    state.toothAngles[11] = 645; //Falling edge of tooth #6

    state.triggerActualTeeth = 12; //Both sides of all teeth over 720 degrees
  }
  else
  {
    // 70 / 110 for 4 cylinder
    state.toothAngles[0] = 715; //Falling edge of tooth #1
    state.toothAngles[1] = 105; //Rising edge of tooth #2
    state.toothAngles[2] = 175; //Falling edge of tooth #2
    state.toothAngles[3] = 285; //Rising edge of tooth #1

    state.toothAngles[4] = 355; //Falling edge of tooth #1
    state.toothAngles[5] = 465; //Rising edge of tooth #2
    state.toothAngles[6] = 535; //Falling edge of tooth #2
    state.toothAngles[7] = 645; //Rising edge of tooth #1

    state.triggerActualTeeth = 8;
  }

  state.triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  state.triggerSecFilterTime = (int)(MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)

  return state;
}

}
}