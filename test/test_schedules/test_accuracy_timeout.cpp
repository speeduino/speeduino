
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler_fuel_controller.h"
#include "../channel_test_helpers.h"
#include "scheduler_ignition_controller.h"

constexpr uint32_t TIMEOUT = 1000U;
constexpr uint16_t DURATION = 1000U;
constexpr uint32_t DELTA = ticksToMicros(6U);

static uint32_t start_time, end_time;
static void startCallback(void) { end_time = micros(); }
static void endCallback(void) { /*Empty*/ }

static void test_accuracy_timeout(Schedule &schedule)
{
    schedule.setCallbacks(startCallback, endCallback);
    start_time = micros();
    setSchedule(schedule, TIMEOUT, DURATION, true);
    while(schedule._status == PENDING) /*Wait*/ ;
    while(schedule._status != OFF) /*Wait*/ ;
    TEST_ASSERT_GREATER_THAN(DELTA, TIMEOUT);
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

static void test_accuracy_timeout_inj(FuelSchedule &schedule)
{
    schedule.reset();
    startFuelSchedulers();
    test_accuracy_timeout(schedule);
    stopFuelSchedulers();
}

static void test_accuracy_timeout_inj(void)
{
  for (auto& schedule: fuelSchedules) {
    test_accuracy_timeout_inj(schedule);
  }
}

static void test_accuracy_timeout_ign(IgnitionSchedule &schedule)
{
    schedule.reset();
    startIgnitionSchedulers();
    test_accuracy_timeout(schedule);
    stopIgnitionSchedulers();
}

static void test_accuracy_timeout_ign(void)
{
    for (auto& schedule: ignitionSchedules)
    {
        test_accuracy_timeout_ign(schedule);
    }
}

void test_accuracy_timeout(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_accuracy_timeout_inj);
    RUN_TEST_P(test_accuracy_timeout_ign);
  }
}
