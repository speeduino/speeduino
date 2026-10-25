#include "audi135.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"


namespace decoders {

namespace audi135 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state;

  state.triggerToothAngle = 8; //135/3 = 45, 360/45 = 8 degrees every 3 teeth
  state.toothCurrentCount = UINT8_MAX; //Default value
  state.triggerFilterTime = (unsigned long)(MICROS_PER_SEC / (MAX_RPM / 60U * 135UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = (int)(MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  state.decoderFeatures.supportsSequential = true;
  state.decoderStatus.toothAngleIsCorrect = true;

  return state;
}


void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4)
{
   decoderState.curGap = curTime - decoderState.toothSystemLastToothTime;
   if ( (decoderState.curGap > decoderState.triggerFilterTime) || (current.startRevolutions == 0) )
   {
     decoderState.toothSystemCount++;

     if ( decoderState.decoderStatus.syncStatus!=SyncStatus::Full ) { decoderState.toothLastToothTime = curTime; }
     else
     {
       if ( decoderState.toothSystemCount >= 3 )
       {
         //We only proceed for every third tooth

         decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
         decoderState.toothSystemLastToothTime = curTime;
         decoderState.toothSystemCount = 0;
         decoderState.toothCurrentCount++; //Increment the tooth counter

         if ( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount > 45) )
         {
           decoderState.toothCurrentCount = 1;
           decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
           decoderState.toothOneTime = curTime;
           decoderState.revolutionOne = !decoderState.revolutionOne;
           current.startRevolutions++; //Counter
         }

         decoderState.setFilter(decoderState.curGap, page4); //Recalc the new filter value

         decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
         decoderState.toothLastToothTime = curTime;
       } //3rd tooth check
     } // Sync check
   } // Trigger filter
}

void triggerSecondary(const statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  if( decoderState.decoderStatus.syncStatus!=SyncStatus::Full )
  {
    decoderState.toothCurrentCount = 0;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.toothSystemCount = 3; //Need to set this to 3 so that the next primary tooth is counted
  }
  else if (page4.useResync == 1) { decoderState.toothCurrentCount = 0; decoderState.toothSystemCount = 3; }
  else if ( (current.startRevolutions < 100) && (decoderState.toothCurrentCount != 45) ) { decoderState.toothCurrentCount = 0; }
  decoderState.revolutionOne = 1; //Sequential revolution reset
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  auto calculator = decoders::shared::atomic_make_angle_caa(decoderState);

  //Handle case where the secondary tooth was the last one seen
  if(calculator._toothCurrentCount == 0) { calculator._toothCurrentCount = 45; }

  return decoders::shared::clampCrankAngle(calculator.calculateCrankAngle(currMicros, page4));
}

}
}