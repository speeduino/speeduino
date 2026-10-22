#include "missingTooth.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"
#include "unit_testing.h"

namespace decoders {

namespace basic_distributor {

shared::state_t __attribute__((optimize("Os"))) initialise(const config2 &page2)
{
  shared::state_t state = {};

  state.decoderFeatures.hasFixedCrankingTiming = true;
  state.decoderStatus.toothAngleIsCorrect = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  state.triggerActualTeeth = page2.nCylinders;
  if(state.triggerActualTeeth == 0) { state.triggerActualTeeth = 1; }

  //The number of degrees that passes from tooth to tooth. Depends on number of cylinders and whether 4 or 2 stroke
  if(page2.strokes == FOUR_STROKE) { state.triggerToothAngle = 720U / state.triggerActualTeeth; }
  else { state.triggerToothAngle = 360U / state.triggerActualTeeth; }

  state.toothCurrentCount = 0; //Default value
  if(page2.nCylinders <= 4U) { state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/90U) * state.triggerToothAngle); }//Minimum 90rpm. (1851uS is the time per degree at 90rpm). This uses 90rpm rather than 50rpm due to the potentially very high stall time on a 4 cylinder if we wait that long.
  else { state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); } //Minimum 50rpm. (3200uS is the time per degree at 50rpm).

  return state;
}

}
}