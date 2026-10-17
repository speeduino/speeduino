#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "context.h"

extern idleController::detail::state_t _idleState;
extern idleController::detail::fnCurMicros_t _idleCurMicros;

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

static void prepare_stepOpenLoop(context_t &context)
{
  context.prepare_idle(IAC_ALGORITHM_STEP_OL);
  context.page9.iacMaxSteps = 100U;
  context.page6.iacStepHyster = 0U;
  context.page6.iacStepHome = 0U;
}

static void test_StoppedUsesCrankTable(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.init();

  context.current.coolant = 110;
  context.current.rotationStatus = EngineRotationStatus::Stopped;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
}

static void test_CrankingUsesCrankTable(void)
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

static void test_StepperSteppingAndCooling(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  constexpr uint8_t COOL_MILLIS = 1;
  context.page9.iacCoolTime = COOL_MILLIS;
  constexpr uint8_t STEP_MILLIS = 2;
  context.page6.iacStepTime = STEP_MILLIS;
  context.init();

  context.current.coolant = 110;
  context.current.rotationStatus = EngineRotationStatus::Stopped;

  idle_micros_override_t microsOverride;
  constexpr uint32_t TIME_INCREMENT_STEP = STEP_MILLIS * 1000UL;
  constexpr uint32_t TIME_INCREMENT_COOL = COOL_MILLIS * 1000UL;
  
  mockIdleMicrosValue = TIME_INCREMENT_STEP;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.stepperStatus);
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
  
  mockIdleMicrosValue += TIME_INCREMENT_STEP-1;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.stepperStatus);
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);

  mockIdleMicrosValue += 1;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.stepperStatus);
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);

  mockIdleMicrosValue += TIME_INCREMENT_COOL-1;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.stepperStatus);
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);

  mockIdleMicrosValue += 1;
  context.page6.iacStepHyster = 255;
  idleControl();
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetIdleStep);
  TEST_ASSERT_EQUAL_INT(2, _idleState.idleStepper.curIdleStep);
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.stepperStatus);
}

static void test_MovesTowardLowerTarget(void)
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

static void test_IdleLoadAtByteBoundary(void)
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

// static void test_StepperSteppingAndCooling(void)
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

static void test_RunningUsesRunningTable(void)
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

static void test_RunningTaper(void)
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

static void test_RunningIdleUp(void)
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

static void test_RunningAirConditioning(void)
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

void testIdleControlStepOl(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_StoppedUsesCrankTable);
  RUN_TEST_P(test_CrankingUsesCrankTable);
  RUN_TEST_P(test_StepperSteppingAndCooling);
  RUN_TEST_P(test_MovesTowardLowerTarget);
  RUN_TEST_P(test_IdleLoadAtByteBoundary);
  RUN_TEST_P(test_RunningUsesRunningTable);
  RUN_TEST_P(test_RunningTaper);
  RUN_TEST_P(test_RunningIdleUp);
  RUN_TEST_P(test_RunningAirConditioning);
  RUN_TEST_P(test_stepperModeDisablesIdle);
}