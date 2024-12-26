
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "scheduledIO.h"

#define TIMEOUT 1000
#define DURATION 1000
#define DELTA 24

static uint32_t start_time, end_time;
static void startCallback(void) { end_time = micros(); }
static void endCallback(void) { /*Empty*/ }

static void test_accuracy_timeout_inj(FuelSchedule &schedule)
{
    initialiseFuelSchedulers();
    startFuelSchedulers();
    schedule.pStartCallback = startCallback;
    schedule.pEndCallback = endCallback;
    start_time = micros();
    setFuelSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
    while(schedule.Status != OFF) /*Wait*/ ;
    stopFuelSchedulers();
}

static void test_accuracy_timeout_inj1(void)
{
    test_accuracy_timeout_inj(fuelSchedule1);
}

static void test_accuracy_timeout_inj2(void)
{
    test_accuracy_timeout_inj(fuelSchedule2);
}

static void test_accuracy_timeout_inj3(void)
{
    test_accuracy_timeout_inj(fuelSchedule3);
}

static void test_accuracy_timeout_inj4(void)
{
    test_accuracy_timeout_inj(fuelSchedule4);
}

static void test_accuracy_timeout_inj5(void)
{
#if INJ_CHANNELS >= 5
    test_accuracy_timeout_inj(fuelSchedule5);
#endif
}

static void test_accuracy_timeout_inj6(void)
{
#if INJ_CHANNELS >= 6
    test_accuracy_timeout_inj(fuelSchedule6);
#endif
}

static void test_accuracy_timeout_inj7(void)
{
#if INJ_CHANNELS >= 7
    test_accuracy_timeout_inj(fuelSchedule7);
#endif
}

static void test_accuracy_timeout_inj8(void)
{
#if INJ_CHANNELS >= 8
    test_accuracy_timeout_inj(fuelSchedule8);
#endif
}

static void test_accuracy_timeout_ign(IgnitionSchedule &schedule)
{
    initialiseIgnitionSchedulers();
    startIgnitionSchedulers();
    schedule.pStartCallback = startCallback;
    schedule.pEndCallback = endCallback;
    start_time = micros();
    setIgnitionSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
    while(schedule.Status != OFF) /*Wait*/ ;
    stopIgnitionSchedulers();
}

static void test_accuracy_timeout_ign1(void)
{
    test_accuracy_timeout_ign(ignitionSchedule1);
}

static void test_accuracy_timeout_ign2(void)
{
    test_accuracy_timeout_ign(ignitionSchedule2);
}

static void test_accuracy_timeout_ign3(void)
{
    test_accuracy_timeout_ign(ignitionSchedule3);
}

static void test_accuracy_timeout_ign4(void)
{
    test_accuracy_timeout_ign(ignitionSchedule4);
}

static void test_accuracy_timeout_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_accuracy_timeout_ign(ignitionSchedule5);
#endif
}

static void test_accuracy_timeout_ign6(void)
{
#if IGN_CHANNELS >= 6
    test_accuracy_timeout_ign(ignitionSchedule6);
#endif
}

static void test_accuracy_timeout_ign7(void)
{
#if IGN_CHANNELS >= 7
    test_accuracy_timeout_ign(ignitionSchedule7);
#endif
}

static void test_accuracy_timeout_ign8(void)
{
#if IGN_CHANNELS >= 8
    test_accuracy_timeout_ign(ignitionSchedule8);
#endif
}

void test_accuracy_timeout(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_accuracy_timeout_inj1);
    RUN_TEST_P(test_accuracy_timeout_inj2);
    RUN_TEST_P(test_accuracy_timeout_inj3);
    RUN_TEST_P(test_accuracy_timeout_inj4);
    RUN_TEST_P(test_accuracy_timeout_inj5);
    RUN_TEST_P(test_accuracy_timeout_inj6);
    RUN_TEST_P(test_accuracy_timeout_inj7);
    RUN_TEST_P(test_accuracy_timeout_inj8);

    RUN_TEST_P(test_accuracy_timeout_ign1);
    RUN_TEST_P(test_accuracy_timeout_ign2);
    RUN_TEST_P(test_accuracy_timeout_ign3);
    RUN_TEST_P(test_accuracy_timeout_ign4);
    RUN_TEST_P(test_accuracy_timeout_ign5);
    RUN_TEST_P(test_accuracy_timeout_ign6);
    RUN_TEST_P(test_accuracy_timeout_ign7);
    RUN_TEST_P(test_accuracy_timeout_ign8);
  }
}
