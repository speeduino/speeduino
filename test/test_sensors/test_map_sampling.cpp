#include <unity.h>
#include "../test_utils.h"
#include "sensors_map_structs.h"
#include "config_pages.h"
#include "statuses.h"
#include "globals.h"
#include "decoders.h"
#include "units.h"

static void test_instantaneous(void) {
  extern bool instanteneousMAPReading(void);

  TEST_ASSERT_TRUE(instanteneousMAPReading());
}

extern bool cycleAverageMAPReading(const statuses &current, const config2 &page2, const decoder_status_t &decoderStatus, map_cycle_average_t &cycle_average, map_adc_readings_t &sensorReadings);
extern bool canUseCycleAverage(const statuses &current, const config2 &page2, const decoder_status_t &decoderStatus);

static void enable_cycle_average(statuses &current, config2 &page2, decoder_status_t &decoderStatus) {
  setRpm(current, 4300U);
  page2.mapSwitchPoint = 15; 
  current.startRevolutions = 55;
  decoderStatus.syncStatus = SyncStatus::Full;
}

static void test_canUseCycleAverge(void) {
  statuses current;
  config2 page2;
  decoder_status_t decoderStatus;
  enable_cycle_average(current, page2, decoderStatus);

  TEST_ASSERT_TRUE(canUseCycleAverage(current, page2, decoderStatus));

  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_FALSE(canUseCycleAverage(current, page2, decoderStatus));
  decoderStatus.syncStatus = SyncStatus::Full;

  current.startRevolutions = 1;
  TEST_ASSERT_FALSE(canUseCycleAverage(current, page2, decoderStatus));
  current.startRevolutions = 55;

  setRpm(current, RPM_COARSE.toUser(page2.mapSwitchPoint-1U));
  TEST_ASSERT_FALSE(canUseCycleAverage(current, page2, decoderStatus));
  setRpm(current, RPM_COARSE.toUser(page2.mapSwitchPoint));
  TEST_ASSERT_FALSE(canUseCycleAverage(current, page2, decoderStatus));
  setRpm(current, RPM_COARSE.toUser(page2.mapSwitchPoint+1U));
  TEST_ASSERT_TRUE(canUseCycleAverage(current, page2, decoderStatus));
}

struct cycleAverageMAPReading_test_data {
  statuses current;
  config2 page2;
  decoder_status_t decoderStatus;
  map_cycle_average_t cycle_average;
  map_adc_readings_t sensorReadings;
};

static void setup_cycle_average(cycleAverageMAPReading_test_data &test_data) {
  enable_cycle_average(test_data.current, test_data.page2, test_data.decoderStatus);
  test_data.cycle_average.cycleStartIndex = 0;
  test_data.cycle_average.sampleCount = 0;
  test_data.cycle_average.emapAdcRunningTotal = 0;
  test_data.cycle_average.mapAdcRunningTotal = 0;
}

static void test_cycleAverageMAPReading_fallback_instantaneous(void) {
  cycleAverageMAPReading_test_data test_data;
  setup_cycle_average(test_data);

  test_data.decoderStatus.syncStatus = SyncStatus::None;
  test_data.sensorReadings.mapADC = 0x1234;
  test_data.sensorReadings.emapADC = 0x1234;

  TEST_ASSERT_TRUE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.emapADC, test_data.cycle_average.emapAdcRunningTotal);

  // Repeat - should get same result.
  TEST_ASSERT_TRUE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.emapADC, test_data.cycle_average.emapAdcRunningTotal);
}

static void test_cycleAverageMAPReading(void) {
  cycleAverageMAPReading_test_data test_data;
  setup_cycle_average(test_data);

  test_data.cycle_average.cycleStartIndex = test_data.current.startRevolutions;
  test_data.sensorReadings.mapADC = 100;
  test_data.sensorReadings.emapADC = 200;
  // Accumulate a few samples
  TEST_ASSERT_FALSE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.emapADC, test_data.cycle_average.emapAdcRunningTotal);
  test_data.sensorReadings.mapADC = 300;
  test_data.sensorReadings.emapADC = 500;
  TEST_ASSERT_FALSE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(2, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(400, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(700, test_data.cycle_average.emapAdcRunningTotal);
  ++test_data.current.startRevolutions;
  test_data.sensorReadings.mapADC = 500;
  test_data.sensorReadings.emapADC = 700;
  TEST_ASSERT_FALSE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.cycle_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(3, test_data.cycle_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(900, test_data.cycle_average.mapAdcRunningTotal);
  TEST_ASSERT_EQUAL_UINT(1400, test_data.cycle_average.emapAdcRunningTotal);

  // Leave the current cycle
  ++test_data.current.startRevolutions;
  TEST_ASSERT_TRUE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.cycle_average, test_data.sensorReadings));
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
  TEST_ASSERT_TRUE(cycleAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.cycle_average, test_data.sensorReadings));
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
  setRpm(test_data.current, 4300U);
  test_data.current.startRevolutions = 0U;
  test_data.page2.mapSwitchPoint = 15; 
  test_data.cycle_min.cycleStartIndex = 0;
  test_data.cycle_min.mapMinimum = UINT16_MAX;
}

static void test_cycleMinimumMAPReading_fallback_instantaneous(void) {
  cycleMinmumMAPReading_test_data test_data;
  setup_cycle_minimum(test_data);

  setRpm(test_data.current, RPM_COARSE.toUser(test_data.page2.mapSwitchPoint - 1U));
  test_data.sensorReadings.mapADC = 300;
  test_data.sensorReadings.emapADC = 500;  
  test_data.cycle_min.mapMinimum = UINT16_MAX;

  TEST_ASSERT_TRUE(cycleMinimumMAPReading(test_data.current, test_data.page2, test_data.cycle_min, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_min.mapMinimum);

  // Repeat - should get same result.
  test_data.cycle_min.mapMinimum = UINT16_MAX;
  TEST_ASSERT_TRUE(cycleMinimumMAPReading(test_data.current, test_data.page2, test_data.cycle_min, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.cycle_min.mapMinimum);
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

extern bool canUseEventAverage(const statuses &current, const config2 &page2, const decoder_status_t &decoderStatus);
extern bool eventAverageMAPReading(const statuses &current, const config2 &page2, const decoder_status_t &decoderStatus, map_event_average_t &eventAverage, map_adc_readings_t &sensorReadings);

static void enable_event_average(statuses &current, config2 &page2, decoder_status_t &decoderStatus) {
  setRpm(current, 4300U);
  page2.mapSwitchPoint = 15; 
  current.startRevolutions = 55;
  decoderStatus.syncStatus = SyncStatus::Full;
  resetEngineProtect(current);
}

static void test_canUseEventAverage(void) {
  statuses current;
  config2 page2;
  decoder_status_t decoderStatus;
  enable_event_average(current, page2, decoderStatus);

  decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_TRUE(canUseEventAverage(current, page2, decoderStatus));

  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, decoderStatus));
  decoderStatus.syncStatus = SyncStatus::Full;

  current.startRevolutions = 1;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, decoderStatus));
  current.startRevolutions = 55;

  setRpm(current, RPM_COARSE.toUser(page2.mapSwitchPoint-1U));
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, decoderStatus));
  setRpm(current, RPM_COARSE.toUser(page2.mapSwitchPoint));
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, decoderStatus));
  setRpm(current, RPM_COARSE.toUser(page2.mapSwitchPoint+1U));
  TEST_ASSERT_TRUE(canUseEventAverage(current, page2, decoderStatus));

  current.engineProtectRpm = true;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, decoderStatus));
  resetEngineProtect(current);

  current.engineProtectBoostCut = true;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, decoderStatus));
  resetEngineProtect(current);

  current.engineProtectOil = true;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, decoderStatus));
  resetEngineProtect(current);

  current.engineProtectAfr = true;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, decoderStatus));
  resetEngineProtect(current);

  current.engineProtectClt = true;
  TEST_ASSERT_FALSE(canUseEventAverage(current, page2, decoderStatus));
  resetEngineProtect(current);
}

struct eventAverageMAPReading_test_data {
  statuses current;
  config2 page2;
  decoder_status_t decoderStatus;
  map_event_average_t event_average;
  map_adc_readings_t sensorReadings;
};

static void setup_event_average(eventAverageMAPReading_test_data &test_data) {
  enable_event_average(test_data.current, test_data.page2, test_data.decoderStatus);
  test_data.event_average.eventStartIndex = 0;
  test_data.event_average.sampleCount = 0;
  test_data.event_average.mapAdcRunningTotal = 0;
  ignitionCount = 0;
}

static void test_eventAverageMAPReading_fallback_instantaneous(void) {
  eventAverageMAPReading_test_data test_data;
  setup_event_average(test_data);

  test_data.decoderStatus.syncStatus = SyncStatus::None;
  test_data.sensorReadings.mapADC = 0x1234;
  test_data.sensorReadings.emapADC = 0x1234;

  TEST_ASSERT_TRUE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.event_average.mapAdcRunningTotal);

  // Repeat - should get same result.
  TEST_ASSERT_TRUE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.event_average.mapAdcRunningTotal);
}


static void test_eventAverageMAPReading(void) {
  eventAverageMAPReading_test_data test_data;
  setup_event_average(test_data);

  test_data.event_average.eventStartIndex = (uint8_t)ignitionCount;
  test_data.sensorReadings.mapADC = 100;
  test_data.sensorReadings.emapADC = 200;
  // Accumulate a few samples
  TEST_ASSERT_FALSE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(1, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.event_average.mapAdcRunningTotal);
  test_data.sensorReadings.mapADC = 300;
  test_data.sensorReadings.emapADC = 500;
  TEST_ASSERT_FALSE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(2, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(400, test_data.event_average.mapAdcRunningTotal);
  test_data.sensorReadings.mapADC = 500;
  test_data.sensorReadings.emapADC = 700;
  TEST_ASSERT_FALSE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(3, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(900, test_data.event_average.mapAdcRunningTotal);

  // Leave the current cycle
  ++ignitionCount;
  TEST_ASSERT_TRUE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.event_average, test_data.sensorReadings));
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
  TEST_ASSERT_TRUE(eventAverageMAPReading(test_data.current, test_data.page2, test_data.decoderStatus, test_data.event_average, test_data.sensorReadings));
  TEST_ASSERT_EQUAL_UINT(100, test_data.sensorReadings.mapADC);
  TEST_ASSERT_EQUAL_UINT(200, test_data.sensorReadings.emapADC);

  TEST_ASSERT_EQUAL_UINT(1, test_data.event_average.sampleCount);
  TEST_ASSERT_EQUAL_UINT(test_data.sensorReadings.mapADC, test_data.event_average.mapAdcRunningTotal);
}

extern uint16_t validateFilterMapSensorReading(uint16_t reading, uint8_t alpha, uint16_t prior);

static void test_validateFilterMapSensorReading(void) {
  TEST_ASSERT_EQUAL_UINT(0, validateFilterMapSensorReading(0, 0, 0));
  TEST_ASSERT_EQUAL_UINT(100, validateFilterMapSensorReading(0, 0, 100));
  TEST_ASSERT_EQUAL_UINT(333, validateFilterMapSensorReading(333, 0, 100));
  TEST_ASSERT_EQUAL_UINT(217, validateFilterMapSensorReading(333, 127, 100));
}

// Extern declaration for function under test (as used in sensors.cpp)
extern bool applyMapAlgorithm(const config2 &page2,
                              const statuses &current,
                              const decoder_status_t &decoderStatus,
                              map_algorithm_t &algorithmState);

static void test_applyMapAlgorithm_instantaneous(void) {
  config2 page2 = {};
  statuses current = {};
  decoder_status_t decoderStatus = {};
  map_algorithm_t alg = {};
  page2.mapSample = MAPSamplingInstantaneous;

  TEST_ASSERT_TRUE(applyMapAlgorithm(page2, current, decoderStatus, alg));
}

static void test_applyMapAlgorithm_cycleAverage_fallback_instantaneous(void) {
  config2 page2 = {};
  statuses current = {};
  decoder_status_t decoderStatus = {};
  map_algorithm_t alg = {};
  page2.mapSample = MAPSamplingCycleAverage;
  page2.mapSwitchPoint = 15;
  // Ensure canUseCycleAverage is false (low RPM and no sync)
  setRpm(current, 0U);
  decoderStatus.syncStatus = SyncStatus::None;
  alg.cycle_average.sampleCount = 0;

  bool ok = applyMapAlgorithm(page2, current, decoderStatus, alg);
  TEST_ASSERT_TRUE(ok);
  // reset() calls accumulate once -> sampleCount becomes 1
  TEST_ASSERT_EQUAL_UINT(1U, alg.cycle_average.sampleCount);
}

static void test_applyMapAlgorithm_cycleAverage_accumulate(void) {
  config2 page2 = {};
  statuses current = {};
  decoder_status_t decoderStatus = {};
  map_algorithm_t alg = {};
  page2.mapSample = MAPSamplingCycleAverage;
  page2.mapSwitchPoint = 15;
  // Enable cycle averaging
  setRpm(current, RPM_COARSE.toUser(page2.mapSwitchPoint + 1U));
  decoderStatus.syncStatus = SyncStatus::Full;
  current.startRevolutions = 55;
  // Force accumulate path
  alg.cycle_average.cycleStartIndex = (uint8_t)current.startRevolutions;
  alg.cycle_average.sampleCount = 0;

  bool ok = applyMapAlgorithm(page2, current, decoderStatus, alg);
  TEST_ASSERT_FALSE(ok);
  TEST_ASSERT_EQUAL_UINT(1U, alg.cycle_average.sampleCount);
}

static void test_applyMapAlgorithm_cycleMinimum_endCycle_true(void) {
  config2 page2 = {};
  statuses current = {};
  decoder_status_t decoderStatus = {};
  map_algorithm_t alg = {};
  page2.mapSample = MAPSamplingCycleMinimum;
  page2.mapSwitchPoint = 15;
  // Ensure RPM above switch point
  setRpm(current, RPM_COARSE.toUser(page2.mapSwitchPoint + 1U));
  // Force end-cycle by making stored cycleStartIndex different
  alg.cycle_min.cycleStartIndex = (uint8_t)(current.startRevolutions - 2U);
  alg.cycle_min.mapMinimum = 123;

  bool ok = applyMapAlgorithm(page2, current, decoderStatus, alg);
  TEST_ASSERT_TRUE(ok);
}

static void test_applyMapAlgorithm_eventAverage_accumulate(void) {
  config2 page2 = {};
  statuses current = {};
  decoder_status_t decoderStatus = {};
  map_algorithm_t alg = {};
  page2.mapSample = MAPSamplingIgnitionEventAverage;
  page2.mapSwitchPoint = 15;
  // Enable event average conditions
  setRpm(current, RPM_COARSE.toUser(page2.mapSwitchPoint + 1U));
  decoderStatus.syncStatus = SyncStatus::Full;
  current.startRevolutions = 55;
  // Ensure engine protect flags cleared
  resetEngineProtect(current);
  // Set ignitionCount and eventStartIndex equal to trigger accumulate
  ignitionCount = 0;
  alg.event_average.eventStartIndex = (uint8_t)ignitionCount;
  alg.event_average.sampleCount = 0;

  bool ok = applyMapAlgorithm(page2, current, decoderStatus, alg);
  TEST_ASSERT_FALSE(ok);
  TEST_ASSERT_EQUAL_UINT(1U, alg.event_average.sampleCount);
}

extern void storeLastMAPReadings(uint32_t currTime, map_last_read_t &lastRead, uint16_t oldMAPValue);

static void test_storeLastMAPReadings_basic(void) {
  map_last_read_t last = {};
  last.currentReadingTime = 1000U;
  last.timeDeltaReadings = 0U;
  last.lastMAPValue = 0U;

  storeLastMAPReadings(1500U, last, 42U);

  TEST_ASSERT_EQUAL_UINT16(42U, last.lastMAPValue);
  TEST_ASSERT_EQUAL_UINT32(500U, last.timeDeltaReadings);
  TEST_ASSERT_EQUAL_UINT32(1500U, last.currentReadingTime);
}

extern void setMAPValuesFromReadings(const map_adc_readings_t &readings, const config2 &page2, bool useEMAP, statuses &current);

static void test_setMAPValuesFromReadings_no_emap(void) {
  map_adc_readings_t readings = {};
  config2 page2 = {};
  statuses current = {};

  // map range 0..100, using mid-scale ADC -> expect ~50
  page2.mapMin = 0;
  page2.mapMax = 100;
  readings.mapADC = 512;
  current.MAP = 0;
  current.EMAP = 123; // should remain unchanged when useEMAP=false

  setMAPValuesFromReadings(readings, page2, false, current);

  TEST_ASSERT_EQUAL_INT(50, current.MAP);
  TEST_ASSERT_EQUAL_INT(123, current.EMAP);
}

static void test_setMAPValuesFromReadings_with_emap(void) {
  map_adc_readings_t readings = {};
  config2 page2 = {};
  statuses current = {};

  // MAP: 0..100 => 512 -> 50
  page2.mapMin = 0;
  page2.mapMax = 100;
  // EMAP: 10..60 => 512 -> 35 (10 + ((512*50)>>10) = 10+25)
  page2.EMAPMin = 10;
  page2.EMAPMax = 60;

  readings.mapADC = 512;
  readings.emapADC = 512;

  setMAPValuesFromReadings(readings, page2, true, current);

  TEST_ASSERT_EQUAL_INT(50, current.MAP);
  TEST_ASSERT_EQUAL_INT(35, current.EMAP);
}

void test_map_sampling(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_instantaneous);
    RUN_TEST_P(test_canUseCycleAverge);
    RUN_TEST_P(test_cycleAverageMAPReading_fallback_instantaneous);
    RUN_TEST_P(test_cycleAverageMAPReading);
    RUN_TEST_P(test_cycleAverageMAPReading_nosamples);
    RUN_TEST_P(test_cycleMinimumMAPReading_fallback_instantaneous);
    RUN_TEST_P(test_cycleMinimumMAPReading);
    RUN_TEST_P(test_canUseEventAverage);
    RUN_TEST_P(test_eventAverageMAPReading_fallback_instantaneous);
    RUN_TEST_P(test_eventAverageMAPReading);
    RUN_TEST_P(test_eventAverageMAPReading_nosamples);
    RUN_TEST_P(test_validateFilterMapSensorReading);
    RUN_TEST_P(test_applyMapAlgorithm_instantaneous);
    RUN_TEST_P(test_applyMapAlgorithm_cycleAverage_fallback_instantaneous);
    RUN_TEST_P(test_applyMapAlgorithm_cycleAverage_accumulate);
    RUN_TEST_P(test_applyMapAlgorithm_cycleMinimum_endCycle_true);
    RUN_TEST_P(test_applyMapAlgorithm_eventAverage_accumulate);
    RUN_TEST_P(test_storeLastMAPReadings_basic);
    RUN_TEST_P(test_setMAPValuesFromReadings_no_emap);
    RUN_TEST_P(test_setMAPValuesFromReadings_with_emap);
  }    
}