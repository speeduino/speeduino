#include "aux_aircon.h"
#include "globals.h"
#include "units.h"
#include BOARD_H

static inline void checkAirConCoolantLockout(void);
static inline void checkAirConTPSLockout(void);
static inline void checkAirConRPMLockout(void);

bool acIsEnabled;
bool acStandAloneFanIsEnabled;
uint8_t acStartDelay;
uint8_t acTPSLockoutDelay;
uint8_t acRPMLockoutDelay;
uint8_t acAfterEngineStartDelay;
bool waitedAfterCranking; // This starts false and prevents the A/C from running until a few seconds after cranking

volatile PORT_TYPE *aircon_comp_pin_port;
volatile PINMASK_TYPE aircon_comp_pin_mask;
volatile PORT_TYPE *aircon_fan_pin_port;
volatile PINMASK_TYPE aircon_fan_pin_mask;
volatile PORT_TYPE *aircon_req_pin_port;
volatile PINMASK_TYPE aircon_req_pin_mask;

/*
Air Conditioning Control
*/
void initialiseAirCon(void)
{
  if( (configPage15.airConEnable) == 1 &&
      pinAirConRequest != 0 &&
      pinAirConComp != 0 )
  {
    // Hold the A/C off until a few seconds after cranking
    acAfterEngineStartDelay = 0;
    waitedAfterCranking = false;

    acStartDelay = 0;
    acTPSLockoutDelay = 0;
    acRPMLockoutDelay = 0;

    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_REQUEST);     // Bit 0
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR);  // Bit 1
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_RPM_LOCKOUT); // Bit 2
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT); // Bit 3
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON);  // Bit 4
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_CLT_LOCKOUT); // Bit 5
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_FAN);         // Bit 6
    aircon_req_pin_port = portInputRegister(digitalPinToPort(pinAirConRequest));
    aircon_req_pin_mask = digitalPinToBitMask(pinAirConRequest);
    aircon_comp_pin_port = portOutputRegister(digitalPinToPort(pinAirConComp));
    aircon_comp_pin_mask = digitalPinToBitMask(pinAirConComp);

    AIRCON_OFF();

    if((configPage15.airConFanEnabled > 0) && (pinAirConFan != 0))
    {
      aircon_fan_pin_port = portOutputRegister(digitalPinToPort(pinAirConFan));
      aircon_fan_pin_mask = digitalPinToBitMask(pinAirConFan);
      AIRCON_FAN_OFF();
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

void airConControl(void)
{
  if(acIsEnabled == true)
  {
    // ------------------------------------------------------------------------------------------------------
    // Check that the engine has been running past the post-start delay period before enabling the compressor
    // ------------------------------------------------------------------------------------------------------
    if (BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN))
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
        BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT) == false &&
        BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_RPM_LOCKOUT) == false &&
        BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_CLT_LOCKOUT) == false )
    {
      // Set the BIT_AIRCON_TURNING_ON bit to notify the idle system to idle up & the cooling fan to start (if enabled)
      BIT_SET(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON);

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        AIRCON_FAN_ON();
      }

      // Start the A/C compressor after the "Compressor On" delay period
      if(acStartDelay >= configPage15.airConCompOnDelay)
      {
        AIRCON_ON();
      }
      else
      {
        acStartDelay++;
      }
    }
    else
    {
      BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON);

      // Stand-alone fan operation
      if(acStandAloneFanIsEnabled == true)
      {
        AIRCON_FAN_OFF();
      }

      AIRCON_OFF();
      acStartDelay = 0;
    }
  }
}

bool READ_AIRCON_REQUEST(void)
{
  if(acIsEnabled == false)
  {
    return false;
  }
  // Read the status of the A/C request pin (A/C button), taking into account the pin's polarity
  bool acReqPinStatus = ( ((configPage15.airConReqPol)==1) ? 
                             !!(*aircon_req_pin_port & aircon_req_pin_mask) :
                             !(*aircon_req_pin_port & aircon_req_pin_mask));
  BIT_WRITE(currentStatus.airConStatus, BIT_AIRCON_REQUEST, acReqPinStatus);
  return acReqPinStatus;
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
    BIT_SET(currentStatus.airConStatus, BIT_AIRCON_CLT_LOCKOUT);
  }
  else if (currentStatus.coolant < (offTemp - 1))
  {
    // Adds a bit of hysteresis (2 degrees) to removing the lockout
    // Yes, it is 2 degrees (not 1 degree or 3 degrees) because we go "> offTemp" to enable and "< (offtemp-1)" to disable,
    // e.g. if offTemp is 100, it needs to go GREATER than 100 to enable, i.e. 101, and then 98 to disable,
    // because the coolant temp is an integer. So 98.5 degrees to 100.5 degrees is the analog null zone where nothing happens,
    // depending on sensor calibration and table interpolation.
    // Hopefully offTemp wasn't -40... otherwise underflow... but that would be ridiculous
    BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_CLT_LOCKOUT);
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
    BIT_SET(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT);
    acTPSLockoutDelay = 0;
  }
  else if ( (BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT) == true) &&
            (currentStatus.TPS <= configPage15.airConTPSCut) )
  {
    // No need for hysteresis as we have the stand-down delay period after the high TPS condition goes away.
    if (acTPSLockoutDelay >= configPage15.airConTPSCutTime)
    {
      BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_TPS_LOCKOUT);
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
    BIT_SET(currentStatus.airConStatus, BIT_AIRCON_RPM_LOCKOUT);
    acRPMLockoutDelay = 0;
  }
  else if ( (currentStatus.RPM >= (configPage15.airConMinRPMdiv10 * 10)) &&
            (currentStatus.RPMdiv100 <= configPage15.airConMaxRPMdiv100) )
  {
    // No need to add hysteresis as we have the stand-down delay period after the high/low RPM condition goes away.
    if (acRPMLockoutDelay >= configPage15.airConRPMCutTime)
    {
      BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_RPM_LOCKOUT);
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