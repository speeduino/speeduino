#include "../device_test_harness.h"
#include "statuses.h"
#include "config_pages.h"
#include "../test_utils.h"

extern bool checkOilPressureLimit(statuses &current, const config6 &page6, const config10 &page10, uint32_t currMillis);
extern table2D_u8_u8_4 oilPressureProtectTable;
extern uint8_t oilProtStartTime;

extern bool checkBoostLimit(const statuses &current, const config6 &page6);

extern bool checkAFRLimit(statuses &current, const config6 &page6, const config9 &page9, uint32_t currMillis);
extern bool checkAFRLimitActive;
extern bool afrProtectCountEnabled;
extern unsigned long afrProtectCount;

extern bool checkEngineProtect(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10, uint32_t currMillis);
extern uint8_t checkRevLimit(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9);
extern table2D_u8_u8_6 coolantProtectTable;
extern uint8_t softLimitTime;

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
    oilProtStartTime = 0;

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
    oilProtStartTime = 0;

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
}

static void test_checkBoostLimit_disabled_by_flag(void) {
    statuses current = {};
    config6 page6 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page6.boostCutEnabled = 0;
    page6.boostLimit = 40;
    current.MAP = (long)(page6.boostLimit * 2UL) + 1;

    TEST_ASSERT_FALSE(checkBoostLimit(current, page6));
}

static void test_checkBoostLimit_activate_when_conditions_met(void) {
    statuses current = {};
    config6 page6 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page6.boostCutEnabled = 1;
    page6.boostLimit = 30;
    current.MAP = (long)(page6.boostLimit * 2UL) + 1;

    TEST_ASSERT_TRUE(checkBoostLimit(current, page6));
}

static void test_checkBoostLimit_no_activate_when_map_low(void) {
    statuses current = {};
    config6 page6 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page6.boostCutEnabled = 1;
    page6.boostLimit = 60;
    current.MAP = (long)(page6.boostLimit * 2UL) - 1;

    TEST_ASSERT_FALSE(checkBoostLimit(current, page6));
}

static statuses setup_afr_protect_active(void) {
    statuses current = {};
    current.MAP = 200; // kPa-like units; ensure above any small min*2
    current.RPMdiv100 = 50;
    current.TPS = 50;
    current.O2 = 20;
    current.afrTarget = 10;
    return current;
}

static void test_checkAFRLimit_disabled_conditions(void) {
    statuses current = {};
    config6 page6 = {};
    config9 page9 = {};

    // Disabled via engineProtectType
    page6.engineProtectType = PROTECT_CUT_OFF;
    page9.afrProtectEnabled = AFR_PROTECT_FIXED;
    page6.egoType = EGO_TYPE_WIDE;
    TEST_ASSERT_FALSE(checkAFRLimit(current, page6, page9, 0));

    // Disabled via afrProtectEnabled flag
    page6.engineProtectType = PROTECT_CUT_IGN;
    page9.afrProtectEnabled = AFR_PROTECT_OFF;
    TEST_ASSERT_FALSE(checkAFRLimit(current, page6, page9, 0));

    // Disabled via egoType not wide
    page9.afrProtectEnabled = AFR_PROTECT_FIXED;
    page6.egoType = 0; // not EGO_TYPE_WIDE
    TEST_ASSERT_FALSE(checkAFRLimit(current, page6, page9, 0));
}

static void test_checkAFRLimit_activate_after_delay_and_reactivate_on_tps(void) {
    // Prepare configs and status to meet AFR protection conditions
    config6 page6 = {};
    config9 page9 = {};
    statuses current = setup_afr_protect_active();

    page6.engineProtectType = PROTECT_CUT_IGN;
    page6.egoType = EGO_TYPE_WIDE;
    page9.afrProtectEnabled = AFR_PROTECT_FIXED; // fixed value mode
    page9.afrProtectMinMAP = 50; // minMAP*2 = 100
    page9.afrProtectMinRPM = 10;
    page9.afrProtectMinTPS = 10;
    page9.afrProtectDeviation = 15; // current.O2 (20) >= 15 -> triggers
    page9.afrProtectCutTime = 1; // 1 * 100ms = 100ms delay
    page9.afrProtectReactivationTPS = 20;

    // Reset file-scoped state
    checkAFRLimitActive = false;
    afrProtectCountEnabled = false;
    afrProtectCount = 0;

    uint32_t t0 = 1000;

    // First call should start the counter but not activate yet
    TEST_ASSERT_FALSE(checkAFRLimit(current, page6, page9, t0));
    TEST_ASSERT_TRUE(afrProtectCountEnabled);
    unsigned long start = afrProtectCount;

    // Before delay expires -> still not active
    TEST_ASSERT_FALSE(checkAFRLimit(current, page6, page9, start + (page9.afrProtectCutTime * 100) - 1));

    // At delay expiry -> becomes active
    TEST_ASSERT_TRUE(checkAFRLimit(current, page6, page9, start + (page9.afrProtectCutTime * 100)));
    TEST_ASSERT_TRUE(checkAFRLimitActive);
    TEST_ASSERT_TRUE(current.engineProtectAfr);

    // Now simulate TPS drop below reactivation threshold to clear protection
    current.TPS = page9.afrProtectReactivationTPS;
    TEST_ASSERT_FALSE(checkAFRLimit(current, page6, page9, start + (page9.afrProtectCutTime * 100) + 1));
    TEST_ASSERT_FALSE(checkAFRLimitActive);
    TEST_ASSERT_FALSE(current.engineProtectAfr);
}

static void test_checkEngineProtect_no_protections(void) {
    statuses current = {};
    config4 page4 = {};
    config6 page6 = {};
    config9 page9 = {};
    config10 page10 = {};

    // Ensure RPM above threshold but no protection conditions set
    page4.engineProtectMaxRPM = 5;
    current.RPMdiv100 = 10;

    TEST_ASSERT_FALSE(checkEngineProtect(current, page4, page6, page9, page10, 0));
}

static void test_checkEngineProtect_protection_but_rpm_low(void) {
    // Set up oil protection to be active but RPM not above max -> no protectActive
    statuses current = setup_oil_protect_table_active();
    config4 page4 = {};
    config6 page6 = {};
    config9 page9 = {};
    config10 page10 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page10.oilPressureProtEnbl = 1;
    page10.oilPressureEnable = 1;
    page10.oilPressureProtTime = 0; // immediate

    page4.engineProtectMaxRPM = 5;
    current.RPMdiv100 = 5; // not greater than max

    TEST_ASSERT_FALSE(checkEngineProtect(current, page4, page6, page9, page10, millis()));
}

static void test_checkEngineProtect_protection_and_rpm_high(void) {
    // Oil protection active and RPM above max -> protectActive should be true
    statuses current = setup_oil_protect_table_active();
    config4 page4 = {};
    config6 page6 = {};
    config9 page9 = {};
    config10 page10 = {};

    page4.engineProtectMaxRPM = 5;
    current.RPMdiv100 = 6; // greater than max

    page6.engineProtectType = PROTECT_CUT_IGN;
    page10.oilPressureProtEnbl = 1;
    page10.oilPressureEnable = 1;
    page10.oilPressureProtTime = 0; // immediate
    oilProtStartTime = 0;

    TEST_ASSERT_TRUE(checkEngineProtect(current, page4, page6, page9, page10, millis()));
}

static void test_checkRevLimit_disabled(void) {
    statuses current = {};
    config4 page4 = {};
    config6 page6 = {};
    config9 page9 = {};

    page6.engineProtectType = PROTECT_CUT_OFF;
    uint8_t limit = checkRevLimit(current, page4, page6, page9);
    TEST_ASSERT_EQUAL_UINT8(UINT8_MAX, limit);
    TEST_ASSERT_FALSE(current.engineProtectRpm);
    TEST_ASSERT_FALSE(current.engineProtectClt);
}

static void test_checkRevLimit_fixed_mode_no_trigger_and_trigger(void) {
    statuses current = {};
    config4 page4 = {};
    config6 page6 = {};
    config9 page9 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page9.hardRevMode = HARD_REV_FIXED;
    page4.HardRevLim = 50;
    softLimitTime = 0;

    current.RPMdiv100 = 49;
    uint8_t limit = checkRevLimit(current, page4, page6, page9);
    TEST_ASSERT_EQUAL_UINT8(50, limit);
    TEST_ASSERT_FALSE(current.engineProtectRpm);

    current.RPMdiv100 = 50;
    limit = checkRevLimit(current, page4, page6, page9);
    TEST_ASSERT_EQUAL_UINT8(50, limit);
    TEST_ASSERT_TRUE(current.engineProtectRpm);
}

static void test_checkRevLimit_softlimit_triggers(void) {
    statuses current = {};
    config4 page4 = {};
    config6 page6 = {};
    config9 page9 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page9.hardRevMode = HARD_REV_FIXED;
    page4.HardRevLim = 100;
    page4.SoftRevLim = 60;
    page4.SoftLimMax = 5;

    // Simulate soft limiter running longer than allowed
    softLimitTime = page4.SoftLimMax + 1;
    current.RPMdiv100 = page4.SoftRevLim;

    uint8_t limit = checkRevLimit(current, page4, page6, page9);
    TEST_ASSERT_EQUAL_UINT8(page4.HardRevLim, limit);
    TEST_ASSERT_TRUE(current.engineProtectRpm);
}

static void test_checkRevLimit_coolant_mode_triggers_clt(void) {
    statuses current = {};
    config4 page4 = {};
    config6 page6 = {};
    config9 page9 = {};

    page6.engineProtectType = PROTECT_CUT_IGN;
    page9.hardRevMode = HARD_REV_COOLANT;

    // Populate coolant protection table with a constant limit (e.g., 40)
    TEST_DATA_P uint8_t bins[] = { 0, 50, 100, 150, 200, 255 };
    TEST_DATA_P uint8_t values[] = { 40, 40, 40, 40, 40, 40 };
    populate_2dtable_P(&coolantProtectTable, values, bins);

    current.coolant = 0;
    current.RPMdiv100 = 41; // greater than 40 -> should trigger

    uint8_t limit = checkRevLimit(current, page4, page6, page9);
    TEST_ASSERT_EQUAL_UINT8(40, limit);
    TEST_ASSERT_TRUE(current.engineProtectClt);
    TEST_ASSERT_TRUE(current.engineProtectRpm);
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
    RUN_TEST_P(test_checkAFRLimit_disabled_conditions);
    RUN_TEST_P(test_checkAFRLimit_activate_after_delay_and_reactivate_on_tps);
    RUN_TEST_P(test_checkEngineProtect_no_protections);
    RUN_TEST_P(test_checkEngineProtect_protection_but_rpm_low);
    RUN_TEST_P(test_checkEngineProtect_protection_and_rpm_high);
    RUN_TEST_P(test_checkRevLimit_disabled);
    RUN_TEST_P(test_checkRevLimit_fixed_mode_no_trigger_and_trigger);
    RUN_TEST_P(test_checkRevLimit_softlimit_triggers);
    RUN_TEST_P(test_checkRevLimit_coolant_mode_triggers_clt);
    }
}

DEVICE_TEST(runAllTests)
