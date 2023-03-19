
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

static void test_ignition_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow(void) {
  static constexpr uint32_t DURATION_OFFSET = 33;
  static constexpr uint32_t TIMEOUT_OFFSET = 77;

  raw_counter_t counter = {INITIAL_COUNTER};
  raw_compare_t compare = {0};
  IgnitionSchedule schedule(counter, compare);

  _setIgnitionScheduleDuration(schedule, TIMEOUT, DURATION);

  schedule.Status = RUNNING;
  CRANK_ANGLE_MAX_IGN = 360;

  // Negative test
  // Calculate a revolution time that will result in 360° taking longer than MAX_TIMER_PERIOD
  auto revTime = MAX_TIMER_PERIOD+(MAX_TIMER_PERIOD/CRANK_ANGLE_MAX_IGN);
  setAngleConverterRevolutionTime(revTime);
  TEST_ASSERT_GREATER_THAN_UINT32(MAX_TIMER_PERIOD, angleToTimeMicroSecPerDegree((uint16_t)CRANK_ANGLE_MAX_IGN));
  
  _setIgnitionScheduleDuration(schedule, TIMEOUT, DURATION);
  // Should not have changed
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
  TEST_ASSERT_EQUAL(RUNNING, schedule.Status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION), schedule.duration);
  TEST_ASSERT_EQUAL(0, schedule.nextStartCompare);

  // Positive test
  setAngleConverterRevolutionTime(revTime/2U);
  TEST_ASSERT_LESS_THAN(MAX_TIMER_PERIOD, angleToTimeMicroSecPerDegree((uint32_t)CRANK_ANGLE_MAX_INJ));    
  _setIgnitionScheduleDuration(schedule, TIMEOUT+TIMEOUT_OFFSET, DURATION+DURATION_OFFSET);
  // Should not have changed
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
  // These should have changed
  TEST_ASSERT_EQUAL(RUNNING_WITHNEXT, schedule.Status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION+DURATION_OFFSET), schedule.duration);
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT+TIMEOUT_OFFSET), schedule.nextStartCompare);
}

void test_ignition_schedule(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_ignition_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow);
    }
}