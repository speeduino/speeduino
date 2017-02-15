#ifndef SENSORS_H
#define SENSORS_H

#include "Arduino.h"


// The following are alpha values for the ADC filters.
// Their values are from 0 to 255 with 0 being no filtering and 255 being maximum
#define ADCFILTER_TPS  128
#define ADCFILTER_CLT  180
#define ADCFILTER_IAT  180
#define ADCFILTER_O2   128
#define ADCFILTER_BAT  128
#define ADCFILTER_MAP   20 //This is only used on Instantaneous MAP readings and is intentionally very weak to allow for faster response

#define BARO_MIN      87
#define BARO_MAX      108

/*
#if defined(CORE_AVR)
  #define ANALOG_ISR
#endif
*/

volatile byte flexCounter = 0;
volatile int AnChannel[15];

unsigned long MAPrunningValue; //Used for tracking either the total of all MAP readings in this cycle (Event average) or the lowest value detected in this cycle (event minimum)
unsigned int MAPcount; //Number of samples taken in the current MAP cycle
byte MAPcurRev; //Tracks which revolution we're sampling on

/*
 * Simple low pass IIR filter macro for the analog inputs
 * This is effectively implementing the smooth filter from http://playground.arduino.cc/Main/Smooth
 * But removes the use of floats and uses 8 bits of fixed precision.
 */
#define ADC_FILTER(input, alpha, prior) (((long)input * (256 - alpha) + ((long)prior * alpha))) >> 8

void instanteneousMAPReading();
void readMAP();
void flexPulse();

unsigned int tempReading;

#if defined(ANALOG_ISR)
//Analog ISR interrupt routine
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
#endif

#endif // SENSORS_H
