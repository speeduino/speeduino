/*  IFX9201 H-Bridge Motor Control Example for the H-Bridge 2GO Evaluation Board

            PWM Mode ONLY

    NOTES   In PWM mode Duty cycle can be varied in integer steps range 0 to 100
            representing integer percentages

            IFX9201 methods forwards and backwards have two methods e.g.
                forwards( )     is use default PWM of 50%
                forwards( 50 )  is same as above use 50%

        Serial Baud Rate 115200
        
  Remember to define your pins for connections how your board is set if NOT H-Bridge 2GO
    DIR     Direction
    DIS     Disable
    PWM     Pulse Width Modulation (speed)
*/
#include <IFX9201.h>
#include <LED.h>

// Change the DIR, PWM, and DIS pins to custom ones for other boards
// Defined for H-Bridge 2GO
#define DIR		6
#define DIS		10
#define PWM 	11

// IFX9201 Object
IFX9201 IFX9201_HBridge = IFX9201( );

// Create an LED object
LED Led;

void setup( )
{
  // Initialize digital pin LED1 as an output and OFF
  Led.Add( LED1 );
  Led.Off( LED1 );

  Serial.begin( 115200 );

  // Use PWM Mode
  // Change the DIR, PWM, and DIS pins to custom ones for other boards
  IFX9201_HBridge.begin( DIR, PWM, DIS );

  delay( 1000 );
}


void loop( )
{
  Serial.println( "Forwards" );
  IFX9201_HBridge.forwards( 50 );       // Same as forwards( )
  delay( 1000 );

  Serial.println( "Stop" );
  IFX9201_HBridge.stop( );
  delay( 1000 );

  Serial.println( "Backwards" );
  IFX9201_HBridge.backwards( 50 );     // Same as backwards( )
  delay( 1000 );

  Serial.println( "Stop" );
  IFX9201_HBridge.stop( );
  delay( 1000 );

  Led.Toggle( LED1 );
}
