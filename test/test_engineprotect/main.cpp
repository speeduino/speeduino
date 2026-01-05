#include "../device_test_harness.h"
#include "statuses.h"
#include "config_pages.h"
#include "../test_utils.h"

extern bool checkOilPressureLimit(statuses &current, const config6 &page6, const config10 &page10, uint32_t currMillis);
extern table2D_u8_u8_4 oilPressureProtectTable;
extern uint8_t oilProtStartTime;

extern bool checkBoostLimit(statuses &current, const config6 &page6);

static void setup_oil_protect_table(void) {
    // Simple axis: 0..3 mapped to same min value
    TEST_DATA_P uint8_t bins[] = { 0, 100, 200, 255 };
    TEST_DATA_P uint8_t values[] = { 50, 50, 50, 50 };
    populate_2dtable_P(&oilPressureProtectTable, values, bins);
}

static statuses setup_oil_protect_table_active(void) {
    setup_oil_protect_table();
    statuses current = {};
    current.oilPressure = 40; // below table min of 50
    current.RPMdiv100 = 0;
    return current;
}

static void test_checkOilPressureLimit_disabled(void) {
    config6 page6 = {};
    config10 page10 = {};
    auto current = setup_oil_protect_table_active();

    page6.engineProtectType = PROTECT_CUT_OFF;
    page10.oilPressureProtEnbl = 1;
    page10.oilPressureEnable = 1;
    TEST_ASSERT_FALSE(checkOilPressureLimit(current, page6, page10, millis()));
    TEST_ASSERT_FALSE(current.engineProtectOil);

    page6.engineProtectType = PROTECT_CUT_BOTH;
    page10.oilPressureProtEnbl = 0;
    page10.oilPressureEnable = 0;
    TEST_ASSERT_FALSE(checkOilPressureLimit(current, page6, page10, millis()));
    TEST_ASSERT_FALSE(current.engineProtectOil);

    page6.engineProtectType = PROTECT_CUT_BOTH;
    page10.oilPressureProtEnbl = 1;
    page10.oilPressureEnable = 0;
    TEST_ASSERT_FALSE(checkOilPressureLimit(current, page6, page10, millis()));
    TEST_ASSERT_FALSE(current.engineProtectOil);

    page6.engineProtectType = PROTECT_CUT_BOTH;
    page10.oilPressureProtEnbl = 0;
    page10.oilPressureEnable = 1;
    TEST_ASSERT_FALSE(checkOilPressureLimit(current, page6, page10, millis()));
    TEST_ASSERT_FALSE(current.engineProtectOil);
}

static void test_checkOilPressureLimit_activate_immediate_when_time_zero(void) {
    auto current = setup_oil_protect_table_active();
    config6 page6 = {};
    config10 page10 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page10.oilPressureProtEnbl = 1;
    page10.oilPressureEnable = 1;
    page10.oilPressureProtTime = 0; // immediate

    TEST_ASSERT_TRUE(checkOilPressureLimit(current, page6, page10, millis()));
    TEST_ASSERT_TRUE(current.engineProtectOil);
}

static void test_checkOilPressureLimit_activate_when_time_expires(void) {
    auto current = setup_oil_protect_table_active();
    config6 page6 = {};
    config10 page10 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page10.oilPressureProtEnbl = 1;
    page10.oilPressureEnable = 1;
    page10.oilPressureProtTime = 21; 
    oilProtStartTime = 100;

    TEST_ASSERT_FALSE(checkOilPressureLimit(current, page6, page10, (oilProtStartTime+page10.oilPressureProtTime-1)*100));
    TEST_ASSERT_FALSE(current.engineProtectOil);
    TEST_ASSERT_TRUE(checkOilPressureLimit(current, page6, page10, (oilProtStartTime+page10.oilPressureProtTime)*100));
    TEST_ASSERT_TRUE(current.engineProtectOil);
    TEST_ASSERT_TRUE(checkOilPressureLimit(current, page6, page10, (oilProtStartTime+page10.oilPressureProtTime+1)*100));
    TEST_ASSERT_TRUE(current.engineProtectOil);
}

static void test_checkOilPressureLimit_no_activation_when_above_limit(void) {
    statuses current = {};
    config6 page6 = {};
    config10 page10 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page10.oilPressureProtEnbl = 1;
    page10.oilPressureEnable = 1;
    page10.oilPressureProtTime = 0;

    setup_oil_protect_table();

    current.oilPressure = 60; // above table min
    current.RPMdiv100 = 0;

    TEST_ASSERT_FALSE(checkOilPressureLimit(current, page6, page10, millis()));
    TEST_ASSERT_FALSE(current.engineProtectOil);
}

static void test_checkBoostLimit_disabled_by_engineProtectType(void) {
    statuses current = {};
    config6 page6 = {};

    page6.engineProtectType = PROTECT_CUT_OFF;
    page6.boostCutEnabled = 1;
    page6.boostLimit = 50;
    current.MAP = (long)(page6.boostLimit * 2UL) + 10;

    TEST_ASSERT_FALSE(checkBoostLimit(current, page6));
    TEST_ASSERT_FALSE(current.engineProtectBoostCut);
}

static void test_checkBoostLimit_disabled_by_flag(void) {
    statuses current = {};
    config6 page6 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page6.boostCutEnabled = 0;
    page6.boostLimit = 40;
    current.MAP = (long)(page6.boostLimit * 2UL) + 1;

    TEST_ASSERT_FALSE(checkBoostLimit(current, page6));
    TEST_ASSERT_FALSE(current.engineProtectBoostCut);
}

static void test_checkBoostLimit_activate_when_conditions_met(void) {
    statuses current = {};
    config6 page6 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page6.boostCutEnabled = 1;
    page6.boostLimit = 30;
    current.MAP = (long)(page6.boostLimit * 2UL) + 1;

    TEST_ASSERT_TRUE(checkBoostLimit(current, page6));
    TEST_ASSERT_TRUE(current.engineProtectBoostCut);
}

static void test_checkBoostLimit_no_activate_when_map_low(void) {
    statuses current = {};
    config6 page6 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page6.boostCutEnabled = 1;
    page6.boostLimit = 60;
    current.MAP = (long)(page6.boostLimit * 2UL) - 1;

    TEST_ASSERT_FALSE(checkBoostLimit(current, page6));
    TEST_ASSERT_FALSE(current.engineProtectBoostCut);
}

void runAllTests(void)
{
    SET_UNITY_FILENAME() {

    RUN_TEST_P(test_checkOilPressureLimit_disabled);
    RUN_TEST_P(test_checkOilPressureLimit_activate_immediate_when_time_zero);
    RUN_TEST_P(test_checkOilPressureLimit_no_activation_when_above_limit);
    RUN_TEST_P(test_checkOilPressureLimit_activate_when_time_expires);
    RUN_TEST_P(test_checkBoostLimit_disabled_by_engineProtectType);
    RUN_TEST_P(test_checkBoostLimit_disabled_by_flag);
    RUN_TEST_P(test_checkBoostLimit_activate_when_conditions_met);
    RUN_TEST_P(test_checkBoostLimit_no_activate_when_map_low);
    }
}

DEVICE_TEST(runAllTests)
