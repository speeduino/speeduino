#include "globals.h"
#include "init.h"
#include "../test_utils.h"

void prepareForInitialiseAll(uint8_t boardId);
uint8_t getPinMode(uint8_t pin);

#if defined(CORE_AVR) || defined(NATIVE_BOARD)
static void prepare_pressure_inputs(void)
{
  prepareForInitialiseAll(3);
  configPage10.fuelPressurePin = 8; // A8 on Mega2560
  configPage10.oilPressurePin = 9; // A9 on Mega2560
  // A sentinel catches unwanted mode changes when disabled or conflicting.
  pinMode(A8, OUTPUT);
  pinMode(A9, OUTPUT);
}
#endif

static void test_enabled_pressure_inputs(void)
{
#if defined(CORE_AVR) || defined(NATIVE_BOARD)
  prepare_pressure_inputs();
  configPage10.fuelPressureEnable = true;
  configPage10.oilPressureEnable = true;
  setPinMapping(3);

  TEST_ASSERT_EQUAL_UINT8(A8, pinNumbers.pinFuelPressure);
  TEST_ASSERT_EQUAL_UINT8(A9, pinNumbers.pinOilPressure);
  TEST_ASSERT_EQUAL_UINT8(INPUT, getPinMode(A8));
  TEST_ASSERT_EQUAL_UINT8(INPUT, getPinMode(A9));
#else
  TEST_IGNORE_MESSAGE("Pin-mode readback uses Mega2560 registers");
#endif
}

static void test_disabled_pressure_inputs_leave_pins_unchanged(void)
{
#if defined(CORE_AVR) || defined(NATIVE_BOARD)
  prepare_pressure_inputs();
  configPage10.fuelPressureEnable = false;
  configPage10.oilPressureEnable = false;
  setPinMapping(3);

  TEST_ASSERT_EQUAL_UINT8(OUTPUT, getPinMode(A8));
  TEST_ASSERT_EQUAL_UINT8(OUTPUT, getPinMode(A9));
#else
  TEST_IGNORE_MESSAGE("Pin-mode readback uses Mega2560 registers");
#endif
}

static void test_pressure_inputs_do_not_override_outputs(void)
{
#if defined(CORE_AVR) || defined(NATIVE_BOARD)
  prepare_pressure_inputs();
  configPage10.fuelPressureEnable = true;
  configPage10.oilPressureEnable = true;
  // Out-of-range analog indices are passed through as physical pin numbers.
  // Even a bad pressure-pin assignment must not reconfigure active outputs.
  configPage10.fuelPressurePin = 17;
  configPage2.tachoPin = 17;
  configPage10.oilPressurePin = 18;
  configPage4.fuelPumpPin = 18;
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  setPinMapping(3);

  TEST_ASSERT_EQUAL_UINT8(pinNumbers.pinTachOut, pinNumbers.pinFuelPressure);
  TEST_ASSERT_EQUAL_UINT8(pinNumbers.pinFuelPump, pinNumbers.pinOilPressure);
  TEST_ASSERT_EQUAL_UINT8(OUTPUT, getPinMode(17));
  TEST_ASSERT_EQUAL_UINT8(OUTPUT, getPinMode(18));
#else
  TEST_IGNORE_MESSAGE("Pin-mode readback uses Mega2560 registers");
#endif
}

void testPressureInputs(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_enabled_pressure_inputs);
    RUN_TEST_P(test_disabled_pressure_inputs_leave_pins_unchanged);
    RUN_TEST_P(test_pressure_inputs_do_not_override_outputs);
  }
}
