#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "context.h"

extern idleController::detail::state_t _idleState;

static void test_StoppedUsesCrankDuty(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.idleFreq = 100U;
  context.page2.idleTaperTime = 0U;
  context.page6.iacPWMrun = true;
  context.init();

  context.current.coolant = 91;
  context.current.rotationStatus = EngineRotationStatus::Stopped;
  idleControl();
  TEST_ASSERT_EQUAL_UINT8(83U, context.current.idleLoad);
}

static void test_RunningUsesRunningTable(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.idleFreq = 100U;
  context.page2.idleTaperTime = 0U;
  context.init();

  context.current.coolant = 110;
  context.current.rotationStatus = EngineRotationStatus::Running;
  idleControl();
  TEST_ASSERT_EQUAL_UINT8(31U, context.current.idleLoad);
}

static void test_RunningTaper(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.idleFreq = 100U;
  context.page2.idleTaperTime = 2U;
  context.init();

  context.current.coolant = 53;
  context.current.rotationStatus = EngineRotationStatus::Running;
  _idleState.idleTaper = 1U;
  idleControl();

  TEST_ASSERT_EQUAL_UINT8(39U, context.current.idleLoad);
  TEST_ASSERT_EQUAL_UINT8(2U, _idleState.idleTaper);
}

static void prepare_pwmFullDuty(context_t &context, uint8_t direction, uint8_t channels)
{
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.idleFreq = 100U;
  context.page6.iacPWMdir = direction;
  context.page6.iacChannels = channels;

  context.current.coolant = 110;
  context.current.RPM = 600U;
  context.current.rotationStatus = EngineRotationStatus::Cranking;
}

static void test_FullDutyNormalSingleChannel(void)
{
  context_t context;
  prepare_pwmFullDuty(context, 0U, 0U);
  context.init();

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(100U, context.current.idleLoad);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
}

static void test_FullDutyNormalDualChannel(void)
{
  context_t context;
  prepare_pwmFullDuty(context, 0U, 1U);
  context.init();

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
  TEST_ASSERT_TRUE(_idleState.idle2_pin._pin.isPinLow());
}

static void test_FullDutyReversedSingleChannel(void)
{
  context_t context;
  prepare_pwmFullDuty(context, 1U, 0U);
  context.init();

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinLow());
}

static void test_FullDutyReversedDualChannel(void)
{
  context_t context;
  prepare_pwmFullDuty(context, 1U, 1U);
  context.init();

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinLow());
  TEST_ASSERT_TRUE(_idleState.idle2_pin._pin.isPinHigh());
}

static void test_ZeroDutyDisablesIdle(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.idleFreq = 100U;
  context.init();

  context.current.coolant = -20;
  context.current.RPM = 600U;
  context.current.rotationStatus = EngineRotationStatus::Cranking;
  context.current.idleOn = true;

  idleControl();

  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinLow());
}

static void test_OpenLoopCranking(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.idleFreq = 100U;
  context.page2.idleTaperTime = 0U;
  context.init();

  context.current.coolant = 91;
  context.current.rotationStatus = EngineRotationStatus::Cranking;
  idleControl();
  TEST_ASSERT_EQUAL_UINT8(83U, context.current.idleLoad);
}

void testIdleControlPwmOl(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_StoppedUsesCrankDuty);
  RUN_TEST_P(test_RunningUsesRunningTable);
  RUN_TEST_P(test_RunningTaper);
  RUN_TEST_P(test_FullDutyNormalSingleChannel);
  RUN_TEST_P(test_FullDutyNormalDualChannel);
  RUN_TEST_P(test_FullDutyReversedSingleChannel);
  RUN_TEST_P(test_FullDutyReversedDualChannel);
  RUN_TEST_P(test_ZeroDutyDisablesIdle);
  RUN_TEST_P(test_OpenLoopCranking);
}