
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

// None of the code in this file is performance critical, so optimize for size.
#pragma GCC optimize("Os")

// Code below relies on these
static_assert(TS_CMD_VSS_RATIO2==TS_CMD_VSS_RATIO1+1, "");
static_assert(TS_CMD_VSS_RATIO3==TS_CMD_VSS_RATIO2+1, "");
static_assert(TS_CMD_VSS_RATIO4==TS_CMD_VSS_RATIO3+1, "");
static_assert(TS_CMD_VSS_RATIO5==TS_CMD_VSS_RATIO4+1, "");
static_assert(TS_CMD_VSS_RATIO6==TS_CMD_VSS_RATIO5+1, "");
static_assert(TS_CMD_VSS_RATIO6==TS_CMD_VSS_RATIO5+1, "");

static_assert(TS_CMD_INJ2_ON-TS_CMD_INJ1_ON==TS_CMD_INJ3_ON-TS_CMD_INJ2_ON, "");
static_assert(TS_CMD_INJ3_ON-TS_CMD_INJ2_ON==TS_CMD_INJ3_ON-TS_CMD_INJ2_ON, "");
static_assert(TS_CMD_INJ4_ON-TS_CMD_INJ3_ON==TS_CMD_INJ3_ON-TS_CMD_INJ2_ON, "");
static_assert(TS_CMD_INJ5_ON-TS_CMD_INJ4_ON==TS_CMD_INJ3_ON-TS_CMD_INJ2_ON, "");
static_assert(TS_CMD_INJ6_ON-TS_CMD_INJ5_ON==TS_CMD_INJ3_ON-TS_CMD_INJ2_ON, "");
static_assert(TS_CMD_INJ7_ON-TS_CMD_INJ6_ON==TS_CMD_INJ3_ON-TS_CMD_INJ2_ON, "");
static_assert(TS_CMD_INJ8_ON-TS_CMD_INJ7_ON==TS_CMD_INJ3_ON-TS_CMD_INJ2_ON, "");

static_assert(TS_CMD_INJ2_OFF-TS_CMD_INJ1_OFF==TS_CMD_INJ3_OFF-TS_CMD_INJ2_OFF, "");
static_assert(TS_CMD_INJ3_OFF-TS_CMD_INJ2_OFF==TS_CMD_INJ3_OFF-TS_CMD_INJ2_OFF, "");
static_assert(TS_CMD_INJ4_OFF-TS_CMD_INJ3_OFF==TS_CMD_INJ3_OFF-TS_CMD_INJ2_OFF, "");
static_assert(TS_CMD_INJ5_OFF-TS_CMD_INJ4_OFF==TS_CMD_INJ3_OFF-TS_CMD_INJ2_OFF, "");
static_assert(TS_CMD_INJ6_OFF-TS_CMD_INJ5_OFF==TS_CMD_INJ3_OFF-TS_CMD_INJ2_OFF, "");
static_assert(TS_CMD_INJ7_OFF-TS_CMD_INJ6_OFF==TS_CMD_INJ3_OFF-TS_CMD_INJ2_OFF, "");
static_assert(TS_CMD_INJ8_OFF-TS_CMD_INJ7_OFF==TS_CMD_INJ3_OFF-TS_CMD_INJ2_OFF, "");

static_assert(TS_CMD_INJ2_PULSED-TS_CMD_INJ1_PULSED==TS_CMD_INJ3_PULSED-TS_CMD_INJ2_PULSED, "");
static_assert(TS_CMD_INJ3_PULSED-TS_CMD_INJ2_PULSED==TS_CMD_INJ3_PULSED-TS_CMD_INJ2_PULSED, "");
static_assert(TS_CMD_INJ4_PULSED-TS_CMD_INJ3_PULSED==TS_CMD_INJ3_PULSED-TS_CMD_INJ2_PULSED, "");
static_assert(TS_CMD_INJ5_PULSED-TS_CMD_INJ4_PULSED==TS_CMD_INJ3_PULSED-TS_CMD_INJ2_PULSED, "");
static_assert(TS_CMD_INJ6_PULSED-TS_CMD_INJ5_PULSED==TS_CMD_INJ3_PULSED-TS_CMD_INJ2_PULSED, "");
static_assert(TS_CMD_INJ7_PULSED-TS_CMD_INJ6_PULSED==TS_CMD_INJ3_PULSED-TS_CMD_INJ2_PULSED, "");
static_assert(TS_CMD_INJ8_PULSED-TS_CMD_INJ7_PULSED==TS_CMD_INJ3_PULSED-TS_CMD_INJ2_PULSED, "");

static_assert(TS_CMD_IGN2_ON-TS_CMD_IGN1_ON==TS_CMD_IGN3_ON-TS_CMD_IGN2_ON, "");
static_assert(TS_CMD_IGN3_ON-TS_CMD_IGN2_ON==TS_CMD_IGN3_ON-TS_CMD_IGN2_ON, "");
static_assert(TS_CMD_IGN4_ON-TS_CMD_IGN3_ON==TS_CMD_IGN3_ON-TS_CMD_IGN2_ON, "");
static_assert(TS_CMD_IGN5_ON-TS_CMD_IGN4_ON==TS_CMD_IGN3_ON-TS_CMD_IGN2_ON, "");
static_assert(TS_CMD_IGN6_ON-TS_CMD_IGN5_ON==TS_CMD_IGN3_ON-TS_CMD_IGN2_ON, "");
static_assert(TS_CMD_IGN7_ON-TS_CMD_IGN6_ON==TS_CMD_IGN3_ON-TS_CMD_IGN2_ON, "");
static_assert(TS_CMD_IGN8_ON-TS_CMD_IGN7_ON==TS_CMD_IGN3_ON-TS_CMD_IGN2_ON, "");

static_assert(TS_CMD_IGN2_OFF-TS_CMD_IGN1_OFF==TS_CMD_IGN3_OFF-TS_CMD_IGN2_OFF, "");
static_assert(TS_CMD_IGN3_OFF-TS_CMD_IGN2_OFF==TS_CMD_IGN3_OFF-TS_CMD_IGN2_OFF, "");
static_assert(TS_CMD_IGN4_OFF-TS_CMD_IGN3_OFF==TS_CMD_IGN3_OFF-TS_CMD_IGN2_OFF, "");
static_assert(TS_CMD_IGN5_OFF-TS_CMD_IGN4_OFF==TS_CMD_IGN3_OFF-TS_CMD_IGN2_OFF, "");
static_assert(TS_CMD_IGN6_OFF-TS_CMD_IGN5_OFF==TS_CMD_IGN3_OFF-TS_CMD_IGN2_OFF, "");
static_assert(TS_CMD_IGN7_OFF-TS_CMD_IGN6_OFF==TS_CMD_IGN3_OFF-TS_CMD_IGN2_OFF, "");
static_assert(TS_CMD_IGN8_OFF-TS_CMD_IGN7_OFF==TS_CMD_IGN3_OFF-TS_CMD_IGN2_OFF, "");

static_assert(TS_CMD_IGN2_PULSED-TS_CMD_IGN1_PULSED==TS_CMD_IGN3_PULSED-TS_CMD_IGN2_PULSED, "");
static_assert(TS_CMD_IGN3_PULSED-TS_CMD_IGN2_PULSED==TS_CMD_IGN3_PULSED-TS_CMD_IGN2_PULSED, "");
static_assert(TS_CMD_IGN4_PULSED-TS_CMD_IGN3_PULSED==TS_CMD_IGN3_PULSED-TS_CMD_IGN2_PULSED, "");
static_assert(TS_CMD_IGN5_PULSED-TS_CMD_IGN4_PULSED==TS_CMD_IGN3_PULSED-TS_CMD_IGN2_PULSED, "");
static_assert(TS_CMD_IGN6_PULSED-TS_CMD_IGN5_PULSED==TS_CMD_IGN3_PULSED-TS_CMD_IGN2_PULSED, "");
static_assert(TS_CMD_IGN7_PULSED-TS_CMD_IGN6_PULSED==TS_CMD_IGN3_PULSED-TS_CMD_IGN2_PULSED, "");
static_assert(TS_CMD_IGN8_PULSED-TS_CMD_IGN7_PULSED==TS_CMD_IGN3_PULSED-TS_CMD_IGN2_PULSED, "");

TESTABLE_STATIC uint8_t testInjectorPulseCount = 0;
TESTABLE_STATIC uint8_t testIgnitionPulseCount = 0;
TESTABLE_STATIC byte HWTest_INJ_Pulsed; /**< Each bit in this variable represents one of the injector channels and it's 50% HW test status */
TESTABLE_STATIC byte HWTest_IGN_Pulsed; /**< Each bit in this variable represents one of the ignition channels and it's 50% HW test status */

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

static void injectorOff(const statuses &current, uint8_t injector)
{
  if( current.isTestModeActive )
  { 
    closeInjector(injector); 
    BIT_CLEAR(HWTest_INJ_Pulsed, injector-1); 
  }
}

static void injectorPulse(const statuses &current, uint8_t injector)
{
  if( current.isTestModeActive ) 
  { 
    BIT_SET(HWTest_INJ_Pulsed, injector-1); 
  }
  
  if(!BIT_CHECK(HWTest_INJ_Pulsed, injector-1)) 
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

static void coilOff(const statuses &current, uint8_t channel)
{
  if( current.isTestModeActive ) 
  {
    endCoilCharge(channel);
    BIT_CLEAR(HWTest_IGN_Pulsed, channel-1); 
  }
}

static void coilPulse(const statuses &current, uint8_t channel)
{
  if( current.isTestModeActive ) 
  { 
    BIT_SET(HWTest_IGN_Pulsed, channel-1); 
  }
  if(!BIT_CHECK(HWTest_IGN_Pulsed, channel-1)) 
  { 
    endCoilCharge(channel); //Ensure this output is turned off (Otherwise the output may stay on permanently)
  }
}

static void computeVssRatio(statuses &current, config2 &page2, uint8_t ratioIndex)
{
  if(current.vss > 0)
  {
    page2.vssRatios[ratioIndex] = (current.vss * 10000UL) / current.RPM;
    savePage(1); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
    current.vssUiRefresh = true;
  }
}

TESTABLE_STATIC uint16_t calcPulsesPerKm(const statuses &current, const config2 &page2, uint32_t (*pGetGap)(byte))
{
  if(page2.vssMode == VSS_MODE_INTERNAL_PIN)
  {
    //Calculate the ratio of VSS reading from Aux/CAN input and actual VSS (assuming that actual VSS is really 60km/h).
    return (current.canin[page2.vssAuxCh] / 60);
  }

  //Calibrate the actual pulses per distance
  uint32_t calibrationGap = pGetGap(0);
  if( calibrationGap > 0 )
  {
    return MICROS_PER_MIN / calibrationGap;
  }

  // No update, so return original value
  return page2.vssPulsesPerKm;
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
      HWTest_INJ_Pulsed = 0;
      HWTest_IGN_Pulsed = 0;
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
      page2.vssPulsesPerKm = calcPulsesPerKm(current, page2, vssGetPulseGap);
      savePage(veSetPage); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
      current.vssUiRefresh = true;
      break;

    //Calculate the RPM to speed ratio for each gear
    case TS_CMD_VSS_RATIO1:
    case TS_CMD_VSS_RATIO2:
    case TS_CMD_VSS_RATIO3:
    case TS_CMD_VSS_RATIO4:
    case TS_CMD_VSS_RATIO5:
    case TS_CMD_VSS_RATIO6:
      computeVssRatio(current, page2, command-TS_CMD_VSS_RATIO1);
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

static void openPulsedInjectors(void)
{
  for (uint8_t inj=0; inj<INJ_CHANNELS; ++inj)
  {
    if(BIT_CHECK(HWTest_INJ_Pulsed, inj))
    { 
      openInjector(inj+1);
    }
  }
  testInjectorPulseCount = 0;
}

static void closePulsedInjectors(const config13 &page13)
{
  if (HWTest_INJ_Pulsed!=0)
  {
    testInjectorPulseCount = nextPulseCount(testInjectorPulseCount, page13.hwTestInjDuration);
    if (testInjectorPulseCount==0U)
    {
      closeAllInjectors();
    }
  }
}

static void beginChargingPulsedCoils(void)
{
  for (uint8_t ign=0; ign<IGN_CHANNELS; ++ign)
  {
    if(BIT_CHECK(HWTest_IGN_Pulsed, ign))
    { 
      beginCoilCharge(ign+1);
    }
  }
  testIgnitionPulseCount = 0;
}

static void dischargePulsedCoils(const config13 &page13)
{
  if (HWTest_IGN_Pulsed!=0U)
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
      openPulsedInjectors();
      beginChargingPulsedCoils();
    }

    // Turn off any of the pulsed testing outputs if they are active and have been running for long enough
    if (BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_1KHZ))
    {
      closePulsedInjectors(page13);
      dischargePulsedCoils(page13);
    }    
  }
}