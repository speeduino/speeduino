#ifndef SENSORS_H
#define SENSORS_H

#include "globals.h"

// The following are alpha values for the ADC filters.
// Their values are from 0 to 240, with 0 being no filtering and 240 being maximum
#define ADCFILTER_TPS_DEFAULT   50U
#define ADCFILTER_CLT_DEFAULT  180U
#define ADCFILTER_IAT_DEFAULT  180U
#define ADCFILTER_O2_DEFAULT   128U
#define ADCFILTER_BAT_DEFAULT  128U
#define ADCFILTER_MAP_DEFAULT   20U //This is only used on Instantaneous MAP readings and is intentionally very weak to allow for faster response
#define ADCFILTER_BARO_DEFAULT  64U

#define ADCFILTER_PSI_DEFAULT  150U //not currently configurable at runtime, used for misc pressure sensors, oil, fuel, etc.

#define FILTER_FLEX_DEFAULT     75U

#define VSS_GEAR_HYSTERESIS 10U
#define VSS_SAMPLES         4U //Must be a power of 2 and smaller than 255

#define TPS_READ_FREQUENCY  30 //ONLY VALID VALUES ARE 15 or 30!!!
#if TPS_READ_FREQUENCY==30
#define TPS_TIMER_BIT BIT_TIMER_30HZ
#elif TPS_READ_FREQUENCY==15
#define TPS_TIMER_BIT BIT_TIMER_15HZ
#else
#error
#endif

extern volatile byte flexCounter;
extern volatile uint32_t flexPulseWidth;

#if defined(CORE_AVR)
  #define READ_FLEX() ((*flex_pin_port & flex_pin_mask) ? true : false)
#else
  #define READ_FLEX() digitalRead(pinFlex)
#endif

#define BIT_SENSORS_AUX_ENBL        0
#define BIT_SENSORS_BARO_SAVED      1
#define BIT_SENSORS_UNUSED2         2
#define BIT_SENSORS_UNUSED3         3
#define BIT_SENSORS_UNUSED4         4
#define BIT_SENSORS_UNUSED5         5
#define BIT_SENSORS_UNUSED6         6
#define BIT_SENSORS_UNUSED7         7
extern uint8_t statusSensors; //Uses the above status bits

void initialiseADC(void);
void readTPS(bool useFilter=true); //Allows the option to override the use of the filter
void readO2_2(void);
void flexPulse(void);
void knockPulse(void);
uint32_t vssGetPulseGap(byte toothHistoryIndex);
void vssPulse(void);
uint16_t getSpeed(void);
byte getGear(void);
byte getFuelPressure(void);
byte getOilPressure(void);
uint16_t readAuxanalog(uint8_t analogPin);
uint16_t readAuxdigital(uint8_t digitalPin);

void readCLT(bool useFilter=true); //Allows the option to override the use of the filter

void readIAT(void);

void readO2(void);

void readBat(void);

void readBaro(void);

/** @brief Initialize the MAP calculation & Baro values */
void initialiseMAPBaro(void);
void resetMAPcycleAndEvent(void);

void readMAP(void);
#define MAP_TIMER_BIT BIT_TIMER_1KHZ

uint8_t getAnalogKnock(void);

/** @brief Get the MAP change between the last 2 readings */
int16_t getMAPDelta(void);

/** @brief Get the time in µS between the last 2 MAP readings */
uint32_t getMAPDeltaTime(void);

extern table2D_u16_u16_32 cltCalibrationTable;
extern table2D_u16_u16_32 iatCalibrationTable;
extern table2D_u16_u8_32 o2CalibrationTable; 

#endif // SENSORS_H
