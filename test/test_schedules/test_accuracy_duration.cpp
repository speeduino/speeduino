
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000
#define DELTA 20

static uint32_t start_time, end_time;
static void startCallback(void) { start_time = micros(); }
static void endCallback(void) { end_time = micros(); }

static void test_accuracy_duration_inj(FuelSchedule &schedule)
{
    initialiseFuelSchedulers();
    startFuelSchedulers();
    schedule.pStartCallback = startCallback;
    schedule.pEndCallback = endCallback;
    setFuelSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
    stopFuelSchedulers();
}

static void test_accuracy_duration_inj1(void)
{
    test_accuracy_duration_inj(fuelSchedule1);
}

static void test_accuracy_duration_inj2(void)
{
    test_accuracy_duration_inj(fuelSchedule2);
}

static void test_accuracy_duration_inj3(void)
{
    test_accuracy_duration_inj(fuelSchedule3);
}

static void test_accuracy_duration_inj4(void)
{
    test_accuracy_duration_inj(fuelSchedule4);
}

static void test_accuracy_duration_inj5(void)
{
#if INJ_CHANNELS >= 5
    test_accuracy_duration_inj(fuelSchedule5);
#endif
}

static void test_accuracy_duration_inj6(void)
{
#if INJ_CHANNELS >= 6
    test_accuracy_duration_inj(fuelSchedule6);
#endif
}

static void test_accuracy_duration_inj7(void)
{
#if INJ_CHANNELS >= 7
    test_accuracy_duration_inj(fuelSchedule7);
#endif
}

static void test_accuracy_duration_inj8(void)
{
#if INJ_CHANNELS >= 8
    test_accuracy_duration_inj(fuelSchedule8);
#endif
}

static void test_accuracy_duration_ign(IgnitionSchedule &schedule)
{
    initialiseIgnitionSchedulers();
    startIgnitionSchedulers();
    schedule.pStartCallback = startCallback;
    schedule.pEndCallback = endCallback;
    setIgnitionSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);    
    stopIgnitionSchedulers();
}
static void test_accuracy_duration_ign1(void)
{
    test_accuracy_duration_ign(ignitionSchedule1);
}

static void test_accuracy_duration_ign2(void)
{
    test_accuracy_duration_ign(ignitionSchedule2);
}

static void test_accuracy_duration_ign3(void)
{
    test_accuracy_duration_ign(ignitionSchedule3);
}

static void test_accuracy_duration_ign4(void)
{
    test_accuracy_duration_ign(ignitionSchedule4);
}

static void test_accuracy_duration_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_accuracy_duration_ign(ignitionSchedule5);
#endif
}

static void test_accuracy_duration_ign6(void)
{
#if INJ_CHANNELS >= 6
    test_accuracy_duration_ign(ignitionSchedule6);
#endif
}

static void test_accuracy_duration_ign7(void)
{
#if INJ_CHANNELS >= 7
    test_accuracy_duration_ign(ignitionSchedule7);
#endif
}

static void test_accuracy_duration_ign8(void)
{
#if INJ_CHANNELS >= 8
    test_accuracy_duration_ign(ignitionSchedule8);
#endif
}

void test_accuracy_duration(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_accuracy_duration_inj1);
    RUN_TEST_P(test_accuracy_duration_inj2);
    RUN_TEST_P(test_accuracy_duration_inj3);
    RUN_TEST_P(test_accuracy_duration_inj4);
    RUN_TEST_P(test_accuracy_duration_inj5);
    RUN_TEST_P(test_accuracy_duration_inj6);
    RUN_TEST_P(test_accuracy_duration_inj7);
    RUN_TEST_P(test_accuracy_duration_inj8);

    RUN_TEST_P(test_accuracy_duration_ign1);
    RUN_TEST_P(test_accuracy_duration_ign2);
    RUN_TEST_P(test_accuracy_duration_ign3);
    RUN_TEST_P(test_accuracy_duration_ign4);
    RUN_TEST_P(test_accuracy_duration_ign5);
    RUN_TEST_P(test_accuracy_duration_ign6);
    RUN_TEST_P(test_accuracy_duration_ign7);
    RUN_TEST_P(test_accuracy_duration_ign8);
  }
}
