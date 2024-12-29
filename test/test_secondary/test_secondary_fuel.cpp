#include <unity.h>
#include <avr/pgmspace.h>
#include "secondaryTables.h"
#include "globals.h"
#include "../test_utils.h"
#include "storage.h"


TEST_DATA_P table3d_value_t fuel2TableValues[] = {
 //0    1    2   3     4    5    6    7    8    9   10   11   12   13    14   15
34,  34,  34,  34,  34,  34,  34,  34,  34,  35,  35,  35,  35,  35,  35,  35, 
34,  35,  36,  37,  39,  41,  42,  43,  43,  44,  44,  44,  44,  44,  44,  44, 
35,  36,  38,  41,  44,  46,  47,  48,  48,  49,  49,  49,  49,  49,  49,  49, 
36,  39,  42,  46,  50,  51,  52,  53,  53,  53,  53,  53,  53,  53,  53,  53, 
38,  43,  48,  52,  55,  56,  57,  58,  58,  58,  58,  58,  58,  58,  58,  58, 
42,  49,  54,  58,  61,  62,  62,  63,  63,  63,  63,  63,  63,  63,  63,  63, 
48,  56,  60,  64,  66,  66,  68,  68,  68,  68,  68,  68,  68,  68,  68,  68, 
54,  62,  66,  69,  71,  71,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72, 
61,  69,  72,  74,  76,  76,  77,  77,  77,  77,  77,  77,  77,  77,  77,  77, 
68,  75,  78,  79,  81,  81,  81,  82,  82,  82,  82,  82,  82,  82,  82,  82, 
74,  80,  83,  84,  85,  86,  86,  86,  87,  87,  87,  87,  87,  87,  87,  87, 
81,  86,  88,  89,  90,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91, 
93,  96,  98,  99,  99,  100, 100, 101, 101, 101, 101, 101, 101, 101, 101, 101, 
98,  101, 103, 103, 104, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 
104, 106, 107, 108, 109, 109, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 
150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
  };
TEST_DATA_P table3d_axis_t tempXAxis[] = {500, 700, 900, 1200, 1600, 2000, 2500, 3100, 3500, 4100, 4700, 5300, 5900, 6500, 6750, 7000};
TEST_DATA_P table3d_axis_t tempYAxis[] = {16, 26, 30, 36, 40, 46, 50, 56, 60, 66, 70, 76, 86, 90, 96, 100};

static table3d16RpmLoad veLookupTable;

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

    page10.fuel2Mode = FUEL2_MODE_OFF;
    page10.fuel2Algorithm = LOAD_SOURCE_MAP;
    current.VE1 = 50;
    current.VE = current.VE1;

    calculateSecondaryFuel(page10, veLookupTable, current);

    assert_2nd_fuel_is_off(current, 50);
}

static void __attribute__((noinline)) test_fuel_mode_cap_UINT8_MAX(uint8_t mode) {
    config10 page10 = {};
    statuses current = {};

    page10.fuel2Mode = mode;    
    page10.fuel2Algorithm = LOAD_SOURCE_MAP;
    current.VE1 = 200;
    current.VE = current.VE1;
    current.MAP = 100; //Load source value
    current.RPM = 7000;

    calculateSecondaryFuel(page10, veLookupTable, current);

    assert_2nd_fuel_is_on(current, 200, 150, UINT8_MAX);
}

static constexpr int8_t SIMPLE_VE1 = 75;
static constexpr int16_t SIMPLE_LOAD_VALUE = 150;

static void __attribute__((noinline)) test_fuel_mode_simple(uint8_t mode, uint8_t expectedVE) {
    config10 page10 = {};
    statuses current = {};

    page10.fuel2Mode = mode;    
    page10.fuel2Algorithm = LOAD_SOURCE_MAP;
    current.VE1 = SIMPLE_VE1;
    current.VE = current.VE1;
    current.MAP = 100; //Load source value
    current.RPM = 7000;

    calculateSecondaryFuel(page10, veLookupTable, current);

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

static void __attribute__((noinline)) setup_test_fuel_mode_cond_switch(config10 &page10, statuses &current, uint8_t cond, uint16_t trigger) {
    page10.fuel2Mode = FUEL2_MODE_CONDITIONAL_SWITCH; 
    page10.fuel2SwitchVariable = cond;
    page10.fuel2SwitchValue = trigger;
    page10.fuel2Algorithm = LOAD_SOURCE_MAP;    
    current.VE1 = SIMPLE_VE1;
    current.VE = current.VE1;
    current.MAP = SWITCHED_LOAD; //Load source value
    current.RPM = 3500;
    current.TPS = 50;
    current.ethanolPct = 50;
}

static void __attribute__((noinline)) test_fuel_mode_cond_switch_negative(uint8_t cond, uint16_t trigger) {
    config10 page10 = {};
    statuses current = {};
    setup_test_fuel_mode_cond_switch(page10, current, cond, trigger);

    calculateSecondaryFuel(page10, veLookupTable, current);

    assert_2nd_fuel_is_off(current, SIMPLE_VE1);    
}

static void __attribute__((noinline)) test_fuel_mode_cond_switch_positive(uint8_t cond, uint16_t trigger) {
    config10 page10 = {};
    statuses current = {};
    setup_test_fuel_mode_cond_switch(page10, current, cond, trigger);

    calculateSecondaryFuel(page10, veLookupTable, current);

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

    page10.fuel2Mode = FUEL2_MODE_INPUT_SWITCH; 
    page10.fuel2Algorithm = LOAD_SOURCE_MAP;
    current.VE1 = SIMPLE_VE1;
    current.VE = current.VE1;
    current.MAP = SWITCHED_LOAD; //Load source value
    current.RPM = 3500;
    page10.fuel2InputPolarity = HIGH;
    pinFuel2Input = 3;   
    pinMode(pinFuel2Input, OUTPUT);

    // On
    digitalWrite(pinFuel2Input, page10.fuel2InputPolarity);
    calculateSecondaryFuel(page10, veLookupTable, current);
    assert_2nd_fuel_is_on(current, SIMPLE_VE1, SWITCHED_VE2, SWITCHED_VE2);

    // Off
    digitalWrite(pinFuel2Input, !page10.fuel2InputPolarity);
    current.VE = current.VE1;
    calculateSecondaryFuel(page10, veLookupTable, current);
    assert_2nd_fuel_is_off(current, SIMPLE_VE1);

    // On again
    digitalWrite(pinFuel2Input, page10.fuel2InputPolarity);
    calculateSecondaryFuel(page10, veLookupTable, current);
    assert_2nd_fuel_is_on(current, SIMPLE_VE1, SWITCHED_VE2, SWITCHED_VE2);
}

void test_calculateSecondaryFuel(void)
{
    populate_table_P(veLookupTable, tempXAxis, tempYAxis, fuel2TableValues);

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