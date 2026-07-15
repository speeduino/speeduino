#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"

extern table2D_u8_u8_4 fanPWMTable;

static void set_coolant_above_ontemp(void)
{
  currentStatus.coolant = temperatureAddOffset(configPage6.fanSP + configPage6.fanHyster + 1);
}

static void set_coolant_below_ontemp(void)
{
  currentStatus.coolant = temperatureRemoveOffset((configPage6.fanSP - configPage6.fanHyster) - 1);
}

static void setup_status_fanoff(void)
{
  currentStatus.fanOn = false;
  currentStatus.fanDuty = 0U;
  currentStatus.rotationStatus = EngineRotationStatus::Stopped;
  currentStatus.airconTurningOn = false;
  set_coolant_below_ontemp();
}

static void setup_status_fanon(void)
{
  currentStatus.fanOn = true;
  currentStatus.fanDuty = 50U;
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  currentStatus.airconTurningOn = false;
  set_coolant_above_ontemp();
}


static void test_fanControl_disabled_does_nothing(void)
{
  setup_nopwm_tune();
  configPage2.fanEnable = 0U;
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanon(); 
  fanControl();
  // fanOn flag is only modified inside fanEnable branches -> stays whatever it was
  TEST_ASSERT_TRUE(currentStatus.fanOn);

  setup_status_fanoff(); 
  fanControl();
  // fanOn flag is only modified inside fanEnable branches -> stays whatever it was
  TEST_ASSERT_FALSE(currentStatus.fanOn);
}

static void setup_fanControl_on_when_engine_running_and_hot(void)
{
  configPage2.fanWhenOff = 0U;
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanon();
}

static void test_fanControl_nopwm_on_when_engine_running_and_hot(void)
{
  setup_nopwm_tune();
  setup_fanControl_on_when_engine_running_and_hot();
  fanControl();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_FAN_PIN));
  TEST_ASSERT_TRUE(currentStatus.fanOn);
}


static void test_fanControl_pwm_on_when_engine_running_and_hot(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_on_when_engine_running_and_hot();
  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.fanDuty);
#endif
}

static void seetup_fanControl_with_engine_stopped(void)
{
  configPage2.fanWhenOff = 0U;             // engine-running gates fan
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanoff();
  set_coolant_above_ontemp();
}

static void test_fanControl_nopwm_off_when_engine_stopped(void)
{
  setup_nopwm_tune();
  seetup_fanControl_with_engine_stopped();
  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
}

static void test_fanControl_pwm_off_when_engine_stopped(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  seetup_fanControl_with_engine_stopped();
  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL(0, currentStatus.fanDuty);
#endif
}

static void setup_fanControl_with_fanWhenOff_set(void)
{
  configPage2.fanWhenOff = 1U;
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanoff();
  set_coolant_above_ontemp();
}

static void test_fanControl_nopwm_runs_when_fanWhenOff_set(void)
{
  setup_nopwm_tune();
  setup_fanControl_with_fanWhenOff_set();
  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
}

static void test_fanControl_pwm_runs_when_fanWhenOff_set(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_with_fanWhenOff_set();
  fanControl();
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.fanDuty);
  TEST_ASSERT_TRUE(currentStatus.fanOn);
#endif
}

static void setup_fanControl_below_hysteresis(void)
{
  configPage2.fanWhenOff = 0U;
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanon();
  set_coolant_below_ontemp();
}

static void test_fanControl_nopwm_when_below_hysteresis(void)
{
  setup_nopwm_tune();
  setup_fanControl_below_hysteresis();
  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
}

static void test_fanControl_pwm_when_below_hysteresis(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_below_hysteresis();
  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL(0, currentStatus.fanDuty);
#endif
}

static void setup_fanControl_in_hysteresis_band(void)
{
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanon();
  // Coolant is between offTemp (75) and onTemp (80). Fan should stay
  // whatever it was — neither branch fires.
  currentStatus.coolant = temperatureRemoveOffset(configPage6.fanSP - (configPage6.fanHyster/2));
}

static void test_fanControl_nopwm_holds_in_hysteresis_band(void)
{
  setup_nopwm_tune();
  setup_fanControl_in_hysteresis_band();

  currentStatus.fanOn = false;
  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);

  currentStatus.fanOn = true;
  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
}

static void test_fanControl_pwm_holds_in_hysteresis_band(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_in_hysteresis_band();

  currentStatus.fanOn = false;
  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.fanDuty);

  currentStatus.fanOn = true;
  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.fanDuty);
#endif
}

static void setup_fanControl_disabled_during_crank(void)
{
  configPage2.fanWhenOff = 1U;             // permit even when not running
  configPage2.fanWhenCranking = 0U;        // disable during cranking
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanon();
  currentStatus.rotationStatus = EngineRotationStatus::Cranking;
}

static void test_fanControl_nopwm_disables_during_crank_when_configured(void)
{
  setup_nopwm_tune();
  setup_fanControl_disabled_during_crank();

  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
}

static void test_fanControl_pwm_disables_during_crank_when_configured(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_disabled_during_crank();

  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL(0, currentStatus.fanDuty);
#endif
}

static void setup_fanControl_running_during_crank(void)
{
  configPage2.fanWhenOff = 1U;
  configPage2.fanWhenCranking = 1U;        // allow during cranking
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanoff();
  set_coolant_above_ontemp();
  currentStatus.rotationStatus = EngineRotationStatus::Cranking;
}

static void test_fanControl_nopwm_runs_during_crank_when_permitted(void)
{
  setup_nopwm_tune();
  setup_fanControl_running_during_crank();
  
  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
}

static void test_fanControl_pwm_runs_during_crank_when_permitted(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_running_during_crank();
  
  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.fanDuty);
#endif
}

static void setup_fanControl_aircon_request_turns_fan_on(void)
{
  configPage15.airConTurnsFanOn = 1U;
  configPage15.airConPwmFanMinDuty = 200;
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanoff();
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  currentStatus.airconTurningOn = true;
}

static void test_fanControl_nopwm_aircon_request_turns_fan_on(void)
{
  setup_nopwm_tune();
  setup_fanControl_aircon_request_turns_fan_on();

  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
}

static void test_fanControl_pwm_aircon_request_turns_fan_on(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_aircon_request_turns_fan_on();

  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.fanDuty);
#endif
}

void tesFanControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_fanControl_disabled_does_nothing);
    RUN_TEST_P(test_fanControl_nopwm_on_when_engine_running_and_hot);
    RUN_TEST_P(test_fanControl_pwm_on_when_engine_running_and_hot);
    RUN_TEST_P(test_fanControl_nopwm_off_when_engine_stopped);
    RUN_TEST_P(test_fanControl_pwm_off_when_engine_stopped);
    RUN_TEST_P(test_fanControl_nopwm_runs_when_fanWhenOff_set);
    RUN_TEST_P(test_fanControl_pwm_runs_when_fanWhenOff_set);
    RUN_TEST_P(test_fanControl_nopwm_when_below_hysteresis);
    RUN_TEST_P(test_fanControl_pwm_when_below_hysteresis);
    RUN_TEST_P(test_fanControl_nopwm_holds_in_hysteresis_band);
    RUN_TEST_P(test_fanControl_pwm_holds_in_hysteresis_band);
    RUN_TEST_P(test_fanControl_nopwm_disables_during_crank_when_configured);
    RUN_TEST_P(test_fanControl_pwm_disables_during_crank_when_configured);
    RUN_TEST_P(test_fanControl_nopwm_runs_during_crank_when_permitted);
    RUN_TEST_P(test_fanControl_pwm_runs_during_crank_when_permitted);
    RUN_TEST_P(test_fanControl_nopwm_aircon_request_turns_fan_on);
    RUN_TEST_P(test_fanControl_pwm_aircon_request_turns_fan_on);
  }
}
