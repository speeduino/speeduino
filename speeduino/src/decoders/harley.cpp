#include "harley.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"

namespace decoders {

namespace harley {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current)
{
  shared::state_t state = {};

  state.triggerToothAngle = 0; // The number of degrees that passes from tooth to tooth, ev. 0. It alternates uneven
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * 60U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  if(current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time
  state.triggerFilterTime = 1500;

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  decoderState.lastGap = decoderState.curGap;
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  decoderState.setFilter(decoderState.curGap, page4); // Filtering adjusted according to setting
  if (decoderState.curGap > decoderState.triggerFilterTime)
  {
    if (current.decoder.primary.isPinHigh()) // Has to be the same as in main() trigger-attach, for readability we do it this way.
    {
        decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
        decoderState.targetGap = decoderState.lastGap ; //Gap is the Time to next toothtrigger, so we know where we are
        decoderState.toothCurrentCount++;
        if (decoderState.curGap > decoderState.targetGap)
        {
          decoderState.toothCurrentCount = 1;
          decoderState.triggerToothAngle = 0;// Has to be equal to Angle Routine
          decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
          decoderState.toothOneTime = curTime;
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
        else
        {
          decoderState.toothCurrentCount = 2;
          decoderState.triggerToothAngle = 157;
          //     decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
          //     decoderState.toothOneTime = curTime;
        }
        decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
        decoderState.toothLastToothTime = curTime;
        current.startRevolutions++; //Counter
    }
    else
    {
      if (decoderState.decoderStatus.syncStatus==SyncStatus::Full) { current.syncLossCounter++; }
      decoderState.decoderStatus.syncStatus = SyncStatus::None;
      decoderState.toothCurrentCount = 0;
    } //Primary trigger high
  } //Trigger filter
}

uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState)
{
  auto sample = atomicToothSpeedSample(current, decoderState);
  uint32_t revolutionTime = 0U;

  if (sample.syncStatus!=SyncStatus::Full) { return 0U; } //Speed is unknown without sync

  if (current.RPM < current.crankRPM)
  {
    //Tooth #1 sets triggerToothAngle to 0, so there is no per tooth calculation: keep the last speed until the next tooth
    if (revolutionTimeFromLastTooth(sample, revolutionTime)==true) { return revolutionTime; }
    return shared::publishedRevolutionTime(current);
  }
  return stdGetRevolutionTime(current, decoderState, CRANK_SPEED);
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  auto data = atomic_copy(decoderState.toothLastToothTime, decoderState.revolutionOne, decoderState.toothCurrentCount);

  int16_t crankAngle = 157;
  if ( (std::get<2>(data) == 1U) || (std::get<2>(data) == 3U) )
  {
    crankAngle = 0;
  }

  decoders::shared::simple_crank_angle_calculator_t calculator;
  std::tie(calculator._toothLastToothTime, calculator._revZeroOrOne, std::ignore) = data;
  return decoders::shared::clampCrankAngle(calculator.calculateCrankAngle(crankAngle, currMicros, page4));
}

}
}