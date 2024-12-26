
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void test_status_pending_to_running_inj(FuelSchedule &schedule)
{
    initialiseFuelSchedulers();
    startFuelSchedulers();
    setFuelSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule.Status);
    while(schedule.Status != OFF) /*Wait*/ ;
    stopFuelSchedulers();
}

static void test_status_pending_to_running_inj1(void)
{
    test_status_pending_to_running_inj(fuelSchedule1);
}

static void test_status_pending_to_running_inj2(void)
{
    test_status_pending_to_running_inj(fuelSchedule2);
}

static void test_status_pending_to_running_inj3(void)
{
    test_status_pending_to_running_inj(fuelSchedule3);
}

static void test_status_pending_to_running_inj4(void)
{
    test_status_pending_to_running_inj(fuelSchedule4);
}

static void test_status_pending_to_running_inj5(void)
{
#if INJ_CHANNELS >= 5
    test_status_pending_to_running_inj(fuelSchedule5);
#endif
}

static void test_status_pending_to_running_inj6(void)
{
#if INJ_CHANNELS >= 6
    test_status_pending_to_running_inj(fuelSchedule6);
#endif
}

static void test_status_pending_to_running_inj7(void)
{
#if INJ_CHANNELS >= 7
    test_status_pending_to_running_inj(fuelSchedule7);
#endif
}

static void test_status_pending_to_running_inj8(void)
{
#if INJ_CHANNELS >= 8
    test_status_pending_to_running_inj(fuelSchedule8);
#endif
}

static void test_status_pending_to_running_ign(IgnitionSchedule &schedule)
{
    initialiseIgnitionSchedulers();
    startIgnitionSchedulers();
    setIgnitionSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule.Status);
    while(schedule.Status != OFF) /*Wait*/ ;
    stopIgnitionSchedulers();
}

static void test_status_pending_to_running_ign1(void)
{
    test_status_pending_to_running_ign(ignitionSchedule1);
}

static void test_status_pending_to_running_ign2(void)
{
    test_status_pending_to_running_ign(ignitionSchedule2);
}

static void test_status_pending_to_running_ign3(void)
{
    test_status_pending_to_running_ign(ignitionSchedule3);
}

static void test_status_pending_to_running_ign4(void)
{
    test_status_pending_to_running_ign(ignitionSchedule4);
}

static void test_status_pending_to_running_ign5(void)
{
#if IGN_CHANNELS >= 5
    test_status_pending_to_running_ign(ignitionSchedule5);
#endif
}

static void test_status_pending_to_running_ign6(void)
{
#if INJ_CHANNELS >= 6
    test_status_pending_to_running_ign(ignitionSchedule6);
#endif
}

static void test_status_pending_to_running_ign7(void)
{
#if INJ_CHANNELS >= 7
    test_status_pending_to_running_ign(ignitionSchedule7);
#endif
}

static void test_status_pending_to_running_ign8(void)
{
#if INJ_CHANNELS >= 8
    test_status_pending_to_running_ign(ignitionSchedule8);
#endif
}

void test_status_pending_to_running(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_pending_to_running_inj1);
    RUN_TEST_P(test_status_pending_to_running_inj2);
    RUN_TEST_P(test_status_pending_to_running_inj3);
    RUN_TEST_P(test_status_pending_to_running_inj4);
    RUN_TEST_P(test_status_pending_to_running_inj5);
    RUN_TEST_P(test_status_pending_to_running_inj6);
    RUN_TEST_P(test_status_pending_to_running_inj7);
    RUN_TEST_P(test_status_pending_to_running_inj8);

    RUN_TEST_P(test_status_pending_to_running_ign1);
    RUN_TEST_P(test_status_pending_to_running_ign2);
    RUN_TEST_P(test_status_pending_to_running_ign3);
    RUN_TEST_P(test_status_pending_to_running_ign4);
    RUN_TEST_P(test_status_pending_to_running_ign5);
    RUN_TEST_P(test_status_pending_to_running_ign6);
    RUN_TEST_P(test_status_pending_to_running_ign7);
    RUN_TEST_P(test_status_pending_to_running_ign8);
  }
}
