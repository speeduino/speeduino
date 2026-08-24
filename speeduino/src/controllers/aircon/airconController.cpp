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


static inline void checkAirConCoolantLockout(void)
{
  // ---------------------------
  // Coolant Temperature Lockout
  // ---------------------------
  int offTemp = temperatureRemoveOffset(configPage15.airConClTempCut);
  if (currentStatus.coolant > offTemp)
  {
    // A/C is cut off due to high coolant
    currentStatus.airconCltLockout = true;
  }
  else if (currentStatus.coolant < (offTemp - 1))
  {
    // Adds a bit of hysteresis (2 degrees) to removing the lockout
    // Yes, it is 2 degrees (not 1 degree or 3 degrees) because we go "> offTemp" to enable and "< (offtemp-1)" to disable,
    // e.g. if offTemp is 100, it needs to go GREATER than 100 to enable, i.e. 101, and then 98 to disable,
    // because the coolant temp is an integer. So 98.5 degrees to 100.5 degrees is the analog null zone where nothing happens,
    // depending on sensor calibration and table interpolation.
    // Hopefully offTemp wasn't -40... otherwise underflow... but that would be ridiculous
    currentStatus.airconCltLockout = false;
  }
}

static inline void checkAirConTPSLockout(void)
{
  // ------------------------------
  // High Throttle Position Lockout
  // ------------------------------
  if (currentStatus.TPS > configPage15.airConTPSCut)
  {
    // A/C is cut off due to high TPS
    currentStatus.airconTpsLockout = true;
    acTPSLockoutDelay = 0;
  }
  else if ( (currentStatus.airconTpsLockout == true) &&
            (currentStatus.TPS <= configPage15.airConTPSCut) )
  {
    // No need for hysteresis as we have the stand-down delay period after the high TPS condition goes away.
    if (acTPSLockoutDelay >= configPage15.airConTPSCutTime)
    {
      currentStatus.airconTpsLockout = false;
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

static inline void checkAirConRPMLockout(void)
{
  // --------------------
  // High/Low RPM Lockout
  // --------------------
  if ( (currentStatus.RPM < (configPage15.airConMinRPMdiv10 * 10)) ||
       (currentStatus.RPMdiv100 > configPage15.airConMaxRPMdiv100) )
  {
    // A/C is cut off due to high/low RPM
    currentStatus.airconRpmLockout = true;
    acRPMLockoutDelay = 0;
  }
  else if ( (currentStatus.RPM >= (configPage15.airConMinRPMdiv10 * 10)) &&
            (currentStatus.RPMdiv100 <= configPage15.airConMaxRPMdiv100) )
  {
    // No need to add hysteresis as we have the stand-down delay period after the high/low RPM condition goes away.
    if (acRPMLockoutDelay >= configPage15.airConRPMCutTime)
    {
      currentStatus.airconRpmLockout = false;
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

TESTABLE_STATIC void airConOn(void)
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
    currentStatus.airconCompressorOn = true; 
  }  
}
TESTABLE_STATIC void airConOff(void)
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
    currentStatus.airconCompressorOn = false; 
  }
}
static void airConFanOn(void)
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
    currentStatus.airconFanOn = true; 
  }
}
static void airConFanOff(void)
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
    currentStatus.airconFanOn = false; 
  }
}

void __attribute__((optimize("Os"))) initialiseAirCon(const pinNumbers_t &pins)
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

    currentStatus.airconRequested = false;
    currentStatus.airconCompressorOn = false;
    currentStatus.airconRpmLockout = false;
    currentStatus.airconTpsLockout = false;
    currentStatus.airconTurningOn = false;
    currentStatus.airconCltLockout = false;
    currentStatus.airconFanOn = false;
    aircon_req_pin.setPin(pins.pinAirConRequest, getAirConRequestPinMode(configPage15));
    aircon_comp_pin.setPin(pins.pinAirConComp, OUTPUT);
  
    airConOff();

    if((configPage15.airConFanEnabled) && (pinIsReserved(pins.pinAirConFan)))
    {
      aircon_fan_pin.setPin(pins.pinAirConFan, OUTPUT);
      airConFanOff();
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

static bool READ_AIRCON_REQUEST(void)
{
  if(acIsEnabled == false)
  {
    return false;
  }
  // Read the status of the A/C request pin (A/C button), taking into account the pin's polarity
  currentStatus.airconRequested = aircon_req_pin.isPinHigh()==configPage15.airConReqPol;
  return currentStatus.airconRequested;
}

void airConControl(void)
{
  if(acIsEnabled == true)
  {
    // ------------------------------------------------------------------------------------------------------
    // Check that the engine has been running past the post-start delay period before enabling the compressor
    // ------------------------------------------------------------------------------------------------------
    if (currentStatus.rotationStatus==EngineRotationStatus::Running)
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
    // These functions set/clear the globl currentStatus.airConStatus bits.
    // --------------------------------------------------------------------
    checkAirConCoolantLockout();
    checkAirConTPSLockout();
    checkAirConRPMLockout();
    
    // -----------------------------------------
    // Check the A/C Request Signal (A/C Button)
    // -----------------------------------------
    if( READ_AIRCON_REQUEST() == true &&
        waitedAfterCranking == true &&
        currentStatus.airconTpsLockout == false &&
        currentStatus.airconRpmLockout == false &&
        currentStatus.airconCltLockout == false )
    {
      // Set the flag bit to notify the idle system to idle up & the cooling fan to start (if enabled)
      currentStatus.airconTurningOn = true;

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        airConFanOn();
      }

      // Start the A/C compressor after the "Compressor On" delay period
      if(acStartDelay >= configPage15.airConCompOnDelay)
      {
        airConOn();
      }
      else
      {
        acStartDelay++;
      }
    }
    else
    {
      currentStatus.airconTurningOn = false;

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        airConFanOff();
      }

      airConOff();
      acStartDelay = 0;
    }
  }
}
