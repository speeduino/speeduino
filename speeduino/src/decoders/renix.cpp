#include "vmax.h"
#include "shared/perToothIgnition.h"
#include "scheduler_ignition_controller.h"

namespace decoders {

namespace renix {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config2 &page2, config4 &page4)
{
  shared::state_t state = {};

  if( page2.nCylinders == 4)
  {
    state.setTriggerToothAngle(90); //The number of degrees that passes from tooth to tooth (primary) this changes between 41 and 49 degrees
    page4.triggerTeeth = 4; // wheel has 44 teeth but we use these to work out which tooth angle to use, therefore speeduino thinks we only have 8 teeth.
    page4.triggerMissingTeeth = 0;
    state.triggerActualTeeth = 4; //The number of teeth we're pretending physically existing on the wheel.
    state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 44U)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  }
  else if (page2.nCylinders == 6)
  {
    state.setTriggerToothAngle(60);
    page4.triggerTeeth = 6; // wheel has 44 teeth but we use these to work out which tooth angle to use, therefore speeduino thinks we only have 6 teeth.
    page4.triggerMissingTeeth = 0;
    state.triggerActualTeeth = 6; //The number of teeth we're pretending physically existing on the wheel.
    state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 66U)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  }
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm). Largest gap between teeth is 90 or 60 degrees depending on decoder.
  state.decoderFeatures.supportsPerToothIgnition = true;

  state.toothSystemCount = 1;
  state.toothCurrentCount = 1;

  return state;
}

// variables used to help calculate gap on the physical 44 or 66 teeth we're pretending don't exist in most of the speeduino code
// reusing existing variables to save storage space as these aren't used in the code for their original purpose.
#define renixSystemLastToothTime         decoderState.toothLastToothRisingTime
#define renixSystemLastMinusOneToothTime decoderState.toothLastSecToothRisingTime

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap = curTime - renixSystemLastToothTime;

  if ( decoderState.curGap >= decoderState.triggerFilterTime )   
  {
       
    decoderState.toothSystemCount++;

    if( renixSystemLastToothTime != 0 && renixSystemLastMinusOneToothTime != 0)
    { decoderState.targetGap = (2 * (renixSystemLastToothTime - renixSystemLastMinusOneToothTime));}  // in real world the physical 2 tooth gap is bigger than 2 teeth - more like 2.5
    else 
    { decoderState.targetGap = 100000000L; } // random large number to stop system thinking we have a gap for the first few teeth on start up

    if( decoderState.curGap >= decoderState.targetGap )
    { 
      /* add two teeth to account for the gap we've just seen */      
      decoderState.toothSystemCount++;
      decoderState.toothSystemCount++;
   
      if( decoderState.toothSystemCount != 12) // if not 12 (the first tooth after the gap) then we've lost sync
      {
        // lost sync
        decoderState.decoderStatus.syncStatus = SyncStatus::None;
        current.syncLossCounter++;            
        decoderState.toothSystemCount = 1; // first tooth after gap is always 1
        decoderState.toothCurrentCount = 1; // Reset as we've lost sync
      }
    }
    else
    { 
      //Recalc the new filter value, only do this on the single gap tooth 
      decoderState.setFilter(decoderState.curGap, page4);  
    }
    renixSystemLastMinusOneToothTime = renixSystemLastToothTime; // needed for target gap calculation
    renixSystemLastToothTime = curTime;

    if( decoderState.toothSystemCount == 12  || decoderState.toothLastToothTime == 0) // decoderState.toothLastToothTime used to ensure we set the value so the code that handles the fuel pump in speeduino.ini has a value to use once the engine is running.
    {
      decoderState.toothCurrentCount++;

      if( (page2.nCylinders == 6 && decoderState.toothCurrentCount == 7) ||    // 6 Pretend teeth on the 66 tooth wheel, if get to severn rotate round back to first tooth
          (page2.nCylinders == 4 && decoderState.toothCurrentCount == 5 ) )    // 4 Pretend teeth on the 44 tooth wheel, if get to five rotate round back to first tooth
      {
        decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
        decoderState.toothOneTime = curTime;
        decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        current.startRevolutions++; //Counter               
        decoderState.revolutionOne = !decoderState.revolutionOne;
        decoderState.toothCurrentCount = 1;  
      }

      decoderState.toothSystemCount = 1;
      decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
      decoderState.toothLastToothTime = curTime; 


      //NEW IGNITION MODE
      if( (page2.perToothIgn == true) && (current.rotationStatus!=EngineRotationStatus::Cranking) ) 
      {
        int16_t crankAngle = ( (decoderState.toothCurrentCount - 1) * decoderState.triggerToothAngle ) + page4.triggerAngle;
        if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (decoderState.revolutionOne == true) && (page4.TrigSpeed == CRANK_SPEED) )
        {
          crankAngle += 360;
          checkPerToothTiming(current, decoderState, page4, crankAngle, (page4.triggerTeeth + decoderState.toothCurrentCount)); 
        }
        else{ checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount); }
      }
    }
  } 
}

static uint16_t __attribute__((noinline)) calcEndTeeth_Renix(const IgnitionSchedule &schedule, uint8_t toothAdder, const shared::state_t &decoderState, const config4 &page4) {
  int16_t tempEndTooth = decoderState.toothNumFromScheduleAngles(page4, schedule) - 1;
  // Clamp to tooth count
  return decoderState.clampToActualTeeth(page4, decoders::shared::clampToToothCount(page4, tempEndTooth, toothAdder), toothAdder);
}

void setEndTeeth(shared::state_t &decoderState, const config4 &page4)
{
  byte toothAdder = 0;
  if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (page4.TrigSpeed == CRANK_SPEED) ) { toothAdder = page4.triggerTeeth; }

  decoderState.ignitionEndTeeth[0] = calcEndTeeth_Renix(ignitionSchedule1, toothAdder, decoderState, page4);
#if (IGN_CHANNELS >= 2)
  decoderState.ignitionEndTeeth[1] = calcEndTeeth_Renix(ignitionSchedule2, toothAdder, decoderState, page4);
#endif
#if (IGN_CHANNELS >= 3)
  decoderState.ignitionEndTeeth[2] = calcEndTeeth_Renix(ignitionSchedule3, toothAdder, decoderState, page4);
#endif
#if (IGN_CHANNELS >= 4)
  decoderState.ignitionEndTeeth[3] = calcEndTeeth_Renix(ignitionSchedule4, toothAdder, decoderState, page4);
#endif
#if IGN_CHANNELS >= 5
  decoderState.ignitionEndTeeth[4] = calcEndTeeth_Renix(ignitionSchedule5, toothAdder, decoderState, page4);
#endif
#if IGN_CHANNELS >= 6
  decoderState.ignitionEndTeeth[5] = calcEndTeeth_Renix(ignitionSchedule6, toothAdder, decoderState, page4);
#endif
#if IGN_CHANNELS >= 7
  decoderState.ignitionEndTeeth[6] = calcEndTeeth_Renix(ignitionSchedule7, toothAdder, decoderState, page4);
#endif
#if IGN_CHANNELS >= 8
  decoderState.ignitionEndTeeth[7] = calcEndTeeth_Renix(ignitionSchedule8, toothAdder, decoderState, page4);
#endif
}

}
}