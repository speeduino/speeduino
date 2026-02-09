/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * Read sensors with appropriate timing / scheduling.
 */

#include "sensors.h"
#include "crankMaths.h"
#include "globals.h"
#include "maths.h"
#include "storage.h"
#include "comms.h"
#include "idle.h"
#include "corrections.h"
#include "pages.h"
#include "decoders.h"
#include "auxiliaries.h"
#include "utilities.h"
#include "unit_testing.h"
#include "sensors_map_structs.h"
#include "units.h"
#include "atomic.h"
#include "board_definition.h"
#include "preprocessor.h"
#include "static_for.hpp"
#include "polling.hpp"

uint8_t statusSensors = 0;

static volatile uint32_t vssTimes[VSS_SAMPLES] = {0};
static volatile uint8_t vssIndex = 0U;

volatile uint8_t flexCounter = 0U;
static volatile uint32_t flexStartTime = 0UL;
volatile uint32_t flexPulseWidth = 0U;

static map_algorithm_t mapAlgorithmState;

static uint16_t cltCalibration_bins[32];
static uint16_t cltCalibration_values[32];
table2D_u16_u16_32 cltCalibrationTable(&cltCalibration_bins, &cltCalibration_values);
static uint16_t iatCalibration_bins[32];
static uint16_t iatCalibration_values[32];
table2D_u16_u16_32 iatCalibrationTable(&iatCalibration_bins, &iatCalibration_values);
static uint16_t o2Calibration_bins[32];
static uint8_t o2Calibration_values[32];
table2D_u16_u8_32 o2CalibrationTable(&o2Calibration_bins, &o2Calibration_values); 

/**
 * @brief A specialist function to map a value in the range [0, 1023] (I.e. 10-bit) to a different range.
 * 
 * Mostly used for analog input voltage level to real world value conversions.
 * 
 * @details
 * analogRead returns a number in the range [0, 1023], representing the pin input 
 * voltage from min to max (typically 0V - 5V)
 * We need to convert that value to the real world value the sensor is reading (pressure, temperature etc.)
 * If:
 *    * rangeMin is the real world value when the sensor is reading 0V
 *    * rangeMax is the real world measurement when the sensor is reading 5V
 *    * There is a linear relationship between voltage output and the real world value.
 * 
 * then this function will return the real world measurement (kPa, °C etc)
 * 
 * @param value Value to map (should be in range [0, 1023])
 * @param rangeMin Minimum of the output range
 * @param rangeMax Maximum of the output range
 * @return int16_t 
 */
TESTABLE_INLINE_STATIC int16_t fastMap10Bit(uint16_t value, int16_t rangeMin, int16_t rangeMax) 
{
  uint16_t range = rangeMax-rangeMin; // Must be positive (assuming rangeMax>=rangeMin)
  uint16_t fromStartOfRange = (uint16_t)rshift<10>((uint32_t)value * range);
  return rangeMin + (int16_t)fromStartOfRange;
}

//
static inline uint16_t readAnalogPin(uint8_t pin) 
{
  // Why do we read twice? Who knows.....
  analogRead(pin);
  // According to the docs, analogRead result should be in range 0-1023
  // Clip the result to zero minimum to prevent rollover just in case
  int tmp = analogRead(pin);
  // max is a macro on some platforms - DO NOT place the call to analogRead as an inline parameter:
  // (you might end up calling it twice)
  return max(0, tmp);
}


#if defined(ANALOG_ISR)
static volatile uint16_t AnChannel[16];
static inline uint16_t readAnalogSensor(uint8_t pin) {
  return AnChannel[pin-A0];
}
static inline uint16_t readMAPSensor(uint8_t pin) {
#if defined(ANALOG_ISR_MAP)
  return AnChannel[pin-A0];
#else
  return readAnalogPin(pin);
#endif
}
#define ADMUX_DEFAULT_CONFIG  0x40 //AVCC reference, ADC0 input, right adjusted, ADC enabled

ISR(ADC_vect)
{
  byte nChannel = (ADMUX & 0x07);

  byte result_low = ADCL;
  byte result_high = ADCH;

  #if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__)
    if (nChannel == 7U) { ADMUX = 0x40; }
  #elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    if( BIT_CHECK(ADCSRB, MUX5) ) { nChannel += 8; }  //8 to 15
    if(nChannel == 15U)
    {
      ADMUX = ADMUX_DEFAULT_CONFIG; //channel 0
      ADCSRB = 0x00; //clear MUX5 bit

      BIT_CLEAR(ADCSRA,ADIE); //Disable interrupt as we're at the end of a full ADC cycle. This will be re-enabled in the main loop
    }
    else if (nChannel == 7U) //channel 7
    {
      ADMUX = ADMUX_DEFAULT_CONFIG;
      ADCSRB = 0x08; //Set MUX5 bit
    }
  #endif
    else { ADMUX++; }

  //ADMUX always appears to be one ahead of the actual channel value that is in ADCL/ADCH. Subtract 1 from it to get the correct channel number
  if(nChannel == 0U) { nChannel = 16;} 
  AnChannel[nChannel-1] = (result_high << 8) | result_low;
}
#else
static inline uint16_t readAnalogSensor(uint8_t pin) {
  return readAnalogPin(pin);
}
static inline uint16_t readMAPSensor(uint8_t pin) {
  return readAnalogPin(pin);
}
#endif

/** Init all ADC conversions by setting resolutions, etc.
 */
void initialiseADC(void)
{
#ifdef CORE_AVR

  #if defined(ANALOG_ISR)
    noInterrupts(); //Interrupts should be turned off when playing with any of these registers

    ADCSRB = 0x00; //ADC Auto Trigger Source is in Free Running mode

    ADMUX = ADMUX_DEFAULT_CONFIG;  //Select AVCC as reference, ADC Right Adjust Result, Starting at channel 0

    //All of the below is the longhand version of: ADCSRA = 0xEE;
    #ifndef ADFR
      #define ADFR 5 //Looks like this is now defined. Retain this for compatibility with earlier versions of Arduino IDE that did not have this.
    #endif
    BIT_SET(ADCSRA,ADFR); //Set free running mode
    BIT_SET(ADCSRA,ADIE); //Set ADC interrupt enabled
    BIT_CLEAR(ADCSRA,ADIF); //Clear interrupt flag

    // Set ADC clock to 125KHz (Prescaler = 128)
    BIT_SET(ADCSRA,ADPS2);
    BIT_SET(ADCSRA,ADPS1);
    BIT_SET(ADCSRA,ADPS0);

    BIT_SET(ADCSRA,ADEN); //Enable ADC

    interrupts();
    BIT_SET(ADCSRA,ADSC); //Start conversion

  #else
    //This sets the ADC (Analog to Digital Converter) to run at 1Mhz, greatly reducing analog read times (MAP/TPS) when using the standard analogRead() function
    //1Mhz is the fastest speed permitted by the CPU without affecting accuracy
    //Please see chapter 11 of 'Practical Arduino' (books.google.com.au/books?id=HsTxON1L6D4C&printsec=frontcover#v=onepage&q&f=false) for more detail
     BIT_SET(ADCSRA,ADPS2);
     BIT_CLEAR(ADCSRA,ADPS1);
     BIT_CLEAR(ADCSRA,ADPS0);
  #endif
#elif defined(ARDUINO_ARCH_STM32) //STM32GENERIC core and ST STM32duino core, change analog read to 12 bit
  analogReadResolution(10); //use 10bits for analog reading on STM32 boards
#endif

  //The following checks the aux inputs and initialises pins if required
  BIT_CLEAR(statusSensors, BIT_SENSORS_AUX_ENBL);
  for (uint8_t AuxinChan = 0U; AuxinChan <16U ; AuxinChan++)
  {
    currentStatus.current_caninchannel = AuxinChan;                   
    if (((configPage9.caninput_sel[currentStatus.current_caninchannel]&12U) == 4U)
    && ((configPage9.enable_secondarySerial == 1U) || ((configPage9.enable_intcan == 1U) && (configPage9.intcan_available == 1U))))
    { //if current input channel is enabled as external input in caninput_selxb(bits 2:3) and secondary serial or internal canbus is enabled(and is mcu supported)                 
      //currentStatus.canin[14] = 22;  Dev test use only!
      BIT_SET(statusSensors, BIT_SENSORS_AUX_ENBL);
    }
    else if ((((configPage9.enable_secondarySerial == 1U) || ((configPage9.enable_intcan == 1U) && (configPage9.intcan_available == 1U))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12U) == 8U)
            || (((configPage9.enable_secondarySerial == 0U) && ( (configPage9.enable_intcan == 1U) && (configPage9.intcan_available == 0U) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3U) == 2U)  
            || (((configPage9.enable_secondarySerial == 0U) && (configPage9.enable_intcan == 0U)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3U) == 2U)))  
    {  //if current input channel is enabled as analog local pin check caninput_selxb(bits 2:3) with &12 and caninput_selxa(bits 0:1) with &3
      uint8_t pinNumber = pinTranslateAnalog(configPage9.Auxinpina[currentStatus.current_caninchannel]&63U);
      if( pinIsUsed(pinNumber) )
      {
        //Do nothing here as the pin is already in use.
        currentStatus.engineProtectIoError = true; //Tell user that there is problem by lighting up the I/O error indicator
      }
      else
      {
        //Channel is active and analog
        pinMode( pinNumber, INPUT);
        //currentStatus.canin[14] = 33;  Dev test use only!
        BIT_SET(statusSensors, BIT_SENSORS_AUX_ENBL);
      }  
    }
    else if ((((configPage9.enable_secondarySerial == 1U) || ((configPage9.enable_intcan == 1U) && (configPage9.intcan_available == 1U))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12U) == 12U)
            || (((configPage9.enable_secondarySerial == 0U) && ( (configPage9.enable_intcan == 1U) && (configPage9.intcan_available == 0U) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3U) == 3U)
            || (((configPage9.enable_secondarySerial == 0U) && (configPage9.enable_intcan == 0U)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3U) == 3U)))
    {  //if current input channel is enabled as digital local pin check caninput_selxb(bits 2:3) with &12 and caninput_selxa(bits 0:1) with &3
       uint8_t pinNumber = (configPage9.Auxinpinb[currentStatus.current_caninchannel]&63U) + 1U;
       if( pinIsUsed(pinNumber) )
       {
          //Do nothing here as the pin is already in use.
          currentStatus.engineProtectIoError = true; //Tell user that there is problem by lighting up the I/O error indicator
       }
       else
       {
         //Channel is active and digital
         pinMode( pinNumber, INPUT);
         //currentStatus.canin[14] = 44;  Dev test use only!
         BIT_SET(statusSensors, BIT_SENSORS_AUX_ENBL);
       }  
    }
    else {
      //  Do nothing. Keep MISRA checker happy
    }
  } //For loop iterating through aux in lines
  

  //Sanity checks to ensure none of the filter values are set above 240 (Which would include the 255 value which is the default on a new arduino)
  //If an invalid value is detected, it's reset to the default the value and burned to EEPROM. 
  //Each sensor has it's own default value
  if(configPage4.ADCFILTER_TPS  > 240U) { configPage4.ADCFILTER_TPS   = ADCFILTER_TPS_DEFAULT;   writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_CLT  > 240U) { configPage4.ADCFILTER_CLT   = ADCFILTER_CLT_DEFAULT;   writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_IAT  > 240U) { configPage4.ADCFILTER_IAT   = ADCFILTER_IAT_DEFAULT;   writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_O2   > 240U) { configPage4.ADCFILTER_O2    = ADCFILTER_O2_DEFAULT;    writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_BAT  > 240U) { configPage4.ADCFILTER_BAT   = ADCFILTER_BAT_DEFAULT;   writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_MAP  > 240U) { configPage4.ADCFILTER_MAP   = ADCFILTER_MAP_DEFAULT;   writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_BARO > 240U) { configPage4.ADCFILTER_BARO  = ADCFILTER_BARO_DEFAULT;  writeConfig(ignSetPage); }
  if(configPage4.FILTER_FLEX    > 240U) { configPage4.FILTER_FLEX     = FILTER_FLEX_DEFAULT;     writeConfig(ignSetPage); }

  flexStartTime = micros();

  vssIndex = 0;
}


// ==========================================  MAP ==========================================

static constexpr uint16_t VALID_MAP_MAX=1022U; //The largest ADC value that is valid for the MAP sensor
static constexpr uint16_t VALID_MAP_MIN=2U; //The smallest ADC value that is valid for the MAP sensor

TESTABLE_INLINE_STATIC bool instanteneousMAPReading(void)
{
  // All we need to do it signal that the new readings should be used as-is
  return true;
}

static inline bool cycleAverageMAPReadingAccumulate(map_cycle_average_t &cycle_average, const map_adc_readings_t &sensorReadings) {
  ++cycle_average.sampleCount;
  cycle_average.mapAdcRunningTotal += sensorReadings.mapADC; 
  cycle_average.emapAdcRunningTotal += sensorReadings.emapADC;

  // We are *not* ready to derive new maps readings yet
  return false;
}

static inline void reset(const statuses &current, map_cycle_average_t &cycle_average, const map_adc_readings_t &sensorReadings) {
  cycle_average.sampleCount = 0U;
  cycle_average.mapAdcRunningTotal = 0U;
  cycle_average.emapAdcRunningTotal = 0U;
  cycle_average.cycleStartIndex = (uint8_t)current.startRevolutions;
  (void)cycleAverageMAPReadingAccumulate(cycle_average, sensorReadings);
}

static inline bool cycleAverageEndCycle(const statuses &current, map_cycle_average_t &cycle_average, map_adc_readings_t &sensorReadings) {
  if( (cycle_average.mapAdcRunningTotal != 0UL) && (cycle_average.sampleCount != 0U) )
  {
    // Record this since we're about to overwrite it....
    map_adc_readings_t rawReadings = sensorReadings;

    sensorReadings.mapADC = fast_div32_16((uint32_t)cycle_average.mapAdcRunningTotal, cycle_average.sampleCount);

    //If EMAP is enabled, the process is identical to the above
    if(sensorReadings.emapADC!=UINT16_MAX)
    {
      sensorReadings.emapADC = fast_div32_16((uint32_t)cycle_average.emapAdcRunningTotal, cycle_average.sampleCount); //Note that the MAP count can be reused here as it will always be the same count.
    }
    reset(current, cycle_average, rawReadings);
    // We can now derive new map values
    return true;
  }
  
  reset(current, cycle_average, sensorReadings);
  return instanteneousMAPReading(); 
}

static inline bool isCycleCurrent(const statuses &current, uint32_t cycleStartIndex) {
  ATOMIC() {
    return (cycleStartIndex == (uint8_t)current.startRevolutions) || ((cycleStartIndex+1U) == (uint8_t)current.startRevolutions);
  }
  return false; // Just here to avoid compiler warning.
}

static inline bool isCycleCurrent(const statuses &current, const map_cycle_average_t &cycle_avg) {
  return isCycleCurrent(current, cycle_avg.cycleStartIndex);
}

TESTABLE_INLINE_STATIC bool canUseCycleAverage(const statuses &current, const config2 &page2, const decoder_status_t&decoderStatus) {
  ATOMIC() {
    return (current.RPMdiv100 > page2.mapSwitchPoint) && decoderStatus.syncStatus!=SyncStatus::None && (current.startRevolutions > 1U);
  }
  return false; // Just here to avoid compiler warning.
}

TESTABLE_INLINE_STATIC bool cycleAverageMAPReading(const statuses &current, const config2 &page2, const decoder_status_t &decoderStatus,  map_cycle_average_t &cycle_average, map_adc_readings_t &sensorReadings) {
  if ( canUseCycleAverage(current, page2, decoderStatus) )
  {
    //2 revolutions are looked at for 4 stroke. 2 stroke not currently catered for.
    if( isCycleCurrent(current, cycle_average) ) {
      return cycleAverageMAPReadingAccumulate(cycle_average, sensorReadings);
    }
    // Reaching here means that the last cycle has completed and the MAP value should be calculated
    return cycleAverageEndCycle(current, cycle_average, sensorReadings);
  }
  
  //If the engine isn't running and RPM below switch point, fall back to instantaneous reads
  reset(current, cycle_average, sensorReadings);
  return instanteneousMAPReading();
}

static inline bool cycleMinimumAccumulate(map_cycle_min_t &cycle_min, const map_adc_readings_t &sensorReadings) {
  //Check whether the current reading is lower than the running minimum
  cycle_min.mapMinimum = min(sensorReadings.mapADC, cycle_min.mapMinimum); 

  // We are *not* ready to derive new maps readings yet
  return false;
}

static inline void reset(const statuses &current, map_cycle_min_t &cycle_min, const map_adc_readings_t &sensorReadings) {
  cycle_min.cycleStartIndex = (uint8_t)current.startRevolutions; //Reset the current rev count
  cycle_min.mapMinimum = sensorReadings.mapADC; //Reset the latest value so the next reading will always be lower
}

static inline bool cycleMinimumEndCycle(const statuses &current, map_cycle_min_t &cycle_min, map_adc_readings_t &sensorReadings) {
  // Record this since we're about to overwrite it....
  map_adc_readings_t rawReadings = sensorReadings;

  sensorReadings.mapADC = cycle_min.mapMinimum;

  reset(current, cycle_min, rawReadings);

  // We can now derive new map values
  return true;
}

static inline bool isCycleCurrent(const statuses &current, const map_cycle_min_t &cycle_min) {
  return isCycleCurrent(current, cycle_min.cycleStartIndex);
}

TESTABLE_INLINE_STATIC bool cycleMinimumMAPReading(const statuses &current, const config2 &page2, map_cycle_min_t &cycle_min, map_adc_readings_t &sensorReadings) {
  if (current.RPMdiv100 > page2.mapSwitchPoint) {
    //2 revolutions are looked at for 4 stroke. 2 stroke not currently catered for.
    if ( isCycleCurrent(current, cycle_min) ) {
      return cycleMinimumAccumulate(cycle_min, sensorReadings);
    }
      //Reaching here means that the last cycle has completed and the MAP value should be calculated
    return cycleMinimumEndCycle(current, cycle_min, sensorReadings);
  }

  //If the engine isn't running and RPM below switch point, fall back to instantaneous reads
  reset(current, cycle_min, sensorReadings);
  return instanteneousMAPReading();
}

static inline bool eventAverageAccumulate(map_event_average_t &eventAverage, const map_adc_readings_t &sensorReadings) {
  eventAverage.mapAdcRunningTotal += sensorReadings.mapADC; //Add the current reading onto the total
  ++eventAverage.sampleCount;

  // We are *not* ready to derive new maps readings yet
  return false;
}

static inline bool isIgnitionEventValid(const map_event_average_t &eventAverage) {
  ATOMIC() {
    return (eventAverage.eventStartIndex < (uint8_t)ignitionCount);
  }
  return false; // Just here to avoid compiler warning.
}

static inline void reset(map_event_average_t &eventAverage, const map_adc_readings_t &sensorReadings) {
  // Reset for next cycle.
  eventAverage.mapAdcRunningTotal = 0U;
  eventAverage.sampleCount = 0U;
  eventAverage.eventStartIndex = (uint8_t)ignitionCount;
  (void)eventAverageAccumulate(eventAverage, sensorReadings);
}

static inline bool eventAverageEndEvent(map_event_average_t &eventAverage, map_adc_readings_t &sensorReadings) {
  //Sanity check
  if( (eventAverage.mapAdcRunningTotal != 0U) && (eventAverage.sampleCount != 0U) && isIgnitionEventValid(eventAverage) )
  {
    // Record this since we're about to overwrite it....
    map_adc_readings_t rawReadings = sensorReadings;
    sensorReadings.mapADC = fast_div32_16((uint32_t)eventAverage.mapAdcRunningTotal, eventAverage.sampleCount);
    reset(eventAverage, rawReadings);
    // We can now derive new map values
    return true;
  }
  
  reset(eventAverage, sensorReadings);
  return instanteneousMAPReading(); 
}

static inline bool isIgnitionEventCurrent(const map_event_average_t &eventAverage) {
  ATOMIC() {
    return (eventAverage.eventStartIndex == (uint8_t)ignitionCount);
  }
  return false; // Just here to avoid compiler warning.
}


TESTABLE_INLINE_STATIC bool canUseEventAverage(const statuses &current, const config2 &page2, const decoder_status_t &decoderStatus) {
  ATOMIC() {
    return (current.RPMdiv100 > page2.mapSwitchPoint) && decoderStatus.syncStatus!=SyncStatus::None && (current.startRevolutions > 1U) && (!isEngineProtectActive(current));
  }
  return false; // Just here to avoid compiler warning.
}

TESTABLE_INLINE_STATIC bool eventAverageMAPReading(const statuses &current, const config2 &page2, const decoder_status_t &decoderStatus, map_event_average_t &eventAverage, map_adc_readings_t &sensorReadings) {
  //Average of an ignition event
  if ( canUseEventAverage(current, page2, decoderStatus) ) //If the engine isn't running, fall back to instantaneous reads
  {
    if( isIgnitionEventCurrent(eventAverage) ) { //Watch for a change in the ignition counter to determine whether we're still on the same event
      return eventAverageAccumulate(eventAverage, sensorReadings);
    }
    
    //Reaching here means that the next ignition event has occurred and the MAP value should be calculated
    return eventAverageEndEvent(eventAverage, sensorReadings);
  }

  reset(eventAverage, sensorReadings);
  return instanteneousMAPReading();
}

static inline bool isValidMapSensorReading(uint16_t reading) {
  return (reading < VALID_MAP_MAX) && (reading > VALID_MAP_MIN);  
}

TESTABLE_INLINE_STATIC uint16_t validateFilterMapSensorReading(uint16_t reading, uint8_t alpha, uint16_t prior) {
  //Error check
  if (isValidMapSensorReading(reading)) { 
    return LOW_PASS_FILTER(reading, alpha, prior);
  }
  return prior;
}

static inline uint16_t readFilteredMapADC(uint8_t pin, uint8_t alpha, uint16_t prior) {
  return validateFilterMapSensorReading(readMAPSensor(pin), alpha, prior);
}

static inline map_adc_readings_t readMapSensors(const map_adc_readings_t &previousReadings, const config4 &page4, bool useEMAP) {
  return {
    readFilteredMapADC(pinMAP, page4.ADCFILTER_MAP, previousReadings.mapADC),
    (uint16_t)(useEMAP ? readFilteredMapADC(pinEMAP, page4.ADCFILTER_MAP, previousReadings.emapADC) : UINT16_MAX)
  };
}

TESTABLE_INLINE_STATIC void storeLastMAPReadings(uint32_t currTime, map_last_read_t &lastRead, uint16_t oldMAPValue) 
{
  //Update the calculation times and last value. These are used by the MAP based Accel enrich
  lastRead.lastMAPValue = oldMAPValue;
  // lastRead.lastReadingTime = lastRead.currentReadingTime;
  lastRead.timeDeltaReadings = currTime - lastRead.currentReadingTime;
  lastRead.currentReadingTime = currTime;
}

static inline uint16_t mapADCToMAP(uint16_t mapADC, int8_t mapMin, uint16_t mapMax) 
{
  int16_t mapped = fastMap10Bit(mapADC, mapMin, mapMax); //Get the current MAP value
  return max((int16_t)0, mapped);  //Sanity check
}

TESTABLE_INLINE_STATIC void setMAPValuesFromReadings(const map_adc_readings_t &readings, const config2 &page2, bool useEMAP, statuses &current) 
{
  current.MAP = mapADCToMAP(readings.mapADC, page2.mapMin, page2.mapMax); //Get the current MAP value
  //Repeat for EMAP if it's enabled
  if(useEMAP) { current.EMAP = mapADCToMAP(readings.emapADC, page2.EMAPMin, page2.EMAPMax); }
}

#if defined(UNIT_TEST)
map_last_read_t& getMapLast(void){
  return mapAlgorithmState.lastReading;
}
#endif

TESTABLE_INLINE_STATIC bool applyMapAlgorithm(const config2 &page2, 
                                              const statuses &current, 
                                              const decoder_status_t &decoderStatus,
                                              map_algorithm_t &algorithmState)
{
  bool readingIsValid;
  switch(page2.mapSample)
  {
    case MAPSamplingCycleAverage:
      readingIsValid = cycleAverageMAPReading(current, page2, decoderStatus, algorithmState.cycle_average, algorithmState.sensorReadings);
      break;

    case MAPSamplingCycleMinimum:
      readingIsValid = cycleMinimumMAPReading(current, page2, algorithmState.cycle_min, algorithmState.sensorReadings);
      break;

    case MAPSamplingIgnitionEventAverage:
      readingIsValid = eventAverageMAPReading(current, page2, decoderStatus, algorithmState.event_average, algorithmState.sensorReadings);
      break; 

    case MAPSamplingInstantaneous:
    default:
      readingIsValid = instanteneousMAPReading();
      break;
  }
  return readingIsValid;
}

static inline void readMAP(void)
{
  // Read sensor(s). Saves filtered ADC readings. Does not set calibrated MAP and EMAP values.
  mapAlgorithmState.sensorReadings = readMapSensors(mapAlgorithmState.sensorReadings, configPage4, configPage6.useEMAP);

  bool readingIsValid = applyMapAlgorithm(configPage2, currentStatus, getDecoderStatus(), mapAlgorithmState);

  // Process sensor readings according to user chosen sampling algorithm
  if(readingIsValid) 
  {
    // Roll over the last reading
    storeLastMAPReadings(micros(), mapAlgorithmState.lastReading, currentStatus.MAP);

    // Convert from filtered sensor readings to kPa
    setMAPValuesFromReadings(mapAlgorithmState.sensorReadings, configPage2, configPage6.useEMAP, currentStatus);
  }
}

/** @brief Get the MAP change between the last 2 readings */
int16_t getMAPDelta(void) {
  return (int16_t)currentStatus.MAP - (int16_t)mapAlgorithmState.lastReading.lastMAPValue;
}

/** @brief Get the time in µS between the last 2 MAP readings */
uint32_t getMAPDeltaTime(void) {
  return mapAlgorithmState.lastReading.timeDeltaReadings;
}

// ========================================== TPS ==========================================

static inline bool isCTPSSensorActive(void) {
  if(configPage2.CTPSPolarity == 0U) { return !digitalRead(pinCTPS); } //Normal mode (ground switched)
  return digitalRead(pinCTPS); //Inverted mode (5v activates closed throttle position sensor)
}

static inline void readTPS(uint8_t tpsADC)
{
  currentStatus.TPSlast = currentStatus.TPS;
  currentStatus.tpsADC = tpsADC;

  uint8_t tempTPSMax = configPage2.tpsMax;
  uint8_t tempTPSMin = configPage2.tpsMin;
  if(configPage2.tpsMin > configPage2.tpsMax)
  {
    //This case occurs when the TPS +5v and gnd are wired backwards, but the user wishes to retain this configuration.
    //In such a case, tpsMin will be greater then tpsMax and hence checks and mapping needs to be reversed

    tpsADC = UINT8_MAX - tpsADC; //Reverse the ADC values
    tempTPSMax = UINT8_MAX - configPage2.tpsMax;
    tempTPSMin = UINT8_MAX - configPage2.tpsMin;
  }

  //Check that the ADC values fall within the min and max ranges (Should always be the case, but noise can cause these to fluctuate outside the defined range).
  tpsADC = clamp(tpsADC, tempTPSMin, tempTPSMax);
  currentStatus.TPS = map(tpsADC, tempTPSMin, tempTPSMax, 0, 200); //Take the raw TPS ADC value and convert it into a TPS% based on the calibrated values

  //Check whether the closed throttle position sensor is active
  currentStatus.CTPSActive = configPage2.CTPSEnabled == true && isCTPSSensorActive();
}

static inline void readTPS(void)
{
  readTPS((uint8_t)LOW_PASS_FILTER((uint16_t)fastMap10Bit(readAnalogSensor(pinTPS), 0U, 255U), configPage4.ADCFILTER_TPS, (uint16_t)currentStatus.tpsADC));
}


void initialiseTPS(void) { 
  readTPS((uint8_t)fastMap10Bit(readAnalogSensor(pinTPS), 0U, 255U)); // Need to read tps to detect flood clear state
}

static inline void readCLT(void)
{
  currentStatus.cltADC = LOW_PASS_FILTER(readAnalogSensor(pinCLT), configPage4.ADCFILTER_CLT, currentStatus.cltADC);
  currentStatus.coolant = temperatureRemoveOffset(table2D_getValue(&cltCalibrationTable, currentStatus.cltADC)); //Temperature calibration values are stored as positive bytes. We subtract 40 from them to allow for negative temperatures
}

void initialiseCLT(void) {
  currentStatus.cltADC = readAnalogSensor(pinCLT);
  currentStatus.coolant = temperatureRemoveOffset(table2D_getValue(&cltCalibrationTable, currentStatus.cltADC)); //Temperature calibration values are stored as positive bytes. We subtract 40 from them to allow for negative temperatures
}

static inline void readIAT(void)
{
  currentStatus.iatADC = LOW_PASS_FILTER(readAnalogSensor(pinIAT), configPage4.ADCFILTER_IAT, currentStatus.iatADC);
  currentStatus.IAT = temperatureRemoveOffset(table2D_getValue(&iatCalibrationTable, currentStatus.iatADC));
}

// ========================================== Baro ==========================================

/* 
* The highest sea-level pressure on Earth occurs in Siberia, where the Siberian High often attains a sea-level pressure above 105 kPa;
* with record highs close to 108.5 kPa.
* The lowest possible baro reading is based on an altitude of 3500m above sea level.
*/
static inline bool isValidBaro(uint8_t baro) 
{
  static constexpr uint16_t BARO_MIN = 65U;
  static constexpr uint16_t BARO_MAX = 108U;

  return (baro >= BARO_MIN) && (baro <= BARO_MAX);
}

static inline void setBaroFromSensorReading(uint16_t sensorReading) 
{
  currentStatus.baroADC = sensorReading;
  int16_t tempValue = fastMap10Bit(currentStatus.baroADC, configPage2.baroMin, configPage2.baroMax);
  currentStatus.baro = (uint8_t)max((int16_t)0, tempValue);
}

// Should only be called when the engine isn't running.
static inline void setBaroFromMAP(void) 
{
  uint16_t tempReading = mapADCToMAP(readMAPSensor(pinMAP), configPage2.mapMin, configPage2.mapMax);
  if (isValidBaro(tempReading)) //Safety check to ensure the baro reading is within the physical limits
  {
    currentStatus.baro = tempReading;
    if(!BIT_CHECK(statusSensors, BIT_SENSORS_BARO_SAVED))
    {
      storeLastBaro(currentStatus.baro); 
      BIT_SET(statusSensors, BIT_SENSORS_BARO_SAVED); //Flag baro as having been saved. This prevents multiple writes happening, which can cause issues on stm32 with internal flash
    }
  }
}

static inline void readBaro(void)
{
  if ( configPage6.useExtBaro != 0U  ) 
  {
    // readings
    setBaroFromSensorReading(LOW_PASS_FILTER(readMAPSensor(pinBaro), configPage4.ADCFILTER_BARO, currentStatus.baroADC)); //Very weak filter
  // If no dedicated baro sensor is available, attempt to get a reading from the MAP sensor. This can only be done if the engine is not running. 
  } else if ((currentStatus.RPM == 0U) && !engineIsRunning(micros()-MICROS_PER_SEC)) {
    setBaroFromMAP();
  } else {
    // Do nothing - baro remains at last read value & MISRA checker is kept happy.
  }
}

void initialiseMAPBaro(void) 
{
  //Initialise MAP values to all 0's
  (void)memset(&mapAlgorithmState, 0, sizeof(mapAlgorithmState));
  
  //Initialise baro
  if ( configPage6.useExtBaro != 0U  )
  {
    // Use raw unfiltered value initially
    setBaroFromSensorReading(readMAPSensor(pinBaro));
  }
  else
  {
    //Attempt to use the last known good baro reading from EEPROM as a starting point
    uint8_t lastBaro = readLastBaro();
    // Make sure it's not invalid (Possible on first run etc)
    currentStatus.baro = isValidBaro(lastBaro) ? lastBaro : 100U;
    // We assume external callers already made sure the engine isn't running
    setBaroFromMAP();
  }
}

void resetMAPcycleAndEvent(void)
{
  (void)memset(&mapAlgorithmState.cycle_average, 0, sizeof(mapAlgorithmState.cycle_average));
  (void)memset(&mapAlgorithmState.cycle_min, 0, sizeof(mapAlgorithmState.cycle_min));
  (void)memset(&mapAlgorithmState.event_average, 0, sizeof(mapAlgorithmState.event_average));
}

// ========================================== O2 ==========================================

static inline void readO2_1(void)
{
  //An O2 read is only performed if an O2 sensor type is selected. This is to prevent potentially dangerous use of the O2 readings prior to proper setup/calibration
  if(configPage6.egoType > 0U)
  {
    currentStatus.O2ADC = LOW_PASS_FILTER(readAnalogSensor(pinO2), configPage4.ADCFILTER_O2, currentStatus.O2ADC);
    currentStatus.O2 = table2D_getValue(&o2CalibrationTable, currentStatus.O2ADC);
  }
  else
  {
    currentStatus.O2ADC = 0U;
    currentStatus.O2 = 0U;
  }
  
}

static inline void readO2_2(void)
{
  // Read second O2 if configured.
  if (pinO2_2!=0U)
  {
    currentStatus.O2_2ADC = LOW_PASS_FILTER(readAnalogSensor(pinO2_2), configPage4.ADCFILTER_O2, currentStatus.O2_2ADC);
    currentStatus.O2_2 = table2D_getValue(&o2CalibrationTable, currentStatus.O2_2ADC);
  }
}

static inline void readO2(void)
{
  if (configPage2.canWBO == 0U) {
    readO2_1();
    readO2_2();
  } 
}

static inline void readBat(void)
{
  int16_t tempReading = fastMap10Bit(readAnalogSensor(pinBat), 0, 245); //Get the current raw Battery value. Permissible values are from 0v to 24.5v (245)

  //Apply the offset calibration value to the reading
  tempReading += configPage4.batVoltCorrect;
  if(tempReading < 0){
    tempReading=0;
  }  //with negative overflow prevention


  //The following is a check for if the voltage has jumped up from under 5.5v to over 7v.
  //If this occurs, it's very likely that the system has gone from being powered by USB to being powered from the 12v power source.
  //Should that happen, we re-trigger the fuel pump priming and idle homing (If using a stepper)
  if( (currentStatus.battery10 < 55U) && (tempReading > 70) && (currentStatus.RPM == 0U) )
  {
    //Re-prime the fuel pump
    fpPrimeTime = currentStatus.secl;
    currentStatus.fpPrimed = false;
    FUEL_PUMP_ON();

    //Redo the stepper homing
    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL) )
    {
      initialiseIdle(true);
    }
  }

  currentStatus.battery10 = LOW_PASS_FILTER(tempReading, configPage4.ADCFILTER_BAT, currentStatus.battery10);
}

#if defined(ANALOG_ISR)
static inline void enableAnalogIsr(void)
{
  //ADC in free running mode does 1 complete conversion of all 16 channels and then the interrupt is disabled. Every 200Hz we re-enable the interrupt to get another conversion cycle
  BIT_SET(ADCSRA,ADIE); //Enable ADC interrupt
}
#endif

/**
 * @brief Returns the VSS pulse gap for a given history point
 * 
 * @param historyIndex The gap number that is wanted. EG:
 * historyIndex = 0 = Latest entry
 * historyIndex = 1 = 2nd entry entry
 */
uint32_t vssGetPulseGap(uint8_t historyIndex)
{
  uint32_t tempGap = 0;
  
  noInterrupts();
  int8_t tempIndex = vssIndex - historyIndex;
  if(tempIndex < 0) { tempIndex += (int8_t)VSS_SAMPLES; }

  if(tempIndex > 0) { tempGap = vssTimes[tempIndex] - vssTimes[tempIndex - 1]; }
  else { tempGap = vssTimes[0] - vssTimes[(VSS_SAMPLES-1U)]; }
  interrupts();

  return tempGap;
}

static inline uint16_t getSpeed(void)
{
  uint16_t tempSpeed = 0;
  // Get VSS from CAN, Serial or Analog by using Aux input channels.
  if(configPage2.vssMode == VSS_MODE_INTERNAL_PIN)
  {
    // Direct reading from Aux channel
    if (configPage2.vssPulsesPerKm == 0U)
    {
      tempSpeed = currentStatus.canin[configPage2.vssAuxCh];
    }
    // Adjust the reading by dividing it by set amount.
    else
    {
      tempSpeed = fast_div(currentStatus.canin[configPage2.vssAuxCh], configPage2.vssPulsesPerKm);
    }
    tempSpeed = LOW_PASS_FILTER(tempSpeed, configPage2.vssSmoothing, currentStatus.vss); //Apply speed smoothing factor
  }
  // Interrupt driven mode
  else if (isExternalVssMode(configPage2))
  {
    uint32_t pulseTime = 0U;
    uint32_t vssTotalTime = 0U;

    //Add up the time between the teeth. Note that the total number of gaps is equal to the number of samples minus 1
    for(uint8_t x = 0U; x<(VSS_SAMPLES-1U); x++)
    {
      vssTotalTime += vssGetPulseGap(x);
    }

    pulseTime = fast_div(vssTotalTime,  VSS_SAMPLES - 1UL);
    if ( (micros() - vssTimes[vssIndex]) > MICROS_PER_SEC ) { tempSpeed = 0; } // Check that the car hasn't come to a stop. Is true if last pulse was more than 1 second ago
    else 
    {
      tempSpeed = fast_div(MICROS_PER_HOUR, pulseTime * configPage2.vssPulsesPerKm); //Convert the pulse gap into km/h
      tempSpeed = LOW_PASS_FILTER(tempSpeed, configPage2.vssSmoothing, currentStatus.vss); //Apply speed smoothing factor
    }
    if(tempSpeed > 1000U) { tempSpeed = currentStatus.vss; } //Safety check. This usually occurs when there is a hardware issue
  } else {
    // Do nothing
  }
  return tempSpeed;
}

static inline void readSpeed(void)
{
  currentStatus.vss = getSpeed();
}

static inline byte getGear(void)
{
  byte tempGear = 0U; //Unknown gear
  if(currentStatus.vss > 0U)
  {
    //If the speed is non-zero, default to the last calculated gear
    tempGear = currentStatus.gear;

    uint16_t pulsesPer1000rpm = fast_div32_16((uint32_t)(currentStatus.vss * 10000UL), currentStatus.RPM); //Gives the current pulses per 1000RPM, multiplied by 10 (10x is the multiplication factor for the ratios in TS)
    //Begin gear detection
    if( (pulsesPer1000rpm > (configPage2.vssRatio1 - VSS_GEAR_HYSTERESIS)) && (pulsesPer1000rpm < (configPage2.vssRatio1 + VSS_GEAR_HYSTERESIS)) ) { tempGear = 1; }
    else if( (pulsesPer1000rpm > (configPage2.vssRatio2 - VSS_GEAR_HYSTERESIS)) && (pulsesPer1000rpm < (configPage2.vssRatio2 + VSS_GEAR_HYSTERESIS)) ) { tempGear = 2; }
    else if( (pulsesPer1000rpm > (configPage2.vssRatio3 - VSS_GEAR_HYSTERESIS)) && (pulsesPer1000rpm < (configPage2.vssRatio3 + VSS_GEAR_HYSTERESIS)) ) { tempGear = 3; }
    else if( (pulsesPer1000rpm > (configPage2.vssRatio4 - VSS_GEAR_HYSTERESIS)) && (pulsesPer1000rpm < (configPage2.vssRatio4 + VSS_GEAR_HYSTERESIS)) ) { tempGear = 4; }
    else if( (pulsesPer1000rpm > (configPage2.vssRatio5 - VSS_GEAR_HYSTERESIS)) && (pulsesPer1000rpm < (configPage2.vssRatio5 + VSS_GEAR_HYSTERESIS)) ) { tempGear = 5; }
    else if( (pulsesPer1000rpm > (configPage2.vssRatio6 - VSS_GEAR_HYSTERESIS)) && (pulsesPer1000rpm < (configPage2.vssRatio6 + VSS_GEAR_HYSTERESIS)) ) { tempGear = 6; }
    else {
      // Do nothing 
    }
  }
  
  return tempGear;
}

static inline void readGear(void)
{
  currentStatus.gear = getGear();
}

static inline byte getFuelPressure(void)
{
  int16_t tempFuelPressure = 0;

  if(configPage10.fuelPressureEnable > 0U)
  {
    tempFuelPressure = fastMap10Bit(readAnalogSensor(pinFuelPressure), configPage10.fuelPressureMin, configPage10.fuelPressureMax);
    tempFuelPressure = LOW_PASS_FILTER(tempFuelPressure, ADCFILTER_PSI_DEFAULT, currentStatus.fuelPressure); //Apply smoothing factor
    //Sanity checks
    tempFuelPressure = clamp(tempFuelPressure, (int16_t)0, (int16_t)configPage10.fuelPressureMax);
  }

  return (byte)tempFuelPressure;
}

static inline void updateFuelPressure(void)
{
  currentStatus.fuelPressure = getFuelPressure();
}

static inline byte getOilPressure(void)
{
  int16_t tempOilPressure = 0;

  if(configPage10.oilPressureEnable > 0U)
  {
    //Perform ADC read
    tempOilPressure = fastMap10Bit(readAnalogSensor(pinOilPressure), configPage10.oilPressureMin, configPage10.oilPressureMax);
    tempOilPressure = LOW_PASS_FILTER(tempOilPressure, ADCFILTER_PSI_DEFAULT, currentStatus.oilPressure); //Apply smoothing factor
    //Sanity check
    tempOilPressure = clamp(tempOilPressure, (int16_t)0, (int16_t)configPage10.oilPressureMax);
  }


  return (byte)tempOilPressure;
}

static inline void updateOilPressure(void)
{
  currentStatus.oilPressure = getOilPressure();
}

BEGIN_LTO_ALWAYS_INLINE(void) readPolledSensors(byte loopTimer)
{
  static constexpr polledAction_t polledSensors[] = {
    {TPS_READ_TIMER_BIT, readTPS},
    {CLT_READ_TIMER_BIT, readCLT},
    {IAT_READ_TIMER_BIT, readIAT},
    {O2_READ_TIMER_BIT, readO2},
    {BAT_READ_TIMER_BIT, readBat},
    {BARO_READ_TIMER_BIT, readBaro},
    {MAP_READ_TIMER_BIT, readMAP},
#if defined(ANALOG_ISR)
    {BIT_TIMER_200HZ, enableAnalogIsr},
#endif
    {BIT_TIMER_10HZ, readSpeed},
    {BIT_TIMER_10HZ, readGear},
    {BIT_TIMER_4HZ, updateFuelPressure},
    {BIT_TIMER_4HZ, updateOilPressure},
  };
  
  static_for<0, _countof(polledSensors)>::repeat_n(executePolledArrayAction, polledSensors, loopTimer);
}
END_LTO_INLINE()

uint8_t getAnalogKnock(void)
{
  uint8_t pinKnock = A15; //Default value in case the user has not selected an analog pin in TunerStudio
  if(configPage10.knock_pin >=47U)
  {
    pinKnock = pinTranslateAnalog(configPage10.knock_pin - 47U); //The knock_pin variable has both digital and analog pins listed. A0 is at position 47
  }

  //Perform ADC read
  return (uint8_t)fastMap10Bit(readAnalogSensor(pinKnock), 0U, 255U);
}

/*
 * The interrupt function for reading the flex sensor frequency and pulse width
 * flexCounter value is incremented with every pulse and reset back to 0 once per second
 */
void flexPulse(void)
{
  if(READ_FLEX() == true)
  {
    uint16_t tempPW = clamp(micros() - flexStartTime, 0UL, (unsigned long)UINT16_MAX); //Calculate the pulse width
    flexPulseWidth = LOW_PASS_FILTER(tempPW, configPage4.FILTER_FLEX, flexPulseWidth);
    ++flexCounter;
  }
  else
  {
    flexStartTime = micros(); //Start pulse width measurement.
  }
}

/*
 * The interrupt function for pulses from a knock conditioner / controller
 * 
 */
void knockPulse(void)
{
  if( (currentStatus.MAP < (configPage10.knock_maxMAP*2)) && (currentStatus.RPMdiv100 < configPage10.knock_maxRPM) )
  {
    if(!currentStatus.knockRetardActive) { currentStatus.knockCount++; } //If knock is not currently active we count every pulse. If knock is already active then additional pulses will be counted in correctionKnockTiming()
    currentStatus.knockPulseDetected = true;
  }
}

/**
 * @brief The ISR function for VSS pulses
 * 
 */
void vssPulse(void)
{
  //TODO: Add basic filtering here
  vssIndex++;
  if(vssIndex == VSS_SAMPLES) { vssIndex = 0U; }

  vssTimes[vssIndex] = micros();
}

// Read the Aux analog value for pin set by analogPin 
uint16_t readAuxanalog(uint8_t analogPin)
{
  return readAnalogSensor(analogPin); // readAnalogSensor is inlined within this CPP file.
} 

uint16_t readAuxdigital(uint8_t digitalPin)
{
  //read the Aux digital value for pin set by digitalPin 
  unsigned int tempReading;
  tempReading = digitalRead(digitalPin); 
  return tempReading;
} 
