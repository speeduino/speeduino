#pragma once

/// @cond
#include <stdint.h>

// These are private to the MAP sampling algorithms but are broken out here
// to support unit testing of the algorithm implementations

struct map_last_read_t {
  uint16_t lastMAPValue;        // kPA
  uint32_t currentReadingTime;  // µS
#if defined(__UINT24_MAX__)
  // Maximum time between readings is ~3s (at min RPM). 24 bits is enough for that
  __uint24 timeDeltaReadings;
#else
  uint32_t timeDeltaReadings;
#endif
};

// A pair of ADC sensor readings
struct map_adc_readings_t {
  uint16_t mapADC;
  uint16_t emapADC;
};

// Working state for the cycle average sampling algorithm
struct map_cycle_average_t {
  uint8_t cycleStartIndex;
#if defined(__UINT24_MAX__)
  // Maximum revolution time is ~1.5s (at min RPM). At a 1KHz sampling rate & 2 revolutions, 
  // we'll store 3000 readings each at maximum of 1023 (~3000000 max). So a 24-bit value
  // should be plenty.
  __uint24 mapAdcRunningTotal;
  __uint24 emapAdcRunningTotal;
#else  
  uint32_t mapAdcRunningTotal;
  uint32_t emapAdcRunningTotal;
#endif
  uint16_t sampleCount;
};

// Working state for the cycle minimum sampling algorithm
struct map_cycle_min_t {
  uint8_t cycleStartIndex;
  uint16_t mapMinimum;
};

// Working state for the event average sampling algorithm
struct map_event_average_t {
#if defined(__UINT24_MAX__)
  __uint24 mapAdcRunningTotal;
#else  
  uint32_t mapAdcRunningTotal;
#endif
  uint16_t sampleCount;
  uint8_t eventStartIndex;
};

// The overall MAP sampling system working state
struct map_algorithm_t {
  map_last_read_t lastReading;
  map_adc_readings_t sensorReadings;

  union {
    map_cycle_average_t cycle_average;
    map_cycle_min_t cycle_min;
    map_event_average_t event_average;
  };
};

/// @endcond