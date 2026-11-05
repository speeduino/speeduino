#include "fordtfi.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"

namespace decoders {

namespace ford_tfi {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config2 &page2)
{
  shared::state_t state = {};

  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  state.decoderStatus.toothAngleIsCorrect = true; //This is always true for this pattern

  state.triggerActualTeeth = page2.nCylinders;
  if(state.triggerActualTeeth == 0) { state.triggerActualTeeth = 1; }

  state.setTriggerToothAngle(720U / state.triggerActualTeeth); //The number of degrees that passes from tooth to tooth, half cylinder count
  state.toothCurrentCount = 0; //Default value
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 30U * state.triggerActualTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.triggerSecFilterTime = state.triggerFilterTime * 4U /5u; //Same as above, but slightly about lower due to signature trigger (about 80%)
  state.lastSyncRevolution = 0;
  if(page2.nCylinders <= 4U) { state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/90U) * state.triggerToothAngle); }//Minimum 90rpm. (1851uS is the time per degree at 90rpm). This uses 90rpm rather than 50rpm due to the potentially very high stall time on a 4 cylinder if we wait that long.
  else { state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); } //Minimum 50rpm. (3200uS is the time per degree at 50rpm).

  return state;
}

// Primary (Rising Edge)
void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  if (curTime >= decoderState.toothLastToothTime) 
    { decoderState.curGap = curTime - decoderState.toothLastToothTime; } 
  else
    { decoderState.curGap = (4294967296 - decoderState.toothLastToothTime + curTime); }
 
  if ( decoderState.curGap >= decoderState.triggerFilterTime )
  {
    if(decoderState.decoderStatus.syncStatus==SyncStatus::Full) { decoderState.setFilter(decoderState.curGap, page4); } //Recalc the new filter value
    else { decoderState.triggerFilterTime = 0; } //If we don't yet have sync, ensure that the filter won't prevent future valid pulses from being ignored
    
    decoderState.toothCurrentCount++; //Increment the tooth counter
    
    if(decoderState.toothCurrentCount > decoderState.triggerActualTeeth)//Check if we're back to the beginning of a revolution
    {
      if ( (decoderState.decoderStatus.syncStatus==SyncStatus::Full)  && ( (decoderState.lastSyncRevolution) + 3  < current.startRevolutions)) // Revolution count when signature tooth was detected, Allow up to 4 cam revolution without sync signal detected
      {
        decoderState.decoderStatus.syncStatus = SyncStatus::None;
        current.syncLossCounter++;
      }
      decoderState.toothCurrentCount = 1; //Reset the counter
      decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
      decoderState.toothOneTime = curTime;
      current.startRevolutions++; //Counter
    }

    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

    if(page2.perToothIgn == true)
    {
      int16_t crankAngle = ( (decoderState.toothCurrentCount-1) * decoderState.triggerToothAngle ) + page4.triggerAngle;
      uint16_t currentTooth = decoderState.toothCurrentCount;
      if(decoderState.toothCurrentCount > (decoderState.triggerActualTeeth/2) ) { currentTooth = (decoderState.toothCurrentCount - (decoderState.triggerActualTeeth/2)); }
      checkPerToothTiming(current, decoderState, page4, crankAngle, currentTooth);
    }
        
    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    decoderState.toothLastToothTime = curTime;

   } //Trigger filter
}

// Ford TFI Secondary (Falling Edge).
void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  if (curTime >= decoderState.toothLastSecToothTime) 
    { decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime; } 
  else
    { decoderState.curGap2 = (4294967296 - decoderState.toothLastSecToothTime + curTime); }
  
 
  //Safety check for initial startup
  if( (decoderState.toothLastSecToothTime == 0) )
  { 
    decoderState.curGap2 = 0; 
    decoderState.toothLastSecToothTime = curTime;
  }
  

  if ( decoderState.curGap2 >= decoderState.triggerSecFilterTime ) // Valid tooth falling edge
  {     
    if ((decoderState.curGap > 0) && (decoderState.curGap < 20000000)) // Limit to prevent overflow
    {  
      decoderState.targetGap2 = percentage(110, decoderState.curGap); // Wide last teeth gap min
      decoderState.targetGap3 = percentage(90, decoderState.curGap); // Narrow last teeth minus one gap max
    }
    else 
    {
    decoderState.targetGap2 = 0;
    decoderState.targetGap3 = 0;
    }
    if ( (decoderState.curGap2 > decoderState.targetGap2) && (decoderState.curGap3 < decoderState.targetGap3) && (decoderState.lastGap < decoderState.targetGap2) && (decoderState.lastGap > decoderState.targetGap3) ) // Signature Tooth detected
    {
      if( (decoderState.decoderStatus.syncStatus!=SyncStatus::Full) || (current.startRevolutions <= page4.StgCycles) )
      {
        decoderState.toothCurrentCount = 2; // Last primary tooth was #2
        decoderState.triggerFilterTime = 0; //Need to turn the filter off here otherwise the first primary tooth after achieving sync is ignored
        decoderState.decoderStatus.syncStatus = SyncStatus::Full;
      }
      else 
      {
        if ( (decoderState.toothCurrentCount != 2) && (current.startRevolutions > 2)) 
        { 
          current.syncLossCounter++;
        } //Indicates likely sync loss.
        if (page4.useResync == 1) 
        { 
          decoderState.toothCurrentCount = 2;
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;  
        }
      }
      decoderState.lastSyncRevolution = current.startRevolutions ; // Revolution count when signature tooth was detected

    }

  decoderState.toothLastSecToothTime = curTime; // 
  decoderState.lastGap = decoderState.curGap3; // Minus two Gap
  decoderState.curGap3 = decoderState.curGap2; // Minus one Gap

  } //Trigger filter
  //else { current.syncLossCounter = 0; }
  decoderState.triggerSecFilterTime = decoderState.curGap >> 1; //Set filter at 50 % speed of last primary gap
}

uint32_t getRevolutionTime(const statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  const uint8_t distributorSpeed = CAM_SPEED;

  const bool useCrankingCalc = (current.RPM < current.crankRPM) || (current.RPM < 1500U);
  const uint32_t revolutionTime = useCrankingCalc ? crankingGetRevolutionTime(current, decoderState, page4, decoderState.triggerActualTeeth, distributorSpeed)
                                                  : stdGetRevolutionTime(current, decoderState, distributorSpeed);

  decoderState.MAX_STALL_TIME = revolutionTime << 1; //Set the stall time to be twice the current RPM. This is a safe figure as there should be no single revolution where this changes more than this
  if(decoderState.MAX_STALL_TIME < 366667UL) { decoderState.MAX_STALL_TIME = 366667UL; } //Check for 50rpm minimum

  return revolutionTime;
}

}
}