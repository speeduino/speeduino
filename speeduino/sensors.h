#ifndef SENSORS_H
#define SENSORS_H

#include "Arduino.h"

// The following are alpha values for the ADC filters.
// Their values are from 0 to 240, with 0 being no filtering and 240 being maximum
#define ADCFILTER_TPS_DEFAULT   50
#define ADCFILTER_CLT_DEFAULT  180
#define ADCFILTER_IAT_DEFAULT  180
#define ADCFILTER_O2_DEFAULT   128
#define ADCFILTER_BAT_DEFAULT  128
#define ADCFILTER_MAP_DEFAULT   20 //This is only used on Instantaneous MAP readings and is intentionally very weak to allow for faster response
#define ADCFILTER_BARO_DEFAULT  64

#define ADCFILTER_PSI_DEFAULT  150 //not currently configurable at runtime, used for misc pressure sensors, oil, fuel, etc.

#define FILTER_FLEX_DEFAULT     75

#define BARO_MIN      65
#define BARO_MAX      108

#define KNOCK_MODE_DIGITAL  1
#define KNOCK_MODE_ANALOG   2

#define VSS_GEAR_HYSTERESIS 10
#define VSS_SAMPLES         4 //Must be a power of 2 and smaller than 255

#define TPS_INTERVAL 39  //[ms]TPS reading interval , (this is to be exactly 39 for TPSdot calculation to be super simple)
#define TPS_READ_FREQUENCY 26 //Hz calculated from interval above and rounded to the nearest integer
#define CLT_INTERVAL 250 //[ms] infrequent CLT readings are not an issue.
#define BARO_INTERVAL 1000 //[ms] infrequent BARO readings are not an issue.


//enum ADCstates:uint8_t {ADCidle,ADCrunning,ADCcomplete};//ADC converter states {ADCidle,ADCrunning,ADCcomplete}
enum ADCstates {ADCidle,ADCrunning,ADCcomplete};//ADC converter states {ADCidle,ADCrunning,ADCcomplete}

/** define board specific ADC conversions functions, etc.
 */
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
#define ADC_start(ulPin) ADC_start_AVR2560(ulPin)
#define ADC_CheckForConversionComplete() ADC_CheckForConversionComplete_AVR2560()
#define ADC_get_value() ADC_get_value_AVR2560()
#elif defined(ARDUINO_ARCH_STM32)
#define ADC_start(ulPin) ADC_start_STM32(ulPin)
#define ADC_CheckForConversionComplete() ADC_CheckForConversionComplete_STM32()
#define ADC_get_value() ADC_get_value_STM32()
#else
// fallback to the analogRead()
uint16_t tempADCreading; //global variable to store the reading between the calls
#define ADC_start(ulPin) (tempADCreading=analogRead(ulPin)) || (1)
#define ADC_CheckForConversionComplete() 1
#define ADC_get_value() tempADCreading
#endif

volatile byte flexCounter = 0;
volatile unsigned long flexStartTime;
volatile unsigned long flexPulseWidth;

#if defined(CORE_AVR)
  #define READ_FLEX() ((*flex_pin_port & flex_pin_mask) ? true : false)
#else
  #define READ_FLEX() digitalRead(pinFlex)
#endif

volatile byte knockCounter = 0;
volatile uint16_t knockAngle;


bool auxIsEnabled;
byte MAPlast; /**< The previous MAP reading */
unsigned long MAP_time; //The time the MAP sample was taken
unsigned long MAPlast_time; //The time the previous MAP sample was taken
volatile unsigned long vssTimes[VSS_SAMPLES] = {0};
volatile byte vssIndex;


//These variables are used for tracking the number of running sensors values that appear to be errors. Once a threshold is reached, the sensor reading will go to default value and assume the sensor is faulty
byte mapErrorCount = 0;
byte iatErrorCount = 0;
byte cltErrorCount = 0;

/**
 * @brief Simple low pass IIR filter macro for the analog inputs
 * This is effectively implementing the smooth filter from playground.arduino.cc/Main/Smooth
 * But removes the use of floats and uses 8 bits of fixed precision.
 */
#define ADC_FILTER(input, alpha, prior) (((long)input * (256 - alpha) + ((long)prior * alpha))) >> 8


static inline void validateMAP();
void initialiseADC();
void initializeAux();    /*Checks the aux inputs and initialises pins if required */
void initializeVSS();    /*Vehicle speed sensor initialization */
void initializeFlex();
void initBaro();         //initialize baro
ADCstates readTPS(bool useFilter, ADCstates adcState); //this is to be called repeatedly //Allows the option to override the use of the filter
void flexPulse();
uint32_t vssGetPulseGap(byte);
void vssPulse();
uint16_t getSpeed();
byte getGear();
uint16_t readAuxanalog(uint8_t analogPin);
uint16_t readAuxdigital(uint8_t digitalPin);
void ADC_sequencer();
void instanteneousMAPReading(uint16_t adcReading);
ADCstates readMAP(ADCstates adcState); //this is to be called repeatedly
ADCstates readEMAP(ADCstates adcState); //this is to be called repeatedly
ADCstates readCLT(bool useFilter,ADCstates adcState); //Allows the option to override the use of the filter
ADCstates readIAT(ADCstates adcState);
ADCstates readO2(ADCstates adcState);
ADCstates readO2_2(ADCstates adcState);
ADCstates readBat(ADCstates adcState);
ADCstates readBaro(ADCstates adcState);
ADCstates readOilpressure(ADCstates adcState);
ADCstates readFuelpressure(ADCstates adcState);


#endif // SENSORS_H
