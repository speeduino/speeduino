#include "missingTooth.h"
#include "details/perToothIgnition.h"
#include "details/rev_time_calcs.h"
#include "details/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"
#include "unit_testing.h"

namespace decoders {

namespace missing_tooth {

detail::state_t __attribute__((optimize("Os"))) intialise(const config4 &page4)
{
  detail::state_t state = {};

  state.decoderFeatures.supportsPerToothIgnition = true;
  state.decoderFeatures.supportsSequential = page4.TrigSpeed == CAM_SPEED;

  // The number of degrees that passes from tooth to tooth
  uint16_t degreeTracking = (page4.TrigSpeed == CAM_SPEED) ? 720 : 360;
  state.setTriggerToothAngle(degreeTracking / page4.triggerTeeth);

  state.triggerActualTeeth = page4.triggerTeeth - page4.triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * page4.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  if (page4.trigPatternSec == SEC_TRIGGER_4_1)
  {
    state.triggerSecFilterTime = MICROS_PER_MIN / MAX_RPM / 4U / 2U;
  }
  else 
  {
    state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U));
  }
  state.checkSyncToothCount = (page4.triggerTeeth) >> 1; //50% of the total teeth.
  state.toothLastMinusOneToothTime = 0;
  state.toothCurrentCount = 0; 
  state.toothOneTime = 0;
  state.toothOneMinusOneTime = 0;
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle * (page4.triggerMissingTeeth + 1U)); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

TESTABLE_STATIC void applyPerToothIgnition(const statuses &current, detail::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  if( (page2.perToothIgn == true) && (current.rotationStatus!=EngineRotationStatus::Cranking) ) 
  {
    int16_t crankAngle = ( (decoderState.toothCurrentCount-1) * decoderState.triggerToothAngle ) + page4.triggerAngle;
    if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (decoderState.revolutionOne == true) && (page4.TrigSpeed == CRANK_SPEED) )
    {
      crankAngle += 360;
      detail::checkPerToothTiming(current, decoderState, page4, crankAngle, (page4.triggerTeeth + decoderState.toothCurrentCount)); 
    }
    else{ checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount); }
  }

}
void triggerPrimary(uint32_t curTime, statuses &current, detail::state_t &decoderState, const config2 &page2, const config4 &page4)
{
   decoderState.curGap = curTime - decoderState.toothLastToothTime;
   if ( decoderState.curGap >= decoderState.triggerFilterTime ) //Pulses should never be less than decoderState.triggerFilterTime, so if they are it means a false trigger. (A 36-1 wheel at 8000pm will have triggers approx. every 200uS)
   {
     decoderState.toothCurrentCount++; //Increment the tooth counter
     decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

      if( (decoderState.toothLastToothTime > 0) && (decoderState.toothLastMinusOneToothTime > 0) )
      {
        bool isMissingTooth = false;

        /*
        Performance Optimisation:
        Only need to try and detect the missing tooth if:
        1. WE don't have sync yet
        2. We have sync and are in the final 1/4 of the wheel (Missing tooth will/should never occur in the first 3/4)
        3. RPM is under 2000. This is to ensure that we don't interfere with strange timing when cranking or idling. Optimisation not really required at these speeds anyway
        */
        if( (decoderState.decoderStatus.syncStatus!=SyncStatus::Full) || (current.RPM < 2000) || (decoderState.toothCurrentCount >= (3 * decoderState.triggerActualTeeth >> 2)) )
        {
          //Begin the missing tooth detection
          //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
          if(page4.triggerMissingTeeth == 1) { decoderState.targetGap = (3 * (decoderState.toothLastToothTime - decoderState.toothLastMinusOneToothTime)) >> 1; } //Multiply by 1.5 (Checks for a gap 1.5x greater than the last one) (Uses bitshift to multiply by 3 then divide by 2. Much faster than multiplying by 1.5)
          else { decoderState.targetGap = ((decoderState.toothLastToothTime - decoderState.toothLastMinusOneToothTime)) * page4.triggerMissingTeeth; } //Multiply by 2 (Checks for a gap 2x greater than the last one)

          if( (decoderState.toothLastToothTime == 0) || (decoderState.toothLastMinusOneToothTime == 0) ) { decoderState.curGap = 0; }

          if ( (decoderState.curGap > decoderState.targetGap) || (decoderState.toothCurrentCount > decoderState.triggerActualTeeth) )
          {
            //Missing tooth detected
            isMissingTooth = true;
            if( (decoderState.toothCurrentCount < decoderState.triggerActualTeeth) && (decoderState.decoderStatus.syncStatus==SyncStatus::Full) ) 
            { 
                //This occurs when we're at tooth #1, but haven't seen all the other teeth. This indicates a signal issue so we flag lost sync so this will attempt to resync on the next revolution.
                decoderState.decoderStatus.syncStatus = SyncStatus::None;
                current.syncLossCounter++;
            }
            //This is to handle a special case on startup where sync can be obtained and the system immediately thinks the revs have jumped:
            else
            {
                if(decoderState.decoderStatus.syncStatus!=SyncStatus::None)
                {
                  current.startRevolutions++; //Counter
                  if ( page4.TrigSpeed == CAM_SPEED ) { current.startRevolutions++; } //Add an extra revolution count if we're running at cam speed
                }
                else { current.startRevolutions = 0; }
                
                decoderState.toothCurrentCount = 1;
                if (page4.trigPatternSec == SEC_TRIGGER_POLL) // at tooth one check if the cam sensor is high or low in poll level mode
                {
                  if (page4.PollLevelPolarity == current.decoder.secondary.isPinHigh()) { decoderState.revolutionOne = 1; }
                  else { decoderState.revolutionOne = 0; }
                }
                else {decoderState.revolutionOne = !decoderState.revolutionOne;} //Flip sequential revolution tracker if poll level is not used
                decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
                decoderState.toothOneTime = curTime;

                //if Sequential fuel or ignition is in use, further checks are needed before determining sync
                if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) || (page2.injLayout == INJ_SEQUENTIAL) )
                {
                  //If either fuel or ignition is sequential, only declare sync if the cam tooth has been seen OR if the missing wheel is on the cam
                  if( (decoderState.secondaryToothCount > 0) || (page4.TrigSpeed == CAM_SPEED) || (page4.trigPatternSec == SEC_TRIGGER_POLL) || (page2.strokes == TWO_STROKE) )
                  {
                    decoderState.decoderStatus.syncStatus = SyncStatus::Full; //the engine is fully synced so clear the Half Sync bit                    
                  }
                  else if(decoderState.decoderStatus.syncStatus!=SyncStatus::Full) { decoderState.decoderStatus.syncStatus = SyncStatus::Partial; } //If there is primary trigger but no secondary we only have half sync.
                }
                else { decoderState.decoderStatus.syncStatus = SyncStatus::Full; } //If nothing is using sequential, we have sync and also clear half sync bit
                if(page4.trigPatternSec == SEC_TRIGGER_SINGLE || page4.trigPatternSec == SEC_TRIGGER_TOYOTA_3) //Reset the secondary tooth counter to prevent it overflowing, done outside of sequental as v6 & v8 engines could be batch firing with VVT that needs the cam resetting
                { 
                  decoderState.secondaryToothCount = 0; 
                } 

                decoderState.triggerFilterTime = 0; //This is used to prevent a condition where serious intermittent signals (Eg someone furiously plugging the sensor wire in and out) can leave the filter in an unrecoverable state
                decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
                decoderState.toothLastToothTime = curTime;
                decoderState.decoderStatus.toothAngleIsCorrect = false; //The tooth angle is double at this point
            }
          }
        }
        
        if(isMissingTooth == false)
        {
          //Regular (non-missing) tooth
          decoderState.setFilter(decoderState.curGap, page4);
          decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
          decoderState.toothLastToothTime = curTime;
          decoderState.decoderStatus.toothAngleIsCorrect = true;
        }
      }
      else
      {
        //We fall here on initial startup when enough teeth have not yet been seen
        decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
        decoderState.toothLastToothTime = curTime;
      }     

      applyPerToothIgnition(current, decoderState, page2, page4);
   }
}

void triggerRecordVVT1Angle (statuses &current, const detail::state_t &decoderState, const config4 &page4, const config6 &page6, const config10 &page10)
{
  //Record the VVT Angle
  if( (page6.vvtEnabled > 0) && (decoderState.revolutionOne == 1) )
  {
    int16_t curAngle = normalize((int16_t)0, (int16_t)360, current.decoder.getCrankAngle());

    curAngle -= page4.triggerAngle; //Value at TDC
    if( page6.vvtMode == VVT_MODE_CLOSED_LOOP ) { curAngle -= page10.vvtCL0DutyAng; }

    current.vvt1.angle = LOW_PASS_FILTER( (curAngle << 1), page4.ANGLEFILTER_VVT, current.vvt1.angle);
  }
}

void triggerSecondary(uint32_t curTime, statuses &current, detail::state_t &decoderState, const config4 &page4, const config6 &page6, const config10 &page10)
{
  decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;

  //Safety check for initial startup
  if( (decoderState.toothLastSecToothTime == 0) )
  { 
    decoderState.curGap2 = 0; 
    decoderState.toothLastSecToothTime = curTime;
  }

  if ( decoderState.curGap2 >= decoderState.triggerSecFilterTime )
  {
    switch (page4.trigPatternSec)
    {
      case SEC_TRIGGER_4_1:
        decoderState.targetGap2 = (3 * (decoderState.toothLastSecToothTime - decoderState.toothLastMinusOneSecToothTime)) >> 1; //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
        decoderState.toothLastMinusOneSecToothTime = decoderState.toothLastSecToothTime;
        if ( (decoderState.curGap2 >= decoderState.targetGap2) || (decoderState.secondaryToothCount > 3) )
        {
          decoderState.secondaryToothCount = 1;
          decoderState.revolutionOne = 1; //Sequential revolution reset
          decoderState.triggerSecFilterTime = 0; //This is used to prevent a condition where serious intermittent signals (Eg someone furiously plugging the sensor wire in and out) can leave the filter in an unrecoverable state
          triggerRecordVVT1Angle(current, decoderState, page4, page6, page10);
        }
        else
        {
          decoderState.triggerSecFilterTime = decoderState.curGap2 >> 2; //Set filter at 25% of the current speed. Filter can only be recalc'd for the regular teeth, not the missing one.
          decoderState.secondaryToothCount++;
        }
        break;

      case SEC_TRIGGER_POLL:
        //Poll is effectively the same as SEC_TRIGGER_SINGLE, however we do not reset decoderState.revolutionOne
        //We do still need to record the angle for VVT though
        decoderState.triggerSecFilterTime = decoderState.curGap2 >> 1; //Next secondary filter is half the current gap
        triggerRecordVVT1Angle(current, decoderState, page4, page6, page10);
        break;

      case SEC_TRIGGER_SINGLE:
        //Standard single tooth cam trigger
        decoderState.revolutionOne = 1; //Sequential revolution reset
        decoderState.triggerSecFilterTime = decoderState.curGap2 >> 1; //Next secondary filter is half the current gap
        decoderState.secondaryToothCount++;
        triggerRecordVVT1Angle(current, decoderState, page4, page6, page10);
        break;

      case SEC_TRIGGER_TOYOTA_3:
        // designed for Toyota VVTI (2JZ) engine - 3 triggers on the cam. 
        // the 2 teeth for this are within 1 rotation (1 tooth first 360, 2 teeth second 360)
        decoderState.secondaryToothCount++;
        if(decoderState.secondaryToothCount == 2)
        { 
          decoderState.revolutionOne = 1; // sequential revolution reset
          triggerRecordVVT1Angle(current, decoderState, page4, page6, page10);
        }        
        //Next secondary filter is 25% the current gap, done here so we don't get a great big gap for the 1st tooth
        decoderState.triggerSecFilterTime = decoderState.curGap2 >> 2; 
        break;
    }
    decoderState.toothLastSecToothTime = curTime;
  } //Trigger filter

}

void triggerTertiary(uint32_t curTime, statuses &current, detail::state_t &decoderState, const config4 &page4, const config6 &page6)
{
  decoderState.curGap3 = curTime - decoderState.toothLastThirdToothTime;

  //Safety check for initial startup
  if( (decoderState.toothLastThirdToothTime == 0) )
  { 
    decoderState.curGap3 = 0; 
    decoderState.toothLastThirdToothTime = curTime;
  }

  if ( decoderState.curGap3 >= decoderState.triggerThirdFilterTime )
  {
    decoderState.triggerThirdFilterTime = decoderState.curGap3 >> 2; //Next third filter is 25% the current gap
    
    int16_t curAngle = normalize((int16_t)0, (int16_t)360, current.decoder.getCrankAngle());

    curAngle -= page4.triggerAngle; //Value at TDC
    if( page6.vvtMode == VVT_MODE_CLOSED_LOOP ) { curAngle -= page4.vvt2CL0DutyAng; }
    current.vvt2.angle = LOW_PASS_FILTER( (curAngle << 1), page4.ANGLEFILTER_VVT, current.vvt2.angle);    

    decoderState.toothLastThirdToothTime = curTime;
  }
}

uint32_t getRevolutionTime(const statuses &current, detail::state_t &decoderState, const config4 &page4)
{
  uint32_t revolutionTime = 0;
  if( current.RPM < current.crankRPM )
  {
    if(decoderState.toothCurrentCount != 1)
    {
      revolutionTime = detail::crankingGetRevolutionTime(current, decoderState, page4, page4.triggerTeeth, page4.TrigSpeed==CAM_SPEED); //Account for cam speed
    }
    else { revolutionTime = detail::publishedRevolutionTime(current); } //Can't do per tooth calculation if we're at tooth #1 as the missing tooth messes the calculation
  }
  else
  {
    revolutionTime = detail::stdGetRevolutionTime(current, decoderState, page4.TrigSpeed==CAM_SPEED); //Account for cam speed
  }
  return revolutionTime;
}

static uint16_t __attribute__((noinline)) calcEndTooth(const detail::state_t &decoderState, const config4 &page4, const IgnitionSchedule &schedule, uint8_t toothAdder) {
  int16_t tempEndTooth = decoderState.toothNumFromScheduleAngles(page4, schedule);
  //For higher tooth count triggers, add a 1 tooth margin to allow for calculation time. 
  if(page4.triggerTeeth > 12U) { tempEndTooth = tempEndTooth - 1; }
  
  // Clamp to tooth count
  return decoderState.clampToActualTeeth(page4, detail::clampToToothCount(page4, tempEndTooth, toothAdder), toothAdder);
}

void setEndTeeth(detail::state_t &decoderState, const config4 &page4)
{
  uint8_t toothAdder = 0;
  if( ((page4.sparkMode == IGN_MODE_SEQUENTIAL) || (page4.sparkMode == IGN_MODE_SINGLE)) && (page4.TrigSpeed == CRANK_SPEED)) { toothAdder = page4.triggerTeeth; }

  decoderState.ignitionEndTeeth[0] = calcEndTooth(decoderState, page4, ignitionSchedule1, toothAdder);
#if (IGN_CHANNELS >= 2)
  decoderState.ignitionEndTeeth[1] = calcEndTooth(decoderState, page4, ignitionSchedule2, toothAdder);
#endif
#if (IGN_CHANNELS >= 3)
  decoderState.ignitionEndTeeth[2] = calcEndTooth(decoderState, page4, ignitionSchedule3, toothAdder);
#endif
#if (IGN_CHANNELS >= 4)
  decoderState.ignitionEndTeeth[3] = calcEndTooth(decoderState, page4, ignitionSchedule4, toothAdder);
#endif
#if IGN_CHANNELS >= 5
  decoderState.ignitionEndTeeth[4] = calcEndTooth(decoderState, page4, ignitionSchedule5, toothAdder);
#endif
#if IGN_CHANNELS >= 6
  decoderState.ignitionEndTeeth[5] = calcEndTooth(decoderState, page4, ignitionSchedule6, toothAdder);
#endif
#if IGN_CHANNELS >= 7
  decoderState.ignitionEndTeeth[6] = calcEndTooth(decoderState, page4, ignitionSchedule7, toothAdder);
#endif
#if IGN_CHANNELS >= 8
  decoderState.ignitionEndTeeth[7] = calcEndTooth(decoderState, page4, ignitionSchedule8, toothAdder);
#endif 
}

int16_t getCrankAngle(uint32_t currMicros, const detail::state_t &decoderState, const config4 &page4)
{
  return decoders::detail::clampCrankAngle(decoders::detail::atomic_make_angle_caa(decoderState).calculateCrankAngle(currMicros, page4));
}

}
}