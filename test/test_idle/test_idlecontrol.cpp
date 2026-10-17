#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "maths.h"
#include "units.h"
#include "context.h"

extern idleController::detail::state_t _idleState;
extern idleController::detail::fnCurMicros_t _idleCurMicros;
extern table2D_u8_u8_10 iacPWMTable;

static unsigned long mockIdleMicrosValue;

static unsigned long mockIdleMicros(void)
{
  return mockIdleMicrosValue;
}

struct idle_micros_override_t
{
  idleController::detail::fnCurMicros_t original;

  idle_micros_override_t(void) : original(_idleCurMicros)
  {
    _idleCurMicros = &mockIdleMicros;
  }

  ~idle_micros_override_t(void)
  {
    _idleCurMicros = original;
  }
};

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

static void test_pwmClosedLoopCranking(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_CL);
  context.page6.idleFreq = 100U;
  context.init();

  context.current.coolant = 91;
  context.current.RPM = 600U;
  context.current.rotationStatus = EngineRotationStatus::Cranking;
  idleControl();

  TEST_ASSERT_EQUAL_UINT8(83U, context.current.idleLoad);
  TEST_ASSERT_EQUAL_UINT32(percentage(83U, _idleState.idle_pwm_max_count), _idleState.idle_pwm_target_value);
  TEST_ASSERT_EQUAL_INT32(_idleState.idle_pwm_target_value << 2, _idleState.idle_pid_target_value);
  TEST_ASSERT_EQUAL_UINT8(0U, _idleState.idleCounter);
}

static void test_pwmOpenLoopClosedLoopCranking(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  context.page6.idleFreq = 100U;
  context.init();

  context.current.coolant = 91;
  context.current.RPM = 600U;
  context.current.rotationStatus = EngineRotationStatus::Cranking;

  idleControl();

  TEST_ASSERT_EQUAL_UINT8(83U, context.current.idleLoad);
  TEST_ASSERT_EQUAL_UINT32(percentage(83U, _idleState.idle_pwm_max_count), _idleState.idle_pwm_target_value);
  TEST_ASSERT_EQUAL_INT32(_idleState.idle_pwm_target_value << 2, _idleState.idle_pid_target_value);
  TEST_ASSERT_EQUAL_UINT8(0U, _idleState.idleCounter);
}

static void test_pwmOpenLoopClosedLoopStoppedUsesCrankDuty(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  context.page6.idleFreq = 100U;
  context.page6.iacPWMrun = true;
  context.init();

  context.current.coolant = 91;
  context.current.RPM = 0U;
  context.current.rotationStatus = EngineRotationStatus::Stopped;

  idleControl();

  TEST_ASSERT_EQUAL_UINT8(83U, context.current.idleLoad);
  TEST_ASSERT_EQUAL_UINT32(percentage(83U, _idleState.idle_pwm_max_count), _idleState.idle_pwm_target_value);
  TEST_ASSERT_EQUAL_UINT8(0U, _idleState.idleCounter);
}

static void test_pwmOpenLoopClosedLoopRunning(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  context.page6.idleFreq = 100U;
  context.page6.idleKP = 0U;
  context.page6.idleKI = 0U;
  context.page6.idleKD = 0U;
  context.init();

  context.current.coolant = 50;
  context.current.RPM = 700U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_EQUAL_UINT32(1900, _idleState.FeedForwardTerm);
  TEST_ASSERT_EQUAL_UINT32(1900>>2, _idleState.idle_pwm_target_value);
  TEST_ASSERT_EQUAL_UINT16(800U, _idleState.idle_cl_target_rpm);
  TEST_ASSERT_EQUAL_UINT8(1U, _idleState.idleCounter);
}

static void test_pwmOpenLoopClosedLoopRunningIdleUp(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  context.page6.idleFreq = 100U;
  context.page6.idleKP = 0U;
  context.page6.idleKI = 0U;
  context.page6.idleKD = 0U;
  context.page2.idleUpEnabled = true;
  context.page2.idleUpAdder = 15U;
  context.init();

  digitalWrite(context.pins.pinIdleUp, LOW);
  context.current.coolant = 50;
  context.current.RPM = 700U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  const uint8_t baseDuty = table2D_getValue(&iacPWMTable, temperatureAddOffset(context.current.coolant));
  const uint32_t expectedFeedForward = percentage(baseDuty, _idleState.idle_pwm_max_count << 2)
      + percentage(context.page2.idleUpAdder, _idleState.idle_pwm_max_count << 2);
  TEST_ASSERT_TRUE(context.current.idleUpActive);
  TEST_ASSERT_EQUAL_UINT32(expectedFeedForward, _idleState.FeedForwardTerm);
}

static void test_pwmOpenLoopClosedLoopRunningAirConditioning(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  context.page6.idleFreq = 100U;
  context.page6.idleKP = 0U;
  context.page6.idleKI = 0U;
  context.page6.idleKD = 0U;
  context.page15.airConIdleSteps = 10U;
  context.init();

  context.current.acStatus.turningOn = true;
  context.current.coolant = 50;
  context.current.RPM = 700U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  const uint8_t baseDuty = table2D_getValue(&iacPWMTable, temperatureAddOffset(context.current.coolant));
  const uint32_t expectedFeedForward = percentage(baseDuty, _idleState.idle_pwm_max_count << 2)
      + percentage(context.page15.airConIdleSteps, _idleState.idle_pwm_max_count << 2);
  TEST_ASSERT_EQUAL_UINT32(expectedFeedForward, _idleState.FeedForwardTerm);
}

static void test_pwmClosedLoopStoppedUsesCrankDuty(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_CL);
  context.page6.idleFreq = 100U;
  context.page6.iacPWMrun = true;
  context.init();

  context.current.coolant = 91;
  context.current.RPM = 0U;
  context.current.rotationStatus = EngineRotationStatus::Stopped;

  idleControl();

  TEST_ASSERT_EQUAL_UINT8(83U, context.current.idleLoad);
  TEST_ASSERT_EQUAL_UINT32(percentage(83U, _idleState.idle_pwm_max_count), _idleState.idle_pwm_target_value);
}

static void test_pwmClosedLoopRunning(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_CL);
  context.page6.idleFreq = 100U;
  context.init();

  context.current.RPM = 700U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_EQUAL_UINT16(800U, _idleState.idle_cl_target_rpm);
  TEST_ASSERT_EQUAL_UINT8(1U, _idleState.idleCounter);
}

static void test_pwmClosedLoopRunningIdleUp(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_CL);
  context.page6.idleFreq = 100U;
  context.page6.idleKP = 0U;
  context.page6.idleKI = 0U;
  context.page6.idleKD = 0U;
  context.page2.idleUpEnabled = true;
  context.page2.idleUpPolarity = 0U;
  context.page2.idleUpAdder = 15U;
  context.init();

  digitalWrite(pinNumbers.pinIdleUp, LOW);
  context.current.RPM = 700U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleUpActive);
  TEST_ASSERT_EQUAL_UINT32(percentage(context.page2.idleUpAdder, _idleState.idle_pwm_max_count << 2) >> 2,
                           _idleState.idle_pwm_target_value);
}

static void test_pwmClosedLoopRunningAirConditioning(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_CL);
  context.page6.idleFreq = 100U;
  context.page6.idleKP = 0U;
  context.page6.idleKI = 0U;
  context.page6.idleKD = 0U;
  context.page15.airConIdleSteps = 10U;
  context.init();

  context.current.acStatus.turningOn = true;
  context.current.RPM = 700U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_EQUAL_UINT32(percentage(context.page15.airConIdleSteps, _idleState.idle_pwm_max_count << 2) >> 2,
                           _idleState.idle_pwm_target_value);
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
  RUN_TEST_P(test_pwmClosedLoopCranking);
  RUN_TEST_P(test_pwmOpenLoopClosedLoopCranking);
  RUN_TEST_P(test_pwmOpenLoopClosedLoopStoppedUsesCrankDuty);
  RUN_TEST_P(test_pwmOpenLoopClosedLoopRunning);
  RUN_TEST_P(test_pwmOpenLoopClosedLoopRunningIdleUp);
  RUN_TEST_P(test_pwmOpenLoopClosedLoopRunningAirConditioning);
  RUN_TEST_P(test_pwmClosedLoopStoppedUsesCrankDuty);
  RUN_TEST_P(test_pwmClosedLoopRunning);
  RUN_TEST_P(test_pwmClosedLoopRunningIdleUp);
  RUN_TEST_P(test_pwmClosedLoopRunningAirConditioning);
  RUN_TEST_P(test_basic_coverage_allModes);
}
