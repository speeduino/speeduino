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


static inline bool isCoolantLockoutActive(const statuses &current, const config15 &page15)
{
  bool lockout = current.acStatus.cltLockoutActive;

  int16_t offTemp = temperatureRemoveOffset(page15.airConClTempCut);
  if (current.coolant > offTemp)
  {
    // A/C is cut off due to high coolant
    lockout = true;
  }
  else if (current.coolant < (offTemp - 1))
  {
    // Adds a bit of hysteresis (2 degrees) to removing the lockout
    // Yes, it is 2 degrees (not 1 degree or 3 degrees) because we go "> offTemp" to enable and "< (offtemp-1)" to disable,
    // e.g. if offTemp is 100, it needs to go GREATER than 100 to enable, i.e. 101, and then 98 to disable,
    // because the coolant temp is an integer. So 98.5 degrees to 100.5 degrees is the analog null zone where nothing happens,
    // depending on sensor calibration and table interpolation.
    lockout = false;
  }
  else
  {
    // Use default
  }
  return lockout;
}

static inline bool isTPSLockoutActive(const statuses &current, const config15 &page15)
{
  bool lockout = current.acStatus.tpsLockoutActive;

  if (current.TPS > page15.airConTPSCut)
  {
    // A/C is cut off due to high TPS
    lockout = true;
    airConState.resetTpsLockoutDelay();
  }
  else if ( (current.acStatus.tpsLockoutActive == true) &&
            (current.TPS <= page15.airConTPSCut) )
  {
    // No need for hysteresis as we have the stand-down delay period after the high TPS condition goes away.
    if (airConState.nextTpsLockoutDelay(page15))
    {
      lockout = false;
    }
  }
  else
  {
    airConState.resetTpsLockoutDelay();
  }

  return lockout;
}

static inline bool isRPMLockoutActive(const statuses &current, const config15 &page15)
{
  bool lockout = current.acStatus.rpmLockoutActive;

  // --------------------
  // High/Low RPM Lockout
  // --------------------
  if ( (current.RPM < RPM_MEDIUM.toUser(page15.airConMinRPMdiv10)) ||
       (current.RPMdiv100 > page15.airConMaxRPMdiv100) )
  {
    // A/C is cut off due to high/low RPM
    lockout = true;
    airConState.resetRpmLockoutDelay();
  }
  else if ( (current.RPM >= RPM_MEDIUM.toUser(page15.airConMinRPMdiv10)) &&
            (current.RPMdiv100 <= page15.airConMaxRPMdiv100) )
  {
    // No need to add hysteresis as we have the stand-down delay period after the high/low RPM condition goes away.
    if (airConState.nextRpmLockoutDelay(page15))
    {
      lockout = false;
    }
  }
  else
  {
    airConState.resetRpmLockoutDelay();
  }

  return lockout;
}

TESTABLE_STATIC void airConOn(statuses &current, const config15 &page15)
{
  if (airConState.compPin.isValid())
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
}

TESTABLE_STATIC void airConOff(statuses &current, const config15 &page15)
{
  if (airConState.compPin.isValid())
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
}

static void airConFanOn(statuses &current, const config15 &page15)
{
  if (airConState.fanPin.isValid())
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
}
static void airConFanOff(statuses &current, const config15 &page15)
{
  if (airConState.fanPin.isValid())
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
}

static __attribute__((optimize("Os"))) bool enableAc(const config15 &page15, const pinNumbers_t &pins)
{
  return (page15.airConEnable) &&
      !pinIsReserved(pins.pinAirConRequest) &&
      !pinIsReserved(pins.pinAirConComp) &&
      !pinIsOutput(pins.pinAirConRequest)
      ;
}

void __attribute__((optimize("Os"))) initialiseAirCon(statuses &current, const config15 &page15, const pinNumbers_t &pins)
{
  airConState = airConController::details::state_t();
  current.acStatus = airConStatus_t();

  if(enableAc(page15, pins))
  {
    airConState.reqPin.setPin(pins.pinAirConRequest, getAirConRequestPinMode(page15));
    airConState.compPin.setPin(pins.pinAirConComp, OUTPUT);
    if ((page15.airConFanEnabled) && (pinIsReserved(pins.pinAirConFan)))
    {
      airConState.fanPin.setPin(pins.pinAirConFan, OUTPUT);
    }

    airConOff(current, page15);
    airConFanOff(current, page15);
  }
}

static bool readRequestPin(const config15 &page15)
{
  // Read the status of the A/C request pin (A/C button), taking into account the pin's polarity
  return airConState.reqPin.isPinHigh()==page15.airConReqPol;
}

void airConControl(statuses &current, const config15 &page15)
{
  if(airConState.compPin.isValid())
  {
    // ------------------------------------------------------------------------------------------------------
    // Check that the engine has been running past the post-start delay period before enabling the compressor
    // ------------------------------------------------------------------------------------------------------
    bool waitedAfterCranking = false;
    if (current.rotationStatus==EngineRotationStatus::Running)
    {
      waitedAfterCranking = airConState.nextAfterEngineStartDelay(page15);
    }
    else
    {
      airConState.resetAfterEngineStartDelay();
    }
    
    // --------------------------------------------------------------------
    // Determine the A/C lockouts based on the noted parameters
    // These functions set/clear the globl current.airConStatus bits.
    // --------------------------------------------------------------------
    current.acStatus.cltLockoutActive = isCoolantLockoutActive(current, page15);
    current.acStatus.tpsLockoutActive = isTPSLockoutActive(current, page15);
    current.acStatus.rpmLockoutActive = isRPMLockoutActive(current, page15);
    
    // -----------------------------------------
    // Check the A/C Request Signal (A/C Button)
    // -----------------------------------------
    current.acStatus.acRequested = readRequestPin(page15);

    if(  current.acStatus.acRequested == true &&
        waitedAfterCranking == true &&
        !current.acStatus.isLockoutActive())
    {
      // Set the flag bit to notify the idle system to idle up & the cooling fan to start (if enabled)
      current.acStatus.turningOn = true;

      // Stand-alone fan operation
      airConFanOn(current, page15);

      // Start the A/C compressor after the "Compressor On" delay period
      if(airConState.nextStartDelay(page15))
      {
        airConOn(current, page15);
      }
    }
    else
    {
      current.acStatus.turningOn = false;
      airConFanOff(current, page15);
      airConOff(current, page15);
      airConState.resetStartDelay();
    }
  }
}
