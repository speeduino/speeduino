#include "st170.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"

namespace decoders {

namespace st170 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(config4 &page4)
{
  shared::state_t state = {};

  //Set these as we are using the existing missing tooth primary decoder and these will never change.
  page4.triggerTeeth = 36;  
  page4.triggerMissingTeeth = 1;
  page4.TrigSpeed = CRANK_SPEED;
  
  state.setTriggerToothAngle(360 / page4.triggerTeeth);
  state.triggerActualTeeth = page4.triggerTeeth - page4.triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * page4.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  
  state.triggerSecFilterTime = MICROS_PER_MIN / MAX_RPM / 8U / 2U; //Cam pattern is 8-3, so 2 nearest teeth are 90 deg crank angle apart. Cam can be advanced by 60 deg, so going from fully retarded to fully advanced closes the gap to 30 deg. Zetec cam pulleys aren't keyed from factory, so I subtracted additional 10 deg to avoid filter to be too aggressive. And there you have it 720/20=36.
  
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.checkSyncToothCount = 36 >> 1; //50% of the total teeth.
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle * (1U + 1U)); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

}
}