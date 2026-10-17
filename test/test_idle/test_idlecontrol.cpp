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
  TEST_ASSERT_TRUE(context.current.idleUpOutputActive);
#if !defined(__AVR__)
  TEST_ASSERT_EQUAL_UINT8(HIGH, digitalRead(TEST_IDLEUP_OUTPUT_PIN));
#endif

  digitalWrite(TEST_IDLEUP_INPUT_PIN, HIGH);
  idleControl();
  TEST_ASSERT_FALSE(context.current.idleUpActive);
  TEST_ASSERT_FALSE(context.current.idleUpOutputActive);
#if !defined(__AVR__)
  TEST_ASSERT_EQUAL_UINT8(LOW, digitalRead(TEST_IDLEUP_OUTPUT_PIN));
#endif

  context.page2.idleUpPolarity = 1U;
  digitalWrite(TEST_IDLEUP_INPUT_PIN, HIGH);
  idleControl();
  TEST_ASSERT_TRUE(context.current.idleUpActive);
  TEST_ASSERT_TRUE(context.current.idleUpOutputActive);
#if !defined(__AVR__)
  TEST_ASSERT_EQUAL_UINT8(HIGH, digitalRead(TEST_IDLEUP_OUTPUT_PIN));
#endif

  digitalWrite(TEST_IDLEUP_INPUT_PIN, LOW);
  idleControl();
#if !defined(__AVR__)
  TEST_ASSERT_FALSE(context.current.idleUpActive);
  TEST_ASSERT_FALSE(context.current.idleUpOutputActive);
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

void testIdleControl(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_idleUpInputAndOutput);
  RUN_TEST_P(test_basic_coverage_allModes);
}
