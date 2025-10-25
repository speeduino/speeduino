#include <unity.h>
#include "../test_utils.h"
#include "sensors_map_structs.h"
#include "config_pages.h"
#include "statuses.h"
#include "globals.h"

static void test_instantaneous(void) {
  extern bool instanteneousMAPReading(void);

  TEST_ASSERT_TRUE(instanteneousMAPReading());
}

extern void setMAPValuesFromReadings(const map_adc_readings_t &readings, const config2 &page2, bool useEMAP, statuses &current);

static void test_setMAPValuesFromReadings(void) {
  config2 page2;
  page2.mapMin = page2.EMAPMin = 10;
  page2.mapMax = page2.EMAPMax = 260;
  statuses current;
  setMAPValuesFromReadings({ 0, 0 }, page2, true, current);
  TEST_ASSERT_EQUAL_UINT(10, current.MAP);
  TEST_ASSERT_EQUAL_UINT(10, current.EMAP);

  setMAPValuesFromReadings({ 1024, 1024 }, page2, true, current);
  TEST_ASSERT_EQUAL_UINT(260, current.MAP);
  TEST_ASSERT_EQUAL_UINT(260, current.EMAP);

  setMAPValuesFromReadings({ 0, 0 }, page2, true, current);
  TEST_ASSERT_EQUAL_UINT(10, current.MAP);
  TEST_ASSERT_EQUAL_UINT(10, current.EMAP);
}

extern bool cycleAverageMAPReading(const statuses &current, const config2 &page2, map_cycle_average_t &cycle_average, map_adc_readings_t &sensorReadings);
extern bool canUseCycleAverage(const statuses &current, const config2 &page2, const map_adc_readings_t &sensorReadings);

static void enable_cycle_average(statuses &current, config2 &page2) {
  current.RPMdiv100 = 43;
  page2.mapSwitchPoint = 15; 
  current.startRevolutions = 55;
  current.hasSync = true;
  current.status3 =  0U;
}

static constexpr uint16_t VALID_MAP_MAX=1022U; //The largest ADC value that is valid for the MAP sensor
static constexpr uint16_t VALID_MAP_MIN=2U; //The smallest ADC value that is valid for the MAP sensor
static constexpr map_adc_readings_t VALID_MAP_READINGS = { VALID_MAP_MIN+1, VALID_MAP_MAX-1 };
static constexpr map_adc_readings_t INVALID_MAP_READINGS = { VALID_MAP_MIN, VALID_MAP_MAX-1 };

extern bool isValidMapSensorReadings(const map_adc_readings_t &sensorReadings);

static void test_isValidMapSensorReadings() {
  TEST_ASSERT_TRUE(isValidMapSensorReadings(VALID_MAP_READINGS));
  TEST_ASSERT_FALSE(isValidMapSensorReadings(INVALID_MAP_READINGS));
  TEST_ASSERT_FALSE(isValidMapSensorReadings({ VALID_MAP_MIN+1, VALID_MAP_MAX }));
  TEST_ASSERT_TRUE(isValidMapSensorReadings({ VALID_MAP_MIN+1, UINT16_MAX }));
  TEST_ASSERT_FALSE(isValidMapSensorReadings({ VALID_MAP_MIN, UINT16_MAX }));
}

static void test_canUseCycleAverge(void) {
  statuses current;
  config2 page2;
  enable_cycle_average(current, page2);

  TEST_ASSERT_TRUE(canUseCycleAverage(current, page2, VALID_MAP_READINGS));

  current.hasSync = false;
  TEST_ASSERT_FALSE(canUseCycleAverage(current, page2, VALID_MAP_READINGS));
  current.hasSync = true;

  current.startRevolutions = 1;
  TEST_ASSERT_FALSE(canUseCycleAverage(current, page2, VALID_MAP_READINGS));
  current.startRevolutions = 55;

  current.RPMdiv100 = page2.mapSwitchPoint-1;
  TEST_ASSERT_FALSE(canUseCycleAverage(current, page2, VALID_MAP_READINGS));
  current.RPMdiv100 = page2.mapSwitchPoint;
  TEST_ASSERT_FALSE(canUseCycleAverage(current, page2, VALID_MAP_READINGS));
  current.RPMdiv100 = page2.mapSwitchPoint+1;
  TEST_ASSERT_TRUE(canUseCycleAverage(current, page2, VALID_MAP_READINGS));

  TEST_ASSERT_FALSE(canUseCycleAverage(current, page2, INVALID_MAP_READINGS));
}

struct cycleAverageMAPReading_test_data {
  statuses current;
  config2 page2;
  map_cycle_average_t cycle_average;
  map_adc_readings_t sensorReadings;
};

static void setup_cycle_average(cycleAverageMAPReading_test_data &test_data) {
  enable_cycle_average(test_data.current, test_data.page2);
  test_data.cycle_average.cycleStartIndex = 0;
  test_data.cycle_average.sampleCount = 0;
  test_data.cycle_average.emapAdcRunningTotal = 0;
  test_data.cycle_average.mapAdcRunningTotal = 0;
}

static void test_cycleAverageMAPReading_fallback_instantaneous(void) {
  cycleAverageMAPReading_test_data test_data;
  setup_cycle_average(test_data);

  test_data.current.hasSync = false;
  test_data.sensorReadings = VALID_MAP_READINGS;

  TEST_ASSERT_TRUE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.emapADC, test_data.cycle_average.emapAdcRunningTotal);

  // Repeat - should get same result.
  TEST_ASSERT_TRUE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.emapADC, test_data.cycle_average.emapAdcRunningTotal);

  // Repeat - should get same result.
  test_data.current.hasSync = true;
  test_data.sensorReadings = INVALID_MAP_READINGS;
  TEST_ASSERT_TRUE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(0, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(0, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(0, test_data.cycle_average.emapAdcRunningTotal);

  // Finally, make sure it was falling back! 
  test_data.current.hasSync = true;
  test_data.sensorReadings = VALID_MAP_READINGS;
  TEST_ASSERT_FALSE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(VALID_MAP_READINGS.mapADC, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(VALID_MAP_READINGS.emapADC, test_data.cycle_average.emapAdcRunningTotal);
}

static void test_cycleAverageMAPReading(void) {
  cycleAverageMAPReading_test_data test_data;
  setup_cycle_average(test_data);

  test_data.cycle_average.cycleStartIndex = test_data.current.startRevolutions;
  test_data.sensorReadings.mapADC = 100;
  test_data.sensorReadings.emapADC = 200;
  // Accumulate a few samples
  TEST_ASSERT_FALSE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.emapADC, test_data.cycle_average.emapAdcRunningTotal);
  test_data.sensorReadings.mapADC = 300;
  test_data.sensorReadings.emapADC = 500;
  TEST_ASSERT_FALSE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(2, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(400, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(700, test_data.cycle_average.emapAdcRunningTotal);
  ++test_data.current.startRevolutions;
  test_data.sensorReadings.mapADC = 500;
  test_data.sensorReadings.emapADC = 700;
  TEST_ASSERT_FALSE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(3, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(900, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(1400, test_data.cycle_average.emapAdcRunningTotal);

  // Leave the current cycle
  ++test_data.current.startRevolutions;
  TEST_ASSERT_TRUE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(900/3, test_data.sensorReadings.mapADC);
  TEST_ASSERT_EQUAL_UINT(1400/3, test_data.sensorReadings.emapADC);

  TEST_ASSERT_EQUAL_UINT(1, test_data.cycle_average.sampleCount);
  TEST_ASSERT_NOT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(500, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_NOT_EQUAL_UINT(test_data.sensorReadings.emapADC, test_data.cycle_average.emapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(700, test_data.cycle_average.emapAdcRunningTotal);
}

static void test_cycleAverageMAPReading_nosamples(void) {
  cycleAverageMAPReading_test_data test_data;
  setup_cycle_average(test_data);

  test_data.cycle_average.cycleStartIndex = test_data.current.startRevolutions;
  test_data.sensorReadings.mapADC = 100;
  test_data.sensorReadings.emapADC = 200;

  ++test_data.current.startRevolutions;
  ++test_data.current.startRevolutions;

  TEST_ASSERT_EQUAL_UINT(0, test_data.cycle_average.sampleCount);
  TEST_ASSERT_TRUE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(100, test_data.sensorReadings.mapADC);
  TEST_ASSERT_EQUAL_UINT(200, test_data.sensorReadings.emapADC);

  TEST_ASSERT_EQUAL_UINT(1, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.emapADC, test_data.cycle_average.emapAdcRunningTotal);
}

extern bool cycleMinimumMAPReading(const statuses &current, const config2 &page2, map_cycle_min_t &cycle_min, map_adc_readings_t &sensorReadings);

struct cycleMinmumMAPReading_test_data {
  statuses current;
  config2 page2;
  map_cycle_min_t cycle_min;
  map_adc_readings_t sensorReadings;
};

static void setup_cycle_minimum(cycleMinmumMAPReading_test_data &test_data) {
  test_data.current.RPMdiv100 = 43;
  test_data.page2.mapSwitchPoint = 15; 
  test_data.cycle_min.cycleStartIndex = 0;
  test_data.cycle_min.mapMinimum = UINT16_MAX;
}

static void test_cycleMinimumMAPReading_fallback_instantaneous(void) {
  cycleMinmumMAPReading_test_data test_data;
  setup_cycle_minimum(test_data);

  test_data.sensorReadings = INVALID_MAP_READINGS;
  TEST_ASSERT_TRUE(cycleMinimumMAPReading(test_data.current, test_data.page2, test_data.cycle_min, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(UINT16_MAX, test_data.cycle_min.mapMinimum);
  TEST_ASSERT_EQUAL_UINT(0U, test_data.cycle_min.cycleStartIndex);

  // Repeat - should get same result.
  TEST_ASSERT_TRUE(cycleMinimumMAPReading(test_data.current, test_data.page2, test_data.cycle_min, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(UINT16_MAX, test_data.cycle_min.mapMinimum);
  TEST_ASSERT_EQUAL_UINT(0U, test_data.cycle_min.cycleStartIndex);

  //
  test_data.current.RPMdiv100 = test_data.page2.mapSwitchPoint - 1U;
  test_data.sensorReadings = VALID_MAP_READINGS;
  TEST_ASSERT_TRUE(cycleMinimumMAPReading(test_data.current, test_data.page2, test_data.cycle_min, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_min.mapMinimum);
  TEST_ASSERT_EQUAL_UINT(test_data.current.startRevolutions, test_data.cycle_min.cycleStartIndex);
}

static void test_cycleMinimumMAPReading(void) {
  cycleMinmumMAPReading_test_data test_data;
  setup_cycle_minimum(test_data);

  test_data.cycle_min.cycleStartIndex = test_data.current.startRevolutions;
  test_data.sensorReadings.mapADC = 100;
  test_data.sensorReadings.emapADC = 200;
  // Accumulate a few samples
  TEST_ASSERT_FALSE(cycleMinimumMAPReading(test_data.current, test_data.page2, test_data.cycle_min, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_min.mapMinimum);
  test_data.sensorReadings.mapADC = 300;
  test_data.sensorReadings.emapADC = 500;
  TEST_ASSERT_FALSE(cycleMinimumMAPReading(test_data.current, test_data.page2, test_data.cycle_min, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(100, test_data.cycle_min.mapMinimum);
  ++test_data.current.startRevolutions;
  test_data.sensorReadings.mapADC = 50;
  test_data.sensorReadings.emapADC = 170;
  TEST_ASSERT_FALSE(cycleMinimumMAPReading(test_data.current, test_data.page2, test_data.cycle_min, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_min.mapMinimum);

  // Leave the current cycle
  ++test_data.current.startRevolutions;
  test_data.sensorReadings.mapADC = 300;
  test_data.sensorReadings.emapADC = 500;
  TEST_ASSERT_TRUE(cycleMinimumMAPReading(test_data.current, test_data.page2, test_data.cycle_min, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(50U, test_data.sensorReadings.mapADC);

  TEST_ASSERT_NOT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_min.mapMinimum);
  TEST_ASSERT_EQUAL_UINT(300, test_data.cycle_min.mapMinimum);
  TEST_ASSERT_EQUAL_UINT8(test_data.current.startRevolutions, test_data.cycle_min.cycleStartIndex);
}

extern bool canUseEventAverage(const statuses &current, const config2 &page2, const map_adc_readings_t &sensorReadings);
extern bool eventAverageMAPReading(const statuses &current, const config2 &page2, map_event_average_t &eventAverage, map_adc_readings_t &sensorReadings);

static void enable_event_average(statuses &current, config2 &page2) {
  current.RPMdiv100 = 43;
  page2.mapSwitchPoint = 15; 
  current.startRevolutions = 55;
  current.hasSync = true;
  current.status3 =  0U;
  current.engineProtectStatus = 0U;
}

static void test_canUseEventAverage(void) {
  statuses current;
  config2 page2;
  enable_event_average(current, page2);

  TEST_ASSERT_TRUE(canUseEventAverage(current, page2, VALID_MAP_READINGS));

  current.hasSync = false;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, VALID_MAP_READINGS));
  current.hasSync = true;

  current.startRevolutions = 1;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, VALID_MAP_READINGS));
  current.startRevolutions = 55;

  current.RPMdiv100 = page2.mapSwitchPoint-1;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, VALID_MAP_READINGS));
  current.RPMdiv100 = page2.mapSwitchPoint;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, VALID_MAP_READINGS));
  current.RPMdiv100 = page2.mapSwitchPoint+1;
  TEST_ASSERT_TRUE(canUseEventAverage(current, page2, VALID_MAP_READINGS));

  current.engineProtectStatus = 1;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, VALID_MAP_READINGS));
  current.engineProtectStatus = 0;  

  TEST_ASSERT_TRUE(canUseEventAverage(current, page2, VALID_MAP_READINGS));
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, INVALID_MAP_READINGS));
}


struct eventAverageMAPReading_test_data {
  statuses current;
  config2 page2;
  map_event_average_t event_average;
  map_adc_readings_t sensorReadings;
};

static void setup_event_average(eventAverageMAPReading_test_data &test_data) {
  enable_event_average(test_data.current, test_data.page2);
  test_data.event_average.eventStartIndex = 0;
  test_data.event_average.sampleCount = 0;
  test_data.event_average.mapAdcRunningTotal = 0;
  ignitionCount = 333;
}

static void test_eventAverageMAPReading_fallback_instantaneous(void) {
  eventAverageMAPReading_test_data test_data;
  setup_event_average(test_data);

  test_data.current.hasSync = false;
  test_data.sensorReadings = VALID_MAP_READINGS;

  TEST_ASSERT_TRUE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT((uint8_t)ignitionCount, test_data.event_average.eventStartIndex);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.event_average.mapAdcRunningTotal);

  // Repeat - should get same result.
  TEST_ASSERT_TRUE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT((uint8_t)ignitionCount, test_data.event_average.eventStartIndex);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.event_average.mapAdcRunningTotal);

  test_data.sensorReadings = INVALID_MAP_READINGS;
  TEST_ASSERT_TRUE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(0, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(0, test_data.event_average.eventStartIndex);
  TEST_ASSERT_EQUAL_UINT(0, test_data.event_average.mapAdcRunningTotal);
}


static void test_eventAverageMAPReading(void) {
  eventAverageMAPReading_test_data test_data;
  setup_event_average(test_data);

  test_data.event_average.eventStartIndex = (uint8_t)ignitionCount;
  test_data.sensorReadings.mapADC = 100;
  test_data.sensorReadings.emapADC = 200;
  // Accumulate a few samples
  TEST_ASSERT_FALSE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.event_average.mapAdcRunningTotal);
  test_data.sensorReadings.mapADC = 300;
  test_data.sensorReadings.emapADC = 500;
  TEST_ASSERT_FALSE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(2, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(400, test_data.event_average.mapAdcRunningTotal);
  test_data.sensorReadings.mapADC = 500;
  test_data.sensorReadings.emapADC = 700;
  TEST_ASSERT_FALSE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(3, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(900, test_data.event_average.mapAdcRunningTotal);

  // Leave the current cycle
  ++ignitionCount;
  TEST_ASSERT_TRUE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(900/3, test_data.sensorReadings.mapADC);

  TEST_ASSERT_EQUAL_UINT(1, test_data.event_average.sampleCount);
  TEST_ASSERT_NOT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.event_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(500, test_data.event_average.mapAdcRunningTotal);
}

static void test_eventAverageMAPReading_nosamples(void) {
  eventAverageMAPReading_test_data test_data;
  setup_event_average(test_data);

  test_data.event_average.eventStartIndex = (uint8_t)ignitionCount;
  test_data.sensorReadings.mapADC = 100;
  test_data.sensorReadings.emapADC = 200;

  ++ignitionCount;
  ++ignitionCount;

  TEST_ASSERT_EQUAL_UINT(0, test_data.event_average.sampleCount);
  TEST_ASSERT_TRUE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(100, test_data.sensorReadings.mapADC);
  TEST_ASSERT_EQUAL_UINT(200, test_data.sensorReadings.emapADC);

  TEST_ASSERT_EQUAL_UINT(1, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.event_average.mapAdcRunningTotal);
}

void test_map_sampling(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_isValidMapSensorReadings);
    RUN_TEST(test_instantaneous);
    RUN_TEST(test_canUseCycleAverge);
    RUN_TEST(test_cycleAverageMAPReading_fallback_instantaneous);
    RUN_TEST(test_cycleAverageMAPReading);
    RUN_TEST(test_cycleAverageMAPReading_nosamples);
    RUN_TEST(test_cycleMinimumMAPReading_fallback_instantaneous);
    RUN_TEST(test_cycleMinimumMAPReading);
    RUN_TEST(test_canUseEventAverage);
    RUN_TEST(test_eventAverageMAPReading_fallback_instantaneous);
    RUN_TEST(test_eventAverageMAPReading);
    RUN_TEST(test_eventAverageMAPReading_nosamples);
    RUN_TEST(test_setMAPValuesFromReadings);
  }    
}