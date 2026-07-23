#include "../test_utils.h"
#include "timers.h"
#include "globals.h"
#include "setup_oneMsInterval.h"
#include "src/pins/boardOutputPin.h"

 extern boardOutputPin_t tach_pin;
extern volatile uint8_t tachoEndTime;
extern volatile uint16_t tachoSweepAccum;

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
  TEST_ASSERT_TRUE(tach_pin._pin.isPinHigh());

  tachoPulseLow();
  TEST_ASSERT_TRUE(tach_pin._pin.isPinLow());

  tachoPulseHigh();
  TEST_ASSERT_TRUE(tach_pin._pin.isPinHigh());
}

static void test_tacho_ready_full_speed_to_active(void)
{
  setup_oneMsInterval();
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
  setup_oneMsInterval();
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
  setup_oneMsInterval();
  // Drive ms_counter forward by 9 ticks first
  run_n_intervals(9U);
  // Now ms_counter == 9. Schedule the tacho end at the next tick (ms_counter == 10).
  tachoEndTime = (uint8_t)(ms_counter + 1U);
  tachoOutputFlag = ACTIVE;

  oneMSInterval();
  TEST_ASSERT_EQUAL(TACHO_INACTIVE, tachoOutputFlag);
}

static void test_tacho_sweep_disables_when_running(void)
{
  setup_oneMsInterval();
  currentStatus.tachoSweepEnabled = true;
  currentStatus.rotationStatus = EngineRotationStatus::Running;

  oneMSInterval();
  TEST_ASSERT_FALSE(currentStatus.tachoSweepEnabled);
}

static void test_tacho_sweep_pulse_marks_ready(void)
{
  setup_oneMsInterval();
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


static void test_tacho_sweep_post_ramp_branch(void)
{
  setup_oneMsInterval();
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

static void test_tacho_sweep_disables_on_timeout(void)
{
  setup_oneMsInterval();
  ms_counter = (unsigned long)TACHO_SWEEP_TIME_MS - 1UL;
  currentStatus.tachoSweepEnabled = true;

  oneMSInterval();
  // ms_counter incremented to TACHO_SWEEP_TIME_MS -> sweep disables.
  TEST_ASSERT_FALSE(currentStatus.tachoSweepEnabled);
}

void testTacho(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_initTacho_setsInactiveFlag);
    RUN_TEST(test_tachoPulseHighAndLow_togglePin);
    RUN_TEST(test_tacho_ready_full_speed_to_active);
    RUN_TEST(test_tacho_ready_half_speed_skips);
    RUN_TEST(test_tacho_active_to_inactive_at_endtime);
    RUN_TEST(test_tacho_sweep_disables_when_running);
    RUN_TEST(test_tacho_sweep_pulse_marks_ready);
    RUN_TEST(test_tacho_sweep_post_ramp_branch);
    RUN_TEST(test_tacho_sweep_disables_on_timeout);
  }
}