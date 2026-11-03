#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "maths.h"
#include "units.h"
#include "context.h"

extern idleController::detail::state_t _idleState;

static void prepare_stepper_disable(context_t &context)
{
  context.prepare_idle(IAC_ALGORITHM_STEP_OL);
  context.page9.iacMaxSteps = 100U;
  context.page6.iacStepHome = 0U;
  context.current.coolant = 50U;
}

static void test_disableIdle_pwm_normal(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.iacPWMdir = 0U;
  context.page6.iacChannels = 0U;
  context.init();

  context.current.idleOn = true;
  context.current.idleLoad = 50U;
  disableIdle();

  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinLow());
}

static void test_disableIdle_pwm_reversed(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.iacPWMdir = 1U;
  context.page6.iacChannels = 0U;
  context.init();

  context.current.idleOn = true;
  context.current.idleLoad = 50U;
  disableIdle();

  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
}

static void test_disableIdle_pwm_dual_channel_normal(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.iacPWMdir = 0U;
  context.page6.iacChannels = 1U;
  context.init();

  context.current.idleOn = true;
  context.current.idleLoad = 50U;
  disableIdle();

  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinLow());
  TEST_ASSERT_TRUE(_idleState.idle2_pin._pin.isPinHigh());
}

static void test_disableIdle_pwm_dual_channel_reversed(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.iacPWMdir = 1U;
  context.page6.iacChannels = 1U;
  context.init();

  context.current.idleOn = true;
  context.current.idleLoad = 50U;
  disableIdle();

  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
  TEST_ASSERT_TRUE(_idleState.idle2_pin._pin.isPinLow());
}

static void test_disableIdle_none_is_noop(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_NONE);
  context.init();

  context.current.idleOn = true;
  context.current.idleLoad = 50U;
  disableIdle();

  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
}

static void test_disableIdle_stepper_moves_to_crank_target(void)
{
  context_t context;
  prepare_stepper_disable(context);
  context.page2.idleUpAdder = 12U;
  context.init();

  context.current.idleUpActive = true;
  _idleState.idleStepper.targetStep = 0;

  disableIdle();

  TEST_ASSERT_EQUAL_INT(180, _idleState.idleStepper.targetStep);
  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
}

static void test_disableIdle_stepper_clamps_crank_target(void)
{
  context_t context;
  prepare_stepper_disable(context);
  context.page9.iacMaxSteps = 20U;
  context.page2.idleUpAdder = 30U;
  context.init();

  context.current.idleUpActive = true;
  _idleState.idleStepper.targetStep = 0;

  disableIdle();

  TEST_ASSERT_EQUAL_INT(60, _idleState.idleStepper.targetStep);
}

static void test_disableIdle_stepper_unhomed_starts_homing(void)
{
  context_t context;
  prepare_stepper_disable(context);
  context.page6.iacStepHome = 2U;
  context.init();

  _idleState.idleStepper.targetStep = 321;

  disableIdle();

  TEST_ASSERT_EQUAL_UINT(1U, _idleState.idleStepper.completedHomeSteps);
  TEST_ASSERT_EQUAL_INT(321, _idleState.idleStepper.targetStep);
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.status);
}

void testDisableIdle(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST(test_disableIdle_pwm_normal);
  RUN_TEST(test_disableIdle_pwm_reversed);
  RUN_TEST(test_disableIdle_pwm_dual_channel_normal);
  RUN_TEST(test_disableIdle_pwm_dual_channel_reversed);
  RUN_TEST(test_disableIdle_none_is_noop);
  RUN_TEST(test_disableIdle_stepper_moves_to_crank_target);
  RUN_TEST(test_disableIdle_stepper_clamps_crank_target);
  RUN_TEST(test_disableIdle_stepper_unhomed_starts_homing);
}
