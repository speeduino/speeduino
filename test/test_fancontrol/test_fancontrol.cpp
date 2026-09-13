#include "globals.h"
#include "src/controllers/fan/fanController.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"
#include "src/pins/boardOutputPin.h"

 extern boardOutputPin_t fan_pin;
 extern table2D_u8_u8_4 fanPWMTable;

static void assert_nopwm_fan_pin_state(bool active)
{
  if (active)
  {
    TEST_ASSERT_EQUAL(200, currentStatus.fanDuty);
  }
  else
  {
    TEST_ASSERT_EQUAL(0, currentStatus.fanDuty);
  }
  TEST_ASSERT_EQUAL(active != (configPage6.fanInv != 0U), fan_pin._pin.isPinHigh());
}

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
  currentStatus.fanDuty = 0U;
  currentStatus.rotationStatus = EngineRotationStatus::Stopped;
  currentStatus.acStatus.turningOn = false;
  set_coolant_below_ontemp();
}

static void setup_status_fanon(void)
{
  currentStatus.fanDuty = 50U;
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  currentStatus.acStatus.turningOn = false;
  set_coolant_above_ontemp();
}


static void test_fanControl_disabled_does_nothing(void)
{
  setup_nopwm_tune();
  configPage2.fanEnable = 0U;
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanon(); 
  currentStatus.fanDuty = 99;
  fanControl();
  // fanOn flag is only modified inside fanEnable branches -> stays whatever it was
  TEST_ASSERT_EQUAL(99, currentStatus.fanDuty);

  setup_status_fanoff(); 
  currentStatus.fanDuty = 99;
  fanControl();
  // fanOn flag is only modified inside fanEnable branches -> stays whatever it was
  TEST_ASSERT_EQUAL(99, currentStatus.fanDuty);
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
  initialiseFan(TEST_FAN_PIN);

  setup_fanControl_on_when_engine_running_and_hot();
  fanControl();
  assert_nopwm_fan_pin_state(true);
}

static void test_fanControl_pwm_on_when_engine_running_and_hot(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  initialiseFan(TEST_FAN_PIN);

  setup_fanControl_on_when_engine_running_and_hot();
  fanControl();
  TEST_ASSERT_EQUAL(200, currentStatus.fanDuty);
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
  assert_nopwm_fan_pin_state(false);
}

static void test_fanControl_pwm_off_when_engine_stopped(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  seetup_fanControl_with_engine_stopped();
  fanControl();
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
  assert_nopwm_fan_pin_state(true);
}

static void test_fanControl_pwm_runs_when_fanWhenOff_set(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_with_fanWhenOff_set();
  fanControl();
  TEST_ASSERT_EQUAL(200, currentStatus.fanDuty);
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
  assert_nopwm_fan_pin_state(false);
}

static void test_fanControl_pwm_when_below_hysteresis(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_below_hysteresis();
  fanControl();

  // Hysterisis isn't a PWM feature.
  TEST_ASSERT_EQUAL(75, currentStatus.fanDuty);
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

  currentStatus.fanDuty = 0;
  fanControl();
  TEST_ASSERT_EQUAL(0, currentStatus.fanDuty);

  currentStatus.fanDuty = 50;
  fanControl();
  TEST_ASSERT_EQUAL(50, currentStatus.fanDuty);
}

static void test_fanControl_pwm_holds_in_hysteresis_band(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_in_hysteresis_band();

  currentStatus.fanDuty = 0;
  fanControl();
  TEST_ASSERT_EQUAL(125, currentStatus.fanDuty);

  currentStatus.fanDuty = 50;
  fanControl();
  TEST_ASSERT_EQUAL(125, currentStatus.fanDuty);
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
  assert_nopwm_fan_pin_state(false);
}

static void test_fanControl_pwm_disables_during_crank_when_configured(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_disabled_during_crank();

  fanControl();
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
  assert_nopwm_fan_pin_state(true);
}

static void test_fanControl_pwm_runs_during_crank_when_permitted(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_running_during_crank();
  
  fanControl();
  TEST_ASSERT_EQUAL(200, currentStatus.fanDuty);
#endif
}

static void setup_fanControl_aircon_request_turns_fan_on(void)
{
  configPage15.airConTurnsFanOn = 1U;
  configPage15.airConPwmFanMinDuty = 200;
  initialiseFan(TEST_FAN_PIN);

  setup_status_fanoff();
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  currentStatus.acStatus.turningOn = true;
}

static void test_fanControl_nopwm_aircon_request_turns_fan_on(void)
{
  setup_nopwm_tune();
  setup_fanControl_aircon_request_turns_fan_on();

  fanControl();
  assert_nopwm_fan_pin_state(true);
}

static void test_fanControl_pwm_aircon_request_turns_fan_on(void)
{
#if defined(PWM_FAN_AVAILABLE)
  setup_pwm_tune();
  setup_fanControl_aircon_request_turns_fan_on();

  fanControl();
  TEST_ASSERT_NOT_EQUAL(0, currentStatus.fanDuty);
  TEST_ASSERT_GREATER_OR_EQUAL(configPage15.airConPwmFanMinDuty, currentStatus.fanDuty);
#endif
}

static void assert_cranking_overrides_hysteresis(bool inverted)
{
  setup_nopwm_tune();
  configPage6.fanInv = inverted;
  configPage2.fanWhenOff = 1U;
  configPage2.fanWhenCranking = 0U;
  initialiseFan(TEST_FAN_PIN);
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  currentStatus.acStatus.turningOn = false;

  const int16_t onTemp = temperatureRemoveOffset(configPage6.fanSP);
  const int16_t holdTemp = onTemp - configPage6.fanHyster / 2U;
  currentStatus.coolant = onTemp;
  fanControl();
  assert_nopwm_fan_pin_state(true);
  currentStatus.coolant = holdTemp;
  fanControl();
  assert_nopwm_fan_pin_state(true);

  currentStatus.rotationStatus = EngineRotationStatus::Cranking;
  fanControl();
  assert_nopwm_fan_pin_state(false);

  // A/C demand must not bypass the configured cranking inhibit either.
  configPage15.airConTurnsFanOn = 1U;
  currentStatus.acStatus.turningOn = true;
  fanControl();
  assert_nopwm_fan_pin_state(false);

  // Resume normal hysteresis once cranking ends.
  currentStatus.acStatus.turningOn = false;
  currentStatus.rotationStatus = EngineRotationStatus::Running;
  fanControl();
  assert_nopwm_fan_pin_state(false);
  currentStatus.coolant = onTemp;
  fanControl();
  assert_nopwm_fan_pin_state(true);
}

static void test_fanControl_cranking_overrides_hysteresis(void)
{
  assert_cranking_overrides_hysteresis(false);
}

static void test_fanControl_cranking_overrides_hysteresis_inverted(void)
{
  assert_cranking_overrides_hysteresis(true);
}

static void test_fanControl_cranking_preserves_hysteresis_when_permitted(void)
{
  setup_nopwm_tune();
  configPage2.fanWhenOff = 1U;
  configPage2.fanWhenCranking = 1U;
  initialiseFan(TEST_FAN_PIN);
  currentStatus.acStatus.turningOn = false;
  currentStatus.rotationStatus = EngineRotationStatus::Cranking;
  const int16_t onTemp = temperatureRemoveOffset(configPage6.fanSP);
  currentStatus.coolant = onTemp - configPage6.fanHyster / 2U;
  fanControl();
  assert_nopwm_fan_pin_state(false);
  currentStatus.coolant = onTemp;
  fanControl();
  assert_nopwm_fan_pin_state(true);
  currentStatus.coolant = onTemp - configPage6.fanHyster / 2U;
  fanControl();
  assert_nopwm_fan_pin_state(true);
}

void tesFanControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_fanControl_cranking_overrides_hysteresis);
    RUN_TEST_P(test_fanControl_cranking_overrides_hysteresis_inverted);
    RUN_TEST_P(test_fanControl_cranking_preserves_hysteresis_when_permitted);
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
