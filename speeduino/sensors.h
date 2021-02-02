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

#define BARO_MIN      65
#define BARO_MAX      108

#define KNOCK_MODE_DIGITAL  1
#define KNOCK_MODE_ANALOG   2

#define VSS_GEAR_HYSTERESIS 10
#define VSS_SAMPLES         4 //Must be a power of 2 and smaller than 255

#define TPS_READ_FREQUENCY  15 //ONLY VALID VALUES ARE 15 or 30!!!

/*
#if defined(CORE_AVR)
  #define ANALOG_ISR
#endif
*/

volatile uint8_t flexCounter = 0;
volatile unsigned long flexStartTime;
volatile unsigned long flexPulseWidth;

#if defined(CORE_AVR)
  #define READ_FLEX() ((*flex_pin_port & flex_pin_mask) ? true : false)
#else
  #define READ_FLEX() digitalRead(pinFlex)
#endif

volatile uint8_t knockCounter = 0;
volatile uint16_t knockAngle;

unsigned long MAPrunningValue; //Used for tracking either the total of all MAP readings in this cycle (Event average) or the lowest value detected in this cycle (event minimum)
unsigned long EMAPrunningValue; //As above but for EMAP
uint16_t MAPcount; //Number of samples taken in the current MAP cycle
uint32_t MAPcurRev; //Tracks which revolution we're sampling on
bool auxIsEnabled;
uint8_t TPSlast; /**< The previous TPS reading */
unsigned long TPS_time; //The time the TPS sample was taken
unsigned long TPSlast_time; //The time the previous TPS sample was taken
uint8_t MAPlast; /**< The previous MAP reading */
unsigned long MAP_time; //The time the MAP sample was taken
unsigned long MAPlast_time; //The time the previous MAP sample was taken
volatile unsigned long vssLastPulseTime; /**< The time of the last VSS pulse of the VSS */
volatile unsigned long vssLastMinusOnePulseTime; /**< The time of the last VSS_NUM_SAMPLES pulses of the VSS are stored in this array */
volatile unsigned long vssTotalTime; /**< Cumulative count of the last VSS_SAMPLES number of pulses */
volatile uint8_t vssCount;


//These variables are used for tracking the number of running sensors values that appear to be errors. Once a threshold is reached, the sensor reading will go to default value and assume the sensor is faulty
uint8_t mapErrorCount = 0;
uint8_t iatErrorCount = 0;
uint8_t cltErrorCount = 0;

/**
 * @brief Simple low pass IIR filter macro for the analog inputs
 * This is effectively implementing the smooth filter from playground.arduino.cc/Main/Smooth
 * But removes the use of floats and uses 8 bits of fixed precision.
 */
#define ADC_FILTER(input, alpha, prior) (((long)input * (256 - alpha) + ((long)prior * alpha))) >> 8

static inline void instanteneousMAPReading() __attribute__((always_inline));
static inline void readMAP() __attribute__((always_inline));
static inline void validateMAP();
void initialiseADC();
void readTPS(bool=true); //Allows the option to override the use of the filter
void readO2_2();
void flexPulse();
void vssPulse();
uint16_t getSpeed();
uint8_t getGear();
uint8_t getFuelPressure();
uint8_t getOilPressure();
uint16_t readAuxanalog(uint8_t analogPin);
uint16_t readAuxdigital(uint8_t digitalPin);
void readCLT(bool=true); //Allows the option to override the use of the filter
void readIAT();
void readO2();
void readBat();
void readBaro();

#if defined(ANALOG_ISR)
volatile int16_t AnChannel[15];

//Analog ISR interrupt routine
/*
ISR(ADC_vect)
{
  uint8_t nChannel;
  int16_t result = ADCL | (ADCH << 8);

  //ADCSRA = 0x6E; - ADC disabled by clearing bit 7(ADEN)
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
  //ADCSRA = 0xEE; - ADC Interrupt Flag enabled
}
*/
ISR(ADC_vect)
{
  uint8_t nChannel = ADMUX & 0x07;
  int16_t result = ADCL | (ADCH << 8);

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
