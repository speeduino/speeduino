/* IFX9201 H-Bridge Motor Control Example for the H-Bridge 2GO Evaluation Board

            PWM Mode ONLY With Error trapping and display
            
            Deliberate run time error to be caught in first movement

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

// error display defines stringifying constants
// print constant name as string and EOL
#define str(x)  Serial.println( #x )
// print value of constant as string
#define str1(x) str(x)

// IFX9201 Object
IFX9201 IFX9201_HBridge = IFX9201( );

// Create an LED object
LED Led;

uint8_t err;


/* Display Errors from IFX9201 library calls
   To assist debugging of motor control */
void displayIFX9201Error( uint8_t error )
{
Serial.print( "Error was - " );
switch( err )
    {
    case IFX9201__MODE_ERROR:   str( IFX9201__MODE_ERROR );
                                break;
    case IFX9201__SPI_WRITE_ERROR:
                                str( IFX9201__SPI_WRITE_ERROR );
                                break;
    case IFX9201__ILLEGAL_OPERATION_ERROR:
                                str( IFX9201__ILLEGAL_OPERATION_ERROR );
                                break;
    case IFX9201__ILLEGAL_FREQUENCY:
                                str( IFX9201__ILLEGAL_FREQUENCY );
                                break;
    case IFX9201__ILLEGAL_PWM_PIN:
                                str( IFX9201__ILLEGAL_PWM_PIN );
                                break;
    case IFX9201__ILLEGAL_DUTY_CYCLE:
                                str( IFX9201__ILLEGAL_DUTY_CYCLE );
                                break;
    case IFX9201__ILLEGAL_DUTY_VALUE:
                                str( IFX9201__ILLEGAL_DUTY_VALUE );
                                break;
    default:    Serial.print( "Unknown error code of - " );
                Serial.println( err );
    }
#ifdef XMC_BOARD
Serial.print( "Running on - " );
str1( XMC_BOARD );
#endif
}


void setup( )
{
  // Initialize digital pin LED1 as an output and OFF
  Led.Add( LED1 );
  Led.Off( LED1 );

  Serial.begin( 115200 );

  // Use PWM Mode
  IFX9201_HBridge.begin( DIR, PWM, DIS );

  delay( 1000 );
}


void loop( )
{
  Serial.println( "Forwards" );
  err = IFX9201_HBridge.forwards( 150 );       // Initially invalid
  if( err )
    displayIFX9201Error( err );
  delay( 1000 );

  Serial.println( "Stop" );
  err = IFX9201_HBridge.stop( );
  if( err )
    displayIFX9201Error( err );
  delay( 1000 );

  Serial.println( "Backwards" );
  err = IFX9201_HBridge.backwards( 50 );     // Same as backwards( )
  if( err )
    displayIFX9201Error( err );
  delay( 1000 );

  Serial.println( "Stop" );
  err = IFX9201_HBridge.stop( );
  if( err )
    displayIFX9201Error( err );
  delay( 1000 );

  Led.Toggle( LED1 );
}
