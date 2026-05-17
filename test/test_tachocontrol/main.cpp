#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "../test_utils.h"
#include "timers.h"
#include "globals.h"

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

static void run_n_intervals(unsigned n)
{
  for (unsigned i = 0U; i < n; ++i) { oneMSInterval(); }
}

static void reset_oneMSInterval_state(void)
{
  initTacho(40U);
  initialiseTimers();
  ms_counter = 0UL;
  lastRPM_100ms = 0U;
  tachoOutputFlag = TACHO_INACTIVE;
  tachoSweepAccum = 0U;
  tachoSweepIncr = 0U;
  tachoEndTime = 0U;

  // Default the global config state to "do nothing" branches inside oneMSInterval().
  configPage2.tachoDiv = 0U;
  configPage2.tachoDuration = 6U;

  currentStatus.rotationStatus = EngineRotationStatus::Stopped;
  currentStatus.tachoSweepEnabled = false;
  currentStatus.tachoAlt = false;
}

static void test_tacho_ready_full_speed_to_active(void)
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

static void test_tacho_ready_half_speed_skips(void)
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

static void test_tacho_active_to_inactive_at_endtime(void)
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

static void test_tacho_sweep_disables_when_running(void)
{
  reset_oneMSInterval_state();
  currentStatus.tachoSweepEnabled = true;
  oneMSInterval();
  TEST_ASSERT_TRUE(currentStatus.tachoSweepEnabled);
  
  reset_oneMSInterval_state();
  currentStatus.tachoSweepEnabled = true;
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  oneMSInterval();
  TEST_ASSERT_FALSE(currentStatus.tachoSweepEnabled);

  reset_oneMSInterval_state();
  currentStatus.tachoSweepEnabled = true;
  currentStatus.rotationStatus = EngineRotationStatus::Cranking;
  oneMSInterval();
  TEST_ASSERT_FALSE(currentStatus.tachoSweepEnabled);

  reset_oneMSInterval_state();
  currentStatus.tachoSweepEnabled = true;
  ms_counter = 1500;
  oneMSInterval();
  TEST_ASSERT_FALSE(currentStatus.tachoSweepEnabled);
}

static void test_tacho_sweep_pulse_marks_ready(void)
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
  TEST_ASSERT_FALSE(sawPulse);
  for (unsigned i = 0U; i < 200U && !sawPulse; ++i)
  {
    oneMSInterval();
    sawPulse = (tachoOutputFlag == READY) || (tachoOutputFlag == ACTIVE);
  }
  TEST_ASSERT_TRUE(sawPulse);

  reset_oneMSInterval_state();
  currentStatus.tachoSweepEnabled = true;
  tachoSweepIncr = 1000U;
  tachoSweepAccum = 0U;
  ms_counter = 1001UL;  // > TACHO_SWEEP_RAMP_MS (1000) is true -> use clamp
  oneMSInterval();
  sawPulse = (tachoOutputFlag == READY) || (tachoOutputFlag == ACTIVE);
  TEST_ASSERT_TRUE(sawPulse);
}

void runAllTests(void)
{
    RUN_TEST_P(test_tacho_ready_full_speed_to_active);
    RUN_TEST_P(test_tacho_ready_half_speed_skips);
    RUN_TEST_P(test_tacho_active_to_inactive_at_endtime)
    RUN_TEST_P(test_tacho_sweep_disables_when_running);
    RUN_TEST_P(test_tacho_sweep_pulse_marks_ready);
}

TEST_HARNESS(runAllTests)
