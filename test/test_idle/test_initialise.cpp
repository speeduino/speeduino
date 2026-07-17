#include "../test_utils.h"
#include "idle.h"
#include "prepare_idle.h"
#include "units.h"

static void test_initialiseIdle_none(void)
{
  prepare_idle(IAC_ALGORITHM_NONE);
  initialiseIdle(false);
  TEST_PASS();
}

static void test_initialiseIdle_onoff_warm_no_pin(void)
{
  prepare_idle(IAC_ALGORITHM_ONOFF);
  configPage6.iacFastTemp = temperatureAddOffset(50);  // Threshold is 50C
  currentStatus.coolant = 80;                          // Warm -> no fast idle
  initialiseIdle(false);
  TEST_PASS();
}

static void test_initialiseIdle_onoff_cold_runs(void)
{
  // idle_pin is a fastOutputPin_t which writes directly to port registers
  // and is not reflected in ArduinoFake's digitalRead() — so we just verify
  // the cold-temp branch runs without crashing.
  prepare_idle(IAC_ALGORITHM_ONOFF);
  configPage6.iacFastTemp = temperatureAddOffset(50);
  currentStatus.coolant = -10;                          // Cold -> fast idle ON
  initialiseIdle(false);
  TEST_PASS();
}

static void test_initialiseIdle_pwm_open_loop(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  TEST_PASS();
}

static void test_initialiseIdle_pwm_closed_loop(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_CL);
  initialiseIdle(false);
  TEST_PASS();
}

static void test_initialiseIdle_pwm_olcl(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OLCL);
  initialiseIdle(false);
  TEST_PASS();
}

static void test_initialiseIdle_step_open_loop(void)
{
  prepare_idle(IAC_ALGORITHM_STEP_OL);
  initialiseIdle(true);
  TEST_PASS();
}

static void test_initialiseIdle_step_closed_loop(void)
{
  prepare_idle(IAC_ALGORITHM_STEP_CL);
  initialiseIdle(true);
  TEST_PASS();
}

static void test_initialiseIdle_step_olcl(void)
{
  prepare_idle(IAC_ALGORITHM_STEP_OLCL);
  initialiseIdle(true);
  TEST_PASS();
}

void testInitialiseIdle(void)
{
  SET_UNITY_FILENAME()
  {
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
}
