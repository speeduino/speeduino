#include "4g63.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"
#include "scheduledIO_ign.h"
#include "unit_testing.h"

namespace decoders {

namespace fourg63 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current, const config2 &page2)
{
    shared::state_t state;

  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.hasFixedCrankingTiming = true;
  state.decoderStatus.toothAngleIsCorrect = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  state.triggerToothAngle = 180; //The number of degrees that passes from tooth to tooth (primary)
  state.toothCurrentCount = 99; //Fake tooth count represents no sync
  state.MAX_STALL_TIME = 366667UL; //Minimum 50rpm based on the 110 degree tooth spacing
  if(!current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time

  //Note that these angles are for every rising and falling edge
  if(page2.nCylinders == 6)
  {
    //New values below
    state.toothAngles[0] = 715; //Rising edge of tooth #1
    state.toothAngles[1] = 45;  //Falling edge of tooth #1
    state.toothAngles[2] = 115; //Rising edge of tooth #2
    state.toothAngles[3] = 165; //Falling edge of tooth #2
    state.toothAngles[4] = 235; //Rising edge of tooth #3
    state.toothAngles[5] = 285; //Falling edge of tooth #3

    state.toothAngles[6] = 355; //Rising edge of tooth #4
    state.toothAngles[7] = 405; //Falling edge of tooth #4
    state.toothAngles[8] = 475; //Rising edge of tooth #5
    state.toothAngles[9] = 525; //Falling edge of tooth $5
    state.toothAngles[10] = 595; //Rising edge of tooth #6
    state.toothAngles[11] = 645; //Falling edge of tooth #6

    state.triggerActualTeeth = 12; //Both sides of all teeth over 720 degrees
  }
  else
  {
    // 70 / 110 for 4 cylinder
    state.toothAngles[0] = 715; //Falling edge of tooth #1
    state.toothAngles[1] = 105; //Rising edge of tooth #2
    state.toothAngles[2] = 175; //Falling edge of tooth #2
    state.toothAngles[3] = 285; //Rising edge of tooth #1

    state.toothAngles[4] = 355; //Falling edge of tooth #1
    state.toothAngles[5] = 465; //Rising edge of tooth #2
    state.toothAngles[6] = 535; //Falling edge of tooth #2
    state.toothAngles[7] = 645; //Rising edge of tooth #1

    state.triggerActualTeeth = 8;
  }

  state.triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  state.triggerSecFilterTime = (int)(MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)

  return state;
}

TESTABLE_INLINE_STATIC bool is70DegTooth(uint16_t toothNum)
{
    return (toothNum == 1) 
        || (toothNum == 3)
        || (toothNum == 5)
        || (toothNum == 7)
        || (toothNum == 9)
        || (toothNum == 11);
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( (decoderState.curGap >= decoderState.triggerFilterTime) || (current.startRevolutions == 0) )
  {
    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
    decoderState.triggerFilterTime = decoderState.curGap >> 2; //This only applies during non-sync conditions. If there is sync then decoderState.triggerFilterTime gets changed again below with a better value.

    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    decoderState.toothLastToothTime = curTime;

    decoderState.toothCurrentCount++;

    if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount > decoderState.triggerActualTeeth) ) //Trigger is on CHANGE, hence 4 pulses = 1 crank rev (or 6 pulses for 6 cylinders)
    {
       decoderState.toothCurrentCount = 1; //Reset the counter
       decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
       decoderState.toothOneTime = curTime;
       current.startRevolutions++; //Counter
    }

    // LCOV_EXCL_START
    // Not testable ATM
    if (decoderState.decoderStatus.syncStatus==SyncStatus::Full)
    {
      if ( (current.rotationStatus==EngineRotationStatus::Cranking) && page4.ignCranklock && (current.startRevolutions >= page4.StgCycles))
      {
        if(page2.nCylinders == 4)
        {
          //This operates in forced wasted spark mode during cranking to align with crank teeth
          if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 5) ) { endCoilCharge(1U); endCoilCharge(3U); }
          else if( (decoderState.toothCurrentCount == 3) || (decoderState.toothCurrentCount == 7) ) { endCoilCharge(2U); endCoilCharge(4U); }
        }
        else if(page2.nCylinders == 6)
        {
          if( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount == 7) ) { endCoilCharge(1U); }
          else if( (decoderState.toothCurrentCount == 3) || (decoderState.toothCurrentCount == 9) ) { endCoilCharge(2U); }
          else if( (decoderState.toothCurrentCount == 5) || (decoderState.toothCurrentCount == 11) ) { endCoilCharge(3U); }
        }
      }
      // LCOV_EXCL_STOP

      //Whilst this is an uneven tooth pattern, if the specific angle between the last 2 teeth is specified, 1st deriv prediction can be used
      if( (page4.triggerFilter == TRIGGER_FILTER_LITE) || (current.RPM < 1400) )
      {
        //Lite filter
        if( is70DegTooth(decoderState.toothCurrentCount) )
        {
          decoderState.triggerToothAngle = 70;
          if(page2.nCylinders == 4)
          {
            decoderState.triggerFilterTime = decoderState.curGap; //Trigger filter is set to whatever time it took to do 70 degrees (Next trigger is 110 degrees away)
          }
          else if(page2.nCylinders == 6)
          {
            decoderState.triggerFilterTime = (decoderState.curGap >> 2); //Trigger filter is set to (70/4)=17.5=17 degrees (Next trigger is 50 degrees away).
          }
        }
        else
        {
          if(page2.nCylinders == 4)
          {
            decoderState.triggerToothAngle = 110;
            decoderState.triggerFilterTime = rshift<3>((uint32_t)(decoderState.curGap * 3UL)); //Trigger filter is set to (110*3)/8=41.25=41 degrees (Next trigger is 70 degrees away).
          }
          else if(page2.nCylinders == 6)
          {
            decoderState.triggerToothAngle = 50;
            decoderState.triggerFilterTime = decoderState.curGap >> 1; //Trigger filter is set to 25 degrees (Next trigger is 70 degrees away).
          }
        }
      }
      else if(page4.triggerFilter == TRIGGER_FILTER_MEDIUM)
      {
        //Medium filter level
        if( is70DegTooth(decoderState.toothCurrentCount) )
        { 
          decoderState.triggerToothAngle = 70; 
          if(page2.nCylinders == 4)
          { 
            decoderState.triggerFilterTime = (decoderState.curGap * 5) >> 2 ; //87.5 degrees with a target of 110
          }
          else
          {
            decoderState.triggerFilterTime = decoderState.curGap >> 1 ; //35 degrees with a target of 50
          }
        } 
        else 
        { 
          if(page2.nCylinders == 4)
          { 
            decoderState.triggerToothAngle = 110; 
            decoderState.triggerFilterTime = (decoderState.curGap >> 1); //55 degrees with a target of 70
          }
          else
          {
            decoderState.triggerToothAngle = 50; 
            decoderState.triggerFilterTime = (decoderState.curGap * 3) >> 2; //Trigger filter is set to (50*3)/4=37.5=37 degrees (Next trigger is 70 degrees away).
          }
        } 
      }
      else if (page4.triggerFilter == TRIGGER_FILTER_AGGRESSIVE)
      {
        //Aggressive filter level
        if( is70DegTooth(decoderState.toothCurrentCount) )
        { 
          decoderState.triggerToothAngle = 70; 
          if(page2.nCylinders == 4)
          { 
            decoderState.triggerFilterTime = rshift<3>((uint32_t)(decoderState.curGap * 11UL));//96.26 degrees with a target of 110
          }
          else
          {
            decoderState.triggerFilterTime = decoderState.curGap >> 1 ; //35 degrees with a target of 50
          }
        } 
        else 
        { 
          if(page2.nCylinders == 4)
          { 
            decoderState.triggerToothAngle = 110; 
            decoderState.triggerFilterTime = rshift<5>((uint32_t)(decoderState.curGap * 9UL)); //61.87 degrees with a target of 70
          }
          else
          {
            decoderState.triggerToothAngle = 50; 
            decoderState.triggerFilterTime = decoderState.curGap; //50 degrees with a target of 70
          }
        } 
      }
      else
      {
        //trigger filter is turned off.
        decoderState.triggerFilterTime = 0;
        if( is70DegTooth(decoderState.toothCurrentCount) )
        { 
          decoderState.triggerToothAngle = 70;
        } 
        else 
        { 
          if(page2.nCylinders == 4) { decoderState.triggerToothAngle = 110; }
          else  { decoderState.triggerToothAngle = 50; }
        }
      }

      //EXPERIMENTAL!
      //New ignition mode is ONLY available on 4g63 when the trigger angle is set to the stock value of 0.
      if( (page2.perToothIgn == true) && (page4.triggerAngle == 0) )
      {
        if( (page2.nCylinders == 4) && (current.advance > 0) )
        {
          int16_t crankAngle = decoderState.toothAngles[(decoderState.toothCurrentCount-1)];

          //Handle non-sequential tooth counts 
          if( (page4.sparkMode != IGN_MODE_SEQUENTIAL) && (decoderState.toothCurrentCount > page2.nCylinders) ) { checkPerToothTiming(current, decoderState, page4, crankAngle, (decoderState.toothCurrentCount-page2.nCylinders) ); }
          else { checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount); }
        }
      }
    } //Has sync
    else
    {
      decoderState.triggerSecFilterTime = 0;
      //New secondary method of determining sync
      if(current.decoder.primary.isPinHigh())
      {
        if(current.decoder.secondary.isPinHigh()) { decoderState.revolutionOne = true; }
        else { decoderState.revolutionOne = false; }
      }
      else
      {
        if( (!current.decoder.secondary.isPinHigh()) && (decoderState.revolutionOne == true) ) 
        { 
          //Crank is low, cam is low and the crank pulse STARTED when the cam was high. 
          if(page2.nCylinders == 4) { decoderState.toothCurrentCount = 1; } //Means we're at 5* BTDC on a 4G63 4 cylinder
          //else if(page2.nCylinders == 6) { decoderState.toothCurrentCount = 8; } 
        } 
        //If sequential is ever enabled, the below decoderState.toothCurrentCount will need to change:
        else if( (current.decoder.secondary.isPinHigh()) && (decoderState.revolutionOne == true) ) 
        { 
          //Crank is low, cam is high and the crank pulse STARTED when the cam was high. 
          if(page2.nCylinders == 4) { decoderState.toothCurrentCount = 5; } //Means we're at 5* BTDC on a 4G63 4 cylinder
          else if(page2.nCylinders == 6) { decoderState.toothCurrentCount = 2; decoderState.decoderStatus.syncStatus = SyncStatus::Full; } //Means we're at 45* ATDC on 6G72 6 cylinder
        } 
      }
    }
  } //Filter time
}

void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;
  if ( (decoderState.curGap2 >= decoderState.triggerSecFilterTime) )//|| (current.startRevolutions == 0) )
  {
    decoderState.toothLastSecToothTime = curTime;
    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
    //addToothLogEntry(decoderState.curGap, TOOTH_CAM_SECONDARY);

    decoderState.triggerSecFilterTime = decoderState.curGap2 >> 1; //Basic 50% filter for the secondary reading
    //More aggressive options:
    //62.5%:
    //decoderState.triggerSecFilterTime = (decoderState.curGap2 * 9) >> 5;
    //75%:
    //decoderState.triggerSecFilterTime = (decoderState.curGap2 * 6) >> 3;

    if( (decoderState.decoderStatus.syncStatus!=SyncStatus::Full) )
    {

      decoderState.triggerFilterTime = 1500; //If this is removed, can have trouble getting sync again after the engine is turned off (but ECU not reset).
      decoderState.triggerSecFilterTime = decoderState.triggerSecFilterTime >> 1; //Divide the secondary filter time by 2 again, making it 25%. Only needed when cranking
      if(current.decoder.primary.isPinHigh())
      {
        if(page2.nCylinders == 4)
        { 
          if(decoderState.toothCurrentCount == 8) { decoderState.decoderStatus.syncStatus = SyncStatus::Full; } //Is 8 for sequential, was 4
        }
        else if(page2.nCylinders == 6) 
        { 
          if(decoderState.toothCurrentCount == 7) { decoderState.decoderStatus.syncStatus = SyncStatus::Full; }
        }

      }
      else
      {
        if(page2.nCylinders == 4)
        { 
          if(decoderState.toothCurrentCount == 5) { decoderState.decoderStatus.syncStatus = SyncStatus::Full; } //Is 5 for sequential, was 1
        }
        //Cannot gain sync for 6 cylinder here. 
      }
    }

    if ( (current.RPM < current.crankRPM) || (page4.useResync == 1) )
    {
      if( (decoderState.decoderStatus.syncStatus==SyncStatus::Full) && (page2.nCylinders == 4) )
      {
        if(current.decoder.primary.isPinHigh())
        {
          //Whilst we're cranking and have sync, we need to watch for noise pulses.
          if(decoderState.toothCurrentCount != 8) 
          { 
            // This should never be true, except when there's noise
            decoderState.decoderStatus.syncStatus = SyncStatus::None;
            current.syncLossCounter++;
          } 
        }
      } //Has sync and 4 cylinder 
    } // Use resync or cranking
  } //Trigger filter
}

uint32_t getRevolutionTime(const statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  shared::tooth_speed_sample_t sample;
  uint32_t revolutionTime = 0U;

  /*
  During cranking, revolution time is calculated 4 times per revolution, once for each rising/falling of the crank signal.
  Because these signals aren't even (Alternating 110 and 70 degrees), this needs a special function
  */
  sample = atomicToothSpeedSample(current, decoderState);
  if (sample.syncStatus==SyncStatus::Full)
  {
    if (current.RPM < current.crankRPM)
    {
      if (revolutionTimeFromLastTooth(sample, revolutionTime)==true)
      {
        decoderState.MAX_STALL_TIME = 366667UL; // 50RPM
        return revolutionTime;
      }
      return shared::publishedRevolutionTime(current);
    }

    revolutionTime = stdGetRevolutionTime(current, decoderState, CAM_SPEED);
    decoderState.MAX_STALL_TIME = revolutionTime << 1; //Set the stall time to be twice the current RPM. This is a safe figure as there should be no single revolution where this changes more than this
    if (decoderState.MAX_STALL_TIME < 366667UL) { decoderState.MAX_STALL_TIME = 366667UL; } //Check for 50rpm minimum
    return revolutionTime;
  }

  return 0U; //Speed is unknown without sync
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  int16_t crankAngle = 0;
  if(decoderState.decoderStatus.syncStatus==SyncStatus::Full)
  {
    auto data = atomic_copy(decoderState.toothLastToothTime, decoderState.toothLastMinusOneToothTime, decoderState.revolutionOne, decoderState.toothCurrentCount, decoderState.triggerToothAngle, decoderState.decoderStatus.toothAngleIsCorrect);
    crankAngle = decoders::shared::lookup_crank_angle_calculator_tooth_interval_t(data).calculateCrankAngle(currMicros, decoderState.toothAngles, page4);
  }
  return decoders::shared::clampCrankAngle(crankAngle);
}

void setEndTeeth(shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  if(page2.nCylinders == 4)
  {
    if(page4.sparkMode == IGN_MODE_SEQUENTIAL) 
    { 
      decoderState.ignitionEndTeeth[0] = 8;
      decoderState.ignitionEndTeeth[1] = 2;
      decoderState.ignitionEndTeeth[2] = 4;
      decoderState.ignitionEndTeeth[3] = 6;
    }
    else
    {
      decoderState.ignitionEndTeeth[0] = 4;
      decoderState.ignitionEndTeeth[1] = 2;
      decoderState.ignitionEndTeeth[2] = 4; //Not used
      decoderState.ignitionEndTeeth[3] = 2;
    }
  }
  if(page2.nCylinders == 6)
  {
    if(page4.sparkMode == IGN_MODE_SEQUENTIAL) 
    { 
      //This should never happen as 6 cylinder sequential not supported
      decoderState.ignitionEndTeeth[0] = 8;
      decoderState.ignitionEndTeeth[1] = 2;
      decoderState.ignitionEndTeeth[2] = 4;
      decoderState.ignitionEndTeeth[3] = 6;
    }
    else
    {
      decoderState.ignitionEndTeeth[0] = 6;
      decoderState.ignitionEndTeeth[1] = 2;
      decoderState.ignitionEndTeeth[2] = 4;
      decoderState.ignitionEndTeeth[3] = 2; //Not used
    }
  }
}

}
}