#include "../../pins/inputPin.h"
#include "../../pins/outputPin.h"
#include "../../../unit_testing.h"
#include "../../../config_pages.h"
#include "../../../atomic.h"
#include "../../../globals.h"
#include "../../../units.h"

TESTABLE_STATIC inputPin_t aircon_req_pin;
TESTABLE_STATIC outputPin_t aircon_comp_pin;
TESTABLE_STATIC outputPin_t aircon_fan_pin;
TESTABLE_STATIC bool acIsEnabled;
TESTABLE_STATIC bool acStandAloneFanIsEnabled;
TESTABLE_STATIC uint8_t acStartDelay;
TESTABLE_STATIC uint8_t acTPSLockoutDelay;
TESTABLE_STATIC uint8_t acRPMLockoutDelay;
TESTABLE_STATIC uint8_t acAfterEngineStartDelay;
TESTABLE_STATIC bool waitedAfterCranking; // This starts false and prevents the A/C from running until a few seconds after cranking

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


static inline void checkAirConCoolantLockout(statuses &current)
{
  // ---------------------------
  // Coolant Temperature Lockout
  // ---------------------------
  int offTemp = temperatureRemoveOffset(configPage15.airConClTempCut);
  if (current.coolant > offTemp)
  {
    // A/C is cut off due to high coolant
    current.airconCltLockout = true;
  }
  else if (current.coolant < (offTemp - 1))
  {
    // Adds a bit of hysteresis (2 degrees) to removing the lockout
    // Yes, it is 2 degrees (not 1 degree or 3 degrees) because we go "> offTemp" to enable and "< (offtemp-1)" to disable,
    // e.g. if offTemp is 100, it needs to go GREATER than 100 to enable, i.e. 101, and then 98 to disable,
    // because the coolant temp is an integer. So 98.5 degrees to 100.5 degrees is the analog null zone where nothing happens,
    // depending on sensor calibration and table interpolation.
    // Hopefully offTemp wasn't -40... otherwise underflow... but that would be ridiculous
    current.airconCltLockout = false;
  }
}

static inline void checkAirConTPSLockout(statuses &current)
{
  // ------------------------------
  // High Throttle Position Lockout
  // ------------------------------
  if (current.TPS > configPage15.airConTPSCut)
  {
    // A/C is cut off due to high TPS
    current.airconTpsLockout = true;
    acTPSLockoutDelay = 0;
  }
  else if ( (current.airconTpsLockout == true) &&
            (current.TPS <= configPage15.airConTPSCut) )
  {
    // No need for hysteresis as we have the stand-down delay period after the high TPS condition goes away.
    if (acTPSLockoutDelay >= configPage15.airConTPSCutTime)
    {
      current.airconTpsLockout = false;
    }
    else
    {
      acTPSLockoutDelay++;
    }
  }
  else
  {
    acTPSLockoutDelay = 0;
  }
}

static inline void checkAirConRPMLockout(statuses &current)
{
  // --------------------
  // High/Low RPM Lockout
  // --------------------
  if ( (current.RPM < (configPage15.airConMinRPMdiv10 * 10)) ||
       (current.RPMdiv100 > configPage15.airConMaxRPMdiv100) )
  {
    // A/C is cut off due to high/low RPM
    current.airconRpmLockout = true;
    acRPMLockoutDelay = 0;
  }
  else if ( (current.RPM >= (configPage15.airConMinRPMdiv10 * 10)) &&
            (current.RPMdiv100 <= configPage15.airConMaxRPMdiv100) )
  {
    // No need to add hysteresis as we have the stand-down delay period after the high/low RPM condition goes away.
    if (acRPMLockoutDelay >= configPage15.airConRPMCutTime)
    {
      current.airconRpmLockout = false;
    }
    else
    {
      acRPMLockoutDelay++;
    }
  }
  else
  {
    acRPMLockoutDelay = 0;
  }
}

TESTABLE_STATIC void airConOn(statuses &current)
{
  ATOMIC() { 
    if (configPage15.airConCompPol)
    {
      aircon_comp_pin.setPinLow();
    }
    else
    {
      aircon_comp_pin.setPinHigh();
    }
    current.airconCompressorOn = true; 
  }  
}
TESTABLE_STATIC void airConOff(statuses &current)
{
  ATOMIC() { 
    if (configPage15.airConCompPol)
    {
      aircon_comp_pin.setPinHigh();
    }
    else
    {
      aircon_comp_pin.setPinLow();
    }
    current.airconCompressorOn = false; 
  }
}
static void airConFanOn(statuses &current)
{
  ATOMIC() { 
    if (configPage15.airConFanPol)
    {
      aircon_fan_pin.setPinLow();
    }
    else
    {
      aircon_fan_pin.setPinHigh();
    }
    current.airconFanOn = true; 
  }
}
static void airConFanOff(statuses &current)
{
  ATOMIC() { 
    if (configPage15.airConFanPol)
    {
      aircon_fan_pin.setPinHigh();
    }
    else
    {
      aircon_fan_pin.setPinLow();
    }
    current.airconFanOn = false; 
  }
}

void __attribute__((optimize("Os"))) initialiseAirCon(statuses &current, const pinNumbers_t &pins)
{
  if( (configPage15.airConEnable) &&
      !pinIsReserved(pins.pinAirConRequest) &&
      !pinIsReserved(pins.pinAirConComp) &&
      !pinIsOutput(pins.pinAirConRequest))
  {
    // Hold the A/C off until a few seconds after cranking
    acAfterEngineStartDelay = 0;
    waitedAfterCranking = false;

    acStartDelay = 0;
    acTPSLockoutDelay = 0;
    acRPMLockoutDelay = 0;

    current.airconRequested = false;
    current.airconCompressorOn = false;
    current.airconRpmLockout = false;
    current.airconTpsLockout = false;
    current.airconTurningOn = false;
    current.airconCltLockout = false;
    current.airconFanOn = false;
    aircon_req_pin.setPin(pins.pinAirConRequest, getAirConRequestPinMode(configPage15));
    aircon_comp_pin.setPin(pins.pinAirConComp, OUTPUT);
  
    airConOff(current);

    if((configPage15.airConFanEnabled) && (pinIsReserved(pins.pinAirConFan)))
    {
      aircon_fan_pin.setPin(pins.pinAirConFan, OUTPUT);
      airConFanOff(current);
      acStandAloneFanIsEnabled = true;
    }
    else
    {
      acStandAloneFanIsEnabled = false;
    }

    acIsEnabled = true;

  }
  else
  {
    acIsEnabled = false;
  }
}

static bool READ_AIRCON_REQUEST(statuses &current)
{
  if(acIsEnabled == false)
  {
    return false;
  }
  // Read the status of the A/C request pin (A/C button), taking into account the pin's polarity
  current.airconRequested = aircon_req_pin.isPinHigh()==configPage15.airConReqPol;
  return current.airconRequested;
}

void airConControl(statuses &current)
{
  if(acIsEnabled == true)
  {
    // ------------------------------------------------------------------------------------------------------
    // Check that the engine has been running past the post-start delay period before enabling the compressor
    // ------------------------------------------------------------------------------------------------------
    if (current.rotationStatus==EngineRotationStatus::Running)
    {
      if(acAfterEngineStartDelay >= configPage15.airConAfterStartDelay)
      {
        waitedAfterCranking = true;
      }
      else
      {
        acAfterEngineStartDelay++;
      }
    }
    else
    {
      acAfterEngineStartDelay = 0;
      waitedAfterCranking = false;
    }
    
    // --------------------------------------------------------------------
    // Determine the A/C lockouts based on the noted parameters
    // These functions set/clear the globl current.airConStatus bits.
    // --------------------------------------------------------------------
    checkAirConCoolantLockout(current);
    checkAirConTPSLockout(current);
    checkAirConRPMLockout(current);
    
    // -----------------------------------------
    // Check the A/C Request Signal (A/C Button)
    // -----------------------------------------
    if( READ_AIRCON_REQUEST(current) == true &&
        waitedAfterCranking == true &&
        current.airconTpsLockout == false &&
        current.airconRpmLockout == false &&
        current.airconCltLockout == false )
    {
      // Set the flag bit to notify the idle system to idle up & the cooling fan to start (if enabled)
      current.airconTurningOn = true;

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        airConFanOn(current);
      }

      // Start the A/C compressor after the "Compressor On" delay period
      if(acStartDelay >= configPage15.airConCompOnDelay)
      {
        airConOn(current);
      }
      else
      {
        acStartDelay++;
      }
    }
    else
    {
      current.airconTurningOn = false;

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        airConFanOff(current);
      }

      airConOff(current);
      acStartDelay = 0;
    }
  }
}
