
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
//**************************************************************************************************

int req_fuel = ((engineCapacity / engineInjectorSize) / engineCylinders / engineStoich) * 100; // This doesn't seem quite correct, but I can't find why. 

// Setup section
int triggerPin = 5;
int triggerTeeth = 12;
int triggerMissingTeeth = 1;
int triggerOffset = 120;



void setup() {
  int thisPin;
  // the array elements are numbered from 0 to (pinCount - 1).
  // use a for loop to initialize each pin as an output:
  for (int thisPin = 0; thisPin < pinCount; thisPin++)  {
    pinMode(ledPins[thisPin], OUTPUT);      
  }
  
  
}

void loop() {
  // loop from the lowest pin to the highest:
  for (int thisPin = 0; thisPin < pinCount; thisPin++) { 
    // turn the pin on:
    digitalWrite(ledPins[thisPin], HIGH);   
    delay(timer);                  
    // turn the pin off:
    digitalWrite(ledPins[thisPin], LOW);    

  }
  

