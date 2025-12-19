
#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

  template<typename _Tp>
    struct remove_reference
    { typedef _Tp   type; };

  template<typename _Tp>
    struct remove_reference<_Tp&>
    { typedef _Tp   type; };

  template<typename _Tp>
    struct remove_reference<_Tp&&>
    { typedef _Tp   type; };

static constexpr uint32_t TIMEOUT = 5000U;
static constexpr uint32_t DURATION = 2000U;
static constexpr COMPARE_TYPE INITIAL_COUNTER = 3333U;

static void test_timeout_TooLarge(void) {
    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    Schedule schedule(COUNTER, COMPARE);
    TEST_ASSERT_EQUAL(OFF, schedule.Status);
    TEST_ASSERT_EQUAL(0, schedule.duration);
    setSchedule(schedule, MAX_TIMER_PERIOD, DURATION, true);
    TEST_ASSERT_EQUAL(OFF, schedule.Status);
    TEST_ASSERT_EQUAL(0, schedule.duration);
}

static void test_timeout_TooSmall(void) {
    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    Schedule schedule(COUNTER, COMPARE);
    TEST_ASSERT_EQUAL(OFF, schedule.Status);
    TEST_ASSERT_EQUAL(0, schedule.duration);
    setSchedule(schedule, 0U, DURATION, true);
    TEST_ASSERT_EQUAL(OFF, schedule.Status);
    TEST_ASSERT_EQUAL(0, schedule.duration);
}

static void test_duration_TooLarge(void) {
#if MAX_TIMER_PERIOD < UINT16_MAX //cppcheck-suppress misra-c2012-20.9
    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    Schedule schedule(COUNTER, COMPARE);
    TEST_ASSERT_EQUAL(OFF, schedule.Status);
    TEST_ASSERT_EQUAL(0, schedule.duration);
    setSchedule(schedule, TIMEOUT, MAX_TIMER_PERIOD+1UL, true);
    TEST_ASSERT_EQUAL(PENDING, schedule.Status);
    TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(MAX_TIMER_PERIOD - 1U), schedule.duration);
#endif
}

static void test_duration_TooSmall(void) {
    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    Schedule schedule(COUNTER, COMPARE);
    TEST_ASSERT_EQUAL(OFF, schedule.Status);
    TEST_ASSERT_EQUAL(0, schedule.duration);
    setSchedule(schedule, TIMEOUT, 0U, true);
    TEST_ASSERT_EQUAL(OFF, schedule.Status);
    TEST_ASSERT_EQUAL(0, schedule.duration);
}

static void test_schedule_OFF_to_PENDING(void) {
    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    Schedule schedule(COUNTER, COMPARE);
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
    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    Schedule schedule(COUNTER, COMPARE);
    setSchedule(schedule, TIMEOUT, DURATION, true);
    setSchedule(schedule, TIMEOUT+1000, DURATION+500, true);
    TEST_ASSERT_EQUAL(PENDING, schedule.Status);
    TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION+500), schedule.duration);
    TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT+1000), schedule._compare);
}

static void test_schedule_RUNNING_to_RUNNINGWITHNEXT(void) {
    static_assert(TIMEOUT>DURATION, "Test relies on timeout > duration");
    static constexpr uint32_t DURATION_OFFSET = 33;
    static constexpr uint32_t TIMEOUT_OFFSET = 77;

    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    Schedule schedule(COUNTER, COMPARE);
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

    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    Schedule schedule(COUNTER, COMPARE);
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
    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    Schedule schedule(COUNTER, COMPARE);
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

static void test_ignition_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow(void) {
    static constexpr uint32_t DURATION_OFFSET = 33;
    static constexpr uint32_t TIMEOUT_OFFSET = 77;

    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    IgnitionSchedule schedule(COUNTER, COMPARE);
    setIgnitionSchedule(schedule, TIMEOUT, DURATION);

    schedule.Status = RUNNING;
    CRANK_ANGLE_MAX_IGN = 360;

    // Negative test
    // Calculate a revolution time that will result in 360° taking longer than MAX_TIMER_PERIOD
    auto revTime = MAX_TIMER_PERIOD+(MAX_TIMER_PERIOD/CRANK_ANGLE_MAX_IGN);
    setAngleConverterRevolutionTime(revTime);
    TEST_ASSERT_GREATER_THAN(MAX_TIMER_PERIOD, angleToTimeMicroSecPerDegree((uint16_t)CRANK_ANGLE_MAX_IGN));
    
    setIgnitionSchedule(schedule, TIMEOUT, DURATION);
    // Should not have changed
    TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
    TEST_ASSERT_EQUAL(RUNNING, schedule.Status);
    TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION), schedule.duration);
    TEST_ASSERT_EQUAL(0, schedule.nextStartCompare);

    // Positive test
    setAngleConverterRevolutionTime(revTime/2U);
    TEST_ASSERT_LESS_THAN(MAX_TIMER_PERIOD, angleToTimeMicroSecPerDegree((uint32_t)CRANK_ANGLE_MAX_INJ));    
    setIgnitionSchedule(schedule, TIMEOUT+TIMEOUT_OFFSET, DURATION+DURATION_OFFSET);
    // Should not have changed
    TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT), schedule._compare);
    // These should have changed
    TEST_ASSERT_EQUAL(RUNNING_WITHNEXT, schedule.Status);
    TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(DURATION+DURATION_OFFSET), schedule.duration);
    TEST_ASSERT_EQUAL(INITIAL_COUNTER + uS_TO_TIMER_COMPARE(TIMEOUT+TIMEOUT_OFFSET), schedule.nextStartCompare);
}

static void test_fuel_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow(void) {
    static constexpr uint32_t DURATION_OFFSET = 33;
    static constexpr uint32_t TIMEOUT_OFFSET = 77;

    remove_reference<Schedule::counter_t>::type COUNTER=INITIAL_COUNTER;
    remove_reference<Schedule::compare_t>::type COMPARE=0;
    FuelSchedule schedule(COUNTER, COMPARE);
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


void test_setSchedule(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_timeout_TooLarge);
        RUN_TEST_P(test_timeout_TooSmall);
        RUN_TEST_P(test_duration_TooLarge);
        RUN_TEST_P(test_duration_TooSmall);
        RUN_TEST_P(test_schedule_OFF_to_PENDING);
        RUN_TEST_P(test_schedule_PENDING_to_PENDING);
        RUN_TEST_P(test_schedule_RUNNING_to_RUNNINGWITHNEXT);
        RUN_TEST_P(test_schedule_RUNNINGWITHNEXT_to_RUNNINGWITHNEXT);
        RUN_TEST_P(test_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow);
        RUN_TEST_P(test_ignition_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow);
        RUN_TEST_P(test_fuel_schedule_RUNNING_to_RUNNINGWITHNEXT_Disallow);
    }
}