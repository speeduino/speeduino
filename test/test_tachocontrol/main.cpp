#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "../test_utils.h"
#include "src/pins/boardOutputPin.h"
#include "src/controllers/tacho/tachoController.h"

namespace tachoControl_detail
{
  extern boardOutputPin_t tach_pin;
  extern tacho_control_state state;
}

using namespace tachoControl_detail;
extern uint16_t TACHO_SWEEP_RAMP_MS;
extern uint16_t TACHO_SWEEP_TIME_MS;

struct tacho_test_context
{
    statuses current = {};
    config2 page2 = {};
    config6 page6 = {};

    tacho_test_context(void)
    {
        page2.tachoDiv = 0U;
        page2.tachoDuration = 6U;

        current.rotationStatus = EngineRotationStatus::Stopped;
    }

    void initialiseTachoControl(void)
    {
        ::initialiseTachoControl(40, page2, page6, current);
    }

    void tachoControl(void)
    {
        ::tachoControl(current);
    }

    void run_n_intervals(unsigned n)
    {
        for (unsigned i = 0U; i < n; ++i) { tachoControl(); }
    }
};

static void test_initTacho_setsInactiveFlag(void)
{
  tacho_test_context context;
  state.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::ACTIVE;
  context.initialiseTachoControl();
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::INACTIVE, state.tachoOutputFlag);
}

static void test_tachoOutputOnOff(void)
{
  tacho_test_context context;

  // Dwell mode
  context.page6.tachoMode = 1U;
  context.initialiseTachoControl();
  TEST_ASSERT_TRUE(state.modeDwell);
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::INACTIVE, state.tachoOutputFlag);

  // IRL, these will set the pin
  tachoOutputOn();
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::INACTIVE, state.tachoOutputFlag);
  tachoOutputOff();
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::INACTIVE, state.tachoOutputFlag);

  // Fixed timing
  context.page6.tachoMode = 0U;
  context.initialiseTachoControl();
  TEST_ASSERT_FALSE(state.modeDwell);
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::INACTIVE, state.tachoOutputFlag);

  tachoOutputOn();
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::READY, state.tachoOutputFlag);
  state.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::INACTIVE;
  tachoOutputOff(); // Does nothing in fixed timing mode
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::INACTIVE, state.tachoOutputFlag);
}

static void test_tacho_sweep_post_ramp_branch(void)
{
  tacho_test_context context;
  context.initialiseTachoControl();

  // After the linear-ramp window, sweep adds tachoSweepIncr unconditionally.
  // Drive ms_counter past TACHO_SWEEP_RAMP_MS but stop before TACHO_SWEEP_TIME_MS
  // (which would auto-disable the sweep), then enable the sweep and step once.
  state.controlCounter = (unsigned long)TACHO_SWEEP_RAMP_MS + 10UL;
  state.tachoSweepEnabled = true;
  state.tachoSweepIncr = 100U;
  state.tachoSweepAccum = 0U;

  context.tachoControl();
  TEST_ASSERT_EQUAL_UINT16(100U, state.tachoSweepAccum);
  TEST_ASSERT_TRUE(state.tachoSweepEnabled);
}

static void test_tacho_sweep_disables_on_timeout(void)
{
  tacho_test_context context;
  context.initialiseTachoControl();

  state.controlCounter = (unsigned long)TACHO_SWEEP_TIME_MS - 1UL;
  state.tachoSweepEnabled = true;

  context.tachoControl();
  TEST_ASSERT_FALSE(state.tachoSweepEnabled);
}

static void test_tacho_ready_full_speed_to_active(void)
{
  tacho_test_context context;
  context.page2.tachoDiv = 0U;       // Full speed
  context.page2.tachoDuration = 5U;  // 5ms pulse
  context.initialiseTachoControl();
  TEST_ASSERT_FALSE(state.tachoAlt);
  
  state.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::READY;

  context.tachoControl();
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::ACTIVE, state.tachoOutputFlag);
  // tachoEndTime = (uint8_t)ms_counter + tachoDuration ; ms_counter is 1 after oneMSInterval()
  TEST_ASSERT_EQUAL_UINT8((uint8_t)(1U + 5U), state.tachoEndTime);
  // Alt flag flipped
  TEST_ASSERT_TRUE(state.tachoAlt);
}

static void test_tacho_ready_half_speed_skips(void)
{
  tacho_test_context context;
  context.page2.tachoDiv = 1U;          // Half speed
  context.initialiseTachoControl();
  TEST_ASSERT_FALSE(state.tachoAlt);
  state.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::READY;

  context.tachoControl();
  // tachoAlt false + tachoDiv != 0 hits the else branch -> set TACHO_INACTIVE
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::INACTIVE, state.tachoOutputFlag);
  TEST_ASSERT_TRUE(state.tachoAlt);
}

static void test_tacho_active_to_inactive_at_endtime(void)
{
  tacho_test_context context;
  context.initialiseTachoControl();
  // Drive ms_counter forward by 9 ticks first
  context.run_n_intervals(9);
  // Now ms_counter == 9. Schedule the tacho end at the next tick (ms_counter == 10).
  state.tachoEndTime = 10U;
  state.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::ACTIVE;

  context.tachoControl();
  TEST_ASSERT_EQUAL(tachoControl_detail::TachoOutputStatus::INACTIVE, state.tachoOutputFlag);
}

static void test_tacho_sweep_disables_when_running(void)
{
  tacho_test_context context;
  context.page2.useTachoSweep = 1;
  context.initialiseTachoControl();
  TEST_ASSERT_TRUE(state.tachoSweepEnabled);
  context.tachoControl();
  TEST_ASSERT_TRUE(state.tachoSweepEnabled);
  
  context = tacho_test_context();
  state.tachoSweepEnabled = true;
  context.current.rotationStatus = EngineRotationStatus::Running;
  context.tachoControl();
  TEST_ASSERT_FALSE(state.tachoSweepEnabled);

  context = tacho_test_context();
  state.tachoSweepEnabled = true;
  context.current.rotationStatus = EngineRotationStatus::Cranking;
  context.tachoControl();
  TEST_ASSERT_FALSE(state.tachoSweepEnabled);

  context = tacho_test_context();
  state.tachoSweepEnabled = true;
  state.controlCounter = 1500;
  context.tachoControl();
  TEST_ASSERT_FALSE(state.tachoSweepEnabled);
}

static void test_tacho_sweep_pulse_marks_ready(void)
{
  tacho_test_context context;

  state.tachoSweepEnabled = true;
  // Make accum overflow on the very first call: incr is added unconditionally during ramp.
  // Pre-load accum so the first ramp-mapped add tips it past MS_PER_SEC (1000).
  state.tachoSweepIncr = 1000U;
  state.tachoSweepAccum = 0U;
  state.controlCounter = 600UL;  // > TACHO_SWEEP_RAMP_MS (1000) is false -> use map() ramp branch

  context.tachoControl();
  // map(601, 0, 1000, 0, 1000) ~ 601, accum becomes 601 -> below MS_PER_SEC, not READY yet.
  // Run more iterations until accum overflows; oneMSInterval also flips READY -> ACTIVE
  // on the same call once ms_counter reaches TACHO_SWEEP_TIME_MS the sweep stops, so
  // bound the loop conservatively.
  bool sawPulse = (state.tachoOutputFlag == tachoControl_detail::TachoOutputStatus::READY) || (state.tachoOutputFlag == tachoControl_detail::TachoOutputStatus::ACTIVE);
  TEST_ASSERT_FALSE(sawPulse);
  for (unsigned i = 0U; i < 200U && !sawPulse; ++i)
  {
    context.tachoControl();
    sawPulse = (state.tachoOutputFlag == tachoControl_detail::TachoOutputStatus::READY) || (state.tachoOutputFlag == tachoControl_detail::TachoOutputStatus::ACTIVE);
  }
  TEST_ASSERT_TRUE(sawPulse);

  context = tacho_test_context();
  state.tachoSweepEnabled = true;
  state.tachoSweepIncr = 1000U;
  state.tachoSweepAccum = 0U;
  state.controlCounter = 1001UL;  // > TACHO_SWEEP_RAMP_MS (1000) is true -> use clamp
  context.tachoControl();
  sawPulse = (state.tachoOutputFlag == tachoControl_detail::TachoOutputStatus::READY) || (state.tachoOutputFlag == tachoControl_detail::TachoOutputStatus::ACTIVE);
  TEST_ASSERT_TRUE(sawPulse);
}

void runAllTests(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_initTacho_setsInactiveFlag);
    RUN_TEST_P(test_tacho_sweep_post_ramp_branch);
    RUN_TEST_P(test_tacho_sweep_disables_on_timeout);
    RUN_TEST_P(test_tacho_ready_full_speed_to_active);
    RUN_TEST_P(test_tacho_ready_half_speed_skips);
    RUN_TEST_P(test_tacho_active_to_inactive_at_endtime)
    RUN_TEST_P(test_tacho_sweep_disables_when_running);
    RUN_TEST_P(test_tacho_sweep_pulse_marks_ready);
    RUN_TEST_P(test_tachoOutputOnOff);
  }
}

TEST_HARNESS(runAllTests)
