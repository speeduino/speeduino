
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "channel_test_helpers.h"

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
    setCallbacks(schedule, startCallback, endCallback);
    setFuelSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status != OFF) /*Wait*/ ;
    stopFuelSchedulers();
#if defined(NATIVE_BOARD)
    TEST_IGNORE_MESSAGE("Timing not accurate on native board");
#else
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
#endif
}

static void test_accuracy_duration_inj1(void)
{
    INJCHANNEL_TEST_HELPER1(test_accuracy_duration_inj(fuelSchedule1));
}

static void test_accuracy_duration_inj2(void)
{
    INJCHANNEL_TEST_HELPER2(test_accuracy_duration_inj(fuelSchedule2));
}

static void test_accuracy_duration_inj3(void)
{
    INJCHANNEL_TEST_HELPER3(test_accuracy_duration_inj(fuelSchedule3));
}

static void test_accuracy_duration_inj4(void)
{
    INJCHANNEL_TEST_HELPER4(test_accuracy_duration_inj(fuelSchedule4));
}

static void test_accuracy_duration_inj5(void)
{
    INJCHANNEL_TEST_HELPER5(test_accuracy_duration_inj(fuelSchedule5));
}

static void test_accuracy_duration_inj6(void)
{
    INJCHANNEL_TEST_HELPER6(test_accuracy_duration_inj(fuelSchedule6));
}

static void test_accuracy_duration_inj7(void)
{
    INJCHANNEL_TEST_HELPER7(test_accuracy_duration_inj(fuelSchedule7));
}

static void test_accuracy_duration_inj8(void)
{
    INJCHANNEL_TEST_HELPER8(test_accuracy_duration_inj(fuelSchedule8));
}

static void test_accuracy_duration_ign(IgnitionSchedule &schedule)
{
    initialiseIgnitionSchedulers();
    startIgnitionSchedulers();
    setCallbacks(schedule, startCallback, endCallback);
    setIgnitionSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status != OFF) /*Wait*/ ;
    stopIgnitionSchedulers();
#if defined(NATIVE_BOARD)
    TEST_IGNORE_MESSAGE("Timing not accurate on native board");
#else
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
#endif    
}

static void test_accuracy_duration_ign1(void)
{
    IGNCHANNEL_TEST_HELPER1(test_accuracy_duration_ign(ignitionSchedule1));
}

static void test_accuracy_duration_ign2(void)
{
    IGNCHANNEL_TEST_HELPER2(test_accuracy_duration_ign(ignitionSchedule2));
}

static void test_accuracy_duration_ign3(void)
{
    IGNCHANNEL_TEST_HELPER3(test_accuracy_duration_ign(ignitionSchedule3));
}

static void test_accuracy_duration_ign4(void)
{
    IGNCHANNEL_TEST_HELPER4(test_accuracy_duration_ign(ignitionSchedule4));
}

static void test_accuracy_duration_ign5(void)
{
    IGNCHANNEL_TEST_HELPER5(test_accuracy_duration_ign(ignitionSchedule5));
}

static void test_accuracy_duration_ign6(void)
{
    IGNCHANNEL_TEST_HELPER6(test_accuracy_duration_ign(ignitionSchedule6));
}

static void test_accuracy_duration_ign7(void)
{
    IGNCHANNEL_TEST_HELPER7(test_accuracy_duration_ign(ignitionSchedule7));
}

static void test_accuracy_duration_ign8(void)
{
    IGNCHANNEL_TEST_HELPER8(test_accuracy_duration_ign(ignitionSchedule8));
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
