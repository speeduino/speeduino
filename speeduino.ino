
//**************************************************************************************************
// Config section

//The following lines are configurable, but the defaults are probably pretty good for most applications
//#define engineInjectorDeadTime 2500 //Time in uS that the injector takes to open minus the time it takes to close
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

int req_fuel_uS, triggerToothAngle, inj_opentime_uS;
volatile int triggerActualTeeth;
unsigned int triggerFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering)
#define MAX_RPM 10000 //This is the maximum rpm that the ECU will attempt to run at. It is NOT related to the rev limiter, but is instead dictates how fast certain operations will be allowed to run. Lower number gives better performance

volatile int toothCurrentCount = 0; //The current number of teeth (Onec sync has been achieved, this can never actually be 0
volatile unsigned long toothLastToothTime = 0; //The time (micros()) that the last tooth was registered
volatile unsigned long toothLastMinusOneToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered
volatile unsigned long toothOneTime = 0; //The time (micros()) that tooth 1 last triggered
volatile unsigned long toothOneMinusOneTime = 0; //The 2nd to last time (micros()) that tooth 1 last triggered
volatile int toothHistory[512];
volatile int toothHistoryIndex = 0;
volatile byte startRevolutions = 0; //A counter for how many revolutions have been completed since sync was achieved.
volatile bool ignitionOn = true; //The current state of the ignition system

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
  pinMode(pinCoil1, OUTPUT);
  pinMode(pinCoil2, OUTPUT);
  pinMode(pinCoil3, OUTPUT);
  pinMode(pinCoil4, OUTPUT);
  pinMode(pinInjector1, OUTPUT);
  pinMode(pinInjector2, OUTPUT);
  pinMode(pinInjector3, OUTPUT);
  pinMode(pinInjector4, OUTPUT);
  pinMode(pinTachOut, OUTPUT);

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
  
  //Once the configs have been loaded, a number of one time calculations can be completed
  req_fuel_uS = configPage1.reqFuel * 100; //Convert to uS and an int. This is the only variable to be used in calculations
  triggerToothAngle = 360 / configPage2.triggerTeeth; //The number of degrees that passes from tooth to tooth
  triggerActualTeeth = configPage2.triggerTeeth - configPage2.triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
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
  attachInterrupt(triggerInterrupt, trigger, FALLING); // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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
  
  //Setup other relevant pins
  pinMode(pinMAP, INPUT);
  pinMode(pinO2, INPUT);
  pinMode(pinTPS, INPUT);
  pinMode(pinIAT, INPUT);
  pinMode(pinCLT, INPUT);
  //Turn on pullups for above pins
  digitalWrite(pinMAP, HIGH);
  //digitalWrite(pinO2, LOW);
  digitalWrite(pinTPS, LOW);
  
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
      noInterrupts();
      unsigned long revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
      interrupts();
      currentStatus.RPM = ldiv(US_IN_MINUTE, revolutionTime).quot; //Calc RPM based on last full revolution time (uses ldiv rather than div as US_IN_MINUTE is a long)
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
      
      //Determine the current crank angle
      //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
      int crankAngle = (toothCurrentCount - 1) * triggerToothAngle + ((int)(configPage2.triggerAngle)*4); //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth. Needs to be multipled by 4 as the trigger angle is divided by 4 for the serial protocol
      
      //How fast are we going? Need to know how long (uS) it will take to get from one tooth to the next. We then use that to estimate how far we are between the last tooth and the next one
      timePerDegree = ldiv( (toothOneTime - toothOneMinusOneTime) , 360).quot; //The time (uS) it is currently taking to move 1 degree
      //degreesPerLoop = ldiv(1000000L, ((long)currentStatus.loopsPerSecond*timePerDegree)).quot; //The number of crank degrees the pass each loop
      crankAngle += ldiv( (micros() - toothLastToothTime), timePerDegree).quot; //Estimate the number of degrees travelled since the last tooth
      if (crankAngle > 360) { crankAngle -= 360; }
      
      //***********************************************************************************************
      //BEGIN INJECTION TIMING
      //Determine next firing angles
      //1
      int PWdivTimerPerDegree = div(currentStatus.PW, timePerDegree).quot; //How many crank degrees the calculated PW will take at the current speed
      injector1StartAngle = 355 - ( PWdivTimerPerDegree ); //This is a little primitive, but is based on the idea that all fuel needs to be delivered before the inlet valve opens. I am using 355 as the point at which the injector MUST be closed by. See http://www.extraefi.co.uk/sequential_fuel.html for more detail
      if(injector1StartAngle < 0) {injector1StartAngle += 360;} 
      //Repeat the above for each cylinder
      //2
      if (configPage1.nCylinders == 2) 
      { 
        injector2StartAngle = (355 + channel2Degrees - ( PWdivTimerPerDegree ));
        if(injector2StartAngle > 360) {injector2StartAngle -= 360;} 
      }
      //3
      else if (configPage1.nCylinders == 3) 
      {
        injector2StartAngle = (355 + channel2Degrees - ( PWdivTimerPerDegree ));
        if(injector2StartAngle > 360) {injector2StartAngle -= 360;} 
        injector3StartAngle = (355 + channel3Degrees - ( PWdivTimerPerDegree ));
        if(injector3StartAngle > 360) {injector3StartAngle -= 360;}        
      }
      //4 
      else if (configPage1.nCylinders == 4) 
      { 
        injector2StartAngle = (355 + channel2Degrees - ( PWdivTimerPerDegree ));
        if(injector2StartAngle > 360) {injector2StartAngle -= 360;} 
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
      //1
        ignition1StartAngle = 360 - currentStatus.advance - dwellAngle; // 360 - desired advance angle - number of degrees the dwell will take
        if(ignition1StartAngle < 0) {ignition1StartAngle += 360;} 
      //2
      if (configPage1.nCylinders == 2) 
      { 
        ignition2StartAngle = channel2Degrees + 360 - currentStatus.advance - dwellAngle;
        if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;} 
      }
      //3
      else if (configPage1.nCylinders == 3) 
      { 
        ignition2StartAngle = channel2Degrees + 360 - currentStatus.advance - dwellAngle;
        if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;} 
        ignition3StartAngle = channel3Degrees + 360 - currentStatus.advance - dwellAngle;
        if(ignition3StartAngle > 360) {ignition3StartAngle -= 360;}
      }
      //4
      else if (configPage1.nCylinders == 4) 
      { 
        ignition2StartAngle = channel2Degrees + 360 - currentStatus.advance - dwellAngle; //(div((configPage2.dwellRun*100), timePerDegree).quot ));
        if(ignition2StartAngle > 360) {ignition2StartAngle -= 360;}
        if(ignition2StartAngle < 0) {ignition2StartAngle += 360;} 
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

//The trigger function is called everytime a crank tooth passes the sensor
volatile unsigned long curTime;
volatile unsigned int curGap;
volatile unsigned int targetGap; 
void trigger()
  {
   // http://www.msextra.com/forums/viewtopic.php?f=94&t=22976
   // http://www.megamanual.com/ms2/wheel.htm
   noInterrupts(); //Turn off interrupts whilst in this routine

   curTime = micros();
   curGap = curTime - toothLastToothTime;
   if ( curGap < triggerFilterTime ) { interrupts(); return; } //Debounce check. Pulses should never be less than triggerFilterTime, so if they are it means a false trigger. (A 36-1 wheel at 8000pm will have triggers approx. every 200uS)
   toothCurrentCount++; //Increment the tooth counter
   
   //High speed tooth logging history
   toothHistory[toothHistoryIndex] = curGap;
   if(toothHistoryIndex == 511)
   { toothHistoryIndex = 0; }
   else
   { toothHistoryIndex++; }
   
   //Begin the missing tooth detection
   //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
   //if ( (curTime - toothLastToothTime) > (1.5 * (toothLastToothTime - toothLastMinusOneToothTime))) { toothCurrentCount = 1; }
   if(configPage2.triggerMissingTeeth == 1) { targetGap = (3 * (toothLastToothTime - toothLastMinusOneToothTime)) >> 1; } //Multiply by 1.5 (Checks for a gap 1.5x greater than the last one) (Uses bitshift to multiply by 3 then divide by 2. Much faster than multiplying by 1.5)
   //else { targetGap = (10 * (toothLastToothTime - toothLastMinusOneToothTime)) >> 2; } //Multiply by 2.5 (Checks for a gap 2.5x greater than the last one)
   else { targetGap = ((toothLastToothTime - toothLastMinusOneToothTime)) * 2; } //Multiply by 2 (Checks for a gap 2x greater than the last one)
   if ( curGap > targetGap )
   { 
     toothCurrentCount = 1; 
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     startRevolutions++; //Counter 
   } 
   
   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
   
   interrupts(); //Turn interrupts back on
   
  }

  

