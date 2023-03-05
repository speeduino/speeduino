#include <Arduino.h>
#include <unity.h>
#include "schedule_calcs.h"
#include "../test_utils.h"
#include "type_traits.h"

using raw_counter_t = type_traits::remove_reference<IgnitionSchedule::counter_t>::type;
using raw_compare_t = type_traits::remove_reference<IgnitionSchedule::compare_t>::type;

void test_adjust_crank_angle_pending_below_minrevolutions()
{
    raw_counter_t counter = {0};
    raw_compare_t compare = {0};
    IgnitionSchedule schedule(counter, compare);

    schedule.Status = PENDING;
    currentStatus.startRevolutions = 0;

    schedule._compare = 101;
    schedule._counter = 100;
    schedule.endAngle = 359;

    // Should do nothing.
    adjustCrankAngle(schedule, 180);

    TEST_ASSERT_EQUAL(101, schedule._compare);
    TEST_ASSERT_EQUAL(100, schedule._counter);
}


void test_adjust_crank_angle_pending_above_minrevolutions()
{
    raw_counter_t counter = {0};
    raw_compare_t compare = {0};
    IgnitionSchedule schedule(counter, compare);
    
    currentStatus.startRevolutions = 2000;
    
    schedule._compare = 101;
    schedule._counter = 100;
    schedule.endCompare = 100;
    schedule.Status = PENDING;

    constexpr uint16_t newCrankAngle = 180;
    constexpr uint16_t chargeAngle = 359;
    schedule.startAngle = chargeAngle;

    adjustCrankAngle(schedule, newCrankAngle);

    // TEST_ASSERT_EQUAL(101, schedule._compare);
    TEST_ASSERT_EQUAL(100, schedule._counter);
    TEST_ASSERT_EQUAL(schedule._counter+uS_TO_TIMER_COMPARE(angleToTimeMicroSecPerDegree(chargeAngle-newCrankAngle)), schedule._compare);
}

void test_adjust_crank_angle_running()
{
    raw_counter_t counter = {0};
    raw_compare_t compare = {0};
    IgnitionSchedule schedule(counter, compare);
    
    schedule.Status = RUNNING;
    currentStatus.startRevolutions = 2000;
    // timePerDegreex16 = 666;

    schedule._compare = 101;
    schedule._counter = 100;
    schedule.endCompare = 100;
    constexpr uint16_t newCrankAngle = 180;
    constexpr uint16_t chargeAngle = 359;
    schedule.endAngle = chargeAngle;

    adjustCrankAngle(schedule, newCrankAngle);

    TEST_ASSERT_EQUAL(schedule._counter+uS_TO_TIMER_COMPARE(angleToTimeMicroSecPerDegree(chargeAngle-newCrankAngle)), schedule._compare);
    TEST_ASSERT_EQUAL(100, schedule._counter);
    TEST_ASSERT_EQUAL(100, schedule.endCompare);
}

void test_adjust_crank_angle()
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_adjust_crank_angle_pending_below_minrevolutions);
    RUN_TEST(test_adjust_crank_angle_pending_above_minrevolutions);
    RUN_TEST(test_adjust_crank_angle_running);
  }
}