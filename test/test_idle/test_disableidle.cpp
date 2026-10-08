#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "maths.h"
#include "units.h"
#include "context.h"

extern idleController::detail::state_t _idleState;
extern table2D_u8_u8_4 iacCrankStepsTable;

static void prepare_stepper_disable(context_t &context)
{
  context.prepare_idle(IAC_ALGORITHM_STEP_OL);
  context.page9.iacMaxSteps = 100U;
  context.page6.iacStepHome = 0U;

  TEST_DATA_P uint8_t crankBins[] = {
    temperatureAddOffset(0),
    temperatureAddOffset(50),
    temperatureAddOffset(100),
    temperatureAddOffset(212)
  };
  TEST_DATA_P uint8_t crankValues[] = { 0U, 30U, 80U, 120U };
  populate_2dtable_P(&iacCrankStepsTable, crankValues, crankBins);

  context.current.coolant = 50U;
  initialiseIdle(false);
  _idleState.completedHomeSteps = 0U;
  _idleState.idleStepper.stepperStatus = idleController::detail::StepperStatus::SOFF;
}

static void test_disableIdle_pwm_normal(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  context.page6.iacPWMdir = 0U;
  context.page6.iacChannels = 0U;
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
  initialiseIdle(false);
  context.page6.iacPWMdir = 1U;
  context.page6.iacChannels = 0U;
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
  initialiseIdle(false);
  context.page6.iacPWMdir = 0U;
  context.page6.iacChannels = 1U;
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
  initialiseIdle(false);
  context.page6.iacPWMdir = 1U;
  context.page6.iacChannels = 1U;
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
  initialiseIdle(false);
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
  context.current.idleUpActive = true;
  _idleState.idleStepper.targetIdleStep = 0;
  _idleState.idle_pid_target_value = 0;

  disableIdle();

  TEST_ASSERT_EQUAL_INT(102, _idleState.idleStepper.targetIdleStep);
  TEST_ASSERT_EQUAL_INT32(408, _idleState.idle_pid_target_value);
  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
}

static void test_disableIdle_stepper_clamps_crank_target(void)
{
  context_t context;
  prepare_stepper_disable(context);
  context.page9.iacMaxSteps = 20U;
  context.page2.idleUpAdder = 30U;
  context.current.idleUpActive = true;
  _idleState.idleStepper.targetIdleStep = 0;
  _idleState.idle_pid_target_value = 0;

  disableIdle();

  TEST_ASSERT_EQUAL_INT(60, _idleState.idleStepper.targetIdleStep);
  TEST_ASSERT_EQUAL_INT32(240, _idleState.idle_pid_target_value);
}

static void test_disableIdle_stepper_unhomed_starts_homing(void)
{
  context_t context;
  prepare_stepper_disable(context);
  context.page6.iacStepHome = 2U;
  _idleState.idleStepper.targetIdleStep = 321;
  _idleState.idle_pid_target_value = 654;

  disableIdle();

  TEST_ASSERT_EQUAL_UINT(1U, _idleState.completedHomeSteps);
  TEST_ASSERT_EQUAL_INT(321, _idleState.idleStepper.targetIdleStep);
  TEST_ASSERT_EQUAL_INT32(654, _idleState.idle_pid_target_value);
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.stepperStatus);
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
