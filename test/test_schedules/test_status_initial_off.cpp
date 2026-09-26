#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler_fuel_controller.h"
#include "../channel_test_helpers.h"
#include "scheduler_ignition_controller.h"

static void test_status_initial_off_inj(FuelSchedule &schedule)
{
    schedule.reset();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

static void test_status_initial_off_inj(void)
{
  for (auto& schedule: fuelSchedules) {
    test_status_initial_off_inj(schedule);
  }    
}

static void test_status_initial_off_ign(IgnitionSchedule &schedule)
{
    schedule.reset();
    TEST_ASSERT_EQUAL(OFF, schedule._status);
}

static void test_status_initial_off_ign(void)
{
    for (auto& schedule: ignitionSchedules)
    {
        test_status_initial_off_ign(schedule);
    }    
}

void test_status_initial_off(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_initial_off_inj); 
    RUN_TEST_P(test_status_initial_off_ign);
  }
}