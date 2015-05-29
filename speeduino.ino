
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

#ifdef __SAM3X8E__
 //Do stuff for ARM based CPUs 
#else
  #include "storage.h"
#endif

#include "fastAnalog.h"
#define DIGITALIO_NO_MIX_ANALOGWRITE
#include <digitalIOPerformance.h>
#include <PID_v1.h>

struct config1 configPage1;
struct config2 configPage2;
struct config3 configPage3;
struct config4 configPage4;

int req_fuel_uS, inj_opentime_uS;
#define MAX_RPM 10000 //This is the maximum rpm that the ECU will attempt to run at. It is NOT related to the rev limiter, but is instead dictates how fast certain operations will be allowed to run. Lower number gives better performance

volatile byte startRevolutions = 0; //A counter for how many revolutions have been completed since sync was achieved.
volatile bool ignitionOn = true; //The current state of the ignition system

void (*trigger)(); //Pointer for the trigger function (Gets pointed to the relevant decoder)
void (*triggerSecondary)(); //Pointer for the secondary trigger function (Gets pointed to the relevant decoder)
int (*getRPM)(); //Pointer to the getRPM function (Gets pointed to the relevant decoder)
int (*getCrankAngle)(int); //Pointer to the getCrank Angle function (Gets pointed to the relevant decoder)

struct table3D fuelTable; //8x8 fuel map
struct table3D ignitionTable; //8x8 ignition map
struct table3D afrTable; //8x8 afr target map
struct table2D taeTable; //4 bin TPS Acceleration Enrichment map (2D)
struct table2D WUETable; //10 bin Warm Up Enrichment map (2D)
struct table2D dwellVCorrectionTable; //6 bin dwell voltage correction (2D)
struct table2D injectorVCorrectionTable; //6 bin injector voltage correction (2D)
byte cltCalibrationTable[CALIBRATION_TABLE_SIZE];
byte iatCalibrationTable[CALIBRATION_TABLE_SIZE];
byte o2CalibrationTable[CALIBRATION_TABLE_SIZE];

unsigned long counter;
unsigned long currentLoopTime; //The time the current loop started (uS)
unsigned long previousLoopTime; //The time the previous loop started (uS)
unsigned long scheduleStart;
unsigned long scheduleEnd;

byte coilHIGH = HIGH;
byte coilLOW = LOW;

struct statuses currentStatus;
volatile int mainLoopCount;
byte ignitionCount;
unsigned long secCounter; //The next time to increment 'runSecs' counter.
int channel1Degrees; //The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones)
int channel2Degrees; //The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC
int channel3Degrees; //The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC
int channel4Degrees; //The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC
int timePerDegree;
byte degreesPerLoop; //The number of crank degrees that pass for each mainloop of the program

void setup() 
{
  //Setup the dummy fuel and ignition tables
  //dummyFuelTable(&fuelTable);
  //dummyIgnitionTable(&ignitionTable);
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
  
  //Once the configs have been loaded, a number of one time calculations can be completed
  req_fuel_uS = configPage1.reqFuel * 100; //Convert to uS and an int. This is the only variable to be used in calculations
  inj_opentime_uS = configPage1.injOpen * 100; //Injector open time. Comes through as ms*10 (Eg 15.5ms = 155). 
  
  //Begin the main crank trigger interrupt pin setup
  //The interrupt numbering is a bit odd - See here for reference: http://arduino.cc/en/Reference/AttachInterrupt
  //These assignments are based on the Arduino Mega AND VARY BETWEEN BOARDS. Please confirm the board you are using and update acordingly. 
  int triggerInterrupt = 0; // By default, use the first interrupt
  currentStatus.RPM = 0;
  currentStatus.hasSync = false;
  currentStatus.runSecs = 0; 
  currentStatus.secl = 0;
  triggerFilterTime = (int)(1000000 / (MAX_RPM / 60 * configPage2.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be disgarded as noise
  
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
  pinMode(pinTrigger, INPUT);
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
      break;
      
    case 1:
      // Basic distributor
      triggerSetup_BasicDistributor();
      trigger = triggerPri_BasicDistributor;
      getRPM = getRPM_BasicDistributor;
      getCrankAngle = getCrankAngle_BasicDistributor;
      break;
          
    case 2:
      trigger = triggerPri_DualWheel;
      break;
      
    default:
      trigger = triggerPri_missingTooth;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;
      break;
  }
  if(configPage2.TrigEdge == 0)
    { attachInterrupt(triggerInterrupt, trigger, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
    else { attachInterrupt(triggerInterrupt, trigger, FALLING); }
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
      channel1Degrees = 0;
      break;
    case 2:
      channel1Degrees = 0;
      channel2Degrees = 180;
      break;
    case 3:
      channel1Degrees = 0;
      channel2Degrees = 120;
      channel3Degrees = 240;
      break;
    case 4:
      channel1Degrees = 0;
      channel2Degrees = 180;
      break;
    case 6:
      channel1Degrees = 0;
      channel2Degrees = 120;
      channel3Degrees = 240;
      break;
    case 8:
      channel1Degrees = 0;
      channel2Degrees = 90;
      channel3Degrees = 180;
      channel4Degrees = 270;
      break;
    default: //Handle this better!!!
      channel1Degrees = 0;
      channel2Degrees = 180;
      break;
  }
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
      
      if (configPage1.displayType && (mainLoopCount & 255) == 1) { updateDisplay();}
     
    //Calculate the RPM based on the uS between the last 2 times tooth One was seen.
    previousLoopTime = currentLoopTime;
    currentLoopTime = micros();
    long timeToLastTooth = (currentLoopTime - toothLastToothTime);
    if ( (timeToLastTooth < 500000L) || (toothLastToothTime > currentLoopTime) ) //Check how long ago the last tooth was seen compared to now. If it was more than half a second ago then the engine is probably stopped. toothLastToothTime can be greater than currentLoopTime if a pulse occurs between getting the lastest time and doing the comparison
    {
      currentStatus.RPM = getRPM();
      if(digitalRead(pinFuelPump) == LOW) { digitalWrite(pinFuelPump, HIGH); } //Check if the fuel pump is on and turn it on if it isn't. 
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
      digitalWrite(pinFuelPump, LOW); //Turn off the fuel pump
    }
    
    //Uncomment the following for testing
    /*
    currentStatus.hasSync = true;
    currentStatus.RPM = 500;
    */
     
    //***SET STATUSES***
    //-----------------------------------------------------------------------------------------------------

    //currentStatus.MAP = map(analogRead(pinMAP), 0, 1023, 10, 255); //Get the current MAP value
    currentStatus.MAP = fastMap1023toX(analogRead(pinMAP), 0, 1023, 10, 255); //Get the current MAP value
    
    //TPS setting to be performed every 32 loops (any faster and it can upset the TPSdot sampling time)
    if ((mainLoopCount & 31) == 1)
    {
      currentStatus.TPSlast = currentStatus.TPS;
      currentStatus.TPSlast_time = currentStatus.TPS_time;
      currentStatus.tpsADC = fastMap1023toX(analogRead(pinTPS), 0, 1023, 0, 255); //Get the current raw TPS ADC value and map it into a byte
      currentStatus.TPS = map(currentStatus.tpsADC, configPage1.tpsMin, configPage1.tpsMax, 0, 100); //Take the raw TPS ADC value and convert it into a TPS% based on the calibrated values
      currentStatus.TPS_time = currentLoopTime;
    }
    
    //The IAT and CLT readings can be done less frequently. This still runs about 4 times per second
    if ((mainLoopCount & 255) == 1)
    {
       currentStatus.cltADC = map(analogRead(pinCLT), 0, 1023, 0, 511); //Get the current raw CLT value
       currentStatus.iatADC = map(analogRead(pinIAT), 0, 1023, 0, 511); //Get the current raw IAT value
       currentStatus.O2ADC = map(analogRead(pinO2), 0, 1023, 0, 511); //Get the current O2 value. Calibration is from AFR values 7.35 to 22.4. This is the correct calibration for an Innovate Wideband 0v - 5V unit. Proper calibration is still a WIP
       currentStatus.battery10 = map(analogRead(pinBat), 0, 1023, 0, 245); //Get the current raw Battery value. Permissible values are from 0v to 24.5v (245)
       //currentStatus.batADC = map(analogRead(pinBat), 0, 1023, 0, 255); //Get the current raw Battery value
       
       currentStatus.coolant = cltCalibrationTable[currentStatus.cltADC] - CALIBRATION_TEMPERATURE_OFFSET; //Temperature calibration values are stored as positive bytes. We subtract 40 from them to allow for negative temperatures
       currentStatus.IAT = iatCalibrationTable[currentStatus.iatADC] - CALIBRATION_TEMPERATURE_OFFSET;
       currentStatus.O2 = o2CalibrationTable[currentStatus.O2ADC];
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
        } 
        else 
        {  //Sets the engine cranking bit, clears the engine running bit
          BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK); 
          BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN); 
          currentStatus.runSecs = 0; //We're cranking (hopefully), so reset the engine run time to prompt ASE.
          //Check whether enough cranking revolutions have been performed to turn the ignition on
          if(startRevolutions > configPage2.StgCycles)
          {ignitionOn = true;}
        } 
      
      //END SETTING STATUSES
      //-----------------------------------------------------------------------------------------------------
      
      //Begin the fuel calculation
      //Calculate an injector pulsewidth from the VE
      currentStatus.corrections = correctionsTotal();
      //currentStatus.corrections = 100;
      if (configPage1.algorithm == 0) //Check with fuelling algorithm is being used
      { 
        //Speed Density
        currentStatus.VE = get3DTableValue(fuelTable, currentStatus.MAP, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
        currentStatus.PW = PW_SD(req_fuel_uS, currentStatus.VE, currentStatus.MAP, currentStatus.corrections, inj_opentime_uS);
        if (configPage2.FixAng == 0) //Check whether the user has set a fixed timing angle
          { currentStatus.advance = get3DTableValue(ignitionTable, currentStatus.MAP, currentStatus.RPM); } //As above, but for ignition advance
         else
          { currentStatus.advance = configPage2.FixAng; }
      }
      else
      { 
        //Alpha-N
        currentStatus.VE = get3DTableValue(fuelTable, currentStatus.TPS, currentStatus.RPM); //Perform lookup into fuel map for RPM vs TPS value
        currentStatus.PW = PW_AN(req_fuel_uS, currentStatus.VE, currentStatus.TPS, currentStatus.corrections, inj_opentime_uS); //Calculate pulsewidth using the Alpha-N algorithm (in uS)
        if (configPage2.FixAng == 0) //Check whether the user has set a fixed timing angle
          { currentStatus.advance = get3DTableValue(ignitionTable, currentStatus.TPS, currentStatus.RPM); } //As above, but for ignition advance
        else
          { currentStatus.advance = configPage2.FixAng; }
      }

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
      timePerDegree = ldiv( 166666L, currentStatus.RPM ).quot; //There is a small amount of rounding in this calculation, however it is less than 0.001 of a uS
      
      //Determine the current crank angle
      int crankAngle = getCrankAngle(timePerDegree);
      
      //***********************************************************************************************
      //BEGIN INJECTION TIMING
      //Determine next firing angles
      //1
      int PWdivTimerPerDegree = div(currentStatus.PW, timePerDegree).quot; //How many crank degrees the calculated PW will take at the current speed
      injector1StartAngle = configPage1.inj1Ang - ( PWdivTimerPerDegree ); //This is a little primitive, but is based on the idea that all fuel needs to be delivered before the inlet valve opens. I am using 355 as the point at which the injector MUST be closed by. See http://www.extraefi.co.uk/sequential_fuel.html for more detail
      if(injector1StartAngle < 0) {injector1StartAngle += 360;} 
      //Repeat the above for each cylinder
      switch (configPage1.nCylinders)
      {
        //2 cylinders
        case 2:
          injector2StartAngle = (configPage1.inj2Ang + channel2Degrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;}
          break;
        //3 cylinders
        case 3:
          injector2StartAngle = (configPage1.inj2Ang + channel2Degrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;} 
          injector3StartAngle = (configPage1.inj3Ang + channel3Degrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > 360) {injector3StartAngle -= 360;}
          break;
        //4 cylinders
        case 4:
          injector2StartAngle = (configPage1.inj2Ang + channel2Degrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;}
          break;
        //6 cylinders
        case 6: 
          injector2StartAngle = (configPage1.inj2Ang + channel2Degrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;} 
          injector3StartAngle = (configPage1.inj3Ang + channel3Degrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > 360) {injector3StartAngle -= 360;}
          break;
        //8 cylinders
        case 8: 
          injector2StartAngle = (configPage1.inj2Ang + channel2Degrees - ( PWdivTimerPerDegree ));
          if(injector2StartAngle > 360) {injector2StartAngle -= 360;} 
          injector3StartAngle = (configPage1.inj3Ang + channel3Degrees - ( PWdivTimerPerDegree ));
          if(injector3StartAngle > 360) {injector3StartAngle -= 360;}
          injector4StartAngle = (configPage1.inj4Ang + channel4Degrees - ( PWdivTimerPerDegree ));
          if(injector4StartAngle > 360) {injector4StartAngle -= 360;}
          break;
        //Will hit the default case on 1 cylinder or >8 cylinders. Do nothing in these cases
        default:
          break;
      }
    
      //***********************************************************************************************
      //| BEGIN IGNITION CALCULATIONS
      if (currentStatus.RPM > ((unsigned int)(configPage2.SoftRevLim * 100)) ) { currentStatus.advance -= configPage2.SoftLimRetard; } //Softcut RPM limit (If we're above softcut limit, delay timing by configured number of degrees)
      
      //Set dwell
       //Dwell is stored as ms * 10. ie Dwell of 4.3ms would be 43 in configPage2. This number therefore needs to be multiplied by 100 to get dwell in uS
      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { currentStatus.dwell =  (configPage2.dwellCrank * 100); }
      else { currentStatus.dwell =  (configPage2.dwellRun * 100); }
      //Pull battery voltage based dwell correction and apply if needed
      currentStatus.dwellCorrection = table2D_getValue(dwellVCorrectionTable, currentStatus.battery10);
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
          ignition2StartAngle = channel2Degrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;}
          break;
        //3 cylinders
        case 3:
          ignition2StartAngle = channel2Degrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;} 
          ignition3StartAngle = channel3Degrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition3StartAngle > 360) {ignition3StartAngle -= 360;}
          break;
        //4 cylinders
        case 4:
          ignition2StartAngle = channel2Degrees + 360 - currentStatus.advance - dwellAngle; //(div((configPage2.dwellRun*100), timePerDegree).quot ));
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;}
          if(ignition2StartAngle < 0) {ignition2StartAngle += 360;}
          break;
        //6 cylinders
        case 6:
          ignition2StartAngle = channel2Degrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;} 
          ignition3StartAngle = channel3Degrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition3StartAngle > 360) {ignition3StartAngle -= 360;}
          break;
        //8 cylinders
        case 8:
          ignition2StartAngle = channel2Degrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;} 
          ignition3StartAngle = channel3Degrees + 360 - currentStatus.advance - dwellAngle;
          if(ignition3StartAngle > 360) {ignition3StartAngle -= 360;}
          ignition4StartAngle = channel4Degrees + 360 - currentStatus.advance - dwellAngle;
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
      tempCrankAngle = crankAngle - channel2Degrees;
      if( tempCrankAngle < 0) { tempCrankAngle += 360; }
      tempStartAngle = injector2StartAngle - channel2Degrees;
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
      
      tempCrankAngle = crankAngle - channel3Degrees;
      if( tempCrankAngle < 0) { tempCrankAngle += 360; }
      tempStartAngle = injector3StartAngle - channel3Degrees;
      if ( tempStartAngle < 0) { tempStartAngle += 360; }
      if (tempStartAngle > tempCrankAngle)
      { 
        setFuelSchedule3(openInjector3, 
                  ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                  (unsigned long)currentStatus.PW,
                  closeInjector3
                  );
      }
      
      tempCrankAngle = crankAngle - channel4Degrees;
      if( tempCrankAngle < 0) { tempCrankAngle += 360; }
      tempStartAngle = injector4StartAngle - channel4Degrees;
      if ( tempStartAngle < 0) { tempStartAngle += 360; }
      if (tempStartAngle > tempCrankAngle)
      { 
        setFuelSchedule4(openInjector4, 
                  ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                  (unsigned long)currentStatus.PW,
                  closeInjector4
                  );
      }
      //***********************************************************************************************
      //| BEGIN IGNITION SCHEDULES
      //Likewise for the ignition
      //Perform an initial check to see if the ignition is turned on (Ignition only turns on after a preset number of cranking revolutions and:
      //Check for hard cut rev limit (If we're above the hardcut limit, we simply don't set a spark schedule)
      if(ignitionOn && (currentStatus.RPM < ((unsigned int)(configPage2.HardRevLim) * 100) ))
      {
        if ( (ignition1StartAngle > crankAngle) )
        { 
            setIgnitionSchedule1(beginCoil1Charge, 
                      ((unsigned long)(ignition1StartAngle - crankAngle) * (unsigned long)timePerDegree),
                      currentStatus.dwell,
                      endCoil1Charge
                      );
        }

        tempCrankAngle = crankAngle - channel2Degrees;
        if( tempCrankAngle < 0) { tempCrankAngle += 360; }
        tempStartAngle = ignition2StartAngle - channel2Degrees;
        if ( tempStartAngle < 0) { tempStartAngle += 360; }
        if (tempStartAngle > tempCrankAngle)
        { 
            setIgnitionSchedule2(beginCoil2Charge, 
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      currentStatus.dwell,
                      endCoil2Charge
                      );
        }
        
        tempCrankAngle = crankAngle - channel3Degrees;
        if( tempCrankAngle < 0) { tempCrankAngle += 360; }
        tempStartAngle = ignition3StartAngle - channel3Degrees;
        if ( tempStartAngle < 0) { tempStartAngle += 360; }
        if (tempStartAngle > tempCrankAngle)
        { 
            setIgnitionSchedule3(beginCoil3Charge, 
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      currentStatus.dwell,
                      endCoil3Charge
                      );
        }
        
        tempCrankAngle = crankAngle - channel4Degrees;
        if( tempCrankAngle < 0) { tempCrankAngle += 360; }
        tempStartAngle = ignition4StartAngle - channel4Degrees;
        if ( tempStartAngle < 0) { tempStartAngle += 360; }
        if (tempStartAngle > tempCrankAngle)
        { 
            setIgnitionSchedule4(beginCoil4Charge, 
                      ((unsigned long)(tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      currentStatus.dwell,
                      endCoil4Charge
                      );
        }
        
      }
      
    }
    
  }
  
//************************************************************************************************
//Interrupts  

//These functions simply trigger the injector/coil driver off or on. 
//NOTE: squirt status is changed as per http://www.msextra.com/doc/ms1extra/COM_RS232.htm#Acmd
void openInjector1() { digitalWrite(pinInjector1, HIGH); BIT_SET(currentStatus.squirt, 0); } 
void closeInjector1() { digitalWrite(pinInjector1, LOW); BIT_CLEAR(currentStatus.squirt, 0); } 
void beginCoil1Charge() { digitalWrite(pinCoil1, coilHIGH); BIT_SET(currentStatus.spark, 0); digitalWrite(pinTachOut, LOW); }
void endCoil1Charge() { digitalWrite(pinCoil1, coilLOW); BIT_CLEAR(currentStatus.spark, 0); }

void openInjector2() { digitalWrite(pinInjector2, HIGH); BIT_SET(currentStatus.squirt, 1); } //Sets the relevant pin HIGH and changes the current status bit for injector 2 (2nd bit of currentStatus.squirt)
void closeInjector2() { digitalWrite(pinInjector2, LOW); BIT_CLEAR(currentStatus.squirt, 1); } 
void beginCoil2Charge() { digitalWrite(pinCoil2, coilHIGH); BIT_SET(currentStatus.spark, 1); digitalWrite(pinTachOut, LOW); }
void endCoil2Charge() { digitalWrite(pinCoil2, coilLOW); BIT_CLEAR(currentStatus.spark, 1);}

void openInjector3() { digitalWrite(pinInjector3, HIGH); BIT_SET(currentStatus.squirt, 2); } //Sets the relevant pin HIGH and changes the current status bit for injector 3 (3rd bit of currentStatus.squirt)
void closeInjector3() { digitalWrite(pinInjector3, LOW); BIT_CLEAR(currentStatus.squirt, 2); } 
void beginCoil3Charge() { digitalWrite(pinCoil3, coilHIGH); BIT_SET(currentStatus.spark, 2); digitalWrite(pinTachOut, LOW); }
void endCoil3Charge() { digitalWrite(pinCoil3, coilLOW); BIT_CLEAR(currentStatus.spark, 2); }

void openInjector4() { digitalWrite(pinInjector4, HIGH); BIT_SET(currentStatus.squirt, 3); } //Sets the relevant pin HIGH and changes the current status bit for injector 4 (4th bit of currentStatus.squirt)
void closeInjector4() { digitalWrite(pinInjector4, LOW); BIT_CLEAR(currentStatus.squirt, 3); } 
void beginCoil4Charge() { digitalWrite(pinCoil4, coilHIGH); BIT_SET(currentStatus.spark, 3); digitalWrite(pinTachOut, LOW); }
void endCoil4Charge() { digitalWrite(pinCoil4, coilLOW); BIT_CLEAR(currentStatus.spark, 3); }

//Combination functions for semi-sequential injection
void openInjector1and4() { digitalWrite(pinInjector1, HIGH); digitalWrite(pinInjector4, HIGH); BIT_SET(currentStatus.squirt, 0); } 
void closeInjector1and4() { digitalWrite(pinInjector1, LOW); digitalWrite(pinInjector4, LOW);BIT_CLEAR(currentStatus.squirt, 0); }
void openInjector2and3() { digitalWrite(pinInjector2, HIGH); digitalWrite(pinInjector3, HIGH); BIT_SET(currentStatus.squirt, 1); }
void closeInjector2and3() { digitalWrite(pinInjector2, LOW); digitalWrite(pinInjector3, LOW); BIT_CLEAR(currentStatus.squirt, 1); } 

  

