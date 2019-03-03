
//Handy bitsetting macros
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_CHECK(var,pos) !!((var) & (1<<(pos)))

#define pinMAP1 A0
#define pinMAP2 A1
#define pinMAP3 A2
#define pinMAP4 A3

#define pinChipSelect 10 //Almost certainly not right

//Calibration values for the MPX4250 sensor
#define MPX2450_min 10
#define MPX2450_max 260

byte *cs_pin_port;
byte cs_pin_mask;

uint16_t loopCount = 0;
volatile uint16_t loopsPerSecond = 0;

volatile byte loop33ms;
volatile byte loop66ms;
volatile byte loop100ms;
volatile byte loop250ms;
volatile int loopSec;
uint16_t outputValue;

#define CS_PIN_LOW()  *cs_pin_port &= ~(cs_pin_mask)
#define CS_PIN_HIGH() *cs_pin_port |= (cs_pin_mask)

#define fastMap10BitX10(x, out_min, out_max) ( ( ((unsigned long)x * 10 * (out_max-out_min)) >> 10 ) + (out_min * 10))
#define fastMap10BitX8(x, out_min, out_max) ( ( ((unsigned long)x * 8 * (out_max-out_min)) >> 10 ) + (out_min * 8))
#define map1(x, in_min, in_max, out_min, out_max)  ( (unsigned long)x * out_max / in_max )
void setDAC();

//Interrupt for timer. Fires every 1ms
ISR(TIMER2_OVF_vect, ISR_NOBLOCK)
{
  //Increment Loop Counters
  loop33ms++;
  loop66ms++;
  loop100ms++;
  loop250ms++;
  loopSec++;

  //30Hz loop
  if (loop33ms == 33)
  {
    loop33ms = 0;
  }

  //15Hz loop
  if (loop66ms == 66)
  {
    loop66ms = 0;
  }

  //Loop executed every 100ms loop
  //Anything inside this if statement will run every 100ms.
  if (loop100ms == 100)
  {
    loop100ms = 0; //Reset counter
  }

  //Loop executed every 250ms loop (1ms x 250 = 250ms)
  //Anything inside this if statement will run every 250ms.
  if (loop250ms == 250)
  {
    loop250ms = 0; //Reset Counter
  }

  //Loop executed every 1 second (1ms x 1000 = 1000ms)
  if (loopSec == 1000)
  {
    loopSec = 0;
    loopsPerSecond = loopCount;
    loopCount = 0;
  }
  
  TCNT2  = 131;           //Preload timer2 with 131 cycles, leaving 125 till overflow. As the timer runs at 125Khz, this causes overflow to occur at 1Khz = 1ms
}

void SPIasyncTransfer(byte data1, byte data0)
{
  //Check whether the last send is still in progress
  if( SPSR & _BV(SPIF) )
  {
    //This means the last transfer has completed and we're safe to do the next one
    SPDR = data1;
  }
  else
  {
    //The last transfer is still in progress
    //Can possibly implement a buffer here, but for now, just do nothing
  }
}

