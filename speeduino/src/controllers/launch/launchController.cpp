#include "launchController.h"
#include "../../../globals.h"

void checkLaunchAndFlatShift()
{
  //Check for launching/flat shift (clutch) based on the current and previous clutch states
  currentStatus.previousClutchTrigger = currentStatus.clutchTrigger;
  //Only check for pinLaunch if any function using it is enabled. Else pins might break starting a board
  if(configPage6.flatSEnable || configPage6.launchEnabled)
  {
    if(configPage6.launchHiLo > 0) { currentStatus.clutchTrigger = digitalRead(pinLaunch); }
    else { currentStatus.clutchTrigger = !digitalRead(pinLaunch); }

    currentStatus.clutchTriggerActive = currentStatus.clutchTrigger; //Stores the value to send to TunerStudio
  }
  if(currentStatus.clutchTrigger && (currentStatus.previousClutchTrigger != currentStatus.clutchTrigger) ) { currentStatus.clutchEngagedRPM = currentStatus.RPM; } //Check whether the clutch has been engaged or disengaged and store the current RPM if so

  //Default flags to off
  currentStatus.launchingHard = false; 
  currentStatus.hardLaunchActive = false;
  currentStatus.flatShiftingHard = false;

  if (configPage6.launchEnabled && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.TPS >= configPage10.lnchCtrlTPS) ) 
  { 
    //Only enable if VSS is not used or if it is, make sure we're not above the speed limit
    if( (configPage2.vssMode == 0) || ((configPage2.vssMode > 0) && (currentStatus.vss < configPage10.lnchCtrlVss)) )
    {
      //Check whether RPM is above the launch limit
      uint16_t launchRPMLimit = (configPage6.lnchHardLim * 100);
      if( (configPage2.hardCutType == HARD_CUT_ROLLING) ) { launchRPMLimit += (configPage15.rollingProtRPMDelta[0] * 10); } //Add the rolling cut delta if enabled (Delta is a negative value)

      if(currentStatus.RPM > launchRPMLimit)
      {
        //HardCut rev limit for 2-step launch control.
        currentStatus.launchingHard = true; 
        currentStatus.hardLaunchActive = true;
      }
    }
  } 
  else 
  { 
    //If launch is not active, check whether flat shift should be active
    if(configPage6.flatSEnable && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM >= ((unsigned int)(configPage6.flatSArm * 100)) ) ) 
    { 
      uint16_t flatRPMLimit = currentStatus.clutchEngagedRPM;
      if( (configPage2.hardCutType == HARD_CUT_ROLLING) ) { flatRPMLimit += (configPage15.rollingProtRPMDelta[0] * 10); } //Add the rolling cut delta if enabled (Delta is a negative value)

      if(currentStatus.RPM > flatRPMLimit)
      {
        //Flat shift rev limit
        currentStatus.flatShiftingHard = true;
      }
    }
  }
}
