
/** \file
 * Header file for the TunerStudio command handler
 * The command handler manages all the inputs FROM TS which are issued when a command button is clicked by the user
 */

#include "globals.h"
#include "TS_CommandButtonHandler.h"
#include "sensors.h"
#include "storage.h"
#include "SD_logger.h"
#include "pages.h"
#include "scheduledIO_ign.h"
#include "scheduledIO_inj.h"
#ifdef USE_MC33810
  #include "acc_mc33810.h"
#endif
#include "scheduler_fuel_controller.h"
#include "scheduler_ignition_controller.h"

static bool commandRequiresStoppedEngine(uint16_t buttonCommand)
{
  return ((buttonCommand >= TS_CMD_INJ1_ON) && (buttonCommand <= TS_CMD_IGN8_PULSED)) 
      || ((buttonCommand == TS_CMD_TEST_ENBL) || (buttonCommand == TS_CMD_TEST_DSBL));
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
    BIT_CLEAR(HWTest_INJ_Pulsed, INJ1_CMD_BIT+(injector-1)); 
  }
}

static void injectorPulse(const statuses &current, uint8_t injector)
{
  if( current.isTestModeActive ) 
  { 
    BIT_SET(HWTest_INJ_Pulsed, INJ1_CMD_BIT+(injector-1)); 
  }
  
  if(!BIT_CHECK(HWTest_INJ_Pulsed, INJ1_CMD_BIT+(injector-1))) 
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

/**
 * @brief 
 * 
 * @param buttonCommand The command number of the button that was clicked. See TS_CommendButtonHandler.h for a list of button IDs
 */
bool TS_CommandButtonsHandler(uint16_t buttonCommand)
{
  if (commandRequiresStoppedEngine(buttonCommand) && currentStatus.RPM != 0)
  {
    return false;
  }
  
  switch (buttonCommand)
  {
    case TS_CMD_TEST_DSBL: // cmd is stop
      currentStatus.isTestModeActive = false;
      stopAllCoilsCharging();
      closeAllInjectors();
      HWTest_INJ_Pulsed = 0;
      HWTest_IGN_Pulsed = 0;
      break;

    case TS_CMD_TEST_ENBL: // cmd is enable
      currentStatus.isTestModeActive = true;
      break;

    case TS_CMD_INJ1_ON:
    case TS_CMD_INJ2_ON:
    case TS_CMD_INJ3_ON:
    case TS_CMD_INJ4_ON:
    case TS_CMD_INJ5_ON:
    case TS_CMD_INJ6_ON:
    case TS_CMD_INJ7_ON:
    case TS_CMD_INJ8_ON:
      injectorOn(currentStatus, computeChannel(buttonCommand, TS_CMD_INJ2_ON, TS_CMD_INJ1_ON));
      break;

    case TS_CMD_INJ1_OFF:
    case TS_CMD_INJ2_OFF:
    case TS_CMD_INJ3_OFF:
    case TS_CMD_INJ4_OFF:
    case TS_CMD_INJ5_OFF:
    case TS_CMD_INJ6_OFF:
    case TS_CMD_INJ7_OFF:
    case TS_CMD_INJ8_OFF:
      injectorOff(currentStatus, computeChannel(buttonCommand, TS_CMD_INJ2_OFF, TS_CMD_INJ1_OFF));
      break;

    case TS_CMD_INJ1_PULSED:
    case TS_CMD_INJ2_PULSED:
    case TS_CMD_INJ3_PULSED:
    case TS_CMD_INJ4_PULSED:
    case TS_CMD_INJ5_PULSED:
    case TS_CMD_INJ6_PULSED:
    case TS_CMD_INJ7_PULSED:
    case TS_CMD_INJ8_PULSED:
      injectorPulse(currentStatus, computeChannel(buttonCommand, TS_CMD_INJ2_PULSED, TS_CMD_INJ1_PULSED));
      break;

    case TS_CMD_IGN1_ON: // cmd group is for spark1 on actions
      if( currentStatus.isTestModeActive ){ beginCoilCharge(1); }
      break;

    case TS_CMD_IGN1_OFF: // cmd group is for spark1 off actions
        if( currentStatus.isTestModeActive ) { endCoilCharge(1); BIT_CLEAR(HWTest_IGN_Pulsed, IGN1_CMD_BIT); }
      break;

    case TS_CMD_IGN1_PULSED: // cmd group is for spark1 50%dc actions
      if( currentStatus.isTestModeActive ) { BIT_SET(HWTest_IGN_Pulsed, IGN1_CMD_BIT); }
      if(!BIT_CHECK(HWTest_IGN_Pulsed, IGN1_CMD_BIT)) { endCoilCharge(1); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN2_ON: // cmd group is for spark2 on actions
      if( currentStatus.isTestModeActive ) { beginCoilCharge(2); }
      break;

    case TS_CMD_IGN2_OFF: // cmd group is for spark2 off actions
      if( currentStatus.isTestModeActive ) { endCoilCharge(2); BIT_CLEAR(HWTest_IGN_Pulsed, IGN2_CMD_BIT); }
      break;

    case TS_CMD_IGN2_PULSED: // cmd group is for spark2 50%dc actions
      if( currentStatus.isTestModeActive ) { BIT_SET(HWTest_IGN_Pulsed, IGN2_CMD_BIT); }
      if(!BIT_CHECK(HWTest_IGN_Pulsed, IGN2_CMD_BIT)) { endCoilCharge(2); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN3_ON: // cmd group is for spark3 on actions
      if( currentStatus.isTestModeActive ) { beginCoilCharge(3); }
      break;

    case TS_CMD_IGN3_OFF: // cmd group is for spark3 off actions
      if( currentStatus.isTestModeActive ) { endCoilCharge(3); BIT_CLEAR(HWTest_IGN_Pulsed, IGN3_CMD_BIT); }
      break;

    case TS_CMD_IGN3_PULSED: // cmd group is for spark3 50%dc actions
      if( currentStatus.isTestModeActive ) { BIT_SET(HWTest_IGN_Pulsed, IGN3_CMD_BIT); }
      if(!BIT_CHECK(HWTest_IGN_Pulsed, IGN3_CMD_BIT)) { endCoilCharge(3); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN4_ON: // cmd group is for spark4 on actions
      if( currentStatus.isTestModeActive ) { beginCoilCharge(4); }
      break;

    case TS_CMD_IGN4_OFF: // cmd group is for spark4 off actions
      if( currentStatus.isTestModeActive ) { endCoilCharge(4); BIT_CLEAR(HWTest_IGN_Pulsed, IGN4_CMD_BIT); }
      break;

    case TS_CMD_IGN4_PULSED: // cmd group is for spark4 50%dc actions
      if( currentStatus.isTestModeActive ) { BIT_SET(HWTest_IGN_Pulsed, IGN4_CMD_BIT); }
      if(!BIT_CHECK(HWTest_IGN_Pulsed, IGN4_CMD_BIT)) { endCoilCharge(4); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN5_ON: // cmd group is for spark5 on actions
      if( currentStatus.isTestModeActive ) { beginCoilCharge(5); }
      break;

    case TS_CMD_IGN5_OFF: // cmd group is for spark5 off actions
      if( currentStatus.isTestModeActive ) { endCoilCharge(5); BIT_CLEAR(HWTest_IGN_Pulsed, IGN5_CMD_BIT); }
      break;

    case TS_CMD_IGN5_PULSED: // cmd group is for spark4 50%dc actions
      if( currentStatus.isTestModeActive ) { BIT_SET(HWTest_IGN_Pulsed, IGN5_CMD_BIT); }
      if(!BIT_CHECK(HWTest_IGN_Pulsed, IGN5_CMD_BIT)) { endCoilCharge(5); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN6_ON: // cmd group is for spark6 on actions
      if( currentStatus.isTestModeActive ) { beginCoilCharge(6); }
      break;

    case TS_CMD_IGN6_OFF: // cmd group is for spark6 off actions
      if( currentStatus.isTestModeActive ) { endCoilCharge(6); BIT_CLEAR(HWTest_IGN_Pulsed, IGN6_CMD_BIT); }
      break;

    case TS_CMD_IGN6_PULSED: // cmd group is for spark6 50%dc actions
      if( currentStatus.isTestModeActive ) { BIT_SET(HWTest_IGN_Pulsed, IGN6_CMD_BIT); }
      if(!BIT_CHECK(HWTest_IGN_Pulsed, IGN6_CMD_BIT)) { endCoilCharge(6); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN7_ON: // cmd group is for spark7 on actions
      if( currentStatus.isTestModeActive ) { beginCoilCharge(7); }
      break;

    case TS_CMD_IGN7_OFF: // cmd group is for spark7 off actions
      if( currentStatus.isTestModeActive ) { endCoilCharge(7); BIT_CLEAR(HWTest_IGN_Pulsed, IGN7_CMD_BIT); }
      break;

    case TS_CMD_IGN7_PULSED: // cmd group is for spark7 50%dc actions
      if( currentStatus.isTestModeActive ) { BIT_SET(HWTest_IGN_Pulsed, IGN7_CMD_BIT); }
      if(!BIT_CHECK(HWTest_IGN_Pulsed, IGN7_CMD_BIT)) { endCoilCharge(7); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    case TS_CMD_IGN8_ON: // cmd group is for spark8 on actions
      if( currentStatus.isTestModeActive ) { beginCoilCharge(8); }
      break;

    case TS_CMD_IGN8_OFF: // cmd group is for spark8 off actions
      if( currentStatus.isTestModeActive ) { endCoilCharge(8); BIT_CLEAR(HWTest_IGN_Pulsed, IGN8_CMD_BIT); }
      break;

    case TS_CMD_IGN8_PULSED: // cmd group is for spark8 50%dc actions
      if( currentStatus.isTestModeActive ) { BIT_SET(HWTest_IGN_Pulsed, IGN8_CMD_BIT); }
      if(!BIT_CHECK(HWTest_IGN_Pulsed, IGN8_CMD_BIT)) { endCoilCharge(8); } //Ensure this output is turned off (Otherwise the output may stay on permanently)
      break;

    //VSS Calibration routines
    case TS_CMD_VSS_60KMH:
      {
        if(configPage2.vssMode == VSS_MODE_INTERNAL_PIN)
        {
          //Calculate the ratio of VSS reading from Aux/CAN input and actual VSS (assuming that actual VSS is really 60km/h).
          configPage2.vssPulsesPerKm = (currentStatus.canin[configPage2.vssAuxCh] / 60);
          savePage(veSetPage); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
          currentStatus.vssUiRefresh = true;
        }
        else
        {
          //Calibrate the actual pulses per distance
          uint32_t calibrationGap = vssGetPulseGap(0);
          if( calibrationGap > 0 )
          {
            configPage2.vssPulsesPerKm = MICROS_PER_MIN / calibrationGap;
            savePage(veSetPage); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
            currentStatus.vssUiRefresh = true;
          }
        }
      }
      break;

    //Calculate the RPM to speed ratio for each gear
    case TS_CMD_VSS_RATIO1:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio1 = (currentStatus.vss * 10000UL) / currentStatus.RPM;
        savePage(1); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
        currentStatus.vssUiRefresh = true;
      }
      break;

    case TS_CMD_VSS_RATIO2:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio2 = (currentStatus.vss * 10000UL) / currentStatus.RPM;
        savePage(1); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
        currentStatus.vssUiRefresh = true;
      }
      break;

    case TS_CMD_VSS_RATIO3:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio3 = (currentStatus.vss * 10000UL) / currentStatus.RPM;
        savePage(1); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
        currentStatus.vssUiRefresh = true;
      }
      break;

    case TS_CMD_VSS_RATIO4: 
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio4 = (currentStatus.vss * 10000UL) / currentStatus.RPM;
        savePage(1); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
        currentStatus.vssUiRefresh = true;
      }
      break;

    case TS_CMD_VSS_RATIO5:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio5 = (currentStatus.vss * 10000UL) / currentStatus.RPM;
        savePage(1); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
        currentStatus.vssUiRefresh = true;
      }
      break;

    case TS_CMD_VSS_RATIO6:
      if(currentStatus.vss > 0)
      {
        configPage2.vssRatio6 = (currentStatus.vss * 10000UL) / currentStatus.RPM;
        savePage(1); // Need to manually save the new config value as it will not trigger a burn in tunerStudio due to use of ControllerPriority
        currentStatus.vssUiRefresh = true;
      }
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
