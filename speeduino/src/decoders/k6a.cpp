#include "k6a.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"

namespace decoders {

namespace k6a {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current, config4 &page4)
{
  shared::state_t state = {};
  state.triggerToothAngle = 90; //The number of degrees that passes from tooth to tooth (primary) - set to a value, needs to be set per tooth
  state.toothCurrentCount = 99; //Fake tooth count represents no sync

  page4.TrigSpeed = CAM_SPEED;
  state.triggerActualTeeth = 7;
  state.toothCurrentCount = 1;

  if(current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time

  // based on data in msextra page linked to above we can deduce,
  // gap between rising and falling edge of a normal 70 degree tooth is 48 degrees, this means the gap is 70 degrees - 48 degrees = 22 degrees.
  // assume this is constant for all similar sized gaps and teeth
  // sync tooth is 35 degrees - eyeball looks like the tooth is 50% tooth and 50% gap so guess its 17 degrees and 18 degrees.

  // coded every tooth here in case you want to try "change" setting on the trigger setup (this is defined in init.ino and what i've set it to, otherwise you need code to select rising or falling in init.ino (steal it from another trigger)). 
  // If you don't want change then drop the 'falling' edges listed below and half the number of edges + reduce the state.triggerActualTeeth
  // nb as you can edit the trigger offset using rising or falling edge setup below is irrelevant as you can adjust via the trigger offset to cover the difference.

  // not using state.toothAngles[0] as i'm hoping it makes logic easier

  state.toothAngles[0] = -70;                 // Wrap around to 650, 
  state.toothAngles[1] = 0;                   // 0 TDC cylinder 1, 
  state.toothAngles[2] = 170;                 // 170 - end of cylinder 1, start of cylinder 3, trigger ignition for cylinder 3 on this tooth
  state.toothAngles[3] = 240;                 // 70 TDC cylinder 3
  state.toothAngles[4] = 410;                 // 170  - end of cylinder 3, start of cylinder2, trigger ignition for cylinder 2 on this tooth
  state.toothAngles[5] = 480;                 // 70 TDC cylinder 2 
  state.toothAngles[6] = 515;                 // 35 Additional sync tooth
  state.toothAngles[7] = 650;                 // 135 end of cylinder 2, start of cylinder 1, trigger ignition for cylinder 1 on this tooth
  state.toothAngles[8] = 720;                 // 70 - gap to rotation to TDC1. array item 1 and 8 are the same, code never gets here its for reference only


  state.MAX_STALL_TIME = (3333UL * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  state.triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  state.triggerSecFilterTime = 0; //Need to figure out something better for this
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  return state;
}


}
}