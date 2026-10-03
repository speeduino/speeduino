#include "src/controllers/fan/fanController.h"
#include "units.h"
#include "../test_utils.h"
#include "shared.h"
#include "src/pins/outputPin.h"
#include "src/pwm/PwmOutputChannel.h"
#include "src/pins/invertableOutputPin.h"

using fanPwmChannel_t = PwmOutputChannel<invertableOutputPinAdaper_t<outputPin_t>>;
extern fanPwmChannel_t _fanPwm;
 extern table2D_u8_u8_4 fanPWMTable;

static void assert_nopwm_fan_pin_state(const test_context_t &context, bool active)
{
  if (active)
  {
    TEST_ASSERT_EQUAL(200, context.current.fanDuty);
  }
  else
  {
    TEST_ASSERT_EQUAL(0, context.current.fanDuty);
  }
  TEST_ASSERT_EQUAL(active, _fanPwm.pin.isPinHigh());
}

static void set_coolant_above_ontemp(test_context_t &context)
{
  context.current.coolant = temperatureAddOffset(context.page6.fanSP + context.page6.fanHyster + 1);
}

static void set_coolant_below_ontemp(test_context_t &context)
{
  context.current.coolant = temperatureRemoveOffset((context.page6.fanSP - context.page6.fanHyster) - 1);
}

static void setup_status_fanoff(test_context_t &context)
{
  context.current.fanDuty = 0U;
  context.current.rotationStatus = EngineRotationStatus::Stopped;
  context.current.acStatus.turningOn = false;
  set_coolant_below_ontemp(context);
}

static void setup_status_fanon(test_context_t &context)
{
  context.current.fanDuty = 50U;
  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.acStatus.turningOn = false;
  set_coolant_above_ontemp(context);
}

static void test_fanControl_disabled_zero_duty(void)
{
  auto context = setup_nopwm_tune();
  context.page2.fanEnable = FANMODE_OFF;
  context.initialise();

  setup_status_fanon(context); 
  context.current.fanDuty = 99;
  context.fanControl();
  TEST_ASSERT_EQUAL(0, context.current.fanDuty);

  setup_status_fanoff(context); 
  context.current.fanDuty = 99;
  context.fanControl();
  TEST_ASSERT_EQUAL(0, context.current.fanDuty);
}

static void setup_fanControl_on_when_engine_running_and_hot(test_context_t &context)
{
  context.page2.fanWhenOff = 0U;
  context.initialise();

  setup_status_fanon(context);
}

static void test_fanControl_nopwm_on_when_engine_running_and_hot(void)
{
  auto context = setup_nopwm_tune();
  context.initialise();

  setup_fanControl_on_when_engine_running_and_hot(context);
  context.fanControl();
  assert_nopwm_fan_pin_state(context, true);
}

static void test_fanControl_pwm_on_when_engine_running_and_hot(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto context = setup_pwm_tune();
  context.initialise();

  setup_fanControl_on_when_engine_running_and_hot(context);
  context.fanControl();
  TEST_ASSERT_EQUAL(200, context.current.fanDuty);
#endif
}

static void setup_fanControl_with_engine_stopped(test_context_t &context)
{
  context.page2.fanWhenOff = 0U;             // engine-running gates fan
  context.initialise();

  setup_status_fanoff(context);
  set_coolant_above_ontemp(context);
}

static void test_fanControl_nopwm_off_when_engine_stopped(void)
{
  auto context = setup_nopwm_tune();
  setup_fanControl_with_engine_stopped(context);
  context.fanControl();
  assert_nopwm_fan_pin_state(context, false);
}

static void test_fanControl_pwm_off_when_engine_stopped(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto context = setup_pwm_tune();
  setup_fanControl_with_engine_stopped(context);
  context.fanControl();
  TEST_ASSERT_EQUAL(0, context.current.fanDuty);
#endif
}

static void setup_fanControl_with_fanWhenOff_set(test_context_t &context)
{
  context.page2.fanWhenOff = 1U;
  context.initialise();

  setup_status_fanoff(context);
  set_coolant_above_ontemp(context);
}

static void test_fanControl_nopwm_runs_when_fanWhenOff_set(void)
{
  auto context = setup_nopwm_tune();
  setup_fanControl_with_fanWhenOff_set(context);
  context.fanControl();
  assert_nopwm_fan_pin_state(context, true);
}

static void test_fanControl_pwm_runs_when_fanWhenOff_set(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto context = setup_pwm_tune();
  setup_fanControl_with_fanWhenOff_set(context);
  context.fanControl();
  TEST_ASSERT_EQUAL(200, context.current.fanDuty);
#endif
}

static void setup_fanControl_below_hysteresis(test_context_t &context)
{
  context.page2.fanWhenOff = 0U;
  context.initialise();

  setup_status_fanon(context);
  set_coolant_below_ontemp(context);
}

static void test_fanControl_nopwm_when_below_hysteresis(void)
{
  auto context = setup_nopwm_tune();
  setup_fanControl_below_hysteresis(context);
  context.fanControl();
  assert_nopwm_fan_pin_state(context, false);
}

static void test_fanControl_pwm_when_below_hysteresis(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto context = setup_pwm_tune();
  setup_fanControl_below_hysteresis(context);
  context.fanControl();

  // Hysterisis isn't a PWM feature.
  TEST_ASSERT_EQUAL(75, context.current.fanDuty);
#endif
}

static void setup_fanControl_in_hysteresis_band(test_context_t &context)
{
  context.initialise();

  setup_status_fanon(context);
  // Coolant is between offTemp (75) and onTemp (80). Fan should stay
  // whatever it was — neither branch fires.
  context.current.coolant = temperatureRemoveOffset(context.page6.fanSP - (context.page6.fanHyster/2));
}

static void test_fanControl_nopwm_holds_in_hysteresis_band(void)
{
  auto context = setup_nopwm_tune();
  setup_fanControl_in_hysteresis_band(context);

  context.current.fanDuty = 0;
  context.fanControl();
  TEST_ASSERT_EQUAL(0, context.current.fanDuty);

  context.current.fanDuty = 50;
  context.fanControl();
  TEST_ASSERT_EQUAL(50, context.current.fanDuty);
}

static void test_fanControl_pwm_holds_in_hysteresis_band(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto context = setup_pwm_tune();
  setup_fanControl_in_hysteresis_band(context);

  context.current.fanDuty = 0;
  context.fanControl();
  TEST_ASSERT_EQUAL(125, context.current.fanDuty);

  context.current.fanDuty = 50;
  context.fanControl();
  TEST_ASSERT_EQUAL(125, context.current.fanDuty);
#endif
}

static void setup_fanControl_disabled_during_crank(test_context_t &context)
{
  context.page2.fanWhenOff = 1U;             // permit even when not running
  context.page2.fanWhenCranking = 0U;        // disable during cranking
  context.initialise();

  setup_status_fanon(context);
  context.current.rotationStatus = EngineRotationStatus::Cranking;
}

static void test_fanControl_nopwm_disables_during_crank_when_configured(void)
{
  auto context = setup_nopwm_tune();
  setup_fanControl_disabled_during_crank(context);

  context.fanControl();
  assert_nopwm_fan_pin_state(context, false);
}

static void test_fanControl_pwm_disables_during_crank_when_configured(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto context = setup_pwm_tune();
  setup_fanControl_disabled_during_crank(context);

  context.fanControl();
  TEST_ASSERT_EQUAL(0, context.current.fanDuty);
#endif
}

static void setup_fanControl_running_during_crank(test_context_t &context)
{
  context.page2.fanWhenOff = 1U;
  context.page2.fanWhenCranking = 1U;        // allow during cranking
  context.initialise();

  setup_status_fanoff(context);
  set_coolant_above_ontemp(context);
  context.current.rotationStatus = EngineRotationStatus::Cranking;
}

static void test_fanControl_nopwm_runs_during_crank_when_permitted(void)
{
  auto context = setup_nopwm_tune();
  setup_fanControl_running_during_crank(context);
  
  context.fanControl();
  assert_nopwm_fan_pin_state(context, true);
}

static void test_fanControl_pwm_runs_during_crank_when_permitted(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto context = setup_pwm_tune();
  setup_fanControl_running_during_crank(context);
  
  context.fanControl();
  TEST_ASSERT_EQUAL(200, context.current.fanDuty);
#endif
}

static void setup_fanControl_aircon_request_turns_fan_on(test_context_t &context)
{
  context.page15.airConTurnsFanOn = 1U;
  context.page15.airConPwmFanMinDuty = 200;
  context.initialise();

  setup_status_fanoff(context);
  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.acStatus.turningOn = true;
}

static void test_fanControl_nopwm_aircon_request_turns_fan_on(void)
{
  auto context = setup_nopwm_tune();
  setup_fanControl_aircon_request_turns_fan_on(context);

  context.fanControl();
  assert_nopwm_fan_pin_state(context, true);
}

static void test_fanControl_pwm_aircon_request_turns_fan_on(void)
{
#if defined(PWM_FAN_AVAILABLE)
  auto context = setup_pwm_tune();
  setup_fanControl_aircon_request_turns_fan_on(context);

  context.fanControl();
  TEST_ASSERT_NOT_EQUAL(0, context.current.fanDuty);
  TEST_ASSERT_GREATER_OR_EQUAL(context.page15.airConPwmFanMinDuty, context.current.fanDuty);
#endif
}

static void assert_cranking_overrides_hysteresis(bool inverted)
{
  auto context = setup_nopwm_tune();
  context.page6.fanInv = inverted;
  context.page2.fanWhenOff = 1U;
  context.page2.fanWhenCranking = 0U;
  context.initialise();
  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.acStatus.turningOn = false;

  const int16_t onTemp = temperatureRemoveOffset(context.page6.fanSP);
  const int16_t holdTemp = onTemp - context.page6.fanHyster / 2U;
  context.current.coolant = onTemp;
  context.fanControl();
  assert_nopwm_fan_pin_state(context, true);
  context.current.coolant = holdTemp;
  context.fanControl();
  assert_nopwm_fan_pin_state(context, true);

  context.current.rotationStatus = EngineRotationStatus::Cranking;
  context.fanControl();
  assert_nopwm_fan_pin_state(context, false);

  // A/C demand must not bypass the configured cranking inhibit either.
  context.page15.airConTurnsFanOn = 1U;
  context.current.acStatus.turningOn = true;
  context.fanControl();
  assert_nopwm_fan_pin_state(context, false);

  // Resume normal hysteresis once cranking ends.
  context.current.acStatus.turningOn = false;
  context.current.rotationStatus = EngineRotationStatus::Running;
  context.fanControl();
  assert_nopwm_fan_pin_state(context, false);
  context.current.coolant = onTemp;
  context.fanControl();
  assert_nopwm_fan_pin_state(context, true);
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
  auto context = setup_nopwm_tune();
  context.page2.fanWhenOff = 1U;
  context.page2.fanWhenCranking = 1U;
  context.initialise();
  context.current.acStatus.turningOn = false;
  context.current.rotationStatus = EngineRotationStatus::Cranking;
  const int16_t onTemp = temperatureRemoveOffset(context.page6.fanSP);
  context.current.coolant = onTemp - context.page6.fanHyster / 2U;
  context.fanControl();
  assert_nopwm_fan_pin_state(context, false);
  context.current.coolant = onTemp;
  context.fanControl();
  assert_nopwm_fan_pin_state(context, true);
  context.current.coolant = onTemp - context.page6.fanHyster / 2U;
  context.fanControl();
  assert_nopwm_fan_pin_state(context, true);
}

void tesFanControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_fanControl_cranking_overrides_hysteresis);
    RUN_TEST_P(test_fanControl_cranking_overrides_hysteresis_inverted);
    RUN_TEST_P(test_fanControl_cranking_preserves_hysteresis_when_permitted);
    RUN_TEST_P(test_fanControl_disabled_zero_duty);
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
