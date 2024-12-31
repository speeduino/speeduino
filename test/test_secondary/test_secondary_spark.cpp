#include <unity.h>
#include <avr/pgmspace.h>
#include "secondaryTables.h"
#include "globals.h"
#include "../test_utils.h"
#include "storage.h"
// #include "corrections.h"
#include "maths.h"

TEST_DATA_P table3d_axis_t tempXAxis[] = {500, 700, 900, 1200, 1600, 2000, 2500, 3100, 3500, 4100, 4700, 5300, 5900, 6500, 6750, 7000};
TEST_DATA_P table3d_axis_t tempYAxis[] = {16, 26, 30, 36, 40, 46, 50, 56, 60, 66, 70, 76, 86, 90, 96, 100};

static void __attribute__((noinline)) assert_2nd_spark_is_off(const statuses &current, int8_t expectedAdvance) {
    TEST_ASSERT_BIT_LOW(BIT_STATUS5_SPARK2_ACTIVE, current.status5);
    TEST_ASSERT_EQUAL(expectedAdvance, current.advance1);
    TEST_ASSERT_EQUAL(0, current.advance2);
    TEST_ASSERT_EQUAL(current.advance1, current.advance);
} 

static void __attribute__((noinline)) assert_2nd_spark_is_on(const statuses &current, int8_t expectedAdvance1, int8_t expectedAdvance2, int8_t expectedAdvance) {
    TEST_ASSERT_BIT_HIGH(BIT_STATUS5_SPARK2_ACTIVE, current.status5);
    TEST_ASSERT_EQUAL(expectedAdvance1, current.advance1);
    TEST_ASSERT_EQUAL(expectedAdvance2, current.advance2);
    TEST_ASSERT_EQUAL(expectedAdvance, current.advance);
} 

static void __attribute__((noinline)) test_mode_off_no_secondary_spark(void) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    page10.spark2Mode = SPARK2_MODE_OFF;
    page10.spark2Algorithm = LOAD_SOURCE_MAP;
    current.advance1 = 50;
    current.advance = current.advance1;

    calculateSecondarySpark(page2, page10, lookupTable, current);

    assert_2nd_spark_is_off(current, 50);
}

static constexpr int8_t CAP_ADVANCE1 = INT8_MAX - 1;
static constexpr int16_t CAP_LOAD_LOOKUP_RESULT = 150;
static constexpr int16_t CAP_LOAD_VALUE = CAP_LOAD_LOOKUP_RESULT-INT16_C(OFFSET_IGNITION);

static void __attribute__((noinline)) setup_test_mode_cap_INT8_MAX(config2 &, config10 &page10, statuses &current, table3d16RpmLoad &lookupTable, uint8_t mode) {
    page10.spark2Mode = mode;    
    page10.spark2Algorithm = LOAD_SOURCE_MAP;
    current.advance1 = CAP_ADVANCE1;
    current.advance = current.advance1;
    current.MAP = tempYAxis[0];
    current.RPM = tempXAxis[0];
    fill_table_values(lookupTable, CAP_LOAD_LOOKUP_RESULT);
    populate_table_axis_P(lookupTable.axisX.begin(), tempXAxis);
    populate_table_axis_P(lookupTable.axisY.begin(), tempYAxis);
}

static void __attribute__((noinline)) test_mode_cap_INT8_MAX(uint8_t mode, int8_t expectedAdvance2) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    setup_test_mode_cap_INT8_MAX(page2, page10, current, lookupTable, mode);

    calculateSecondarySpark(page2, page10, lookupTable, current);

    assert_2nd_spark_is_on(current, CAP_ADVANCE1, expectedAdvance2, INT8_MAX);
}

static constexpr int8_t SIMPLE_ADVANCE1 = 35;
static constexpr int16_t SIMPLE_LOAD_LOOKUP_RESULT = 68;
static constexpr int16_t SIMPLE_LOAD_VALUE = SIMPLE_LOAD_LOOKUP_RESULT-INT16_C(OFFSET_IGNITION);

static void __attribute__((noinline)) setup_test_mode_simple(config2 &, config10 &page10, statuses &current, table3d16RpmLoad &lookupTable, uint8_t mode) {
    page10.spark2Mode = mode;    
    page10.spark2Algorithm = LOAD_SOURCE_MAP;
    current.advance1 = SIMPLE_ADVANCE1;
    current.advance = current.advance1;
    current.MAP = tempYAxis[0];
    current.RPM = tempXAxis[0];
    fill_table_values(lookupTable, SIMPLE_LOAD_LOOKUP_RESULT);
    populate_table_axis_P(lookupTable.axisX.begin(), tempXAxis);
    populate_table_axis_P(lookupTable.axisY.begin(), tempYAxis);
}

static void __attribute__((noinline)) test_mode_simple(uint8_t mode, int8_t expectedAdvance, int8_t expectedAdvance2) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    setup_test_mode_simple(page2, page10, current, lookupTable, mode);
    calculateSecondarySpark(page2, page10, lookupTable, current);
    assert_2nd_spark_is_on(current, SIMPLE_ADVANCE1, expectedAdvance2, expectedAdvance);
}

static void __attribute__((noinline)) test_sparkmode_multiply_cap_INT8_MAX(void) {
    test_mode_cap_INT8_MAX(SPARK2_MODE_MULTIPLY, CAP_LOAD_VALUE-INT8_MAX);
}

static void __attribute__((noinline)) test_sparkmode_multiply(uint8_t multiplier) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    setup_test_mode_simple(page2, page10, current, lookupTable, SPARK2_MODE_MULTIPLY);
    fill_table_values(lookupTable, multiplier+INT16_C(OFFSET_IGNITION));
    calculateSecondarySpark(page2, page10, lookupTable, current);
    assert_2nd_spark_is_on(current, SIMPLE_ADVANCE1, multiplier-INT8_MAX, DIV_ROUND_CLOSEST((SIMPLE_ADVANCE1*multiplier), 100, int16_t));
}

static void __attribute__((noinline)) test_sparkmode_multiply_0(void) {
    test_sparkmode_multiply(0);
}

static void __attribute__((noinline)) test_sparkmode_multiply_50(void) {
    test_sparkmode_multiply(50);
}

static void __attribute__((noinline)) test_sparkmode_multiply_100(void) {
    test_sparkmode_multiply(100);
}

static void __attribute__((noinline)) test_sparkmode_multiply_150(void) {
    test_sparkmode_multiply(150);
}

static void __attribute__((noinline)) test_sparkmode_multiply_200(void) {
    test_sparkmode_multiply(200);
}

static void __attribute__((noinline)) test_sparkmode_multiply_215(void) {
    test_sparkmode_multiply(215);
}

static void __attribute__((noinline)) test_fixed_timing_no_secondary_spark(void) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    setup_test_mode_simple(page2, page10, current, lookupTable, SPARK2_MODE_MULTIPLY);
    page2.fixAngEnable = 1U;// Should turn 2nd table off
    calculateSecondarySpark(page2, page10, lookupTable, current);
    assert_2nd_spark_is_off(current, SIMPLE_ADVANCE1);
}

static void __attribute__((noinline)) test_cranking_no_secondary_spark(void) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    setup_test_mode_simple(page2, page10, current, lookupTable, SPARK2_MODE_MULTIPLY);
    BIT_SET(current.engine, BIT_ENGINE_CRANK);// Should turn 2nd table off
    calculateSecondarySpark(page2, page10, lookupTable, current);
    assert_2nd_spark_is_off(current, SIMPLE_ADVANCE1);
}

static void __attribute__((noinline)) test_sparkmode_add(void) {
    test_mode_simple(SPARK2_MODE_ADD, SIMPLE_ADVANCE1+SIMPLE_LOAD_VALUE, SIMPLE_LOAD_VALUE);
}

static void __attribute__((noinline)) test_sparkmode_add_cap_INT8_MAX(void) {
    test_mode_cap_INT8_MAX(SPARK2_MODE_ADD, (int16_t)150-INT16_C(OFFSET_IGNITION));
}

static void __attribute__((noinline)) setup_test_mode_cond_switch(config2 &page2, config10 &page10, statuses &current, table3d16RpmLoad &lookupTable, uint8_t cond, uint16_t trigger) {
    setup_test_mode_simple(page2, page10, current, lookupTable, SPARK2_MODE_CONDITIONAL_SWITCH);
    page10.spark2SwitchVariable = cond;
    page10.spark2SwitchValue = trigger;
    current.MAP = 50; //Load source value
    current.RPM = 3500;
    current.TPS = 50;
    current.ethanolPct = 50;
}

static void __attribute__((noinline)) test_sparkmode_cond_switch_negative(uint8_t cond, uint16_t trigger) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    setup_test_mode_cond_switch(page2, page10, current, lookupTable, cond, trigger);

    calculateSecondarySpark(page2, page10, lookupTable, current);

    assert_2nd_spark_is_off(current, SIMPLE_ADVANCE1);    
}

static void __attribute__((noinline)) test_sparkmode_cond_switch_positive(uint8_t cond, uint16_t trigger) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    setup_test_mode_cond_switch(page2, page10, current, lookupTable, cond, trigger);

    calculateSecondarySpark(page2, page10, lookupTable, current);

    assert_2nd_spark_is_on(current, SIMPLE_ADVANCE1, SIMPLE_LOAD_VALUE, SIMPLE_LOAD_VALUE);
}

static void __attribute__((noinline)) test_sparkmode_cond_switch_rpm(void) {
    test_sparkmode_cond_switch_positive(SPARK2_CONDITION_RPM, 3499);    
    test_sparkmode_cond_switch_negative(SPARK2_CONDITION_RPM, 3501);    
    test_sparkmode_cond_switch_positive(SPARK2_CONDITION_RPM, 3499);    
}

static void __attribute__((noinline)) test_sparkmode_cond_switch_tps(void) {
    test_sparkmode_cond_switch_positive(SPARK2_CONDITION_TPS, 49);    
    test_sparkmode_cond_switch_negative(SPARK2_CONDITION_TPS, 51);    
    test_sparkmode_cond_switch_positive(SPARK2_CONDITION_TPS, 49);    
}

static void __attribute__((noinline)) test_sparkmode_cond_switch_map(void) {
    test_sparkmode_cond_switch_positive(SPARK2_CONDITION_MAP, 49);    
    test_sparkmode_cond_switch_negative(SPARK2_CONDITION_MAP, 51);    
    test_sparkmode_cond_switch_positive(SPARK2_CONDITION_MAP, 49);    
}

static void __attribute__((noinline)) test_sparkmode_cond_switch_ethanol_pct(void) {
    test_sparkmode_cond_switch_positive(SPARK2_CONDITION_ETH, 49);    
    test_sparkmode_cond_switch_negative(SPARK2_CONDITION_ETH, 51);    
    test_sparkmode_cond_switch_positive(SPARK2_CONDITION_ETH, 49);    
}

static void __attribute__((noinline)) test_sparkmode_input_switch(void) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    table3d16RpmLoad lookupTable;

    setup_test_mode_simple(page2, page10, current, lookupTable, SPARK2_MODE_INPUT_SWITCH);

    page10.spark2InputPolarity = HIGH;
    pinSpark2Input = 3;   
    pinMode(pinSpark2Input, OUTPUT);

    // On
    digitalWrite(pinSpark2Input, page10.spark2InputPolarity);
    calculateSecondarySpark(page2, page10, lookupTable, current);
    assert_2nd_spark_is_on(current, SIMPLE_ADVANCE1, SIMPLE_LOAD_VALUE, SIMPLE_LOAD_VALUE);

    // Off
    digitalWrite(pinSpark2Input, !page10.spark2InputPolarity);
    current.advance = current.advance1;
    calculateSecondarySpark(page2, page10, lookupTable, current);
    assert_2nd_spark_is_off(current, SIMPLE_ADVANCE1);

    // On again
    digitalWrite(pinSpark2Input, page10.spark2InputPolarity);
    calculateSecondarySpark(page2, page10, lookupTable, current);
    assert_2nd_spark_is_on(current, SIMPLE_ADVANCE1, SIMPLE_LOAD_VALUE, SIMPLE_LOAD_VALUE);
}

void test_calculateSecondarySpark(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST(test_mode_off_no_secondary_spark);
        RUN_TEST(test_fixed_timing_no_secondary_spark);
        RUN_TEST(test_cranking_no_secondary_spark);
        RUN_TEST(test_sparkmode_multiply_0);
        RUN_TEST(test_sparkmode_multiply_50);
        RUN_TEST(test_sparkmode_multiply_100);
        RUN_TEST(test_sparkmode_multiply_150);
        RUN_TEST(test_sparkmode_multiply_200);
        RUN_TEST(test_sparkmode_multiply_215);
        RUN_TEST(test_sparkmode_multiply_cap_INT8_MAX); 
        RUN_TEST(test_sparkmode_add);
        RUN_TEST(test_sparkmode_add_cap_INT8_MAX);
        RUN_TEST(test_sparkmode_cond_switch_rpm);
        RUN_TEST(test_sparkmode_cond_switch_tps);
        RUN_TEST(test_sparkmode_cond_switch_map);
        RUN_TEST(test_sparkmode_cond_switch_ethanol_pct);
        RUN_TEST(test_sparkmode_input_switch);
    }
}