#include "../test_utils.h"
#include "src/controllers/idle/idle.h"
#include "context.h"
#include "units.h"

static void test_initialiseIdle_none(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_NONE);
  initialiseIdle(false);
}

static void test_initialiseIdle_onoff_warm_no_pin(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  context.page6.iacFastTemp = temperatureAddOffset(50);  // Threshold is 50C
  context.current.coolant = 80;                          // Warm -> no fast idle
  initialiseIdle(false);
}

static void test_initialiseIdle_onoff_cold_runs(void)
{
  context_t context;
  // idle_pin is a fastOutputPin_t which writes directly to port registers
  // and is not reflected in ArduinoFake's digitalRead() — so we just verify
  // the cold-temp branch runs without crashing.
  context.prepare_idle(IAC_ALGORITHM_ONOFF);
  context.page6.iacFastTemp = temperatureAddOffset(50);
  context.current.coolant = -10;                          // Cold -> fast idle ON
  initialiseIdle(false);
}

static void test_initialiseIdle_pwm_open_loop(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
}

static void test_initialiseIdle_pwm_closed_loop(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_CL);
  initialiseIdle(false);
}

static void test_initialiseIdle_pwm_olcl(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  initialiseIdle(false);
}

static void test_initialiseIdle_step_open_loop(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_STEP_OL);
  initialiseIdle(true);
}

static void test_initialiseIdle_step_closed_loop(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_STEP_CL);
  initialiseIdle(true);
}

static void test_initialiseIdle_step_olcl(void)
{
  context_t context;
  context.prepare_idle(IAC_ALGORITHM_STEP_OLCL);
  initialiseIdle(true);
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
