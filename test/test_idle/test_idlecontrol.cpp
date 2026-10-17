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

static void prepare_stepOpenLoop(context_t &context)
{
  context.prepare_idle(IAC_ALGORITHM_STEP_OL);
  context.page9.iacMaxSteps = 100U;
  context.page6.iacStepHyster = 0U;
  context.page6.iacStepHome = 0U;
}

static void prepare_stepClosedLoop(context_t &context, uint8_t algorithm = IAC_ALGORITHM_STEP_CL)
{
  context.prepare_idle(algorithm);
  context.page6.idleKP = 0U;
  context.page6.idleKI = 0U;
  context.page6.idleKD = 0U;
  context.page6.iacStepHyster = 0U;
  context.page6.iacStepHome = 0U;
  context.page9.iacMaxSteps = 100U;
}

static void test_onOff(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  context.page6.iacFastTemp = 110; // 50C threshold
  context.init();

  context.current.coolant = 0;                            // Cold: below threshold -> idle ON

  idleControl();

  TEST_ASSERT_TRUE(_idleState.idleOn);
  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
  TEST_ASSERT_EQUAL_UINT8(100U, context.current.idleLoad);

  context.current.coolant = 80;                           // Warm: above threshold -> idle OFF
  idleControl();

  TEST_ASSERT_FALSE(_idleState.idleOn);
  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinLow());
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
}

static void test_pwmFullDutyNormalSingleChannel(void)
{
  context_t context;
  prepare_pwmFullDuty(context, 0U, 0U);
  context.init();

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(100U, context.current.idleLoad);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
}

static void test_pwmFullDutyNormalDualChannel(void)
{
  context_t context;
  prepare_pwmFullDuty(context, 0U, 1U);
  context.init();

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
  TEST_ASSERT_TRUE(_idleState.idle2_pin._pin.isPinLow());
}

static void test_pwmFullDutyReversedSingleChannel(void)
{
  context_t context;
  prepare_pwmFullDuty(context, 1U, 0U);
  context.init();

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinLow());
}

static void test_pwmFullDutyReversedDualChannel(void)
{
  context_t context;
  prepare_pwmFullDuty(context, 1U, 1U);
  context.init();

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinLow());
  TEST_ASSERT_TRUE(_idleState.idle2_pin._pin.isPinHigh());
}

static void test_pwmZeroDutyDisablesIdle(void)
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

static void test_stepOpenLoopStoppedUsesCrankTable(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.init();

  context.current.coolant = 110;
  context.current.rotationStatus = EngineRotationStatus::Stopped;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
}

static void test_stepOpenLoopCrankingUsesCrankTable(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.init();

  context.current.coolant = 110;
  context.current.RPM = 600U;
  context.current.rotationStatus = EngineRotationStatus::Cranking;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
}

static void test_stepOpenLoopStepperSteppingAndCooling(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.init();

  context.current.coolant = 110;
  context.current.rotationStatus = EngineRotationStatus::Stopped;

  idle_micros_override_t microsOverride;
  _idleState.iacStepTime_uS = 100U;
  _idleState.iacCoolTime_uS = 200U;
  _idleState.idleStepper.stepStartTime = 50U;
  _idleState.idleStepper.targetIdleStep = 17;
  _idleState.idleStepper.stepperStatus = idleController::detail::StepperStatus::STEPPING;

  mockIdleMicrosValue = 149U;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.stepperStatus);
  TEST_ASSERT_EQUAL_INT(17, _idleState.idleStepper.targetIdleStep);

  mockIdleMicrosValue = 150U;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.stepperStatus);
  TEST_ASSERT_EQUAL_INT(17, _idleState.idleStepper.targetIdleStep);

  mockIdleMicrosValue = 349U;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.stepperStatus);
  TEST_ASSERT_EQUAL_INT(17, _idleState.idleStepper.targetIdleStep);

  mockIdleMicrosValue = 350U;
  context.page6.iacStepHyster = 255;
  idleControl();
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
  TEST_ASSERT_EQUAL_INT(1, _idleState.idleStepper.curIdleStep);
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.stepperStatus);
}

static void test_stepOpenLoopMovesTowardLowerTarget(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.init();

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.RPM = 1000U;
  _idleState.idleStepper.curIdleStep = 5;
  _idleState.idleStepper.targetIdleStep = 0;

  idleControl();

  TEST_ASSERT_EQUAL_INT(4, _idleState.idleStepper.curIdleStep);
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.stepperStatus);
}

static void test_stepOpenLoopIdleLoadAtByteBoundary(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.page9.iacMaxSteps = 85U;
  context.init();

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.RPM = 1000U;
  _idleState.idleStepper.curIdleStep = 10;
  _idleState.idleStepper.targetIdleStep = 10;

  idleControl();

  TEST_ASSERT_EQUAL_UINT8(10U, context.current.idleLoad);
  TEST_ASSERT_EQUAL_INT(10, _idleState.idleStepper.curIdleStep);
  TEST_ASSERT_FALSE(context.current.idleOn);
}

// static void test_stepOpenLoopStepperSteppingAndCooling(void)
// {
//   context_t context;
//   prepare_stepOpenLoop(context);
//   context.current.coolant = 110;
//   context.current.rotationStatus = EngineRotationStatus::Stopped;
//   initialiseIdle(false);

//   idle_micros_override_t microsOverride;
//   _idleState.iacStepTime_uS = 100U;
//   _idleState.iacCoolTime_uS = 200U;
//   _idleState.idleStepper.stepStartTime = 50U;
//   _idleState.idleStepper.targetIdleStep = 17;
//   _idleState.idleStepper.stepperStatus = idleController::detail::StepperStatus::STEPPING;

//   mockIdleMicrosValue = 149U;
//   idleControl();
//   TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.stepperStatus);
//   TEST_ASSERT_EQUAL_INT(17, _idleState.idleStepper.targetIdleStep);

//   mockIdleMicrosValue = 150U;
//   idleControl();
//   TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.stepperStatus);
//   TEST_ASSERT_EQUAL_INT(17, _idleState.idleStepper.targetIdleStep);

//   mockIdleMicrosValue = 349U;
//   idleControl();
//   TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.stepperStatus);
//   TEST_ASSERT_EQUAL_INT(17, _idleState.idleStepper.targetIdleStep);

//   mockIdleMicrosValue = 350U;
//   idleControl();
//   TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
//   TEST_ASSERT_EQUAL_INT(1, _idleState.idleStepper.curIdleStep);
//   TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.stepperStatus);
// }

static void test_stepOpenLoopRunningUsesRunningTable(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.init();

  context.current.coolant = 50;
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_EQUAL_INT(180, _idleState.idleStepper.targetIdleStep);
}

static void test_stepOpenLoopRunningTaper(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.page2.idleTaperTime = 2U;
  context.init();

  context.current.coolant = 50;
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;
  _idleState.idleTaper = 1U;

  idleControl();

  TEST_ASSERT_EQUAL_INT(174, _idleState.idleStepper.targetIdleStep);
  TEST_ASSERT_EQUAL_UINT8(2U, _idleState.idleTaper);
}

static void test_stepOpenLoopRunningIdleUp(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.page2.idleUpEnabled = true;
  context.page2.idleUpAdder = 15U;
  context.init();

  digitalWrite(context.pins.pinIdleUp, LOW);
  context.current.coolant = 50;
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleUpActive);
  TEST_ASSERT_EQUAL_INT(195, _idleState.idleStepper.targetIdleStep);
}

static void test_stepOpenLoopRunningAirConditioning(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.page15.airConIdleSteps = 10U;
  context.init();

  context.current.acStatus.turningOn = true;
  context.current.coolant = 50;
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_EQUAL_INT(190, _idleState.idleStepper.targetIdleStep);
}

static void test_stepClosedLoopStoppedUsesCrankTable(void)
{
  context_t context;
  prepare_stepClosedLoop(context);
  context.init();
  context.current.coolant = 110;
  context.current.rotationStatus = EngineRotationStatus::Stopped;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
}

static void test_stepClosedLoopCrankingUsesCrankTable(void)
{
  context_t context;
  prepare_stepClosedLoop(context);
  context.init();
  context.current.coolant = 110;
  context.current.RPM = 600U;
  context.current.rotationStatus = EngineRotationStatus::Cranking;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
}

static void test_stepClosedLoopRunning(void)
{
  context_t context;
  prepare_stepClosedLoop(context);
  context.init();
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_EQUAL_INT(60, _idleState.idleStepper.targetIdleStep);
}

static void test_stepClosedLoopRunningTaper(void)
{
  context_t context;
  prepare_stepClosedLoop(context);
  context.page2.idleTaperTime = 3U;
  context.init();

  context.current.coolant = 110;
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;
  _idleState.idle_pid_target_value = 1200;
  _idleState.idleTaper = 1U;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
  TEST_ASSERT_EQUAL_UINT8(2U, _idleState.idleTaper);
}

static void test_stepOpenLoopClosedLoopRunningTaper(void)
{
  context_t context;
  prepare_stepClosedLoop(context, IAC_ALGORITHM_STEP_OLCL);
  context.page2.idleTaperTime = 3U;
  context.init();

  context.current.coolant = 110;
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;
  _idleState.idle_pid_target_value = 1200;
  _idleState.idleTaper = 1U;

  idleControl();

  TEST_ASSERT_EQUAL_INT(295, _idleState.idleStepper.targetIdleStep);
  TEST_ASSERT_EQUAL_UINT8(2U, _idleState.idleTaper);
}

static void test_stepOpenLoopClosedLoopRunningUsesFeedForward(void)
{
  context_t context;
  prepare_stepClosedLoop(context, IAC_ALGORITHM_STEP_OLCL);
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

static void test_stepClosedLoopRunningIdleUp(void)
{
  context_t context;
  prepare_stepClosedLoop(context);
  context.page2.idleUpEnabled = true;
  context.page2.idleUpAdder = 15U;
  context.init();

  context.pins.pinIdleUp = TEST_IDLE1_PIN;
  digitalWrite(context.pins.pinIdleUp, LOW);
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_TRUE(context.current.idleUpActive);
  TEST_ASSERT_EQUAL_INT(75, _idleState.idleStepper.targetIdleStep);
}

static void test_stepClosedLoopRunningAirConditioning(void)
{
  context_t context;
  prepare_stepClosedLoop(context);
  context.page15.airConIdleSteps = 10U;
  context.init();

  context.current.acStatus.turningOn = true;
  context.current.RPM = 1000U;
  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_EQUAL_INT(70, _idleState.idleStepper.targetIdleStep);
}

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

static void test_pwmOpenLoopCranking(void)
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

static void test_pwmOpenLoopStoppedUsesCrankDuty(void)
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

static void test_pwmOpenLoopRunningUsesRunningTable(void)
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

static void test_pwmOpenLoopRunningTaper(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.page6.idleFreq = 100U;
  context.page2.idleTaperTime = 2U;
  context.init();

  context.current.LOOP_TIMER = (1U << BIT_TIMER_10HZ);
  context.current.coolant = 53;
  context.current.rotationStatus = EngineRotationStatus::Running;
  _idleState.idleTaper = 1U;
  idleControl();

  TEST_ASSERT_EQUAL_UINT8(39U, context.current.idleLoad);
  TEST_ASSERT_EQUAL_UINT8(2U, _idleState.idleTaper);
}

static void test_stepperModeDisablesIdle(void)
{
  context_t context;
  // Use a stepper mode to ensure isStepperIac(configPage6) evaluates to true
  prepare_stepOpenLoop(context); 
  context.init();

  // Simulate a state where idle is active (to ensure disableIdle works)
  _idleState.idleStepper.stepperStatus = idleController::detail::StepperStatus::STEPPING;
  _idleState.idleStepper.targetIdleStep = 10;
  context.current.idleOn = true;
  context.current.idleLoad = 50;

  // Run idleControl once to fully initialize and set up the state variables correctly for the test
  idleControl(); 
  
  // Now test the disableIdle function
  disableIdle();

  // Assert that idle control has been turned off and load set to zero
  TEST_ASSERT_FALSE(context.current.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, context.current.idleLoad);
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

  RUN_TEST_P(test_onOff);
  RUN_TEST_P(test_pwmFullDutyNormalSingleChannel);
  RUN_TEST_P(test_pwmFullDutyNormalDualChannel);
  RUN_TEST_P(test_pwmFullDutyReversedSingleChannel);
  RUN_TEST_P(test_pwmFullDutyReversedDualChannel);
  RUN_TEST_P(test_pwmZeroDutyDisablesIdle);
  RUN_TEST_P(test_stepOpenLoopStoppedUsesCrankTable);
  RUN_TEST_P(test_stepOpenLoopCrankingUsesCrankTable);
  RUN_TEST_P(test_stepOpenLoopStepperSteppingAndCooling);
  RUN_TEST_P(test_stepOpenLoopMovesTowardLowerTarget);
  RUN_TEST_P(test_stepOpenLoopIdleLoadAtByteBoundary);
  RUN_TEST_P(test_stepOpenLoopRunningUsesRunningTable);
  RUN_TEST_P(test_stepOpenLoopRunningTaper);
  RUN_TEST_P(test_stepOpenLoopRunningIdleUp);
  RUN_TEST_P(test_stepOpenLoopRunningAirConditioning);
  RUN_TEST_P(test_stepClosedLoopStoppedUsesCrankTable);
  RUN_TEST_P(test_stepClosedLoopCrankingUsesCrankTable);
  RUN_TEST_P(test_stepClosedLoopRunning);
  RUN_TEST_P(test_stepClosedLoopRunningTaper);
  RUN_TEST_P(test_stepOpenLoopClosedLoopRunningTaper);
  RUN_TEST_P(test_stepOpenLoopClosedLoopRunningUsesFeedForward);
  RUN_TEST_P(test_stepClosedLoopRunningIdleUp);
  RUN_TEST_P(test_stepClosedLoopRunningAirConditioning);
  RUN_TEST_P(test_idleUpInputAndOutput);
  RUN_TEST_P(test_pwmOpenLoopCranking);
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
  RUN_TEST_P(test_pwmOpenLoopStoppedUsesCrankDuty);
  RUN_TEST_P(test_pwmOpenLoopRunningUsesRunningTable);
  RUN_TEST_P(test_pwmOpenLoopRunningTaper);
  RUN_TEST_P(test_stepperModeDisablesIdle);
  RUN_TEST_P(test_basic_coverage_allModes);
}
