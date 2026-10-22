#include "missingTooth.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"
#include "unit_testing.h"

namespace decoders {

namespace basic_distributor {

shared::state_t __attribute__((optimize("Os"))) initialise(const config2 &page2)
{
  shared::state_t state = {};

  state.decoderFeatures.hasFixedCrankingTiming = true;
  state.decoderStatus.toothAngleIsCorrect = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  state.triggerActualTeeth = page2.nCylinders;
  if(state.triggerActualTeeth == 0) { state.triggerActualTeeth = 1; }

  //The number of degrees that passes from tooth to tooth. Depends on number of cylinders and whether 4 or 2 stroke
  if(page2.strokes == FOUR_STROKE) { state.triggerToothAngle = 720U / state.triggerActualTeeth; }
  else { state.triggerToothAngle = 360U / state.triggerActualTeeth; }

  state.toothCurrentCount = 0; //Default value
  if(page2.nCylinders <= 4U) { state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/90U) * state.triggerToothAngle); }//Minimum 90rpm. (1851uS is the time per degree at 90rpm). This uses 90rpm rather than 50rpm due to the potentially very high stall time on a 4 cylinder if we wait that long.
  else { state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); } //Minimum 50rpm. (3200uS is the time per degree at 50rpm).

  return state;
}


void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( (decoderState.curGap >= decoderState.triggerFilterTime) )
  {
    if(decoderState.decoderStatus.syncStatus==SyncStatus::Full) { decoderState.setFilter(decoderState.curGap, page4); } //Recalc the new filter value
    else { decoderState.triggerFilterTime = 0; } //If we don't yet have sync, ensure that the filter won't prevent future valid pulses from being ignored. 
    
    if( (decoderState.toothCurrentCount == decoderState.triggerActualTeeth) || (decoderState.decoderStatus.syncStatus!=SyncStatus::Full) ) //Check if we're back to the beginning of a revolution
    {
      decoderState.toothCurrentCount = 1; //Reset the counter
      decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
      decoderState.toothOneTime = curTime;
      decoderState.decoderStatus.syncStatus = SyncStatus::Full;
      current.startRevolutions++; //Counter
    }
    else
    {
      if( (decoderState.toothCurrentCount < decoderState.triggerActualTeeth) ) { decoderState.toothCurrentCount++; } //Increment the tooth counter
      else
      {
        //This means decoderState.toothCurrentCount is greater than decoderState.triggerActualTeeth, which is bad.
        //If we have sync here then there's a problem. Throw a sync loss
        if( decoderState.decoderStatus.syncStatus==SyncStatus::Full ) 
        { 
          current.syncLossCounter++;
          decoderState.decoderStatus.syncStatus = SyncStatus::None;
        }
      }
    }

    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

    if ( page4.ignCranklock && (current.rotationStatus==EngineRotationStatus::Cranking) )
    {
      stopAllCoilsCharging();
    }

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

uint32_t getRevolutionTime(const statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  uint8_t distributorSpeed = CAM_SPEED; //Default to cam speed
  if(page2.strokes == TWO_STROKE) { distributorSpeed = CRANK_SPEED; } //For 2 stroke distributors, the tooth rate is based on crank speed, not 'cam'

  const bool useCrankingCalc = (current.RPM < current.crankRPM) || (current.RPM < 1500U);
  const uint32_t revolutionTime = useCrankingCalc ? crankingGetRevolutionTime(current, decoderState, page4, decoderState.triggerActualTeeth, distributorSpeed)
                                                  : stdGetRevolutionTime(current, decoderState, distributorSpeed);

  decoderState.MAX_STALL_TIME = revolutionTime << 1; //Set the stall time to be twice the current RPM. This is a safe figure as there should be no single revolution where this changes more than this
  if(decoderState.triggerActualTeeth == 1) { decoderState.MAX_STALL_TIME = revolutionTime << 1; } //Special case for 1 cylinder engines that only get 1 pulse every 720 degrees
  if(decoderState.MAX_STALL_TIME < 366667UL) { decoderState.MAX_STALL_TIME = 366667UL; } //Check for 50rpm minimum

  return revolutionTime;
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  auto data = atomic_copy(decoderState.toothLastToothTime, decoderState.toothLastMinusOneToothTime, decoderState.revolutionOne, decoderState.toothCurrentCount, decoderState.triggerToothAngle, decoderState.decoderStatus.toothAngleIsCorrect);
  return decoders::shared::clampCrankAngle(decoders::shared::compute_crank_angle_calculator_tooth_interval_t(data).calculateCrankAngle(currMicros, page4));
}


}
}