#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "context.h"

extern idleController::detail::state_t _idleState;

static void prepare_stepOpenLoop(context_t &context)
{
  context.prepare_idle(IAC_ALGORITHM_STEP_OL);
  context.page9.iacMaxSteps = 100U;
  context.page6.iacStepHyster = 0U;
  context.page6.iacStepHome = 0U;
  context.page9.iacCoolTime = 1U;
  context.page6.iacStepTime = 1U;
}

static void test_StoppedUsesCrankTable(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.init();

  context.current.coolant = 110;
  context.current.rotationStatus = EngineRotationStatus::Stopped;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);
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

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);
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
  
  idle_micros_override_t::mockIdleMicrosValue = TIME_INCREMENT_STEP;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.status);
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);
  
  idle_micros_override_t::mockIdleMicrosValue += TIME_INCREMENT_STEP-1;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.status);
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);

  idle_micros_override_t::mockIdleMicrosValue += 1;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.status);
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);

  idle_micros_override_t::mockIdleMicrosValue += TIME_INCREMENT_COOL-1;
  idleControl();
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.status);
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);

  idle_micros_override_t::mockIdleMicrosValue += 1;
  context.page6.iacStepHyster = 255;
  idleControl();
  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);
  TEST_ASSERT_EQUAL_INT(2, _idleState.idleStepper.curStep);
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.status);
}

static void test_MovesTowardLowerTarget(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.init();

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.RPM = 1000U;
  _idleState.idleStepper.curStep = 5;
  _idleState.idleStepper.targetStep = 0;

  BIT_CLEAR(context.current.LOOP_TIMER, BIT_TIMER_1HZ);
  BIT_CLEAR(context.current.LOOP_TIMER, BIT_TIMER_10HZ);
  idleControl();

  TEST_ASSERT_EQUAL_INT(4, _idleState.idleStepper.curStep);
  TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.status);
}

static void test_IdleLoadAtByteBoundary(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.page9.iacMaxSteps = 85U;
  context.init();

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.RPM = 1000U;
  _idleState.idleStepper.curStep = 10;
  _idleState.idleStepper.targetStep = 10;

  BIT_CLEAR(context.current.LOOP_TIMER, BIT_TIMER_1HZ);
  BIT_CLEAR(context.current.LOOP_TIMER, BIT_TIMER_10HZ);
  idleControl();

  TEST_ASSERT_EQUAL_UINT8(10U, context.current.idleLoad);
  TEST_ASSERT_EQUAL_INT(10, _idleState.idleStepper.curStep);
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
//   _idleState.idleStepper.startTime = 50U;
//   _idleState.idleStepper.targetStep = 17;
//   _idleState.idleStepper.status = idleController::detail::StepperStatus::STEPPING;

//   idle_micros_override_t::mockIdleMicrosValue = 149U;
//   idleControl();
//   TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.status);
//   TEST_ASSERT_EQUAL_INT(17, _idleState.idleStepper.targetStep);

//   idle_micros_override_t::mockIdleMicrosValue = 150U;
//   idleControl();
//   TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.status);
//   TEST_ASSERT_EQUAL_INT(17, _idleState.idleStepper.targetStep);

//   idle_micros_override_t::mockIdleMicrosValue = 349U;
//   idleControl();
//   TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::COOLING, _idleState.idleStepper.status);
//   TEST_ASSERT_EQUAL_INT(17, _idleState.idleStepper.targetStep);

//   idle_micros_override_t::mockIdleMicrosValue = 350U;
//   idleControl();
//   TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);
//   TEST_ASSERT_EQUAL_INT(1, _idleState.idleStepper.curStep);
//   TEST_ASSERT_EQUAL(idleController::detail::StepperStatus::STEPPING, _idleState.idleStepper.status);
// }

static void test_RunningUsesRunningTable(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.init();

  context.current.coolant = 50;
  context.current.RPM = 1000U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_EQUAL_INT(180, _idleState.idleStepper.targetStep);
}

static void test_RunningTaper(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.page2.idleTaperTime = 2U;
  context.init();

  context.current.coolant = 50;
  context.current.RPM = 1000U;
  context.current.rotationStatus = EngineRotationStatus::Running;
  _idleState.idleTaper = 1U;

  idleControl();

  TEST_ASSERT_EQUAL_INT(174, _idleState.idleStepper.targetStep);
  TEST_ASSERT_EQUAL_UINT8(2U, _idleState.idleTaper);
}

static void test_RunningIdleUp(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.page2.idleUpAdder = 15U;
  context.page2.idleUpEnabled = true;
  context.page2.idleUpOutputEnabled = true;
  context.init();

  context.current.coolant = 110;
  context.current.RPM = 1000U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  _idleState.pinIdleUpInput._pin.setPinHigh();
  auto curMicros = context.idleControl(10000, 2000, 1);
  auto baseSteps = _idleState.idleStepper.targetStep;
  TEST_ASSERT_NOT_EQUAL(0, baseSteps);
  TEST_ASSERT_FALSE(context.current.idleUpActive);

  _idleState.pinIdleUpInput._pin.setPinLow();
  curMicros = context.idleControl(curMicros, 2000, 2);
  TEST_ASSERT_EQUAL(baseSteps+context.page2.idleUpAdder, _idleState.idleStepper.targetStep);
  TEST_ASSERT_TRUE(context.current.idleUpActive);
}

static void test_RunningAirConditioning(void)
{
  context_t context;
  prepare_stepOpenLoop(context);
  context.page15.airConIdleSteps = 10U;
  context.init();

  context.current.coolant = 110;
  context.current.RPM = 1000U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  context.current.acStatus.turningOn = false;
  auto curMicros = context.idleControl(10000, 2000, 1);
  auto baseSteps = _idleState.idleStepper.targetStep;
  TEST_ASSERT_NOT_EQUAL(0, baseSteps);

  context.current.acStatus.turningOn = true;
  curMicros = context.idleControl(curMicros, 2000, 2);
  TEST_ASSERT_EQUAL(baseSteps+context.page15.airConIdleSteps, _idleState.idleStepper.targetStep);
}

static void test_stepperModeDisablesIdle(void)
{
  context_t context;
  // Use a stepper mode to ensure isStepperIac(configPage6) evaluates to true
  prepare_stepOpenLoop(context); 
  context.init();

  // Simulate a state where idle is active (to ensure disableIdle works)
  _idleState.idleStepper.status = idleController::detail::StepperStatus::STEPPING;
  _idleState.idleStepper.targetStep = 10;
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