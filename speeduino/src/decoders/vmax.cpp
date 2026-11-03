#include "vmax.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"

namespace decoders {

namespace vmax {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current)
{
  shared::state_t state = {};

  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * 60U); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  if(!current.initialisationComplete == false) { state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time
  state.triggerFilterTime = 1500;
  state.decoderStatus.validTrigger = true; // We must start with a valid trigger or we cannot start measuring the lobe width. We only have a false trigger on the lobe up event when it doesn't pass the filter. Then, the lobe width will also not be beasured.
  state.toothAngles[1] = 0;      //tooth #1, these are the absolute tooth positions
  state.toothAngles[2] = 40;     //tooth #2
  state.toothAngles[3] = 110;    //tooth #3
  state.toothAngles[4] = 180;    //tooth #4
  state.toothAngles[5] = 220;    //tooth #5
  state.toothAngles[6] = 290;    //tooth #6

  return state;
}

//decoderState.curGap = microseconds between primary triggers
//decoderState.curGap2 = microseconds between secondary triggers
//decoderState.toothCurrentCount = the current number for the end of a lobe
//decoderState.secondaryToothCount = the current number of the beginning of a lobe
//We measure the width of a lobe so on the end of a lobe, but want to trigger on the beginning. Variable decoderState.toothCurrentCount tracks the downward events, and decoderState.secondaryToothCount updates on the upward events. Ideally, it should be the other way round but the engine stall routine resets decoderState.secondaryToothCount, so it would not sync again after an engine stall.
void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  if(current.decoder.primary.isTriggered()){// Forwarded from the config page to setup the primary trigger edge (rising or falling). Inverting VR-conditioners require FALLING, non-inverting VR-conditioners require RISING in the Trigger edge setup.
    decoderState.curGap2 = curTime;
    decoderState.curGap = curTime - decoderState.toothLastToothTime;
    if ( (decoderState.curGap >= decoderState.triggerFilterTime) ){
      decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
      if (decoderState.toothCurrentCount > 0) // We have sync based on the tooth width.
      {
          decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
          if (decoderState.toothCurrentCount==1)
          {
            decoderState.secondaryToothCount = 1;
            decoderState.triggerToothAngle = 70;// Has to be equal to Angle Routine, and describe the delta between two teeth.
            decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
            decoderState.toothOneTime = curTime;
            decoderState.decoderStatus.syncStatus = SyncStatus::Full;
            //setFilter((decoderState.curGap/1.75));//Angle to this tooth is 70, next is in 40, compensating.
            decoderState.setFilter( ((decoderState.curGap*4)/7), page4 );//Angle to this tooth is 70, next is in 40, compensating.
            current.startRevolutions++; //Counter
          }
          else if (decoderState.toothCurrentCount==2)
          {
            decoderState.secondaryToothCount = 2;
            decoderState.triggerToothAngle = 40;
            //setFilter((decoderState.curGap*1.75));//Angle to this tooth is 40, next is in 70, compensating.
            decoderState.setFilter( ((decoderState.curGap*7)/4), page4 );//Angle to this tooth is 40, next is in 70, compensating.
          }
          else if (decoderState.toothCurrentCount==3)
          {
            decoderState.secondaryToothCount = 3;
            decoderState.triggerToothAngle = 70;
            decoderState.setFilter(decoderState.curGap, page4);//Angle to this tooth is 70, next is in 70. No need to compensate.
          }
          else if (decoderState.toothCurrentCount==4)
          {
            decoderState.secondaryToothCount = 4;
            decoderState.triggerToothAngle = 70;
            //setFilter((decoderState.curGap/1.75));//Angle to this tooth is 70, next is in 40, compensating.
            decoderState.setFilter( ((decoderState.curGap*4)/7), page4 );//Angle to this tooth is 70, next is in 40, compensating.
          }
          else if (decoderState.toothCurrentCount==5)
          {
            decoderState.secondaryToothCount = 5;
            decoderState.triggerToothAngle = 40;
            //setFilter((decoderState.curGap*1.75));//Angle to this tooth is 40, next is in 70, compensating.
            decoderState.setFilter( ((decoderState.curGap*7)/4), page4 );//Angle to this tooth is 40, next is in 70, compensating.
          }
          else if (decoderState.toothCurrentCount==6)
          {
            decoderState.secondaryToothCount = 6;
            decoderState.triggerToothAngle = 70;
            decoderState.setFilter(decoderState.curGap, page4);//Angle to this tooth is 70, next is in 70. No need to compensate.
          }
          decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
          decoderState.toothLastToothTime = curTime;
          if (decoderState.triggerFilterTime > 50000){//The first pulse seen 
            decoderState.triggerFilterTime = 0;
          }
      }
      else{
        decoderState.triggerFilterTime = 0;
        return;//Zero, no sync yet.
      }
    }
    else{
      decoderState.decoderStatus.validTrigger = false; //Flag this pulse as being an invalid trigger
    }
  }
  else if( decoderState.decoderStatus.validTrigger ) // Inverted due to vr conditioner. So this is the falling lobe. We only process if there was a valid trigger.
  {
    unsigned long curGapLocal = curTime - decoderState.curGap2;
    if (curGapLocal > (decoderState.lastGap * 2)){// Small lobe is 5 degrees, big lobe is 45 degrees. So this should be the wide lobe.
        if (decoderState.toothCurrentCount == 0 || decoderState.toothCurrentCount == 6){//Wide should be seen with decoderState.toothCurrentCount = 0, when there is no sync yet, or decoderState.toothCurrentCount = 6 when we have done a full revolution. 
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        }
        else{//Wide lobe seen where it shouldn't, adding a sync error.
          current.syncLossCounter++;
        }
        decoderState.toothCurrentCount = 1;
    }
    else if(decoderState.toothCurrentCount == 6){//The 6th lobe should be wide, adding a sync error.
        decoderState.toothCurrentCount = 1;
        current.syncLossCounter++;
    }
    else{// Small lobe, just add 1 to the decoderState.toothCurrentCount.
      decoderState.toothCurrentCount++;
    }
    decoderState.lastGap = curGapLocal;
    return;
  }
  else if( decoderState.decoderStatus.validTrigger == false)
  {
    decoderState.decoderStatus.validTrigger = true; //We reset this every time to ensure we only filter when needed.
  }
}

uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState)
{
  auto sample = atomicToothSpeedSample(current, decoderState);
  uint32_t revolutionTime = 0U;

  if (sample.syncStatus!=SyncStatus::Full) { return 0U; } //Speed is unknown without sync

  if (current.RPM < current.crankRPM)
  {
    if (revolutionTimeFromLastTooth(sample, revolutionTime)==true) { return revolutionTime; } //triggerToothAngle is zero until the first tooth after sync
    return shared::publishedRevolutionTime(current);
  }
  return stdGetRevolutionTime(current, decoderState, CRANK_SPEED);
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  auto calculator = decoders::shared::atomic_make_lookup_caa_secondary(decoderState);
  ++calculator._toothCurrentCount; // Since calculate() uses 0-based indices
  return decoders::shared::clampCrankAngle(calculator.calculateCrankAngle(currMicros, decoderState.toothAngles, page4));
}

}
}