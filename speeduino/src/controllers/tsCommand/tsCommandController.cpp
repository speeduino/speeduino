#include "../../../injector_bench.h"

/** \file
 * Header file for the TunerStudio command handler
 * The command handler manages all the inputs FROM TS which are issued when a command button is clicked by the user
 */

#include "tsCommandController.h"
#include "../../../sensors.h"
#include "../../../storage.h"
#include "../../../SD_logger.h"
#include "../../../pages.h"
#include "../../../scheduledIO_ign.h"
#include "../../../scheduledIO_inj.h"
#include "../../../scheduler_fuel_controller.h"
#include "../../../scheduler_ignition_controller.h"

// None of the code in this file is performance critical, so optimize for size.
#pragma GCC optimize("Os")

// Code below relies on these
static_assert(TS_CMD_VSS_RATIO2==TS_CMD_VSS_RATIO1+1, "");
static_assert(TS_CMD_VSS_RATIO3==TS_CMD_VSS_RATIO2+1, "");
static_assert(TS_CMD_VSS_RATIO4==TS_CMD_VSS_RATIO3+1, "");
static_assert(TS_CMD_VSS_RATIO5==TS_CMD_VSS_RATIO4+1, "");
static_assert(TS_CMD_VSS_RATIO6==TS_CMD_VSS_RATIO5+1, "");
static_assert(TS_CMD_VSS_RATIO6==TS_CMD_VSS_RATIO5+1, "");

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

bool handleTsCommand(uint16_t command, statuses &current, config2 &page2)
{
  if(command==TS_CMD_TEST_DSBL) {injectorBenchStop();current.isTestModeActive=false;return true;}
  if(injectorBenchOwnsOutputs() || command==TS_CMD_TEST_ENBL || (command>=513 && command<=792)) return false;
  
  switch (command)
  {
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
