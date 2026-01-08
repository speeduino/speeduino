#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "engineProtection.h"
#include "../test_utils.h"
#include "decoders.h"
#include "units.h"

static void setSyncStatus(SyncStatus syncStatus)
{
    extern decoder_status_t decoderStatus;
    decoderStatus.syncStatus = syncStatus;
}

extern bool checkOilPressureLimit(const statuses &current, const config6 &page6, const config10 &page10, uint32_t currMillis);
extern table2D_u8_u8_4 oilPressureProtectTable;
extern uint32_t oilProtEndTime;

extern bool checkBoostLimit(const statuses &current, const config6 &page6);

extern bool checkAFRLimit(const statuses &current, const config6 &page6, const config9 &page9, uint32_t currMillis);
extern bool checkAFRLimitActive;
extern bool afrProtectCountEnabled;
extern unsigned long afrProtectCount;

extern bool checkEngineProtect(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10, uint32_t currMillis);
extern uint8_t checkRevLimit(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9);
extern table2D_u8_u8_6 coolantProtectTable;
extern uint8_t softLimitTime;
extern table2D_i8_u8_4 rollingCutTable;
extern uint32_t rollingCutLastRev;

static void resetInternalState(void)
{
    oilProtEndTime = 0;
    checkAFRLimitActive = false;
    afrProtectCountEnabled = false;
    afrProtectCount = 0;
    softLimitTime = 0;
    rollingCutLastRev = 0;
}

static void setup_oil_protect_table(void) {
    // Simple axis: 0..3 mapped to same min value
    TEST_DATA_P uint8_t bins[] = { 0, 100, 200, 255 };
    TEST_DATA_P uint8_t values[] = { 50, 50, 50, 50 };
    populate_2dtable_P(&oilPressureProtectTable, values, bins);
}

static void populateCoolantProtectTable(void)
{
    // Populate coolant protection table with a constant limit (e.g., 40)
    TEST_DATA_P uint8_t bins[] = { 0, 50, 100, 150, 200, 255 };
    TEST_DATA_P uint8_t values[] = { 40, 40, 40, 40, 40, 40 };
    populate_2dtable_P(&coolantProtectTable, values, bins);
}

static void populateRollingCutTable(void)
{
    TEST_DATA_P int8_t bins[] = { -20, -10, 0, 10 };
    TEST_DATA_P uint8_t values[] = { 100, 50, 25, 0 };
    populate_2dtable_P(&rollingCutTable, values, bins);
}

struct engineProtection_test_context_t
{
    statuses current = {};
    config2 page2 = {};
    config4 page4 = {};
    config6 page6 = {};
    config9 page9 = {};
    config10 page10 = {};

    void setOilPressureActive(void)
    {
        resetInternalState();
        setup_oil_protect_table();
        current.oilPressure = oilPressureProtectTable.values[0]-10; // below table min
        setRpm(current, 0U);
        page6.engineProtectType = PROTECT_CUT_IGN;
        page10.oilPressureProtEnbl = 1;
        page10.oilPressureEnable = 1;
        page10.oilPressureProtTime = 0; // immediate        
    }

    void setBoostActive(void)
    {
        page6.engineProtectType = PROTECT_CUT_IGN;
        page6.boostCutEnabled = 1;
        page6.boostLimit = 30;
        current.MAP = (long)(page6.boostLimit * 2UL) + 1;
    }

    void setAfrActive(void)
    {
        resetInternalState();
        current.MAP = 200; // kPa-like units; ensure above any small min*2
        setRpm(current, 5000);
        current.TPS = 50;
        current.O2 = 20;
        current.afrTarget = 10;
        page6.engineProtectType = PROTECT_CUT_BOTH;
        page9.afrProtectEnabled = AFR_PROTECT_FIXED;
        page6.egoType = EGO_TYPE_WIDE;
    }

    void setRpmActive(uint8_t type)
    {
        resetInternalState();
        populateCoolantProtectTable();

        page6.engineProtectType = PROTECT_CUT_IGN;
        page9.hardRevMode = type;
    }

    void setBeyondStaging(void)
    {
        resetInternalState();
        setSyncStatus(SyncStatus::Full);
        current.startRevolutions = 10;
        page4.StgCycles = current.startRevolutions-1; // staging complete
    }

    void setHardCutFull(void)
    {
        setBeyondStaging();
        page2.hardCutType = HARD_CUT_FULL;
        current.schedulerCutState = { 0x00, 0xFF, 0xFF };
    }

    void setHardCutRolling(void)
    {
        // Prepare rolling cut table and limits so rolling cut branch triggers
        setBeyondStaging();
        populateRollingCutTable();
        page2.hardCutType = HARD_CUT_ROLLING;
        page4.HardRevLim = 10; // div100 -> 1000 RPM
        current.RPM = (page4.HardRevLim*100U) - (rollingCutTable.axis[0]*10); // > (1000 + axis[0]*10) (-20*10 = -200 -> threshold 800)
        current.schedulerCutState = { 0x00, 0xFF, 0xFF };
    }
};

static void test_checkOilPressureLimit_disabled(void) {
    engineProtection_test_context_t context;
    
    context.setOilPressureActive(); 
    context.page6.engineProtectType = PROTECT_CUT_OFF;
    context.page10.oilPressureProtEnbl = 1;
    context.page10.oilPressureEnable = 1;
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));

    context.setOilPressureActive(); 
    context.page6.engineProtectType = PROTECT_CUT_BOTH;
    context.page10.oilPressureProtEnbl = 0;
    context.page10.oilPressureEnable = 0;
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));

    context.setOilPressureActive(); 
    context.page6.engineProtectType = PROTECT_CUT_BOTH;
    context.page10.oilPressureProtEnbl = 1;
    context.page10.oilPressureEnable = 0;
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));

    context.setOilPressureActive(); 
    context.page6.engineProtectType = PROTECT_CUT_BOTH;
    context.page10.oilPressureProtEnbl = 0;
    context.page10.oilPressureEnable = 1;
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));
}

static void test_checkOilPressureLimit_activate_immediate_when_time_zero(void) {
    engineProtection_test_context_t context;
    context.setOilPressureActive(); 

    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));
}

static void test_checkOilPressureLimit_activate_when_time_expires(void) {
    engineProtection_test_context_t context;
    context.setOilPressureActive(); 

    oilProtEndTime = 10000;

    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, oilProtEndTime-1));
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, oilProtEndTime));
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, oilProtEndTime+1));
}

static void test_checkOilPressureLimit_no_activation_when_above_limit(void) {
    engineProtection_test_context_t context;
    context.setOilPressureActive(); 

    context.current.oilPressure = 60; // above table min
    setRpm(context.current, 0U);

    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));
}

struct checkBoostLimit_context_t
{
    statuses current = {};
    config6 page6 = {};

    void setActive(void)
    {
        page6.engineProtectType = PROTECT_CUT_IGN;
        page6.boostCutEnabled = 1;
        page6.boostLimit = 30;
        current.MAP = (long)(page6.boostLimit * 2UL) + 1;
    }

    bool operator()(void)
    {
        return checkBoostLimit(current, page6);
    }
};

static void test_checkBoostLimit_disabled_by_engineProtectType(void) {
    engineProtection_test_context_t context;

    context.setBoostActive();
    context.page6.engineProtectType = PROTECT_CUT_OFF;

    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));
}

static void test_checkBoostLimit_disabled_by_flag(void) {
    engineProtection_test_context_t context;

    context.setBoostActive();
    context.page6.boostCutEnabled = 0;

    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));
}

static void test_checkBoostLimit_activate_when_conditions_met(void) {
    engineProtection_test_context_t context;

    context.setBoostActive();

    TEST_ASSERT_TRUE(checkBoostLimit(context.current, context.page6));
}

static void test_checkBoostLimit_no_activate_when_map_low(void) {
    engineProtection_test_context_t context;

    context.setBoostActive();
    context.current.MAP = (long)(context.page6.boostLimit * 2UL) - 1;

    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));
}

static void test_checkBoostLimit_equal_to_threshold_no_trigger(void) {
    engineProtection_test_context_t context;

    context.setBoostActive();
    context.current.MAP = (long)(context.page6.boostLimit * 2UL); // exactly equal

    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));
}

static void test_checkAFRLimit_disabled_conditions(void) {
    engineProtection_test_context_t context;

    // Disabled via engineProtectType
    context.setAfrActive();
    context.page6.engineProtectType = PROTECT_CUT_OFF;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, 0));

    // Disabled via afrProtectEnabled flag
    context.setAfrActive();
    context.page9.afrProtectEnabled = AFR_PROTECT_OFF;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, 0));

    // Disabled via egoType not wide
    context.setAfrActive();
    context.page6.egoType = 0; // not EGO_TYPE_WIDE
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, 0));
}

static void test_checkAFRLimit_immediate_cut_time_zero(void) {
    engineProtection_test_context_t context;
    context.setAfrActive();

    context.page9.afrProtectMinMAP = 1;
    context.page9.afrProtectMinRPM = 1;
    context.page9.afrProtectMinTPS = 1;
    context.page9.afrProtectDeviation = 15; // current.O2=20 >= 15
    context.page9.afrProtectCutTime = 0; // immediate

    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, 1234));
    TEST_ASSERT_TRUE(checkAFRLimitActive);
}

static void test_checkAFRLimit_table_mode_boundary(void) {
    engineProtection_test_context_t context;
    context.setAfrActive();

    context.page9.afrProtectMinMAP = 50;
    context.page9.afrProtectMinRPM = 10;
    context.page9.afrProtectMinTPS = 10;
    context.page9.afrProtectDeviation = 5; // will be added to afrTarget (10) -> limit 15
    context.page9.afrProtectCutTime = 0;

    // current.O2 exactly at target+deviation (10+5=15) should trigger
    context.current.O2 = context.current.afrTarget + context.page9.afrProtectDeviation;

    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, 1234));
    TEST_ASSERT_TRUE(checkAFRLimitActive);
}

static void test_checkAFRLimit_activate_after_delay_and_reactivate_on_tps(void) {
    engineProtection_test_context_t context;
    context.setAfrActive();

    context.page9.afrProtectMinMAP = 50; // minMAP*2 = 100
    context.page9.afrProtectMinRPM = 10;
    context.page9.afrProtectMinTPS = 10;
    context.page9.afrProtectDeviation = 15; // current.O2 (20) >= 15 -> triggers
    context.page9.afrProtectCutTime = 1; // 1 * 100ms = 100ms delay
    context.page9.afrProtectReactivationTPS = 20;

    // First call should start the counter but not activate yet
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, 1000));
    TEST_ASSERT_TRUE(afrProtectCountEnabled);
    unsigned long start = afrProtectCount;

    // Before delay expires -> still not active
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, start + (context.page9.afrProtectCutTime * 100) - 1));

    // At delay expiry -> becomes active
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, start + (context.page9.afrProtectCutTime * 100)));
    TEST_ASSERT_TRUE(checkAFRLimitActive);

    // Now simulate TPS drop below reactivation threshold to clear protection
    context.current.TPS = context.page9.afrProtectReactivationTPS;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, start + (context.page9.afrProtectCutTime * 100) + 1));
    TEST_ASSERT_FALSE(checkAFRLimitActive);
}

static void test_checkEngineProtect_no_protections(void) {
    // Ensure RPM above threshold but no protection conditions set
    resetInternalState();
    engineProtection_test_context_t context;
    context.page4.engineProtectMaxRPM = 5;
    setRpm(context.current, 1000);

    TEST_ASSERT_FALSE(checkEngineProtect(context.current, context.page4, context.page6, context.page9, context.page10, 0));
    TEST_ASSERT_FALSE(context.current.engineProtectBoostCut);
    TEST_ASSERT_FALSE(context.current.engineProtectOil);
    TEST_ASSERT_FALSE(context.current.engineProtectAfr);
}

static void test_checkEngineProtect_protection_but_rpm_low(void) {
    // Set up oil protection to be active but RPM not above max -> no protectActive
    engineProtection_test_context_t context;
    context.setOilPressureActive();

    context.page4.engineProtectMaxRPM = 5;
    setRpm(context.current, RPM_COARSE.toUser(context.page4.engineProtectMaxRPM)); // not greater than max

    TEST_ASSERT_FALSE(checkEngineProtect(context.current, context.page4, context.page6, context.page9, context.page10, millis()));
    TEST_ASSERT_FALSE(context.current.engineProtectBoostCut);
    TEST_ASSERT_TRUE(context.current.engineProtectOil);
    TEST_ASSERT_FALSE(context.current.engineProtectAfr);
}

static void test_checkEngineProtect_protection_and_rpm_high(void) {
    engineProtection_test_context_t context;
    context.setOilPressureActive();

    context.page4.engineProtectMaxRPM = 5;
    setRpm(context.current, RPM_COARSE.toUser(context.page4.engineProtectMaxRPM+1U)); // greater than max

    TEST_ASSERT_TRUE(checkEngineProtect(context.current, context.page4, context.page6, context.page9, context.page10, millis()));
    TEST_ASSERT_FALSE(context.current.engineProtectBoostCut);
    TEST_ASSERT_TRUE(context.current.engineProtectOil);
    TEST_ASSERT_FALSE(context.current.engineProtectAfr);
}

static void test_checkRevLimit_disabled(void) {
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);

    context.page6.engineProtectType = PROTECT_CUT_OFF;
    TEST_ASSERT_EQUAL_UINT8(UINT8_MAX, checkRevLimit(context.current, context.page4, context.page6, context.page9));
    TEST_ASSERT_FALSE(context.current.engineProtectRpm);
    TEST_ASSERT_FALSE(context.current.engineProtectClt);
}

static void test_checkRevLimit_fixed_mode_no_trigger_and_trigger(void) {
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.page4.HardRevLim = 50;

    setRpm(context.current, RPM_COARSE.toUser(context.page4.HardRevLim-1U));
    TEST_ASSERT_EQUAL_UINT8(50, checkRevLimit(context.current, context.page4, context.page6, context.page9));
    TEST_ASSERT_FALSE(context.current.engineProtectRpm);

    setRpm(context.current, RPM_COARSE.toUser(context.page4.HardRevLim));
    TEST_ASSERT_EQUAL_UINT8(50, checkRevLimit(context.current, context.page4, context.page6, context.page9));
    TEST_ASSERT_TRUE(context.current.engineProtectRpm);
}

static void test_checkRevLimit_softlimit_triggers(void) {
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);

    context.page4.HardRevLim = 100;
    context.page4.SoftRevLim = 60;
    context.page4.SoftLimMax = 5;

    // Simulate soft limiter running longer than allowed
    softLimitTime = context.page4.SoftLimMax + 1;
    setRpm(context.current, RPM_COARSE.toUser(context.page4.SoftRevLim));

    TEST_ASSERT_EQUAL_UINT8(context.page4.HardRevLim, checkRevLimit(context.current, context.page4, context.page6, context.page9));
    TEST_ASSERT_TRUE(context.current.engineProtectRpm);
}

static void test_checkRevLimit_coolant_mode_triggers_clt(void) {
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_COOLANT);

    context.current.coolant = 0;
    setRpm(context.current, RPM_COARSE.toUser(coolantProtectTable.values[0]+1U)); // greater than 40 -> should trigger

    TEST_ASSERT_EQUAL_UINT8(40, checkRevLimit(context.current, context.page4, context.page6, context.page9));
    TEST_ASSERT_TRUE(context.current.engineProtectClt);
    TEST_ASSERT_TRUE(context.current.engineProtectRpm);
}

static void test_checkRevLimit_coolant_equal_no_trigger(void) {
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_COOLANT);

    context.current.coolant = 0;
    setRpm(context.current, RPM_COARSE.toUser(coolantProtectTable.values[0])); // equal to limit -> should NOT trigger (uses >)

    TEST_ASSERT_EQUAL_UINT8(40, checkRevLimit(context.current, context.page4, context.page6, context.page9));
    TEST_ASSERT_FALSE(context.current.engineProtectClt);
    TEST_ASSERT_FALSE(context.current.engineProtectRpm);
}

static void test_calculateFuelIgnitionChannelCut_nosync(void)
{
    engineProtection_test_context_t context;
    resetInternalState();
    setSyncStatus(SyncStatus::None);

    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL(0, onOff.fuelChannels);
    TEST_ASSERT_EQUAL(0, onOff.ignitionChannels);
}

static void test_calculateFuelIgnitionChannelCut_staging_complete_all_on(void)
{
    engineProtection_test_context_t context;
    context.setBeyondStaging();

    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.ignitionChannels);
}

static void test_calculateFuelIgnitionChannelCut_hardcut_full_ignition_only(void)
{
    engineProtection_test_context_t context;
    context.setHardCutFull();
    context.setRpmActive(HARD_REV_FIXED);
    
    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.fuelChannels); // fuel remains on
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.ignitionChannels); // ignition cut
    TEST_ASSERT_TRUE(context.current.hardLimitActive);
}

static void test_calculateFuelIgnitionChannelCut_hardcut_full_both(void)
{
    engineProtection_test_context_t context;
    context.setHardCutFull();
    context.setRpmActive(HARD_REV_FIXED);

    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.ignitionChannels);
    TEST_ASSERT_TRUE(context.current.hardLimitActive);
}

static void test_calculateFuelIgnitionChannelCut_rolling_cut_ignition_only(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.current.maxIgnOutputs = 1;
    context.current.maxInjOutputs = 1;

    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0xFE, onOff.ignitionChannels);
}

static void test_calculateFuelIgnitionChannelCut_rolling_cut_both(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.current.maxIgnOutputs = 1;
    context.current.maxInjOutputs = 1;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL_HEX8(0xFE, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0xFE, onOff.ignitionChannels);
}

static void test_calculateFuelIgnitionChannelCut_rolling_cut_multi_channel_fullcut(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.current.RPM *= 2; // ensure rpmDelta >= 0 for full cut
    context.current.maxIgnOutputs = 4;
    context.current.maxInjOutputs = 4;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    // lower 4 bits should be cleared -> 0xF0
    TEST_ASSERT_EQUAL_HEX8(0xF0, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0xF0, onOff.ignitionChannels);
}

static void test_calculateFuelIgnitionChannelCut_fullcut_updates_rollingCutLastRev(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.current.RPM *= 2; // ensure rpmDelta >= 0 for full cut
    context.current.maxIgnOutputs = 1;
    context.current.maxInjOutputs = 1;

    rollingCutLastRev = 0;
    auto revBefore = context.current.startRevolutions;
    (void)calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL_UINT32(revBefore, rollingCutLastRev);
}

static void test_calculateFuelIgnitionChannelCut_no_rolling_cut_does_not_update_lastRev(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.page4.HardRevLim = 200; // high limit so not triggered
    context.current.RPM = 500; // below threshold
    context.current.maxIgnOutputs = 1;
    context.current.maxInjOutputs = 1;
    
    rollingCutLastRev = 0;
    calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL_UINT32(0U, rollingCutLastRev);
}

#if 0
static void test_calculateFuelIgnitionChannelCut_pending_ignition_clears_after_two_revs(void)
{
    // This test was removed due to unreliable internal state interactions (random1to100)
    // TODO: refactor & re-work test so it's reliable.
}
#endif

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
    RUN_TEST_P(test_checkBoostLimit_equal_to_threshold_no_trigger);
    RUN_TEST_P(test_checkAFRLimit_disabled_conditions);
    RUN_TEST_P(test_checkAFRLimit_activate_after_delay_and_reactivate_on_tps);
    RUN_TEST_P(test_checkAFRLimit_immediate_cut_time_zero);
    RUN_TEST_P(test_checkAFRLimit_table_mode_boundary);
    RUN_TEST_P(test_checkEngineProtect_no_protections);
    RUN_TEST_P(test_checkEngineProtect_protection_but_rpm_low);
    RUN_TEST_P(test_checkEngineProtect_protection_and_rpm_high);
    RUN_TEST_P(test_checkRevLimit_disabled);
    RUN_TEST_P(test_checkRevLimit_fixed_mode_no_trigger_and_trigger);
    RUN_TEST_P(test_checkRevLimit_softlimit_triggers);
    RUN_TEST_P(test_checkRevLimit_coolant_mode_triggers_clt);
    RUN_TEST_P(test_checkRevLimit_coolant_equal_no_trigger);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_ignition_only);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_both);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_multi_channel_fullcut);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_fullcut_updates_rollingCutLastRev);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_no_rolling_cut_does_not_update_lastRev);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_nosync);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_staging_complete_all_on);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_hardcut_full_ignition_only);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_hardcut_full_both);
    }
}

TEST_HARNESS(runAllTests)
