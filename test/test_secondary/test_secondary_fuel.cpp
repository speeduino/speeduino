#include <unity.h>
#include <avr/pgmspace.h>
#include "secondaryTables.h"
#include "globals.h"
#include "../test_utils.h"
#include "storage.h"

TEST_DATA_P table3d_axis_t tempXAxis[] = {500, 700, 900, 1200, 1600, 2000, 2500, 3100, 3500, 4100, 4700, 5300, 5900, 6500, 6750, 7000};
TEST_DATA_P table3d_axis_t tempYAxis[] = {16, 26, 30, 36, 40, 46, 50, 56, 60, 66, 70, 76, 86, 90, 96, 100};

static void __attribute__((noinline)) assert_2nd_fuel_is_off(const statuses &current, uint8_t expectedVE) {
    TEST_ASSERT_BIT_LOW(BIT_STATUS3_FUEL2_ACTIVE, current.status3);
    TEST_ASSERT_EQUAL(expectedVE, current.VE1);
    TEST_ASSERT_EQUAL(0, current.VE2);
    TEST_ASSERT_EQUAL(current.VE1, current.VE);
} 

static void __attribute__((noinline)) assert_2nd_fuel_is_on(const statuses &current, uint8_t expectedVE1, uint8_t expectedVE2, uint8_t expectedVE) {
    TEST_ASSERT_BIT_HIGH(BIT_STATUS3_FUEL2_ACTIVE, current.status3);
    TEST_ASSERT_EQUAL(expectedVE1, current.VE1);
    TEST_ASSERT_EQUAL(expectedVE2, current.VE2);
    TEST_ASSERT_EQUAL(expectedVE, current.VE);
} 

static void test_no_secondary_fuel(void) {
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    page10.fuel2Mode = FUEL2_MODE_OFF;
    page10.fuel2Algorithm = LOAD_SOURCE_MAP;
    current.VE1 = 50;
    current.VE = current.VE1;

    calculateSecondaryFuel(page10, lookupTable, current);

    assert_2nd_fuel_is_off(current, 50);
}

static constexpr int16_t SIMPLE_LOAD_VALUE = 150;

static void __attribute__((noinline)) test_fuel_mode_cap_UINT8_MAX(uint8_t mode) {
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    fill_table_values(lookupTable, SIMPLE_LOAD_VALUE);
    populate_table_axis_P(lookupTable.axisX.begin(), tempXAxis);
    populate_table_axis_P(lookupTable.axisY.begin(), tempYAxis);

    page10.fuel2Mode = mode;    
    page10.fuel2Algorithm = LOAD_SOURCE_MAP;
    current.VE1 = 200;
    current.VE = current.VE1;
    current.MAP = 100; //Load source value
    current.RPM = 7000;

    calculateSecondaryFuel(page10, lookupTable, current);

    assert_2nd_fuel_is_on(current, 200, 150, UINT8_MAX);
}

static constexpr int8_t SIMPLE_VE1 = 75;

static void __attribute__((noinline)) setup_test_fuel_mode_simple(config10 &page10, statuses &current, table3d16RpmLoad &lookupTable, uint8_t mode) {
    fill_table_values(lookupTable, SIMPLE_LOAD_VALUE);
    populate_table_axis_P(lookupTable.axisX.begin(), tempXAxis);
    populate_table_axis_P(lookupTable.axisY.begin(), tempYAxis);

    page10.fuel2Mode = mode;    
    page10.fuel2Algorithm = LOAD_SOURCE_MAP;
    current.VE1 = SIMPLE_VE1;
    current.VE = current.VE1;
    current.MAP = tempYAxis[0]; //Load source value
    current.RPM = tempXAxis[0];
}
static void __attribute__((noinline)) test_fuel_mode_simple(uint8_t mode, uint8_t expectedVE) {
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    setup_test_fuel_mode_simple(page10, current, lookupTable, mode);

    calculateSecondaryFuel(page10, lookupTable, current);

    assert_2nd_fuel_is_on(current, SIMPLE_VE1, SIMPLE_LOAD_VALUE, expectedVE);
}

static void test_fuel_mode_multiply_cap_UINT8_MAX(void) {
    test_fuel_mode_cap_UINT8_MAX(FUEL2_MODE_MULTIPLY);
}

static void test_fuel_mode_multiply(void) {
    test_fuel_mode_simple(FUEL2_MODE_MULTIPLY, ((SIMPLE_VE1*SIMPLE_LOAD_VALUE)/100)+1);
}

static void test_fuel_mode_add(void) {
    test_fuel_mode_simple(FUEL2_MODE_ADD, SIMPLE_VE1+SIMPLE_LOAD_VALUE);
}

static void test_fuel_mode_add_cap_UINT8_MAX(void) {
    test_fuel_mode_cap_UINT8_MAX(FUEL2_MODE_ADD);
}

static constexpr int16_t SWITCHED_LOAD = 50;
static constexpr int16_t SWITCHED_VE2 = 68;

static void __attribute__((noinline)) setup_test_fuel_mode_cond_switch(config10 &page10, statuses &current, table3d16RpmLoad &lookupTable, uint8_t cond, uint16_t trigger) {
    setup_test_fuel_mode_simple(page10, current, lookupTable, FUEL2_MODE_CONDITIONAL_SWITCH);
    fill_table_values(lookupTable, SWITCHED_VE2);
    page10.fuel2SwitchVariable = cond;
    page10.fuel2SwitchValue = trigger;

    current.MAP = SWITCHED_LOAD; //Load source value
    current.RPM = 3500;
    current.TPS = 50;
    current.ethanolPct = 50;
}

static void __attribute__((noinline)) test_fuel_mode_cond_switch_negative(uint8_t cond, uint16_t trigger) {
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;
    setup_test_fuel_mode_cond_switch(page10, current, lookupTable, cond, trigger);

    calculateSecondaryFuel(page10, lookupTable, current);

    assert_2nd_fuel_is_off(current, SIMPLE_VE1);    
}

static void __attribute__((noinline)) test_fuel_mode_cond_switch_positive(uint8_t cond, uint16_t trigger) {
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;
    setup_test_fuel_mode_cond_switch(page10, current, lookupTable, cond, trigger);

    calculateSecondaryFuel(page10, lookupTable, current);

    assert_2nd_fuel_is_on(current, SIMPLE_VE1, SWITCHED_VE2, SWITCHED_VE2);
}

static void __attribute__((noinline)) test_fuel_mode_cond_switch_rpm(void) {
    test_fuel_mode_cond_switch_positive(FUEL2_CONDITION_RPM, 3499);    
    test_fuel_mode_cond_switch_negative(FUEL2_CONDITION_RPM, 3501);    
    test_fuel_mode_cond_switch_positive(FUEL2_CONDITION_RPM, 3499);    
}

static void __attribute__((noinline)) test_fuel_mode_cond_switch_tps(void) {
    test_fuel_mode_cond_switch_positive(FUEL2_CONDITION_TPS, 49);    
    test_fuel_mode_cond_switch_negative(FUEL2_CONDITION_TPS, 51);    
    test_fuel_mode_cond_switch_positive(FUEL2_CONDITION_TPS, 49);    
}

static void __attribute__((noinline)) test_fuel_mode_cond_switch_map(void) {
    test_fuel_mode_cond_switch_positive(FUEL2_CONDITION_MAP, 49);    
    test_fuel_mode_cond_switch_negative(FUEL2_CONDITION_MAP, 51);    
    test_fuel_mode_cond_switch_positive(FUEL2_CONDITION_MAP, 49);    
}

static void __attribute__((noinline)) test_fuel_mode_cond_switch_ethanol_pct(void) {
    test_fuel_mode_cond_switch_positive(FUEL2_CONDITION_ETH, 49);    
    test_fuel_mode_cond_switch_negative(FUEL2_CONDITION_ETH, 51);    
    test_fuel_mode_cond_switch_positive(FUEL2_CONDITION_ETH, 49);    
}

static void __attribute__((noinline)) test_fuel_mode_input_switch(void) {
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;
    setup_test_fuel_mode_simple(page10, current, lookupTable, FUEL2_MODE_INPUT_SWITCH);
    fill_table_values(lookupTable, SWITCHED_VE2);

    page10.fuel2InputPolarity = HIGH;
    pinFuel2Input = 3;   
    pinMode(pinFuel2Input, OUTPUT);

    // On
    digitalWrite(pinFuel2Input, page10.fuel2InputPolarity);
    calculateSecondaryFuel(page10, lookupTable, current);
    assert_2nd_fuel_is_on(current, SIMPLE_VE1, SWITCHED_VE2, SWITCHED_VE2);

    // Off
    digitalWrite(pinFuel2Input, !page10.fuel2InputPolarity);
    current.VE = current.VE1;
    calculateSecondaryFuel(page10, lookupTable, current);
    assert_2nd_fuel_is_off(current, SIMPLE_VE1);

    // On again
    digitalWrite(pinFuel2Input, page10.fuel2InputPolarity);
    calculateSecondaryFuel(page10, lookupTable, current);
    assert_2nd_fuel_is_on(current, SIMPLE_VE1, SWITCHED_VE2, SWITCHED_VE2);
}

void test_calculateSecondaryFuel(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST(test_no_secondary_fuel);
        RUN_TEST(test_fuel_mode_multiply);
        RUN_TEST(test_fuel_mode_multiply_cap_UINT8_MAX);
        RUN_TEST(test_fuel_mode_add);
        RUN_TEST(test_fuel_mode_add_cap_UINT8_MAX);
        RUN_TEST(test_fuel_mode_cond_switch_rpm);
        RUN_TEST(test_fuel_mode_cond_switch_tps);
        RUN_TEST(test_fuel_mode_cond_switch_map);
        RUN_TEST(test_fuel_mode_cond_switch_ethanol_pct);
        RUN_TEST(test_fuel_mode_input_switch);
    }
}