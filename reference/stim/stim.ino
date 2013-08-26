/* FORD 1996 CKP crank signal simulator 36-1
 by Ichabod mudd 
 */
#include "Arduino.h"
#define PULSE_PIN 10
#define CAM_PIN  8
#define MPH_PIN 12
#define POT_PIN 0

#define PULSE_DURATION 20

int mph_toggle = 1 ;
int val;
unsigned int pulse_gap;

// the setup routine runs once when you press reset:
void setup() 
{
  pinMode(PULSE_PIN, OUTPUT);
  pinMode(CAM_PIN, OUTPUT);
  pinMode(MPH_PIN, OUTPUT);
}


//reluctor wheel
// subroutines first

//function to first go HiGH

void triggerHigh(int duration, int count)
{
switch (count)
{
  case 1:  // cam CMP pin 8
digitalWrite(CAM_PIN, HIGH);
break;
  case 2:
digitalWrite(CAM_PIN, LOW);
digitalWrite(MPH_PIN, LOW); //VSS
break;
   
  case 12:
digitalWrite(CAM_PIN, HIGH);
break;
  case 13:
digitalWrite(CAM_PIN, LOW);
break;
  case 24:
digitalWrite(CAM_PIN, HIGH);
break;
  case 25:
digitalWrite(CAM_PIN, LOW);
break;
} // end cases


  //hold CKP PIN high for this delay.
  digitalWrite(PULSE_PIN, HIGH);
  delayMicroseconds(PULSE_DURATION);
  // now CKP go low
  digitalWrite(PULSE_PIN, LOW);
  // end function
}


//Simulates a 36 tooth reluctor wheel
//with a  1 tooth reference

// begin main loop program section
void loop()
{
  // read potentiometer wiper pin 0?
  // analog A/D channel 0 
  val = analogRead(POT_PIN);
  pulse_gap = map(val, 0, 1023, 500, 50000);
  // for loop 36 counts  , 150 uS to 1000 uS or  5000 to 800 rpm
  for (int i = 0; i < 11; i++) 
  {
    // go high then low , in Symmetry
      digitalWrite(PULSE_PIN, HIGH);
      delayMicroseconds(PULSE_DURATION);
      digitalWrite(PULSE_PIN, LOW);
      delayMicroseconds( (pulse_gap - PULSE_DURATION) );
  } 
  // simulate the missing tooth next
  delayMicroseconds( (pulse_gap - PULSE_DURATION) );

  } 
// end main loop  version 7 , now perfect  800 rpm to 5000
// added cam pulse 7/4/2013  
// using Delay calls, suck but , this is easy.
// the switch case trick, gets the cam sensor working.
// the engine fires every 120 degr. on crank, 12 teeth, and 10 degr per tooth=120
// added MPH pin, for VSS signals to ECU
