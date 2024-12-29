#include <unity.h>
#include <avr/pgmspace.h>
#include "secondaryTables.h"
#include "globals.h"
#include "../test_utils.h"
#include "storage.h"
#include "corrections.h"


TEST_DATA_P table3d_value_t values[] = {
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
93,  96,  98,  99,  99,  107, 107, 101, 101, 101, 101, 101, 101, 101, 101, 101, 
98,  101, 103, 103, 104, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 
104, 106, 107, 108, 109, 109, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 
150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
  };
TEST_DATA_P table3d_axis_t tempXAxis[] = {500, 700, 900, 1200, 1600, 2000, 2500, 3100, 3500, 4100, 4700, 5300, 5900, 6500, 6750, 7000};
TEST_DATA_P table3d_axis_t tempYAxis[] = {16, 26, 30, 36, 40, 46, 50, 56, 60, 66, 70, 76, 86, 90, 96, 100};

static table3d16RpmLoad lookupTable;

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

    page10.spark2Mode = SPARK2_MODE_OFF;
    page10.spark2Algorithm = LOAD_SOURCE_MAP;
    current.advance1 = 50;
    current.advance = current.advance1;

    calculateSecondarySpark(page2, page10, lookupTable, current);

    assert_2nd_spark_is_off(current, 50);
}

static constexpr int8_t CAP_ADVANCE1 = INT8_MAX - 1;
static constexpr int16_t CAP_LOAD_LOOKUP_MAP = 100;
static constexpr uint16_t CAP_LOAD_LOOKUP_RPM = 7000;
static constexpr int16_t CAP_LOAD_LOOKUP_RESULT = 150;
static constexpr int16_t CAP_LOAD_VALUE = CAP_LOAD_LOOKUP_RESULT-INT16_C(OFFSET_IGNITION);

static void __attribute__((noinline)) setup_test_mode_cap_INT8_MAX(config2 &, config10 &page10, statuses &current, uint8_t mode) {
    page10.spark2Mode = mode;    
    page10.spark2Algorithm = LOAD_SOURCE_MAP;
    current.advance1 = CAP_ADVANCE1;
    current.advance = current.advance1;
    current.MAP = CAP_LOAD_LOOKUP_MAP;
    current.RPM = CAP_LOAD_LOOKUP_RPM;
}

static void __attribute__((noinline)) test_mode_cap_INT8_MAX(uint8_t mode, int8_t expectedAdvance2) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};

    setup_test_mode_cap_INT8_MAX(page2, page10, current, mode);

    calculateSecondarySpark(page2, page10, lookupTable, current);

    assert_2nd_spark_is_on(current, CAP_ADVANCE1, expectedAdvance2, INT8_MAX);
}

static constexpr int8_t SIMPLE_ADVANCE1 = 75;
static constexpr int16_t SIMPLE_LOAD_LOOKUP_MAP = 50;
static constexpr uint16_t SIMPLE_LOAD_LOOKUP_RPM = 3500;
static constexpr int16_t SIMPLE_LOAD_LOOKUP_RESULT = 68;
static constexpr int16_t SIMPLE_LOAD_VALUE = SIMPLE_LOAD_LOOKUP_RESULT-INT16_C(OFFSET_IGNITION);

static void __attribute__((noinline)) setup_test_mode_simple(config2 &, config10 &page10, statuses &current, uint8_t mode) {
    page10.spark2Mode = mode;    
    page10.spark2Algorithm = LOAD_SOURCE_MAP;
    current.advance1 = SIMPLE_ADVANCE1;
    current.advance = current.advance1;
    current.MAP = SIMPLE_LOAD_LOOKUP_MAP;
    current.RPM = SIMPLE_LOAD_LOOKUP_RPM;
}

static void __attribute__((noinline)) test_mode_simple(uint8_t mode, int8_t expectedAdvance, int8_t expectedAdvance2) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};

    setup_test_mode_simple(page2, page10, current, mode);
    calculateSecondarySpark(page2, page10, lookupTable, current);
    assert_2nd_spark_is_on(current, SIMPLE_ADVANCE1, expectedAdvance2, expectedAdvance);
}

static void __attribute__((noinline)) test_sparkmode_multiply_cap_INT8_MAX(void) {
    test_mode_cap_INT8_MAX(SPARK2_MODE_MULTIPLY, CAP_LOAD_VALUE-INT8_MAX);
}

static void __attribute__((noinline)) test_sparkmode_multiply(void) {
    test_mode_simple(SPARK2_MODE_MULTIPLY, (SIMPLE_ADVANCE1*SIMPLE_LOAD_VALUE)/100, SIMPLE_LOAD_VALUE-INT8_MAX);
}

static void __attribute__((noinline)) test_fixed_timing_no_secondary_spark(void) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    setup_test_mode_simple(page2, page10, current, SPARK2_MODE_MULTIPLY);
    page2.fixAngEnable = 1U;// Should turn 2nd table off
    calculateSecondarySpark(page2, page10, lookupTable, current);
    assert_2nd_spark_is_off(current, SIMPLE_ADVANCE1);
}

static void __attribute__((noinline)) test_cranking_no_secondary_spark(void) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};
    setup_test_mode_simple(page2, page10, current, SPARK2_MODE_MULTIPLY);
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

static void __attribute__((noinline)) setup_test_mode_cond_switch(config2 &, config10 &page10, statuses &current, uint8_t cond, uint16_t trigger) {
    page10.spark2Mode = SPARK2_MODE_CONDITIONAL_SWITCH; 
    page10.spark2SwitchVariable = cond;
    page10.spark2SwitchValue = trigger;
    page10.spark2Algorithm = LOAD_SOURCE_MAP;    
    current.advance1 = SIMPLE_ADVANCE1;
    current.advance = current.advance1;
    current.MAP = 50; //Load source value
    current.RPM = 3500;
    current.TPS = 50;
    current.ethanolPct = 50;
}

static void __attribute__((noinline)) test_sparkmode_cond_switch_negative(uint8_t cond, uint16_t trigger) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};

    setup_test_mode_cond_switch(page2, page10, current, cond, trigger);

    calculateSecondarySpark(page2, page10, lookupTable, current);

    assert_2nd_spark_is_off(current, SIMPLE_ADVANCE1);    
}

static void __attribute__((noinline)) test_sparkmode_cond_switch_positive(uint8_t cond, uint16_t trigger) {
    config2 page2 = {};
    config10 page10 = {};
    statuses current = {};

    setup_test_mode_cond_switch(page2, page10, current, cond, trigger);

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

    page10.spark2Mode = SPARK2_MODE_INPUT_SWITCH; 
    page10.spark2Algorithm = LOAD_SOURCE_MAP;
    current.advance1 = SIMPLE_ADVANCE1;
    current.advance = current.advance1;
    current.MAP = 50; //Load source value
    current.RPM = 3500;
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
    populate_table_P(lookupTable, tempXAxis, tempYAxis, values);

    SET_UNITY_FILENAME() {
        RUN_TEST(test_mode_off_no_secondary_spark);
        RUN_TEST(test_fixed_timing_no_secondary_spark);
        RUN_TEST(test_cranking_no_secondary_spark);
        RUN_TEST(test_sparkmode_multiply);
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