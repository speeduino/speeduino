#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "context.h"

extern idleController::detail::state_t _idleState;

static context_t setup_stepper_cl(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_STEP_CL);
  context.page6.idleKP = 0U;
  context.page6.idleKI = 0U;
  context.page6.idleKD = 0U;
  context.page6.iacStepHyster = 0U;
  context.page6.iacStepHome = 0U;
  context.page9.iacMaxSteps = 100U;
  return context;
}

static void test_StoppedUsesCrankTable(void)
{
  auto context = setup_stepper_cl();
  context.init();
  context.current.coolant = 110;
  context.current.rotationStatus = EngineRotationStatus::Stopped;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);
}

static void test_CrankingUsesCrankTable(void)
{
  auto context = setup_stepper_cl();
  context.init();
  context.current.coolant = 110;
  context.current.RPM = 600U;
  context.current.rotationStatus = EngineRotationStatus::Cranking;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);
}

static void test_Running(void)
{
  auto context = setup_stepper_cl();
  context.init();
  context.current.RPM = 1000U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  idleControl();

  TEST_ASSERT_EQUAL_INT(60, _idleState.idleStepper.targetStep);
}

static void test_RunningTaper(void)
{
  auto context = setup_stepper_cl();
  context.page2.idleTaperTime = 3U;
  context.init();

  context.current.coolant = 110;
  context.current.RPM = 1000U;
  context.current.rotationStatus = EngineRotationStatus::Running;
  _idleState.idleTaper = 1U;

  idleControl();

  TEST_ASSERT_EQUAL_INT(300, _idleState.idleStepper.targetStep);
  TEST_ASSERT_EQUAL_UINT8(2U, _idleState.idleTaper);
}

static void test_RunningIdleUp(void)
{
  auto context = setup_stepper_cl();
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
  auto context = setup_stepper_cl();
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

void testIdleControlStepCl(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_StoppedUsesCrankTable);
  RUN_TEST_P(test_CrankingUsesCrankTable);
  RUN_TEST_P(test_Running);
  RUN_TEST_P(test_RunningTaper);
  RUN_TEST_P(test_RunningIdleUp);
  RUN_TEST_P(test_RunningAirConditioning);
}