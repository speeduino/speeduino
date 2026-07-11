#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "src/stdlib/type_traits.h"

using raw_counter_t = type_traits::remove_reference<IgnitionSchedule::counter_t>::type;
using raw_compare_t = type_traits::remove_reference<IgnitionSchedule::compare_t>::type;

extern bool isOverDwellActive(const config4 &page4, const statuses &current);

static statuses rpmBelowLimit(void)
{
  statuses current = {};
  current.setRpm( 400);
  current.crankRPM = 420;
  return current;
}

static void test_isOverDwellActive_rpmBelowLimit(void) {
  config4 page4 = {};

  page4.useDwellLim = true; // Enable dwell limiter
  page4.ignCranklock = true;
  TEST_ASSERT_FALSE(isOverDwellActive(page4, rpmBelowLimit())); 

  page4.useDwellLim = true;
  page4.ignCranklock = false;
  TEST_ASSERT_TRUE(isOverDwellActive(page4, rpmBelowLimit())); 

  page4.useDwellLim = false;
  page4.ignCranklock = true;
  TEST_ASSERT_FALSE(isOverDwellActive(page4, rpmBelowLimit())); 

  page4.useDwellLim = false;
  page4.ignCranklock = false;
  TEST_ASSERT_FALSE(isOverDwellActive(page4, rpmBelowLimit())); 
}

static statuses rpmAboveLimit(void)
{
  statuses current = {};
  current.setRpm( 420);
  current.crankRPM = 400;
  return current;
}

static void test_isOverDwellActive_rpmAboveLimit(void) {
  config4 page4 = {};

  page4.useDwellLim = true;
  page4.ignCranklock = true;
  TEST_ASSERT_TRUE(isOverDwellActive(page4, rpmAboveLimit())); 

  page4.useDwellLim = true;
  page4.ignCranklock = false;
  TEST_ASSERT_TRUE(isOverDwellActive(page4, rpmAboveLimit())); 

  page4.useDwellLim = false;
  page4.ignCranklock = true;
  TEST_ASSERT_FALSE(isOverDwellActive(page4, rpmAboveLimit())); 

  page4.useDwellLim = false;
  page4.ignCranklock = false;
  TEST_ASSERT_FALSE(isOverDwellActive(page4, rpmAboveLimit())); 
}

extern void applyChannelOverDwellProtection(IgnitionSchedule &schedule, uint32_t now, uint32_t dwellLimit_uS);

static uint8_t counter = 0;
static void counter_callback(void) {
  ++counter;
}

static void test_applyChannelOverDwellProtection_notRunning(void) {
  raw_counter_t counterReg = {101};
  raw_compare_t compareReg = {100};
  IgnitionSchedule schedule(counterReg, compareReg);

  counter = 0;
  setCallbacks(schedule, counter_callback, counter_callback);

  schedule._status = PENDING;
  schedule._startTime = 0;
  applyChannelOverDwellProtection(schedule, 2000, 1000);
  TEST_ASSERT_EQUAL(0, counter); // Check that the callback was not called when the schedule is not running
}

static void test_applyChannelOverDwellProtection_running_notimeout(void) {
  raw_counter_t counterReg = {101};
  raw_compare_t compareReg = {100};
  IgnitionSchedule schedule(counterReg, compareReg);

  counter = 0;
  setCallbacks(schedule, counter_callback, counter_callback);

  schedule._status = RUNNING;
  schedule._startTime = 2000;
  applyChannelOverDwellProtection(schedule, 2500, 1000);
  TEST_ASSERT_EQUAL(0, counter); // Check that the callback was not called when the dwell limit has not elapsed
}

static void test_applyChannelOverDwellProtection_running_timeout(void) {
  raw_counter_t counterReg = {101};
  raw_compare_t compareReg = {100};
  IgnitionSchedule schedule(counterReg, compareReg);

  counter = 0;
  setCallbacks(schedule, counter_callback, counter_callback);

  schedule._status = RUNNING;
  schedule._startTime = 0;
  applyChannelOverDwellProtection(schedule, 2000, 1000);
  TEST_ASSERT_EQUAL(1, counter); // Check that the callback was called when the schedule is running
}

static void test_applyChannelOverDwellProtection_running_notimeout_rollover(void) {
  raw_counter_t counterReg = {101};
  raw_compare_t compareReg = {100};
  IgnitionSchedule schedule(counterReg, compareReg);

  counter = 0;
  setCallbacks(schedule, counter_callback, counter_callback);

  schedule._status = RUNNING;
  schedule._startTime = UINT32_MAX - 500U; // Dwell started just before the micros() wrap
  applyChannelOverDwellProtection(schedule, 400U, 1000U); // 901 uS elapsed across the wrap
  TEST_ASSERT_EQUAL(0, counter); // Dwell limit not reached: the coil must not be cut
}

static void test_applyChannelOverDwellProtection_running_timeout_rollover(void) {
  raw_counter_t counterReg = {101};
  raw_compare_t compareReg = {100};
  IgnitionSchedule schedule(counterReg, compareReg);

  counter = 0;
  setCallbacks(schedule, counter_callback, counter_callback);

  schedule._status = RUNNING;
  schedule._startTime = UINT32_MAX - 500U; // Dwell started just before the micros() wrap
  applyChannelOverDwellProtection(schedule, 600U, 1000U); // 1101 uS elapsed across the wrap
  TEST_ASSERT_EQUAL(1, counter); // Dwell limit exceeded: the coil must be cut
}

void test_overdwell(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_isOverDwellActive_rpmAboveLimit);
    RUN_TEST_P(test_isOverDwellActive_rpmBelowLimit);
    RUN_TEST_P(test_applyChannelOverDwellProtection_notRunning);
    RUN_TEST_P(test_applyChannelOverDwellProtection_running_notimeout);
    RUN_TEST_P(test_applyChannelOverDwellProtection_running_timeout);
    RUN_TEST_P(test_applyChannelOverDwellProtection_running_notimeout_rollover);
    RUN_TEST_P(test_applyChannelOverDwellProtection_running_timeout_rollover);
  }
}