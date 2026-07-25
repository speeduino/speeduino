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
// With forced homing curIdleStep starts at 0, so idleLoad is expected to be 0
// in both max-steps regimes; the two tests exist to cover both sides of the
// "does (iacMaxSteps * 3) exceed a byte?" branch inside the helper.

static void prepare_stepper_homed(uint8_t maxSteps)
{
  prepare_idle(IAC_ALGORITHM_STEP_OL);
  configPage6.iacStepHome = 0U;   // completedHomeSteps(0) < 0 is false -> already homed
  configPage9.iacMaxSteps = maxSteps;
  configPage6.iacStepHyster = 0U;
  // Not "Running" -> the cranking branch sets the target directly, with no
  // loop-timer gate, so the helper is reached deterministically on this call.
  currentStatus.rotationStatus = EngineRotationStatus::Cranking;
  currentStatus.coolant = 80;
  initialiseIdle(true);           // forcehoming: curIdleStep = 0, status = SOFF
}

static void test_idleControl_step_ol_load_directResolution(void)
{
  // iacMaxSteps * 3 == 60, which fits in a byte -> idleLoad = curIdleStep.
  prepare_stepper_homed(20U);
  currentStatus.idleLoad = 0xFFU; // sentinel: must be overwritten by the helper
  idleControl();
  TEST_ASSERT_EQUAL(0U, currentStatus.idleLoad);
}

static void test_idleControl_step_ol_load_halfResolution(void)
{
  // iacMaxSteps * 3 == 300, which exceeds a byte -> idleLoad = curIdleStep / 2.
  prepare_stepper_homed(100U);
  currentStatus.idleLoad = 0xFFU; // sentinel: must be overwritten by the helper
  idleControl();
  TEST_ASSERT_EQUAL(0U, currentStatus.idleLoad);
}

void testIdleControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_idleControl_step_ol_load_directResolution);
    RUN_TEST(test_idleControl_step_ol_load_halfResolution);
  }
}
