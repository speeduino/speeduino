
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler_fuel_controller.h"
#include "../channel_test_helpers.h"
#include "scheduler_ignition_controller.h"

#define TIMEOUT 1000
#define DURATION 1000

void test_status_off_to_pending(Schedule &schedule)
{
    setSchedule(schedule, TIMEOUT, DURATION, true);
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
    while(schedule._status != OFF) /*Wait*/ ;
}

void test_status_off_to_pending_inj(FuelSchedule &schedule)
{
    schedule.reset();
    startFuelSchedulers();
    test_status_off_to_pending(schedule);
    stopFuelSchedulers();
}

void test_status_off_to_pending_inj(void)
{
  for (auto& schedule: fuelSchedules) {
    test_status_off_to_pending_inj(schedule);
  }
}

void test_status_off_to_pending_ign(IgnitionSchedule &schedule)
{
    schedule.reset();
    startIgnitionSchedulers();
    test_status_off_to_pending(schedule);
    stopIgnitionSchedulers();
}

void test_status_off_to_pending_ign(void)
{
    for (auto& schedule: ignitionSchedules)
    {
        test_status_off_to_pending_ign(schedule);
    }
}

void test_status_off_to_pending(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_off_to_pending_inj);
    RUN_TEST_P(test_status_off_to_pending_ign);
  }
}
