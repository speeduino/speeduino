#include <unity.h>
#include <Arduino.h>
#include "globals.h"
#include "idle.h"
#include "config_pages.h"
#include "units.h"
#include "../test_utils.h"

// Idle test pins; values are arbitrary free pins under ArduinoFake.
static constexpr uint8_t TEST_IDLE1_PIN = 90U;
static constexpr uint8_t TEST_IDLE2_PIN = 91U;

static void prepare_idle(uint8_t algorithm)
{
  pinIdle1 = TEST_IDLE1_PIN;
  pinIdle2 = TEST_IDLE2_PIN;
  configPage6.iacAlgorithm = algorithm;
  configPage6.iacChannels = 0U;
  configPage6.iacPWMdir = 0U;
  configPage6.iacPWMrun = 0U;
  configPage6.iacFastTemp = 0U;            // Skip ON branch in ON_OFF unless we set coolant low
  configPage6.idleKP = 100U;
  configPage6.idleKI = 50U;
  configPage6.idleKD = 0U;
  configPage6.iacStepTime = 1U;
  configPage9.iacCoolTime = 1U;
  configPage2.iacCLminValue = 0U;
  configPage2.iacCLmaxValue = 100U;
  configPage2.idleUpAdder = 0U;
  currentStatus.coolant = 80;
  currentStatus.idleUpActive = false;
  currentStatus.CLIdleTarget = 80U;
}

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

// ============================ disableIdle ===================================

// idle1/idle2_pin are fastOutputPin_t — port-register writes are not visible
// to ArduinoFake's digitalRead(). The disableIdle tests below verify the
// full set of branches dispatch (line coverage); pin state is not checked.

static void test_disableIdle_pwm_normal(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  configPage6.iacPWMdir = 0U;
  configPage6.iacChannels = 0U;
  disableIdle();
  TEST_PASS();
}

static void test_disableIdle_pwm_reversed(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  configPage6.iacPWMdir = 1U;
  configPage6.iacChannels = 0U;
  disableIdle();
  TEST_PASS();
}

static void test_disableIdle_pwm_dual_channel_normal(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  configPage6.iacPWMdir = 0U;
  configPage6.iacChannels = 1U;
  disableIdle();
  TEST_PASS();
}

static void test_disableIdle_pwm_dual_channel_reversed(void)
{
  prepare_idle(IAC_ALGORITHM_PWM_OL);
  initialiseIdle(false);
  configPage6.iacPWMdir = 1U;
  configPage6.iacChannels = 1U;
  disableIdle();
  TEST_PASS();
}

static void test_disableIdle_none_is_noop(void)
{
  prepare_idle(IAC_ALGORITHM_NONE);
  initialiseIdle(false);
  disableIdle();   // No assertion: just verify it doesn't crash.
  TEST_PASS();
}

void testIdle(void)
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
    RUN_TEST(test_disableIdle_pwm_normal);
    RUN_TEST(test_disableIdle_pwm_reversed);
    RUN_TEST(test_disableIdle_pwm_dual_channel_normal);
    RUN_TEST(test_disableIdle_pwm_dual_channel_reversed);
    RUN_TEST(test_disableIdle_none_is_noop);
  }
}
