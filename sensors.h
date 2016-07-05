// The following are alpha values for the ADC filters. 
// Their values are from 0 to 255 with 0 being no filtering and 255 being maximum
#define ADCFILTER_TPS  128
#define ADCFILTER_CLT  180
#define ADCFILTER_IAT  180
#define ADCFILTER_O2  128
#define ADCFILTER_BAT  128
#define ADCFILTER_FLEX 128

/*
 * Simple low pass IIR filter macro for the analog inputs
 * This is effectively implementing the smooth filter from http://playground.arduino.cc/Main/Smooth
 * But removes the use of floats and uses 8 bits of fixed precision. 
 */
#define ADC_FILTER(input, alpha, prior) (((long)input * (256 - alpha) + ((long)prior * alpha))) >> 8

void instanteneousMAPReading();
void readMAP();
