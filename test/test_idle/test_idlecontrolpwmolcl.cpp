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
  TEST_ASSERT_EQUAL_UINT32(percentage(83U, _idleState.idle_pwm_max_count), _idleState.idle_pwm_target_value);
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

  TEST_ASSERT_EQUAL_UINT8(83U, context.current.idleLoad);
  TEST_ASSERT_EQUAL_UINT32(percentage(83U, _idleState.idle_pwm_max_count), _idleState.idle_pwm_target_value);
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

  TEST_ASSERT_EQUAL_UINT32(1900>>2, _idleState.idle_pwm_target_value);
  TEST_ASSERT_EQUAL_UINT16(800U, _idleState.idle_cl_target_rpm);
  TEST_ASSERT_EQUAL_UINT8(1U, _idleState.idleCounter);
}

static void test_RunningIdleUp(void)
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

  TEST_ASSERT_EQUAL_UINT32(850, _idleState.idle_pwm_target_value);
 }

static void test_RunningAirConditioning(void)
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

  TEST_ASSERT_EQUAL_UINT32(725, _idleState.idle_pwm_target_value);
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