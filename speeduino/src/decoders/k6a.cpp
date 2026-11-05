#include "k6a.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"

namespace decoders {

namespace k6a {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current, config4 &page4)
{
  shared::state_t state = {};
  state.triggerToothAngle = 90; //The number of degrees that passes from tooth to tooth (primary) - set to a value, needs to be set per tooth
  state.toothCurrentCount = 99; //Fake tooth count represents no sync

  page4.TrigSpeed = CAM_SPEED;
  state.triggerActualTeeth = 7;
  state.toothCurrentCount = 1;

  if(current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time

  // based on data in msextra page linked to above we can deduce,
  // gap between rising and falling edge of a normal 70 degree tooth is 48 degrees, this means the gap is 70 degrees - 48 degrees = 22 degrees.
  // assume this is constant for all similar sized gaps and teeth
  // sync tooth is 35 degrees - eyeball looks like the tooth is 50% tooth and 50% gap so guess its 17 degrees and 18 degrees.

  // coded every tooth here in case you want to try "change" setting on the trigger setup (this is defined in init.ino and what i've set it to, otherwise you need code to select rising or falling in init.ino (steal it from another trigger)). 
  // If you don't want change then drop the 'falling' edges listed below and half the number of edges + reduce the state.triggerActualTeeth
  // nb as you can edit the trigger offset using rising or falling edge setup below is irrelevant as you can adjust via the trigger offset to cover the difference.

  // not using state.toothAngles[0] as i'm hoping it makes logic easier

  state.toothAngles[0] = -70;                 // Wrap around to 650, 
  state.toothAngles[1] = 0;                   // 0 TDC cylinder 1, 
  state.toothAngles[2] = 170;                 // 170 - end of cylinder 1, start of cylinder 3, trigger ignition for cylinder 3 on this tooth
  state.toothAngles[3] = 240;                 // 70 TDC cylinder 3
  state.toothAngles[4] = 410;                 // 170  - end of cylinder 3, start of cylinder2, trigger ignition for cylinder 2 on this tooth
  state.toothAngles[5] = 480;                 // 70 TDC cylinder 2 
  state.toothAngles[6] = 515;                 // 35 Additional sync tooth
  state.toothAngles[7] = 650;                 // 135 end of cylinder 2, start of cylinder 1, trigger ignition for cylinder 1 on this tooth
  state.toothAngles[8] = 720;                 // 70 - gap to rotation to TDC1. array item 1 and 8 are the same, code never gets here its for reference only

  state.MAX_STALL_TIME = (3333UL * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  state.triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  state.triggerSecFilterTime = 0; //Need to figure out something better for this
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, config4 &page4)
{
  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( (decoderState.curGap >= decoderState.triggerFilterTime) || (current.startRevolutions == 0U) )
  {    
    decoderState.toothCurrentCount++;
    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    decoderState.toothLastToothTime = curTime;

    // now to figure out if its a normal tooth or the extra sync tooth
    // pattern is normally small tooth, big tooth, small tooth, big tooth. The extra tooth breaks the pattern go it goes, big tooth (decoderState.curGap3), small tooth(decoderState.curGap2), small tooth(decoderState.curGap)
    // reuse decoderState.curGap2 and decoderState.curGap3 (from secondary and Tertiary decoders) to store previous tooth sizes as not needed in this decoder.
    
    if ( (  decoderState.curGap <= decoderState.curGap2 ) && 
         ( decoderState.curGap2 <= decoderState.curGap3 ) )
    {
      // cur Gap is smaller than last gap & last gap is smaller than gap before that - means we must be on sync tooth
      decoderState.toothCurrentCount = 6; // set tooth counter to correct tooth
      decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    }
    
    decoderState.curGap3 = decoderState.curGap2; // update values for next time we're in the loop
    decoderState.curGap2 = decoderState.curGap;
    
    
    if( (decoderState.toothCurrentCount == (decoderState.triggerActualTeeth + 1U)) && decoderState.decoderStatus.syncStatus==SyncStatus::Full  )
    {
      // seen enough teeth to have a revolution of the crank
      decoderState.toothCurrentCount = 1; //Reset the counter
      decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
      decoderState.toothOneTime = curTime;
      current.startRevolutions = current.startRevolutions + 2U; // increment for 2 revs as we do 720 degrees on the the crank       
    }
    else if (decoderState.toothCurrentCount > (decoderState.triggerActualTeeth + 1U))
    {
      // Lost sync
      decoderState.decoderStatus.syncStatus = SyncStatus::None; 
      current.syncLossCounter++;
      decoderState.triggerFilterTime = 0;
      decoderState.toothCurrentCount=0;
    } else {
      // We have sync, but are part way through a revolution
      // Nothing to do but keep MISRA happy.
    }

    // check gaps match with tooth to check we have sync 
    // so if we *think* we've seen tooth 3 whose gap should be smaller than the previous tooth & it isn't, 
    // then we've lost sync
    switch (decoderState.toothCurrentCount)
    {
      case 1:
      case 3:      
      case 5:      
      case 6:
        // current tooth gap is bigger than previous tooth gap = syncloss
        // eg tooth 3 should be smaller than tooth 2 gap, if its not then we've lost sync and the tooth 3 we've just seen isn't really tooth 3
        if (decoderState.curGap > decoderState.curGap2)
        { 
          decoderState.decoderStatus.syncStatus = SyncStatus::None; 
          current.syncLossCounter++;
          decoderState.triggerFilterTime = 0;
          decoderState.toothCurrentCount=2;
        }          
        break;

      case 2:
      case 4:
      case 7:
      default:
        // current tooth gap is smaller than the previous tooth gap = syncloss
        // eg tooth 2 should be bigger than tooth 1, if its not then we've got syncloss
        if (decoderState.curGap < decoderState.curGap2)
        { 
          decoderState.decoderStatus.syncStatus = SyncStatus::None; 
          current.syncLossCounter++;
          decoderState.triggerFilterTime = 0;
          decoderState.toothCurrentCount=1;
        }
        break;
    }

    // Setup data to allow other areas of the system to work due to odd sized teeth - this could be merged with sync checking above, left separate to keep code clearer as its doing only one function at once
    // % of filter are not based on previous tooth size but expected next tooth size
    // decoderState.triggerToothAngle is the size of the previous tooth not the future tooth
    if (decoderState.decoderStatus.syncStatus==SyncStatus::Full )
    {
      switch (decoderState.toothCurrentCount) // Set tooth angle based on previous gap and decoderState.triggerFilterTime based on previous gap and next gap
      {
        case 2:
        case 4:
          // equivalent of tooth 1 except we've not done rotation code yet so its 8
          // 170 degree tooth, next tooth is 70
          switch (page4.triggerFilter)
          {
            case 1: // 25 % 17 degrees
              decoderState.triggerFilterTime = rshift<3>((uint32_t)decoderState.curGap);
              break;
            case 2: // 50 % 35 degrees
              decoderState.triggerFilterTime = rshift<3>((uint32_t)decoderState.curGap) + rshift<4>((uint32_t)decoderState.curGap);
              break;
            case 3: // 75 % 52 degrees
              decoderState.triggerFilterTime = rshift<2>((uint32_t)decoderState.curGap) + rshift<4>((uint32_t)decoderState.curGap);
              break;
            default:
              decoderState.triggerFilterTime = 0;
              break;
          }          
          break;

        case 5:
          // 70 degrees, next tooth is 35
          switch (page4.triggerFilter)
          {
            case 1: // 25 % 8 degrees
              decoderState.triggerFilterTime = rshift<3>((uint32_t)decoderState.curGap);
              break;
            case 2: // 50 % 17 degrees
              decoderState.triggerFilterTime = rshift<2>((uint32_t)decoderState.curGap);
              break;
            case 3: // 75 % 25 degrees
              decoderState.triggerFilterTime = rshift<2>((uint32_t)decoderState.curGap) + rshift<3>((uint32_t)decoderState.curGap);
              break;
            default:
              decoderState.triggerFilterTime = 0;
              break;
          }
          break;

        case 6:
          // sync tooth, next tooth is 135
          switch (page4.triggerFilter)
          {
            case 1: // 25 % 33 degrees
              decoderState.triggerFilterTime = decoderState.curGap;
              break;
            case 2: // 50 % 67 degrees
              decoderState.triggerFilterTime = decoderState.curGap * 2U;
              break;
            case 3: // 75 % 100 degrees
              decoderState.triggerFilterTime = decoderState.curGap * 3U;
              break;
            default:
              decoderState.triggerFilterTime = 0;
              break;
          }
          break;

        case 7:
          // 135 degre tooth, next tooth is 70
          switch (page4.triggerFilter)
          {
            case 1: // 25 % 17 degrees
              decoderState.triggerFilterTime = rshift<3>((uint32_t)decoderState.curGap);
              break;
            case 2: // 50 % 35 degrees
              decoderState.triggerFilterTime = rshift<2>((uint32_t)decoderState.curGap);
              break;
            case 3: // 75 % 52 degrees
              decoderState.triggerFilterTime = rshift<2>((uint32_t)decoderState.curGap) + rshift<3>((uint32_t)decoderState.curGap);
              break;
            default:
              decoderState.triggerFilterTime = 0;
              break;
          }          
          break;

        case 1:
        case 3:
          // 70 degree tooth, next tooth is 170
          switch (page4.triggerFilter)
          {
            case 1: // 25 % 42 degrees
              decoderState.triggerFilterTime = rshift<1>((uint32_t)decoderState.curGap) + rshift<3>((uint32_t)decoderState.curGap);
              break;
            case 2: // 50 % 85 degrees
              decoderState.triggerFilterTime = decoderState.curGap + rshift<2>((uint32_t)decoderState.curGap);
              break;
            case 3: // 75 % 127 degrees
              decoderState.triggerFilterTime = decoderState.curGap + rshift<1>((uint32_t)decoderState.curGap) + rshift<2>((uint32_t)decoderState.curGap);
              break;
            default:
              decoderState.triggerFilterTime = 0;
              break;
          }
          break;

        default:
          decoderState.triggerFilterTime = 0;
          break;
      }
      
      //NEW IGNITION MODE
      if( (page2.perToothIgn == true) ) 
      {  
        int16_t crankAngle = decoderState.toothAngles[decoderState.toothCurrentCount] + page4.triggerAngle;
        checkPerToothTiming(current, decoderState, page4, crankAngle, decoderState.toothCurrentCount);
      }     

    } // has sync

  } //Trigger filter
}

uint32_t getRevolutionTime(const statuses &current, shared::state_t &decoderState)
{
  //Cranking code needs working out. 

  uint32_t revolutionTime = stdGetRevolutionTime(current, decoderState, CAM_SPEED);

  decoderState.MAX_STALL_TIME = revolutionTime << 1; //Set the stall time to be twice the current RPM. This is a safe figure as there should be no single revolution where this changes more than this
  if(decoderState.MAX_STALL_TIME < 366667UL) { decoderState.MAX_STALL_TIME = 366667UL; } //Check for 50rpm minimum

  return revolutionTime;
}

int16_t getCrankAngle(uint32_t currMicros, shared::state_t &decoderState, const config4 &page4)
{
  auto calculator = decoders::shared::atomic_make_lookup_caa(decoderState);

  // TODO: check if this can be removed.
  if (calculator._toothCurrentCount!=0U) {
    decoderState.triggerToothAngle = (uint16_t)decoderState.toothAngles[calculator._toothCurrentCount] - (uint16_t)decoderState.toothAngles[calculator._toothCurrentCount-1U];
  }
  ++calculator._toothCurrentCount;
  
  return decoders::shared::clampCrankAngle(calculator.calculateCrankAngle(currMicros, decoderState.toothAngles, page4));
}

static uint16_t __attribute__((noinline)) calcEndTooth(const IgnitionSchedule &schedule, const shared::state_t &decoderState, const config4 &page4) {
  //Temp variables are used here to avoid potential issues if a trigger interrupt occurs part way through this function
  const int16_t tempIgnitionEndTooth = ignitionLimits(schedule.dischargeAngle - page4.triggerAngle);

  uint8_t nCount=1U;
  while ((nCount<8U) && (tempIgnitionEndTooth > decoderState.toothAngles[nCount])) {
    ++nCount;
  }
  if(nCount==1U || nCount==8U) {
    // didn't find a match, use tooth 7 as it must be greater than 7 but less than 1.  
    return 7U;
  } 

  // The tooth we want is the tooth prior to this one.     
  return nCount-1U;
}

void setEndTeeth(shared::state_t &decoderState, const config4 &page4)
{
  decoderState.ignitionEndTeeth[0] = calcEndTooth(ignitionSchedule1, decoderState, page4);
#if (IGN_CHANNELS >= 2)
  decoderState.ignitionEndTeeth[1] = calcEndTooth(ignitionSchedule2, decoderState, page4);
#endif
#if (IGN_CHANNELS >= 3)
  decoderState.ignitionEndTeeth[2] = calcEndTooth(ignitionSchedule3, decoderState, page4);
#endif
}

}
}