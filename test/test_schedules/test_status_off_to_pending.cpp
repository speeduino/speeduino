
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

void test_status_off_to_pending_ign1(void)
{
    IGNCHANNEL_TEST_HELPER1(test_status_off_to_pending_ign(ignitionSchedule1));
}

void test_status_off_to_pending_ign2(void)
{
    IGNCHANNEL_TEST_HELPER2(test_status_off_to_pending_ign(ignitionSchedule2));
}

void test_status_off_to_pending_ign3(void)
{
    IGNCHANNEL_TEST_HELPER3(test_status_off_to_pending_ign(ignitionSchedule3));
}

void test_status_off_to_pending_ign4(void)
{
    IGNCHANNEL_TEST_HELPER4(test_status_off_to_pending_ign(ignitionSchedule4));
}

void test_status_off_to_pending_ign5(void)
{
    IGNCHANNEL_TEST_HELPER5(test_status_off_to_pending_ign(ignitionSchedule5));
}

void test_status_off_to_pending_ign6(void)
{
    IGNCHANNEL_TEST_HELPER6(test_status_off_to_pending_ign(ignitionSchedule6));
}

void test_status_off_to_pending_ign7(void)
{
    IGNCHANNEL_TEST_HELPER7(test_status_off_to_pending_ign(ignitionSchedule7));
}

void test_status_off_to_pending_ign8(void)
{
    IGNCHANNEL_TEST_HELPER8(test_status_off_to_pending_ign(ignitionSchedule8));
}

void test_status_off_to_pending(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_off_to_pending_inj);

    RUN_TEST_P(test_status_off_to_pending_ign1);
    RUN_TEST_P(test_status_off_to_pending_ign2);
    RUN_TEST_P(test_status_off_to_pending_ign3);
    RUN_TEST_P(test_status_off_to_pending_ign4);
    RUN_TEST_P(test_status_off_to_pending_ign5);
    RUN_TEST_P(test_status_off_to_pending_ign6);
    RUN_TEST_P(test_status_off_to_pending_ign7);
    RUN_TEST_P(test_status_off_to_pending_ign8);
  }
}
