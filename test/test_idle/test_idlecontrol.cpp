#include "../test_utils.h"
#include "idle.h"
#include "prepare_idle.h"
#include "units.h"

// These tests drive idleControl() through the open-loop stepper path far
// enough to execute updateIdleStepAndLoad() (the helper extracted from the two
// previously-duplicated idle branches). idleStepper is file-static, so we can't
// read curIdleStep directly; instead we assert on currentStatus.idleLoad, which
// the helper writes. A sentinel value is stored first to prove the helper ran.
//
// The cranking-table lookup is loaded with a large step count so the computed
// target exceeds the configured max: that exercises the helper's clamp branch,
// and the two maxSteps regimes cover both sides of its "does (iacMaxSteps * 3)
// exceed a byte?" resolution branch. With forced homing curIdleStep starts at 0,
// so idleLoad reads back as 0 (it is latched from curIdleStep before doStep()).

static void prepare_stepper_homed(uint8_t algorithm, uint8_t maxSteps)
{
  prepare_idle(algorithm);
  configPage6.iacStepHome = 0U;   // completedHomeSteps(0) < 0 is false -> already homed
  configPage9.iacMaxSteps = maxSteps;
  configPage6.iacStepHyster = 0U;

  // Load the cranking step table so the lookup returns a large value. The target
  // (returned value * 3) then exceeds iacMaxSteps * 3 for both regimes below,
  // so the helper's clamp branch actually fires.
  for (uint8_t i = 0U; i < 4U; i++)
  {
    configPage6.iacCrankBins[i] = i;     // all below the lookup key -> clamp to last value
    configPage6.iacCrankSteps[i] = 120U; // 120 * 3 = 360 target
  }

  // Not "Running" -> the cranking branch sets the target directly, with no
  // loop-timer gate, so the helper is reached deterministically on this call.
  currentStatus.rotationStatus = EngineRotationStatus::Cranking;
  currentStatus.coolant = 80;
  initialiseIdle(true);           // forcehoming: curIdleStep = 0, status = SOFF
}

static void test_idleControl_step_ol_clamp_directResolution(void)
{
  // Open-loop stepper. iacMaxSteps * 3 == 60: target 360 is clamped down to 60,
  // and 60 fits in a byte -> idleLoad = curIdleStep.
  prepare_stepper_homed(IAC_ALGORITHM_STEP_OL, 20U);
  currentStatus.idleLoad = 0xFFU; // sentinel: must be overwritten by the helper
  idleControl();
  TEST_ASSERT_EQUAL(0U, currentStatus.idleLoad);
}

static void test_idleControl_step_ol_clamp_halfResolution(void)
{
  // Open-loop stepper. iacMaxSteps * 3 == 300: target 360 is clamped down to
  // 300, which exceeds a byte -> idleLoad = curIdleStep / 2.
  prepare_stepper_homed(IAC_ALGORITHM_STEP_OL, 100U);
  currentStatus.idleLoad = 0xFFU; // sentinel: must be overwritten by the helper
  idleControl();
  TEST_ASSERT_EQUAL(0U, currentStatus.idleLoad);
}

static void test_idleControl_step_cl_reachesHelper(void)
{
  // Closed-loop stepper reaches the same helper from the other (previously
  // duplicated) call site, via its cranking sub-path.
  prepare_stepper_homed(IAC_ALGORITHM_STEP_CL, 20U);
  currentStatus.idleLoad = 0xFFU; // sentinel: must be overwritten by the helper
  idleControl();
  TEST_ASSERT_EQUAL(0U, currentStatus.idleLoad);
}

void testIdleControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_idleControl_step_ol_clamp_directResolution);
    RUN_TEST(test_idleControl_step_ol_clamp_halfResolution);
    RUN_TEST(test_idleControl_step_cl_reachesHelper);
  }
}
