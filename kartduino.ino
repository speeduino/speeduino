
//**************************************************************************************************
// Config section
//this section is where all the user set stuff is. This will eventually be replaced by a config file

/*
Need to calculate the req_fuel figure here, preferably in pre-processor macro
*/
#define engineCapacity 100 // In cc
#define engineCylinders 1 // May support more than 1 cyl later. Always will assume 1 injector per cylinder. 
#define engineInjectorSize 100 // In cc/min
#define engineStoich 14.7 // Stoichiometric ratio of fuel used
#define engineStrokes 4 //Can be 2 stroke or 4 stroke, any other value will cause problems
#define triggerTeeth 12 //The full count of teeth on the trigger wheel if there were no gaps
#define triggerMissingTeeth 1 //The size of the tooth gap (ie number of missing teeth)

//The following lines are configurable, but the defaults are probably pretty good for most applications
#define engineInjectorDeadTime 1.5 //Time in ms that the injector takes to open
#define engineSquirtsPerCycle 2 //Would be 1 for a 2 stroke
//**************************************************************************************************

#include "utils.h"
#include "table.h"
#include "testing.h"

int req_fuel = ((engineCapacity / engineInjectorSize) / engineCylinders / engineStoich) * 100; // This doesn't seem quite correct, but I can't find why. It will be close enough to start an engine



// Setup section
// These aren't really configuration options, more so a description of how the hardware is setup. These are things that will be defined in the recommended hardware setup
int triggerActualTeeth = triggerTeeth - triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
int triggerOffset = 120;
int triggerToothAngle = 360 / triggerTeeth; //The number of degrees that passes from tooth to tooth
int pinInjector = 6; // The standard pin for the injector driver
int pinTrigger = 2;

volatile boolean hasSync = false;
volatile int toothCurrentCount = 0; //The current number of teeth (Onec sync has been achieved, this can never actually be 0
volatile unsigned long toothLastToothTime = 0; //The time (micros()) that the last tooth was registered
volatile unsigned long toothLastMinusOneToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered

int rpm = 0; //Stores the last recorded RPM value
struct table fuelTable;



void setup() {
  
  //Begin the main crank trigger interrupt pin setup
  //The interrupt numbering is a bit odd - See here for reference: http://arduino.cc/en/Reference/AttachInterrupt
  int triggerInterrupt = 0; // By default, use the first interrupt. The user should always have set things up (Or even better, use the recommended pinouts)
  switch (pinInjector) {
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
  attachInterrupt(triggerInterrupt, trigger, RISING); // Attach the crank trigger wheel interrupt
  //End crank triger interrupt attachment
  
  req_fuel = req_fuel / engineSquirtsPerCycle; //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle. If we're doing more than 1 squirt per cycle then we need to split the amount accordingly. (Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the stroke to make the single squirt on)

  
  
  
  
}

void loop() 
  {
    //Always check for sync
    //Main loop runs within this clause
    if (hasSync)
    {
      
      //Calculate the RPM based on the time between the last 2 teeth. I have no idea whether this will be accurate AT ALL, but it's fairly efficient and means there doesn't need to be another variable placed into the trigger interrupt
      if (toothCurrentCount != 1) //We can't perform the RPM calculation if we're at the first tooth as the timing would be double (Well, we can, but it would need a different calculation and I don't think it's worth it, just use the last RPM value)
      {
        long revolutionTime = (triggerTeeth * (toothLastToothTime - toothLastMinusOneToothTime)); //The time in us that one revolution would take at current speed
        rpm = US_IN_MINUTE / revolutionTime;
      }
      
      //Get the current MAP value
      int MAP = 1; //Placeholder
      
      //Perform lookup into fuel map for RPM vs MAP value
      int VE = getTableValue(fuelTable, rpm, MAP);
      
      //From all of the above, calculate an injector pulsewidth
      int pulseWidth = PW(req_fuel, VE, MAP, 100, engineInjectorDeadTime); //The 100 here is just a placeholder for any enrichment factors (Cold start, acceleration etc). To add 10% extra fuel, this would be 110
      
    
    }
    else
    { getSync(); }

  }
  
//The get Sync function attempts to wait 
void getSync()
  {
    
  }


//Interrupts  

//These 2 functions simply trigger the injector driver off or on. 
void openInjector() { digitalWrite(pinInjector, HIGH); } // Set based on an estimate of when to open the injector
void closeInjector() { digitalWrite(pinInjector, LOW); } // Is called x ms after the open time where x is calculated by the rpm, load and req_fuel

//The trigger function is called everytime a crank tooth passes the sensor
void trigger()
  {
   // http://www.msextra.com/forums/viewtopic.php?f=94&t=22976
   // http://www.megamanual.com/ms2/wheel.htm 
   toothCurrentCount++; //Increment the tooth counter
   
   //Begin the missing tooth detection
   unsigned long curTime = micros();
   //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
   if ( (curTime - toothLastToothTime) > (1.5 * (toothLastToothTime - toothLastMinusOneToothTime))) { toothCurrentCount = 1; }
   
   // Update the last few tooth times
   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
   
  }

  

