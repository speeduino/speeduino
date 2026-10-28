#include "nissan360.h"
#include "shared/perToothIgnition.h"

namespace decoders {

namespace nissan360 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(void)
{
  shared::state_t state = {};

  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 360UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.toothCurrentCount = 1;
  state.triggerToothAngle = 2;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( decoderState.curGap < decoderState.triggerFilterTime ) { return; }

  decoderState.toothCurrentCount++; //Increment the tooth counter
  decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

  decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
  decoderState.toothLastToothTime = curTime;

  if ( decoderState.decoderStatus.syncStatus==SyncStatus::Full )
  {
    if ( decoderState.toothCurrentCount == 361 ) //2 complete crank revolutions
    {
      decoderState.toothCurrentCount = 1;
      decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
      decoderState.toothOneTime = curTime;
      current.startRevolutions++; //Counter
    }
    //Recalc the new filter value
    decoderState.setFilter(decoderState.curGap, page4);

    //EXPERIMENTAL!
    if(page2.perToothIgn == true)
    {
      int16_t crankAngle = ( (decoderState.toothCurrentCount-1) * 2 ) + page4.triggerAngle;
      if(crankAngle > CRANK_ANGLE_MAX_IGN) 
      { 
        crankAngle -= CRANK_ANGLE_MAX_IGN;
        checkPerToothTiming(current, decoderState, page4, crankAngle, (decoderState.toothCurrentCount/2) );
      }
      else
      {
        checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount);
      }
    }
  }
}

void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;
  decoderState.toothLastSecToothTime = curTime;

  //Calculate number of primary teeth that this window has been active for
  if( (decoderState.secondaryToothCount == 0) || (current.decoder.secondary.isTriggered()) ) { decoderState.secondaryToothCount = decoderState.toothCurrentCount; } //This occurs on the first rotation upon powerup OR the start of a secondary window
  else
  {
    //If we reach here, we are at the end of a secondary window
    byte secondaryDuration = decoderState.toothCurrentCount - decoderState.secondaryToothCount; //How many primary teeth have passed during the duration of this secondary window

    if(decoderState.decoderStatus.syncStatus!=SyncStatus::Full)
    {
      if(page2.nCylinders == 4)
      {
        //Supported pattern is where all the inner windows as a different size (Most SR engines)
        //These equate to 4,8,12,16 teeth spacings
        if( (secondaryDuration >= 15) && (secondaryDuration <= 17) ) //Duration of window = 16 primary teeth
        {
          decoderState.toothCurrentCount = 16; //End of first window (The longest) occurs 16 teeth after TDC
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
        else if( (secondaryDuration >= 11) && (secondaryDuration <= 13) ) //Duration of window = 12 primary teeth
        {
          decoderState.toothCurrentCount = 102; //End of second window is after 90+12 primary teeth
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
        else if( (secondaryDuration >= 7) && (secondaryDuration <= 9) ) //Duration of window = 8 primary teeth
        {
          decoderState.toothCurrentCount = 188; //End of third window is after 90+90+8 primary teeth
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
        else if( (secondaryDuration >= 3) && (secondaryDuration <= 5) ) //Duration of window = 4 primary teeth
        {
          decoderState.toothCurrentCount = 274; //End of fourth window is after 90+90+90+4 primary teeth
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
        else { decoderState.decoderStatus.syncStatus = SyncStatus::None; current.syncLossCounter++; } //This should really never happen
      }
      else if(page2.nCylinders == 6)
      {
        //Pattern on the 6 cylinders is 4-8-12-16-20-24
        if( (secondaryDuration >= 3) && (secondaryDuration <= 5) ) //Duration of window = 4 primary teeth
        {
          decoderState.toothCurrentCount = 124; //End of smallest window is after 60+60+4 primary teeth
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
      }
      else if(page2.nCylinders == 8)
      {
        //V8 Optispark
        //Pattern on the 8 cylinders is the same as the 6 cylinder 4-8-12-16-20-24
        if( (secondaryDuration >= 6) && (secondaryDuration <= 8) ) //Duration of window = 16 primary teeth
        {
          decoderState.toothCurrentCount = 56; //End of the shortest of the individual windows. Occurs at 102 crank degrees. 
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
      }
      else { decoderState.decoderStatus.syncStatus = SyncStatus::None; } //This should really never happen (Only 4, 6 and 8 cylinder engines for this pattern)
    }
    else
    {
      if (page4.useResync == true)
      {
        //Already have sync, but do a verify every 720 degrees.
        if(page2.nCylinders == 4)
        {
          if( (secondaryDuration >= 15) && (secondaryDuration <= 17) ) //Duration of window = 16 primary teeth
          {
            decoderState.toothCurrentCount = 16; //End of first window (The longest) occurs 16 teeth after TDC
          }
        }
        else if(page2.nCylinders == 6)
        {
          if( (secondaryDuration >= 3) && (secondaryDuration <= 5) ) //Duration of window = 4 primary teeth
          {
            decoderState.toothCurrentCount = 124; //End of smallest window is after 60+60+4 primary teeth
          }
        } //Cylinder count
      } //use resync
    } //Has sync
  } //First getting sync or not
}

}
}