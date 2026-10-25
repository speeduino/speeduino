#include "hondaj32.h"
#include "shared/crank_angle_calculator.h"

namespace decoders {

namespace hondaj32 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state;

  state.triggerToothAngle = 360 / 24; //The number of degrees that passes from tooth to tooth
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/10U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  
  // Filter (ignore) triggers that are faster than this.
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60 * 24));
  state.toothCurrentCount = 0;
  state.toothOneTime = 0;
  state.toothOneMinusOneTime = 0;
  state.lastGap = 0;
  state.revolutionOne = 0;

  return state;
}

}
}