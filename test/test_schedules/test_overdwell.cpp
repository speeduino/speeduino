// #include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

extern bool isOverDwellActive(const config4 &page4, const statuses &current);

static void test_isOverDwellActive_rpmBelowLimit(void) {
  config4 page4 = {};
  statuses current = {};
  page4.useDwellLim = true; // Enable dwell limiter
  page4.ignCranklock = true;
  current.RPM = 400;
  current.crankRPM = 420; // Set RPM to 400
  TEST_ASSERT_FALSE(isOverDwellActive(page4, current)); 
}

static void test_isOverDwellActive_rpmAboveLimit(void) {
  config4 page4 = {};
  statuses current = {};
  page4.useDwellLim = true; // Enable dwell limiter
  page4.ignCranklock = true;
  current.RPM = 420;
  current.crankRPM = 400; // Set RPM to 400
  TEST_ASSERT_TRUE(isOverDwellActive(page4, current)); 
}

static void test_isOverDwellActive_NoCrankLock(void) {
  config4 page4 = {};
  statuses current = {};
  page4.ignCranklock = false;
  page4.useDwellLim = true; // Enable dwell limiter

  current.RPM = 420;
  current.crankRPM = 400; // Set RPM to 400
  TEST_ASSERT_TRUE(isOverDwellActive(page4, current)); 

  current.RPM = 400;
  current.crankRPM = 420; // Set RPM to 400
  TEST_ASSERT_TRUE(isOverDwellActive(page4, current)); 
}

extern void applyChannelOverDwellProtection(IgnitionSchedule &schedule, uint32_t targetOverdwellTime);

static uint8_t counter = 0;
static void counter_callback(void) {
  ++counter;
}

static void test_applyChannelOverDwellProtection_notRunning(void) {
  IgnitionSchedule schedule(IGN1_COUNTER, IGN1_COMPARE);
  counter = 0;
  setCallbacks(schedule, counter_callback, counter_callback);

  schedule.Status = PENDING;
  schedule._startTime = 0; 
  applyChannelOverDwellProtection(schedule, 1000);
  TEST_ASSERT_EQUAL(0, counter); // Check that the callback was not called when the schedule is not running
}

static void test_applyChannelOverDwellProtection_running_notimeout(void) {
  IgnitionSchedule schedule(IGN1_COUNTER, IGN1_COMPARE);
  counter = 0;
  setCallbacks(schedule, counter_callback, counter_callback);
  
  schedule.Status = RUNNING;
  schedule._startTime = 2000; 
  applyChannelOverDwellProtection(schedule, 1000);
  TEST_ASSERT_EQUAL(0, counter); // Check that the callback was not called when the schedule is not running
}

static void test_applyChannelOverDwellProtection_running_timeout(void) {
  IgnitionSchedule schedule(IGN1_COUNTER, IGN1_COMPARE);
  counter = 0;
  setCallbacks(schedule, counter_callback, counter_callback);

  schedule.Status = RUNNING;
  schedule._startTime = 0; 
  applyChannelOverDwellProtection(schedule, 1000);
  TEST_ASSERT_EQUAL(1, counter); // Check that the callback was called when the schedule is running
}

void test_overdwell(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_isOverDwellActive_rpmAboveLimit);
    RUN_TEST(test_isOverDwellActive_rpmBelowLimit);
    RUN_TEST(test_isOverDwellActive_NoCrankLock);
    RUN_TEST(test_applyChannelOverDwellProtection_notRunning);
    RUN_TEST(test_applyChannelOverDwellProtection_running_notimeout);
    RUN_TEST(test_applyChannelOverDwellProtection_running_timeout);
  }
}