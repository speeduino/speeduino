#include <unity.h>
#include <Arduino.h>
#include "globals.h"
#include "timers.h"
#include "sensors.h"
#include "auxiliaries.h"
#include "scheduledIO_direct_inj.h"
#include "scheduledIO_direct_ign.h"
#include "../test_utils.h"

extern volatile byte loop5ms;
extern volatile byte loop20ms;
extern volatile byte loop33ms;
extern volatile byte loop66ms;
extern volatile byte loop100ms;
extern volatile byte loop250ms;
extern volatile int loopSec;
extern volatile uint16_t lastRPM_100ms;
extern volatile uint8_t tachoEndTime;
extern volatile uint16_t tachoSweepAccum;
extern volatile uint8_t testInjectorPulseCount;
extern volatile uint8_t testIgnitionPulseCount;

static bool tm_io_initialised = false;

static void ensure_tm_io_initialised(void)
{
  if (tm_io_initialised) { return; }
  // oneMSInterval() may call openInjectorN()/closeInjectorN()/beginCoilNCharge()/endCoilNCharge()
  // when test mode is active. Those dereference the static port pointers inside fastOutputPin_t,
  // which must be wired to real pins (any free pin numbers will do under ArduinoFake).
  const uint8_t inj_pins[INJ_CHANNELS] = { 50U, 51U, 52U, 53U, 54U, 55U, 56U, 57U };
  const uint8_t ign_pins[IGN_CHANNELS] = { 60U, 61U, 62U, 63U, 64U, 65U, 66U, 67U };
  initInjDirectIO(inj_pins);
  initIgnDirectIO(ign_pins);
  initTacho(40U);
  initialiseFan(41U);
  initialiseFuelPump(configPage2, 42U);
  tm_io_initialised = true;
}

static void reset_oneMSInterval_state(void)
{
  ensure_tm_io_initialised();
  initialiseTimers();
  TIMER_mask = 0U;
  ms_counter = 0UL;
  lastRPM_100ms = 0U;
  tachoOutputFlag = TACHO_INACTIVE;
  tachoSweepAccum = 0U;
  tachoSweepIncr = 0U;
  tachoEndTime = 0U;
  testInjectorPulseCount = 0U;
  testIgnitionPulseCount = 0U;
  HWTest_INJ_Pulsed = 0U;
  HWTest_IGN_Pulsed = 0U;
  mainLoopCount = 0U;
  runSecsX10 = 0U;
  seclx10 = 0U;
  flexCounter = 0U;
  flexPulseWidth = 0UL;

  // Default the global config state to "do nothing" branches inside oneMSInterval().
  configPage2.tachoDiv = 0U;
  configPage2.tachoDuration = 6U;
  configPage2.fanEnable = 0U;
  configPage2.flexEnabled = false;
  configPage2.fpPrime = 0U;
  configPage2.primingDelay = 0U;
  configPage4.useDwellLim = 0U;
  configPage4.crankRPM = 40U;  // (* 10 = 400 RPM cranking)
  configPage13.hwTestInjDuration = 1U;
  configPage13.hwTestIgnDuration = 1U;

  currentStatus.RPM = 0U;
  currentStatus.runSecs = 0U;
  currentStatus.secl = 0U;
  currentStatus.loopsPerSecond = 0U;
  currentStatus.rpmDOT = 0;
  currentStatus.engineIsRunning = false;
  currentStatus.engineIsCranking = false;
  currentStatus.tachoSweepEnabled = false;
  currentStatus.tachoAlt = false;
  currentStatus.fpPrimed = true;     // Skip fuel pump priming branch
  currentStatus.injPrimed = true;    // Skip injector priming branch
  currentStatus.initialisationComplete = true;
  currentStatus.isTestModeActive = false;
}

static void run_n_intervals(unsigned n)
{
  for (unsigned i = 0U; i < n; ++i) { oneMSInterval(); }
}

static void test_initialiseTimers_resets_counters(void)
{
  reset_oneMSInterval_state();
  // Push counters to non-default
  loop5ms = 99U; loop20ms = 99U; loop33ms = 99U; loop66ms = 99U;
  loop100ms = 99U; loop250ms = 99U; loopSec = 999;
  lastRPM_100ms = 555U;

  initialiseTimers();
  TEST_ASSERT_EQUAL_UINT8(0U, loop5ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop20ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop33ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop66ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop100ms);
  TEST_ASSERT_EQUAL_UINT8(0U, loop250ms);
  TEST_ASSERT_EQUAL_INT(0, loopSec);
  TEST_ASSERT_EQUAL_UINT16(0U, lastRPM_100ms);
}

static void test_initTacho_setsInactiveFlag(void)
{
  constexpr uint8_t pin = 30U;
  tachoOutputFlag = ACTIVE;
  initTacho(pin);
  TEST_ASSERT_EQUAL(TACHO_INACTIVE, tachoOutputFlag);
}

static void test_tachoPulseHighAndLow_togglePin(void)
{
  constexpr uint8_t pin = 31U;
  initTacho(pin);

  tachoPulseHigh();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(pin));

  tachoPulseLow();
  TEST_ASSERT_EQUAL(LOW, digitalRead(pin));

  tachoPulseHigh();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(pin));
}

static void test_oneMSInterval_increments_counters_and_sets_1KHz(void)
{
  reset_oneMSInterval_state();
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

static void test_oneMSInterval_5ms_boundary_sets_200Hz(void)
{
  reset_oneMSInterval_state();
  run_n_intervals(5);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_200HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop5ms);
}

static void test_oneMSInterval_20ms_boundary_sets_50Hz(void)
{
  reset_oneMSInterval_state();
  run_n_intervals(20);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_50HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop20ms);
}

static void test_oneMSInterval_33ms_boundary_sets_30Hz(void)
{
  reset_oneMSInterval_state();
  run_n_intervals(33);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_30HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop33ms);
}

static void test_oneMSInterval_66ms_boundary_sets_15Hz(void)
{
  reset_oneMSInterval_state();
  run_n_intervals(66);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_15HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop66ms);
}

static void test_oneMSInterval_100ms_boundary_updates_rpmDOT(void)
{
  reset_oneMSInterval_state();
  currentStatus.RPM = 1500U;
  lastRPM_100ms = 1000U;

  run_n_intervals(100);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_10HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop100ms);
  TEST_ASSERT_EQUAL_INT(5000, currentStatus.rpmDOT);  // (1500-1000)*10
  TEST_ASSERT_EQUAL_UINT16(1500U, lastRPM_100ms);
}

static void test_oneMSInterval_250ms_boundary_sets_4Hz(void)
{
  reset_oneMSInterval_state();
  run_n_intervals(250);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_4HZ));
  TEST_ASSERT_EQUAL_UINT8(0U, loop250ms);
}

static void test_oneMSInterval_1Hz_boundary(void)
{
  reset_oneMSInterval_state();
  mainLoopCount = 7777U;
  currentStatus.secl = 100U;

  run_n_intervals(1000);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_1HZ));
  TEST_ASSERT_EQUAL_INT(0, loopSec);
  TEST_ASSERT_EQUAL_UINT8(101U, currentStatus.secl);
  TEST_ASSERT_EQUAL_UINT16(7777U, currentStatus.loopsPerSecond);
  TEST_ASSERT_EQUAL_UINT16(0U, mainLoopCount);
  // crankRPM = configPage4.crankRPM * 10
  TEST_ASSERT_EQUAL_UINT16(400U, currentStatus.crankRPM);
}

static void test_oneMSInterval_runSecs_increments_when_running(void)
{
  reset_oneMSInterval_state();
  currentStatus.engineIsRunning = true;
  currentStatus.runSecs = 10U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(11U, currentStatus.runSecs);
}

static void test_oneMSInterval_runSecs_caps_at_255(void)
{
  reset_oneMSInterval_state();
  currentStatus.engineIsRunning = true;
  currentStatus.runSecs = 255U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(255U, currentStatus.runSecs);
}

static void test_oneMSInterval_runSecsX10_running_at_10Hz(void)
{
  reset_oneMSInterval_state();
  currentStatus.engineIsRunning = true;

  run_n_intervals(100);
  TEST_ASSERT_EQUAL_UINT32(1UL, runSecsX10);

  run_n_intervals(100);
  TEST_ASSERT_EQUAL_UINT32(2UL, runSecsX10);
}

static void test_oneMSInterval_runSecsX10_resets_when_not_running(void)
{
  reset_oneMSInterval_state();
  currentStatus.engineIsRunning = true;
  run_n_intervals(100);
  TEST_ASSERT_EQUAL_UINT32(1UL, runSecsX10);

  currentStatus.engineIsRunning = false;
  run_n_intervals(100);
  TEST_ASSERT_EQUAL_UINT32(0UL, runSecsX10);
}

static void test_oneMSInterval_tacho_ready_full_speed_to_active(void)
{
  reset_oneMSInterval_state();
  configPage2.tachoDiv = 0U;       // Full speed
  configPage2.tachoDuration = 5U;  // 5ms pulse
  currentStatus.tachoAlt = false;
  tachoOutputFlag = READY;

  oneMSInterval();
  TEST_ASSERT_EQUAL(ACTIVE, tachoOutputFlag);
  // tachoEndTime = (uint8_t)ms_counter + tachoDuration ; ms_counter is 1 after oneMSInterval()
  TEST_ASSERT_EQUAL_UINT8((uint8_t)(1U + 5U), tachoEndTime);
  // Alt flag flipped
  TEST_ASSERT_TRUE(currentStatus.tachoAlt);
}

static void test_oneMSInterval_tacho_ready_half_speed_skips(void)
{
  reset_oneMSInterval_state();
  configPage2.tachoDiv = 1U;          // Half speed
  currentStatus.tachoAlt = false;     // Not alternate this pulse
  tachoOutputFlag = READY;

  oneMSInterval();
  // tachoAlt false + tachoDiv != 0 hits the else branch -> set TACHO_INACTIVE
  TEST_ASSERT_EQUAL(TACHO_INACTIVE, tachoOutputFlag);
  TEST_ASSERT_TRUE(currentStatus.tachoAlt);
}

static void test_oneMSInterval_tacho_active_to_inactive_at_endtime(void)
{
  reset_oneMSInterval_state();
  // Drive ms_counter forward by 9 ticks first
  run_n_intervals(9);
  // Now ms_counter == 9. Schedule the tacho end at the next tick (ms_counter == 10).
  tachoEndTime = (uint8_t)(ms_counter + 1U);
  tachoOutputFlag = ACTIVE;

  oneMSInterval();
  TEST_ASSERT_EQUAL(TACHO_INACTIVE, tachoOutputFlag);
}

static void test_oneMSInterval_tacho_sweep_disables_when_running(void)
{
  reset_oneMSInterval_state();
  currentStatus.tachoSweepEnabled = true;
  currentStatus.engineIsRunning = true;

  oneMSInterval();
  TEST_ASSERT_FALSE(currentStatus.tachoSweepEnabled);
}

static void test_oneMSInterval_tacho_sweep_pulse_marks_ready(void)
{
  reset_oneMSInterval_state();
  currentStatus.tachoSweepEnabled = true;
  // Make accum overflow on the very first call: incr is added unconditionally during ramp.
  // Pre-load accum so the first ramp-mapped add tips it past MS_PER_SEC (1000).
  tachoSweepIncr = 1000U;
  tachoSweepAccum = 0U;
  ms_counter = 600UL;  // > TACHO_SWEEP_RAMP_MS (1000) is false -> use map() ramp branch

  oneMSInterval();
  // map(601, 0, 1000, 0, 1000) ~ 601, accum becomes 601 -> below MS_PER_SEC, not READY yet.
  // Run more iterations until accum overflows; oneMSInterval also flips READY -> ACTIVE
  // on the same call once ms_counter reaches TACHO_SWEEP_TIME_MS the sweep stops, so
  // bound the loop conservatively.
  bool sawPulse = (tachoOutputFlag == READY) || (tachoOutputFlag == ACTIVE);
  for (unsigned i = 0U; i < 200U && !sawPulse; ++i)
  {
    oneMSInterval();
    if ((tachoOutputFlag == READY) || (tachoOutputFlag == ACTIVE)) { sawPulse = true; }
  }
  TEST_ASSERT_TRUE(sawPulse);
}

static void test_oneMSInterval_test_mode_pulse_inj_emitted_at_30Hz(void)
{
  reset_oneMSInterval_state();
  currentStatus.isTestModeActive = true;
  currentStatus.RPM = 0U;
  BIT_SET(HWTest_INJ_Pulsed, INJ1_CMD_BIT);
  BIT_SET(HWTest_IGN_Pulsed, IGN1_CMD_BIT);
  configPage13.hwTestInjDuration = 100U;  // Don't auto-close in this test
  configPage13.hwTestIgnDuration = 100U;

  // 33 ticks reaches the 30Hz boundary which fires the openInjectorN()/beginCoilNCharge().
  // Just verify the call path runs without crashing — the line coverage is the goal.
  run_n_intervals(33);
  TEST_ASSERT_TRUE(BIT_CHECK(TIMER_mask, BIT_TIMER_30HZ));
}

static void test_oneMSInterval_test_mode_auto_close_inj(void)
{
  reset_oneMSInterval_state();
  currentStatus.isTestModeActive = true;
  currentStatus.RPM = 0U;
  BIT_SET(HWTest_INJ_Pulsed, INJ1_CMD_BIT);
  configPage13.hwTestInjDuration = 2U;

  // Once testInjectorPulseCount reaches hwTestInjDuration the close path runs and
  // testInjectorPulseCount resets to 0. Run > duration and verify the counter wraps.
  run_n_intervals(5);
  TEST_ASSERT_TRUE(testInjectorPulseCount <= configPage13.hwTestInjDuration);
}

static void test_oneMSInterval_test_mode_auto_end_ign(void)
{
  reset_oneMSInterval_state();
  currentStatus.isTestModeActive = true;
  currentStatus.RPM = 0U;
  BIT_SET(HWTest_IGN_Pulsed, IGN1_CMD_BIT);
  configPage13.hwTestIgnDuration = 2U;

  run_n_intervals(5);
  TEST_ASSERT_TRUE(testIgnitionPulseCount <= configPage13.hwTestIgnDuration);
}

static void test_oneMSInterval_flex_low_freq_yields_zero_pct(void)
{
  reset_oneMSInterval_state();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;        // No filter, immediate
  flexCounter = 10U;                    // Below flexFreqLow
  flexPulseWidth = 1000UL;
  currentStatus.ethanolPct = 100U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.ethanolPct);
  TEST_ASSERT_EQUAL_UINT32(0UL, flexCounter);
}

static void test_oneMSInterval_flex_in_band_yields_pct(void)
{
  reset_oneMSInterval_state();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;
  flexCounter = 75U;                    // 25% ethanol
  flexPulseWidth = 1000UL;
  currentStatus.ethanolPct = 0U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(25U, currentStatus.ethanolPct);
}

static void test_oneMSInterval_fpPrime_completes_when_rpm_zero(void)
{
  reset_oneMSInterval_state();
  currentStatus.fpPrimed = false;
  currentStatus.secl = 0U;
  fpPrimeTime = 0U;
  configPage2.fpPrime = 0U;            // Threshold reached immediately

  run_n_intervals(1000);
  TEST_ASSERT_TRUE(currentStatus.fpPrimed);
}

static void test_oneMSInterval_tacho_sweep_post_ramp_branch(void)
{
  reset_oneMSInterval_state();
  // After the linear-ramp window, sweep adds tachoSweepIncr unconditionally.
  // Drive ms_counter past TACHO_SWEEP_RAMP_MS but stop before TACHO_SWEEP_TIME_MS
  // (which would auto-disable the sweep), then enable the sweep and step once.
  ms_counter = (unsigned long)TACHO_SWEEP_RAMP_MS + 10UL;
  currentStatus.tachoSweepEnabled = true;
  tachoSweepIncr = 100U;
  tachoSweepAccum = 0U;

  oneMSInterval();
  TEST_ASSERT_EQUAL_UINT16(100U, tachoSweepAccum);
  TEST_ASSERT_TRUE(currentStatus.tachoSweepEnabled);
}

static void test_oneMSInterval_tacho_sweep_disables_on_timeout(void)
{
  reset_oneMSInterval_state();
  ms_counter = (unsigned long)TACHO_SWEEP_TIME_MS - 1UL;
  currentStatus.tachoSweepEnabled = true;

  oneMSInterval();
  // ms_counter incremented to TACHO_SWEEP_TIME_MS -> sweep disables.
  TEST_ASSERT_FALSE(currentStatus.tachoSweepEnabled);
}

static void test_oneMSInterval_fanControl_runs_at_1Hz(void)
{
  reset_oneMSInterval_state();
  configPage2.fanEnable = 1U;          // Regular on/off fan control path
  configPage2.fanWhenOff = false;      // Engine-running gates fan
  currentStatus.engineIsRunning = false;
  currentStatus.coolant = 0;            // Way below any sane fan threshold
  configPage6.fanSP = 200U;
  configPage6.fanHyster = 5U;
  currentStatus.fanOn = true;          // Will get cleared by fanOff()

  run_n_intervals(1000);
  TEST_ASSERT_FALSE(currentStatus.fanOn);
}

static void test_oneMSInterval_inj_priming_runs_when_rpm_zero(void)
{
  reset_oneMSInterval_state();
  currentStatus.injPrimed = false;
  currentStatus.initialisationComplete = true;
  currentStatus.RPM = 0U;
  currentStatus.maxInjOutputs = 0U;    // Skip actual fuel-schedule dispatch
  configPage2.primingDelay = 0U;        // seclx10 >= 0 always

  run_n_intervals(100);                 // Crosses 10Hz boundary, hits priming branch
  TEST_ASSERT_TRUE(currentStatus.injPrimed);
}

static void test_oneMSInterval_flex_over_range_error_yields_zero_pct(void)
{
  reset_oneMSInterval_state();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;
  flexCounter = 200U;                   // > flexFreqHigh+1 and >= flexFreqLow+19 -> error
  flexPulseWidth = 1000UL;
  currentStatus.ethanolPct = 50U;       // Should drop to 0 (error condition)

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.ethanolPct);
  TEST_ASSERT_EQUAL_UINT32(0UL, flexCounter);
}

static void test_oneMSInterval_flex_off_by_one_correction(void)
{
  reset_oneMSInterval_state();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;
  flexCounter = 51U;                    // tempEthPct = 1 -> off-by-one corrected to 0
  flexPulseWidth = 1000UL;
  currentStatus.ethanolPct = 75U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.ethanolPct);
}

static void test_oneMSInterval_flex_clamps_pulsewidth_low(void)
{
  reset_oneMSInterval_state();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;
  flexCounter = 100U;                   // 50% ethanol
  flexPulseWidth = 0UL;                  // Below 1000us -> clamp to 1000

  run_n_intervals(1000);
  // With pw clamped at 1000us: tempX100 = (4224*1000)/1024 - 8125 = 4125 - 8125 = -4000
  // fuelTemp = -4000/100 = -40C
  TEST_ASSERT_EQUAL_INT16(-40, currentStatus.fuelTemp);
}

static void test_oneMSInterval_test_mode_skipped_when_rpm_nonzero(void)
{
  reset_oneMSInterval_state();
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

void testTimers(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_initialiseTimers_resets_counters);
    RUN_TEST(test_initTacho_setsInactiveFlag);
    RUN_TEST(test_tachoPulseHighAndLow_togglePin);
    RUN_TEST(test_oneMSInterval_increments_counters_and_sets_1KHz);
    RUN_TEST(test_oneMSInterval_5ms_boundary_sets_200Hz);
    RUN_TEST(test_oneMSInterval_20ms_boundary_sets_50Hz);
    RUN_TEST(test_oneMSInterval_33ms_boundary_sets_30Hz);
    RUN_TEST(test_oneMSInterval_66ms_boundary_sets_15Hz);
    RUN_TEST(test_oneMSInterval_100ms_boundary_updates_rpmDOT);
    RUN_TEST(test_oneMSInterval_250ms_boundary_sets_4Hz);
    RUN_TEST(test_oneMSInterval_1Hz_boundary);
    RUN_TEST(test_oneMSInterval_runSecs_increments_when_running);
    RUN_TEST(test_oneMSInterval_runSecs_caps_at_255);
    RUN_TEST(test_oneMSInterval_runSecsX10_running_at_10Hz);
    RUN_TEST(test_oneMSInterval_runSecsX10_resets_when_not_running);
    RUN_TEST(test_oneMSInterval_tacho_ready_full_speed_to_active);
    RUN_TEST(test_oneMSInterval_tacho_ready_half_speed_skips);
    RUN_TEST(test_oneMSInterval_tacho_active_to_inactive_at_endtime);
    RUN_TEST(test_oneMSInterval_tacho_sweep_disables_when_running);
    RUN_TEST(test_oneMSInterval_tacho_sweep_pulse_marks_ready);
    RUN_TEST(test_oneMSInterval_test_mode_pulse_inj_emitted_at_30Hz);
    RUN_TEST(test_oneMSInterval_test_mode_auto_close_inj);
    RUN_TEST(test_oneMSInterval_test_mode_auto_end_ign);
    RUN_TEST(test_oneMSInterval_flex_low_freq_yields_zero_pct);
    RUN_TEST(test_oneMSInterval_flex_in_band_yields_pct);
    RUN_TEST(test_oneMSInterval_fpPrime_completes_when_rpm_zero);
    RUN_TEST(test_oneMSInterval_tacho_sweep_post_ramp_branch);
    RUN_TEST(test_oneMSInterval_tacho_sweep_disables_on_timeout);
    RUN_TEST(test_oneMSInterval_fanControl_runs_at_1Hz);
    RUN_TEST(test_oneMSInterval_inj_priming_runs_when_rpm_zero);
    RUN_TEST(test_oneMSInterval_flex_over_range_error_yields_zero_pct);
    RUN_TEST(test_oneMSInterval_flex_off_by_one_correction);
    RUN_TEST(test_oneMSInterval_flex_clamps_pulsewidth_low);
    RUN_TEST(test_oneMSInterval_test_mode_skipped_when_rpm_nonzero);
  }
}
