#include "hondad17.h"
// #include "shared/perToothIgnition.h"
// #include "shared/rev_time_calcs.h"
// #include "shared/crank_angle_calculator.h"
// #include "scheduler_ignition_controller.h"
// #include "unit_testing.h"

namespace decoders {

namespace hondad17 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state;
  state.triggerToothAngle = 360 / 12; //The number of degrees that passes from tooth to tooth
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

}
}