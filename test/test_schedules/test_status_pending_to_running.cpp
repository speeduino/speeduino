
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

static void test_status_pending_to_running_ign(void)
{
    for (auto& schedule: ignitionSchedules)
    {
        test_status_pending_to_running_ign(schedule);
    }    
}

void test_status_pending_to_running(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_pending_to_running_inj);
    RUN_TEST_P(test_status_pending_to_running_ign);
  }
}
