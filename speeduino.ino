
/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

//**************************************************************************************************
// Config section
#define engineSquirtsPerCycle 2 //Would be 1 for a 2 stroke
//**************************************************************************************************

#include "globals.h"
#include "utils.h"
#include "table.h"
#include "scheduler.h"
#include "comms.h"
#include "math.h"
#include "corrections.h"
#include "timers.h"
#include "display.h"
#include "decoders.h"
#include "idle.h"
#include "auxiliaries.h"

#ifdef __SAM3X8E__
 //Do stuff for ARM based CPUs 
#else
  #include "storage.h"
#endif

#include "fastAnalog.h"
#include <PID_v1.h>

struct config1 configPage1;
struct config2 configPage2;
struct config3 configPage3;
struct config4 configPage4;

int req_fuel_uS, inj_opentime_uS;
#define MAX_RPM 15000 //This is the maximum rpm that the ECU will attempt to run at. It is NOT related to the rev limiter, but is instead dictates how fast certain operations will be allowed to run. Lower number gives better performance

volatile byte startRevolutions = 0; //A counter for how many revolutions have been completed since sync was achieved.
volatile byte ign1LastRev;
volatile byte ign2LastRev;
volatile byte ign3LastRev;
volatile byte ign4LastRev;
bool ignitionOn = false; //The current state of the ignition system
bool fuelOn = false; //The current state of the ignition system
bool fuelPumpOn = false; //The current status of the fuel pump

void (*trigger)(); //Pointer for the trigger function (Gets pointed to the relevant decoder)
void (*triggerSecondary)(); //Pointer for the secondary trigger function (Gets pointed to the relevant decoder)
int (*getRPM)(); //Pointer to the getRPM function (Gets pointed to the relevant decoder)
int (*getCrankAngle)(int); //Pointer to the getCrank Angle function (Gets pointed to the relevant decoder)

struct table3D fuelTable; //16x16 fuel map
struct table3D ignitionTable; //16x16 ignition map
struct table3D afrTable; //16x16 afr target map
struct table3D boostTable; //8x8 boost map
struct table3D vvtTable; //8x8 vvt map
struct table2D taeTable; //4 bin TPS Acceleration Enrichment map (2D)
struct table2D WUETable; //10 bin Warm Up Enrichment map (2D)
struct table2D dwellVCorrectionTable; //6 bin dwell voltage correction (2D)
struct table2D injectorVCorrectionTable; //6 bin injector voltage correction (2D)
struct table2D IATDensityCorrectionTable; //9 bin inlet air temperature density correction (2D)
byte cltCalibrationTable[CALIBRATION_TABLE_SIZE];
byte iatCalibrationTable[CALIBRATION_TABLE_SIZE];
byte o2CalibrationTable[CALIBRATION_TABLE_SIZE];

unsigned long counter;
unsigned long currentLoopTime; //The time the current loop started (uS)
unsigned long previousLoopTime; //The time the previous loop started (uS)

unsigned long MAPrunningValue; //Used for tracking either the total of all MAP readings in this cycle (Event average) or the lowest value detected in this cycle (event minimum)
unsigned int MAPcount; //Number of samples taken in the current MAP cycle
byte MAPcurRev = 0; //Tracks which revolution we're sampling on


byte coilHIGH = HIGH;
byte coilLOW = LOW;
byte fanHIGH = HIGH;             // Used to invert the cooling fan output
byte fanLOW = LOW;               // Used to invert the cooling fan output

struct statuses currentStatus;
volatile int mainLoopCount;
byte ignitionCount;
unsigned long secCounter; //The next time to increment 'runSecs' counter.
int channel1IgnDegrees; //The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones)
int channel2IgnDegrees; //The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC
int channel3IgnDegrees; //The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC
int channel4IgnDegrees; //The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC
int channel1InjDegrees; //The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones)
int channel2InjDegrees; //The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC
int channel3InjDegrees; //The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC
int channel4InjDegrees; //The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC

//These are the functions the get called to begin and end the ignition coil charging. They are required for the various spark output modes
void (*ign1StartFunction)();
void (*ign1EndFunction)();
void (*ign2StartFunction)();
void (*ign2EndFunction)();
void (*ign3StartFunction)();
void (*ign3EndFunction)();
void (*ign4StartFunction)();
void (*ign4EndFunction)();

int timePerDegree;
byte degreesPerLoop; //The number of crank degrees that pass for each mainloop of the program

void setup() 
{
  //Setup the dummy fuel and ignition tables
  //dummyFuelTable(&fuelTable);
  //dummyIgnitionTable(&ignitionTable);
  table3D_setSize(&fuelTable, 16);
  table3D_setSize(&ignitionTable, 16);
  table3D_setSize(&afrTable, 16);
  table3D_setSize(&boostTable, 8);
  table3D_setSize(&vvtTable, 8);
  
  loadConfig();
  
  //Repoint the 2D table structs to the config pages that were just loaded
  taeTable.valueSize = SIZE_BYTE; //Set this table to use byte values
  taeTable.xSize = 4;
  taeTable.values = configPage2.taeValues;
  taeTable.axisX = configPage2.taeBins;
  WUETable.valueSize = SIZE_BYTE; //Set this table to use byte values
  WUETable.xSize = 10;
  WUETable.values = configPage1.wueValues;
  WUETable.axisX = configPage2.wueBins;
  //The WUE X axis values are hard coded (Don't ask, they just are)
  WUETable.axisX[0] = 0;
  WUETable.axisX[1] = 11;
  WUETable.axisX[2] = 22;
  WUETable.axisX[3] = 33;
  WUETable.axisX[4] = 44;
  WUETable.axisX[5] = 56;
  WUETable.axisX[6] = 67;
  WUETable.axisX[7] = 78;
  WUETable.axisX[8] = 94;
  WUETable.axisX[9] = 111;
  
  dwellVCorrectionTable.valueSize = SIZE_BYTE;
  dwellVCorrectionTable.xSize = 6;
  dwellVCorrectionTable.values = configPage2.dwellCorrectionValues;
  dwellVCorrectionTable.axisX = configPage3.voltageCorrectionBins;
  injectorVCorrectionTable.valueSize = SIZE_BYTE;
  injectorVCorrectionTable.xSize = 6;
  injectorVCorrectionTable.values = configPage3.injVoltageCorrectionValues;
  injectorVCorrectionTable.axisX = configPage3.voltageCorrectionBins;
  IATDensityCorrectionTable.valueSize = SIZE_BYTE;
  IATDensityCorrectionTable.xSize = 9;
  IATDensityCorrectionTable.values = configPage3.airDenRates;
  IATDensityCorrectionTable.axisX = configPage3.airDenBins;
  
  //Setup the calibration tables
  loadCalibration();
  //Set the pin mappings
  setPinMapping(configPage1.pinMapping);
  
  //Need to check early on whether the coil charging is inverted. If this is not set straight away it can cause an unwanted spark at bootup  
  if(configPage2.IgInv == 1) { coilHIGH = LOW, coilLOW = HIGH; }
  else { coilHIGH = HIGH, coilLOW = LOW; }
  digitalWrite(pinCoil1, coilLOW);
  digitalWrite(pinCoil2, coilLOW);
  digitalWrite(pinCoil3, coilLOW);
  digitalWrite(pinCoil4, coilLOW);
  
  //Similar for injectors, make sure they're turned off
  digitalWrite(pinInjector1, LOW);
  digitalWrite(pinInjector2, LOW);
  digitalWrite(pinInjector3, LOW);
  digitalWrite(pinInjector4, LOW);
  
  //Set the tacho output default state
  digitalWrite(pinTachOut, HIGH);
  
  initialiseSchedulers();
  initialiseTimers();
  initialiseDisplay();
  initialiseIdle();
  initialiseFan();
  initialiseAuxPWM();
  
  //Once the configs have been loaded, a number of one time calculations can be completed
  req_fuel_uS = configPage1.reqFuel * 100; //Convert to uS and an int. This is the only variable to be used in calculations
  inj_opentime_uS = configPage1.injOpen * 100; //Injector open time. Comes through as ms*10 (Eg 15.5ms = 155). 
  
  //Begin the main crank trigger interrupt pin setup
  //The interrupt numbering is a bit odd - See here for reference: http://arduino.cc/en/Reference/AttachInterrupt
  //These assignments are based on the Arduino Mega AND VARY BETWEEN BOARDS. Please confirm the board you are using and update acordingly. 
  byte triggerInterrupt = 0; // By default, use the first interrupt
  byte triggerInterrupt2 = 1;
  currentStatus.RPM = 0;
  currentStatus.hasSync = false;
  currentStatus.runSecs = 0; 
  currentStatus.secl = 0;
  triggerFilterTime = 0; //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be disgarded as noise. This is simply a default value, the actual values are set in the setup() functinos of each decoder
  
  switch (pinTrigger) {  
    //Arduino Mega 2560 mapping
    case 2:
      triggerInterrupt = 0; break;
    case 3:
      triggerInterrupt = 1; break;
    case 18:
      triggerInterrupt = 5; break;
    case 19:
      triggerInterrupt = 4; break;
    case 20:
      triggerInterrupt = 3; break;
    case 21:
      triggerInterrupt = 2; break;
     
  }
  switch (pinTrigger2) {  
    //Arduino Mega 2560 mapping
    case 2:
      triggerInterrupt2 = 0; break;
    case 3:
      triggerInterrupt2 = 1; break;
    case 18:
      triggerInterrupt2 = 5; break;
    case 19:
      triggerInterrupt2 = 4; break;
    case 20:
      triggerInterrupt2 = 3; break;
    case 21:
      triggerInterrupt2 = 2; break;
     
  }
  pinMode(pinTrigger, INPUT);
  pinMode(pinTrigger2, INPUT);
  pinMode(pinTrigger3, INPUT);
  //digitalWrite(pinTrigger, HIGH);

  
  //Set the trigger function based on the decoder in the config
  switch (configPage2.TrigPattern)
  {
    case 0:
      //Missing tooth decoder
      triggerSetup_missingTooth();
      trigger = triggerPri_missingTooth;
      triggerSecondary = triggerSec_missingTooth;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;
      
      if(configPage2.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      break;
      
    case 1:
      // Basic distributor
      triggerSetup_BasicDistributor();
      trigger = triggerPri_BasicDistributor;
      getRPM = getRPM_BasicDistributor;
      getCrankAngle = getCrankAngle_BasicDistributor;
      
      if(configPage2.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      break;
          
    case 2:
      triggerSetup_DualWheel();
      trigger = triggerPri_DualWheel;
      getRPM = getRPM_DualWheel;
      getCrankAngle = getCrankAngle_DualWheel;
      
      if(configPage2.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      attachInterrupt(triggerInterrupt2, triggerSec_DualWheel, RISING);
      break;
      
    case 3:
      triggerSetup_GM7X();
      trigger = triggerPri_GM7X;
      getRPM = getRPM_GM7X;
      getCrankAngle = getCrankAngle_GM7X;
      
      if(configPage2.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      break;
      
    case 4:
      triggerSetup_4G63();
      trigger = triggerPri_4G63;
      getRPM = getRPM_4G63;
      getCrankAngle = getCrankAngle_4G63;
      
      //These may both need to change, not sure
      if(configPage2.TrigEdge == 0)
      {
        attachInterrupt(triggerInterrupt, trigger, CHANGE);  // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
        attachInterrupt(triggerInterrupt2, triggerSec_4G63, FALLING); //changed
      }
      else
      {
        attachInterrupt(triggerInterrupt, trigger, CHANGE); // Primary trigger connects to 
        attachInterrupt(triggerInterrupt2, triggerSec_4G63, FALLING);
      }
      
      break;
      
    case 5:
      triggerSetup_24X();
      trigger = triggerPri_24X;
      getRPM = getRPM_24X;
      getCrankAngle = getCrankAngle_24X;
      
      if(configPage2.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); } // Primary trigger connects to 
      attachInterrupt(triggerInterrupt2, triggerSec_24X, CHANGE);
      break;
      
    case 6:
      triggerSetup_Jeep2000();
      trigger = triggerPri_Jeep2000;
      getRPM = getRPM_Jeep2000;
      getCrankAngle = getCrankAngle_Jeep2000;
      
      if(configPage2.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); } // Primary trigger connects to 
      attachInterrupt(triggerInterrupt2, triggerSec_Jeep2000, CHANGE);
      break;
      
    default:
      trigger = triggerPri_missingTooth;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;
      
      if(configPage2.TrigEdge == 0) { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
      break;
  }

  //End crank triger interrupt attachment
  
  req_fuel_uS = req_fuel_uS / engineSquirtsPerCycle; //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle. If we're doing more than 1 squirt per cycle then we need to split the amount accordingly. (Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the stroke to make the single squirt on)
  
  //Initial values for loop times
  previousLoopTime = 0;
  currentLoopTime = micros();

  Serial.begin(115200);
  
  //This sets the ADC (Analog to Digitial Converter) to run at 1Mhz, greatly reducing analog read times (MAP/TPS)
  //1Mhz is the fastest speed permitted by the CPU without affecting accuracy
  //Please see chapter 11 of 'Practical Arduino' (http://books.google.com.au/books?id=HsTxON1L6D4C&printsec=frontcover#v=onepage&q&f=false) for more details
  //Can be disabled by removing the #include "fastAnalog.h" above
  #ifdef sbi
    sbi(ADCSRA,ADPS2);
    cbi(ADCSRA,ADPS1);
    cbi(ADCSRA,ADPS0);
  #endif
  
  mainLoopCount = 0;
  ignitionCount = 0;
  
  //Calculate the number of degrees between cylinders
  switch (configPage1.nCylinders) {
    case 1:
      channel1IgnDegrees = 0;
      channel1InjDegrees = 0;
      break;
    case 2:
      channel1IgnDegrees = 0;
      channel2IgnDegrees = 180;
      
      //For alternatiing injection, the squirt occurs at different times for each channel
      if(configPage1.alternate)
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 180;
      }
      else { channel1InjDegrees = channel2InjDegrees = 0; } //For simultaneous, all squirts happen at the same time
      break;
    case 3:
      channel1IgnDegrees = 0;
      channel2IgnDegrees = 120;
      channel3IgnDegrees = 240;
      
      //For alternatiing injection, the squirt occurs at different times for each channel
      if(configPage1.alternate)
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 120;
        channel3InjDegrees = 240;
      }
      else { channel1InjDegrees = channel2InjDegrees = channel3InjDegrees = 0; } //For simultaneous, all squirts happen at the same time
      break;
    case 4:
      channel1IgnDegrees = 0;
      channel2IgnDegrees = 180;

      //For alternatiing injection, the squirt occurs at different times for each channel
      if(configPage1.alternate)
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 180;
      }
      else { channel1InjDegrees = channel2InjDegrees = 0; } //For simultaneous, all squirts happen at the same time
      break;
    case 6:
      channel1IgnDegrees = 0;
      channel2IgnDegrees = 120;
      channel3IgnDegrees = 240;
      
      //For alternatiing injection, the squirt occurs at different times for each channel
      if(configPage1.alternate)
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 120;
        channel3InjDegrees = 240;
      }
      else { channel1InjDegrees = channel2InjDegrees = channel3InjDegrees = 0; } //For simultaneous, all squirts happen at the same time
      
      configPage1.injTiming = 0; //This is a failsafe. We can never run semi-sequential with more than 4 cylinders
      break;
    case 8:
      channel1IgnDegrees = 0;
      channel2IgnDegrees = 90;
      channel3IgnDegrees = 180;
      channel4IgnDegrees = 270;
      
      //For alternatiing injection, the squirt occurs at different times for each channel
      if(configPage1.alternate)
      {
        channel1InjDegrees = 0;
        channel2InjDegrees = 90;
        channel3InjDegrees = 180;
        channel4InjDegrees = 270;
      }
      else { channel1InjDegrees = channel2InjDegrees = channel3InjDegrees = channel4InjDegrees = 0; } //For simultaneous, all squirts happen at the same time
      
      configPage1.injTiming = 0; //This is a failsafe. We can never run semi-sequential with more than 4 cylinders
      break;
    default: //Handle this better!!!
      channel1InjDegrees = 0;
      channel2InjDegrees = 180;
      break;
  }
  
  switch(configPage2.sparkMode)
  {
    case 0:
      //Wasted Spark (Normal mode)
      ign1StartFunction = beginCoil1Charge;
      ign1EndFunction = endCoil1Charge;
      ign2StartFunction = beginCoil2Charge;
      ign2EndFunction = endCoil2Charge;
      ign3StartFunction = beginCoil3Charge;
      ign3EndFunction = endCoil3Charge;
      ign4StartFunction = beginCoil4Charge;
      ign4EndFunction = endCoil4Charge;
      break;
      
    case 1:
      //Single channel mode. All ignition pulses are on channel 1
      ign1StartFunction = beginCoil1Charge;
      ign1EndFunction = endCoil1Charge;
      ign2StartFunction = beginCoil1Charge;
      ign2EndFunction = endCoil1Charge;
      ign3StartFunction = beginCoil1Charge;
      ign3EndFunction = endCoil1Charge;
      ign4StartFunction = beginCoil1Charge;
      ign4EndFunction = endCoil1Charge;
      break;
      
    case 2:
      //Wasted COP mode. Ignition channels 1&3 and 2&4 are paired together
      //This is not a valid mode for >4 cylinders
      if( configPage1.nCylinders <= 4 )
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
      break;
  }
  
  //Perform the priming pulses. Set these to run at an arbitrary time in the future (100us). The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
  setFuelSchedule1(openInjector1and4, 100, (unsigned long)(configPage1.primePulse * 100), closeInjector1and4);
  setFuelSchedule2(openInjector2and3, 100, (unsigned long)(configPage1.primePulse * 100), closeInjector2and3);
}

void loop() 
{
    
      mainLoopCount++;    
      //Check for any requets from serial. Serial operations are checked under 2 scenarios:
      // 1) Every 64 loops (64 Is more than fast enough for TunerStudio). This function is equivalent to ((loopCount % 64) == 1) but is considerably faster due to not using the mod or division operations
      // 2) If the amount of data in the serial buffer is greater than a set threhold (See globals.h). This is to avoid serial buffer overflow when large amounts of data is being sent
      if ( ((mainLoopCount & 63) == 1) or (Serial.available() > SERIAL_BUFFER_THRESHOLD) ) 
      {
        if (Serial.available() > 0) 
        {
          command();
        }
      }

      // if (configPage1.displayType && (mainLoopCount & 255) == 1) { updateDisplay();} //Timers currently disabled
     
    //Calculate the RPM based on the uS between the last 2 times tooth One was seen.
    previousLoopTime = currentLoopTime;
    currentLoopTime = micros();
    long timeToLastTooth = (currentLoopTime - toothLastToothTime);
    if ( (timeToLastTooth < 500000L) || (toothLastToothTime > currentLoopTime) ) //Check how long ago the last tooth was seen compared to now. If it was more than half a second ago then the engine is probably stopped. toothLastToothTime can be greater than currentLoopTime if a pulse occurs between getting the lastest time and doing the comparison
    {
      int lastRPM = currentStatus.RPM; //Need to record this for rpmDOT calculation
      currentStatus.RPM = getRPM();
      if(fuelPumpOn == false) { digitalWrite(pinFuelPump, HIGH); fuelPumpOn = true; } //Check if the fuel pump is on and turn it on if it isn't. 
      currentStatus.rpmDOT = ldiv(1000000, (currentLoopTime - previousLoopTime)).quot * (currentStatus.RPM - lastRPM); //This is the RPM per second that the engine has accelerated/decelleratedin the last loop
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      currentStatus.RPM = 0; 
      currentStatus.PW = 0;
      currentStatus.VE = 0;
      currentStatus.hasSync = false;
      currentStatus.runSecs = 0; //Reset the counter for number of seconds running.
      secCounter = 0; //Reset our seconds counter.
      startRevolutions = 0;
      currentStatus.rpmDOT = 0;
      ignitionOn = false;
      fuelOn = false;
      digitalWrite(pinFuelPump, LOW); //Turn off the fuel pump
      fuelPumpOn = false;
    }
    
    //Uncomment the following for testing
    /*
    currentStatus.hasSync = true;
    currentStatus.RPM = 500;
    */
     
    //***SET STATUSES***
    //-----------------------------------------------------------------------------------------------------

    //MAP Sampling system
    switch(configPage1.mapSample)
    {
      case 0:
        //Instantaneous MAP readings
        currentStatus.mapADC = analogRead(pinMAP);
        currentStatus.MAP = map(currentStatus.mapADC, 0, 1023, configPage1.mapMin, configPage1.mapMax); //Get the current MAP value
        break;
        
      case 1:
        //Average of a cycle
        if( (MAPcurRev == startRevolutions) || (MAPcurRev == startRevolutions+1) ) //2 revolutions are looked at for 4 stroke. 2 stroke not currently catered for. 
        {
          MAPrunningValue = MAPrunningValue + analogRead(pinMAP); //Add the current reading onto the total
          MAPcount++;
        }
        else
        {
          //Reaching here means that the last cylce has completed and the MAP value should be calculated
          currentStatus.mapADC = ldiv(MAPrunningValue, MAPcount).quot;
          currentStatus.MAP = map(currentStatus.mapADC, 0, 1023, configPage1.mapMin, configPage1.mapMax); //Get the current MAP value
          MAPcurRev = startRevolutions; //Reset the current rev count
          MAPrunningValue = 0;
          MAPcount = 0;
        }
        break;
      
      case 2:
        //Minimum reading in a cycle
        if( (MAPcurRev == startRevolutions) || (MAPcurRev == startRevolutions+1) ) //2 revolutions are looked at for 4 stroke. 2 stroke not currently catered for. 
        {
          int tempValue = analogRead(pinMAP);
          if( tempValue < MAPrunningValue) { MAPrunningValue = tempValue; } //Check whether the current reading is lower than the running minimum
          MAPcount++;
        }
        else
        {
          //Reaching here means that the last cylce has completed and the MAP value should be calculated
          currentStatus.mapADC = MAPrunningValue;
          currentStatus.MAP = map(currentStatus.mapADC, 0, 1023, configPage1.mapMin, configPage1.mapMax); //Get the current MAP value
          MAPcurRev = startRevolutions; //Reset the current rev count
          MAPrunningValue = 1023; //Reset the latest value so the next reading will always be lower
        }
        break;
    }
    
    //TPS setting to be performed every 32 loops (any faster and it can upset the TPSdot sampling time)
    if ((mainLoopCount & 31) == 1)
    {
      currentStatus.TPSlast = currentStatus.TPS;
      currentStatus.TPSlast_time = currentStatus.TPS_time;
      currentStatus.tpsADC = fastMap1023toX(analogRead(pinTPS), 0, 1023, 0, 255); //Get the current raw TPS ADC value and map it into a byte
      currentStatus.TPS = map(currentStatus.tpsADC, configPage1.tpsMin, configPage1.tpsMax, 0, 100); //Take the raw TPS ADC value and convert it into a TPS% based on the calibrated values
      currentStatus.TPS_time = currentLoopTime;  
     
      //Check for launch in (clutch) can be done around here too
      currentStatus.launching = !digitalRead(pinLaunch);
      //And check whether the tooth log buffer is ready
      if(toothHistoryIndex > TOOTH_LOG_SIZE) { BIT_SET(currentStatus.squirt, BIT_SQUIRT_TOOTHLOG1READY); }
    }
    
    //The IAT and CLT readings can be done less frequently. This still runs about 4 times per second
    if ((mainLoopCount & 255) == 1)
    {
       //currentStatus.cltADC = map(analogRead(pinCLT), 0, 1023, 0, 511); //Get the current raw CLT value
       currentStatus.cltADC = fastMap1023toX(analogRead(pinCLT), 0, 1023, 0, 511); //Get the current raw CLT value
       currentStatus.iatADC = map(analogRead(pinIAT), 0, 1023, 0, 511); //Get the current raw IAT value
       currentStatus.O2ADC = map(analogRead(pinO2), 0, 1023, 0, 511); //Get the current O2 value. Calibration is from AFR values 7.35 to 22.4. This is the correct calibration for an Innovate Wideband 0v - 5V unit. Proper calibration is still a WIP
       currentStatus.O2_2ADC = map(analogRead(pinO2_2), 0, 1023, 0, 511); //Get the current O2 value. Calibration is from AFR values 7.35 to 22.4. This is the correct calibration for an Innovate Wideband 0v - 5V unit. Proper calibration is still a WIP
       //currentStatus.battery10 = map(analogRead(pinBat), 0, 1023, 0, 245); //Get the current raw Battery value. Permissible values are from 0v to 24.5v (245)
       currentStatus.battery10 = fastMap1023toX(analogRead(pinBat), 0, 1023, 0, 245); //Get the current raw Battery value. Permissible values are from 0v to 24.5v (245)
       //currentStatus.batADC = map(analogRead(pinBat), 0, 1023, 0, 255); //Get the current raw Battery value
       
       currentStatus.coolant = cltCalibrationTable[currentStatus.cltADC] - CALIBRATION_TEMPERATURE_OFFSET; //Temperature calibration values are stored as positive bytes. We subtract 40 from them to allow for negative temperatures
       currentStatus.IAT = iatCalibrationTable[currentStatus.iatADC] - CALIBRATION_TEMPERATURE_OFFSET;
       currentStatus.O2 = o2CalibrationTable[currentStatus.O2ADC];
       currentStatus.O2_2 = o2CalibrationTable[currentStatus.O2_2ADC];
       
      vvtControl();
      boostControl(); //Most boost tends to run at about 30Hz, so placing it here ensures a new target time is fetched frequently enough
    }

    //Always check for sync
    //Main loop runs within this clause
    if (currentStatus.hasSync && (currentStatus.RPM > 0))
    {
        //If it is, check is we're running or cranking
        if(currentStatus.RPM > ((unsigned int)configPage2.crankRPM * 100)) //Crank RPM stored in byte as RPM / 100 
        { //Sets the engine running bit, clears the engine cranking bit
          BIT_SET(currentStatus.engine, BIT_ENGINE_RUN); 
          BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
          if(startRevolutions >= configPage2.StgCycles) { ignitionOn = true; fuelOn = true;}
        } 
        else 
        {  //Sets the engine cranking bit, clears the engine running bit
          BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK); 
          BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN); 
          currentStatus.runSecs = 0; //We're cranking (hopefully), so reset the engine run time to prompt ASE.
          //Check whether enough cranking revolutions have been performed to turn the ignition on
          if(startRevolutions >= configPage2.StgCycles)  { ignitionOn = true; fuelOn = true;}
        } 
      
      idleControl(); //Perform any idle realted actions
      //END SETTING STATUSES
      //-----------------------------------------------------------------------------------------------------
      
      //Begin the fuel calculation
      //Calculate an injector pulsewidth from the VE
      currentStatus.corrections = correctionsTotal();
      //currentStatus.corrections = 100;
      if (configPage1.algorithm == 0) //Check which fuelling algorithm is being used
      { 
        //Speed Density
        currentStatus.VE = get3DTableValue(&fuelTable, currentStatus.MAP, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
        currentStatus.PW = PW_SD(req_fuel_uS, currentStatus.VE, currentStatus.MAP, currentStatus.corrections, inj_opentime_uS);
        currentStatus.advance = get3DTableValue(&ignitionTable, currentStatus.MAP, currentStatus.RPM); //As above, but for ignition advance
      }
      else
      { 
        //Alpha-N
        currentStatus.VE = get3DTableValue(&fuelTable, currentStatus.TPS, currentStatus.RPM); //Perform lookup into fuel map for RPM vs TPS value
        currentStatus.PW = PW_AN(req_fuel_uS, currentStatus.VE, currentStatus.TPS, currentStatus.corrections, inj_opentime_uS); //Calculate pulsewidth using the Alpha-N algorithm (in uS)
        currentStatus.advance = get3DTableValue(&ignitionTable, currentStatus.TPS, currentStatus.RPM); //As above, but for ignition advance
      }
      
      //Check for fixed ignition angles
      if (configPage2.FixAng != 0) { currentStatus.advance = configPage2.FixAng; } //Check whether the user has set a fixed timing angle
      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { currentStatus.advance = configPage2.CrankAng; } //Use the fixed cranking ignition angle

      int injector1StartAngle = 0;
      int injector2StartAngle = 0;
      int injector3StartAngle = 0; //Currently used for 3 cylinder only
      int injector4StartAngle = 0; //Not used until sequential gets written
      int ignition1StartAngle = 0;
      int ignition2StartAngle = 0;
      int ignition3StartAngle = 0; //Not used until sequential or 4+ cylinders support gets written
      int ignition4StartAngle = 0; //Not used until sequential or 4+ cylinders support gets written
      //These are used for comparisons on channels above 1 where the starting angle (for injectors or ignition) can be less than a single loop time
      //(Don't ask why this is needed, it will break your head)
      int tempCrankAngle;
      int tempStartAngle; 
      
      //How fast are we going? Need to know how long (uS) it will take to get from one tooth to the next. We then use that to estimate how far we are between the last tooth and the next one
      //We use a 1st Deriv accleration prediction, but only when there is an even spacing between primary sensor teeth
      //Any decoder that has uneven spacing has its triggerToothAngle set to 0
      if(triggerToothAngle > 0)
      {
        long toothAccel = toothDeltaV / triggerToothAngle; //An amount represengint the current acceleration or decceleration of the crank in degrees per uS per uS
        timePerDegree = ldiv( 166666L, currentStatus.RPM ).quot + (toothAccel * (micros() - toothLastToothTime)); //There is a small amount of rounding in this calculation, however it is less than 0.001 of a uS (Faster as ldiv than / )
      }
      else
      {
        timePerDegree = ldiv( 166666L, currentStatus.RPM ).quot; //There is a small amount of rounding in this calculation, however it is less than 0.001 of a uS (Faster as ldiv than / )
      }
      
      //Check that the duty cycle of the chosen pulsewidth isn't too high. This is disabled at cranking
      if( !BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {
        unsigned long pwLimit = percentage(configPage1.dutyLim, revolutionTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
        if (currentStatus.PW > pwLimit) { currentStatus.PW = pwLimit; }
      }
      
      
      //***********************************************************************************************
      //BEGIN INJECTION TIMING
      //Determine next firing angles
      int PWdivTimerPerDegree = div(currentStatus.PW, timePerDegree).quot; //How many crank degrees the calculated PW will take at the current speed
      injector1StartAngle = configPage1.inj1Ang - ( PWdivTimerPerDegree ); //This is a little primitive, but is based on the idea that all fuel needs to be delivered before the inlet valve opens. See http://www.extraefi.co.uk/sequential_fuel.html for more detail
      if(injector1StartAngle < 0) {injector1StartAngle += 360;} 
      //Repeat the above for each cylinder
      switch (configPage1.nCylinders)
      {
        //2 cylinders
        case 2:
          injector2StartAngle = (configPage1.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;}
          break;
        //3 cylinders
        case 3:
          injector2StartAngle = (configPage1.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;} 
          injector3StartAngle = (configPage1.inj3Ang + channel3InjDegrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > 360) {injector3StartAngle -= 360;}
          break;
        //4 cylinders
        case 4:
          injector2StartAngle = (configPage1.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;}
          break;
        //6 cylinders
        case 6: 
          injector2StartAngle = (configPage1.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;} 
          injector3StartAngle = (configPage1.inj3Ang + channel3InjDegrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > 360) {injector3StartAngle -= 360;}
          break;
        //8 cylinders
        case 8: 
          injector2StartAngle = (configPage1.inj2Ang + channel2InjDegrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;} 
          injector3StartAngle = (configPage1.inj3Ang + channel3InjDegrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > 360) {injector3StartAngle -= 360;}
          injector4StartAngle = (configPage1.inj4Ang + channel4InjDegrees - ( PWdivTimerPerDegree ));
          if(injector4StartAngle > 360) {injector4StartAngle -= 360;}
          break;
        //Will hit the default case on 1 cylinder or >8 cylinders. Do nothing in these cases
        default:
          break;
      }
    
      //***********************************************************************************************
      //| BEGIN IGNITION CALCULATIONS
      if (currentStatus.RPM > ((unsigned int)(configPage2.SoftRevLim) * 100) ) { currentStatus.advance = configPage2.SoftLimRetard; } //Softcut RPM limit (If we're above softcut limit, delay timing by configured number of degrees)
      if (configPage3.launchEnabled && currentStatus.launching && (currentStatus.RPM > ((unsigned int)(configPage3.lnchSoftLim) * 100)) ) { currentStatus.advance = configPage3.lnchRetard; } //SoftCut rev limit for 2-step launch control
      
      //Set dwell
       //Dwell is stored as ms * 10. ie Dwell of 4.3ms would be 43 in configPage2. This number therefore needs to be multiplied by 100 to get dwell in uS
      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { currentStatus.dwell =  (configPage2.dwellCrank * 100); }
      else { currentStatus.dwell =  (configPage2.dwellRun * 100); }
      //Pull battery voltage based dwell correction and apply if needed
      currentStatus.dwellCorrection = table2D_getValue(&dwellVCorrectionTable, currentStatus.battery10);
      if (currentStatus.dwellCorrection != 100) { currentStatus.dwell = divs100(currentStatus.dwell) * currentStatus.dwellCorrection; }
      int dwellAngle = (div(currentStatus.dwell, timePerDegree).quot ); //Convert the dwell time to dwell angle based on the current engine speed
      
      //Calculate start angle for each channel
      //1 cylinder (Everyone gets this)
      ignition1StartAngle = 360 - currentStatus.advance - dwellAngle; // 360 - desired advance angle - number of degrees the dwell will take
      if(ignition1StartAngle < 0) {ignition1StartAngle += 360;} 
      
      //This test for more cylinders and do the same thing
      switch (configPage1.nCylinders)
      {
        //2 cylinders
        case 2:
          ignition2StartAngle = channel2IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;}
          break;
        //3 cylinders
        case 3:
          ignition2StartAngle = channel2IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;} 
          ignition3StartAngle = channel3IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition3StartAngle > 360) {ignition3StartAngle -= 360;}
          break;
        //4 cylinders
        case 4:
          ignition2StartAngle = channel2IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;}
          if(ignition2StartAngle < 0) {ignition2StartAngle += 360;}
          break;
        //6 cylinders
        case 6:
          ignition2StartAngle = channel2IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;} 
          ignition3StartAngle = channel3IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition3StartAngle > 360) {ignition3StartAngle -= 360;}
          break;
        //8 cylinders
        case 8:
          ignition2StartAngle = channel2IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;} 
          ignition3StartAngle = channel3IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition3StartAngle > 360) {ignition3StartAngle -= 360;}
          ignition4StartAngle = channel4IgnDegrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition4StartAngle > 360) {ignition4StartAngle -= 360;}
          break;
          
        //Will hit the default case on 1 cylinder or >8 cylinders. Do nothing in these cases
        default:
          break;
      }
      
      //***********************************************************************************************
      //| BEGIN FUEL SCHEDULES
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the shcedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time
      
      //Determine the current crank angle
      int crankAngle = getCrankAngle(timePerDegree);
      
      if (fuelOn)
      {
        if (injector1StartAngle > crankAngle)
        { 
          if (configPage1.injTiming == 1)
          {
            setFuelSchedule1(openInjector1and4, 
                      ((unsigned long)(injector1StartAngle - crankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW,
                      closeInjector1and4
                      );
          }
          else
          {
            setFuelSchedule1(openInjector1, 
                      ((unsigned long)(injector1StartAngle - crankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW,
                      closeInjector1
                      );
          }
        }
        
        /*-----------------------------------------------------------------------------------------
        | A Note on tempCrankAngle and tempStartAngle:
        |   The use of tempCrankAngle/tempStartAngle is described below. It is then used in the same way for channels 2, 3 and 4 on both injectors and ignition
        |   Essentially, these 2 variables are used to realign the current crank and and the desired start angle around 0 degrees for the given cylinder/output
        |   Eg: If cylinder 2 TDC is 180 degrees after cylinder 1 (Eg a standard 4 cylidner engine), then tempCrankAngle is 180* less than the current crank angle and
        |       tempStartAngle is the desired open time less 180*. Thus the cylinder is being treated relative to its own TDC, regardless of its offset
        |
        |   This is done to avoid problems with very short of very long times until tempStartAngle. 
        |   This will very likely need to be rewritten when sequential is enabled
        |------------------------------------------------------------------------------------------
        */
        tempCrankAngle = crankAngle - channel2InjDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += 360; }
        tempStartAngle = injector2StartAngle - channel2InjDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += 360; }
        if (tempStartAngle > tempCrankAngle)
        { 
          if (configPage1.injTiming == 1)
          {
            setFuelSchedule2(openInjector2and3, 
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW,
                      closeInjector2and3
                      );
          }
          else
          {
            setFuelSchedule2(openInjector2, 
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW,
                      closeInjector2
                      );
          }
        }
        
        tempCrankAngle = crankAngle - channel3InjDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += 360; }
        tempStartAngle = injector3StartAngle - channel3InjDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += 360; }
        if (tempStartAngle > tempCrankAngle)
        { 
          setFuelSchedule3(openInjector3, 
                    ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                    (unsigned long)currentStatus.PW,
                    closeInjector3
                    );
        }
        
        tempCrankAngle = crankAngle - channel4InjDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += 360; }
        tempStartAngle = injector4StartAngle - channel4InjDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += 360; }
        if (tempStartAngle > tempCrankAngle)
        { 
          setFuelSchedule4(openInjector4, 
                    ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                    (unsigned long)currentStatus.PW,
                    closeInjector4
                    );
        }
      }
      //***********************************************************************************************
      //| BEGIN IGNITION SCHEDULES
      //Likewise for the ignition
      //Perform an initial check to see if the ignition is turned on (Ignition only turns on after a preset number of cranking revolutions and:
      //Check for hard cut rev limit (If we're above the hardcut limit, we simply don't set a spark schedule)
      crankAngle = getCrankAngle(timePerDegree); //Refresh with the latest crank angle
      if(ignitionOn && (currentStatus.RPM < ((unsigned int)(configPage2.HardRevLim) * 100) ))
      {
        if ( (ignition1StartAngle > crankAngle) && ign1LastRev != startRevolutions)
        //if (ign1LastRev != startRevolutions)
        {
            unsigned long ignition1StartTime;
            if(ignition1StartAngle > crankAngle) { ignition1StartTime = ((unsigned long)(ignition1StartAngle - crankAngle) * (unsigned long)timePerDegree); }
            else { ignition1StartTime = ((unsigned long)(360 - crankAngle + ignition1StartAngle) * (unsigned long)timePerDegree); }
            
            setIgnitionSchedule1(ign1StartFunction, 
                      ((unsigned long)(ignition1StartAngle - crankAngle) * (unsigned long)timePerDegree),
                      currentStatus.dwell,
                      ign1EndFunction
                      );
        }

        tempCrankAngle = crankAngle - channel2IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += 360; }
        tempStartAngle = ignition2StartAngle - channel2IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += 360; }
        if ( (tempStartAngle > tempCrankAngle)  && ign2LastRev != startRevolutions)
        //if ( ign2LastRev != startRevolutions )
        { 
            unsigned long ignition2StartTime;
            if(tempStartAngle > tempCrankAngle) { ignition2StartTime = ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree); }
            else { ignition2StartTime = ((unsigned long)(360 - tempCrankAngle + tempStartAngle) * (unsigned long)timePerDegree); }
            
            setIgnitionSchedule2(ign2StartFunction, 
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      currentStatus.dwell,
                      ign2EndFunction
                      );
        }
        
        tempCrankAngle = crankAngle - channel3IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += 360; }
        tempStartAngle = ignition3StartAngle - channel3IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += 360; }
        if (tempStartAngle > tempCrankAngle)
        { 
            setIgnitionSchedule3(ign3StartFunction, 
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      currentStatus.dwell,
                      ign3EndFunction
                      );
        }
        
        tempCrankAngle = crankAngle - channel4IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += 360; }
        tempStartAngle = ignition4StartAngle - channel4IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += 360; }
        if (tempStartAngle > tempCrankAngle)
        { 
            setIgnitionSchedule4(ign4StartFunction, 
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      currentStatus.dwell,
                      ign4EndFunction
                      );
        }
        
      }
      
    }
    
  }
  
//************************************************************************************************
//Interrupts  

//These functions simply trigger the injector/coil driver off or on. 
//NOTE: squirt status is changed as per http://www.msextra.com/doc/ms1extra/COM_RS232.htm#Acmd
void openInjector1() { digitalWrite(pinInjector1, HIGH); BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ1); } 
void closeInjector1() { digitalWrite(pinInjector1, LOW); BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ1); } 
//void openInjector1() { *inj1_pin_port |= (inj1_pin_mask); ; BIT_SET(currentStatus.squirt, 0); } 
//void closeInjector1() { *inj1_pin_port &= ~(inj1_pin_mask);  BIT_CLEAR(currentStatus.squirt, 0); }
void beginCoil1Charge() { digitalWrite(pinCoil1, coilHIGH); BIT_SET(currentStatus.spark, 0); digitalWrite(pinTachOut, LOW); }
void endCoil1Charge() { digitalWrite(pinCoil1, coilLOW); BIT_CLEAR(currentStatus.spark, 0); }
//void beginCoil1Charge() { *ign1_pin_port |= (ign1_pin_mask); }
//void endCoil1Charge() { *ign1_pin_port &= ~(ign1_pin_mask); }

void openInjector2() { digitalWrite(pinInjector2, HIGH); BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ2); } //Sets the relevant pin HIGH and changes the current status bit for injector 2 (2nd bit of currentStatus.squirt)
void closeInjector2() { digitalWrite(pinInjector2, LOW); BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ2); } 
//void openInjector2() { *inj2_pin_port |= (inj2_pin_mask); ; BIT_SET(currentStatus.squirt, 0); } 
//void closeInjector2() { *inj2_pin_port &= ~(inj2_pin_mask);  BIT_CLEAR(currentStatus.squirt, 0); }
void beginCoil2Charge() { digitalWrite(pinCoil2, coilHIGH); BIT_SET(currentStatus.spark, 1); digitalWrite(pinTachOut, LOW); }
void endCoil2Charge() { digitalWrite(pinCoil2, coilLOW); BIT_CLEAR(currentStatus.spark, 1);}

void openInjector3() { digitalWrite(pinInjector3, HIGH); BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ3); } //Sets the relevant pin HIGH and changes the current status bit for injector 3 (3rd bit of currentStatus.squirt)
void closeInjector3() { digitalWrite(pinInjector3, LOW); BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ3); } 
void beginCoil3Charge() { digitalWrite(pinCoil3, coilHIGH); BIT_SET(currentStatus.spark, 2); digitalWrite(pinTachOut, LOW); }
void endCoil3Charge() { digitalWrite(pinCoil3, coilLOW); BIT_CLEAR(currentStatus.spark, 2); }

void openInjector4() { digitalWrite(pinInjector4, HIGH); BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ4); } //Sets the relevant pin HIGH and changes the current status bit for injector 4 (4th bit of currentStatus.squirt)
void closeInjector4() { digitalWrite(pinInjector4, LOW); BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ4); } 
void beginCoil4Charge() { digitalWrite(pinCoil4, coilHIGH); BIT_SET(currentStatus.spark, 3); digitalWrite(pinTachOut, LOW); }
void endCoil4Charge() { digitalWrite(pinCoil4, coilLOW); BIT_CLEAR(currentStatus.spark, 3); }

//Combination functions for semi-sequential injection
void openInjector1and4() { digitalWrite(pinInjector1, HIGH); digitalWrite(pinInjector4, HIGH); BIT_SET(currentStatus.squirt, 0); } 
void closeInjector1and4() { digitalWrite(pinInjector1, LOW); digitalWrite(pinInjector4, LOW);BIT_CLEAR(currentStatus.squirt, 0); }
void openInjector2and3() { digitalWrite(pinInjector2, HIGH); digitalWrite(pinInjector3, HIGH); BIT_SET(currentStatus.squirt, 1); }
void closeInjector2and3() { digitalWrite(pinInjector2, LOW); digitalWrite(pinInjector3, LOW); BIT_CLEAR(currentStatus.squirt, 1); } 

//As above but for ignition (Wasted COP mode)
void beginCoil1and3Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil3, coilHIGH); BIT_SET(currentStatus.spark, 0); BIT_SET(currentStatus.spark, 2); digitalWrite(pinTachOut, LOW); }
void endCoil1and3Charge() { digitalWrite(pinCoil1, coilLOW); digitalWrite(pinCoil3, coilLOW); BIT_CLEAR(currentStatus.spark, 0); BIT_CLEAR(currentStatus.spark, 2); }
void beginCoil2and4Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil4, coilHIGH); BIT_SET(currentStatus.spark, 1); BIT_SET(currentStatus.spark, 3); digitalWrite(pinTachOut, LOW); }
void endCoil2and4Charge() { digitalWrite(pinCoil2, coilLOW); digitalWrite(pinCoil4, coilLOW); BIT_CLEAR(currentStatus.spark, 1); BIT_CLEAR(currentStatus.spark, 3); }

void nullCallback() { return; }
  

