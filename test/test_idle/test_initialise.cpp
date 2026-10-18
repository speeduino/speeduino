#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "context.h"
#include "units.h"
#include "src/controllers/idle/idleController_state.h"

extern idleController::detail::state_t _idleState;

static void test_initialiseIdle_none(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_NONE);
  context.init();

  TEST_ASSERT_EQUAL_UINT8(IAC_ALGORITHM_NONE, _idleState.idleInitComplete);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.idleLoad);
}

static void test_initialiseIdle_onoff_warm_no_pin(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  context.page6.iacFastTemp = temperatureAddOffset(50);  // Threshold is 50C
  context.current.coolant = 80;                          // Warm -> no fast idle
  context.init();

  TEST_ASSERT_EQUAL_UINT8(IAC_ALGORITHM_ONOFF, _idleState.idleInitComplete);
  TEST_ASSERT_FALSE(_idleState.idleOn);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.idleLoad);
  TEST_ASSERT_FALSE(_idleState.idle_pin._pin.isPinHigh());
}

static void test_initialiseIdle_onoff_cold_runs(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  context.page6.iacFastTemp = temperatureAddOffset(50);
  context.current.coolant = -10;                          // Cold -> fast idle ON
  context.init();
  TEST_ASSERT_EQUAL_UINT8(IAC_ALGORITHM_ONOFF, _idleState.idleInitComplete);
  TEST_ASSERT_TRUE(_idleState.idleOn);
  TEST_ASSERT_TRUE(_idleState.idle_pin._pin.isPinHigh());
}

static void test_initialiseIdle_pwm_open_loop(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  context.init();
  TEST_ASSERT_EQUAL_UINT8(IAC_ALGORITHM_PWM_OL, _idleState.idleInitComplete);
  TEST_ASSERT_FALSE(_idleState.idleOn);
  TEST_ASSERT_NOT_EQUAL(0, _idleState.idle_pwm_max_count);
}

static void test_initialiseIdle_pwm_closed_loop(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_CL);
  context.init();
  TEST_ASSERT_EQUAL_UINT8(IAC_ALGORITHM_PWM_CL, _idleState.idleInitComplete);
  TEST_ASSERT_FALSE(_idleState.idleOn);
  TEST_ASSERT_NOT_EQUAL(0, _idleState.idle_pwm_max_count);
}

static void test_initialiseIdle_pwm_olcl(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  context.init();
  TEST_ASSERT_EQUAL_UINT8(IAC_ALGORITHM_PWM_OLCL, _idleState.idleInitComplete);
  TEST_ASSERT_FALSE(_idleState.idleOn);
  TEST_ASSERT_NOT_EQUAL(0, _idleState.idle_pwm_max_count);
}

static void test_initialiseIdle_step_open_loop(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_STEP_OL);
  context.init();

  TEST_ASSERT_EQUAL_UINT8(IAC_ALGORITHM_STEP_OL, _idleState.idleInitComplete);
  TEST_ASSERT_FALSE(_idleState.idleOn);
  TEST_ASSERT_NOT_EQUAL(0, _idleState.idle_pwm_max_count);
}

static void test_initialiseIdle_step_closed_loop(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_STEP_CL);
  context.init();

  TEST_ASSERT_EQUAL_UINT8(IAC_ALGORITHM_STEP_CL, _idleState.idleInitComplete);
  TEST_ASSERT_FALSE(_idleState.idleOn);
  TEST_ASSERT_NOT_EQUAL(0, _idleState.idle_pwm_max_count);
}

static void test_initialiseIdle_step_olcl(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_STEP_OLCL);
  context.init();

  TEST_ASSERT_EQUAL_UINT8(IAC_ALGORITHM_STEP_OLCL, _idleState.idleInitComplete);
  TEST_ASSERT_FALSE(_idleState.idleOn);
  TEST_ASSERT_NOT_EQUAL(0, _idleState.idle_pwm_max_count);
}

void testInitialiseIdle(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST(test_initialiseIdle_none);
  RUN_TEST(test_initialiseIdle_onoff_warm_no_pin);
  RUN_TEST(test_initialiseIdle_onoff_cold_runs);
  RUN_TEST(test_initialiseIdle_pwm_open_loop);
  RUN_TEST(test_initialiseIdle_pwm_closed_loop);
  RUN_TEST(test_initialiseIdle_pwm_olcl);
  RUN_TEST(test_initialiseIdle_step_open_loop);
  RUN_TEST(test_initialiseIdle_step_closed_loop);
  RUN_TEST(test_initialiseIdle_step_olcl);
}
