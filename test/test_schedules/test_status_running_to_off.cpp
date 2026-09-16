
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler_fuel_controller.h"
#include "../channel_test_helpers.h"
#include "scheduler_ignition_controller.h"

#define TIMEOUT 1000
#define DURATION 1000

static void test_status_running_to_off(Schedule &schedule)
{
    setSchedule(schedule, TIMEOUT, DURATION, true);
    while( (schedule._status == PENDING) || (schedule._status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

static void test_status_running_to_off_inj(FuelSchedule &schedule)
{
    schedule.reset();
    startFuelSchedulers();
    test_status_running_to_off(schedule);
    stopFuelSchedulers();
}

static void test_status_running_to_off_inj(void)
{
  for (auto& schedule: fuelSchedules) {
    test_status_running_to_off_inj(schedule);
  }
}

static void test_status_running_to_off_ign(IgnitionSchedule &schedule)
{
    schedule.reset();
    startIgnitionSchedulers();
    test_status_running_to_off(schedule);
    stopIgnitionSchedulers();
}

static void test_status_running_to_off_ign1(void)
{
    IGNCHANNEL_TEST_HELPER1(test_status_running_to_off_ign(ignitionSchedules[0]));
}

static void test_status_running_to_off_ign2(void)
{
    IGNCHANNEL_TEST_HELPER2(test_status_running_to_off_ign(ignitionSchedules[1]));
}

static void test_status_running_to_off_ign3(void)
{
    IGNCHANNEL_TEST_HELPER3(test_status_running_to_off_ign(ignitionSchedules[2]));
}

static void test_status_running_to_off_ign4(void)
{
    IGNCHANNEL_TEST_HELPER4(test_status_running_to_off_ign(ignitionSchedules[3]));
}

static void test_status_running_to_off_ign5(void)
{
    IGNCHANNEL_TEST_HELPER5(test_status_running_to_off_ign(ignitionSchedules[4]));
}

static void test_status_running_to_off_ign6(void)
{
    IGNCHANNEL_TEST_HELPER6(test_status_running_to_off_ign(ignitionSchedules[5]));
}

static void test_status_running_to_off_ign7(void)
{
    IGNCHANNEL_TEST_HELPER7(test_status_running_to_off_ign(ignitionSchedules[6]));
}

static void test_status_running_to_off_ign8(void)
{
    IGNCHANNEL_TEST_HELPER8(test_status_running_to_off_ign(ignitionSchedules[7]));
}

void test_status_running_to_off(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_running_to_off_inj);

    RUN_TEST_P(test_status_running_to_off_ign1);
    RUN_TEST_P(test_status_running_to_off_ign2);
    RUN_TEST_P(test_status_running_to_off_ign3);
    RUN_TEST_P(test_status_running_to_off_ign4);
    RUN_TEST_P(test_status_running_to_off_ign5);
    RUN_TEST_P(test_status_running_to_off_ign6);
    RUN_TEST_P(test_status_running_to_off_ign7);
    RUN_TEST_P(test_status_running_to_off_ign8);
  }
}
