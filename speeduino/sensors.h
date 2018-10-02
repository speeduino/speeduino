#ifndef SENSORS_H
#define SENSORS_H

#include "Arduino.h"

// The following are alpha values for the ADC filters.
// Their values are from 0 to 255 with 0 being no filtering and 255 being maximum
/*
#define ADCFILTER_TPS  128
#define ADCFILTER_CLT  180
#define ADCFILTER_IAT  180
#define ADCFILTER_O2   128
#define ADCFILTER_BAT  128
#define ADCFILTER_MAP   20 //This is only used on Instantaneous MAP readings and is intentionally very weak to allow for faster response
#define ADCFILTER_BARO  64
*/

#define BARO_MIN      87
#define BARO_MAX      108

#define KNOCK_MODE_DIGITAL  1
#define KNOCK_MODE_ANALOG   2

/*
#if defined(CORE_AVR)
  #define ANALOG_ISR
#endif
*/

volatile byte flexCounter = 0;
volatile byte knockCounter = 0;
volatile uint16_t knockAngle;
volatile int AnChannel[15];

unsigned long MAPrunningValue; //Used for tracking either the total of all MAP readings in this cycle (Event average) or the lowest value detected in this cycle (event minimum)
unsigned long EMAPrunningValue; //As above but for EMAP
unsigned int MAPcount; //Number of samples taken in the current MAP cycle
uint32_t MAPcurRev; //Tracks which revolution we're sampling on
bool auxIsEnabled;

//These variables are used for tracking the number of running sensors values that appear to be errors. Once a threshold is reached, the sensor reading will go to default value and assume the sensor is faulty
byte mapErrorCount = 0;
byte iatErrorCount = 0;
byte cltErrorCount = 0;

/*
 * Simple low pass IIR filter macro for the analog inputs
 * This is effectively implementing the smooth filter from http://playground.arduino.cc/Main/Smooth
 * But removes the use of floats and uses 8 bits of fixed precision.
 */
#define ADC_FILTER(input, alpha, prior) (((long)input * (256 - alpha) + ((long)prior * alpha))) >> 8
//These functions all do checks on a pin to determine if it is already in use by another (higher importance) function
#define pinIsInjector(pin)  ( (pin == pinInjector1) || (pin == pinInjector2) || (pin == pinInjector3) || (pin == pinInjector4) )
#define pinIsIgnition(pin)  ( (pin == pinCoil1) || (pin == pinCoil2) || (pin == pinCoil3) || (pin == pinCoil4) )
#define pinIsSensor(pin)    ( (pin == pinCLT) || (pin == pinIAT) || (pin == pinMAP) || (pin == pinTPS) || (pin == pinO2) || (pin == pinBat) )
#define pinIsUsed(pin)      ( pinIsInjector(pin) || pinIsIgnition(pin) || pinIsSensor(pin) )

static inline void instanteneousMAPReading() __attribute__((always_inline));
static inline void readMAP() __attribute__((always_inline));
void initialiseADC();
void readTPS();
void readO2_2();
void flexPulse();
uint16_t readAuxanalog(uint8_t analogPin);
uint16_t readAuxdigital(uint8_t digitalPin);
void readCLT();
void readIAT();
void readO2();
void readBat();
void readBaro();

#if defined(ANALOG_ISR)
//Analog ISR interrupt routine
/*
ISR(ADC_vect)
{
  byte nChannel;
  int result = ADCL | (ADCH << 8);

  //ADCSRA = 0x6E;  // ADC disabled by clearing bit 7(ADEN)
  //BIT_CLEAR(ADCSRA, ADIE);

  nChannel = ADMUX & 0x07;
  #if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__)
    if (nChannel==7) { ADMUX = 0x40; }
  #elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    if(ADCSRB & 0x08) { nChannel += 8; }  //8 to 15
    if(nChannel == 15)
    {
      ADMUX = 0x40; //channel 0
      ADCSRB = 0x00; //clear MUX5 bit
    }
    else if (nChannel == 7) //channel 7
    {
      ADMUX = 0x40;
      ADCSRB = 0x08; //Set MUX5 bit
    }
  #endif
    else { ADMUX++; }
  AnChannel[nChannel-1] = result;

  //BIT_SET(ADCSRA, ADIE);
  //ADCSRA = 0xEE; // ADC Interrupt Flag enabled
}
*/
ISR(ADC_vect)
{
  byte nChannel = ADMUX & 0x07;
  int result = ADCL | (ADCH << 8);

  BIT_CLEAR(ADCSRA, ADEN); //Disable ADC for Changing Channel (see chapter 26.5 of datasheet)

  #if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__)
    if (nChannel==7) { ADMUX = 0x40; }
  #elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    if( (ADCSRB & 0x08) > 0) { nChannel += 8; }  //8 to 15
    if(nChannel == 15)
    {
      ADMUX = 0x40; //channel 0
      ADCSRB = 0x00; //clear MUX5 bit
    }
    else if (nChannel == 7) //channel 7
    {
      ADMUX = 0x40;
      ADCSRB = 0x08; //Set MUX5 bit
    }
  #endif
    else { ADMUX++; }
  AnChannel[nChannel] = result;

  BIT_SET(ADCSRA, ADEN); //Enable ADC
}
#endif

#endif // SENSORS_H
