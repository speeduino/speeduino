
//**************************************************************************************************
// Config section
//this section is where all the user set stuff is. This will eventually be replaced by a config file

/*
Need to calculate the req_fuel figure here, preferably in pre-processor macro
*/


//The following lines are configurable, but the defaults are probably pretty good for most applications
#define engineInjectorDeadTime 1500 //Time in uS that the injector takes to open
#define engineSquirtsPerCycle 2 //Would be 1 for a 2 stroke

#define pinInjector 6 //Output pin the injector is on (Assumes 1 cyl only)
#define pinCoil 7 //Pin for the coil (AS above, 1 cyl only)
#define pinTPS 8 //TPS input pin
#define pinTrigger 2 //The CAS pin
#define pinMAP 0 //MAP sensor pin
#define pinO2 1 //O2 Sensor pin
//**************************************************************************************************

#include "globals.h"
#include "utils.h"
#include "table.h"
#include "testing.h"
#include "scheduler.h"
#include "storage.h"
#include "comms.h"
#include "math.h"

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

struct table fuelTable;
struct table ignitionTable;

unsigned long counter;
unsigned long scheduleStart;
unsigned long scheduleEnd;

struct statuses currentStatus;
int loopCount;

void setup() 
{

  pinMode(pinCoil, OUTPUT);
  digitalWrite(pinCoil, LOW);
  
  
  //Setup the dummy fuel and ignition tables
  //dummyFuelTable(&fuelTable);
  //dummyIgnitionTable(&ignitionTable);
  loadConfig();
  initialiseSchedulers();
  
  //Once the configs have been loaded, a number of one time calculations can be completed
  req_fuel_uS = configPage1.reqFuel * 1000; //Convert to uS and an int. This is the only variable to be used in calculations
  triggerToothAngle = 360 / configPage2.triggerTeeth; //The number of degrees that passes from tooth to tooth
  triggerActualTeeth = configPage2.triggerTeeth - configPage2.triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
  
  
  //Begin the main crank trigger interrupt pin setup
  //The interrupt numbering is a bit odd - See here for reference: http://arduino.cc/en/Reference/AttachInterrupt
  //These assignments are based on the Arduino Mega AND VARY BETWEEN BOARDS. Please confirm the board you are using and update acordingly. 
  int triggerInterrupt = 0; // By default, use the first interrupt. The user should always have set things up (Or even better, use the recommended pinouts)
  currentStatus.RPM = 0;
  currentStatus.hasSync = false;
  switch (pinTrigger) {
    
    //Arduino Mega 2560 mapping (Uncomment to use)
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
  
  loopCount = 0;
  
  triggerActualTeeth = configPage2.triggerTeeth - configPage2.triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
  triggerToothAngle = 360 / configPage2.triggerTeeth;
  
  //Setup some LEDs for testing
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
}

void loop() 
  {
      loopCount++;    
      //Check for any requets from serial
      if (loopCount == 50) //Only check the serial buffer (And hence process serial commands) once every x loops (50 Is more than fast enough for TunerStudio)
      {
        if (Serial.available() > 0) 
        {
          command();
        }
        loopCount = 0;
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
    if ((micros() - toothLastToothTime) < 500000L) //Check how long ago the last tooth was seen compared to now. If it was more than half a second ago then the engine is probably stopped
    {
      noInterrupts();
        unsigned long revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
      interrupts();
      //currentStatus.RPM = US_IN_MINUTE / revolutionTime;
      currentStatus.RPM = fastDivide32(US_IN_MINUTE, revolutionTime);  // (fastDivide version of above line)
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      currentStatus.RPM = 0; 
      currentStatus.hasSync = false;
    }
     
     
    //Always check for sync
    //Main loop runs within this clause
    if (currentStatus.hasSync)
    {
      //***SET STATUSES***
      //-----------------------------------------------------------------------------------------------------
      currentStatus.MAP = fastMap(analogRead(pinMAP), 0, 1023, 0, 100); //Get the current MAP value
      currentStatus.TPS = fastMap(analogRead(pinTPS), 0, 1023, 0, 100); //Get the current TPS value
      currentStatus.O2 = fastMap(analogRead(pinO2), 0, 1023, 117, 358); //Get the current O2 value. Calibration is from AFR values 7.35 to 22.4, then multiple by 16 (<< 4). This is the correct calibration for an Innovate Wideband 0v - 5V unit
      if(currentStatus.RPM > 0) //Check if the engine is turning at all
      { 
        //If it is, check is we're running or cranking
        if(currentStatus.RPM > configPage2.crankRPM) { BIT_SET(currentStatus.engine, 0); BIT_CLEAR(currentStatus.engine, 1); } //Sets the engine running bit, clears the engine cranking bit
        else {  BIT_SET(currentStatus.engine, 1); BIT_CLEAR(currentStatus.engine, 0); } //Sets the engine cranking bit, clears the engine running bit
      }
      
      //END SETTING STATUSES
      //-----------------------------------------------------------------------------------------------------
      
      //Begin the fuel calculation
      //Perform lookup into fuel map for RPM vs MAP value
      currentStatus.VE = getTableValue(fuelTable, currentStatus.MAP, currentStatus.RPM);
      //Calculate an injector pulsewidth form the VE
      currentStatus.PW = PW(req_fuel_uS, currentStatus.VE, currentStatus.MAP, 100, engineInjectorDeadTime); //The 100 here is just a placeholder for any enrichment factors (Cold start, acceleration etc). To add 10% extra fuel, this would be 110
      //Perform a lookup to get the desired ignition advance
      byte ignitionAdvance = getTableValue(ignitionTable, currentStatus.MAP, currentStatus.RPM);
      
      //Determine the current crank angle
      //This is the current angle ATDC the engine is at
      int crankAngle = (toothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth
      if (crankAngle > 360) { crankAngle -= 360; }
      
      //How fast are we going? Can possibly work this out from RPM, but I don't think it's going to take a lot of CPU
      //unsigned long timePerDegree = (toothLastToothTime - toothLastMinusOneToothTime) / triggerToothAngle; //The time (uS) it is currently taking to move 1 degree
      unsigned long timePerDegree = fastDivide32( (toothLastToothTime - toothLastMinusOneToothTime), triggerToothAngle); //The time (uS) it is currently taking to move 1 degree (fastDivide version)
      //crankAngle += (micros() - toothLastToothTime) / timePerDegree; //Estimate the number of degrees travelled since the last tooth
      crankAngle += fastDivide32( (micros() - toothLastToothTime), timePerDegree); //Estimate the number of degrees travelled since the last tooth (fastDivide version)
      
      //Determine next firing angles
      //int injectorStartAngle = 355 - (currentStatus.PW / timePerDegree); //This is a bit rough, but is based on the idea that all fuel needs to be delivered before the inlet valve opens. I am using 355 as the point at which the injector MUST be closed by. See http://www.extraefi.co.uk/sequential_fuel.html for more detail
      //int ignitionStartAngle = 360 - ignitionAdvance - (configPage2.dwellRun / timePerDegree); // 360 - desired advance angle - number of degrees the dwell will take
      int injectorStartAngle = 355 - ( fastDivide32(currentStatus.PW, timePerDegree) ); //As above, but using fastDivide function
      int ignitionStartAngle = 360 - ignitionAdvance - (fastDivide32(configPage2.dwellRun, timePerDegree) ); //As above, but using fastDivide function
      
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the shcedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time
      if (injectorStartAngle > crankAngle) 
      { 
        setFuelSchedule1(openInjector1, 
                  (injectorStartAngle - crankAngle) * timePerDegree,
                  currentStatus.PW,
                  closeInjector1
                  );
      }
      //Likewise for the ignition
      if (ignitionStartAngle > crankAngle) 
      { 
        setIgnitionSchedule1(beginCoil1Charge, 
                  (ignitionStartAngle - crankAngle) * timePerDegree,
                  configPage2.dwellRun,
                  endCoil1Charge
                  );
      }
      
    }
    

  }
  

//Interrupts  

//These functions simply trigger the injector/coil driver off or on. 
//NOTE: squirt status is changed as per http://www.msextra.com/doc/ms1extra/COM_RS232.htm#Acmd
//Useful bit math:
// x &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
// x |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
void openInjector1() { digitalWrite(pinInjector, HIGH); BIT_SET(currentStatus.squirt, 0); } 
void closeInjector1() { digitalWrite(pinInjector, LOW); BIT_CLEAR(currentStatus.squirt, 0);} 
void beginCoil1Charge() { digitalWrite(pinCoil, HIGH); }
void endCoil1Charge() { digitalWrite(pinCoil, LOW); }

void openInjector2() { digitalWrite(pinInjector, HIGH); BIT_SET(currentStatus.squirt, 1); } //Sets the relevant pin HIGH and changes the current status bit for injector 2 (2nd bit of currentStatus.squirt)
void closeInjector2() { digitalWrite(pinInjector, LOW); } 
void beginCoil2Charge() { digitalWrite(pinCoil, HIGH); }
void endCoil2Charge() { digitalWrite(pinCoil, LOW); }

//The trigger function is called everytime a crank tooth passes the sensor
void trigger()
  {
   // http://www.msextra.com/forums/viewtopic.php?f=94&t=22976
   // http://www.megamanual.com/ms2/wheel.htm
   noInterrupts(); //Turn off interrupts whilst in this routine

   volatile unsigned long curTime = micros();
   if ( (curTime - toothLastToothTime) < triggerFilterTime) { interrupts(); return; } //Debounce check. Pulses should never be less than 100uS, so if they are it means a false trigger. (A 36-1 wheel at 8000pm will have triggers approx. every 200uS)
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
   
   //TESTING METHOD
   /*
   if (toothCurrentCount > triggerActualTeeth)
   { 
      toothCurrentCount = 1; 
      toothOneMinusOneTime = toothOneTime;
      toothOneTime = curTime;
   }
   */
   
   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
   
   interrupts(); //Turn interrupts back on
   
  }

  

