#include <Arduino.h>
#include <unity.h>
#include "schedule_calcs.h"
#include "../test_utils.h"
#include "type_traits.h"

using raw_counter_t = type_traits::remove_reference<IgnitionSchedule::counter_t>::type;
using raw_compare_t = type_traits::remove_reference<IgnitionSchedule::compare_t>::type;

struct test_subject_t
{
  raw_counter_t counterReg = {101};
  raw_compare_t compareReg = {100};
  IgnitionSchedule schedule;
  test_subject_t() : schedule(counterReg, compareReg) { }
#if defined(NATIVE_BOARD)
  test_subject_t(const test_subject_t &other) 
  : test_subject_t() 
  { 
    counterReg.store(other.counterReg.load());
    compareReg.store(other.compareReg.load());
  }
#endif
};

static test_subject_t setupSubject() {
  setAngleConverterRevolutionTime(6000000UL);
  return test_subject_t();
}

void test_adjust_crank_angle_pending_below_minrevolutions()
{
  auto subject = setupSubject();

  subject.schedule.Status = PENDING;
  currentStatus.startRevolutions = 0;

  subject.schedule.dischargeAngle = 359;

  // Should do nothing.
  adjustCrankAngle(subject.schedule, 180);
  TEST_ASSERT_EQUAL(100, subject.schedule._compare);
  TEST_ASSERT_EQUAL(101, subject.schedule._counter);
}


void test_adjust_crank_angle_pending_above_minrevolutions()
{
  auto subject = setupSubject();
  subject.schedule.Status = PENDING;  
  currentStatus.startRevolutions = 2000;

  constexpr uint16_t newCrankAngle = 180;
  constexpr uint16_t chargeAngle = 359;
  subject.schedule.chargeAngle = chargeAngle;

  adjustCrankAngle(subject.schedule, newCrankAngle);
  TEST_ASSERT_EQUAL(101, subject.schedule._counter);
  TEST_ASSERT_EQUAL(subject.schedule._counter+uS_TO_TIMER_COMPARE(angleToTimeMicroSecPerDegree(chargeAngle-newCrankAngle)), subject.schedule._compare);
}

void test_adjust_crank_angle_pending_above_minrevolutions_negative_angle()
{
  auto subject = setupSubject();
  subject.schedule.Status = PENDING;  
  currentStatus.startRevolutions = 2000;

  constexpr uint16_t newCrankAngle = 180;
  constexpr uint16_t chargeAngle = 100;
  subject.schedule.chargeAngle = chargeAngle;

  adjustCrankAngle(subject.schedule, newCrankAngle);
  TEST_ASSERT_EQUAL(101, subject.schedule._counter);
  TEST_ASSERT_EQUAL(100, subject.schedule._compare);
}

void test_adjust_crank_angle_running()
{
  auto subject = setupSubject();
  subject.schedule.Status = RUNNING;

  constexpr uint16_t newCrankAngle = 180;
  constexpr uint16_t chargeAngle = 359;
  subject.schedule.dischargeAngle = chargeAngle;

  adjustCrankAngle(subject.schedule, newCrankAngle);
  TEST_ASSERT_EQUAL(101, subject.schedule._counter);
  TEST_ASSERT_EQUAL(subject.schedule._counter+uS_TO_TIMER_COMPARE(angleToTimeMicroSecPerDegree(chargeAngle-newCrankAngle)), subject.schedule._compare);
}

void test_adjust_crank_angle_running_negative_angle()
{
  auto subject = setupSubject();
  subject.schedule.Status = RUNNING;

  constexpr uint16_t newCrankAngle = 180;
  constexpr uint16_t chargeAngle = 179;
  subject.schedule.dischargeAngle = chargeAngle;

  adjustCrankAngle(subject.schedule, newCrankAngle);
  TEST_ASSERT_EQUAL(101, subject.schedule._counter);
  TEST_ASSERT_EQUAL(100, subject.schedule._compare);
}

void test_adjust_crank_angle()
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_adjust_crank_angle_pending_below_minrevolutions);
    RUN_TEST(test_adjust_crank_angle_pending_above_minrevolutions);
    RUN_TEST(test_adjust_crank_angle_pending_above_minrevolutions_negative_angle);
    RUN_TEST(test_adjust_crank_angle_running);
    RUN_TEST(test_adjust_crank_angle_running_negative_angle);
  }
}