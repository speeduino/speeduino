#include "ngc.h"
#include "shared/perToothIgnition.h"

namespace decoders {

namespace ngc {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(const config2 &page2, config4 &page4)
{
  shared::state_t state = {};
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;

  //Primary trigger
  page4.triggerTeeth = 36; //The number of teeth on the wheel incl missing teeth.
  state.setTriggerToothAngle(10);  //The number of degrees that passes from tooth to tooth
  state.triggerFilterTime = MICROS_PER_SEC / (MAX_RPM/60U) / (360U/state.triggerToothAngle); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle * 2U ); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  //Secondary trigger
  if (page2.nCylinders == 4) {
    state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM/60U) / (360U/36U) * 2U); //Two nearest edges are 36 degrees apart. Multiply by 2 for half cam speed.
  } else {
    state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM/60U) / (360U/21U) * 2U); //Two nearest edges are 21 degrees apart. Multiply by 2 for half cam speed.
  }

  //state.toothAngles is reused to store the cam pattern, only used for 6 and 8 cylinder pattern
  if (page2.nCylinders == 6) {
    state.toothAngles[0] = 1; // Pos 0 is required to be the same as group 6 for easier math
    state.toothAngles[1] = 3; // Group 1 ...
    state.toothAngles[2] = 1;
    state.toothAngles[3] = 2;
    state.toothAngles[4] = 3;
    state.toothAngles[5] = 2;
    state.toothAngles[6] = 1;
    state.toothAngles[7] = 3; // Pos 7 is required to be the same as group 1 for easier math
  }
  else if (page2.nCylinders == 8) {
    state.toothAngles[0] = 3; // Pos 0 is required to be the same as group 8 for easier math
    state.toothAngles[1] = 1; // Group 1 ...
    state.toothAngles[2] = 1;
    state.toothAngles[3] = 2;
    state.toothAngles[4] = 3;
    state.toothAngles[5] = 2;
    state.toothAngles[6] = 2;
    state.toothAngles[7] = 1;
    state.toothAngles[8] = 3;
    state.toothAngles[9] = 1; // Pos 9 is required to be the same as group 1 for easier math
  } else {
    // 4 cylinder - do nothing
  }

  return state;
}

void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
  // We need to know the polarity of the missing tooth to determine position
  if (current.decoder.primary.isPinHigh()) {
    decoderState.toothLastToothRisingTime = curTime;
    return;
  }

  decoderState.curGap = curTime - decoderState.toothLastToothTime;
  if ( decoderState.curGap >= decoderState.triggerFilterTime ) //Pulses should never be less than decoderState.triggerFilterTime, so if they are it means a false trigger.
  {
    decoderState.toothCurrentCount++;
    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)
    bool isMissingTooth = false;

    if ( decoderState.toothLastToothTime > 0 && decoderState.toothLastMinusOneToothTime > 0 ) { //Make sure we haven't enough tooth information to calculate missing tooth length

      //Only check for missing tooth if we expect this one to be it or if we haven't found one yet
      if (decoderState.toothCurrentCount == 17 || decoderState.toothCurrentCount == 35 || ( decoderState.decoderStatus.syncStatus==SyncStatus::None) ) {
        //If the time between the current tooth and the last is greater than 2x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
        if (decoderState.curGap > ( (decoderState.toothLastToothTime - decoderState.toothLastMinusOneToothTime) * 2 ) )
        {
          isMissingTooth = true; //Missing tooth detected
          decoderState.triggerFilterTime = 0; //This is used to prevent a condition where serious intermittent signals (Eg someone furiously plugging the sensor wire in and out) can leave the filter in an unrecoverable state
          decoderState.decoderStatus.toothAngleIsCorrect = false; //The tooth angle is double at this point
          
          // Figure out the polarity of the missing tooth by comparing how far ago the last tooth rose
          if ((decoderState.toothLastToothRisingTime - decoderState.toothLastToothTime) < (curTime - decoderState.toothLastToothRisingTime)) {
            //Just passed the HIGH missing tooth
            decoderState.toothCurrentCount = 1;

            decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
            decoderState.toothOneTime = curTime;

            if (decoderState.decoderStatus.syncStatus==SyncStatus::Full) { current.startRevolutions++; }
            else { current.startRevolutions = 0; }
          }
          else {
            //Just passed the first tooth after the LOW missing tooth
            decoderState.toothCurrentCount = 19;
          }

          //If Sequential fuel or ignition is in use, further checks are needed before determining sync
          if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) || (page2.injLayout == INJ_SEQUENTIAL) )
          {
            // Verify the tooth counters are valid and use this to determine current revolution
            if (
              ( page2.nCylinders == 4 && ( (decoderState.toothCurrentCount == 1 && (decoderState.secondaryToothCount == 1 || decoderState.secondaryToothCount == 2) ) || (decoderState.toothCurrentCount == 19 && decoderState.secondaryToothCount == 4) ) ) ||
              ( page2.nCylinders == 6 && ( (decoderState.toothCurrentCount == 1 && (decoderState.toothSystemCount == 1    || decoderState.toothSystemCount == 2) )    || (decoderState.toothCurrentCount == 19 && (decoderState.toothSystemCount == 2 || decoderState.toothSystemCount == 3) ) ) ) ||
              ( page2.nCylinders == 8 && ( (decoderState.toothCurrentCount == 1 && (decoderState.toothSystemCount == 1    || decoderState.toothSystemCount == 2) )    || (decoderState.toothCurrentCount == 19 && (decoderState.toothSystemCount == 3 || decoderState.toothSystemCount == 4) ) ) ) )
            {
              decoderState.revolutionOne = false;
              decoderState.decoderStatus.syncStatus = SyncStatus::Full; //the engine is fully synced so clear the Half Sync bit
            }
            else if (
              ( page2.nCylinders == 4 && ( (decoderState.toothCurrentCount == 1 && decoderState.secondaryToothCount == 5)                          || (decoderState.toothCurrentCount == 19 && decoderState.secondaryToothCount == 7) ) ) ||
              ( page2.nCylinders == 6 && ( (decoderState.toothCurrentCount == 1 && (decoderState.toothSystemCount == 4 || decoderState.toothSystemCount == 5) ) || (decoderState.toothCurrentCount == 19 && (decoderState.toothSystemCount == 5 || decoderState.toothSystemCount == 6) ) ) ) ||
              ( page2.nCylinders == 8 && ( (decoderState.toothCurrentCount == 1 && (decoderState.toothSystemCount == 5 || decoderState.toothSystemCount == 6) ) || (decoderState.toothCurrentCount == 19 && (decoderState.toothSystemCount == 7 || decoderState.toothSystemCount == 8) ) ) ) )
            {
              decoderState.revolutionOne = true;
              decoderState.decoderStatus.syncStatus = SyncStatus::Full; //the engine is fully synced so clear the Half Sync bit
            }
            // If tooth counters are not valid, set half sync bit
            else {
              if (decoderState.decoderStatus.syncStatus==SyncStatus::Full) { current.syncLossCounter++; }
              decoderState.decoderStatus.syncStatus = SyncStatus::Partial; //If there is primary trigger but no secondary we only have half sync.
            }
          }
          else { decoderState.decoderStatus.syncStatus = SyncStatus::Full; } //If nothing is using sequential, we have sync and also clear half sync bit

        }
        else {
          // If we have found a missing tooth and don't get the next one at the correct tooth we end up here -> Resync
          if (decoderState.decoderStatus.syncStatus==SyncStatus::Full) { current.syncLossCounter++; }
          decoderState.decoderStatus.syncStatus = SyncStatus::None;
        }
      }

      if(isMissingTooth == false)
      {
        //Regular (non-missing) tooth
        decoderState.setFilter(decoderState.curGap, page4);
        decoderState.decoderStatus.toothAngleIsCorrect = true;
      }
    }

    if (isMissingTooth == true) { // If we have a missing tooth, copy the gap from the previous tooth as that is the correct normal tooth length
      decoderState.toothLastMinusOneToothTime = curTime - (decoderState.toothLastToothTime - decoderState.toothLastMinusOneToothTime);
    }
    else {
      decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    }
    decoderState.toothLastToothTime = curTime;

    //NEW IGNITION MODE
    if( (page2.perToothIgn == true) && (current.rotationStatus!=EngineRotationStatus::Cranking) ) 
    {
      int16_t crankAngle = ( (decoderState.toothCurrentCount-1) * decoderState.triggerToothAngle ) + page4.triggerAngle;
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
}