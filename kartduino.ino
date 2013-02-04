
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
//**************************************************************************************************

#include "utils.h"

int req_fuel = ((engineCapacity / engineInjectorSize) / engineCylinders / engineStoich) * 100; // This doesn't seem quite correct, but I can't find why. It will be close enough to start an engine

// Setup section
// These aren't really configuration options, more so a description of how the hardware is setup. These are things that will be defined in the recommended hardware setup
int triggerPin = 2;
int triggerTeeth = 12;
int triggerMissingTeeth = 1;
int triggerOffset = 120;
int injectorPin = 6; // The standard pin for the injector driver

volatile boolean hasSync = false;
volatile int currentToothCount = 0; //The current 



void setup() {
  
  //Begin the main crank trigger interttupt setup
  //The interrupt numbering is a bit odd - See here for reference: http://arduino.cc/en/Reference/AttachInterrupt
  int triggerInterrupt = 0; // By default, use 
  switch (triggerPin) {
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
  

  
  
  
  
}

void loop() 
  {
    //Always check for sync
    //Main loop runs within this clause
    if (hasSync)
    {
    
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
void openInjector() { digitalWrite(injectorPin, HIGH); } // Set based on an estimate of when to open the injector
void closeInjector() { digitalWrite(injectorPin, LOW); } // Is called x ms after the open time where x is calculated by the rpm, load and req_fuel

//The trigger function is called everytime a crank tooth passes the 
void trigger()
  {
   
    
  }

  

