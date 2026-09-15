
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler_fuel_controller.h"
#include "../channel_test_helpers.h"
#include "scheduler_ignition_controller.h"

#define TIMEOUT 1000
#define DURATION 1000

static void test_status_pending_to_running(Schedule &schedule)
{
    setSchedule(schedule, TIMEOUT, DURATION, true);
    while(schedule._status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, schedule._status);
    while(schedule._status != OFF) /*Wait*/ ;
}

static void test_status_pending_to_running_inj(FuelSchedule &schedule)
{
    schedule.reset();
    startFuelSchedulers();
    test_status_pending_to_running(schedule);
    stopFuelSchedulers();
}

static void test_status_pending_to_running_inj(void)
{
  for (auto& schedule: fuelSchedules) {
    test_status_pending_to_running_inj(schedule);
  }
}

static void test_status_pending_to_running_ign(IgnitionSchedule &schedule)
{
    schedule.reset();
    startIgnitionSchedulers();
    test_status_pending_to_running(schedule);
    stopIgnitionSchedulers();
}

static void test_status_pending_to_running_ign1(void)
{
    IGNCHANNEL_TEST_HELPER1(test_status_pending_to_running_ign(ignitionSchedule1));
}

static void test_status_pending_to_running_ign2(void)
{
    IGNCHANNEL_TEST_HELPER2(test_status_pending_to_running_ign(ignitionSchedule2));
}

static void test_status_pending_to_running_ign3(void)
{
    IGNCHANNEL_TEST_HELPER3(test_status_pending_to_running_ign(ignitionSchedule3));
}

static void test_status_pending_to_running_ign4(void)
{
    IGNCHANNEL_TEST_HELPER4(test_status_pending_to_running_ign(ignitionSchedule4));
}

static void test_status_pending_to_running_ign5(void)
{
    IGNCHANNEL_TEST_HELPER5(test_status_pending_to_running_ign(ignitionSchedule5));
}

static void test_status_pending_to_running_ign6(void)
{
    IGNCHANNEL_TEST_HELPER6(test_status_pending_to_running_ign(ignitionSchedule6));
}

static void test_status_pending_to_running_ign7(void)
{
    IGNCHANNEL_TEST_HELPER7(test_status_pending_to_running_ign(ignitionSchedule7));
}

static void test_status_pending_to_running_ign8(void)
{
    IGNCHANNEL_TEST_HELPER8(test_status_pending_to_running_ign(ignitionSchedule8));
}

void test_status_pending_to_running(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_pending_to_running_inj);

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
