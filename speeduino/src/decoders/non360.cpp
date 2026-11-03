#include "non360.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"

namespace decoders {

namespace non360 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config4 &page4)
{
  shared::state_t state = {};

  state.triggerToothAngle = (360U * page4.TrigAngMul) / page4.triggerTeeth; //The number of degrees that passes from tooth to tooth multiplied by the additional multiplier
  state.toothCurrentCount = UINT8_MAX; //Default value
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * page4.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  state.decoderFeatures.supportsSequential = true;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState, const config4 &page4)
{
  if ( (decoderState.decoderStatus.syncStatus==SyncStatus::Full) && (decoderState.toothCurrentCount != 0) )
  {
    if (current.RPM < current.crankRPM) { return crankingGetRevolutionTime(current, decoderState, page4, page4.triggerTeeth, CRANK_SPEED); }
    return stdGetRevolutionTime(current, decoderState, CRANK_SPEED);
  }
  return 0U; //Speed is unknown without sync
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  auto data = atomic_copy(decoderState.toothLastToothTime, decoderState.revolutionOne, decoderState.toothCurrentCount, decoderState.triggerToothAngle);

  //Handle case where the secondary tooth was the last one seen
  uint16_t toothCount = std::get<2>(data);
  if(toothCount == 0)
  { 
    toothCount = page4.triggerTeeth; 
  }

  //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
  //Have to divide by the multiplier to get back to actual crank angle.
  int16_t crankAngle = ((toothCount - 1) * std::get<3>(data)) / page4.TrigAngMul;

  decoders::shared::simple_crank_angle_calculator_t calculator;
  std::tie(calculator._toothLastToothTime, calculator._revZeroOrOne, std::ignore, std::ignore) = data;
  return decoders::shared::clampCrankAngle(calculator.calculateCrankAngle(crankAngle, currMicros, page4));
}

}
}