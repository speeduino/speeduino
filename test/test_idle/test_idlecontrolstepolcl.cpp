#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "context.h"

extern idleController::detail::state_t _idleState;

static context_t setup_olcl(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_STEP_OLCL);
  context.page6.idleKP = 0U;
  context.page6.idleKI = 0U;
  context.page6.idleKD = 0U;
  context.page6.iacStepHyster = 0U;
  context.page6.iacStepHome = 0U;
  context.page9.iacMaxSteps = 100U;
  return context;
}

static void test_runningTaper(void)
{
  auto context = setup_olcl();
  context.page2.idleTaperTime = 3U;
  context.init();

  context.current.coolant = 110;
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;
  _idleState.idleTaper = 1U;

  idleControl();

  TEST_ASSERT_EQUAL_INT(295, _idleState.idleStepper.targetIdleStep);
  TEST_ASSERT_EQUAL_UINT8(2U, _idleState.idleTaper);
}

static void test_runningUsesFeedForward(void)
{
  auto context = setup_olcl();
  context.page2.idleTaperTime = 1U;
  context.init();

  context.current.coolant = 50;
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;
  _idleState.idleTaper = 1U;

  idleControl();

  TEST_ASSERT_EQUAL_INT(720, _idleState.FeedForwardTerm);
  TEST_ASSERT_EQUAL_INT(180, _idleState.idleStepper.targetIdleStep);
}

void testIdleControlStepOlCl(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_runningTaper);
  RUN_TEST_P(test_runningUsesFeedForward);
}