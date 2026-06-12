#include "launchController.h"

void checkLaunchAndFlatShift(statuses &current, uint8_t launchPin, const config2 &page2, const config6 &page6, const config10 &page10, const config15 &page15)
{
  //Check for launching/flat shift (clutch) based on the current and previous clutch states
  current.previousClutchTrigger = current.clutchTrigger;
  //Only check for pinLaunch if any function using it is enabled. Else pins might break starting a board
  if(page6.flatSEnable || page6.launchEnabled)
  {
    if(page6.launchHiLo > 0) { current.clutchTrigger = digitalRead(launchPin); }
    else { current.clutchTrigger = !digitalRead(launchPin); }

    current.clutchTriggerActive = current.clutchTrigger; //Stores the value to send to TunerStudio
  }
  if(current.clutchTrigger && (current.previousClutchTrigger != current.clutchTrigger) ) { current.clutchEngagedRPM = current.RPM; } //Check whether the clutch has been engaged or disengaged and store the current RPM if so

  //Default flags to off
  current.launchingHard = false; 
  current.hardLaunchActive = false;
  current.flatShiftingHard = false;

  if (page6.launchEnabled && current.clutchTrigger && (current.clutchEngagedRPM < ((unsigned int)(page6.flatSArm) * 100)) && (current.TPS >= page10.lnchCtrlTPS) ) 
  { 
    //Only enable if VSS is not used or if it is, make sure we're not above the speed limit
    if( (page2.vssMode == 0) || ((page2.vssMode > 0) && (current.vss < page10.lnchCtrlVss)) )
    {
      //Check whether RPM is above the launch limit
      uint16_t launchRPMLimit = (page6.lnchHardLim * 100);
      if( (page2.hardCutType == HARD_CUT_ROLLING) ) { launchRPMLimit += (page15.rollingProtRPMDelta[0] * 10); } //Add the rolling cut delta if enabled (Delta is a negative value)

      if(current.RPM > launchRPMLimit)
      {
        //HardCut rev limit for 2-step launch control.
        current.launchingHard = true; 
        current.hardLaunchActive = true;
      }
    }
  } 
  else 
  { 
    //If launch is not active, check whether flat shift should be active
    if(page6.flatSEnable && current.clutchTrigger && (current.clutchEngagedRPM >= ((unsigned int)(page6.flatSArm * 100)) ) ) 
    { 
      uint16_t flatRPMLimit = current.clutchEngagedRPM;
      if( (page2.hardCutType == HARD_CUT_ROLLING) ) { flatRPMLimit += (page15.rollingProtRPMDelta[0] * 10); } //Add the rolling cut delta if enabled (Delta is a negative value)

      if(current.RPM > flatRPMLimit)
      {
        //Flat shift rev limit
        current.flatShiftingHard = true;
      }
    }
  }
}
