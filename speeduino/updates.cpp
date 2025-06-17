/** @file
 * EEPROM Storage updates.
 */
/** Store and load various configs to/from EEPROM considering the the data format versions of various SW generations.
 * This routine is used for doing any data conversions that are required during firmware changes.
 * This prevents users getting difference reports in TS when such a data change occurs.
 * It also can be used for setting good values when there are variables that move locations in the ini.
 * When a user skips multiple firmware versions at a time, this will roll through the updates 1 at a time.
 * The doUpdates() uses may lower level routines from Arduino EEPROM library and storage.ino to carry out EEPROM storage tasks.
 */
#include "globals.h"
#include "storage.h"
#include "sensors.h"
#include "updates.h"
#include "pages.h"
#include "comms_CAN.h"
#include "units.h"
#include "preprocessor.h"

#if defined(CORE_AVR)
#pragma GCC push_options
// This minimizes flash usage - code here is not performance critical
// since it's really only run once per firmware flash.
#pragma GCC optimize ("Os") 
#endif

static void tableValueAdd(table_row_iterator &row, table3d_value_t addValue) {  // cppcheck-suppress [constParameter,constParameterCallback]
  *row = *row + addValue; 
}

static void tableValueMultiply(table_row_iterator &row, table3d_value_t multiplier) {  // cppcheck-suppress [constParameter,constParameterCallback]
  *row = *row * multiplier; 
}

static void tableAxisMultiply(table_axis_iterator &axis, table3d_axis_t multiplier) {  // cppcheck-suppress [constParameter,constParameterCallback]
  *axis = *axis * multiplier; 
}

static void tableAxisDivide(table_axis_iterator &axis, table3d_axis_t divisor) {  // cppcheck-suppress [constParameter,constParameterCallback]
  *axis = *axis / divisor; 
}

void doUpdates(void)
{
  #define CURRENT_DATA_VERSION    25U
  //Only the latest update for small flash devices must be retained
   #ifndef SMALL_FLASH_MODE

  //May 2017 firmware introduced a -40 offset on the ignition table. Update that table to +40
  if (loadEEPROMVersion() == 2U)
  {
    for_each(ignitionTable.values.begin(), tableValueAdd, (table3d_value_t)40U);
    saveAllPages();
    saveEEPROMVersion(3);
  }
  //June 2017 required the forced addition of some CAN values to avoid weird errors
  if (loadEEPROMVersion() == 3U)
  {
    configPage9.speeduino_tsCanId = 0;
    configPage9.true_address = 256;
    configPage9.realtime_base_address = 336;

    //There was a bad value in the May base tune for the spark duration setting, fix it here if it's a problem
    if(configPage4.sparkDur == UINT8_MAX) { configPage4.sparkDur = 10; }

    saveAllPages();
    saveEEPROMVersion(4);
  }
  //July 2017 adds a cranking enrichment curve in place of the single value. This converts that single value to the curve
  if (loadEEPROMVersion() == 4U)
  {
    //Some default values for the bins (Doesn't matter too much here as the values against them will all be identical)
    configPage10.crankingEnrichBins[0] = 0;
    configPage10.crankingEnrichBins[1] = 40;
    configPage10.crankingEnrichBins[2] = 70;
    configPage10.crankingEnrichBins[3] = 100;

    configPage10.crankingEnrichValues[0] = 100U + configPage2.crankingPct;
    configPage10.crankingEnrichValues[1] = 100U + configPage2.crankingPct;
    configPage10.crankingEnrichValues[2] = 100U + configPage2.crankingPct;
    configPage10.crankingEnrichValues[3] = 100U + configPage2.crankingPct;

    saveAllPages();
    saveEEPROMVersion(5);
  }
  //September 2017 had a major change to increase the minimum table size to 128. This required multiple pieces of data being moved around
  if (loadEEPROMVersion() == 5U)
  {
    //Data after page 4 has to move up 128 bytes
    static constexpr uint16_t PAGE5_V5_START = 815;
    static constexpr uint16_t PAGE5_V5_SHIFT_DISTANCE = 128;
    static constexpr uint16_t PAGE5_V5_BLOCK_SIZE = 1152;
    moveBlock(getStorageAPI(), PAGE5_V5_START+PAGE5_V5_SHIFT_DISTANCE, PAGE5_V5_START, PAGE5_V5_BLOCK_SIZE);

    //The remaining data only has to move up 64 bytes
    static constexpr uint16_t OTHER_V5_START = 527;
    static constexpr uint16_t OTHER_V5_SHIFT_DISTANCE = 64;
    static constexpr uint16_t OTHER_V5_BLOCK_SIZE = 352;
    moveBlock(getStorageAPI(), OTHER_V5_START+OTHER_V5_SHIFT_DISTANCE, OTHER_V5_START, OTHER_V5_BLOCK_SIZE);

    saveEEPROMVersion(6);
    loadAllPages(); //Reload the config after changing everything in EEPROM
  }
  //November 2017 added the staging table that comes after boost and vvt in the EEPROM. This required multiple pieces of data being moved around
  if (loadEEPROMVersion() == 6U)
  {
    //Data after page 8 has to move up 82 bytes
    static constexpr uint16_t PAGE9_V6_START = 1484;
    static constexpr uint16_t PAGE9_V6_SHIFT_DISTANCE = 82;
    static constexpr uint16_t PAGE9_V6_BLOCK_SIZE = 529;
    moveBlock(getStorageAPI(), PAGE9_V6_START+PAGE9_V6_SHIFT_DISTANCE, PAGE9_V6_START, PAGE9_V6_BLOCK_SIZE);

    saveEEPROMVersion(7);
    loadAllPages(); //Reload the config after changing everything in EEPROM
  }

  if (loadEEPROMVersion() == 7U) {
    //Convert whatever flex fuel settings are there into the new tables

    configPage10.flexBoostBins[0] = 0;
    configPage10.flexBoostAdj[0]  = configPage2.aeColdPct;

    configPage10.flexFuelBins[0] = 0;
    configPage10.flexFuelAdj[0]  = configPage2.idleUpPin;

    configPage10.flexAdvBins[0] = 0;
    configPage10.flexAdvAdj[0]  = configPage2.aeTaperMin;

    for (uint8_t x = 1U; x < 6U; x++)
    {
      uint8_t pct = x * 20U;
      configPage10.flexBoostBins[x] = pct;
      configPage10.flexFuelBins[x] = pct;
      configPage10.flexAdvBins[x] = pct;

      int16_t boostAdder = ((((int16_t)configPage2.aeColdTaperMin - (int16_t)configPage2.aeColdPct) * (int16_t)pct) / 100) + (int16_t)configPage2.aeColdPct;
      configPage10.flexBoostAdj[x] = boostAdder;

      uint8_t fuelAdder = (((configPage2.idleUpAdder - configPage2.idleUpPin) * pct) / 100U) + configPage2.idleUpPin;
      configPage10.flexFuelAdj[x] = fuelAdder;

      uint8_t advanceAdder = (((configPage2.aeTaperMax - configPage2.aeTaperMin) * pct) / 100U) + configPage2.aeTaperMin;
      configPage10.flexAdvAdj[x] = advanceAdder;
    }

    saveAllPages();
    saveEEPROMVersion(8);
  }

  if (loadEEPROMVersion() == 8U)
  {
    //May 2018 adds separate load sources for fuel and ignition. Copy the existing load algorithm into Both
    configPage2.fuelAlgorithm = (LoadSource)configPage2.legacyMAP; //Was configPage2.unused2_38c
    configPage2.ignAlgorithm = (LoadSource)configPage2.legacyMAP; //Was configPage2.unused2_38c

    //Add option back in for open or closed loop boost. For all current configs to use closed
    configPage4.boostType = 1;

    saveAllPages();
    saveEEPROMVersion(9);
  }

  if (loadEEPROMVersion() == 9U)
  {
    //October 2018 set default values for all the aux in variables (These were introduced in Aug, but no defaults were set then)
    //All aux channels set to Off
    for (uint8_t AuxinChan = 0; AuxinChan <_countof(configPage9.caninput_sel); AuxinChan++)
    {
      configPage9.caninput_sel[AuxinChan] = 0;
    }

    //Ability to change the analog filter values was added. Set default values for these:
    configPage4.ADCFILTER_TPS  = ADCFILTER_TPS_DEFAULT;
    configPage4.ADCFILTER_CLT  = ADCFILTER_CLT_DEFAULT;
    configPage4.ADCFILTER_IAT  = ADCFILTER_IAT_DEFAULT;
    configPage4.ADCFILTER_O2   = ADCFILTER_O2_DEFAULT;
    configPage4.ADCFILTER_BAT  = ADCFILTER_BAT_DEFAULT;
    configPage4.ADCFILTER_MAP  = ADCFILTER_MAP_DEFAULT;
    configPage4.ADCFILTER_BARO = ADCFILTER_BARO_DEFAULT;

    saveAllPages();
    saveEEPROMVersion(10);
  }

  if (loadEEPROMVersion() == 10U)
  {
    //May 2019 version adds the use of a 2D table for the priming pulse rather than a single value.
    //This sets all the values in the 2D table to be the same as the previous single value
    configPage2.primePulse[0] = configPage2.aeColdTaperMax / 5U; //New priming pulse values are in the range 0-127.5 rather than 0-25.5 so they must be divided by 5
    configPage2.primePulse[1] = configPage2.aeColdTaperMax / 5U; //New priming pulse values are in the range 0-127.5 rather than 0-25.5 so they must be divided by 5
    configPage2.primePulse[2] = configPage2.aeColdTaperMax / 5U; //New priming pulse values are in the range 0-127.5 rather than 0-25.5 so they must be divided by 5
    configPage2.primePulse[3] = configPage2.aeColdTaperMax / 5U; //New priming pulse values are in the range 0-127.5 rather than 0-25.5 so they must be divided by 5
    //Set some sane default temperatures for this table
    configPage2.primeBins[0] = 0;
    configPage2.primeBins[1] = 40;
    configPage2.primeBins[2] = 70;
    configPage2.primeBins[3] = 100;

    //Also added is coolant based ASE for both duration and amount
    //All the adder amounts are set to what the single value was previously
    configPage2.asePct[0] = configPage2.aeColdTaperMin;
    configPage2.asePct[1] = configPage2.aeColdTaperMin;
    configPage2.asePct[2] = configPage2.aeColdTaperMin;
    configPage2.asePct[3] = configPage2.aeColdTaperMin;
    //ASE duration is set to 10s for all coolant values
    configPage2.aseCount[0] = 10;
    configPage2.aseCount[1] = 10;
    configPage2.aseCount[2] = 10;
    configPage2.aseCount[3] = 10;
    //Finally the coolant bins for the above are set to sane values (Remembering these are offset values)
    configPage2.aseBins[0] = 0;
    configPage2.aseBins[1] = 20;
    configPage2.aseBins[2] = 60;
    configPage2.aseBins[3] = 80;

    //Coolant based ignition advance was added also. Set sane values
    configPage4.cltAdvBins[0] = 0;
    configPage4.cltAdvBins[1] = 30;
    configPage4.cltAdvBins[2] = 60;
    configPage4.cltAdvBins[3] = 70;
    configPage4.cltAdvBins[4] = 85;
    configPage4.cltAdvBins[5] = 100;
    configPage4.cltAdvValues[0] = 0;
    configPage4.cltAdvValues[1] = 0;
    configPage4.cltAdvValues[2] = 0;
    configPage4.cltAdvValues[3] = 0;
    configPage4.cltAdvValues[4] = 0;
    configPage4.cltAdvValues[5] = 0;


    //March 19 added a tacho pulse duration that could default to stupidly high values. Check if this is the case and fix it if found. 6ms is the maximum allowed value
    if(configPage2.tachoDuration > 6U) { configPage2.tachoDuration = 3; }

    //MAP based AE was introduced, force the AE mode to be TPS for all existing tunes
    configPage2.aeMode = AE_MODE_TPS;
    configPage2.maeThresh = configPage2.taeThresh;
    //Set some sane values for the MAP AE curve
    configPage4.maeRates[0] = 75;
    configPage4.maeRates[1] = 75;
    configPage4.maeRates[2] = 75;
    configPage4.maeRates[3] = 75;
    configPage4.maeBins[0] = 7;
    configPage4.maeBins[1] = 12;
    configPage4.maeBins[2] = 20;
    configPage4.maeBins[3] = 40;

    //The 2nd fuel table was added. To prevent issues, force it to be disabled.
    configPage10.fuel2Mode = 0;


    saveAllPages();
    saveEEPROMVersion(11);
  }

  if (loadEEPROMVersion() == 11U)
  {
    //Sep 2019
    //A battery calibration offset value was introduced. Set default value to 0
    configPage4.batVoltCorrect = 0;

    //An option was added to select the older method of performing MAP reads with the pullup resistor active
    configPage2.legacyMAP = 0;

    //Secondary fuel table was added for switching. Make sure it's all turned off initially
    configPage10.fuel2Mode = 0;
    configPage10.fuel2SwitchVariable = 0; //Set switch variable to RPM
    configPage10.fuel2SwitchValue = 7000; //7000 RPM switch point is safe

    saveAllPages();
    saveEEPROMVersion(12);
  }

  if (loadEEPROMVersion() == 12U)
  {
    //Nov 2019

    //Manual baro correction curve was added. Give it some default values (All baro readings set to 100%)
    configPage4.baroFuelBins[0] = 80;
    configPage4.baroFuelBins[1] = 85;
    configPage4.baroFuelBins[2] = 90;
    configPage4.baroFuelBins[3] = 95;
    configPage4.baroFuelBins[4] = 100;
    configPage4.baroFuelBins[5] = 105;
    configPage4.baroFuelBins[6] = 110;
    configPage4.baroFuelBins[7] = 115;

    configPage4.baroFuelValues[0] = 100;
    configPage4.baroFuelValues[1] = 100;
    configPage4.baroFuelValues[2] = 100;
    configPage4.baroFuelValues[3] = 100;
    configPage4.baroFuelValues[4] = 100;
    configPage4.baroFuelValues[5] = 100;
    configPage4.baroFuelValues[6] = 100;
    configPage4.baroFuelValues[7] = 100;

    //Idle advance curve was added. Add default values
    configPage2.idleAdvEnabled = 0; //Turn this off by default
    configPage2.idleAdvTPS = 5; //Active below 5% tps
    configPage2.idleAdvRPM = 20; //Active below 2000 RPM
    configPage4.idleAdvBins[0] = 30;
    configPage4.idleAdvBins[1] = 40;
    configPage4.idleAdvBins[2] = 50;
    configPage4.idleAdvBins[3] = 60;
    configPage4.idleAdvBins[4] = 70;
    configPage4.idleAdvBins[5] = 80;
    configPage4.idleAdvValues[0] = 15; //These values offset by 15, so this is just making this equal to 0
    configPage4.idleAdvValues[1] = 15;
    configPage4.idleAdvValues[2] = 15;
    configPage4.idleAdvValues[3] = 15;
    configPage4.idleAdvValues[4] = 15;
    configPage4.idleAdvValues[5] = 15;

    saveAllPages();
    saveEEPROMVersion(13);
  }

  if (loadEEPROMVersion() == 13U)
  {
    //202005
    //Cranking enrichment range 0..1275% instead of older 0.255, so need to divide old values by 5
    configPage10.crankingEnrichValues[0] = configPage10.crankingEnrichValues[0] / 5U;
    configPage10.crankingEnrichValues[1] = configPage10.crankingEnrichValues[1] / 5U;
    configPage10.crankingEnrichValues[2] = configPage10.crankingEnrichValues[2] / 5U;
    configPage10.crankingEnrichValues[3] = configPage10.crankingEnrichValues[3] / 5U;

    //Added the injector timing curve
    //Set all the values to be the same as the first one. 
    configPage2.injAng[0] = configPage2.injAng[0];  // cppcheck-suppress selfAssignment; Obviously not needed, but here for completeness
    configPage2.injAng[1] = configPage2.injAng[0];
    configPage2.injAng[2] = configPage2.injAng[0];
    configPage2.injAng[3] = configPage2.injAng[0];
    //The RPMs are divided by 100
    configPage2.injAngRPM[0] = 5;
    configPage2.injAngRPM[1] = 25;
    configPage2.injAngRPM[2] = 45;
    configPage2.injAngRPM[3] = 65;

    //Introduced a DFCO delay option. Default it to 0
    configPage2.dfcoDelay = 0;
    //Introduced a minimum temperature for DFCO. Default it to 40C
    configPage2.dfcoMinCLT = temperatureAddOffset(40);

    //Update flex fuel ignition config values for 40 degrees offset
    for (uint8_t i=0; i<_countof(configPage10.flexAdvAdj); i++)
    {
      configPage10.flexAdvAdj[i] += 40U;
    }
    
    //AE cold modifier added. Default to sane values
    configPage2.aeColdPct = 100;
    configPage2.aeColdTaperMin = 40;
    configPage2.aeColdTaperMax = 100;

    //New PID resolution, old resolution was 100% for each increase, 100% now is stored as 32
    auto pidCorrection = [](uint8_t old) { 
       return old >= 8U ? UINT8_MAX : old<<5U;
    };
    configPage6.idleKP = pidCorrection(configPage6.idleKP);
    configPage6.idleKI = pidCorrection(configPage6.idleKI);
    configPage6.idleKD = pidCorrection(configPage6.idleKD);
    configPage10.vvtCLKP = pidCorrection(configPage10.vvtCLKP);
    configPage10.vvtCLKI = pidCorrection(configPage10.vvtCLKI);
    configPage10.vvtCLKD = pidCorrection(configPage10.vvtCLKD);

    //Cranking enrichment to run taper added. Default it to 0,1 secs
    configPage10.crankingEnrichTaper = 1;
    
    //ASE to run taper added. Default it to 0,1 secs
    configPage2.aseTaperTime = 1;

    // there is now option for fixed and relative timing retard for soft limit. This sets the soft limiter to the old fixed timing mode.
    configPage2.SoftLimitMode = SOFT_LIMIT_FIXED;

    //VSS was added for testing, disable it by default
    configPage2.vssMode = 0;

    saveAllPages();
    saveEEPROMVersion(14);
  }

  if (loadEEPROMVersion() == 14U)
  {
    //202008

    //MAJOR update to move the coolant, IAT and O2 calibrations to 2D tables
    
    //These were the values used previously when all calibration tables were 512 long. They need to be retained so the update process (202005 -> 202008) can work
    constexpr uint16_t EEPROM_CALIBRATION_O2_OLD = 2559;
    constexpr uint16_t EEPROM_CALIBRATION_IAT_OLD = 3071;
    constexpr uint16_t EEPROM_CALIBRATION_CLT_OLD = 3583;

    for(uint16_t x=0; x<(CALIBRATION_TABLE_SIZE/16U); x++) //Each calibration table is 512 bytes long
    {
      uint16_t y = EEPROM_CALIBRATION_CLT_OLD + (x * 16U);
      cltCalibrationTable.values[x] = getStorageAPI().read(y);
      cltCalibrationTable.axis[x] = (x * 32U);

      y = EEPROM_CALIBRATION_IAT_OLD + (x * 16u);
      iatCalibrationTable.values[x] = getStorageAPI().read(y);
      iatCalibrationTable.axis[x] = (x * 32U);

      y = EEPROM_CALIBRATION_O2_OLD + (x * 16U);
      o2CalibrationTable.values[x] = getStorageAPI().read(y);
      o2CalibrationTable.axis[x] = (x * 32U);
    }
    saveAllCalibrationTables();

    //Oil and fuel pressure inputs were introduced. Disable them both by default
    configPage10.oilPressureProtEnbl = false;
    configPage10.oilPressureEnable = false;
    configPage10.fuelPressureEnable = false;
    
    //wmi
    configPage10.wmiEnabled = 0;
    configPage10.wmiMode = 0;
    configPage10.wmiOffset = 0;
    configPage10.wmiIndicatorEnabled = 0;
    configPage10.wmiEmptyEnabled = 0;
    configPage10.wmiAdvEnabled = 0;
    for(int i=0; i<6; i++)
    {
      configPage10.wmiAdvBins[i] = i*100/2;
      configPage10.wmiAdvAdj[i] = OFFSET_IGNITION;
    }

    //Programmable outputs added. Set all to disabled
    configPage13.outputPin[0] = 0;
    configPage13.outputPin[1] = 0;
    configPage13.outputPin[2] = 0;
    configPage13.outputPin[3] = 0;
    configPage13.outputPin[4] = 0;
    configPage13.outputPin[5] = 0;
    configPage13.outputPin[6] = 0;
    configPage13.outputPin[7] = 0;

    //New multiply MAP option added. Set new option to be the same as old
    configPage2.multiplyMAP = configPage2.crkngAddCLTAdv;
    //New AE option added to allow for PW added in addition to existing PW multiply
    configPage2.aeApplyMode = 0; //Set the AE mode to Multiply

    //Injector priming delay added
    configPage2.primingDelay = 0;
    //ASE taper time added
    configPage2.aseTaperTime = 10; //1 second taper

    saveAllPages();
    saveEEPROMVersion(15);
  }

  if (loadEEPROMVersion() == 15U)
  {
    //202012
    configPage10.spark2Mode = 0; //Disable 2nd spark table

    saveAllPages();
    saveEEPROMVersion(16);
  }

  //Move this #endif to only do latest updates to safe ROM space on small devices.
  #endif
  if (loadEEPROMVersion() == 16U)
  {
    //Fix for wrong placed page 13
    static constexpr int PAGE14_V16_END = 2998;
    static constexpr int PAGE14_V16_START = 2580;
    static constexpr int PAGE14_V16_BLOCK_SIZE = PAGE14_V16_START - PAGE14_V16_END;
    static constexpr int PAGE14_V16_SHIFT_DISTANCE = 112;
    moveBlock(getStorageAPI(), PAGE14_V16_START+PAGE14_V16_SHIFT_DISTANCE, PAGE14_V16_START, PAGE14_V16_BLOCK_SIZE);

    configPage6.iacPWMrun = false; // just in case. This should be false anyways, but sill.
    configPage2.useDwellMap = 0; //Dwell map added, use old fixed value as default

    saveAllPages();
    saveEEPROMVersion(17);
  }

  if (loadEEPROMVersion() == 17U)
  {
    //VVT stuff has now 0.5 accuracy, so shift values in vvt table by one.
    for_each(vvtTable.values.begin(), tableValueMultiply, (table3d_value_t)2U);

    configPage10.vvtCLholdDuty = configPage10.vvtCLholdDuty << 1;
    configPage10.vvtCLminDuty = configPage10.vvtCLminDuty << 1;
    configPage10.vvtCLmaxDuty = configPage10.vvtCLmaxDuty << 1;

    //VVT2 added, so default values and disable it
    configPage10.vvt2Enabled = 0;
    configPage4.vvt2PWMdir = 0;
    configPage10.TrigEdgeThrd = 0;

    //Old use as On/Off selection is removed, so change VVT mode to On/Off based on that
    if(configPage6.tachoMode == 1U) { configPage6.vvtMode = VVT_MODE_ONOFF; }

    //Closed loop VVT improvements. Set safety limits to max/min working values and filter to minimum.
    configPage10.vvtCLMinAng = 0U;
    configPage10.vvtCLMaxAng = 200U;
    configPage4.ANGLEFILTER_VVT = 0U;

    configPage2.idleAdvDelay *= 2U; //Increased resolution to 0.5 second
    
    //RPM switch point added for map sample method. Set to 0 to not affect existing tunes.
    configPage2.mapSwitchPoint = 0;

    configPage9.boostByGearEnabled = 0;

    //Added possibility to set minimum programmable output time
    configPage13.outputTimeLimit[0] = 0;
    configPage13.outputTimeLimit[1] = 0;
    configPage13.outputTimeLimit[2] = 0;
    configPage13.outputTimeLimit[3] = 0;
    configPage13.outputTimeLimit[4] = 0;
    configPage13.outputTimeLimit[5] = 0;
    configPage13.outputTimeLimit[6] = 0;
    configPage13.outputTimeLimit[7] = 0;

    saveAllPages();
    saveEEPROMVersion(18);
  }

  if (loadEEPROMVersion() == 18U)
  {
    //202202
    configPage2.fanEnable = configPage6.fanUnused; // PWM Fan mode added, but take the previous setting of Fan in use.

    //TPS resolution increased to 0.5%
    //configPage2.taeThresh *= 2;
    configPage2.idleAdvTPS *= 2;
    configPage2.iacTPSlimit *= 2;
    configPage4.floodClear *= 2;
    configPage4.dfcoTPSThresh *= 2;
    configPage6.egoTPSMax *= 2;
    configPage10.lnchCtrlTPS *= 2;
    configPage10.wmiTPS *= 2;
    configPage10.n2o_minTPS *= 2;
    if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_TPS) { configPage10.fuel2SwitchValue *= 2U; }
    if(configPage10.spark2SwitchVariable == SPARK2_CONDITION_TPS) { configPage10.spark2SwitchVariable *= 2U; }

    // Each table Y axis need to be updated as well if TPS is the source
    if(configPage2.fuelAlgorithm == LOAD_SOURCE_TPS)
    {
      for_each(fuelTable.axisY.begin(),  tableAxisMultiply, (table3d_axis_t)4U);
      for_each(afrTable.axisY.begin(),   tableAxisMultiply, (table3d_axis_t)4U);
      for_each(trim1Table.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U);
      for_each(trim2Table.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U);
      for_each(trim3Table.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U);
      for_each(trim4Table.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U);
      for_each(trim5Table.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U);
      for_each(trim6Table.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U);
      for_each(trim7Table.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U);
      for_each(trim8Table.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U);
      if(configPage4.sparkMode == IGN_MODE_ROTARY)
      { 
        for(uint8_t x = 0; x < _countof(configPage10.rotarySplitBins); x++)
        {
          configPage10.rotarySplitBins[x] *= 2U;
        }
      }
    }
    if(configPage2.ignAlgorithm == LOAD_SOURCE_TPS) { for_each(ignitionTable.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U); }
    if(configPage10.fuel2Algorithm == LOAD_SOURCE_TPS) { for_each(fuelTable2.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U); }
    if(configPage10.spark2Algorithm == LOAD_SOURCE_TPS) { for_each(ignitionTable2.axisY.begin(), tableAxisMultiply, (table3d_axis_t)4U); }

    for_each(boostTable.axisY.begin(), tableAxisMultiply, (table3d_axis_t)2U); // Boost table used 1.0 previously, so it only needs a 2x multiplier
    if(configPage6.vvtLoadSource == VVT_LOAD_TPS)
    {
      //NOTE: The VVT tables all had 1.0 as the multiply value rather than 2.0 used in all other tables. For this reason they only need to be multiplied by 2 when updating
      for_each(vvtTable.axisY.begin(),  tableAxisMultiply, (table3d_axis_t)2U);
      for_each(vvt2Table.axisY.begin(), tableAxisMultiply, (table3d_axis_t)2U);
    }
    else
    {
      //NOTE: The VVT tables all had 1.0 as the multiply value rather than 2.0 used in all other tables. For this reason they need to be divided by 2 when updating
      for_each(vvtTable.axisY.begin(),  tableAxisDivide, (table3d_axis_t)2U);
      for_each(vvt2Table.axisY.begin(), tableAxisDivide, (table3d_axis_t)2U);
    }


    configPage4.vvtDelay = 0;
    configPage4.vvtMinClt = 0;

    //Set SD logging related settings to zero.
    configPage13.onboard_log_csv_separator = 0;
    configPage13.onboard_log_file_style = 0;
    configPage13.onboard_log_file_rate = 0;
    configPage13.onboard_log_filenaming = 0;
    configPage13.onboard_log_storage = 0;
    configPage13.onboard_log_trigger_boot = 0;
    configPage13.onboard_log_trigger_RPM = 0;
    configPage13.onboard_log_trigger_prot = 0;
    configPage13.onboard_log_trigger_Vbat = 0;
    configPage13.onboard_log_trigger_Epin = 0;
    configPage13.onboard_log_tr1_duration = 0;
    configPage13.onboard_log_tr2_thr_on = 0;
    configPage13.onboard_log_tr2_thr_off = 0;
    configPage13.onboard_log_tr3_thr_RPM = 0;
    configPage13.onboard_log_tr3_thr_MAP = 0;
    configPage13.onboard_log_tr3_thr_Oil = 0;
    configPage13.onboard_log_tr3_thr_AFR = 0;
    configPage13.onboard_log_tr4_thr_on = 0;
    configPage13.onboard_log_tr4_thr_off = 0;
    configPage13.onboard_log_tr5_Epin_pin = 0;

    saveAllPages();
    saveEEPROMVersion(19);
  }
  
  if (loadEEPROMVersion() == 19U)
  {
    //202207

    //Option added to select injector pairing on 4 cylinder engines
    if( configPage4.inj4cylPairing > INJ_PAIR_14_23 ) { configPage4.inj4cylPairing = 0; } //Check valid value
    if( configPage2.nCylinders == 4U )
    {
      if ( configPage2.injLayout == INJ_SEQUENTIAL ) { configPage4.inj4cylPairing = INJ_PAIR_13_24; } //Since #478 engine will always start in semi, make the sequence right for the majority of inlie 4 engines
      else { configPage4.inj4cylPairing = INJ_PAIR_14_23; } //Force setting to use the default mode from previous FW versions. This is to prevent issues on any setups that have been wired accordingly
    }

    configPage9.hardRevMode = 1; //Set hard rev limiter to Fixed mode
    configPage6.tachoMode = 0;

    //CAN broadcast introduced
    configPage4.CANBroadcastProtocol = CAN_BROADCAST_PROTOCOL_OFF;
    
    configPage15.boostDCWhenDisabled = 0;
    configPage15.boostControlEnable = EN_BOOST_CONTROL_BARO;
    
    //Fill the boostTableLookupDuty with all 50% duty cycle. This is the same as the hardcoded 50% DC that had been used before.
    //This makes the boostcontrol fully backwards compatible.  
    for_each(boostTableLookupDuty.values.begin(), setValue, (table3d_value_t)(50U*2U));

    //Set some sensible values at the RPM axis
    uint16_t i = 0;
    auto setXAxis= [](table_axis_iterator &it, uint16_t* pI) {  //cppcheck-suppress [misra-c2012-13.1,constParameter]
      ++(*pI); *it = 1000+(500*(*pI));
    }; 
    for_each<uint16_t*>(boostTableLookupDuty.axisX.begin(), setXAxis, &i); // cppcheck-suppress [constParameter]

    //Set some sensible values at the boosttarget axis
    i = 0;
    auto setYAxis= [](table_axis_iterator &it, uint16_t* pI) {  //cppcheck-suppress [misra-c2012-13.1,constParameter]
      ++(*pI); *it = (120 + 10*(*pI)); 
    }; //cppcheck-,constParameter] 
    for_each<uint16_t*>(boostTableLookupDuty.axisX.begin(), setYAxis, &i); //cppcheck-suppress [constParameter]

    //AFR Protection added, add default values
    configPage9.afrProtectEnabled = 0U; //Disable by default
    configPage9.afrProtectMinMAP = 90U; //Is divided by 2, value represents 180kPa
    configPage9.afrProtectMinRPM = 40U; //4000 RPM min
    configPage9.afrProtectMinTPS = 160U; //80% TPS min
    configPage9.afrProtectDeviation = 14U; //1.4 AFR deviation    
    
    saveAllPages();
    saveEEPROMVersion(20);
  }

  if (loadEEPROMVersion() == 20U)
  {
    //202305
    configPage2.taeMinChange = 4; //Default is 2% minimum change to match prior behaviour. (4 = 2% account for 0.5 resolution)
    configPage2.maeMinChange = 2; //Default is 2% minimum change to match prior behaviour.

    configPage2.decelAmount = 100; //Default decel fuel amount is 100%, so no change in fueling in decel as before.
    //full status structure has been changed. Update programmable outputs settings to match.
    for (uint8_t y = 0; y < sizeof(configPage13.outputPin); y++)
    {
      if ((configPage13.firstDataIn[y] > 22U) && (configPage13.firstDataIn[y] < 240U)) {configPage13.firstDataIn[y]++;}
      if ((configPage13.firstDataIn[y] > 92U) && (configPage13.firstDataIn[y] < 240U)) {configPage13.firstDataIn[y]++;}
      if ((configPage13.secondDataIn[y] > 22U) && (configPage13.secondDataIn[y] < 240U)) {configPage13.secondDataIn[y]++;}
      if ((configPage13.secondDataIn[y] > 92U) && (configPage13.secondDataIn[y] < 240U)) {configPage13.secondDataIn[y]++;}
    }
    
    //AC Control (configPage15)
    //Set A/C default values - these line up with the ini file defaults
    configPage15.airConEnable = 0;

    //Oil Pressure protection delay added. Set to 0 to match existing behaviour
    configPage10.oilPressureProtTime = 0;

    //Option to power stepper motor constantly was added. Default to previous behaviour
    configPage9.iacStepperPower = 0;

    saveAllPages();
    saveEEPROMVersion(21);
  }

  if (loadEEPROMVersion() == 21U)
  {
    //202310

    //Rolling cut curve added. Default values
    configPage15.rollingProtRPMDelta[0]   = -30;
    configPage15.rollingProtRPMDelta[1]   = -20;
    configPage15.rollingProtRPMDelta[2]   = -10;
    configPage15.rollingProtRPMDelta[3]   = -5;
    configPage15.rollingProtCutPercent[0] = 50;
    configPage15.rollingProtCutPercent[1] = 65;
    configPage15.rollingProtCutPercent[2] = 80;
    configPage15.rollingProtCutPercent[3] = 95;

    //DFCO Hyster was multiplied by 2 to allow a range of 0-500. Existing values must be halved
    configPage4.dfcoHyster = configPage4.dfcoHyster / 2U;

    saveAllPages();
    saveEEPROMVersion(22);
  }

  if (loadEEPROMVersion() == 22U)
  {
    //202402
    
    if( configPage10.wmiMode >= WMI_MODE_OPENLOOP ) {
      for_each(wmiTable.axisX.begin(),    tableAxisMultiply, (table3d_axis_t)2U);
      for_each(wmiTable.axisY.begin(),    tableAxisMultiply, (table3d_axis_t)2U);
      for_each(wmiTable.values.begin(),   tableValueMultiply, (table3d_value_t)2U);

      for_each(vvt2Table.axisX.begin(),   tableAxisMultiply, (table3d_axis_t)2U);
      for_each(vvt2Table.axisY.begin(),   tableAxisMultiply, (table3d_axis_t)2U);
      for_each(vvt2Table.values.begin(),  tableValueMultiply, (table3d_value_t)2U);

      for_each(dwellTable.axisX.begin(),  tableAxisMultiply, (table3d_axis_t)2U);
      for_each(dwellTable.axisY.begin(),  tableAxisMultiply, (table3d_axis_t)2U);
      for_each(dwellTable.values.begin(), tableValueMultiply, (table3d_value_t)2U);
    }

    //Default values for pulsed hw test modes
    configPage13.hwTestInjDuration = 8;
    configPage13.hwTestIgnDuration = 4;

    //DFCO taper default values (Feature disabled by default)
    configPage9.dfcoTaperEnable = 0; //Disable
    configPage9.dfcoTaperTime = 10; //1 second
    configPage9.dfcoTaperFuel = 100; //Don't scale fuel
    configPage9.dfcoTaperAdvance = 20; //Reduce 20deg until full fuel cut
    
    //EGO MAP Limits
    configPage9.egoMAPMax = 255U; // 255 will be 510 kpa
    configPage9.egoMAPMin = 0U;  // 0 will be 0 kpa

    //rusEFI CAN Wideband
    configPage2.canWBO = 0;

    saveAllPages();
    saveEEPROMVersion(23);
  }

  if(loadEEPROMVersion() == 23)
  {
    //202501
    configPage10.knock_mode = KNOCK_MODE_OFF;

    //Change the CAN Broadcast settings to be a selection
    //Note that 1 preference will be lost if both BMW AND VAG protocols were enabled, but that is not a likely combination.
    if(configPage2.unused1_126_1 == true) { configPage4.CANBroadcastProtocol = CAN_BROADCAST_PROTOCOL_BMW; } //unused1_126_1 was canBMWCluster
    if(configPage2.unused1_126_2 == true) { configPage4.CANBroadcastProtocol = CAN_BROADCAST_PROTOCOL_VAG; } //unused1_126_2 was canVAGCluster

    //VSS max limit on launch control
    configPage10.lnchCtrlVss = 255;
    
    //Default all existing tunes to GM flex sensors
    configPage2.flexFreqLow = 50;
    configPage2.flexFreqHigh = 150;

    //Realign configPage10 to correct unaligned pointer warnings
    //Move boostIntv from position 27 to 25
    uint8_t origBoostIntv = ((uint8_t *)&configPage10)[27];
    ((uint8_t *)&configPage10)[27] = ((uint8_t *)&configPage10)[26];
    ((uint8_t *)&configPage10)[26] = ((uint8_t *)&configPage10)[25];
    ((uint8_t *)&configPage10)[25] = origBoostIntv;
    //Move lnchCtrlTPS from position 32 to 74
    uint8_t origlnchCtrlTPS= ((uint8_t *)&configPage10)[32];
    for(byte x=32U; x<74U; x++)
    {
      ((uint8_t *)&configPage10)[x] = ((uint8_t *)&configPage10)[x+1];
    }
    ((uint8_t *)&configPage10)[74] = origlnchCtrlTPS;

    saveAllPages();
    saveEEPROMVersion(24);
  }
  
  if(loadEEPROMVersion() == 24)
  {
    //202504


    saveAllPages();
    saveEEPROMVersion(25);
  }
  
  
  //Final check is always for 255 and 0 (Brand new arduino)
  if( (loadEEPROMVersion() == 0U) || (loadEEPROMVersion() == 255U) )
  {
    configPage9.true_address = 0x200;
    
    //Programmable outputs added. Set all to disabled
    configPage13.outputPin[0] = 0;
    configPage13.outputPin[1] = 0;
    configPage13.outputPin[2] = 0;
    configPage13.outputPin[3] = 0;
    configPage13.outputPin[4] = 0;
    configPage13.outputPin[5] = 0;
    configPage13.outputPin[6] = 0;
    configPage13.outputPin[7] = 0;

    configPage4.FILTER_FLEX = FILTER_FLEX_DEFAULT;

    saveEEPROMVersion(CURRENT_DATA_VERSION);
  }

  //Check to see if someone has downgraded versions:
  if( loadEEPROMVersion() > CURRENT_DATA_VERSION ) { saveEEPROMVersion(CURRENT_DATA_VERSION); }
}

#if defined(CORE_AVR)
#pragma GCC pop_options
#endif