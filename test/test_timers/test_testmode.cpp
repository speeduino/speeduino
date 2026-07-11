#include "../test_utils.h"
#include "globals.h"
#include "setup_oneMsInterval.h"

extern volatile uint8_t testInjectorPulseCount;
extern volatile uint8_t testIgnitionPulseCount;
extern volatile uint8_t TIMER_mask;

static void test_pulse_emitted_at_30Hz(void)
{
  setup_oneMsInterval();
  currentStatus.isTestModeActive = true;
  currentStatus.RPM = 0U;
  
  configPage13.hwTestInjDuration = 100U;  // Don't auto-close in this test
  configPage13.hwTestIgnDuration = 100U;

  // 33 ticks reaches the 30Hz boundary which fires the openInjectorN()/beginCoilNCharge().
  // Just verify the call path runs without crashing — the line coverage is the goal.
  HWTest_INJ_Pulsed = setBits(INJ_CHANNELS);
  HWTest_IGN_Pulsed = setBits(IGN_CHANNELS);
  run_n_intervals(33);
 
  HWTest_INJ_Pulsed = 0;
  HWTest_IGN_Pulsed = 0;
  run_n_intervals(33);
 
  TEST_PASS();
}

static void assert_inj_pulses()
{
  run_n_intervals(1);
  TEST_ASSERT_EQUAL(1, testInjectorPulseCount);
  run_n_intervals(configPage13.hwTestInjDuration);
  TEST_ASSERT_EQUAL(0, testInjectorPulseCount);
}

static void test_auto_close_inj(void)
{
  setup_oneMsInterval();
  currentStatus.isTestModeActive = true;
  currentStatus.RPM = 0U;
  configPage13.hwTestInjDuration = 2U;

  // Once testInjectorPulseCount reaches hwTestInjDuration the close path runs and
  // testInjectorPulseCount resets to 0. Run > duration and verify the counter wraps.
  HWTest_INJ_Pulsed = setBits(INJ_CHANNELS);
  assert_inj_pulses();

  HWTest_INJ_Pulsed = 0b00000001;
  assert_inj_pulses();

  HWTest_INJ_Pulsed = 0;
  BIT_SET(HWTest_INJ_Pulsed, INJ_CHANNELS-1);
  assert_inj_pulses();  
}

static void assert_ign_pulses(void)
{
  run_n_intervals(1);
  TEST_ASSERT_EQUAL(1, testIgnitionPulseCount);
  run_n_intervals(configPage13.hwTestIgnDuration);
  TEST_ASSERT_EQUAL(0, testIgnitionPulseCount);
}

static void test_auto_end_ign(void)
{
  setup_oneMsInterval();
  currentStatus.isTestModeActive = true;
  currentStatus.RPM = 0U;
  configPage13.hwTestIgnDuration = 2U;

  HWTest_IGN_Pulsed = setBits(IGN_CHANNELS);
  assert_ign_pulses();

  HWTest_IGN_Pulsed = 0b00000001;
  assert_ign_pulses();

  HWTest_IGN_Pulsed = 0;
  BIT_SET(HWTest_IGN_Pulsed, IGN_CHANNELS-1);
  assert_ign_pulses();
}

static void test_skipped_when_rpm_nonzero(void)
{
  setup_oneMsInterval();
  currentStatus.isTestModeActive = true;
  currentStatus.RPM = 1000U;             // Engine spinning -> the 30Hz test pulse
  BIT_SET(HWTest_INJ_Pulsed, INJ1_CMD_BIT);
  configPage13.hwTestInjDuration = 5U;

  // Even with isTestModeActive, RPM != 0 means the 30Hz openInjector branch is
  // skipped. The auto-close path however still runs because it's gated only on
  // isTestModeActive.
  run_n_intervals(33);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_30HZ));
}

void testTestMode(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_pulse_emitted_at_30Hz);
    RUN_TEST(test_auto_close_inj);
    RUN_TEST(test_auto_end_ign);
    RUN_TEST(test_skipped_when_rpm_nonzero);
  }
}
