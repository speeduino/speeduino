
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "type_traits.h"

using raw_counter_t = type_traits::remove_reference<IgnitionSchedule::counter_t>::type;
using raw_compare_t = type_traits::remove_reference<IgnitionSchedule::compare_t>::type;

static constexpr uint32_t TIMEOUT = 5000U;
static constexpr uint32_t DURATION = 2000U;
static constexpr COMPARE_TYPE INITIAL_COUNTER = 3333U;

static void test_fuel_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow(void) {
    static constexpr uint32_t DURATION_OFFSET = 33;
    static constexpr uint32_t TIMEOUT_OFFSET = 77;

    raw_counter_t counter = {3333U};
    raw_compare_t compare = {0};
    FuelSchedule schedule(counter, compare);

    setFuelSchedule(schedule, TIMEOUT, DURATION);

    schedule.Status = RUNNING;
    CRANK_ANGLE_MAX_INJ = 360;

    // Negative test
    // Calculate a revolution time that will result in 360° taking longer than MAX_TIMER_PERIOD
    auto revTime = MAX_TIMER_PERIOD+(MAX_TIMER_PERIOD/CRANK_ANGLE_MAX_INJ);
    setAngleConverterRevolutionTime(revTime);
    TEST_ASSERT_GREATER_THAN(MAX_TIMER_PERIOD, angleToTimeMicroSecPerDegree((uint16_t)CRANK_ANGLE_MAX_INJ));
    setFuelSchedule(schedule, TIMEOUT, DURATION);
    // Should not have changed
    TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
    TEST_ASSERT_EQUAL(RUNNING, schedule.Status);
    TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION), schedule.duration);
    TEST_ASSERT_EQUAL(0, schedule.nextStartCompare);

    // Positive test
    setAngleConverterRevolutionTime(revTime/2U);
    TEST_ASSERT_LESS_THAN(MAX_TIMER_PERIOD, angleToTimeMicroSecPerDegree((uint32_t)CRANK_ANGLE_MAX_INJ));    
    setFuelSchedule(schedule, TIMEOUT+TIMEOUT_OFFSET, DURATION+DURATION_OFFSET);
    // Should not have changed
    TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
    // These should have changed
    TEST_ASSERT_EQUAL(RUNNING_WITHNEXT, schedule.Status);
    TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION+DURATION_OFFSET), schedule.duration);
    TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT+TIMEOUT_OFFSET), schedule.nextStartCompare);
}


void test_fuel_schedule(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_fuel_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow);
    }
}