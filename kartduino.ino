
//**************************************************************************************************
// Config section
//this section is where all the user set stuff is. This will eventually be replaced by a config file

/*
Need to calculate the req_fuel figure here, preferably in pre-processor macro
*/
#define engineCapacity 148 // In cc
#define engineCylinders 1 // May support more than 1 cyl later. Always will assume 1 injector per cylinder. 
#define engineInjectorSize 80 // In cc/min
#define engineStoich 14.7 // Stoichiometric ratio of fuel used
#define engineStrokes 4 //Can be 2 stroke or 4 stroke, any other value will cause problems
#define engineDwell 3000 //The spark dwell time in uS
#define triggerTeeth 12 //The full count of teeth on the trigger wheel if there were no gaps
#define triggerMissingTeeth 1 //The size of the tooth gap (ie number of missing teeth)
#define triggerAngle 110 // The angle (Degrees) from TDC that No 1 cylinder is at when tooth #1 passes the sensor. CANNOT BE 0

//The following lines are configurable, but the defaults are probably pretty good for most applications
#define engineInjectorDeadTime 1500 //Time in uS that the injector takes to open
#define engineSquirtsPerCycle 2 //Would be 1 for a 2 stroke

#define pinInjector 6 //Output pin the injector is on (Assumes 1 cyl only)
#define pinCoil 7 //Pin for the coil (AS above, 1 cyl only)
#define pinTPS 8 //TPS input pin
#define pinTrigger 2 //The CAS pin
//**************************************************************************************************

#include "utils.h"
#include "table.h"
#include "testing.h"
#include "scheduler.h"

#include "fastAnalog.h"
#include "digitalIOPerformance.h"

float req_fuel = ((engineCapacity / engineInjectorSize) / engineCylinders / engineStoich) * 100; // This doesn't seem quite correct, but I can't find why. It will be close enough to start an engine
int req_fuel_uS = req_fuel * 1000; //Convert to uS and, importantly, an int. This is the only variable to be used in calculations

// Setup section
// These aren't really configuration options, more so a description of how the hardware is setup. These are things that will be defined in the recommended hardware setup
int triggerActualTeeth = triggerTeeth - triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
int triggerOffset = 120;
int triggerToothAngle = 360 / triggerTeeth; //The number of degrees that passes from tooth to tooth

volatile boolean hasSync = false;
volatile int toothCurrentCount = 0; //The current number of teeth (Onec sync has been achieved, this can never actually be 0
volatile unsigned long toothLastToothTime = 0; //The time (micros()) that the last tooth was registered
volatile unsigned long toothLastMinusOneToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered

int rpm = 0; //Stores the last recorded RPM value
struct table fuelTable;
struct table ignitionTable;

unsigned long injectTime[engineCylinders]; //The system time in uS that each injector needs to next fire at
boolean intjectorNeedsFire[engineCylinders]; //Whether each injector needs to fire or not

unsigned long counter;
unsigned long scheduleStart;
unsigned long scheduleEnd;

void setup() {
  
  //Begin the main crank trigger interrupt pin setup
  //The interrupt numbering is a bit odd - See here for reference: http://arduino.cc/en/Reference/AttachInterrupt
  //These assignments are based on the Arduino Mega AND VARY BETWEEN BOARDS. Please confirm the board you are using and update acordingly. 
  int triggerInterrupt = 0; // By default, use the first interrupt. The user should always have set things up (Or even better, use the recommended pinouts)
  switch (pinTrigger) {
    
    //Arduino Mega 2560 mapping (Uncomment to use)
    /*
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
      */
      
    //Arduino Leo(nardo/stick) mapping (Comment this section if using a mega)
    case 3:
      triggerInterrupt = 0; break;
    case 2:
      triggerInterrupt = 1; break;
    case 0:
      triggerInterrupt = 2; break;
    case 1:
      triggerInterrupt = 3; break;
    case 7:
      triggerInterrupt = 4; break;  
  }
  attachInterrupt(triggerInterrupt, trigger, FALLING); // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
  //End crank triger interrupt attachment
  
  req_fuel_uS = req_fuel_uS / engineSquirtsPerCycle; //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle. If we're doing more than 1 squirt per cycle then we need to split the amount accordingly. (Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the stroke to make the single squirt on)

  Serial.begin(9600);
  
  //This sets the ADC (Analog to Digitial Converter) to run at 1Mhz, greatly reducing analog read times (MAP/TPS)
  //1Mhz is the fastest speed permitted by the CPU without affecting accuracy
  //Please see chapter 11 of 'Practical Arduino' (http://books.google.com.au/books?id=HsTxON1L6D4C&printsec=frontcover#v=onepage&q&f=false) for more details
  //Can be disabled by removing the #include "fastAnalog.h" above
  #ifdef sbi
    sbi(ADCSRA,ADPS2);
    cbi(ADCSRA,ADPS1);
    cbi(ADCSRA,ADPS0);
  #endif
  
  //Setup the dummy fuel and ignition tables
  dummyFuelTable(&fuelTable);
  dummyIgnitionTable(&ignitionTable);
  initialiseScheduler();
  counter = 0;
}

void loop() 
  {
    //delay(2500);
    //Always check for sync
    //Main loop runs within this clause
    if (hasSync)
    {
      
      //Calculate the RPM based on the time between the last 2 teeth. I have no idea whether this will be accurate AT ALL, but it's fairly efficient and means there doesn't need to be another variable placed into the trigger interrupt
      if (toothCurrentCount != 1) //We can't perform the RPM calculation if we're at the first tooth as the timing would be double (Well, we can, but it would need a different calculation and I don't think it's worth it, just use the last RPM value)
      {
        long revolutionTime = (triggerTeeth * (toothLastToothTime - toothLastMinusOneToothTime)); //The time in uS that one revolution would take at current speed
        rpm = US_IN_MINUTE / revolutionTime;
      }
      //Serial.print("RPM: "); Serial.println(rpm);
      rpm = 1000;
      //Get the current MAP value
      int MAP = 20; //Placeholder
      int TPS = 20; //Placeholder
      
      //Begin the fuel calculation
      //Perform lookup into fuel map for RPM vs MAP value
      int VE = getTableValue(fuelTable, MAP, rpm);
      //Calculate an injector pulsewidth form the VE
      unsigned long pulseWidth = PW(req_fuel_uS, VE, MAP, 100, engineInjectorDeadTime); //The 100 here is just a placeholder for any enrichment factors (Cold start, acceleration etc). To add 10% extra fuel, this would be 110
      
      //Perform a lookup to get the desired ignition advance
      int ignitionAdvance = getTableValue(ignitionTable, MAP, rpm);
      
      //Determine the current crank angle
      int crankAngle = (toothCurrentCount - 1) * triggerToothAngle + triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is from TDC
      if (crankAngle > 360) { crankAngle -= 360; } //Not sure if this is actually required
      
      //Serial.print("Crank angle: "); Serial.println(crankAngle);
      
      //How fast are we going? Can possibly work this out from RPM, but I don't think it's going to take a lot of CPU
      unsigned long timePerDegree = (toothLastToothTime - toothLastMinusOneToothTime) / triggerToothAngle; //The time (uS) it is currently taking to move 1 degree
      
      //Determine next firing angles
      int injectorStartAngle = 355 - (pulseWidth / timePerDegree); //This is a bit rough, but is based on the idea that all fuel needs to be delivered before the inlet valve opens. I am using 355 as the point at which the injector MUST be closed by. See http://www.extraefi.co.uk/sequential_fuel.html for more detail
      int ignitionStartAngle = 360 - ignitionAdvance; //Simple
      
      //Serial.print("Injector start angle: "); Serial.println(injectorStartAngle);
      
      
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the shcedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time
      if (injectorStartAngle > crankAngle) 
      { 
        setSchedule1(openInjector, 
                  (injectorStartAngle - crankAngle) * timePerDegree,
                  pulseWidth,
                  closeInjector
                  );
      }
      //Likewise for the ignition
      if (ignitionStartAngle > crankAngle) 
      { 
        setSchedule2(beginCoilCharge, 
                  (ignitionStartAngle - crankAngle) * timePerDegree,
                  engineDwell,
                  endCoilCharge
                  );
      }
      
      
      //Serial.println(VE);
      //Serial.print("VE: ");
      //Serial.println(VE);
      
      //Serial.print("Injector pulsewidth: ");
      //Serial.println(pulseWidth);
      //Serial.println(req_fuel * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(100/100.0) + engineInjectorDeadTime);
      //Serial.println( (float)(req_fuel * (float)(VE/100)) );
      //Serial.println( (float)(VE/100.0));
      
      /*
      if (counter > 100000) {
      scheduleStart = micros();
      setSchedule1(openInjector2, 1000000);
      counter = 0;
      }
      counter++;
      
      
      if (scheduleEnd != 0) { 
        Serial.print("The schedule took (uS): "); 
        Serial.println(scheduleEnd - scheduleStart);
        scheduleEnd = 0;
      }
      */
      trigger();
    
    }
    else
    { getSync(); }
    

    //Serial.println(toothLastToothTime);
    //Serial.println(toothLastMinusOneToothTime);
    //Serial.println(rpm);
    //delay(100);

  }
  
//The get Sync function attempts to wait 
void getSync()
  {
    
    //VERY basic waiting for sync routine. Artifically set the tooth count to be great than 1, then just wait for it to be reset to one (Which occurs in the trigger interrupt function)
    /*
    toothCurrentCount = 2;
    while (toothCurrentCount > 1)
    {
      delay(1);
    }
    */
    
    //The are some placeholder values so we can get a fake RPM
    toothLastMinusOneToothTime = micros();
    delay(1); //A 1000us delay should make for about a 5000rpm test speed with a 12 tooth wheel(60000000us / (1000us * triggerTeeth)
    toothLastToothTime = micros();
    
    hasSync = true;
  }


//Interrupts  

//These 4 functions simply trigger the injector/coil driver off or on. 
//void openInjector2() { scheduleEnd = micros();}
void openInjector() { digitalWrite(pinInjector, HIGH); } 
void closeInjector() { digitalWrite(pinInjector, LOW); } 
void beginCoilCharge() { digitalWrite(pinCoil, HIGH); }
void endCoilCharge() { digitalWrite(pinCoil, LOW); }

//The trigger function is called everytime a crank tooth passes the sensor
void trigger()
  {
   // http://www.msextra.com/forums/viewtopic.php?f=94&t=22976
   // http://www.megamanual.com/ms2/wheel.htm
   unsigned long curTime = micros();
   toothCurrentCount++; //Increment the tooth counter
   
   //Begin the missing tooth detection
   //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
   //if ( (curTime - toothLastToothTime) > (1.5 * (toothLastToothTime - toothLastMinusOneToothTime))) { toothCurrentCount = 1; }
   if ( (curTime - toothLastToothTime) > ((3 * (toothLastToothTime - toothLastMinusOneToothTime))>>1)) { toothCurrentCount = 1; } //Same as above, but uses bitshift instead of multiplying by 1.5
   //if (toothCurrentCount > triggerActualTeeth) { toothCurrentCount = 1; } //For testing ONLY
   
   // Update the last few tooth times
   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
   
  }

  

