#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "type_traits.h"

using raw_counter_t = type_traits::remove_reference<IgnitionSchedule::counter_t>::type;
using raw_compare_t = type_traits::remove_reference<IgnitionSchedule::compare_t>::type;

static void test_disable(void)
{
  raw_counter_t counter = {0};
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  schedule.Status = OFF;
  disableSchedule(schedule);
  TEST_ASSERT_EQUAL(OFF, schedule.Status);

  schedule.Status = PENDING;
  disableSchedule(schedule);
  TEST_ASSERT_EQUAL(OFF, schedule.Status);

  // We do *not* stop a schedule part way through running.
  // I.e. injector has opened (but not clsosed) or coil is charging (but not discharged).
  schedule.Status = RUNNING;
  disableSchedule(schedule);
  TEST_ASSERT_EQUAL(RUNNING, schedule.Status);

  schedule.Status = RUNNING_WITHNEXT;
  disableSchedule(schedule);
  TEST_ASSERT_EQUAL(RUNNING, schedule.Status);
}

static constexpr uint32_t TIMEOUT = 5000U;
static constexpr uint32_t DURATION = 2000U;
static constexpr COMPARE_TYPE INITIAL_COUNTER = 3333U;

static void test_timeout_TooLarge(void) {
  raw_counter_t counter = { INITIAL_COUNTER };
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  TEST_ASSERT_EQUAL(OFF, schedule.Status);
  TEST_ASSERT_EQUAL(0, schedule.duration);
  setSchedule(schedule, MAX_TIMER_PERIOD, DURATION, true);
  TEST_ASSERT_EQUAL(OFF, schedule.Status);
  TEST_ASSERT_EQUAL(0, schedule.duration);
}

static void test_timeout_TooSmall(void) {
  raw_counter_t counter = { INITIAL_COUNTER };
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  TEST_ASSERT_EQUAL(OFF, schedule.Status);
  TEST_ASSERT_EQUAL(0, schedule.duration);
  setSchedule(schedule, 0U, DURATION, true);
  TEST_ASSERT_EQUAL(OFF, schedule.Status);
  TEST_ASSERT_EQUAL(0, schedule.duration);
}

static void test_duration_TooLarge(void) {
#if MAX_TIMER_PERIOD < UINT16_MAX //cppcheck-suppress misra-c2012-20.9
  raw_counter_t counter = { INITIAL_COUNTER };
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  TEST_ASSERT_EQUAL(OFF, schedule.Status);
  TEST_ASSERT_EQUAL(0, schedule.duration);
  setSchedule(schedule, TIMEOUT, MAX_TIMER_PERIOD+1UL, true);
  TEST_ASSERT_EQUAL(PENDING, schedule.Status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(MAX_TIMER_PERIOD - 1U), schedule.duration);
#else
  TEST_IGNORE_MESSAGE("Not applicable to this board");
#endif
}

static void test_duration_TooSmall(void) {
  raw_counter_t counter = { INITIAL_COUNTER };
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  TEST_ASSERT_EQUAL(OFF, schedule.Status);
  TEST_ASSERT_EQUAL(0, schedule.duration);
  setSchedule(schedule, TIMEOUT, 0U, true);
  TEST_ASSERT_EQUAL(OFF, schedule.Status);
  TEST_ASSERT_EQUAL(0, schedule.duration);
}

static void test_schedule_OFF_to_PENDING(void) {
  raw_counter_t counter = { INITIAL_COUNTER };
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  TEST_ASSERT_EQUAL(OFF, schedule.Status);
  TEST_ASSERT_EQUAL(0, schedule.duration);
  TEST_ASSERT_EQUAL(0, schedule.nextStartCompare);
  TEST_ASSERT_EQUAL(0, schedule._compare);
  TEST_ASSERT_EQUAL(INITIAL_COUNTER, schedule._counter);
  setSchedule(schedule, TIMEOUT, DURATION, true);
  TEST_ASSERT_EQUAL(PENDING, schedule.Status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION), schedule.duration);
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
}

static void test_schedule_PENDING_to_PENDING(void) {
  raw_counter_t counter = { INITIAL_COUNTER };
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  // setSchedule(schedule, TIMEOUT, DURATION, true);
  setSchedule(schedule, TIMEOUT+1000, DURATION+500, true);
  TEST_ASSERT_EQUAL(PENDING, schedule.Status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION+500), schedule.duration);
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT+1000), schedule._compare);
}

static void test_schedule_RUNNING_to_RUNNINGWITHNEXT(void) {
    static_assert(TIMEOUT>DURATION, "Test relies on timeout > duration");
    static constexpr uint32_t DURATION_OFFSET = 33;
    static constexpr uint32_t TIMEOUT_OFFSET = 77;

  raw_counter_t counter = { INITIAL_COUNTER };
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  setSchedule(schedule, TIMEOUT, DURATION, true);
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);

  schedule.Status = RUNNING;
  setSchedule(schedule, TIMEOUT+TIMEOUT_OFFSET, DURATION+DURATION_OFFSET, true);
  // Should not have changed
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
  // These should have changed
  TEST_ASSERT_EQUAL(RUNNING_WITHNEXT, schedule.Status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION+DURATION_OFFSET), schedule.duration);
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT+TIMEOUT_OFFSET), schedule.nextStartCompare);
}

static void test_schedule_RUNNINGWITHNEXT_to_RUNNINGWITHNEXT(void) 
{
    static_assert(TIMEOUT>DURATION, "Test relies on timeout > duration");
    static constexpr uint32_t DURATION_OFFSET = 33;
    static constexpr uint32_t TIMEOUT_OFFSET = 77;

  raw_counter_t counter = { INITIAL_COUNTER };
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  setSchedule(schedule, TIMEOUT, DURATION, true);
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);

  schedule.Status = RUNNING_WITHNEXT;
  setSchedule(schedule, TIMEOUT+TIMEOUT_OFFSET, DURATION+DURATION_OFFSET, true);
  // Should not have changed
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
  TEST_ASSERT_EQUAL(RUNNING_WITHNEXT, schedule.Status);
  // These should have changed
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION+DURATION_OFFSET), schedule.duration);
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT+TIMEOUT_OFFSET), schedule.nextStartCompare);
}

static void test_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow(void) {
  raw_counter_t counter = { INITIAL_COUNTER };
  raw_compare_t compare = {0};
  Schedule schedule(counter, compare);

  setSchedule(schedule, TIMEOUT, DURATION, true);
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);

  schedule.Status = RUNNING;
  setSchedule(schedule, TIMEOUT+77U, DURATION+66U, false /* This should prevent the schedule being queued*/);
  // Should not have changed
  TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
  TEST_ASSERT_EQUAL(RUNNING, schedule.Status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION), schedule.duration);
  TEST_ASSERT_EQUAL(0, schedule.nextStartCompare);
}

void test_schedule(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_disable);
    RUN_TEST_P(test_timeout_TooLarge);
    RUN_TEST_P(test_timeout_TooSmall);
    RUN_TEST_P(test_duration_TooLarge);
    RUN_TEST_P(test_duration_TooSmall);
    RUN_TEST_P(test_schedule_OFF_to_PENDING);
    RUN_TEST_P(test_schedule_PENDING_to_PENDING);
    RUN_TEST_P(test_schedule_RUNNING_to_RUNNINGWITHNEXT);
    RUN_TEST_P(test_schedule_RUNNINGWITHNEXT_to_RUNNINGWITHNEXT);
    RUN_TEST_P(test_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow);
  }
}
