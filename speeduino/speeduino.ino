/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,la
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef UNIT_TEST  // Scope guard for unit testing

#include <stdint.h> //https://developer.mbed.org/handbook/C-Data-Types
//************************************************
#include "globals.h"
#include "speeduino.h"
#include "utils.h"
#include "table.h"
#include "scheduler.h"
#include "comms.h"
#include "cancomms.h"
#include "maths.h"
#include "corrections.h"
#include "timers.h"
//#include "display.h"
#include "decoders.h"
#include "idle.h"
#include "auxiliaries.h"
#include "sensors.h"
//#include "src/DigitalWriteFast/digitalWriteFast.h"
#include "errors.h"
#include "storage.h"
#include "scheduledIO.h"
#include "crankMaths.h"
#include "updates.h"
#include <EEPROM.h>
#if defined (CORE_TEENSY)
#include <FlexCAN.h>
#endif

struct config2 configPage2;
struct config4 configPage4; //Done
struct config6 configPage6;
struct config9 configPage9;
struct config10 configPage10;
/*
struct config2 configPage1;
struct config6 configPage3;
struct config9 configPage9;
struct config10 configPage11;
*/

uint16_t req_fuel_uS, inj_opentime_uS;
uint16_t staged_req_fuel_mult_pri;
uint16_t staged_req_fuel_mult_sec;

bool ignitionOn = false; //The current state of the ignition system
bool fuelOn = false; //The current state of the ignition system

void (*trigger)(); //Pointer for the trigger function (Gets pointed to the relevant decoder)
void (*triggerSecondary)(); //Pointer for the secondary trigger function (Gets pointed to the relevant decoder)
uint16_t (*getRPM)(); //Pointer to the getRPM function (Gets pointed to the relevant decoder)
int (*getCrankAngle)(); //Pointer to the getCrank Angle function (Gets pointed to the relevant decoder)
void (*triggerSetEndTeeth)(); //Pointer to the triggerSetEndTeeth function of each decoder

byte cltCalibrationTable[CALIBRATION_TABLE_SIZE];
byte iatCalibrationTable[CALIBRATION_TABLE_SIZE];
byte o2CalibrationTable[CALIBRATION_TABLE_SIZE];

unsigned long counter;
unsigned long currentLoopTime; //The time the current loop started (uS)
unsigned long previousLoopTime; //The time the previous loop started (uS)

volatile uint16_t mainLoopCount;
byte deltaToothCount = 0; //The last tooth that was used with the deltaV calc
int rpmDelta;
byte maxIgnOutputs = 1; //Used for rolling rev limiter
byte curRollingCut = 0; //Rolling rev limiter, current ignition channel being cut
byte rollingCutCounter = 0; //how many times (revolutions) the ignition has been cut in a row
uint32_t rollingCutLastRev = 0; //Tracks whether we're on the same or a different rev for the rolling cut
uint16_t fixedCrankingOverride = 0;


unsigned long secCounter; //The next time to incremen 'runSecs' counter.
int channel1IgnDegrees; //The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones)
int channel2IgnDegrees; //The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC
int channel3IgnDegrees; //The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC
int channel4IgnDegrees; //The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC
int channel5IgnDegrees; //The number of crank degrees until cylinder 5 is at TDC
int channel6IgnDegrees; //The number of crank degrees until cylinder 6 is at TDC
int channel7IgnDegrees; //The number of crank degrees until cylinder 7 is at TDC
int channel8IgnDegrees; //The number of crank degrees until cylinder 8 is at TDC
int channel1InjDegrees; //The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones)
int channel2InjDegrees; //The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC
int channel3InjDegrees; //The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC
int channel4InjDegrees; //The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC
int channel5InjDegrees; //The number of crank degrees until cylinder 5 is at TDC
int channel6InjDegrees; //The number of crank degrees until cylinder 6 is at TDC
int channel7InjDegrees; //The number of crank degrees until cylinder 7 is at TDC
int channel8InjDegrees; //The number of crank degrees until cylinder 8 is at TDC

//These are the functions the get called to begin and end the ignition coil charging. They are required for the various spark output modes
void (*ign1StartFunction)();
void (*ign1EndFunction)();
void (*ign2StartFunction)();
void (*ign2EndFunction)();
void (*ign3StartFunction)();
void (*ign3EndFunction)();
void (*ign4StartFunction)();
void (*ign4EndFunction)();
void (*ign5StartFunction)();
void (*ign5EndFunction)();
void (*ign6StartFunction)();
void (*ign6EndFunction)();
void (*ign7StartFunction)();
void (*ign7EndFunction)();
void (*ign8StartFunction)();
void (*ign8EndFunction)();

volatile bool fpPrimed = false; //Tracks whether or not the fuel pump priming has been completed yet

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  //Setup the dummy fuel and ignition tables
  //dummyFuelTable(&fuelTable);
  //dummyIgnitionTable(&ignitionTable);
  table3D_setSize(&fuelTable, 16);
  table3D_setSize(&ignitionTable, 16);
  table3D_setSize(&afrTable, 16);
  table3D_setSize(&stagingTable, 8);
  table3D_setSize(&boostTable, 8);
  table3D_setSize(&vvtTable, 8);
  table3D_setSize(&trim1Table, 6);
  table3D_setSize(&trim2Table, 6);
  table3D_setSize(&trim3Table, 6);
  table3D_setSize(&trim4Table, 6);
  initialiseTimers();

  loadConfig();
  doUpdates(); //Check if any data items need updating (Occurs ith firmware updates)

  //Always start with a clean slate on the bootloader capabilities level
  //This should be 0 until we hear otherwise from the 16u2
  configPage4.bootloaderCaps = 0;

  Serial.begin(115200);
  if (configPage9.enable_canbus == 1) { CANSerial.begin(115200); }

  #if defined(CORE_STM32) || defined(CORE_TEENSY)
  else if (configPage9.enable_canbus == 2)
  {
    //Teensy onboard CAN not used currently
    //enable local can interface
    //setup can interface to 250k
    //FlexCAN CANbus0(2500000, 0);
    //static CAN_message_t txmsg,rxmsg;
    //CANbus0.begin();
  }

  #endif

  //Repoint the 2D table structs to the config pages that were just loaded
  taeTable.valueSize = SIZE_BYTE; //Set this table to use byte values
  taeTable.xSize = 4;
  taeTable.values = configPage4.taeValues;
  taeTable.axisX = configPage4.taeBins;
  WUETable.valueSize = SIZE_BYTE; //Set this table to use byte values
  WUETable.xSize = 10;
  WUETable.values = configPage2.wueValues;
  WUETable.axisX = configPage4.wueBins;
  crankingEnrichTable.valueSize = SIZE_BYTE;
  crankingEnrichTable.xSize = 4;
  crankingEnrichTable.values = configPage10.crankingEnrichValues;
  crankingEnrichTable.axisX = configPage10.crankingEnrichBins;

  dwellVCorrectionTable.valueSize = SIZE_BYTE;
  dwellVCorrectionTable.xSize = 6;
  dwellVCorrectionTable.values = configPage4.dwellCorrectionValues;
  dwellVCorrectionTable.axisX = configPage6.voltageCorrectionBins;
  injectorVCorrectionTable.valueSize = SIZE_BYTE;
  injectorVCorrectionTable.xSize = 6;
  injectorVCorrectionTable.values = configPage6.injVoltageCorrectionValues;
  injectorVCorrectionTable.axisX = configPage6.voltageCorrectionBins;
  IATDensityCorrectionTable.valueSize = SIZE_BYTE;
  IATDensityCorrectionTable.xSize = 9;
  IATDensityCorrectionTable.values = configPage6.airDenRates;
  IATDensityCorrectionTable.axisX = configPage6.airDenBins;
  IATRetardTable.valueSize = SIZE_BYTE;
  IATRetardTable.xSize = 6;
  IATRetardTable.values = configPage4.iatRetValues;
  IATRetardTable.axisX = configPage4.iatRetBins;
  rotarySplitTable.valueSize = SIZE_BYTE;
  rotarySplitTable.xSize = 8;
  rotarySplitTable.values = configPage10.rotarySplitValues;
  rotarySplitTable.axisX = configPage10.rotarySplitBins;

  flexFuelTable.valueSize = SIZE_BYTE;
  flexFuelTable.xSize = 6;
  flexFuelTable.values = configPage10.flexFuelAdj;
  flexFuelTable.axisX = configPage10.flexFuelBins;
  flexAdvTable.valueSize = SIZE_BYTE;
  flexAdvTable.xSize = 6;
  flexAdvTable.values = configPage10.flexAdvAdj;
  flexAdvTable.axisX = configPage10.flexAdvBins;
  flexBoostTable.valueSize = SIZE_INT;
  flexBoostTable.xSize = 6;
  flexBoostTable.values16 = configPage10.flexBoostAdj;
  flexBoostTable.axisX = configPage10.flexBoostBins;

  //Setup the calibration tables
  loadCalibration();

  //Set the pin mappings
  if(configPage2.pinMapping == 255)
  {
    //First time running on this board
    setPinMapping(3); //Force board to v0.4
    configPage2.flexEnabled = false; //Have to disable flex. If this isn't done and the wrong flex pin is interrupt attached below, system can hang.
  }
  else { setPinMapping(configPage2.pinMapping); }

  //Need to check early on whether the coil charging is inverted. If this is not set straight away it can cause an unwanted spark at bootup
  if(configPage4.IgInv == 1) { coilHIGH = LOW; coilLOW = HIGH; }
  else { coilHIGH = HIGH; coilLOW = LOW; }
  endCoil1Charge();
  endCoil2Charge();
  endCoil3Charge();
  endCoil4Charge();
  endCoil5Charge();

  //Similar for injectors, make sure they're turned off
  closeInjector1();
  closeInjector2();
  closeInjector3();
  closeInjector4();
  closeInjector5();

  //Set the tacho output default state
  digitalWrite(pinTachOut, HIGH);
  //Perform all initialisations
  initialiseSchedulers();
  //initialiseDisplay();
  initialiseIdle();
  initialiseFan();
  initialiseAuxPWM();
  initialiseCorrections();
  initialiseADC();

  //Lookup the current MAP reading for barometric pressure
  instanteneousMAPReading();
  //barometric reading can be taken from either an external sensor if enabled, or simply by using the initial MAP value
  if ( configPage6.useExtBaro != 0 )
  {
    readBaro();
    EEPROM.update(EEPROM_LAST_BARO, currentStatus.baro);
  }
  else
  {
    /*
     * The highest sea-level pressure on Earth occurs in Siberia, where the Siberian High often attains a sea-level pressure above 105 kPa;
     * with record highs close to 108.5 kPa.
     * The lowest measurable sea-level pressure is found at the centers of tropical cyclones and tornadoes, with a record low of 87 kPa;
     */
    if ((currentStatus.MAP >= BARO_MIN) && (currentStatus.MAP <= BARO_MAX)) //Check if engine isn't running
    {
      currentStatus.baro = currentStatus.MAP;
      EEPROM.update(EEPROM_LAST_BARO, currentStatus.baro);
    }
    else
    {
      //Attempt to use the last known good baro reading from EEPROM
      if ((EEPROM.read(EEPROM_LAST_BARO) >= BARO_MIN) && (EEPROM.read(EEPROM_LAST_BARO) <= BARO_MAX)) //Make sure it's not invalid (Possible on first run etc)
      { currentStatus.baro = EEPROM.read(EEPROM_LAST_BARO); } //last baro correction
      else { currentStatus.baro = 100; } //Final fall back position.
    }
  }

  //Check whether the flex sensor is enabled and if so, attach an interupt for it
  if(configPage2.flexEnabled > 0)
  {
    attachInterrupt(digitalPinToInterrupt(pinFlex), flexPulse, RISING);
    currentStatus.ethanolPct = 0;
  }

  //Once the configs have been loaded, a number of one time calculations can be completed
  req_fuel_uS = configPage2.reqFuel * 100; //Convert to uS and an int. This is the only variable to be used in calculations
  inj_opentime_uS = configPage2.injOpen * 100; //Injector open time. Comes through as ms*10 (Eg 15.5ms = 155).

  if(configPage10.stagingEnabled == true)
  {
    uint32_t totalInjector = configPage10.stagedInjSizePri + configPage10.stagedInjSizeSec;
    /*
      These values are a percentage of the req_fuel value that would be required for each injector channel to deliver that much fuel.
      Eg:
      Pri injectors are 250cc
      Sec injectors are 500cc
      Total injector capacity = 750cc

      staged_req_fuel_mult_pri = 300% (The primary injectors would have to run 3x the overall PW in order to be the equivalent of the full 750cc capacity
      staged_req_fuel_mult_sec = 150% (The secondary injectors would have to run 1.5x the overall PW in order to be the equivalent of the full 750cc capacity
    */
    staged_req_fuel_mult_pri = (100 * totalInjector) / configPage10.stagedInjSizePri;
    staged_req_fuel_mult_sec = (100 * totalInjector) / configPage10.stagedInjSizeSec;
  }

  //Begin the main crank trigger interrupt pin setup
  //The interrupt numbering is a bit odd - See here for reference: http://arduino.cc/en/Reference/AttachInterrupt
  //These assignments are based on the Arduino Mega AND VARY BETWEEN BOARDS. Please confirm the board you are using and update acordingly.
  currentStatus.RPM = 0;
  currentStatus.hasSync = false;
  currentStatus.runSecs = 0;
  currentStatus.secl = 0;
  currentStatus.startRevolutions = 0;
  currentStatus.syncLossCounter = 0;
  currentStatus.flatShiftingHard = false;
  currentStatus.launchingHard = false;
  currentStatus.crankRPM = ((unsigned int)configPage4.crankRPM * 10); //Crank RPM limit (Saves us calculating this over and over again. It's updated once per second in timers.ino)
  currentStatus.fuelPumpOn = false;
  triggerFilterTime = 0; //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be disgarded as noise. This is simply a default value, the actual values are set in the setup() functinos of each decoder
  dwellLimit_uS = (1000 * configPage4.dwellLimit);
  currentStatus.nChannels = (INJ_CHANNELS << 4) + IGN_CHANNELS; //First 4 bits store the number of injection channels, 2nd 4 store the number of ignition channels

  noInterrupts();
  initialiseTriggers();

  //End crank triger interrupt attachment
  req_fuel_uS = req_fuel_uS / engineSquirtsPerCycle; //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle. If we're doing more than 1 squirt per cycle then we need to split the amount accordingly. (Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the stroke to make the single squirt on)

  //Initial values for loop times
  previousLoopTime = 0;
  currentLoopTime = micros_safe();

  mainLoopCount = 0;

  currentStatus.nSquirts = configPage2.nCylinders / configPage2.divider; //The number of squirts being requested. This is manaully overriden below for sequential setups (Due to TS req_fuel calc limitations)
  CRANK_ANGLE_MAX_INJ = 720 / currentStatus.nSquirts;

  //Calculate the number of degrees between cylinders
  switch (configPage2.nCylinders) {
    case 1:
      channel1IgnDegrees = 0;
      channel1InjDegrees = 0;
      maxIgnOutputs = 1;

      //Sequential ignition works identically on a 1 cylinder whether it's odd or even fire. 
      if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL) { CRANK_ANGLE_MAX_IGN = 720; }

      if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        CRANK_ANGLE_MAX_INJ = 720;
        currentStatus.nSquirts = 1;
        req_fuel_uS = req_fuel_uS * 2;
      }

      channel1InjEnabled = true;
      break;

    case 2:
      channel1IgnDegrees = 0;
      channel1InjDegrees = 0;
      maxIgnOutputs = 2;
      if (configPage2.engineType == EVEN_FIRE ) { channel2IgnDegrees = 180; }
      else { channel2IgnDegrees = configPage2.oddfire2; }

      //Sequential ignition works identically on a 2 cylinder whether it's odd or even fire (With the default being a 180 degree second cylinder). 
      if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL) { CRANK_ANGLE_MAX_IGN = 720; }

      if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        CRANK_ANGLE_MAX_INJ = 720;
        currentStatus.nSquirts = 1;
        req_fuel_uS = req_fuel_uS * 2;
      }
      //The below are true regardless of whether this is running sequential or not
      if (configPage2.engineType == EVEN_FIRE ) { channel2InjDegrees = 180; }
      else { channel2InjDegrees = configPage2.oddfire2; }
      if (!configPage2.injTiming) 
      { 
        //For simultaneous, all squirts happen at the same time
        channel1InjDegrees = 0;
        channel2InjDegrees = 0; 
      } 

      channel1InjEnabled = true;
      channel2InjEnabled = true;
      break;

    case 3:
      channel1IgnDegrees = 0;
      maxIgnOutputs = 3;
      if (configPage2.engineType == EVEN_FIRE )
      {
        if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
        {
          channel2IgnDegrees = 240;
          channel3IgnDegrees = 480;

          CRANK_ANGLE_MAX_IGN = 720;
        }
        else
        {
          channel2IgnDegrees = 120;
          channel3IgnDegrees = 240;
        }
      }
      else
      {
        channel2IgnDegrees = configPage2.oddfire2;
        channel3IgnDegrees = configPage2.oddfire3;
      }

      //For alternatiing injection, the squirt occurs at different times for each channel
      if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) )
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 120;
        channel3InjDegrees = 240;

        //Adjust the injection angles based on the number of squirts
        if (currentStatus.nSquirts > 2)
        {
          channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
          channel3InjDegrees = (channel3InjDegrees * 2) / currentStatus.nSquirts;
        }

        if (!configPage2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          channel1InjDegrees = 0;
          channel2InjDegrees = 0;
          channel3InjDegrees = 0; 
        } 
      }
      else if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 240;
        channel3InjDegrees = 480;
        CRANK_ANGLE_MAX_INJ = 720;
        currentStatus.nSquirts = 1;
        req_fuel_uS = req_fuel_uS * 2;
      }

      channel1InjEnabled = true;
      channel2InjEnabled = true;
      channel3InjEnabled = true;
      break;
    case 4:
      channel1IgnDegrees = 0;
      channel1InjDegrees = 0;
      maxIgnOutputs = 2; //Default value for 4 cylinder, may be changed below
      if (configPage2.engineType == EVEN_FIRE )
      {
        channel2IgnDegrees = 180;
        //Adjust the injection angles based on the number of squirts
        if (currentStatus.nSquirts > 2)
        {
          channel2InjDegrees = channel2InjDegrees / (currentStatus.nSquirts / 2);
        }

        if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
        {
          channel3IgnDegrees = 360;
          channel4IgnDegrees = 540;

          CRANK_ANGLE_MAX_IGN = 720;
          maxIgnOutputs = 4;
        }
        else if(configPage4.sparkMode == IGN_MODE_ROTARY)
        {
          //Rotary uses the ign 3 and 4 schedules for the trailing spark. They are offset from the ign 1 and 2 channels respectively and so use the same degrees as them
          channel3IgnDegrees = 0;
          channel4IgnDegrees = 180;
        }
      }
      else
      {
        channel2IgnDegrees = configPage2.oddfire2;
        channel3IgnDegrees = configPage2.oddfire3;
        channel4IgnDegrees = configPage2.oddfire4;
        maxIgnOutputs = 4;
      }

      //For alternatiing injection, the squirt occurs at different times for each channel
      if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) )
      {
        channel2InjDegrees = 180;

        if (!configPage2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          channel1InjDegrees = 0;
          channel2InjDegrees = 0; 
        }
      }
      else if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        channel2InjDegrees = 180;
        channel3InjDegrees = 360;
        channel4InjDegrees = 540;

        channel3InjEnabled = true;
        channel4InjEnabled = true;

        CRANK_ANGLE_MAX_INJ = 720;
        currentStatus.nSquirts = 1;
        req_fuel_uS = req_fuel_uS * 2;
      }

      //Check if injector staging is enabled
      if(configPage10.stagingEnabled == true)
      {
        channel3InjEnabled = true;
        channel4InjEnabled = true;

        channel3InjDegrees = channel1InjDegrees;
        channel4InjDegrees = channel2InjDegrees;
      }

      channel1InjEnabled = true;
      channel2InjEnabled = true;
      break;
    case 5:
      channel1IgnDegrees = 0;
      channel2IgnDegrees = 72;
      channel3IgnDegrees = 144;
      channel4IgnDegrees = 216;
      channel5IgnDegrees = 288;
      maxIgnOutputs = 4; //Only 4 actual outputs, so that's all that can be cut

      if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
      {
        channel2IgnDegrees = 144;
        channel3IgnDegrees = 288;
        channel4IgnDegrees = 432;
        channel5IgnDegrees = 576;

        CRANK_ANGLE_MAX_IGN = 720;
      }

      //For alternatiing injection, the squirt occurs at different times for each channel
      if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) )
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 72;
        channel3InjDegrees = 144;
        channel4InjDegrees = 216;
        channel5InjDegrees = 288;
      }
      else if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 144;
        channel3InjDegrees = 288;
        channel4InjDegrees = 432;
        channel5InjDegrees = 576;

        CRANK_ANGLE_MAX_INJ = 720;
        currentStatus.nSquirts = 1;
      }
      if (!configPage2.injTiming) 
      { 
        //For simultaneous, all squirts happen at the same time
        channel1InjDegrees = 0;
        channel2InjDegrees = 0;
        channel3InjDegrees = 0;
        channel4InjDegrees = 0;
        channel5InjDegrees = 0; 
      }

      channel1InjEnabled = true;
      channel2InjEnabled = true;
      channel3InjEnabled = false; //this is disabled as injector 5 function calls 3 & 5 together
      channel4InjEnabled = true;
      channel5InjEnabled = true;
      break;
    case 6:
      channel1IgnDegrees = 0;
      channel1InjDegrees = 0;
      channel2IgnDegrees = 120;
      channel2InjDegrees = 120;
      channel3IgnDegrees = 240;
      channel3InjDegrees = 240;
      maxIgnOutputs = 3;

      //Adjust the injection angles based on the number of squirts
      if (currentStatus.nSquirts > 2)
      {
        channel2InjDegrees = channel2InjDegrees / (currentStatus.nSquirts / 2);
        channel3InjDegrees = channel3InjDegrees / (currentStatus.nSquirts / 2);
      }

#if INJ_CHANNELS >= 6
      if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 120;
        channel3InjDegrees = 240;
        channel4InjDegrees = 360;
        channel5InjDegrees = 480;
        channel6InjDegrees = 600;

        channel4InjEnabled = true;
        channel5InjEnabled = true;
        channel6InjEnabled = true;

        CRANK_ANGLE_MAX_INJ = 720;
        currentStatus.nSquirts = 1;
        req_fuel_uS = req_fuel_uS * 2;
      }
#endif

      if (!configPage2.injTiming) 
      { 
        //For simultaneous, all squirts happen at the same time
        channel1InjDegrees = 0;
        channel2InjDegrees = 0;
        channel3InjDegrees = 0; 
      } 

      configPage2.injLayout = 0; //This is a failsafe. We can never run semi-sequential with more than 4 cylinders

      channel1InjEnabled = true;
      channel2InjEnabled = true;
      channel3InjEnabled = true;
      break;
    case 8:
      channel1IgnDegrees = 0;
      channel2IgnDegrees = channel2InjDegrees = 90;
      channel3IgnDegrees = channel3InjDegrees = 180;
      channel4IgnDegrees = channel4InjDegrees = 270;

      //Adjust the injection angles based on the number of squirts
      if (currentStatus.nSquirts > 2)
      {
        channel2InjDegrees = channel2InjDegrees / (currentStatus.nSquirts / 2);
        channel3InjDegrees = channel3InjDegrees / (currentStatus.nSquirts / 2);
        channel4InjDegrees = channel4InjDegrees / (currentStatus.nSquirts / 2);
      }

#if INJ_CHANNELS >= 8
      if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 90;
        channel3InjDegrees = 180;
        channel4InjDegrees = 270;
        channel5InjDegrees = 360;
        channel6InjDegrees = 450;
        channel7InjDegrees = 540;
        channel8InjDegrees = 630;

        channel5InjEnabled = true;
        channel6InjEnabled = true;
        channel7InjEnabled = true;
        channel8InjEnabled = true;

        CRANK_ANGLE_MAX_INJ = 720;
        currentStatus.nSquirts = 1;
        req_fuel_uS = req_fuel_uS * 2;
      }
#endif

      maxIgnOutputs = 4;

      if (!configPage2.injTiming) 
      { 
        //For simultaneous, all squirts happen at the same time
        channel1InjDegrees = 0;
        channel2InjDegrees = 0;
        channel3InjDegrees = 0;
        channel4InjDegrees = 0; 
      }

      configPage2.injLayout = 0; //This is a failsafe. We can never run semi-sequential with more than 4 cylinders

      channel1InjEnabled = true;
      channel2InjEnabled = true;
      channel3InjEnabled = true;
      channel4InjEnabled = true;
      break;
    default: //Handle this better!!!
      channel1InjDegrees = 0;
      channel2InjDegrees = 180;
      break;
  }

  if(CRANK_ANGLE_MAX_IGN == CRANK_ANGLE_MAX_INJ) { CRANK_ANGLE_MAX = CRANK_ANGLE_MAX_IGN; } //If both the injector max and ignition max angles are the same, make the overall system max this value
  else if (CRANK_ANGLE_MAX_IGN > CRANK_ANGLE_MAX_INJ) { CRANK_ANGLE_MAX = CRANK_ANGLE_MAX_IGN; }
  else { CRANK_ANGLE_MAX = CRANK_ANGLE_MAX_INJ; }
  currentStatus.status3 = currentStatus.nSquirts << BIT_STATUS3_NSQUIRTS1; //Top 3 bits of the status3 variable are the number of squirts. This must be done after the above section due to nSquirts being forced to 1 for sequential

  switch(configPage4.sparkMode)
  {
    case IGN_MODE_WASTED:
      //Wasted Spark (Normal mode)
      ign1StartFunction = beginCoil1Charge;
      ign1EndFunction = endCoil1Charge;
      ign2StartFunction = beginCoil2Charge;
      ign2EndFunction = endCoil2Charge;
      ign3StartFunction = beginCoil3Charge;
      ign3EndFunction = endCoil3Charge;
      ign4StartFunction = beginCoil4Charge;
      ign4EndFunction = endCoil4Charge;
      ign5StartFunction = beginCoil5Charge;
      ign5EndFunction = endCoil5Charge;
      break;

    case IGN_MODE_SINGLE:
      //Single channel mode. All ignition pulses are on channel 1
      ign1StartFunction = beginCoil1Charge;
      ign1EndFunction = endCoil1Charge;
      ign2StartFunction = beginCoil1Charge;
      ign2EndFunction = endCoil1Charge;
      ign3StartFunction = beginCoil1Charge;
      ign3EndFunction = endCoil1Charge;
      ign4StartFunction = beginCoil1Charge;
      ign4EndFunction = endCoil1Charge;
      ign5StartFunction = beginCoil1Charge;
      ign5EndFunction = endCoil1Charge;
      break;

    case IGN_MODE_WASTEDCOP:
      //Wasted COP mode. Ignition channels 1&3 and 2&4 are paired together
      //This is not a valid mode for >4 cylinders
      if( configPage2.nCylinders <= 4 )
      {
        ign1StartFunction = beginCoil1and3Charge;
        ign1EndFunction = endCoil1and3Charge;
        ign2StartFunction = beginCoil2and4Charge;
        ign2EndFunction = endCoil2and4Charge;

        ign3StartFunction = nullCallback;
        ign3EndFunction = nullCallback;
        ign4StartFunction = nullCallback;
        ign4EndFunction = nullCallback;
      }
      else
      {
        //If the person has inadvertantly selected this when running more than 4 cylinders, just use standard Wasted spark mode
        ign1StartFunction = beginCoil1Charge;
        ign1EndFunction = endCoil1Charge;
        ign2StartFunction = beginCoil2Charge;
        ign2EndFunction = endCoil2Charge;
        ign3StartFunction = beginCoil3Charge;
        ign3EndFunction = endCoil3Charge;
        ign4StartFunction = beginCoil4Charge;
        ign4EndFunction = endCoil4Charge;
        ign5StartFunction = beginCoil5Charge;
        ign5EndFunction = endCoil5Charge;
      }
      break;

    case IGN_MODE_SEQUENTIAL:
      ign1StartFunction = beginCoil1Charge;
      ign1EndFunction = endCoil1Charge;
      ign2StartFunction = beginCoil2Charge;
      ign2EndFunction = endCoil2Charge;
      ign3StartFunction = beginCoil3Charge;
      ign3EndFunction = endCoil3Charge;
      ign4StartFunction = beginCoil4Charge;
      ign4EndFunction = endCoil4Charge;
      ign5StartFunction = beginCoil5Charge;
      ign5EndFunction = endCoil5Charge;
      ign6StartFunction = beginCoil6Charge;
      ign6EndFunction = endCoil6Charge;
      ign7StartFunction = beginCoil7Charge;
      ign7EndFunction = endCoil7Charge;
      ign8StartFunction = beginCoil8Charge;
      ign8EndFunction = endCoil8Charge;
      break;

    case IGN_MODE_ROTARY:
      if(configPage10.rotaryType == ROTARY_IGN_FC)
      {
        ign1StartFunction = beginCoil1Charge;
        ign1EndFunction = endCoil1Charge;
        ign2StartFunction = beginCoil1Charge;
        ign2EndFunction = endCoil1Charge;

        ign3StartFunction = beginTrailingCoilCharge;
        ign3EndFunction = endTrailingCoilCharge1;
        ign4StartFunction = beginTrailingCoilCharge;
        ign4EndFunction = endTrailingCoilCharge2;
      }
      break;



    default:
      //Wasted spark (Shouldn't ever happen anyway)
      ign1StartFunction = beginCoil1Charge;
      ign1EndFunction = endCoil1Charge;
      ign2StartFunction = beginCoil2Charge;
      ign2EndFunction = endCoil2Charge;
      ign3StartFunction = beginCoil3Charge;
      ign3EndFunction = endCoil3Charge;
      ign4StartFunction = beginCoil4Charge;
      ign4EndFunction = endCoil4Charge;
      ign5StartFunction = beginCoil5Charge;
      ign5EndFunction = endCoil5Charge;
      break;
  }

  //Begin priming the fuel pump. This is turned off in the low resolution, 1s interrupt in timers.ino
  FUEL_PUMP_ON();
  currentStatus.fuelPumpOn = true;
  interrupts();
  //Perform the priming pulses. Set these to run at an arbitrary time in the future (100us). The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
  setFuelSchedule1(100, (unsigned long)(configPage2.primePulse * 100));
  setFuelSchedule2(100, (unsigned long)(configPage2.primePulse * 100));
  setFuelSchedule3(100, (unsigned long)(configPage2.primePulse * 100));
  setFuelSchedule4(100, (unsigned long)(configPage2.primePulse * 100));

  initialisationComplete = true;
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
      mainLoopCount++;
      LOOP_TIMER = TIMER_mask;
      //Check for any requets from serial. Serial operations are checked under 2 scenarios:
      // 1) Every 64 loops (64 Is more than fast enough for TunerStudio). This function is equivalent to ((loopCount % 64) == 1) but is considerably faster due to not using the mod or division operations
      // 2) If the amount of data in the serial buffer is greater than a set threhold (See globals.h). This is to avoid serial buffer overflow when large amounts of data is being sent
      //if ( (BIT_CHECK(TIMER_mask, BIT_TIMER_15HZ)) || (Serial.available() > SERIAL_BUFFER_THRESHOLD) )
      if ( ((mainLoopCount & 31) == 1) or (Serial.available() > SERIAL_BUFFER_THRESHOLD) )
      {
        if (Serial.available() > 0) { command(); }
      }
      //if can or secondary serial interface is enabled then check for requests.
      if (configPage9.enable_canbus == 1)  //secondary serial interface enabled
          {
            if ( ((mainLoopCount & 31) == 1) or (CANSerial.available() > SERIAL_BUFFER_THRESHOLD) )
                {
                  if (CANSerial.available() > 0)  { canCommand(); }
                }
          }
      #if  defined(CORE_TEENSY) || defined(CORE_STM32)
          else if (configPage9.enable_canbus == 2) // can module enabled
          {
            //check local can module
            // if ( BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ) or (CANbus0.available())
            //    {
            //      CANbus0.read(rx_msg);
            //    }
          }
      #endif

    //Displays currently disabled
    // if (configPage2.displayType && (mainLoopCount & 255) == 1) { updateDisplay();}

    previousLoopTime = currentLoopTime;
    currentLoopTime = micros_safe();
    unsigned long timeToLastTooth = (currentLoopTime - toothLastToothTime);
    if ( (timeToLastTooth < MAX_STALL_TIME) || (toothLastToothTime > currentLoopTime) ) //Check how long ago the last tooth was seen compared to now. If it was more than half a second ago then the engine is probably stopped. toothLastToothTime can be greater than currentLoopTime if a pulse occurs between getting the lastest time and doing the comparison
    {
      currentStatus.longRPM = getRPM(); //Long RPM is included here
      currentStatus.RPM = currentStatus.longRPM;
      FUEL_PUMP_ON();
      currentStatus.fuelPumpOn = true; //Not sure if this is needed.
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      currentStatus.RPM = 0;
      currentStatus.PW1 = 0;
      currentStatus.VE = 0;
      toothLastToothTime = 0;
      toothLastSecToothTime = 0;
      //toothLastMinusOneToothTime = 0;
      currentStatus.hasSync = false;
      currentStatus.runSecs = 0; //Reset the counter for number of seconds running.
      secCounter = 0; //Reset our seconds counter.
      currentStatus.startRevolutions = 0;
      toothSystemCount = 0;
      secondaryToothCount = 0;
      MAPcurRev = 0;
      MAPcount = 0;
      currentStatus.rpmDOT = 0;
      AFRnextCycle = 0;
      ignitionCount = 0;
      ignitionOn = false;
      fuelOn = false;
      if (fpPrimed == true) { FUEL_PUMP_OFF(); currentStatus.fuelPumpOn = false; } //Turn off the fuel pump, but only if the priming is complete
      disableIdle(); //Turn off the idle PWM
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK); //Clear cranking bit (Can otherwise get stuck 'on' even with 0 rpm)
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_WARMUP); //Same as above except for WUE
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN); //Same as above except for RUNNING status
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ASE); //Same as above except for ASE status
      //This is a safety check. If for some reason the interrupts have got screwed up (Leading to 0rpm), this resets them.
      //It can possibly be run much less frequently.
      initialiseTriggers();

      VVT_PIN_LOW();
      DISABLE_VVT_TIMER();
      boostDisable();
    }

    //Uncomment the following for testing
    /*
    currentStatus.hasSync = true;
    currentStatus.RPM = 500;
    */

    //***Perform sensor reads***
    //-----------------------------------------------------------------------------------------------------
    readMAP();

    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ)) //Every 32 loops
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_15HZ);
      readTPS(); //TPS reading to be performed every 32 loops (any faster and it can upset the TPSdot sampling time)

      //Check for launching/flat shift (clutch) can be done around here too
      previousClutchTrigger = clutchTrigger;
      if(configPage6.launchHiLo > 0) { clutchTrigger = digitalRead(pinLaunch); }
      else { clutchTrigger = !digitalRead(pinLaunch); }

      if(previousClutchTrigger != clutchTrigger) { currentStatus.clutchEngagedRPM = currentStatus.RPM; }

      if (configPage6.launchEnabled && clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > ((unsigned int)(configPage6.lnchHardLim) * 100)) && (currentStatus.TPS >= configPage10.lnchCtrlTPS) ) 
      { 
        //HardCut rev limit for 2-step launch control.
        currentStatus.launchingHard = true; 
        BIT_SET(currentStatus.spark, BIT_SPARK_HLAUNCH); 
      } 
      else { currentStatus.launchingHard = false; BIT_CLEAR(currentStatus.spark, BIT_SPARK_HLAUNCH); }

      if(configPage6.flatSEnable && clutchTrigger && (currentStatus.RPM > ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > currentStatus.clutchEngagedRPM) ) { currentStatus.flatShiftingHard = true; }
      else { currentStatus.flatShiftingHard = false; }

      //Boost cutoff is very similar to launchControl, but with a check against MAP rather than a switch
      if( (configPage6.boostCutType > 0) && (currentStatus.MAP > (configPage6.boostLimit * 2)) ) //The boost limit is divided by 2 to allow a limit up to 511kPa
      {
        switch(configPage6.boostCutType)
        {
          case 1:
            BIT_SET(currentStatus.spark, BIT_SPARK_BOOSTCUT);
            BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
            break;
          case 2:
            BIT_SET(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
            BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
            break;
          case 3:
            BIT_SET(currentStatus.spark, BIT_SPARK_BOOSTCUT);
            BIT_SET(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
            break;
          default:
            //Shouldn't ever happen, but just in case, disable all cuts
            BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
            BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
        }
      }
      else
      {
        BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
        BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
      }

      //And check whether the tooth log buffer is ready
      if(toothHistoryIndex > TOOTH_LOG_SIZE) { BIT_SET(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY); }


    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ)) //30 hertz
    {
      //Nothing here currently
      BIT_CLEAR(TIMER_mask, BIT_TIMER_30HZ);
      //Most boost tends to run at about 30Hz, so placing it here ensures a new target time is fetched frequently enough
      //currentStatus.RPM = 3000;
      boostControl();
    }
    //The IAT and CLT readings can be done less frequently (4 times per second)
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_4HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_4HZ);
      readCLT();
      readIAT();
      readO2();
      readO2_2();
      readBat();
      nitrousControl();

      if(eepromWritesPending == true) { writeAllConfig(); } //Check for any outstanding EEPROM writes.

      if(auxIsEnabled == true)
      {
        //check through the Aux input channels if enabed for Can or local use
        for (byte AuxinChan = 0; AuxinChan <16 ; AuxinChan++)
        {
          currentStatus.current_caninchannel = AuxinChan;          
          //currentStatus.canin[14] = ((configPage9.Auxinpinb[currentStatus.current_caninchannel]&127)+1);
          //currentStatus.canin[13] = (configPage9.caninput_sel[currentStatus.current_caninchannel]&3);          
          if (configPage9.caninput_sel[currentStatus.current_caninchannel] == 1)  //if current input channel is enabled as canbus
          {
            if (configPage9.enable_candata_in > 0)     //if external data input is enabled
            {
              if (configPage9.enable_canbus == 1)  // megas only support can via secondary serial
              {
                sendCancommand(2,0,currentStatus.current_caninchannel,0,((configPage9.caninput_source_can_address[currentStatus.current_caninchannel]&2047)+0x100));
                //send an R command for data from caninput_source_address[currentStatus.current_caninchannel]
              }
              #if defined(CORE_STM32) || defined(CORE_TEENSY)
              else if (configPage9.enable_canbus == 2) // can via internal can module
              {
                sendCancommand(3,configPage9.speeduino_tsCanId,currentStatus.current_caninchannel,0,configPage9.caninput_source_can_address[currentStatus.current_caninchannel]);    //send via localcanbus the command for data from paramgroup[currentStatus.current_caninchannel]
              }
              #endif
            }
          }
          else if ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 2)  //if current input channel is enabled as analog local pin
          {
            //read analog channel specified
            currentStatus.canin[currentStatus.current_caninchannel] = readAuxanalog(configPage9.Auxinpina[currentStatus.current_caninchannel]&127);
          }
          else if ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 3)  //if current input channel is enabled as digital local pin
          {
            //read digital channel specified
            currentStatus.canin[currentStatus.current_caninchannel] = readAuxdigital((configPage9.Auxinpinb[currentStatus.current_caninchannel]&127)+1);
          } //Channel type
        } //For loop going through each channel
      } //aux channels are enabled

       vvtControl();
       idleControl(); //Perform any idle related actions. Even at higher frequencies, running 4x per second is sufficient.
    } //4Hz timer
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1HZ)) //Once per second)
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_1HZ);
      readBaro(); //Infrequent baro readings are not an issue.
    } //1Hz timer

    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL) )  { idleControl(); } //Run idlecontrol every loop for stepper idle.

    //Always check for sync
    //Main loop runs within this clause
    if (currentStatus.hasSync && (currentStatus.RPM > 0))
    {
        if(currentStatus.startRevolutions >= configPage4.StgCycles)  { ignitionOn = true; fuelOn = true; } //Enable the fuel and ignition, assuming staging revolutions are complete
        //If it is, check is we're running or cranking
        if(currentStatus.RPM > currentStatus.crankRPM) //Crank RPM in the config is stored as a x10. currentStatus.crankRPM is set in timers.ino and represents the true value
        {
          BIT_SET(currentStatus.engine, BIT_ENGINE_RUN); //Sets the engine running bit
          //Only need to do anything if we're transitioning from cranking to running
          if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
          {
            BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK); //clears the engine cranking bit
            if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, HIGH); }
          }
        }
        else
        {  //Sets the engine cranking bit, clears the engine running bit
          BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
          BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN);
          currentStatus.runSecs = 0; //We're cranking (hopefully), so reset the engine run time to prompt ASE.
          if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); }
        }
      //END SETTING STATUSES
      //-----------------------------------------------------------------------------------------------------

      //Begin the fuel calculation
      //Calculate an injector pulsewidth from the VE
      currentStatus.corrections = correctionsFuel();

      currentStatus.VE = getVE();
      currentStatus.advance = getAdvance();
      currentStatus.PW1 = PW(req_fuel_uS, currentStatus.VE, currentStatus.MAP, currentStatus.corrections, inj_opentime_uS);

      //Manual adder for nitrous. These are not in correctionsFuel() because  they are direct adders to the ms value, not % based
      if(currentStatus.nitrous_status == NITROUS_STAGE1)
      { 
        int16_t adderRange = (configPage10.n2o_stage1_maxRPM - configPage10.n2o_stage1_minRPM) * 100;
        int16_t adderPercent = ((currentStatus.RPM - (configPage10.n2o_stage1_minRPM * 100)) * 100) / adderRange; //The percentage of the way through the RPM range
        adderPercent = 100 - adderPercent; //Flip the percentage as we go from a higher adder to a lower adder as the RPMs rise
        currentStatus.PW1 = currentStatus.PW1 + (configPage10.n2o_stage1_adderMax + percentage(adderPercent, (configPage10.n2o_stage1_adderMin - configPage10.n2o_stage1_adderMax))) * 100; //Calculate the above percentage of the calculated ms value.
      }
      if(currentStatus.nitrous_status == NITROUS_STAGE2)
      {
        int16_t adderRange = (configPage10.n2o_stage2_maxRPM - configPage10.n2o_stage2_minRPM) * 100;
        int16_t adderPercent = ((currentStatus.RPM - (configPage10.n2o_stage2_minRPM * 100)) * 100) / adderRange; //The percentage of the way through the RPM range
        adderPercent = 100 - adderPercent; //Flip the percentage as we go from a higher adder to a lower adder as the RPMs rise
        currentStatus.PW1 = currentStatus.PW1 + (configPage10.n2o_stage2_adderMax + percentage(adderPercent, (configPage10.n2o_stage2_adderMin - configPage10.n2o_stage2_adderMax))) * 100; //Calculate the above percentage of the calculated ms value.
      }

      int injector1StartAngle = 0;
      uint16_t injector2StartAngle = 0;
      uint16_t injector3StartAngle = 0;
      uint16_t injector4StartAngle = 0;
      uint16_t injector5StartAngle = 0; //For 5 cylinder testing
#if INJ_CHANNELS >= 6
      int injector6StartAngle = 0;
#endif
#if INJ_CHANNELS >= 7
      int injector7StartAngle = 0;
#endif
#if INJ_CHANNELS >= 8
      int injector8StartAngle = 0;
#endif
      int ignition1StartAngle = 0;
      int ignition2StartAngle = 0;
      int ignition3StartAngle = 0;
      int ignition4StartAngle = 0;
      int ignition5StartAngle = 0;
#if IGN_CHANNELS >= 6
      int ignition6StartAngle = 0;
#endif
#if IGN_CHANNELS >= 7
      int ignition7StartAngle = 0;
#endif
#if IGN_CHANNELS >= 8
      int ignition8StartAngle = 0;
#endif
      //These are used for comparisons on channels above 1 where the starting angle (for injectors or ignition) can be less than a single loop time
      //(Don't ask why this is needed, it will break your head)
      int tempCrankAngle;
      int tempStartAngle;

      //********************************************************
      //How fast are we going? Need to know how long (uS) it will take to get from one tooth to the next. We then use that to estimate how far we are between the last tooth and the next one
      //We use a 1st Deriv accleration prediction, but only when there is an even spacing between primary sensor teeth
      //Any decoder that has uneven spacing has its triggerToothAngle set to 0
      if( (secondDerivEnabled > 0) && (toothHistoryIndex >= 3) && (currentStatus.RPM < 2000) ) //toothHistoryIndex must be greater than or equal to 3 as we need the last 3 entries. Currently this mode only runs below 3000 rpm
      //if(true)
      {
        //Only recalculate deltaV if the tooth has changed since last time (DeltaV stays the same until the next tooth)
        //if (deltaToothCount != toothCurrentCount)
        {
          deltaToothCount = toothCurrentCount;
          int angle1, angle2; //These represent the crank angles that are travelled for the last 2 pulses
          if(configPage4.TrigPattern == 4)
          {
            //Special case for 70/110 pattern on 4g63
            angle2 = triggerToothAngle; //Angle 2 is the most recent
            if (angle2 == 70) { angle1 = 110; }
            else { angle1 = 70; }
          }
          else if(configPage4.TrigPattern == 0)
          {
            //Special case for missing tooth decoder where the missing tooth was one of the last 2 seen
            if(toothCurrentCount == 1) { angle2 = 2*triggerToothAngle; angle1 = triggerToothAngle; }
            else if(toothCurrentCount == 2) { angle1 = 2*triggerToothAngle; angle2 = triggerToothAngle; }
            else { angle1 = triggerToothAngle; angle2 = triggerToothAngle; }
          }
          else { angle1 = triggerToothAngle; angle2 = triggerToothAngle; }

          long toothDeltaV = (1000000L * angle2 / toothHistory[toothHistoryIndex]) - (1000000L * angle1 / toothHistory[toothHistoryIndex-1]);
          long toothDeltaT = toothHistory[toothHistoryIndex];
          //long timeToLastTooth = micros() - toothLastToothTime;

          rpmDelta = (toothDeltaV << 10) / (6 * toothDeltaT);
        }

          timePerDegreex16 = ldiv( 2666656L, currentStatus.RPM + rpmDelta).quot; //This give accuracy down to 0.1 of a degree and can provide noticably better timing results on low res triggers
          timePerDegree = timePerDegreex16 / 16;
      }
      else
      {
        //If we can, attempt to get the timePerDegree by comparing the times of the last two teeth seen. This is only possible for evenly spaced teeth
        if( (triggerToothAngleIsCorrect == true) && (toothLastToothTime > toothLastMinusOneToothTime) && false ) //This is currently NOT working. Don't know why yet
        {
          noInterrupts();
          unsigned long tempToothLastToothTime = toothLastToothTime;
          unsigned long tempToothLastMinusOneToothTime = toothLastMinusOneToothTime;
          uint16_t tempTriggerToothAngle = triggerToothAngle;
          interrupts();
          timePerDegreex16 = (unsigned long)( (tempToothLastToothTime - tempToothLastMinusOneToothTime)*16) / tempTriggerToothAngle;
          timePerDegree = timePerDegreex16 / 16;
        }
        else
        {
          long timeThisRevolution = (micros_safe() - toothOneTime); //micros() is no longer interrupt safe
          long rpm_adjust = (timeThisRevolution * (long)currentStatus.rpmDOT) / 1000000; //Take into account any likely accleration that has occurred since the last full revolution completed
          rpm_adjust = 0;
          timePerDegreex16 = ldiv( 2666656L, currentStatus.RPM + rpm_adjust).quot; //The use of a x16 value gives accuracy down to 0.1 of a degree and can provide noticably better timing results on low res triggers
          timePerDegree = timePerDegreex16 / 16;
        }

      }
      degreesPeruSx2048 = 2048 / timePerDegree;

      //Check that the duty cycle of the chosen pulsewidth isn't too high.
      unsigned long pwLimit = percentage(configPage2.dutyLim, revolutionTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
      if (CRANK_ANGLE_MAX_INJ == 720) { pwLimit = pwLimit * 2; } //For sequential, the maximum pulse time is double (2 revolutions). Wouldn't work for 2 stroke...
      else if (CRANK_ANGLE_MAX_INJ < 360) { pwLimit = pwLimit / currentStatus.nSquirts; } //Handle cases where there are multiple squirts per rev
      //Apply the pwLimit if staging is dsiabled and engine is not cranking
      if( (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) && (configPage10.stagingEnabled == false) ) { if (currentStatus.PW1 > pwLimit) { currentStatus.PW1 = pwLimit; } }

      //Calculate staging pulsewidths if used
      //To run staged injection, the number of cylinders must be less than or equal to the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
      if( (configPage10.stagingEnabled == true) && (configPage2.nCylinders <= INJ_CHANNELS) )
      {
        //Scale the 'full' pulsewidth by each of the injector capacities
        uint32_t tempPW1 = ((unsigned long)currentStatus.PW1 * staged_req_fuel_mult_pri) / 100;

        if(configPage10.stagingMode == STAGING_MODE_TABLE)
        {
          uint32_t tempPW3 = ((unsigned long)currentStatus.PW1 * staged_req_fuel_mult_sec) / 100; //This is ONLY needed in in table mode. Auto mode only calculates the difference.

          byte stagingSplit = get3DTableValue(&stagingTable, currentStatus.MAP, currentStatus.RPM);
          currentStatus.PW1 = ((100 - stagingSplit) * tempPW1) / 100;

          if(stagingSplit > 0) { currentStatus.PW3 = (stagingSplit * tempPW3) / 100; }
          else { currentStatus.PW3 = 0; }
        }
        else if(configPage10.stagingMode == STAGING_MODE_AUTO)
        {
          currentStatus.PW1 = tempPW1;
          //If automatic mode, the primary injectors are used all the way up to their limit (COnfigured by the pulsewidth limit setting)
          //If they exceed their limit, the extra duty is passed to the secondaries
          if(tempPW1 > pwLimit)
          {
            uint32_t extraPW = tempPW1 - pwLimit;
            currentStatus.PW1 = pwLimit;
            currentStatus.PW3 = ((extraPW * staged_req_fuel_mult_sec) / staged_req_fuel_mult_pri) + inj_opentime_uS; //Convert the 'left over' fuel amount from primary injector scaling to secondary
          }
          else { currentStatus.PW3 = 0; } //If tempPW1 < pwLImit it means that the entire fuel load can be handled by the primaries. Simply set the secondaries to 0
        }

        //currentStatus.PW3 = 2000;
        //Set the 2nd channel of each stage with the same pulseWidth
        currentStatus.PW2 = currentStatus.PW1;
        currentStatus.PW4 = currentStatus.PW3;
      }
      else 
      { 
        //If staging is off, all the pulse widths are set the same (Sequential and other adjustments may be made below)
        currentStatus.PW2 = currentStatus.PW1;
        currentStatus.PW3 = currentStatus.PW1;
        currentStatus.PW4 = currentStatus.PW1;
        currentStatus.PW5 = currentStatus.PW1;
        currentStatus.PW6 = currentStatus.PW1;
        currentStatus.PW7 = currentStatus.PW1; 
      }

      //***********************************************************************************************
      //BEGIN INJECTION TIMING
      //Determine next firing angles
      if(!configPage2.indInjAng) 
      {
        //Forcing all injector close angles to be the same.
        configPage2.inj2Ang = configPage2.inj1Ang;
        configPage2.inj3Ang = configPage2.inj1Ang;
        configPage2.inj4Ang = configPage2.inj1Ang;
      } 
      unsigned int PWdivTimerPerDegree = div(currentStatus.PW1, timePerDegree).quot; //How many crank degrees the calculated PW will take at the current speed
      //This is a little primitive, but is based on the idea that all fuel needs to be delivered before the inlet valve opens. See http://www.extraefi.co.uk/sequential_fuel.html for more detail
      if(configPage2.inj1Ang > PWdivTimerPerDegree) { injector1StartAngle = configPage2.inj1Ang - ( PWdivTimerPerDegree ); }
      else { injector1StartAngle = configPage2.inj1Ang + CRANK_ANGLE_MAX_INJ - PWdivTimerPerDegree; } //Just incase 
      if(injector1StartAngle > CRANK_ANGLE_MAX_INJ) {injector1StartAngle -= CRANK_ANGLE_MAX_INJ;}

      //Repeat the above for each cylinder
      switch (configPage2.nCylinders)
      {
        //2 cylinders
        case 2:
          /*
          injector2StartAngle = (configPage2.inj2Ang + channel2InjDegrees);
          if(injector2StartAngle < PWdivTimerPerDegree) { injector2StartAngle += CRANK_ANGLE_MAX_INJ; }
          injector2StartAngle -= PWdivTimerPerDegree;
          if(injector2StartAngle > CRANK_ANGLE_MAX_INJ) { injector2StartAngle -= CRANK_ANGLE_MAX_INJ; }
          */
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          break;
        //3 cylinders
        case 3:
          /*
          injector2StartAngle = (configPage2.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > CRANK_ANGLE_MAX_INJ) {injector2StartAngle -= CRANK_ANGLE_MAX_INJ;}
          if(injector2StartAngle < 0) {injector2StartAngle += CRANK_ANGLE_MAX_INJ;}
          */
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);

          /*
          injector3StartAngle = (configPage2.inj3Ang + channel3InjDegrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > CRANK_ANGLE_MAX_INJ) {injector3StartAngle -= CRANK_ANGLE_MAX_INJ;}
          if(injector3StartAngle < 0) {injector3StartAngle += CRANK_ANGLE_MAX_INJ;}
          */
          injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);

          break;
        //4 cylinders
        case 4:
          /*
          injector2StartAngle = (configPage2.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > CRANK_ANGLE_MAX_INJ) {injector2StartAngle -= CRANK_ANGLE_MAX_INJ;}
          if(injector2StartAngle < 0) {injector2StartAngle += CRANK_ANGLE_MAX_INJ;}
          */
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);

          if(configPage2.injLayout == INJ_SEQUENTIAL)
          {
            /*
            injector3StartAngle = (configPage2.inj3Ang + channel3InjDegrees - ( PWdivTimerPerDegree ));
            if(injector3StartAngle > CRANK_ANGLE_MAX_INJ) {injector3StartAngle -= CRANK_ANGLE_MAX_INJ;}
            if(injector3StartAngle < 0) {injector3StartAngle += CRANK_ANGLE_MAX_INJ;}
            */
            injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);

            /*
            injector4StartAngle = (configPage2.inj4Ang + channel4InjDegrees - ( PWdivTimerPerDegree ));
            if(injector4StartAngle > CRANK_ANGLE_MAX_INJ) {injector4StartAngle -= CRANK_ANGLE_MAX_INJ;}
            if(injector4StartAngle < 0) {injector4StartAngle += CRANK_ANGLE_MAX_INJ;}
            */
            injector4StartAngle = calculateInjector4StartAngle(PWdivTimerPerDegree);

            if(configPage6.fuelTrimEnabled > 0)
            {
              unsigned long pw1percent = 100 + (byte)get3DTableValue(&trim1Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;
              unsigned long pw2percent = 100 + (byte)get3DTableValue(&trim2Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;
              unsigned long pw3percent = 100 + (byte)get3DTableValue(&trim3Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;
              unsigned long pw4percent = 100 + (byte)get3DTableValue(&trim4Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;

              if (pw1percent != 100) { currentStatus.PW1 = (pw1percent * currentStatus.PW1) / 100; }
              if (pw2percent != 100) { currentStatus.PW2 = (pw2percent * currentStatus.PW2) / 100; }
              if (pw3percent != 100) { currentStatus.PW3 = (pw3percent * currentStatus.PW3) / 100; }
              if (pw4percent != 100) { currentStatus.PW4 = (pw4percent * currentStatus.PW4) / 100; }
            }
          }
          else if(configPage10.stagingEnabled == true)
          {
            PWdivTimerPerDegree = div(currentStatus.PW3, timePerDegree).quot; //Need to redo this for PW3 as it will be dramatically different to PW1 when staging
            
            /*
            injector3StartAngle = configPage2.inj3Ang - ( PWdivTimerPerDegree ); //This is a little primitive, but is based on the idea that all fuel needs to be delivered before the inlet valve opens. See http://www.extraefi.co.uk/sequential_fuel.html for more detail
            if(injector3StartAngle < 0) {injector3StartAngle += CRANK_ANGLE_MAX_INJ;}
            if(injector3StartAngle > CRANK_ANGLE_MAX_INJ) {injector3StartAngle -= CRANK_ANGLE_MAX_INJ;}
            */
            injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);

            injector4StartAngle = injector3StartAngle + (CRANK_ANGLE_MAX_INJ / 2); //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
            if(injector4StartAngle < 0) {injector4StartAngle += CRANK_ANGLE_MAX_INJ;}
            if(injector4StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { injector4StartAngle -= CRANK_ANGLE_MAX_INJ; }
          }
          break;
        //5 cylinders
        case 5:
          /*
          injector2StartAngle = (configPage2.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > CRANK_ANGLE_MAX_INJ) {injector2StartAngle -= CRANK_ANGLE_MAX_INJ;}
          */
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);

          /*
          injector3StartAngle = (configPage2.inj3Ang + channel3InjDegrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > CRANK_ANGLE_MAX_INJ) {injector3StartAngle -= CRANK_ANGLE_MAX_INJ;}
          */
          injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);

          /*
          injector4StartAngle = (configPage2.inj4Ang + channel4InjDegrees - ( PWdivTimerPerDegree ));
          if(injector4StartAngle > CRANK_ANGLE_MAX_INJ) {injector4StartAngle -= CRANK_ANGLE_MAX_INJ;}
          */
          injector4StartAngle = calculateInjector4StartAngle(PWdivTimerPerDegree);

          /*
          injector5StartAngle = (configPage2.inj1Ang + channel5InjDegrees - ( PWdivTimerPerDegree ));
          if(injector5StartAngle > CRANK_ANGLE_MAX_INJ) {injector5StartAngle -= CRANK_ANGLE_MAX_INJ;}
          */
          injector5StartAngle = calculateInjector5StartAngle(PWdivTimerPerDegree);
          break;
        //6 cylinders
        case 6:
          /*
          injector2StartAngle = (configPage2.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > CRANK_ANGLE_MAX_INJ) {injector2StartAngle -= CRANK_ANGLE_MAX_INJ;}
          */
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);

          /*
          injector3StartAngle = (configPage2.inj3Ang + channel3InjDegrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > CRANK_ANGLE_MAX_INJ) {injector3StartAngle -= CRANK_ANGLE_MAX_INJ;}
          */
          injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
#if INJ_CHANNELS >= 6
          if(configPage2.injLayout == INJ_SEQUENTIAL)
          {
            injector4StartAngle = (configPage2.inj1Ang + channel4InjDegrees - ( PWdivTimerPerDegree ));
            if(injector4StartAngle > CRANK_ANGLE_MAX_INJ) {injector4StartAngle -= CRANK_ANGLE_MAX_INJ;}
            injector5StartAngle = (configPage2.inj2Ang + channel5InjDegrees - ( PWdivTimerPerDegree ));
            if(injector5StartAngle > CRANK_ANGLE_MAX_INJ) {injector5StartAngle -= CRANK_ANGLE_MAX_INJ;}
            injector6StartAngle = (configPage2.inj3Ang + channel6InjDegrees - ( PWdivTimerPerDegree ));
            if(injector6StartAngle > CRANK_ANGLE_MAX_INJ) {injector6StartAngle -= CRANK_ANGLE_MAX_INJ;}
          }
#endif
          break;
        //8 cylinders
        case 8:
          /*
          injector2StartAngle = (configPage2.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > CRANK_ANGLE_MAX_INJ) {injector2StartAngle -= CRANK_ANGLE_MAX_INJ;}
          */
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);

          /*
          injector3StartAngle = (configPage2.inj3Ang + channel3InjDegrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > CRANK_ANGLE_MAX_INJ) {injector3StartAngle -= CRANK_ANGLE_MAX_INJ;}
          */
          injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);

          /*
          injector4StartAngle = (configPage2.inj4Ang + channel4InjDegrees - ( PWdivTimerPerDegree ));
          if(injector4StartAngle > CRANK_ANGLE_MAX_INJ) {injector4StartAngle -= CRANK_ANGLE_MAX_INJ;}
          */
          injector4StartAngle = calculateInjector4StartAngle(PWdivTimerPerDegree);
          break;
        //Will hit the default case on 1 cylinder or >8 cylinders. Do nothing in these cases
        default:
          break;
      }

      //***********************************************************************************************
      //| BEGIN IGNITION CALCULATIONS
      if (currentStatus.RPM > ((unsigned int)(configPage4.HardRevLim) * 100) ) { BIT_SET(currentStatus.spark, BIT_SPARK_HRDLIM); } //Hardcut RPM limit
      else { BIT_CLEAR(currentStatus.spark, BIT_SPARK_HRDLIM); }


      //Set dwell
       //Dwell is stored as ms * 10. ie Dwell of 4.3ms would be 43 in configPage4. This number therefore needs to be multiplied by 100 to get dwell in uS
      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { currentStatus.dwell =  (configPage4.dwellCrank * 100); }
      else { currentStatus.dwell =  (configPage4.dwellRun * 100); }
      currentStatus.dwell = correctionsDwell(currentStatus.dwell);

      int dwellAngle = timeToAngle(currentStatus.dwell, CRANKMATH_METHOD_INTERVAL_REV); //Convert the dwell time to dwell angle based on the current engine speed

      //Calculate start angle for each channel
      //1 cylinder (Everyone gets this)
      ignition1EndAngle = CRANK_ANGLE_MAX_IGN - currentStatus.advance;
      if(ignition1EndAngle > CRANK_ANGLE_MAX_IGN) {ignition1EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition1StartAngle = ignition1EndAngle - dwellAngle; // 360 - desired advance angle - number of degrees the dwell will take
      if(ignition1StartAngle < 0) {ignition1StartAngle += CRANK_ANGLE_MAX_IGN;}

      //This test for more cylinders and do the same thing
      switch (configPage2.nCylinders)
      {
        //2 cylinders
        case 2:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}
          break;
        //3 cylinders
        case 3:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
          if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition3StartAngle = channel3IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}
          break;
        //4 cylinders
        case 4:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
          {
            ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
            if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
            ignition3StartAngle = ignition3EndAngle - dwellAngle;
            if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

            ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
            if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
            ignition4StartAngle = ignition4EndAngle - dwellAngle;
            if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
          }
          else if(configPage4.sparkMode == IGN_MODE_ROTARY)
          {
            if(configPage10.rotaryType == ROTARY_IGN_FC)
            {
              byte splitDegrees = 0;
              if (configPage2.fuelAlgorithm == LOAD_SOURCE_MAP) { splitDegrees = table2D_getValue(&rotarySplitTable, currentStatus.MAP/2); }
              else { splitDegrees = table2D_getValue(&rotarySplitTable, currentStatus.TPS/2); }

              //The trailing angles are set relative to the leading ones
              ignition3EndAngle = ignition1EndAngle + splitDegrees;
              ignition3StartAngle = ignition3EndAngle - dwellAngle;
              if(ignition3StartAngle > CRANK_ANGLE_MAX_IGN) {ignition3StartAngle -= CRANK_ANGLE_MAX_IGN;}
              if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

              ignition4EndAngle = ignition2EndAngle + splitDegrees;
              ignition4StartAngle = ignition4EndAngle - dwellAngle;
              if(ignition4StartAngle > CRANK_ANGLE_MAX_IGN) {ignition4StartAngle -= CRANK_ANGLE_MAX_IGN;}
              if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
            }
          }
          break;
        //5 cylinders
        case 5:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
          if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition3StartAngle = ignition3EndAngle - dwellAngle;
          if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
          if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition4StartAngle = ignition4EndAngle - dwellAngle;
          if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition5EndAngle = channel5IgnDegrees - currentStatus.advance - dwellAngle;
          if(ignition5EndAngle > CRANK_ANGLE_MAX_IGN) {ignition5EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition5StartAngle = ignition5EndAngle - dwellAngle;
          if(ignition5StartAngle < 0) {ignition5StartAngle += CRANK_ANGLE_MAX_IGN;}

          break;
        //6 cylinders
        case 6:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
          if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition3StartAngle = ignition3EndAngle - dwellAngle;
          if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}
          break;
        //8 cylinders
        case 8:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
          if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition3StartAngle = ignition3EndAngle - dwellAngle;
          if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
          if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition4StartAngle = ignition4EndAngle - dwellAngle;
          if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
          break;

        //Will hit the default case on 1 cylinder or >8 cylinders. Do nothing in these cases
        default:
          break;
      }
      //If ignition timing is being tracked per tooth, perform the calcs to get the end teeth
      //This only needs to be run if the advance figure has changed, otherwise the end teeth will still be the same
      if( (configPage2.perToothIgn == true) && (lastToothCalcAdvance != currentStatus.advance) ) { triggerSetEndTeeth(); }

      //***********************************************************************************************
      //| BEGIN FUEL SCHEDULES
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the shcedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time

      //Determine the current crank angle
      int crankAngle = getCrankAngle();
      if (crankAngle > CRANK_ANGLE_MAX_INJ ) { crankAngle -= CRANK_ANGLE_MAX_INJ; }

      if(Serial && false)
      {
        if(ignition1StartAngle > crankAngle)
        {
          noInterrupts();
          Serial.print("Time2LastTooth:"); Serial.println(micros()-toothLastToothTime);
          Serial.print("elapsedTime:"); Serial.println(elapsedTime);
          Serial.print("CurAngle:"); Serial.println(crankAngle);
          Serial.print("RPM:"); Serial.println(currentStatus.RPM);
          Serial.print("Tooth:"); Serial.println(toothCurrentCount);
          Serial.print("timePerDegree:"); Serial.println(timePerDegree);
          Serial.print("IGN1Angle:"); Serial.println(ignition1StartAngle);
          Serial.print("TimeToIGN1:"); Serial.println(angleToTime((ignition1StartAngle - crankAngle), CRANKMATH_METHOD_INTERVAL_REV));
          interrupts();
        }
      }

#if INJ_CHANNELS >= 1
      if (fuelOn && !BIT_CHECK(currentStatus.status1, BIT_STATUS1_BOOSTCUT))
      {
        if(currentStatus.PW1 >= inj_opentime_uS)
        {
          if ( (injector1StartAngle <= crankAngle) && (fuelSchedule1.Status == RUNNING) ) { injector1StartAngle += CRANK_ANGLE_MAX_INJ; }
          if (injector1StartAngle > crankAngle)
          {
            setFuelSchedule1(
                      ((injector1StartAngle - crankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW1
                      );
          }
        }
#endif

        /*-----------------------------------------------------------------------------------------
        | A Note on tempCrankAngle and tempStartAngle:
        |   The use of tempCrankAngle/tempStartAngle is described below. It is then used in the same way for channels 2, 3 and 4 on both injectors and ignition
        |   Essentially, these 2 variables are used to realign the current crank angle and the desired start angle around 0 degrees for the given cylinder/output
        |   Eg: If cylinder 2 TDC is 180 degrees after cylinder 1 (Eg a standard 4 cylidner engine), then tempCrankAngle is 180* less than the current crank angle and
        |       tempStartAngle is the desired open time less 180*. Thus the cylinder is being treated relative to its own TDC, regardless of its offset
        |
        |   This is done to avoid problems with very short of very long times until tempStartAngle.
        |   This will very likely need to be rewritten when sequential is enabled
        |------------------------------------------------------------------------------------------
        */
#if INJ_CHANNELS >= 2
        if( (channel2InjEnabled) && (currentStatus.PW2 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel2InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector2StartAngle - channel2InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule2.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule2(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW2
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 3
        if( (channel3InjEnabled) && (currentStatus.PW3 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel3InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector3StartAngle - channel3InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule3.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule3(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW3
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 4
        if( (channel4InjEnabled) && (currentStatus.PW4 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel4InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector4StartAngle - channel4InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule4.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule4(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW4
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 5
        if( (channel5InjEnabled) && (currentStatus.PW4 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel5InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector5StartAngle - channel5InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if (tempStartAngle <= tempCrankAngle && fuelSchedule5.schedulesSet == 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            //Note the hacky use of fuel schedule 3 below
            /*
            setFuelSchedule3(openInjector3and5,
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW1,
                      closeInjector3and5
                    );*/
            setFuelSchedule3(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW1
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 6
        if( (channel6InjEnabled) && (currentStatus.PW6 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel6InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector6StartAngle - channel6InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule6.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule6(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW6
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 7
        if( (channel7InjEnabled) && (currentStatus.PW7 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel7InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector7StartAngle - channel7InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule7.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule7(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW7
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 8
        if( (channel8InjEnabled) && (currentStatus.PW8 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel8InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector8StartAngle - channel8InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule8.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule8(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW8
                      );
          }
        }
#endif
      }
      //***********************************************************************************************
      //| BEGIN IGNITION SCHEDULES
      //Likewise for the ignition

      //fixedCrankingOverride is used to extend the dwell during cranking so that the decoder can trigger the spark upon seeing a certain tooth. Currently only available on the basic distributor and 4g63 decoders.
      if ( configPage4.ignCranklock && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (decoderHasFixedCrankingTiming == true) )
      {
        fixedCrankingOverride = currentStatus.dwell * 3;
        //This is a safety step to prevent the ignition start time occuring AFTER the target tooth pulse has already occcured. It simply moves the start time forward a little, which is compensated for by the increase in the dwell time
        if(currentStatus.RPM < 250)
        {
          ignition1StartAngle -= 5;
          ignition2StartAngle -= 5;
          ignition3StartAngle -= 5;
          ignition4StartAngle -= 5;
        }
      }
      else { fixedCrankingOverride = 0; }

      //Perform an initial check to see if the ignition is turned on (Ignition only turns on after a preset number of cranking revolutions and:
      //Check for any of the hard cut rev limits being on
      if(currentStatus.launchingHard || BIT_CHECK(currentStatus.spark, BIT_SPARK_BOOSTCUT) || BIT_CHECK(currentStatus.spark, BIT_SPARK_HRDLIM) || currentStatus.flatShiftingHard)
      {
        if(configPage2.hardCutType == HARD_CUT_FULL) { ignitionOn = false; }
        else 
        { 
          if(rollingCutCounter >= 2) //Vary this number to change the intensity of the roll. The higher the number, the closer is it to full cut
          { 
            //Rolls through each of the active ignition channels based on how many revolutions have taken place
            //curRollingCut = ( (currentStatus.startRevolutions / 2) % maxIgnOutputs) + 1;
            rollingCutCounter = 0;
            ignitionOn = true;
            curRollingCut = 0;
          }
          else
          {
            if(rollingCutLastRev == 0) { rollingCutLastRev = currentStatus.startRevolutions; } //
            if (rollingCutLastRev != currentStatus.startRevolutions)
            {
              rollingCutLastRev = currentStatus.startRevolutions;
              rollingCutCounter++;
            }
            ignitionOn = false; //Finally the ignition is fully cut completely
          }
        } 
      }
      else { curRollingCut = 0; } //Disables the rolling hard cut

      //if(ignitionOn && !currentStatus.launchingHard && !BIT_CHECK(currentStatus.spark, BIT_SPARK_BOOSTCUT) && !BIT_CHECK(currentStatus.spark, BIT_SPARK_HRDLIM) && !currentStatus.flatShiftingHard)
      if(ignitionOn)
      {
        //Refresh the current crank angle info
        //ignition1StartAngle = 335;
        crankAngle = getCrankAngle(); //Refresh with the latest crank angle
        if (crankAngle > CRANK_ANGLE_MAX_IGN ) { crankAngle -= 360; }

#if IGN_CHANNELS >= 1
        if ( (ignition1StartAngle > crankAngle) && (curRollingCut != 1) )
        {
            /*
            long some_time = ((unsigned long)(ignition1StartAngle - crankAngle) * (unsigned long)timePerDegree);
            long newRPM = (long)(some_time * currentStatus.rpmDOT) / 1000000L;
            newRPM = currentStatus.RPM + (newRPM/2);
            unsigned long timePerDegree_1 = ldiv( 166666L, newRPM).quot;
            unsigned long timeout = (unsigned long)(ignition1StartAngle - crankAngle) * 282UL;
            */
            if(ignitionSchedule1.Status != RUNNING)
            {
              setIgnitionSchedule1(ign1StartFunction,
                        //((unsigned long)(ignition1StartAngle - crankAngle) * (unsigned long)timePerDegree),
                        angleToTime((ignition1StartAngle - crankAngle), CRANKMATH_METHOD_INTERVAL_REV),
                        currentStatus.dwell + fixedCrankingOverride, //((unsigned long)((unsigned long)currentStatus.dwell* currentStatus.RPM) / newRPM) + fixedCrankingOverride,
                        ign1EndFunction
                        );
            }
        }
#endif

#if defined(USE_IGN_REFRESH)
        if( (ignitionSchedule1.Status == RUNNING) && (ignition1EndAngle > crankAngle) && (configPage4.StgCycles == 0) )
        {
          unsigned long uSToEnd = 0;

          crankAngle = getCrankAngle(); //Refresh with the latest crank angle
          if (crankAngle > CRANK_ANGLE_MAX_IGN ) { crankAngle -= 360; }
          
          //ONLY ONE OF THE BELOW SHOULD BE USED (PROBABLY THE FIRST):
          //*********
          if(ignition1EndAngle > crankAngle) { uSToEnd = fastDegreesToUS( (ignition1EndAngle - crankAngle) ); }
          else { uSToEnd = fastDegreesToUS( (360 + ignition1EndAngle - crankAngle) ); }
          //*********
          //uSToEnd = ((ignition1EndAngle - crankAngle) * (toothLastToothTime - toothLastMinusOneToothTime)) / triggerToothAngle;
          //*********

          refreshIgnitionSchedule1( uSToEnd + fixedCrankingOverride );
        }
  #endif
        


#if IGN_CHANNELS >= 2
        tempCrankAngle = crankAngle - channel2IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition2StartAngle - channel2IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        {
            unsigned long ignition2StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition2StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition2StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition2StartTime = 0; }

            if( (ignition2StartTime > 0) && (curRollingCut != 2) )
            {
              setIgnitionSchedule2(ign2StartFunction,
                        ignition2StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign2EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 3
        tempCrankAngle = crankAngle - channel3IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition3StartAngle - channel3IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {
            long ignition3StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition3StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition4StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition3StartTime = 0; }

            if( (ignition3StartTime > 0) && (curRollingCut != 3) )
            {
              setIgnitionSchedule3(ign3StartFunction,
                        ignition3StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign3EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 4
        tempCrankAngle = crankAngle - channel4IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition4StartAngle - channel4IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {

            long ignition4StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition4StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition4StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition4StartTime = 0; }

            if( (ignition4StartTime > 0) && (curRollingCut != 4) )
            {
              setIgnitionSchedule4(ign4StartFunction,
                        ignition4StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign4EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 5
        tempCrankAngle = crankAngle - channel5IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition5StartAngle - channel5IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        //if (tempStartAngle > tempCrankAngle)
        {

            long ignition5StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition5StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            //else if (tempStartAngle < tempCrankAngle) { ignition4StartTime = ((long)(360 - tempCrankAngle + tempStartAngle) * (long)timePerDegree); }
            else { ignition5StartTime = 0; }

            if( (ignition5StartTime > 0) && (curRollingCut != 5) ) {
            setIgnitionSchedule5(ign5StartFunction,
                      ignition5StartTime,
                      currentStatus.dwell + fixedCrankingOverride,
                      ign5EndFunction
                      );
            }
        }
#endif

#if IGN_CHANNELS >= 6
        tempCrankAngle = crankAngle - channel6IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition6StartAngle - channel6IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        {
            unsigned long ignition6StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition6StartTime = angleToTime((tempStartAngle - tempCrankAngle)); }
            else { ignition6StartTime = 0; }

            if( (ignition6StartTime > 0) && (curRollingCut != 2) )
            {
              setIgnitionSchedule6(ign6StartFunction,
                        ignition6StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign6EndFunction
                        );
            }
        }
#endif

      } //Ignition schedules on

      if ( (!BIT_CHECK(currentStatus.status3, BIT_STATUS3_RESET_PREVENT)) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) ) 
      {
        //Reset prevention is supposed to be on while the engine is running but isn't. Fix that.
        digitalWrite(pinResetControl, HIGH);
        BIT_SET(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      }
    } //Has sync and RPM
    else if ( (BIT_CHECK(currentStatus.status3, BIT_STATUS3_RESET_PREVENT) > 0) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) )
    {
      digitalWrite(pinResetControl, LOW);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
    }
} //loop()

/*
  This function retuns a pulsewidth time (in us) given the following:
  REQ_FUEL
  VE: Lookup from the main fuel table. This can either have been MAP or TPS based, depending on the algorithm used
  MAP: In KPa, read from the sensor (This is used when performing a multiply of the map only. It is applicable in both Speed density and Alpha-N)
  GammaE: Sum of Enrichment factors (Cold start, acceleration). This is a multiplication factor (Eg to add 10%, this should be 110)
  injDT: Injector dead time. The time the injector take to open minus the time it takes to close (Both in uS)
*/
uint16_t PW(int REQ_FUEL, byte VE, long MAP, int corrections, int injOpen)
{
  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpen);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint16_t iVE, iCorrections;
  uint16_t iMAP = 100;
  uint16_t iAFR = 147;

  //100% float free version, does sacrifice a little bit of accuracy, but not much.
  iVE = ((unsigned int)VE << 7) / 100;
  if ( configPage2.multiplyMAP == true ) {
    iMAP = ((unsigned int)MAP << 7) / currentStatus.baro;  //Include multiply MAP (vs baro) if enabled
  }
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == 2)) {
    iAFR = ((unsigned int)currentStatus.O2 << 7) / currentStatus.afrTarget;  //Include AFR (vs target) if enabled
  }
  iCorrections = (corrections << 7) / 100;


  unsigned long intermediate = ((long)REQ_FUEL * (long)iVE) >> 7; //Need to use an intermediate value to avoid overflowing the long
  if ( configPage2.multiplyMAP == true ) {
    intermediate = (intermediate * (unsigned long)iMAP) >> 7;
  }
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == 2) ) {
    intermediate = (intermediate * (unsigned long)iAFR) >> 7;  //EGO type must be set to wideband for this to be used
  }
  intermediate = (intermediate * (unsigned long)iCorrections) >> 7;
  if (intermediate != 0)
  {
    //If intermeditate is not 0, we need to add the opening time (0 typically indicates that one of the full fuel cuts is active)
    intermediate += injOpen; //Add the injector opening time
    if ( intermediate > 65535)
    {
      intermediate = 65535;  //Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
    }
  }
  return (unsigned int)(intermediate);
}

byte getVE()
{
  byte tempVE = 100;
  if (configPage2.fuelAlgorithm == LOAD_SOURCE_MAP) //Check which fuelling algorithm is being used
  {
    //Speed Density
    currentStatus.fuelLoad = currentStatus.MAP;
  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.fuelLoad = currentStatus.TPS;
  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.fuelLoad = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  else { currentStatus.fuelLoad = currentStatus.MAP; } //Fallback position
  tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value

  return tempVE;
}

byte getAdvance()
{
  byte tempAdvance = 0;
  if (configPage2.ignAlgorithm == LOAD_SOURCE_MAP) //Check which fuelling algorithm is being used
  {
    //Speed Density
    currentStatus.ignLoad = currentStatus.MAP;
  }
  else if(configPage2.ignAlgorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.ignLoad = currentStatus.TPS;

  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.ignLoad = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  tempAdvance = get3DTableValue(&ignitionTable, currentStatus.ignLoad, currentStatus.RPM) - OFFSET_IGNITION; //As above, but for ignition advance
  tempAdvance = correctionsIgn(tempAdvance);

  return tempAdvance;
}

uint16_t calculateInjector2StartAngle(unsigned int PWdivTimerPerDegree)
{
  uint16_t tempInjector2StartAngle = (configPage2.inj2Ang + channel2InjDegrees); //This makes the start angle equal to the end angle
  if(tempInjector2StartAngle < PWdivTimerPerDegree) { tempInjector2StartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjector2StartAngle -= PWdivTimerPerDegree; //Subtract the number of degrees the PW will take to get the start angle
  if(tempInjector2StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjector2StartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjector2StartAngle;
}
uint16_t calculateInjector3StartAngle(unsigned int PWdivTimerPerDegree)
{
  uint16_t tempInjector3StartAngle = (configPage2.inj3Ang + channel3InjDegrees);
  if(tempInjector3StartAngle < PWdivTimerPerDegree) { tempInjector3StartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjector3StartAngle -= PWdivTimerPerDegree;
  if(tempInjector3StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjector3StartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjector3StartAngle;
}
uint16_t calculateInjector4StartAngle(unsigned int PWdivTimerPerDegree)
{
  uint16_t tempInjector4StartAngle = (configPage2.inj4Ang + channel4InjDegrees);
  if(tempInjector4StartAngle < PWdivTimerPerDegree) { tempInjector4StartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjector4StartAngle -= PWdivTimerPerDegree;
  if(tempInjector4StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjector4StartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjector4StartAngle;
}
uint16_t calculateInjector5StartAngle(unsigned int PWdivTimerPerDegree)
{
  uint16_t tempInjector5StartAngle = (configPage2.inj1Ang + channel4InjDegrees); //Note the use of inj1Ang here
  if(tempInjector5StartAngle < PWdivTimerPerDegree) { tempInjector5StartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjector5StartAngle -= PWdivTimerPerDegree;
  if(tempInjector5StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjector5StartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjector5StartAngle;
}

#endif //Unit testing scope guard
