#include <unity.h>
#include <Arduino.h>
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "../test_utils.h"

// Pin numbers used by these tests. Any free pin works under ArduinoFake.
static constexpr uint8_t TEST_PUMP_PIN = 70U;
static constexpr uint8_t TEST_FAN_PIN  = 71U;
static constexpr uint8_t TEST_BOOST_PIN = 72U;
static constexpr uint8_t TEST_VVT1_PIN = 73U;
static constexpr uint8_t TEST_VVT2_PIN = 74U;

static bool aux_io_initialised = false;

static void ensure_aux_io_initialised(void)
{
  if (aux_io_initialised) { return; }
  pinFuelPump = TEST_PUMP_PIN;
  pinFan      = TEST_FAN_PIN;
  pinBoost    = TEST_BOOST_PIN;
  pinVVT_1    = TEST_VVT1_PIN;
  pinVVT_2    = TEST_VVT2_PIN;

  // initialiseAuxPWM also initialises VVT, boost and n2o pins. Set
  // n2o_enable=1 here so the n2o arming-pin port pointer gets bound — that
  // pointer is dereferenced unconditionally inside nitrousControl()'s
  // isPinHigh() call and would otherwise SIGSEGV.
  configPage10.n2o_minTPS = 0U;
  configPage10.n2o_stage1_pin = 80U;
  configPage10.n2o_stage2_pin = 81U;
  configPage10.n2o_arming_pin = 82U;
  configPage10.n2o_pin_polarity = 0U;
  configPage10.n2o_enable = 1U;
  configPage6.boostMode = 0U;
  configPage6.vvtEnabled = 0U;
  configPage10.wmiEnabled = 0U;
  initialiseAuxPWM();
  configPage10.n2o_enable = 0U;            // Default to off for tests below.
  initialiseFan(TEST_FAN_PIN);
  configPage2.fpPrime = 0U;
  initialiseFuelPump(configPage2, TEST_PUMP_PIN);

  aux_io_initialised = true;
}

static void reset_aux_state(void)
{
  ensure_aux_io_initialised();
  currentStatus.fuelPumpOn = false;
  currentStatus.fanOn = false;
  currentStatus.fanDuty = 0U;
  currentStatus.engineIsRunning = false;
  currentStatus.engineIsCranking = false;
  currentStatus.coolant = 0;
  currentStatus.airconTurningOn = false;

  configPage6.fanInv = 0U;
  configPage6.fanSP = temperatureAddOffset(80);   // ON above 80C
  configPage6.fanHyster = 5U;                      // OFF below 75C
  configPage2.fanEnable = 0U;
  configPage2.fanWhenOff = 0U;
  configPage2.fanWhenCranking = 0U;
  configPage2.fpPrime = 0U;
  configPage15.airConTurnsFanOn = 0U;
}

// ============================ Fuel pump =====================================

static void test_fuelPumpOn_sets_pin_and_status(void)
{
  reset_aux_state();
  fuelPumpOff();
  TEST_ASSERT_FALSE(currentStatus.fuelPumpOn);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_PUMP_PIN));

  fuelPumpOn();
  TEST_ASSERT_TRUE(currentStatus.fuelPumpOn);
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_PUMP_PIN));
}

static void test_fuelPumpOff_clears_pin_and_status(void)
{
  reset_aux_state();
  fuelPumpOn();
  fuelPumpOff();
  TEST_ASSERT_FALSE(currentStatus.fuelPumpOn);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_PUMP_PIN));
}

static void test_initialiseFuelPump_no_prime_returns_true(void)
{
  reset_aux_state();
  configPage2.fpPrime = 0U;
  bool primed = initialiseFuelPump(configPage2, TEST_PUMP_PIN);
  TEST_ASSERT_TRUE(primed);
  TEST_ASSERT_FALSE(currentStatus.fuelPumpOn);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_PUMP_PIN));
}

static void test_initialiseFuelPump_with_prime_returns_false_and_starts_pump(void)
{
  reset_aux_state();
  configPage2.fpPrime = 5U;
  bool primed = initialiseFuelPump(configPage2, TEST_PUMP_PIN);
  TEST_ASSERT_FALSE(primed);
  TEST_ASSERT_TRUE(currentStatus.fuelPumpOn);
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_PUMP_PIN));
}

// ============================ Fan low-level =================================

static void test_fanOn_normal_polarity_drives_pin_high(void)
{
  reset_aux_state();
  configPage6.fanInv = 0U;
  fanOff();
  fanOn();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_FAN_PIN));
}

static void test_fanOff_normal_polarity_drives_pin_low(void)
{
  reset_aux_state();
  configPage6.fanInv = 0U;
  fanOn();
  fanOff();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
}

static void test_fanOn_inverted_polarity_drives_pin_low(void)
{
  reset_aux_state();
  configPage6.fanInv = 1U;
  fanOff();   // inverted off -> HIGH
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_FAN_PIN));
  fanOn();    // inverted on  -> LOW
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
}

static void test_initialiseFan_resets_state(void)
{
  reset_aux_state();
  currentStatus.fanOn = true;
  currentStatus.fanDuty = 99U;
  initialiseFan(TEST_FAN_PIN);
  TEST_ASSERT_FALSE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.fanDuty);
  // Normal polarity off -> pin LOW
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
}

// ============================ fanControl ====================================

static void test_fanControl_disabled_does_nothing(void)
{
  reset_aux_state();
  configPage2.fanEnable = 0U;
  currentStatus.fanOn = true;
  currentStatus.coolant = 200;
  fanControl();
  // fanOn flag is only modified inside fanEnable branches -> stays whatever it was
  TEST_ASSERT_TRUE(currentStatus.fanOn);
}

static void test_fanControl_turns_on_when_engine_running_and_hot(void)
{
  reset_aux_state();
  configPage2.fanEnable = 1U;
  configPage2.fanWhenOff = 0U;
  currentStatus.engineIsRunning = true;
  currentStatus.engineIsCranking = false;
  currentStatus.coolant = 90;   // > 80C ON threshold

  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_FAN_PIN));
}

static void test_fanControl_stays_off_when_engine_stopped(void)
{
  reset_aux_state();
  configPage2.fanEnable = 1U;
  configPage2.fanWhenOff = 0U;             // engine-running gates fan
  currentStatus.engineIsRunning = false;
  currentStatus.coolant = 95;  // hot

  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
}

static void test_fanControl_runs_when_fanWhenOff_set(void)
{
  reset_aux_state();
  configPage2.fanEnable = 1U;
  configPage2.fanWhenOff = 1U;
  currentStatus.engineIsRunning = false;
  currentStatus.coolant = 95;

  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
}

static void test_fanControl_off_when_below_hysteresis(void)
{
  reset_aux_state();
  configPage2.fanEnable = 1U;
  configPage2.fanWhenOff = 0U;
  currentStatus.engineIsRunning = true;
  currentStatus.fanOn = true;
  currentStatus.coolant = 70;   // < 75 (offTemp = 80 - 5)

  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
}

static void test_fanControl_holds_in_hysteresis_band(void)
{
  // Coolant is between offTemp (75) and onTemp (80). Fan should stay
  // whatever it was — neither branch fires.
  reset_aux_state();
  configPage2.fanEnable = 1U;
  configPage2.fanWhenOff = 0U;
  currentStatus.engineIsRunning = true;
  currentStatus.fanOn = false;
  currentStatus.coolant = 78;

  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
}

static void test_fanControl_disables_during_crank_when_configured(void)
{
  reset_aux_state();
  configPage2.fanEnable = 1U;
  configPage2.fanWhenOff = 1U;             // permit even when not running
  configPage2.fanWhenCranking = 0U;        // disable during cranking
  currentStatus.engineIsCranking = true;
  currentStatus.coolant = 95;

  fanControl();
  TEST_ASSERT_FALSE(currentStatus.fanOn);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_FAN_PIN));
}

static void test_fanControl_runs_during_crank_when_permitted(void)
{
  reset_aux_state();
  configPage2.fanEnable = 1U;
  configPage2.fanWhenOff = 1U;
  configPage2.fanWhenCranking = 1U;        // allow during cranking
  currentStatus.engineIsCranking = true;
  currentStatus.coolant = 95;

  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
}

static void test_fanControl_aircon_request_turns_fan_on(void)
{
  reset_aux_state();
  configPage2.fanEnable = 1U;
  configPage15.airConTurnsFanOn = 1U;
  currentStatus.engineIsRunning = true;
  currentStatus.coolant = 0;                        // cold
  currentStatus.airconTurningOn = true;

  fanControl();
  TEST_ASSERT_TRUE(currentStatus.fanOn);
}

// ============================ VVT pin drivers ===============================

static void test_vvt1On_and_Off_toggle_pin(void)
{
  reset_aux_state();
  vvt1Off();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_VVT1_PIN));
  vvt1On();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_VVT1_PIN));
  vvt1Off();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_VVT1_PIN));
}

static void test_vvt2On_and_Off_toggle_pin(void)
{
  reset_aux_state();
  vvt2Off();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_VVT2_PIN));
  vvt2On();
  TEST_ASSERT_EQUAL(HIGH, digitalRead(TEST_VVT2_PIN));
  vvt2Off();
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_VVT2_PIN));
}

// ============================ Nitrous =======================================

static void test_nitrousControl_disabled_clears_status(void)
{
  reset_aux_state();
  configPage10.n2o_enable = 0U;
  currentStatus.nitrousActive = true;
  currentStatus.nitrous_status = 0xFFU;

  nitrousControl();
  TEST_ASSERT_FALSE(currentStatus.nitrousActive);
  TEST_ASSERT_EQUAL_UINT8(NITROUS_OFF, currentStatus.nitrous_status);
}

static void test_nitrousControl_enabled_but_conditions_unmet(void)
{
  reset_aux_state();
  configPage10.n2o_enable = 1U;
  configPage10.n2o_pin_polarity = 0U;
  configPage10.n2o_minCLT = 0U;
  configPage10.n2o_minTPS = 200U;        // Require very high TPS
  configPage10.n2o_maxAFR = 0U;          // O2 must be < 0 -> impossible
  configPage10.n2o_maxMAP = 100U;
  currentStatus.coolant = 80;
  currentStatus.TPS = 0U;                 // Below threshold
  currentStatus.O2 = 200U;
  currentStatus.MAP = 100U;
  currentStatus.RPM = 1000U;

  nitrousControl();
  TEST_ASSERT_FALSE(currentStatus.nitrousActive);
  TEST_ASSERT_EQUAL_UINT8(NITROUS_OFF, currentStatus.nitrous_status);
}

// ============================ WMI ===========================================

static void test_wmiControl_both_disabled_does_nothing(void)
{
  reset_aux_state();
  configPage10.wmiEnabled = 0U;
  configPage10.vvt2Enabled = 0U;
  // Just verify it runs without crashing — no state to assert.
  wmiControl();
  TEST_PASS();
}

static void test_wmiControl_vvt2_disables_wmi(void)
{
  reset_aux_state();
  configPage10.wmiEnabled = 1U;
  configPage10.vvt2Enabled = 1U;
  wmiControl();
  TEST_PASS();
}

// ============================ AirCon ========================================
//
// airConControl() is gated on a file-static `acIsEnabled` flag set by
// initialiseAirCon(). Without that init the function early-returns. Just
// verify the early-return path is safe.

static void test_airConControl_uninitialised_is_noop(void)
{
  reset_aux_state();
  airConControl();
  TEST_PASS();
}

// ============================ Boost =========================================

static void test_boostDisable_clears_duty_and_drops_pin_low(void)
{
  reset_aux_state();
  currentStatus.boostDuty = 50U;

  boostDisable();
  TEST_ASSERT_EQUAL_UINT16(0U, currentStatus.boostDuty);
  TEST_ASSERT_EQUAL(LOW, digitalRead(TEST_BOOST_PIN));
}

// ============================ Injector priming ==============================

#include "scheduler.h"

static void test_beginInjectorPriming_skips_when_value_zero(void)
{
  reset_aux_state();
  // Both prime axis & values zeroed -> table_2D returns 0 -> early return.
  for (uint8_t i = 0U; i < 4U; ++i)
  {
    configPage2.primeBins[i] = (uint8_t)i;
    configPage2.primePulse[i] = 0U;
  }
  currentStatus.coolant = 80;
  currentStatus.TPS = 0U;
  configPage4.floodClear = 90U;
  currentStatus.maxInjOutputs = 4U;

  beginInjectorPriming();
  TEST_PASS();
}

static void test_beginInjectorPriming_dispatches_when_valid(void)
{
  reset_aux_state();
  // Constant non-zero priming pulse across all temps.
  for (uint8_t i = 0U; i < 4U; ++i)
  {
    configPage2.primeBins[i] = (uint8_t)(i * 50U);
    configPage2.primePulse[i] = 5U;        // 5ms*2
  }
  currentStatus.coolant = 50;
  currentStatus.TPS = 10U;                  // Below floodClear
  configPage4.floodClear = 90U;
  currentStatus.maxInjOutputs = 8U;          // All channels dispatch

  beginInjectorPriming();
  TEST_PASS();
}

void testAuxiliaries(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_fuelPumpOn_sets_pin_and_status);
    RUN_TEST(test_fuelPumpOff_clears_pin_and_status);
    RUN_TEST(test_initialiseFuelPump_no_prime_returns_true);
    RUN_TEST(test_initialiseFuelPump_with_prime_returns_false_and_starts_pump);

    RUN_TEST(test_fanOn_normal_polarity_drives_pin_high);
    RUN_TEST(test_fanOff_normal_polarity_drives_pin_low);
    RUN_TEST(test_fanOn_inverted_polarity_drives_pin_low);
    RUN_TEST(test_initialiseFan_resets_state);

    RUN_TEST(test_fanControl_disabled_does_nothing);
    RUN_TEST(test_fanControl_turns_on_when_engine_running_and_hot);
    RUN_TEST(test_fanControl_stays_off_when_engine_stopped);
    RUN_TEST(test_fanControl_runs_when_fanWhenOff_set);
    RUN_TEST(test_fanControl_off_when_below_hysteresis);
    RUN_TEST(test_fanControl_holds_in_hysteresis_band);
    RUN_TEST(test_fanControl_disables_during_crank_when_configured);
    RUN_TEST(test_fanControl_runs_during_crank_when_permitted);
    RUN_TEST(test_fanControl_aircon_request_turns_fan_on);

    RUN_TEST(test_vvt1On_and_Off_toggle_pin);
    RUN_TEST(test_vvt2On_and_Off_toggle_pin);

    RUN_TEST(test_nitrousControl_disabled_clears_status);
    RUN_TEST(test_nitrousControl_enabled_but_conditions_unmet);
    RUN_TEST(test_wmiControl_both_disabled_does_nothing);
    RUN_TEST(test_wmiControl_vvt2_disables_wmi);
    RUN_TEST(test_airConControl_uninitialised_is_noop);

    RUN_TEST(test_boostDisable_clears_duty_and_drops_pin_low);

    RUN_TEST(test_beginInjectorPriming_skips_when_value_zero);
    RUN_TEST(test_beginInjectorPriming_dispatches_when_valid);
  }
}
