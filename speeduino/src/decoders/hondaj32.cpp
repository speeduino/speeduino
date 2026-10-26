#include "hondaj32.h"
#include "shared/crank_angle_calculator.h"
#include "shared/rev_time_calcs.h"

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

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState)
{
  // This function is called only on rising edges, which occur as we lose sight of a tooth.
  // This function sets the following state variables for use in other functions:
  // decoderState.toothLastToothTime, decoderState.toothOneTime, decoderState.revolutionOne (just toggles - not correct)
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  decoderState.toothLastToothTime = curTime;

  decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
  
  if (decoderState.decoderStatus.syncStatus==SyncStatus::Full) // We have sync
  {
    decoderState.toothCurrentCount++;

    if (decoderState.toothCurrentCount == 25) { // handle rollover.  Normal sized tooth here
      decoderState.toothCurrentCount = 1;
      decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
      decoderState.toothOneTime = curTime;
      current.startRevolutions++;
    }
    else if (decoderState.toothCurrentCount == 23 || decoderState.toothCurrentCount == 15) // This is the first tooth after a missing tooth
    {
      decoderState.toothCurrentCount++; // account for missing tooth
      if (decoderState.curGap < (decoderState.lastGap >> 1) * 3) // This should be a big gap, if we find it's not actually big, we lost sync
      {
        decoderState.decoderStatus.syncStatus = SyncStatus::None;
        decoderState.toothCurrentCount=1;
      }
    }
    else if (decoderState.toothCurrentCount != 14 && decoderState.toothCurrentCount != 22)
    {
      // Teeth 14 and 22 are about 18 rather than 15 degrees so don't update last_gap with this unusual spacing
      decoderState.lastGap = decoderState.curGap;
    }
    // else decoderState.toothCurrentCount == 14 or 22.  Take no further action. 
  }
  else // we do not have sync yet. While syncing, treat tooth 14 and 22 as normal teeth
  {
    if (decoderState.curGap < (decoderState.lastGap >> 1) * 3 || decoderState.lastGap == 0){ // Regular tooth, decoderState.lastGap == 0 at startup
      decoderState.toothCurrentCount++;  // Increment teeth between gaps
      decoderState.lastGap = decoderState.curGap;
    }
    else { // First tooth after the missing tooth
      if (decoderState.toothCurrentCount == 15) {  // 15 teeth since the gap before this, meaning we just passed the second gap and are synced
        decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        decoderState.toothCurrentCount = 16;  // This so happens to be the tooth number of the first tooth in the string of 7 (where we are now)
        decoderState.toothOneTime = curTime - (15 * decoderState.lastGap); // Initialize tooth 1 times based on last gap width.
        decoderState.toothOneMinusOneTime = decoderState.toothOneTime - (24 * decoderState.lastGap);
      }
      else{ // Unclear which gap we just passed. reset counter
        decoderState.toothCurrentCount = 1;
      }
    }
  }
}

uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState)
{
  // The tooth #1 times are only updated while we have sync. They are estimated from the tooth gap when sync is first gained,
  // so this doesn't have to wait for a full revolution (as stdGetRevolutionTime(currentStatus, _decoderState, ) would)
  auto sample = atomicToothSpeedSample(current, decoderState);
  uint32_t interval = 0U;
  if ((sample.syncStatus==SyncStatus::None) || (toothOneInterval(sample, interval)==false)) { return shared::publishedRevolutionTime(current); }
  return interval;
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  auto calculator = decoders::shared::atomic_make_angle_caa(decoderState);  

  // Tooth 1 time occurs 360/24 degrees after TDC.
  ++calculator._toothCurrentCount;
  int16_t crankAngle = calculator.calculateCrankAngle(currMicros, page4);

  // Teeth 14 and 22 are unusually sized (18 degrees), but the missing tooth is smaller (12 degrees), 
  // so this oddity only applies when _decoderState.toothCurrentCount = 14 || 22 (15 || 23 since we incremented _decoderState.toothCurrentCount above)
  if ((calculator._toothCurrentCount == 15) || (calculator._toothCurrentCount == 23))
  {
    constexpr uint16_t HALF_TOOTH_WIDTH_DEG = (18U-12U)/2U;
    crankAngle += HALF_TOOTH_WIDTH_DEG;
  }
  return decoders::shared::clampCrankAngle(crankAngle);
}

}
}