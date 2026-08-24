#include "airconController.h"
#include "airconController_details.h"
#include "../../pins/inputPin.h"
#include "../../pins/outputPin.h"
#include "../../../unit_testing.h"
#include "../../../config_pages.h"
#include "../../../atomic.h"
#include "../../../globals.h"
#include "../../../units.h"

TESTABLE_STATIC airConController::details::state_t airConState;

static __attribute__((optimize("Os"))) uint8_t getAirConRequestPinMode(const config15 &page15)
{
  if(page15.airConReqPol)
  {
    // Inverted
    // +5V is ON, Use external pull-down resistor for OFF
    return INPUT;
  }
  else
  {
    //Normal
    // Pin pulled to Ground is ON. Floating (internally pulled up to +5V) is OFF.
    return INPUT_PULLUP;
  }
}


static inline void checkAirConCoolantLockout(statuses &current, const config15 &page15)
{
  // ---------------------------
  // Coolant Temperature Lockout
  // ---------------------------
  int offTemp = temperatureRemoveOffset(page15.airConClTempCut);
  if (current.coolant > offTemp)
  {
    // A/C is cut off due to high coolant
    current.acStatus.cltLockoutActive = true;
  }
  else if (current.coolant < (offTemp - 1))
  {
    // Adds a bit of hysteresis (2 degrees) to removing the lockout
    // Yes, it is 2 degrees (not 1 degree or 3 degrees) because we go "> offTemp" to enable and "< (offtemp-1)" to disable,
    // e.g. if offTemp is 100, it needs to go GREATER than 100 to enable, i.e. 101, and then 98 to disable,
    // because the coolant temp is an integer. So 98.5 degrees to 100.5 degrees is the analog null zone where nothing happens,
    // depending on sensor calibration and table interpolation.
    // Hopefully offTemp wasn't -40... otherwise underflow... but that would be ridiculous
    current.acStatus.cltLockoutActive = false;
  }
}

static inline void checkAirConTPSLockout(statuses &current, const config15 &page15)
{
  // ------------------------------
  // High Throttle Position Lockout
  // ------------------------------
  if (current.TPS > page15.airConTPSCut)
  {
    // A/C is cut off due to high TPS
    current.acStatus.tpsLockoutActive = true;
    airConState.tpsLockoutDelay = 0;
  }
  else if ( (current.acStatus.tpsLockoutActive == true) &&
            (current.TPS <= page15.airConTPSCut) )
  {
    // No need for hysteresis as we have the stand-down delay period after the high TPS condition goes away.
    if (airConState.tpsLockoutDelay >= page15.airConTPSCutTime)
    {
      current.acStatus.tpsLockoutActive = false;
    }
    else
    {
      airConState.tpsLockoutDelay++;
    }
  }
  else
  {
    airConState.tpsLockoutDelay = 0;
  }
}

static inline void checkAirConRPMLockout(statuses &current, const config15 &page15)
{
  // --------------------
  // High/Low RPM Lockout
  // --------------------
  if ( (current.RPM < (page15.airConMinRPMdiv10 * 10)) ||
       (current.RPMdiv100 > page15.airConMaxRPMdiv100) )
  {
    // A/C is cut off due to high/low RPM
    current.acStatus.rpmLockoutActive = true;
    airConState.rpmLockoutDelay = 0;
  }
  else if ( (current.RPM >= (page15.airConMinRPMdiv10 * 10)) &&
            (current.RPMdiv100 <= page15.airConMaxRPMdiv100) )
  {
    // No need to add hysteresis as we have the stand-down delay period after the high/low RPM condition goes away.
    if (airConState.rpmLockoutDelay >= page15.airConRPMCutTime)
    {
      current.acStatus.rpmLockoutActive = false;
    }
    else
    {
      airConState.rpmLockoutDelay++;
    }
  }
  else
  {
    airConState.rpmLockoutDelay = 0;
  }
}

TESTABLE_STATIC void airConOn(statuses &current, const config15 &page15)
{
  ATOMIC() { 
    if (page15.airConCompPol)
    {
      airConState.compPin.setPinLow();
    }
    else
    {
      airConState.compPin.setPinHigh();
    }
    current.acStatus.compressorOn = true; 
  }  
}
TESTABLE_STATIC void airConOff(statuses &current, const config15 &page15)
{
  ATOMIC() { 
    if (page15.airConCompPol)
    {
      airConState.compPin.setPinHigh();
    }
    else
    {
      airConState.compPin.setPinLow();
    }
    current.acStatus.compressorOn = false; 
  }
}
static void airConFanOn(statuses &current, const config15 &page15)
{
  ATOMIC() { 
    if (page15.airConFanPol)
    {
      airConState.fanPin.setPinLow();
    }
    else
    {
      airConState.fanPin.setPinHigh();
    }
    current.acStatus.fanOn = true; 
  }
}
static void airConFanOff(statuses &current, const config15 &page15)
{
  ATOMIC() { 
    if (page15.airConFanPol)
    {
      airConState.fanPin.setPinHigh();
    }
    else
    {
      airConState.fanPin.setPinLow();
    }
    current.acStatus.fanOn = false; 
  }
}

void __attribute__((optimize("Os"))) initialiseAirCon(statuses &current, const config15 &page15, const pinNumbers_t &pins)
{
  airConState = airConController::details::state_t();
  current.acStatus = airConStatus_t();

  if( (page15.airConEnable) &&
      !pinIsReserved(pins.pinAirConRequest) &&
      !pinIsReserved(pins.pinAirConComp) &&
      !pinIsOutput(pins.pinAirConRequest))
  {
    airConState.reqPin.setPin(pins.pinAirConRequest, getAirConRequestPinMode(page15));
    airConState.compPin.setPin(pins.pinAirConComp, OUTPUT);

    airConOff(current, page15);

    if((page15.airConFanEnabled) && (pinIsReserved(pins.pinAirConFan)))
    {
      airConState.fanPin.setPin(pins.pinAirConFan, OUTPUT);
      airConFanOff(current, page15);
      airConState.standAloneFanIsEnabled = true;
    }
    else
    {
      airConState.standAloneFanIsEnabled = false;
    }

    airConState.isEnabled = true;
  }
}

static bool READ_AIRCON_REQUEST(statuses &current, const config15 &page15)
{
  if(airConState.isEnabled == false)
  {
    return false;
  }
  // Read the status of the A/C request pin (A/C button), taking into account the pin's polarity
  current.acStatus.acRequested = airConState.reqPin.isPinHigh()==page15.airConReqPol;
  return current.acStatus.acRequested;
}

void airConControl(statuses &current, const config15 &page15)
{
  if(airConState.isEnabled == true)
  {
    // ------------------------------------------------------------------------------------------------------
    // Check that the engine has been running past the post-start delay period before enabling the compressor
    // ------------------------------------------------------------------------------------------------------
    if (current.rotationStatus==EngineRotationStatus::Running)
    {
      if(airConState.afterEngineStartDelay >= page15.airConAfterStartDelay)
      {
        airConState.waitedAfterCranking = true;
      }
      else
      {
        airConState.afterEngineStartDelay++;
      }
    }
    else
    {
      airConState.afterEngineStartDelay = 0;
      airConState.waitedAfterCranking = false;
    }
    
    // --------------------------------------------------------------------
    // Determine the A/C lockouts based on the noted parameters
    // These functions set/clear the globl current.airConStatus bits.
    // --------------------------------------------------------------------
    checkAirConCoolantLockout(current, page15);
    checkAirConTPSLockout(current, page15);
    checkAirConRPMLockout(current, page15);
    
    // -----------------------------------------
    // Check the A/C Request Signal (A/C Button)
    // -----------------------------------------
    if( READ_AIRCON_REQUEST(current, page15) == true &&
        airConState.waitedAfterCranking == true &&
        current.acStatus.tpsLockoutActive == false &&
        current.acStatus.rpmLockoutActive == false &&
        current.acStatus.cltLockoutActive == false )
    {
      // Set the flag bit to notify the idle system to idle up & the cooling fan to start (if enabled)
      current.acStatus.turningOn = true;

      // Stand-alone fan operation
      if(airConState.standAloneFanIsEnabled == true)
      {
        airConFanOn(current, page15);
      }

      // Start the A/C compressor after the "Compressor On" delay period
      if(airConState.startDelay >= page15.airConCompOnDelay)
      {
        airConOn(current, page15);
      }
      else
      {
        airConState.startDelay++;
      }
    }
    else
    {
      current.acStatus.turningOn = false;

      // Stand-alone fan operation
      if(airConState.standAloneFanIsEnabled == true)
      {
        airConFanOff(current, page15);
      }

      airConOff(current, page15);
      airConState.startDelay = 0;
    }
  }
}
