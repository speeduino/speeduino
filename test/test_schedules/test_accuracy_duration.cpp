
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
static void startCallback(void) { start_time = micros(); }
static void endCallback(void) { end_time = micros(); }

static void test_accuracy_duration(Schedule &schedule)
{
    schedule.setCallbacks(startCallback, endCallback);
    setSchedule(schedule, TIMEOUT, DURATION, true);
    while(schedule._status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

static void test_accuracy_duration_inj(FuelSchedule &schedule)
{
    schedule.reset();
    startFuelSchedulers();
    test_accuracy_duration(schedule);
    stopFuelSchedulers();
}

static void test_accuracy_duration_inj(void)
{
  for (auto& schedule: fuelSchedules) {
    test_accuracy_duration_inj(schedule);
  }
}

static void test_accuracy_duration_ign(IgnitionSchedule &schedule)
{
    schedule.reset();
    startIgnitionSchedulers();
    test_accuracy_duration(schedule);
    stopIgnitionSchedulers();
}

static void test_accuracy_duration_ign1(void)
{
    IGNCHANNEL_TEST_HELPER1(test_accuracy_duration_ign(ignitionSchedules[0]));
}

static void test_accuracy_duration_ign2(void)
{
    IGNCHANNEL_TEST_HELPER2(test_accuracy_duration_ign(ignitionSchedules[1]));
}

static void test_accuracy_duration_ign3(void)
{
    IGNCHANNEL_TEST_HELPER3(test_accuracy_duration_ign(ignitionSchedules[2]));
}

static void test_accuracy_duration_ign4(void)
{
    IGNCHANNEL_TEST_HELPER4(test_accuracy_duration_ign(ignitionSchedules[3]));
}

static void test_accuracy_duration_ign5(void)
{
    IGNCHANNEL_TEST_HELPER5(test_accuracy_duration_ign(ignitionSchedules[4]));
}

static void test_accuracy_duration_ign6(void)
{
    IGNCHANNEL_TEST_HELPER6(test_accuracy_duration_ign(ignitionSchedules[5]));
}

static void test_accuracy_duration_ign7(void)
{
    IGNCHANNEL_TEST_HELPER7(test_accuracy_duration_ign(ignitionSchedules[6]));
}

static void test_accuracy_duration_ign8(void)
{
    IGNCHANNEL_TEST_HELPER8(test_accuracy_duration_ign(ignitionSchedules[7]));
}

void test_accuracy_duration(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_accuracy_duration_inj);

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
