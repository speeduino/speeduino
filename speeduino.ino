
//**************************************************************************************************
// Config section

//The following lines are configurable, but the defaults are probably pretty good for most applications
#define engineInjectorDeadTime 1500 //Time in uS that the injector takes to open minus the time it takes to close
#define engineSquirtsPerCycle 2 //Would be 1 for a 2 stroke

//Pin mappings as per the v0.1 shield
#define pinInjector1 8 //Output pin injector 1 is on
#define pinInjector2 9 //Output pin injector 2 is on
#define pinInjector3 11 //Output pin injector 3 is on
#define pinInjector4 10 //Output pin injector 4 is on
#define pinCoil1 6 //Pin for coil 1
#define pinCoil2 7 //Pin for coil 2
#define pinCoil3 12 //Pin for coil 3
#define pinCoil4 13 //Pin for coil 4
#define pinTrigger 2 //The CAS pin
#define pinTPS A0 //TPS input pin
#define pinMAP A1 //MAP sensor pin
#define pinIAT A2 //IAT sensor pin
#define pinCLT A3 //CLS sensor pin
#define pinO2 A4 //O2 Sensor pin

//**************************************************************************************************

#include "globals.h"
#include "utils.h"
#include "table.h"
#include "testing.h"
#include "scheduler.h"
#include "storage.h"
#include "comms.h"
#include "math.h"
#include "corrections.h"
#include "timers.h"

#include "fastAnalog.h"
#define DIGITALIO_NO_MIX_ANALOGWRITE
#include "digitalIOPerformance.h"

struct config1 configPage1;
struct config2 configPage2;

int req_fuel_uS, triggerToothAngle;
volatile int triggerActualTeeth;
int triggerFilterTime = 500; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering)

volatile int toothCurrentCount = 0; //The current number of teeth (Onec sync has been achieved, this can never actually be 0
volatile unsigned long toothLastToothTime = 0; //The time (micros()) that the last tooth was registered
volatile unsigned long toothLastMinusOneToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered
volatile unsigned long toothOneTime = 0; //The time (micros()) that tooth 1 last triggered
volatile unsigned long toothOneMinusOneTime = 0; //The 2nd to last time (micros()) that tooth 1 last triggered

struct table3D fuelTable; //8x8 fuel map
struct table3D ignitionTable; //8x8 ignition map
struct table2D taeTable; //4 bin TPS Acceleration Enrichment map (2D)
struct table2D WUETable; //10 bin Warm Up Enrichment map (2D)
struct table2D cltCalibrationTable;
struct table2D iatCalibrationTable;
struct table2D o2CalibrationTable;

unsigned long counter;
unsigned long currentLoopTime; //The time the current loop started (uS)
unsigned long previousLoopTime; //The time the previous loop started (uS)
unsigned long scheduleStart;
unsigned long scheduleEnd;

byte coilHIGH = HIGH;
byte coilLOW = LOW;

struct statuses currentStatus;
volatile int mainLoopCount;
unsigned long secCounter; //The next time to increment 'runSecs' counter.

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
  
  //Setup the calibration tables
  
  cltCalibrationTable.valueSize = SIZE_INT;
  iatCalibrationTable.valueSize = SIZE_INT;
  o2CalibrationTable.valueSize = SIZE_INT;
  loadCalibration();

  //Need to check early on whether the coil charging is inverted. If this is not set straight away it can cause an unwanted spark at bootup  
  if(configPage2.IgInv == 1) { coilHIGH = LOW, coilLOW = HIGH; }
  else { coilHIGH = HIGH, coilLOW = LOW; }
  digitalWrite(pinCoil1, coilLOW);
  digitalWrite(pinCoil2, coilLOW);
  digitalWrite(pinCoil3, coilLOW);
  digitalWrite(pinCoil4, coilLOW);
  
  initialiseSchedulers();
  initialiseTimers();
  
  //Once the configs have been loaded, a number of one time calculations can be completed
  req_fuel_uS = configPage1.reqFuel * 1000; //Convert to uS and an int. This is the only variable to be used in calculations
  triggerToothAngle = 360 / configPage2.triggerTeeth; //The number of degrees that passes from tooth to tooth
  triggerActualTeeth = configPage2.triggerTeeth - configPage2.triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
  
  //Begin the main crank trigger interrupt pin setup
  //The interrupt numbering is a bit odd - See here for reference: http://arduino.cc/en/Reference/AttachInterrupt
  //These assignments are based on the Arduino Mega AND VARY BETWEEN BOARDS. Please confirm the board you are using and update acordingly. 
  int triggerInterrupt = 0; // By default, use the first interrupt
  currentStatus.RPM = 0;
  currentStatus.hasSync = false;
  currentStatus.runSecs = 0; 
  currentStatus.secl = 0;
  
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
  digitalWrite(pinTrigger, HIGH);
  attachInterrupt(triggerInterrupt, trigger, FALLING); // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
  //End crank triger interrupt attachment
  
  req_fuel_uS = req_fuel_uS / engineSquirtsPerCycle; //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle. If we're doing more than 1 squirt per cycle then we need to split the amount accordingly. (Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the stroke to make the single squirt on)
  
  //Initial values for loop times
  previousLoopTime = 0;
  currentLoopTime = micros();

  //Serial.begin(9600);
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
  
  //Setup other relevant pins
  pinMode(pinMAP, INPUT);
  pinMode(pinO2, INPUT);
  pinMode(pinTPS, INPUT);
  pinMode(pinIAT, INPUT);
  //Turn on pullups for above pins
  digitalWrite(pinMAP, HIGH);
  digitalWrite(pinO2, LOW);
  digitalWrite(pinTPS, LOW);
}

void loop() 
  {
      mainLoopCount++;    
      //Check for any requets from serial
      if ((mainLoopCount & 63) == 1) //Only check the serial buffer (And hence process serial commands) once every 64 loops (64 Is more than fast enough for TunerStudio). This function is equivalent to ((loopCount % 64) == 1) but is considerably faster due to not using the mod or division operations
      {
        if (Serial.available() > 0) 
        {
          command();
        }
      }
     
     /*
     Serial.print("RPM: ");
     Serial.println(currentStatus.RPM);
     
     Serial.print("toothLastToothTime: ");
     Serial.println(toothLastToothTime);
     Serial.print("toothOneMinusOneTime: ");
     Serial.println(toothOneMinusOneTime);
     Serial.print("RevolutionTime: ");
     Serial.println(toothOneTime-toothOneMinusOneTime);
     Serial.print("Tooth Number: ");
     Serial.println(toothCurrentCount);
     */
     
    //Calculate the RPM based on the uS between the last 2 times tooth One was seen.
    previousLoopTime = currentLoopTime;
    currentLoopTime = micros();
    if ((currentLoopTime - toothLastToothTime) < 500000L) //Check how long ago the last tooth was seen compared to now. If it was more than half a second ago then the engine is probably stopped
    {
      noInterrupts();
      unsigned long revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
      interrupts();
      currentStatus.RPM = div(US_IN_MINUTE, revolutionTime).quot; //Calc RPM based on last full revolution time
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      currentStatus.RPM = 0; 
      currentStatus.hasSync = false;
      currentStatus.runSecs = 0; //Reset the counter for number of seconds running.
      secCounter = 0; //Reset our seconds counter.
    }
    
    //Uncomment the following for testing
    
    currentStatus.hasSync = true;
    currentStatus.RPM = 5500;
    
     
    //***SET STATUSES***
    //-----------------------------------------------------------------------------------------------------
    currentStatus.TPSlast = currentStatus.TPS;
    currentStatus.MAP = fastMap(analogRead(pinMAP), 0, 1023, 0, 100); //Get the current MAP value
    currentStatus.tpsADC = fastMap(analogRead(pinTPS), 0, 1023, 0, 255); //Get the current raw TPS ADC value and map it into a byte
    currentStatus.TPS = fastMap(currentStatus.tpsADC, configPage1.tpsMin, configPage1.tpsMax, 0, 100); //Take the raw TPS ADC value and convert it into a TPS% based on the calibrated values
    currentStatus.O2 = fastMap(analogRead(pinO2), 0, 1023, 117, 358); //Get the current O2 value. Calibration is from AFR values 7.35 to 22.4, then multiplied by 16 (<< 4). This is the correct calibration for an Innovate Wideband 0v - 5V unit
    //The IAT and CLT readings can be done less frequently. This still runs about 10 times per second
    if ((mainLoopCount & 127) == 1)
    {
       currentStatus.cltADC = fastMap(analogRead(pinCLT), 0, 1023, 0, 255); //Get the current raw CLT value
       currentStatus.iatADC = fastMap(analogRead(pinIAT), 0, 1023, 0, 255); //Get the current raw IAT value
       
       currentStatus.coolant = table2D_getValue(cltCalibrationTable, currentStatus.cltADC);
       currentStatus.IAT = table2D_getValue(iatCalibrationTable, currentStatus.iatADC);
    }

    //Always check for sync
    //Main loop runs within this clause
    if (currentStatus.hasSync)
    {
       if(currentStatus.RPM > 0) //Check if the engine is turning at all
       { 
          //If it is, check is we're running or cranking
          if(currentStatus.RPM > configPage2.crankRPM) 
          { //Sets the engine running bit, clears the engine cranking bit
            BIT_SET(currentStatus.engine, BIT_ENGINE_RUN); 
            BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK); 
          } 
          else 
          {  //Sets the engine cranking bit, clears the engine running bit
            BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK); 
            BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN); 
            currentStatus.runSecs = 0; //We're cranking (hopefully), so reset the engine run time to prompt ASE.
          } 
       }
      
      //END SETTING STATUSES
      //-----------------------------------------------------------------------------------------------------
      
      //Begin the fuel calculation
      //Calculate an injector pulsewidth from the VE
      byte corrections = correctionsTotal();
      if (configPage1.algorithm == 0) //Check with fuelling algorithm is being used
      { 
        //Speed Density
        currentStatus.VE = get3DTableValue(fuelTable, currentStatus.MAP, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
        currentStatus.PW = PW_SD(req_fuel_uS, currentStatus.VE, currentStatus.MAP, corrections, engineInjectorDeadTime);
        currentStatus.advance = get3DTableValue(ignitionTable, currentStatus.MAP, currentStatus.RPM); //As above, but for ignition advance
      }
      else
      { 
        //Alpha-N
        currentStatus.VE = get3DTableValue(fuelTable, currentStatus.TPS, currentStatus.RPM); //Perform lookup into fuel map for RPM vs TPS value
        currentStatus.PW = PW_AN(req_fuel_uS, currentStatus.VE, currentStatus.TPS, corrections, engineInjectorDeadTime); //The 100 here is just a placeholder for any enrichment factors (Cold start, acceleration etc). To add 10% extra fuel, this would be 110 
        currentStatus.advance = get3DTableValue(ignitionTable, currentStatus.TPS, currentStatus.RPM); //As above, but for ignition advance
      }

      
      //Determine the current crank angle
      //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
      int crankAngle = (toothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth
      if (crankAngle > 360) { crankAngle -= 360; }
      
      //How fast are we going? Need to know how long (uS) it will take to get from one tooth to the next. We then use that to estimate how far we are between the last tooth and the next one
      unsigned long timePerDegree = div( (toothOneTime - toothOneMinusOneTime), (triggerToothAngle * configPage2.triggerTeeth)).quot; //The time (uS) it is currently taking to move 1 degree (fastDivide version)
      crankAngle += div( (micros() - toothLastToothTime), timePerDegree).quot; //Estimate the number of degrees travelled since the last tooth
      
      //Determine next firing angles
      int injector1StartAngle = 355 - ( div(currentStatus.PW, timePerDegree).quot ); //This is a little primitive, but is based on the idea that all fuel needs to be delivered before the inlet valve opens. I am using 355 as the point at which the injector MUST be closed by. See http://www.extraefi.co.uk/sequential_fuel.html for more detail

      if (currentStatus.RPM > ((int)(configPage2.SoftRevLim * 100)) ) { currentStatus.advance -= configPage2.SoftLimRetard; } //Softcut RPM limit (If we're above softcut limit, delay timing by configured number of degrees)
      int ignition1StartAngle = 360 - currentStatus.advance - (div((configPage2.dwellRun*100), timePerDegree).quot ); // 360 - desired advance angle - number of degrees the dwell will take

      
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the shcedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time
      if (injector1StartAngle > crankAngle) 
      { 
        setFuelSchedule1(openInjector1, 
                  (injector1StartAngle - crankAngle) * timePerDegree,
                  currentStatus.PW,
                  closeInjector1
                  );
      }
      //Likewise for the ignition
      if ( ignition1StartAngle > crankAngle)
      { 
        if (currentStatus.RPM < ((int)(configPage2.HardRevLim) * 100) ) //Check for hard cut rev limit (If we're above the hardcut limit, we simply don't set a spark schedule)
        {
          setIgnitionSchedule1(beginCoil1Charge, 
                    (ignition1StartAngle - crankAngle) * timePerDegree,
                    (configPage2.dwellRun * 100), //Dwell is stored as ms * 10. ie Dwell of 4.3ms would be 43 in configPage2. This number therefore needs to be multiplied by 100 to get dwell in uS
                    endCoil1Charge
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
void closeInjector1() { digitalWrite(pinInjector1, LOW); BIT_CLEAR(currentStatus.squirt, 0);} 
void beginCoil1Charge() { digitalWrite(pinCoil1, coilHIGH); }
void endCoil1Charge() { digitalWrite(pinCoil1, coilLOW); }

void openInjector2() { digitalWrite(pinInjector2, HIGH); BIT_SET(currentStatus.squirt, 1); } //Sets the relevant pin HIGH and changes the current status bit for injector 2 (2nd bit of currentStatus.squirt)
void closeInjector2() { digitalWrite(pinInjector2, LOW); BIT_CLEAR(currentStatus.squirt, 1); } 
void beginCoil2Charge() { digitalWrite(pinCoil2, coilHIGH); }
void endCoil2Charge() { digitalWrite(pinCoil2, coilLOW); }

void openInjector3() { digitalWrite(pinInjector3, HIGH); BIT_SET(currentStatus.squirt, 1); } //Sets the relevant pin HIGH and changes the current status bit for injector 2 (2nd bit of currentStatus.squirt)
void closeInjector3() { digitalWrite(pinInjector3, LOW); BIT_CLEAR(currentStatus.squirt, 1); } 
void beginCoil3Charge() { digitalWrite(pinCoil3, coilHIGH); }
void endCoil3Charge() { digitalWrite(pinCoil3, coilLOW); }

void openInjector4() { digitalWrite(pinInjector4, HIGH); BIT_SET(currentStatus.squirt, 1); } //Sets the relevant pin HIGH and changes the current status bit for injector 2 (2nd bit of currentStatus.squirt)
void closeInjector4() { digitalWrite(pinInjector4, LOW); BIT_CLEAR(currentStatus.squirt, 1); } 
void beginCoil4Charge() { digitalWrite(pinCoil4, coilHIGH); }
void endCoil4Charge() { digitalWrite(pinCoil4, coilLOW); }

//The trigger function is called everytime a crank tooth passes the sensor
void trigger()
  {
   // http://www.msextra.com/forums/viewtopic.php?f=94&t=22976
   // http://www.megamanual.com/ms2/wheel.htm
   noInterrupts(); //Turn off interrupts whilst in this routine

   volatile unsigned long curTime = micros();
   if ( (curTime - toothLastToothTime) < triggerFilterTime) { interrupts(); return; } //Debounce check. Pulses should never be less than triggerFilterTime, so if they are it means a false trigger. (A 36-1 wheel at 8000pm will have triggers approx. every 200uS)
   toothCurrentCount++; //Increment the tooth counter   
   
   //Begin the missing tooth detection
   //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
   //if ( (curTime - toothLastToothTime) > (1.5 * (toothLastToothTime - toothLastMinusOneToothTime))) { toothCurrentCount = 1; }
   if ( (curTime - toothLastToothTime) > ((3 * (toothLastToothTime - toothLastMinusOneToothTime))>>1)) //Same as above, but uses bitshift instead of multiplying by 1.5
   { 
     toothCurrentCount = 1; 
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
   } 
   
   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
   
   interrupts(); //Turn interrupts back on
   
  }

  

