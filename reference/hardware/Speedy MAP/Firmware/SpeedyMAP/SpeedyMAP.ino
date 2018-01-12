#include <SPI.h>
#include <Arduino.h>
#include "globals.h"
#include "comms.h"

uint16_t map1_adc, map2_adc, map3_adc, map4_adc;
uint16_t map5_adc, map6_adc, map7_adc, map8_adc;
uint16_t currentMAP, map1, map2, map3, map4; //These values are all stored in kPa x 8 for 1 point of extra precision. They are the instantaneous values
uint16_t map1_min, map2_min, map3_min, map4_min; //As above, but represent the minimum reading for each sensor within the current cycle

byte currentLowestCylinder;
uint16_t cycle_count = 0;
unsigned long cycleStartTime;

bool serialStream = false;

void setup() {
  //This sets the ADC (Analog to Digitial Converter) to run at 1Mhz, greatly reducing analog read times (MAP/TPS) when using the standard analogRead() function
  //1Mhz is the fastest speed permitted by the CPU without affecting accuracy
  //Please see chapter 11 of 'Practical Arduino' (http://books.google.com.au/books?id=HsTxON1L6D4C&printsec=frontcover#v=onepage&q&f=false) for more detail
  BIT_SET(ADCSRA,ADPS2);
  BIT_CLEAR(ADCSRA,ADPS1);
  BIT_CLEAR(ADCSRA,ADPS0);

  Serial.begin(115200);

  //Setup for the MCP4921 SPI
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4); //Probably need to speed this up
  pinMode(pinChipSelect, OUTPUT);
  digitalWrite(pinChipSelect, HIGH);

  //Configure Timer2 for our 1ms interrupt code.
  TCCR2B = 0x00;          //Disbale Timer2 while we set it up
  TCNT2  = 131;           //Preload timer2 with 131 cycles, leaving 125 till overflow. As the timer runs at 125Khz, this causes overflow to occur at 1Khz = 1ms
  TIFR2  = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
  TIMSK2 = 0x01;          //Timer2 Set Overflow Interrupt enabled.
  TCCR2A = 0x00;          //Timer2 Control Reg A: Wave Gen Mode normal
  /* Now configure the prescaler to CPU clock divided by 128 = 125Khz */
  TCCR2B |= (1<<CS22)  | (1<<CS20); // Set bits
  TCCR2B &= ~(1<<CS21);             // Clear bit

  //Set the port and mask for the cable select pin
  cs_pin_port = portOutputRegister(digitalPinToPort(pinChipSelect));
  cs_pin_mask = digitalPinToBitMask(pinChipSelect);

  cycleStartTime = micros();
}

void loop() 
{
  loopCount++;

  if(Serial.available() >= SERIAL_BUFFER_THRESHOLD) { command(); }
  if(serialStream == true && Serial.availableForWrite() ) { sendCSV(); } //If serialStream is enabled and the serial write buffer isn't full, the current map values are sent continually as CSV

  //Read each of the 4 sensors and map them using the calibration values (Results in map1 value in kPa etc)
  map1_adc = analogRead(pinMAP1);
  map1_adc = analogRead(pinMAP1);
  map1 = fastMap10BitX8(map1_adc, MPX2450_min, MPX2450_max);

  map2_adc = analogRead(pinMAP2);
  map2_adc = analogRead(pinMAP2);
  map2 = fastMap10BitX8(map2_adc, MPX2450_min, MPX2450_max);

  map3_adc = analogRead(pinMAP3);
  map3_adc = analogRead(pinMAP3);
  map3 = fastMap10BitX8(map3_adc, MPX2450_min, MPX2450_max);

  map4_adc = analogRead(pinMAP4);
  map4_adc = analogRead(pinMAP4);
  map4 = fastMap10BitX8(map4_adc, MPX2450_min, MPX2450_max);

  //Find the lowest current value
  byte tempLowestCylinder = 1;
  currentMAP = map1;
  if(map2 < currentMAP) { currentMAP = map2; tempLowestCylinder = 2; }
  if(map3 < currentMAP) { currentMAP = map3; tempLowestCylinder = 3; }
  if(map4 < currentMAP) { currentMAP = map4; tempLowestCylinder = 4; }

  //Check if we're starting a new cycle yet
  //This is determined to be when sensor 1 has the lowest reading, but only if the previous lowest reading was on another cylinder
  //Note that this is only really accurate for determining that a new cycle has started. RPM readings based on it will bounce around by a few 100
  if( tempLowestCylinder == 1 && currentLowestCylinder != 1)
  {
    cycle_count++;
    currentLowestCylinder = tempLowestCylinder;
    cycleStartTime = micros();
  }
  

  //Set the DAC output value from the above
  setDAC();
}

static inline void setDAC()
{
  
  byte outputValueByte0, outputValueByte1;
  
  outputValue = map(currentMAP, 0, 2048, 0, 4095); //The MAP readings have been multiplied by 8 for better resolution. The 2048 brings them back into an 8 bit (256) range equivalent
  outputValueByte0 = byte(outputValue);
  outputValue = outputValue >> 8;
  outputValueByte1 = byte(outputValue | 0b00110000); //Combines the remaining part of the 
  
  CS_PIN_LOW();
  SPI.transfer(outputValueByte1);
  SPI.transfer(outputValueByte0);
  CS_PIN_HIGH();
}
