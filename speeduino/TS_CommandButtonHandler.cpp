
/** \file
 * Header file for the TunerStudio command handler
 * The command handler manages all the inputs FROM TS which are issued when a command button is clicked by the user
 */

#include "TS_CommandButtonHandler.h"
#include "sensors.h"
#include "storage.h"
#include "SD_logger.h"
#include "pages.h"
#include "scheduledIO_ign.h"
#include "scheduledIO_inj.h"
#include "scheduler_fuel_controller.h"
#include "scheduler_ignition_controller.h"

TESTABLE_STATIC uint8_t testInjectorPulseCount = 0;
TESTABLE_STATIC uint8_t testIgnitionPulseCount = 0;

static bool commandRequiresStoppedEngine(uint16_t command)
{
  return ((command >= TS_CMD_INJ1_ON) && (command <= TS_CMD_IGN8_PULSED)) 
      || ((command == TS_CMD_TEST_ENBL) || (command == TS_CMD_TEST_DSBL));
}

static void injectorOn(const statuses &current, uint8_t injector)
{
  if( current.isTestModeActive )
  {
    openInjector(injector);
  }
}

static void injectorOff(statuses &current, uint8_t injector)
{
  if( current.isTestModeActive )
  { 
    closeInjector(injector); 
    BIT_CLEAR(current.HWTest_INJ_Pulsed, INJ1_CMD_BIT+(injector-1)); 
  }
}

static void injectorPulse(statuses &current, uint8_t injector)
{
  if( current.isTestModeActive ) 
  { 
    BIT_SET(current.HWTest_INJ_Pulsed, INJ1_CMD_BIT+(injector-1)); 
  }
  
  if(!BIT_CHECK(current.HWTest_INJ_Pulsed, INJ1_CMD_BIT+(injector-1))) 
  { 
    closeInjector(injector); //Ensure this output is turned off (Otherwise the output may stay on permanently)
  }
}

/**
 * @brief Given 3 command indexes, calculate the channel number
 * 
 * @note THIS ASSUMES THE COMMAND INDICES ARE REGULARLY SPACED. 
 * 
 * @param cmd Command index to convert to channel. E.g. TS_CMD_IGN5_ON
 * @param baseCmd2 First command index. E.g. TS_CMD_IGN2_ON
 * @param baseCmd1 First command index. E.g. TS_CMD_IGN1_ON
 * @return uint8_t 
 */
static uint8_t computeChannel(uint16_t cmd, uint16_t baseCmd2, uint16_t baseCmd1)
{
  uint16_t stride = baseCmd2-baseCmd1;
  return 1+((cmd-baseCmd1)/stride);
}

static void coilOn(const statuses &current, uint8_t channel)
{
  if( current.isTestModeActive )
  { 
    beginCoilCharge(channel); 
  }
}

static void coilOff(statuses &current, uint8_t channel)
{
  if( current.isTestModeActive ) 
  {
    endCoilCharge(channel);
    BIT_CLEAR(current.HWTest_IGN_Pulsed, IGN1_CMD_BIT+(channel-1)); 
  }
}

static void coilPulse(statuses &current, uint8_t channel)
{
  if( current.isTestModeActive ) 
  { 
    BIT_SET(current.HWTest_IGN_Pulsed, IGN1_CMD_BIT+(channel-1)); 
  }
  if(!BIT_CHECK(current.HWTest_IGN_Pulsed, IGN1_CMD_BIT+(channel-1))) 
  { 
    endCoilCharge(channel); //Ensure this output is turned off (Otherwise the output may stay on permanently)
  }
}

static void computeVssRatio(statuses &current, config2 &page2, uint16_t config2::* pRatio)
{
  if(current.vss > 0)
  {
    (page2.*pRatio) = (current.vss * 10000UL) / current.RPM;
    savePage(1); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
    current.vssUiRefresh = true;
  }
}

/**
 * @brief 
 * 
 * @param command The command number of the button that was clicked. See TS_CommendButtonHandler.h for a list of button IDs
 */
bool handleTsCommand(uint16_t command, statuses &current, config2 &page2)
{
  if (commandRequiresStoppedEngine(command) && current.RPM != 0)
  {
    return false;
  }
  
  switch (command)
  {
    case TS_CMD_TEST_DSBL: // cmd is stop
      current.isTestModeActive = false;
      stopAllCoilsCharging();
      closeAllInjectors();
      current.HWTest_INJ_Pulsed = 0;
      current.HWTest_IGN_Pulsed = 0;
      break;

    case TS_CMD_TEST_ENBL: // cmd is enable
      current.isTestModeActive = true;
      break;

    case TS_CMD_INJ1_ON:
    case TS_CMD_INJ2_ON:
    case TS_CMD_INJ3_ON:
    case TS_CMD_INJ4_ON:
    case TS_CMD_INJ5_ON:
    case TS_CMD_INJ6_ON:
    case TS_CMD_INJ7_ON:
    case TS_CMD_INJ8_ON:
      injectorOn(current, computeChannel(command, TS_CMD_INJ2_ON, TS_CMD_INJ1_ON));
      break;

    case TS_CMD_INJ1_OFF:
    case TS_CMD_INJ2_OFF:
    case TS_CMD_INJ3_OFF:
    case TS_CMD_INJ4_OFF:
    case TS_CMD_INJ5_OFF:
    case TS_CMD_INJ6_OFF:
    case TS_CMD_INJ7_OFF:
    case TS_CMD_INJ8_OFF:
      injectorOff(current, computeChannel(command, TS_CMD_INJ2_OFF, TS_CMD_INJ1_OFF));
      break;

    case TS_CMD_INJ1_PULSED:
    case TS_CMD_INJ2_PULSED:
    case TS_CMD_INJ3_PULSED:
    case TS_CMD_INJ4_PULSED:
    case TS_CMD_INJ5_PULSED:
    case TS_CMD_INJ6_PULSED:
    case TS_CMD_INJ7_PULSED:
    case TS_CMD_INJ8_PULSED:
      injectorPulse(current, computeChannel(command, TS_CMD_INJ2_PULSED, TS_CMD_INJ1_PULSED));
      break;

    case TS_CMD_IGN1_ON:
    case TS_CMD_IGN2_ON:
    case TS_CMD_IGN3_ON:
    case TS_CMD_IGN4_ON:
    case TS_CMD_IGN5_ON:
    case TS_CMD_IGN6_ON:
    case TS_CMD_IGN7_ON:
    case TS_CMD_IGN8_ON:
      coilOn(current, computeChannel(command, TS_CMD_IGN2_ON, TS_CMD_IGN1_ON));
      break;

    case TS_CMD_IGN1_OFF:
    case TS_CMD_IGN2_OFF:
    case TS_CMD_IGN3_OFF:
    case TS_CMD_IGN4_OFF:
    case TS_CMD_IGN5_OFF:
    case TS_CMD_IGN6_OFF:
    case TS_CMD_IGN7_OFF:
    case TS_CMD_IGN8_OFF:
      coilOff(current, computeChannel(command, TS_CMD_IGN2_OFF, TS_CMD_IGN1_OFF));
      break;

    case TS_CMD_IGN1_PULSED:
    case TS_CMD_IGN2_PULSED:
    case TS_CMD_IGN3_PULSED:
    case TS_CMD_IGN4_PULSED:
    case TS_CMD_IGN5_PULSED:
    case TS_CMD_IGN6_PULSED:
    case TS_CMD_IGN7_PULSED:
    case TS_CMD_IGN8_PULSED:
      coilPulse(current, computeChannel(command, TS_CMD_IGN2_PULSED, TS_CMD_IGN1_PULSED));
      break;

    //VSS Calibration routines
    case TS_CMD_VSS_60KMH:
      {
        if(page2.vssMode == VSS_MODE_INTERNAL_PIN)
        {
          //Calculate the ratio of VSS reading from Aux/CAN input and actual VSS (assuming that actual VSS is really 60km/h).
          page2.vssPulsesPerKm = (current.canin[page2.vssAuxCh] / 60);
          savePage(veSetPage); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
          current.vssUiRefresh = true;
        }
        else
        {
          //Calibrate the actual pulses per distance
          uint32_t calibrationGap = vssGetPulseGap(0);
          if( calibrationGap > 0 )
          {
            page2.vssPulsesPerKm = MICROS_PER_MIN / calibrationGap;
            savePage(veSetPage); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
            current.vssUiRefresh = true;
          }
        }
      }
      break;

    //Calculate the RPM to speed ratio for each gear
    case TS_CMD_VSS_RATIO1:
      computeVssRatio(current, page2, &config2::vssRatio1);
      break;

    case TS_CMD_VSS_RATIO2:
      computeVssRatio(current, page2, &config2::vssRatio2);
      break;

    case TS_CMD_VSS_RATIO3:
      computeVssRatio(current, page2, &config2::vssRatio3);
      break;

    case TS_CMD_VSS_RATIO4: 
      computeVssRatio(current, page2, &config2::vssRatio4);
      break;

    case TS_CMD_VSS_RATIO5:
      computeVssRatio(current, page2, &config2::vssRatio5);
      break;

    case TS_CMD_VSS_RATIO6:
      computeVssRatio(current, page2, &config2::vssRatio6);
      break;
      
// LCOV_EXCL_START
    //STM32 Commands
    case TS_CMD_STM32_REBOOT: //
      doSystemReset();
      break;

    case TS_CMD_STM32_BOOTLOADER: //
      jumpToBootloader();
      break;

#ifdef SD_LOGGING
    case TS_CMD_SD_FORMAT: //Format SD card
      formatExFat();
      break;
#endif
// LCOV_EXCL_STOP

    default:
      return false;
      break;
  }

  return true;
}

static uint8_t nextPulseCount(uint8_t current, uint8_t max)
{
  if(current >= max)
  {
    current = 0;
  }
  else 
  { 
    ++current;
  }
  return current;
}

static void openPulsedInjectors(const statuses &current)
{
  for (uint8_t inj=0; inj<INJ_CHANNELS; ++inj)
  {
    if(BIT_CHECK(current.HWTest_INJ_Pulsed, inj))
    { 
      openInjector(inj+1);
    }
  }
  testInjectorPulseCount = 0;
}

static void closePulsedInjectors(const statuses &current, const config13 &page13)
{
  if (current.HWTest_INJ_Pulsed!=0)
  {
    testInjectorPulseCount = nextPulseCount(testInjectorPulseCount, page13.hwTestInjDuration);
    if (testInjectorPulseCount==0U)
    {
      closeAllInjectors();
    }
  }
}

static void beginChargingPulsedCoils(const statuses &current)
{
  for (uint8_t ign=0; ign<IGN_CHANNELS; ++ign)
  {
    if(BIT_CHECK(current.HWTest_IGN_Pulsed, ign))
    { 
      beginCoilCharge(ign+1);
    }
  }
  testIgnitionPulseCount = 0;
}

static void dischargePulsedCoils(const statuses &current, const config13 &page13)
{
  if (current.HWTest_IGN_Pulsed!=0U)
  {
    testIgnitionPulseCount = nextPulseCount(testIgnitionPulseCount, page13.hwTestIgnDuration);
    if (testIgnitionPulseCount==0U)
    {
      stopAllCoilsCharging();
    }
  }
}

void pulsedCommandController(const statuses &current, const config13 &page13)
{
  if( current.isTestModeActive )
  {
    // Pulse fuel and ignition test outputs are set at 30Hz
    if (BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_30HZ)
        && (current.RPM == 0))
    {
      openPulsedInjectors(current);
      beginChargingPulsedCoils(current);
    }

    // Turn off any of the pulsed testing outputs if they are active and have been running for long enough
    if (BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_1KHZ))
    {
      closePulsedInjectors(current, page13);
      dischargePulsedCoils(current, page13);
    }    
  }
}