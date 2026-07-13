#include "../test_utils.h"
#include "globals.h"
#include "timers.h"
#include "auxiliaries.h"
#include "setup_oneMsInterval.h"

extern volatile byte loop5ms;
extern volatile byte loop20ms;
extern volatile byte loop33ms;
extern volatile byte loop66ms;
extern volatile byte loop100ms;
extern volatile byte loop250ms;
extern volatile int loopSec;
extern volatile uint16_t lastRPM_100ms;
extern volatile uint8_t TIMER_mask;

static void test_increments_counters_and_sets_1KHz(void)
{
  setup_oneMsInterval();
  oneMSInterval();
  TEST_ASSERT_EQUAL_UINT32(1UL, ms_counter);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_1KHZ));
  TEST_ASSERT_EQUAL_UINT8(1U, loop5ms);
  TEST_ASSERT_EQUAL_UINT8(1U, loop20ms);
  TEST_ASSERT_EQUAL_UINT8(1U, loop33ms);
  TEST_ASSERT_EQUAL_UINT8(1U, loop66ms);
  TEST_ASSERT_EQUAL_UINT8(1U, loop100ms);
  TEST_ASSERT_EQUAL_UINT8(1U, loop250ms);
  TEST_ASSERT_EQUAL_INT(1, loopSec);
}

static void test_5ms_boundary_sets_200Hz(void)
{
  setup_oneMsInterval();
  run_n_intervals(5);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_200HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop5ms);
  TEST_ASSERT_TRUE(BIT_CHECK( getAndClearTimerMask(), BIT_TIMER_200HZ));
  TEST_ASSERT_FALSE(BIT_CHECK(TIMER_mask, BIT_TIMER_200HZ));
}

static void test_20ms_boundary_sets_50Hz(void)
{
  setup_oneMsInterval();
  run_n_intervals(20);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_50HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop20ms);
  TEST_ASSERT_TRUE(BIT_CHECK( getAndClearTimerMask(), BIT_TIMER_50HZ));
  TEST_ASSERT_FALSE(BIT_CHECK(TIMER_mask, BIT_TIMER_50HZ));
}

static void test_33ms_boundary_sets_30Hz(void)
{
  setup_oneMsInterval();
  run_n_intervals(33);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_30HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop33ms);
  TEST_ASSERT_TRUE(BIT_CHECK( getAndClearTimerMask(), BIT_TIMER_30HZ));
  TEST_ASSERT_FALSE(BIT_CHECK(TIMER_mask, BIT_TIMER_30HZ));
}

static void test_66ms_boundary_sets_15Hz(void)
{
  setup_oneMsInterval();
  run_n_intervals(66);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_15HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop66ms);
  TEST_ASSERT_TRUE(BIT_CHECK( getAndClearTimerMask(), BIT_TIMER_15HZ));
  TEST_ASSERT_FALSE(BIT_CHECK(TIMER_mask, BIT_TIMER_15HZ));
}

static void test_100ms_boundary_updates_rpmDOT(void)
{
  setup_oneMsInterval();
  currentStatus.RPM = 1500U;
  lastRPM_100ms = 1000U;

  run_n_intervals(100);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_10HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop100ms);
  TEST_ASSERT_EQUAL_INT(5000, currentStatus.rpmDOT);  // (1500-1000)*10
  TEST_ASSERT_EQUAL_UINT16(1500U, lastRPM_100ms);
}

static void test_250ms_boundary_sets_4Hz(void)
{
  setup_oneMsInterval();
  run_n_intervals(250);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_4HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop250ms);
  TEST_ASSERT_TRUE(BIT_CHECK( getAndClearTimerMask(), BIT_TIMER_4HZ));
  TEST_ASSERT_FALSE(BIT_CHECK(TIMER_mask, BIT_TIMER_4HZ));
}

static void test_1Hz_boundary(void)
{
  setup_oneMsInterval();
  mainLoopCount = 7777U;
  currentStatus.secl = 100U;

  run_n_intervals(1000);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_1HZ));
  TEST_ASSERT_EQUAL_INT(0, loopSec);
  TEST_ASSERT_TRUE(BIT_CHECK( getAndClearTimerMask(), BIT_TIMER_1HZ));
  TEST_ASSERT_FALSE(BIT_CHECK(TIMER_mask, BIT_TIMER_1HZ));

  TEST_ASSERT_EQUAL_UINT8(101U, currentStatus.secl);
  TEST_ASSERT_EQUAL_UINT16(7777U, currentStatus.loopsPerSecond);
  TEST_ASSERT_EQUAL_UINT16(0U, mainLoopCount);
  // crankRPM = configPage4.crankRPM * 10
  TEST_ASSERT_EQUAL_UINT16(400U, currentStatus.crankRPM);
}

static void test_runSecs_increments_when_running(void)
{
  setup_oneMsInterval();
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  currentStatus.runSecs = 10U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(11U, currentStatus.runSecs);
}

static void test_runSecs_caps_at_255(void)
{
  setup_oneMsInterval();
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  currentStatus.runSecs = 255U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(255U, currentStatus.runSecs);
}

static void test_runSecsX10_running_at_10Hz(void)
{
  setup_oneMsInterval();
  currentStatus.rotationStatus = EngineRotationStatus::Running;

  run_n_intervals(100);
  TEST_ASSERT_EQUAL_UINT32(1UL, runSecsX10);

  run_n_intervals(100);
  TEST_ASSERT_EQUAL_UINT32(2UL, runSecsX10);
}

static void test_runSecsX10_resets_when_not_running(void)
{
  setup_oneMsInterval();
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  run_n_intervals(100);
  TEST_ASSERT_EQUAL_UINT32(1UL, runSecsX10);

  currentStatus.rotationStatus = EngineRotationStatus::Stopped;
  run_n_intervals(100);
  TEST_ASSERT_EQUAL_UINT32(0UL, runSecsX10);
}

static void test_fanControl_runs_at_1Hz(void)
{
  setup_oneMsInterval();
  configPage2.fanEnable = 1U;          // Regular on/off fan control path
  configPage2.fanWhenOff = false;      // Engine-running gates fan
  currentStatus.rotationStatus = EngineRotationStatus::Stopped;
  currentStatus.coolant = 0;            // Way below any sane fan threshold
  configPage6.fanSP = 200U;
  configPage6.fanHyster = 5U;
  currentStatus.fanOn = true;          // Will get cleared by fanOff()

  run_n_intervals(1000);
  TEST_ASSERT_FALSE(currentStatus.fanOn);
}

static void test_inj_priming_runs_when_rpm_zero(void)
{
  setup_oneMsInterval();
  currentStatus.injPrimed = false;
  currentStatus.initialisationComplete = true;
  currentStatus.RPM = 0U;
  currentStatus.numPrimaryInjOutputs = 0U;    // Skip actual fuel-schedule dispatch
  currentStatus.numSecondaryInjOutputs = 0U;    // Skip actual fuel-schedule dispatch
  configPage2.primingDelay = 0U;        // seclx10 >= 0 always

  run_n_intervals(100);                 // Crosses 10Hz boundary, hits priming branch
  TEST_ASSERT_TRUE(currentStatus.injPrimed);
}

void testOneMsInterval(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_increments_counters_and_sets_1KHz);
    RUN_TEST(test_5ms_boundary_sets_200Hz);
    RUN_TEST(test_20ms_boundary_sets_50Hz);
    RUN_TEST(test_33ms_boundary_sets_30Hz);
    RUN_TEST(test_66ms_boundary_sets_15Hz);
    RUN_TEST(test_100ms_boundary_updates_rpmDOT);
    RUN_TEST(test_250ms_boundary_sets_4Hz);
    RUN_TEST(test_1Hz_boundary);
    RUN_TEST(test_runSecs_increments_when_running);
    RUN_TEST(test_runSecs_caps_at_255);
    RUN_TEST(test_runSecsX10_running_at_10Hz);
    RUN_TEST(test_runSecsX10_resets_when_not_running);
    RUN_TEST(test_fanControl_runs_at_1Hz);
    RUN_TEST(test_inj_priming_runs_when_rpm_zero);
  }
}
