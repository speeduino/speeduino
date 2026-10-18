#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "src/controllers/idle/idleController_state.h"
#include "context.h"
#include "units.h"

extern idleController::detail::state_t _idleState;
extern table2D_u8_u8_10 iacPWMTable;

static void test_Cranking(void)
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
  TEST_ASSERT_EQUAL_UINT8(0U, _idleState.idleCounter);
}

static void test_StoppedUsesCrankDuty(void)
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

  TEST_ASSERT_UINT8_WITHIN(1, 83, context.current.idleLoad);
  TEST_ASSERT_EQUAL_UINT8(0U, _idleState.idleCounter);
}

static void test_Running(void)
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

  TEST_ASSERT_UINT8_WITHIN(1, 19, context.current.idleLoad);
  TEST_ASSERT_EQUAL_UINT16(800U, _idleState.idle_cl_target_rpm);
  TEST_ASSERT_EQUAL_UINT8(1U, _idleState.idleCounter);
}

static void test_RunningIdleUp(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  context.page6.idleFreq = 100U;
  context.page6.idleKP = 1U;
  context.page6.idleKI = 1U;
  context.page6.idleKD = 1U;
  context.page2.idleUpAdder = 15U;
  context.page2.idleUpEnabled = true;
  context.page2.idleUpOutputEnabled = true;
  context.init();

  context.current.coolant = 110;
  context.current.RPM = 1000U;
  context.current.rotationStatus = EngineRotationStatus::Running;

  digitalWrite(context.pins.pinIdleUp, HIGH);
  auto curMicros = context.idleControl(10000, 2000, 5);
  auto baseLoad = context.current.idleLoad;
  TEST_ASSERT_NOT_EQUAL(0, baseLoad);
  TEST_ASSERT_FALSE(context.current.idleUpActive);

#if !defined(__AVR__) // Fix when using pin wrappers
  digitalWrite(context.pins.pinIdleUp, LOW);
  curMicros = context.idleControl(curMicros, 2000, 1);
  TEST_ASSERT_EQUAL(baseLoad+context.page2.idleUpAdder, context.current.idleLoad);
  TEST_ASSERT_TRUE(context.current.idleUpActive);
#endif
}

static void test_RunningAirConditioning(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  context.page6.idleFreq = 100U;
  context.page6.idleKP = 1U;
  context.page6.idleKI = 1U;
  context.page6.idleKD = 1U;
  context.page15.airConIdleSteps = 10U;
  context.init();

  context.current.coolant = 110;
  context.current.setRpm(500);
  context.current.rotationStatus = EngineRotationStatus::Running;

  context.current.acStatus.turningOn = false;
  auto curMicros = context.idleControl(10000, 2000, 5);
  auto baseLoad = context.current.idleLoad;
  TEST_ASSERT_NOT_EQUAL(0, baseLoad);

  context.current.acStatus.turningOn = true;
  curMicros = context.idleControl(curMicros, 2000, 1);
  TEST_ASSERT_UINT8_WITHIN(1, baseLoad+context.page15.airConIdleSteps, context.current.idleLoad);
}

void testIdleControlPwmOlCl(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_Cranking);
  RUN_TEST_P(test_StoppedUsesCrankDuty);
  RUN_TEST_P(test_Running);
  RUN_TEST_P(test_RunningIdleUp);
  RUN_TEST_P(test_RunningAirConditioning);
}