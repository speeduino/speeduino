#include "36minus222.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"

namespace decoders {

namespace thirtysixminus222 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config4 &page4)
{
  shared::state_t state = {};

  state.triggerToothAngle = 10; //The number of degrees that passes from tooth to tooth
  state.triggerActualTeeth = 30; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60UL * 36UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.checkSyncToothCount = page4.triggerTeeth >> 1; //50% of the total teeth.
  state.toothLastMinusOneToothTime = 0;
  state.toothCurrentCount = 0;
  state.toothOneTime = 0;
  state.toothOneMinusOneTime = 0;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle * 2U ); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
   decoderState.curGap = curTime - decoderState.toothLastToothTime;
   if ( decoderState.curGap >= decoderState.triggerFilterTime ) //Pulses should never be less than decoderState.triggerFilterTime, so if they are it means a false trigger. (A 36-1 wheel at 8000pm will have triggers approx. every 200uS)
   {
     decoderState.toothCurrentCount++; //Increment the tooth counter
     decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

     //Begin the missing tooth detection
     //If the time between the current tooth and the last is greater than 2x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after a gap
     //decoderState.toothSystemCount is used to keep track of which missed tooth we're on. It will be set to 1 if that last tooth seen was the middle one in the -2-2 area. At all other times it will be 0
     if(decoderState.toothSystemCount == 0) { decoderState.targetGap = ((decoderState.toothLastToothTime - decoderState.toothLastMinusOneToothTime)) * 2; } //Multiply by 2 (Checks for a gap 2x greater than the last one)


     if( (decoderState.toothLastToothTime == 0) || (decoderState.toothLastMinusOneToothTime == 0) ) { decoderState.curGap = 0; }

     if ( (decoderState.curGap > decoderState.targetGap) )
     {
       {
         if(decoderState.toothSystemCount == 1)
         {
           //This occurs when we're at the first tooth after the 2 lots of 2x missing tooth.
           if(page2.nCylinders == 4 ) { decoderState.toothCurrentCount = 19; } //H4
           else if(page2.nCylinders == 6) { decoderState.toothCurrentCount = 12; } //H6 - NOT TESTED!
           
           decoderState.toothSystemCount = 0;
           decoderState.decoderStatus.syncStatus = SyncStatus::Full;
         }
         else
         {
           //We've seen a missing tooth set, but do not yet know whether it is the single one or the double one.
           decoderState.toothSystemCount = 1;
           decoderState.toothCurrentCount++;
           decoderState.toothCurrentCount++; //Accurately reflect the actual tooth count, including the skipped ones
         }
         decoderState.decoderStatus.toothAngleIsCorrect = false; //The tooth angle is double at this point
         decoderState.triggerFilterTime = 0; //This is used to prevent a condition where serious intermittent signals (Eg someone furiously plugging the sensor wire in and out) can leave the filter in an unrecoverable state
       }
     }
     else
     {
       if(decoderState.toothCurrentCount > 36)
       {
         //Means a complete rotation has occurred.
         decoderState.toothCurrentCount = 1;
         decoderState.revolutionOne = !decoderState.revolutionOne; //Flip sequential revolution tracker
         decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
         decoderState.toothOneTime = curTime;
         current.startRevolutions++; //Counter

       }
       else if(decoderState.toothSystemCount == 1)
       {
          //This occurs when a set of missing teeth had been seen, but the next one was NOT missing.
          if(page2.nCylinders == 4 )
          { 
            //H4
            decoderState.toothCurrentCount = 35; 
            decoderState.decoderStatus.syncStatus = SyncStatus::Full;
          } 
          else if(page2.nCylinders == 6) 
          { 
            //H6 - THIS NEEDS TESTING
            decoderState.toothCurrentCount = 34; 
            decoderState.decoderStatus.syncStatus = SyncStatus::Full;
          } 
          
       }

       //Filter can only be recalculated for the regular teeth, not the missing one.
       decoderState.setFilter(decoderState.curGap, page4);

       decoderState.decoderStatus.toothAngleIsCorrect = true;
       decoderState.toothSystemCount = 0;
     }

     decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
     decoderState.toothLastToothTime = curTime;

     if(page2.perToothIgn == true)
     {
       int16_t crankAngle = ( (decoderState.toothCurrentCount-1) * decoderState.triggerToothAngle ) + page4.triggerAngle;
       checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount);
     }
   }
}

uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  uint32_t revolutionTime = 0;
  if( current.RPM < current.crankRPM)
  {
    if( (page2.nCylinders == 4) && (decoderState.toothCurrentCount != 19) && (decoderState.toothCurrentCount != 16) && (decoderState.toothCurrentCount != 34) && (decoderState.decoderStatus.toothAngleIsCorrect) )
    {
      revolutionTime = crankingGetRevolutionTime(current, decoderState, page4, 36, CRANK_SPEED);
    }
    else if( (page2.nCylinders == 6) && (decoderState.toothCurrentCount != 9) && (decoderState.toothCurrentCount != 12) && (decoderState.toothCurrentCount != 33) && (decoderState.decoderStatus.toothAngleIsCorrect) )
    {
      revolutionTime = crankingGetRevolutionTime(current, decoderState, page4, 36, CRANK_SPEED);
    }
    else { revolutionTime = shared::publishedRevolutionTime(current); } //Can't do per tooth calculation if we're at and of the missing teeth as it messes the calculation
  }
  else
  {
    revolutionTime = stdGetRevolutionTime(current, decoderState, CRANK_SPEED);
  }
  return revolutionTime;
}

}
}