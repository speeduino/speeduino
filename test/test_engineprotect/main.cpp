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
extern unsigned long afrProtectedActivateTime;

extern bool checkEngineProtect(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9, const config10 &page10, uint32_t currMillis);
extern uint8_t checkRevLimit(statuses &current, const config4 &page4, const config6 &page6, const config9 &page9);
extern table2D_u8_u8_6 coolantProtectTable;
extern uint8_t softLimitTime;
extern table2D_i8_u8_4 rollingCutTable;
extern uint32_t rollingCutLastRev;

using randFunc = uint8_t (*)(void);
extern randFunc rollingCutRandFunc;

struct rollingCutRandFunc_override_t
{
    rollingCutRandFunc_override_t(uint8_t (*fn)(void))
    : _oldFunc(rollingCutRandFunc)
    {
        rollingCutRandFunc = fn;
    }

    ~rollingCutRandFunc_override_t()
    {
        rollingCutRandFunc = _oldFunc;
    }

    randFunc _oldFunc;
};

static void resetInternalState(void)
{
    oilProtEndTime = 0;
    checkAFRLimitActive = false;
    afrProtectedActivateTime = 0;
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
        current.MAP = MAP.toUser(page6.boostLimit) + 1;
    }

    void setAfrActive(void)
    {
        resetInternalState();
        afrProtectedActivateTime = 1;
        current.MAP = 200; // kPa-like units; ensure above any small min*2
        setRpm(current, 5000);
        current.TPS = 50;
        current.O2 = 20;
        current.afrTarget = 10;
        page6.engineProtectType = PROTECT_CUT_BOTH;
        page9.afrProtectEnabled = AFR_PROTECT_FIXED;
        page6.egoType = EGO_TYPE_WIDE;
        page9.afrProtectCutTime = 0; // immediate
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
        setRpm(current, RPM_COARSE.toUser(page4.HardRevLim) - (rollingCutTable.axis[0]*10)); // > (1000 + axis[0]*10) (-20*10 = -200 -> threshold 800)
        current.schedulerCutState = { 0x00, 0xFF, 0xFF };
    }
};

static void test_checkOilPressureLimit_disabled_conditions(void) {
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

    context.setOilPressureActive(); 
    context.current.oilPressure = 60; // above table min
    setRpm(context.current, 0U);
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));
}

static void test_checkOilPressureLimit_activate_when_time_expires(void) {
    engineProtection_test_context_t context;
    context.setOilPressureActive(); 

    oilProtEndTime = 0;
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));

    oilProtEndTime = 10000;
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, oilProtEndTime-1));
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, oilProtEndTime));
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, oilProtEndTime+1));
}

static void test_checkOilPressureLimit_timer_and_activation(void)
{
    engineProtection_test_context_t context;
    context.setOilPressureActive();

    // Set a non-zero delay (2 -> 200ms)
    context.page10.oilPressureProtTime = 2;
    oilProtEndTime = 0;

    unsigned long now = 12345UL;
    // First call should arm the timer but not yet activate
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, now));
    TEST_ASSERT_EQUAL_UINT32(now + ((uint16_t)context.page10.oilPressureProtTime * 100U), oilProtEndTime);

    // Before expiry -> still false
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, oilProtEndTime - 1));
    // At expiry -> true
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, oilProtEndTime));
}

static void test_checkOilPressureLimit_existing_engineProtect_forces_cut(void)
{
    engineProtection_test_context_t context;
    context.setOilPressureActive();

    context.page10.oilPressureProtTime = 10;
    unsigned long now = 20000UL;
    oilProtEndTime = now + 5000UL; // future

    // Even though timer hasn't expired, existing engineProtect.oil should force a cut
    context.current.engineProtectOil = true;
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, now));
}

static void test_checkOilPressureLimit_timer_resets_when_pressure_recovers(void)
{
    engineProtection_test_context_t context;
    context.setOilPressureActive();

    context.page10.oilPressureProtTime = 5;
    unsigned long now = 30000UL;
    oilProtEndTime = 0;

    // Arm the timer
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, now));
    TEST_ASSERT_NOT_EQUAL(0U, oilProtEndTime);

    // Simulate pressure recovery above the limit
    context.current.oilPressure = table2D_getValue(&oilPressureProtectTable, context.current.RPMdiv100) + 10;
    TEST_ASSERT_FALSE(checkOilPressureLimit(context.current, context.page6, context.page10, now + 1));
    TEST_ASSERT_EQUAL_UINT32(0U, oilProtEndTime);
}

static void test_checkBoostLimit_disabled_conditions(void) {
    engineProtection_test_context_t context;

    context.setBoostActive();
    context.page6.engineProtectType = PROTECT_CUT_OFF;
    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));

    context.setBoostActive();
    context.page6.boostCutEnabled = 0;
    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));

    context.setBoostActive();
    context.current.MAP = (long)(context.page6.boostLimit * 2UL) - 1;
    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));

    context.setBoostActive();
    context.current.MAP = (long)(context.page6.boostLimit * 2UL); // exactly equal
    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));
}

static void test_checkBoostLimit_activate_when_conditions_met(void) {
    engineProtection_test_context_t context;

    context.setBoostActive();
    TEST_ASSERT_TRUE(checkBoostLimit(context.current, context.page6));

    context.page6.engineProtectType = PROTECT_CUT_BOTH;
    TEST_ASSERT_TRUE(checkBoostLimit(context.current, context.page6));

    context.page6.engineProtectType = PROTECT_CUT_FUEL;
    TEST_ASSERT_TRUE(checkBoostLimit(context.current, context.page6));
}

static void test_checkAFRLimit_disabled_conditions(void) {
    engineProtection_test_context_t context;

    constexpr uint32_t NOW = 1234U;

    // Disabled via engineProtectType
    context.setAfrActive();
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    context.page6.engineProtectType = PROTECT_CUT_OFF;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, NOW));

    // Disabled via afrProtectEnabled flag
    context.setAfrActive();
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    context.page9.afrProtectEnabled = AFR_PROTECT_OFF;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, NOW));

    // Disabled via egoType not wide
    context.setAfrActive();
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    context.page6.egoType = 0; // not EGO_TYPE_WIDE
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
}

static void test_checkAFRLimit_immediate_cut_time_zero(void) {
    engineProtection_test_context_t context;
    context.setAfrActive();

    context.page9.afrProtectDeviation = 15; // current.O2=20 >= 15
    context.page9.afrProtectCutTime = 0; // immediate

    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, 1234));
    TEST_ASSERT_TRUE(checkAFRLimitActive);
}

static void test_checkAFRLimit_table_mode_boundary(void) {
    engineProtection_test_context_t context;
    context.setAfrActive();

    context.page9.afrProtectEnabled = AFR_PROTECT_TABLE;
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

    context.page9.afrProtectReactivationTPS = 20;

    constexpr uint32_t NOW = 1000;
    afrProtectedActivateTime = NOW;

    // At delay expiry -> becomes active
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    TEST_ASSERT_TRUE(checkAFRLimitActive);

    // Now simulate TPS drop below reactivation threshold to clear protection
    context.current.TPS = context.page9.afrProtectReactivationTPS;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    TEST_ASSERT_FALSE(checkAFRLimitActive);
}

static void test_checkAFRLimit_counter_reset_on_condition_change(void)
{
    engineProtection_test_context_t context;
    context.setAfrActive();

    context.page9.afrProtectMinMAP = 1;
    context.page9.afrProtectCutTime = 5;
    afrProtectedActivateTime = 0U;

    constexpr uint32_t now = 1000UL;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, now));
    TEST_ASSERT_EQUAL_UINT32(now + (context.page9.afrProtectCutTime*100), afrProtectedActivateTime);

    // Drop MAP below minimum -> counter should reset
    context.current.MAP = 0;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, now + 1));
    TEST_ASSERT_EQUAL_UINT32(0U, afrProtectedActivateTime);
}

static void test_checkAFRLimit_delay_boundary_robustness(void)
{
    engineProtection_test_context_t context;
    context.setAfrActive();
    context.page9.afrProtectCutTime = 2; // 200ms
    constexpr uint32_t now = 5000UL;
    afrProtectedActivateTime = now;

    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, now - 1));
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, now));
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, now + 1));
}

static void test_checkAFRLimit_zero_deviation_fixed_mode(void)
{
    engineProtection_test_context_t context;
    context.setAfrActive();

    context.page9.afrProtectEnabled = AFR_PROTECT_FIXED;

    // With deviation 0 and cut time 0, O2 == 0 should trigger immediately
    context.current.O2 = 0;
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, 1234));
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
    TEST_ASSERT_FALSE(context.current.engineProtectClt);
    TEST_ASSERT_FALSE(context.current.engineProtectRpm);

    setRpm(context.current, RPM_COARSE.toUser(context.page4.HardRevLim));
    TEST_ASSERT_EQUAL_UINT8(50, checkRevLimit(context.current, context.page4, context.page6, context.page9));
    TEST_ASSERT_FALSE(context.current.engineProtectClt);
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
    TEST_ASSERT_BITS_HIGH(0x01, onOff.fuelChannels);
    TEST_ASSERT_BITS_LOW(0x01, onOff.ignitionChannels);
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
    TEST_ASSERT_BITS_LOW(0x01, onOff.fuelChannels);
    TEST_ASSERT_BITS_LOW(0x01, onOff.ignitionChannels);
}

static void test_calculateFuelIgnitionChannelCut_rolling_cut_multi_channel_fullcut(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    setRpm(context.current, context.current.RPM * 2U); // ensure rpmDelta >= 0 for full cut
    context.current.maxIgnOutputs = 4;
    context.current.maxInjOutputs = 4;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    // lower 4 bits should be cleared -> 0xF0
    TEST_ASSERT_BITS_LOW(0x0F, onOff.fuelChannels);
    TEST_ASSERT_BITS_LOW(0x0F, onOff.ignitionChannels);
}

static void test_calculateFuelIgnitionChannelCut_fullcut_updates_rollingCutLastRev(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    setRpm(context.current, context.current.RPM * 2U); // ensure rpmDelta >= 0 for full cut
    context.current.maxIgnOutputs = 1;
    context.current.maxInjOutputs = 1;

    rollingCutLastRev = 0;
    auto revBefore = context.current.startRevolutions;
    (void)calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL_UINT32(revBefore, rollingCutLastRev);
}

static void test_calculateFuelIgnitionChannelCut_pending_ignition_clears_deterministic(void)
{
    engineProtection_test_context_t context;
    context.setHardCutRolling();
    context.setRpmActive(HARD_REV_FIXED);

    context.page2.strokes = FOUR_STROKE;
    setRpm(context.current, RPM_COARSE.toUser(context.page4.HardRevLim-1U)); // between (maxAllowed + axis[0]*10) and maxAllowed
    context.current.maxIgnOutputs = 2;
    context.current.maxInjOutputs = 2;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    // Prepare a schedulerCutState where ignition channels are pending
    context.current.schedulerCutState.fuelChannels = 0x03; // channels 0-1 on
    context.current.schedulerCutState.ignitionChannels = 0x00;
    context.current.schedulerCutState.ignitionChannelsPending = 0x03;

    // Ensure rollingCutLastRev is 3 revolutions earlier so inner cut doesn't run but pending clear will
    rollingCutLastRev = context.current.startRevolutions - 3;

    auto cut = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);

    TEST_ASSERT_EQUAL_HEX8(cut.fuelChannels, cut.ignitionChannels);
    TEST_ASSERT_EQUAL_UINT8(0, cut.ignitionChannelsPending);
}

static void test_calculateFuelIgnitionChannelCut_no_rolling_cut_does_not_update_lastRev(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.page4.HardRevLim = 200; // high limit so not triggered
    setRpm(context.current, 500); // below threshold
    context.current.maxIgnOutputs = 1;
    context.current.maxInjOutputs = 1;
    
    rollingCutLastRev = 0;
    calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    TEST_ASSERT_EQUAL_UINT32(0U, rollingCutLastRev);
}

// Deterministic RNG stubs for tests
static uint8_t deterministic_rand_low(void)  { return 1U; }   // always triggers (< cutPercent)
static uint8_t deterministic_rand_high(void) { return 255U; } // never triggers (>= cutPercent)

// Force all channels to be cut via deterministic low RNG
static void test_calculateFuelIgnitionChannelCut_rolling_cut_forced_all_channels_cut(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.current.RPM *= 2; // ensure full-cut condition if rpmDelta >= 0
    context.current.maxIgnOutputs = 4;
    context.current.maxInjOutputs = 4;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    // Inject deterministic RNG that always triggers cuts
    rollingCutRandFunc_override_t rngOverride(deterministic_rand_low);

    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);
    // lower 4 bits should be cleared -> 0xF0
    TEST_ASSERT_BITS_LOW(0x0F, onOff.fuelChannels);
    TEST_ASSERT_BITS_LOW(0x0F, onOff.ignitionChannels);
}

// Force no channels to be cut via deterministic high RNG
static void test_calculateFuelIgnitionChannelCut_rolling_cut_forced_no_channel_cut(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.current.maxIgnOutputs = 2;
    context.current.maxInjOutputs = 2;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    // Ensure rpm sits in table-driven partial-cut zone (not full 100%)
    context.current.RPM = (context.page4.HardRevLim * 100U) + (rollingCutTable.axis[1] * 10); // use mid axis

    // Inject deterministic RNG that never triggers cuts
    rollingCutRandFunc_override_t rngOverride(deterministic_rand_high);

    auto onOff = calculateFuelIgnitionChannelCut(context.current, context.page2, context.page4, context.page6, context.page9, context.page10);

    // No channels should be cut
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.ignitionChannels);
}

void runAllTests(void)
{
    SET_UNITY_FILENAME() {

    RUN_TEST_P(test_checkOilPressureLimit_disabled_conditions);
    RUN_TEST_P(test_checkOilPressureLimit_activate_when_time_expires);
    RUN_TEST_P(test_checkOilPressureLimit_timer_and_activation)
    RUN_TEST_P(test_checkOilPressureLimit_existing_engineProtect_forces_cut);
    RUN_TEST_P(test_checkOilPressureLimit_timer_resets_when_pressure_recovers);
    RUN_TEST_P(test_checkBoostLimit_disabled_conditions);
    RUN_TEST_P(test_checkBoostLimit_activate_when_conditions_met);
    RUN_TEST_P(test_checkAFRLimit_disabled_conditions);
    RUN_TEST_P(test_checkAFRLimit_activate_after_delay_and_reactivate_on_tps);
    RUN_TEST_P(test_checkAFRLimit_immediate_cut_time_zero);
    RUN_TEST_P(test_checkAFRLimit_table_mode_boundary);
    RUN_TEST_P(test_checkAFRLimit_counter_reset_on_condition_change);
    RUN_TEST_P(test_checkAFRLimit_delay_boundary_robustness);
    RUN_TEST_P(test_checkAFRLimit_zero_deviation_fixed_mode);
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
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_pending_ignition_clears_deterministic);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_no_rolling_cut_does_not_update_lastRev);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_nosync);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_staging_complete_all_on);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_hardcut_full_ignition_only);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_hardcut_full_both);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_forced_all_channels_cut);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_forced_no_channel_cut);
    }
}

TEST_HARNESS(runAllTests)
