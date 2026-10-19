#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "maths.h"
#include "units.h"
#include "context.h"

extern idleController::detail::state_t _idleState;

static void test_idleUpInputAndOutput(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_NONE);
  context.page2.idleUpEnabled = true;
  context.page2.idleUpOutputEnabled = true;
  context.init();

  digitalWrite(TEST_IDLEUP_INPUT_PIN, LOW);
  idleControl();
  TEST_ASSERT_TRUE(context.current.idleUpActive);
#if !defined(__AVR__)
  TEST_ASSERT_EQUAL_UINT8(HIGH, digitalRead(TEST_IDLEUP_OUTPUT_PIN));
#endif

  digitalWrite(TEST_IDLEUP_INPUT_PIN, HIGH);
  idleControl();
  TEST_ASSERT_FALSE(context.current.idleUpActive);
#if !defined(__AVR__)
  TEST_ASSERT_EQUAL_UINT8(LOW, digitalRead(TEST_IDLEUP_OUTPUT_PIN));
#endif

  context.page2.idleUpPolarity = 1U;
  digitalWrite(TEST_IDLEUP_INPUT_PIN, HIGH);
  idleControl();
  TEST_ASSERT_TRUE(context.current.idleUpActive);
#if !defined(__AVR__)
  TEST_ASSERT_EQUAL_UINT8(HIGH, digitalRead(TEST_IDLEUP_OUTPUT_PIN));
#endif

  digitalWrite(TEST_IDLEUP_INPUT_PIN, LOW);
  idleControl();
#if !defined(__AVR__)
  TEST_ASSERT_FALSE(context.current.idleUpActive);
  TEST_ASSERT_EQUAL_UINT8(LOW, digitalRead(TEST_IDLEUP_OUTPUT_PIN));
#endif
}

static void test_basic_coverage_allModes(void)
{
  for (auto mode : { IAC_ALGORITHM_NONE, IAC_ALGORITHM_ONOFF, IAC_ALGORITHM_PWM_OL, IAC_ALGORITHM_PWM_CL, IAC_ALGORITHM_STEP_OL, IAC_ALGORITHM_STEP_CL, IAC_ALGORITHM_PWM_OLCL, IAC_ALGORITHM_STEP_OLCL })
  {
    context_t context;
    context.prepare_idle(mode);
    idleControl();
    TEST_ASSERT_EQUAL(mode, _idleState.idleInitComplete);
  }
}

static uint8_t transitionMode = IAC_ALGORITHM_NONE;
static void test_transitions(void)
{
    context_t context;
    context.prepare_idle(transitionMode);
    context.page6.iacStepHome = 0U;
    context.page9.iacCoolTime = 0U;
    context.page6.iacStepTime = 0U;
    context.page6.iacPWMrun = true;
    context.page9.iacMaxSteps = 100U;
    context.current.coolant = 50U;
    context.init();
    TEST_ASSERT_EQUAL(transitionMode, _idleState.idleInitComplete);

    const uint8_t ITERS = 15;
    const uint32_t TIME_STEP = 500;
    context.current.rotationStatus = EngineRotationStatus::Stopped;
    uint32_t time = 1000;
    // Steppers need a couple of iterations to "move"
    time = context.idleControl(time, TIME_STEP, ITERS);
    uint8_t stoppedLoad = context.current.idleLoad;
    TEST_ASSERT_NOT_EQUAL(0, stoppedLoad);

    context.current.rotationStatus = EngineRotationStatus::Cranking;
    time = context.idleControl(time, TIME_STEP, ITERS);
    uint8_t crankingLoad = context.current.idleLoad;
    TEST_ASSERT_NOT_EQUAL(0, crankingLoad);

    context.current.rotationStatus = EngineRotationStatus::Running;
    time = context.idleControl(time, TIME_STEP, ITERS);
    uint8_t runningLoad = context.current.idleLoad;
    TEST_ASSERT_NOT_EQUAL(0, runningLoad);
    TEST_ASSERT_NOT_EQUAL(crankingLoad, runningLoad);
}

static void test_transitions_allModes(void)
{
  for (auto mode : { IAC_ALGORITHM_PWM_OL, IAC_ALGORITHM_PWM_CL, IAC_ALGORITHM_STEP_OL, IAC_ALGORITHM_STEP_CL, IAC_ALGORITHM_PWM_OLCL, IAC_ALGORITHM_STEP_OLCL })
  {
    transitionMode = mode;
    char szMsg[64];
    sprintf(szMsg, "%" PRIu8, transitionMode);
    RUN_TEST_POSTFIX_P(test_transitions, szMsg)
  }
}

void testIdleControl(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_idleUpInputAndOutput);
  RUN_TEST_P(test_basic_coverage_allModes);

  test_transitions_allModes();
}
