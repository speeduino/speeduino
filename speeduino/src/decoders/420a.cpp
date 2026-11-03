#include "420a.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"

namespace decoders {

namespace dsm420a {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state = {};

  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 360UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.toothCurrentCount = 1;
  state.triggerToothAngle = 20; //Is only correct for the 4 short pulses before each TDC
  state.decoderStatus.toothAngleIsCorrect = false;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * 93U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  state.toothAngles[0] = 711; //tooth #1, just before #1 TDC
  state.toothAngles[1] = 111;
  state.toothAngles[2] = 131;
  state.toothAngles[3] = 151;
  state.toothAngles[4] = 171; //Just before #3 TDC
  state.toothAngles[5] = state.toothAngles[1] + 180;
  state.toothAngles[6] = state.toothAngles[2] + 180;
  state.toothAngles[7] = state.toothAngles[3] + 180;
  state.toothAngles[8] = state.toothAngles[4] + 180; //Just before #4 TDC
  state.toothAngles[9]  = state.toothAngles[1] + 360;
  state.toothAngles[10] = state.toothAngles[2] + 360;
  state.toothAngles[11] = state.toothAngles[3] + 360;
  state.toothAngles[12] = state.toothAngles[4] + 360; //Just before #2 TDC
  state.toothAngles[13] = state.toothAngles[1] + 540;
  state.toothAngles[14] = state.toothAngles[2] + 540;
  state.toothAngles[15] = state.toothAngles[3] + 540;

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( decoderState.curGap >= decoderState.triggerFilterTime ) //Pulses should never be less than decoderState.triggerFilterTime, so if they are it means a false trigger. (A 36-1 wheel at 8000pm will have triggers approx. every 200uS)
  {
    decoderState.toothCurrentCount++; //Increment the tooth counter
    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

    if( (decoderState.toothLastToothTime == 0) || (decoderState.toothLastMinusOneToothTime == 0) ) { decoderState.curGap = 0; }

    if( (decoderState.toothCurrentCount > 16) && (decoderState.decoderStatus.syncStatus==SyncStatus::Full) )
    {
      //Means a complete rotation has occurred.
      decoderState.toothCurrentCount = 1;
      decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
      decoderState.toothOneTime = curTime;
      current.startRevolutions++; //Counter
    }

    //Filter can only be recalculated for the regular teeth, not the missing one.
    decoderState.triggerFilterTime = 0;

    decoderState.decoderStatus.toothAngleIsCorrect = false;

    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    decoderState.toothLastToothTime = curTime;

    //EXPERIMENTAL!
    if(page2.perToothIgn == true)
    {
      int16_t crankAngle = ( decoderState.toothAngles[decoderState.toothCurrentCount-1] ) + page4.triggerAngle;
      checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount);
    }
  }
}

void triggerSecondary(statuses &current, shared::state_t &decoderState)
{
  //Secondary trigger is only on falling edge

  if(current.decoder.primary.isPinHigh())
  {
    //Secondary signal is falling and primary signal is HIGH
    if( decoderState.decoderStatus.syncStatus!=SyncStatus::Full )
    {
      //If we don't have sync, then assume the signal is good
      decoderState.toothCurrentCount = 13;
      decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    }
    else
    {
      //If we DO have sync, then check that the tooth count matches what we expect
      if(decoderState.toothCurrentCount != 13)
      {
        current.syncLossCounter++;
        decoderState.toothCurrentCount = 13;
      }
    }
  }
  else
  {
    //Secondary signal is falling and primary signal is LOW
    if( decoderState.decoderStatus.syncStatus!=SyncStatus::Full )
    {
      //If we don't have sync, then assume the signal is good
      decoderState.toothCurrentCount = 5;
      decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    }
    else
    {
      //If we DO have sync, then check that the tooth count matches what we expect
      if(decoderState.toothCurrentCount != 5)
      {
        current.syncLossCounter++;
        decoderState.toothCurrentCount = 5;
      }
    }
  }
}

uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState)
{
  //Possibly look at doing special handling for cranking in the future, but for now just use the standard method
  return stdGetRevolutionTime(current, decoderState, CAM_SPEED);
}

void setEndTeeth(const statuses &current, shared::state_t &decoderState)
{
  if(current.advance < 9)
  {
    decoderState.ignitionEndTeeth[0] = 1;
    decoderState.ignitionEndTeeth[1] = 5;
    decoderState.ignitionEndTeeth[2] = 9;
    decoderState.ignitionEndTeeth[3] = 13;  
  }
  else
  {
    decoderState.ignitionEndTeeth[0] = 16;
    decoderState.ignitionEndTeeth[1] = 4;
    decoderState.ignitionEndTeeth[2] = 8;
    decoderState.ignitionEndTeeth[3] = 12;  
  }
}


}
}