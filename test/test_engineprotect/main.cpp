#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "engineProtection.h"
#include "../test_utils.h"
#include "decoders.h"
#include "units.h"

extern bool checkOilPressureLimit(const statuses &current, const config6 &page6, const config10 &page10, uint32_t currMillis);
extern table2D_u8_u8_4 oilPressureProtectTable;
extern uint32_t oilProtEndTime;

extern bool checkBoostLimit(const statuses &current, const config6 &page6);
extern bool checkRpmLimit(const statuses &current, const config4 &page4, const config6 &page6, const config9 &page9);
extern bool checkCoolantLimit(const statuses &current, const config6 &page6, const config9 &page9);

extern bool checkAFRLimit(const statuses &current, const config6 &page6, const config9 &page9, uint32_t currMillis);
extern bool checkAFRLimitActive;
extern unsigned long afrProtectedActivateTime;

extern table2D_u8_u8_6 coolantProtectTable;
extern uint8_t softLimitTime;
extern table2D_i8_u8_4 rollingCutTable;
extern uint32_t rollingCutLastRev;

extern bool supportPendingIgnitionCut(const config2 &page2, const config4 &page4, const config6 &page6);

extern statuses::scheduler_cut_t applyRollingCutPercentage(const statuses &current, const config6 &page6, uint8_t cutPercent, bool pendingIgnitionCut);

extern statuses::scheduler_cut_t applyPendingIgnitionCuts(statuses::scheduler_cut_t cutState, const statuses &current);

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

extern bool useRollingCut(const statuses &current, const config2 &page2, uint16_t maxAllowedRPM);
extern uint8_t calcRollingCutRevolutions(const config2 &page2, const config4 &page4);
extern uint8_t calcRollingCutPercentage(const statuses &current, uint16_t maxAllowedRPM);
extern statuses::scheduler_cut_t channelOn(statuses::scheduler_cut_t cutState, bool pendingIgnitionCut, uint8_t channel);
extern uint16_t getMaxRpm(const statuses &current, const config4 &page4, const config6 &page6, const config9 &page9);

extern uint8_t getHardRevLimit(const statuses &current, const config4 &page4, const config9 &page9);
extern uint8_t applyEngineProtectionRevLimit(uint8_t curLimit, const statuses &current, const config4 &page4);
extern uint8_t applyHardLaunchRevLimit(uint8_t curLimit, const statuses &current, const config6 &page6);
extern uint16_t applyFlatShiftRevLimit(uint16_t curLimit, const statuses &current);

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
    TEST_DATA_P int8_t bins[] = { -20, -15, -10, -1 };
    TEST_DATA_P uint8_t values[] = { 0, 25, 50, 100 };
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
    decoder_status_t decoderStatus = {};

    statuses::scheduler_cut_t calculateFuelIgnitionChannelCut(void)
    {
        current.engineProtect = ::checkEngineProtection(current, page4, page6, page9, page10);
        return ::calculateFuelIgnitionChannelCut(current, decoderStatus, page2, page4, page6, page9);
    }

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
        page4.HardRevLim = 80;
        page4.SoftRevLim = 60;
        page4.SoftLimMax = 5;
        setRpm(current, RPM_COARSE.toUser(page4.HardRevLim));
    }

    void setBeyondStaging(void)
    {
        resetInternalState();
        decoderStatus.syncStatus = SyncStatus::Full;
        current.startRevolutions = 10;
        page4.StgCycles = current.startRevolutions-1; // staging complete
    }

    void setHardCutFull(void)
    {
        setBeyondStaging();
        page2.hardCutType = HARD_CUT_FULL;
        current.schedulerCutState = { 0x00, 0xFF, 0xFF, SchedulerCutStatus::None };
    }

    void setHardCutRolling(void)
    {
        // Prepare rolling cut table and limits so rolling cut branch triggers
        setBeyondStaging();
        populateRollingCutTable();
        page2.hardCutType = HARD_CUT_ROLLING;
        page4.HardRevLim = 50; // div100 -> 1000 RPM
        setRpm(current, RPM_COARSE.toUser(page4.HardRevLim) - SIGNED_RPM_MEDIUM.toUser(rollingCutTable.axis[0]));
        current.schedulerCutState = { 0x00, 0xFF, 0xFF, SchedulerCutStatus::None };
    }

    void setCoolantActive(void)
    {
        resetInternalState();
        page9.hardRevMode = HARD_REV_COOLANT;
        page6.engineProtectType = PROTECT_CUT_IGN;

        // populate table with constant 40
        populateCoolantProtectTable();

        current.coolant = 0;
        setRpm(current, RPM_COARSE.toUser(coolantProtectTable.values[0]+1U)); // greater -> trigger
    }
};

static void test_checkOilPressureLimit_basic(void) {
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

    // PROTECT_CUT_IGN type should trigger
    context.setOilPressureActive();
    context.page6.engineProtectType = PROTECT_CUT_IGN;
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));

    // PROTECT_CUT_FUEL type should trigger
    context.setOilPressureActive();
    context.page6.engineProtectType = PROTECT_CUT_FUEL;
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));

    // PROTECT_CUT_BOTH type should trigger
    context.setOilPressureActive();
    context.page6.engineProtectType = PROTECT_CUT_BOTH;
    TEST_ASSERT_TRUE(checkOilPressureLimit(context.current, context.page6, context.page10, millis()));    
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
    TEST_ASSERT_EQUAL_UINT32(now + TIME_TEN_MILLIS.toUser(context.page10.oilPressureProtTime), oilProtEndTime);

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
    context.current.engineProtect.oil = true;
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
    context.current.MAP = MAP.toUser(context.page6.boostLimit) - 1;
    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));

    // Boost exactly at limit (not > limit) should NOT trigger
    context.setBoostActive();
    context.current.MAP = (long)MAP.toUser(context.page6.boostLimit);
    TEST_ASSERT_FALSE(checkBoostLimit(context.current, context.page6));

    // Boost just above limit should trigger
    context.setBoostActive();
    context.current.MAP = (long)MAP.toUser(context.page6.boostLimit) + 1;
    TEST_ASSERT_TRUE(checkBoostLimit(context.current, context.page6));
}

static void test_checkBoostLimit_activate_when_conditions_met(void) {
    engineProtection_test_context_t context;

    context.setBoostActive();
    TEST_ASSERT_TRUE(checkBoostLimit(context.current, context.page6));

    context.page6.engineProtectType = PROTECT_CUT_BOTH;
    TEST_ASSERT_TRUE(checkBoostLimit(context.current, context.page6));

    context.page6.engineProtectType = PROTECT_CUT_FUEL;
    TEST_ASSERT_TRUE(checkBoostLimit(context.current, context.page6));

    context.page6.engineProtectType = PROTECT_CUT_IGN;
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
    context.current.O2 = context.current.afrTarget + (uint16_t)context.page9.afrProtectDeviation;

    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, 1234));
    TEST_ASSERT_TRUE(checkAFRLimitActive);
}

static void test_checkAFRLimit_activate_after_delay_and_reactivate_on_tps(void) {
    engineProtection_test_context_t context;
    context.setAfrActive();

    context.page9.afrProtectReactivationTPS = 20;

    constexpr uint32_t NOW = 1000;
    afrProtectedActivateTime = NOW;

    // Cycle 1: Activate immediately (all conditions met, cutTime=0)
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    TEST_ASSERT_TRUE(checkAFRLimitActive);

    // Revert map condition -> still active!
    context.current.MAP = (context.page9.afrProtectMinMAP-1) * 100;
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    TEST_ASSERT_TRUE(checkAFRLimitActive);

    // Now simulate TPS drop below reactivation threshold to clear protection
    context.current.TPS = context.page9.afrProtectReactivationTPS;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    TEST_ASSERT_FALSE(checkAFRLimitActive);

    // Driver increases TPS again, condition becomes active -> should reactivate
    context.setAfrActive(); // Reset to active conditions
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    TEST_ASSERT_TRUE(checkAFRLimitActive);

    // Cycle 3: Clear again and reactivate
    context.current.TPS = context.page9.afrProtectReactivationTPS;
    TEST_ASSERT_FALSE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    TEST_ASSERT_FALSE(checkAFRLimitActive);

    context.setAfrActive();
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, NOW));
    TEST_ASSERT_TRUE(checkAFRLimitActive);

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

static void test_getHardRevLimit(void)
{
    engineProtection_test_context_t context;

    context.page9.hardRevMode = HARD_REV_FIXED;
    context.page4.HardRevLim = 65; // div100
    TEST_ASSERT_EQUAL_UINT8(65, getHardRevLimit(context.current, context.page4, context.page9));

    context.page9.hardRevMode = HARD_REV_COOLANT;
    populateCoolantProtectTable(); // constant 40
    context.current.coolant = 50; // should map to 40 in table
    TEST_ASSERT_EQUAL_UINT8(40, getHardRevLimit(context.current, context.page4, context.page9));

    context.page9.hardRevMode = 0; // or undefined
    context.page4.HardRevLim = 50; // should be ignored
    TEST_ASSERT_EQUAL_UINT8(UINT8_MAX, getHardRevLimit(context.current, context.page4, context.page9));

    context.page9.hardRevMode = HARD_REV_FIXED;
    context.page4.HardRevLim = 0;
    TEST_ASSERT_EQUAL_UINT8(0, getHardRevLimit(context.current, context.page4, context.page9));

    context.page9.hardRevMode = HARD_REV_FIXED;
    context.page4.HardRevLim = 255; // max uint8
    TEST_ASSERT_EQUAL_UINT8(255, getHardRevLimit(context.current, context.page4, context.page9));
}

static void test_applyEngineProtectionRevLimit(void)
{
    engineProtection_test_context_t context;

    // All protection flags false -> no clamping
    uint8_t curLimit = 80;
    {
        engineProtection_test_context_t context;
        TEST_ASSERT_EQUAL_UINT8(curLimit, applyEngineProtectionRevLimit(curLimit, context.current, context.page4));
    }

    {
        engineProtection_test_context_t context;
        context.current.engineProtect.oil = true;
        context.page4.engineProtectMaxRPM = curLimit + 10; // lower than current limit
        TEST_ASSERT_EQUAL_UINT8(curLimit, applyEngineProtectionRevLimit(curLimit, context.current, context.page4));
    }
    
    {
        engineProtection_test_context_t context;
        context.current.engineProtect.oil = true;
        context.page4.engineProtectMaxRPM = curLimit - 10; // lower than current limit
        TEST_ASSERT_EQUAL_UINT8(context.page4.engineProtectMaxRPM, applyEngineProtectionRevLimit(curLimit, context.current, context.page4));
    }

    {
        engineProtection_test_context_t context;
        context.current.engineProtect.boostCut = true;
        context.page4.engineProtectMaxRPM = 60;
        TEST_ASSERT_EQUAL_UINT8(context.page4.engineProtectMaxRPM, applyEngineProtectionRevLimit(curLimit, context.current, context.page4));
    }

    {
        engineProtection_test_context_t context;
        context.current.engineProtect.afr = true;
        context.page4.engineProtectMaxRPM = 45;
        TEST_ASSERT_EQUAL_UINT8(context.page4.engineProtectMaxRPM, applyEngineProtectionRevLimit(curLimit, context.current, context.page4));
    }
}

static void test_applyEngineProtectionRevLimit_multiple_protections_lowest_wins(void)
{
    engineProtection_test_context_t context;

    context.current.engineProtect.oil = true;
    context.current.engineProtect.boostCut = true;
    context.current.engineProtect.afr = true;
    context.page4.engineProtectMaxRPM = 40; // all use same limit

    uint8_t curLimit = 90;
    uint8_t result = applyEngineProtectionRevLimit(curLimit, context.current, context.page4);

    TEST_ASSERT_EQUAL_UINT8(40, result);
}

static void test_applyHardLaunchRevLimit(void)
{
    uint8_t curLimit = 80;

    engineProtection_test_context_t context;
    context.current.launchingHard = false;
    context.page6.lnchHardLim = curLimit-10;
    TEST_ASSERT_EQUAL_UINT8(curLimit, applyHardLaunchRevLimit(curLimit, context.current, context.page6)); // no change

    context.current.launchingHard = true;
    TEST_ASSERT_EQUAL_UINT8(context.page6.lnchHardLim, applyHardLaunchRevLimit(curLimit, context.current, context.page6)); // no change

    context.current.launchingHard = true;
    context.page6.lnchHardLim = curLimit+10;
    TEST_ASSERT_EQUAL_UINT8(curLimit, applyHardLaunchRevLimit(curLimit, context.current, context.page6)); // no change
}

static void test_applyFlatShiftRevLimit(void)
{
    uint16_t curLimit = 8000;
    engineProtection_test_context_t context;

    context.current.flatShiftingHard = false;
    context.current.clutchEngagedRPM = curLimit-100;
    TEST_ASSERT_EQUAL_UINT16(curLimit, applyFlatShiftRevLimit(curLimit, context.current)); // no change
   
    context.current.flatShiftingHard = true;
    TEST_ASSERT_EQUAL_UINT16(context.current.clutchEngagedRPM, applyFlatShiftRevLimit(curLimit, context.current)); // no change

    context.current.clutchEngagedRPM = curLimit+100;
    TEST_ASSERT_EQUAL_UINT16(curLimit, applyFlatShiftRevLimit(curLimit, context.current)); // no change
}

static void test_getMaxRpm_hard_fixed_limit(void)
{
    engineProtection_test_context_t context;
    // Hard fixed mode
    context.page9.hardRevMode = HARD_REV_FIXED;
    context.page4.HardRevLim = 50; // div100 -> 5000 RPM

    uint16_t result = getMaxRpm(context.current, context.page4, context.page6, context.page9);
    TEST_ASSERT_EQUAL_UINT16(5000, result);
}

static void test_getMaxRpm_coolant_limit(void)
{
    engineProtection_test_context_t context;
    // Coolant table must be populated
    populateCoolantProtectTable();
    context.page9.hardRevMode = HARD_REV_COOLANT;
    context.current.coolant = 0; // index hitting first bin -> table value 40

    uint16_t result = getMaxRpm(context.current, context.page4, context.page6, context.page9);
    TEST_ASSERT_EQUAL_UINT16((uint16_t)coolantProtectTable.values[0] * 100U, result);
}

static void test_getMaxRpm_engineProtectMaxRPM_applies(void)
{
    engineProtection_test_context_t context;
    // Hard fixed high, but engine protection active should clamp to engineProtectMaxRPM
    context.page9.hardRevMode = HARD_REV_FIXED;
    context.page4.HardRevLim = 80; // 8000
    context.current.engineProtect.boostCut = true; // any protection bit triggers the clamp
    context.page4.engineProtectMaxRPM = 60; // clamp to 6000

    uint16_t result = getMaxRpm(context.current, context.page4, context.page6, context.page9);
    TEST_ASSERT_EQUAL_UINT16(6000, result);
}

static void test_getMaxRpm_launch_and_flatshift_priority(void)
{
    engineProtection_test_context_t context;

    // Part A: launchingHard should apply lnchHardLim
    context.page9.hardRevMode = HARD_REV_FIXED;
    context.page4.HardRevLim = 80; // 8000
    context.current.launchingHard = true;
    context.page6.lnchHardLim = 30; // div100 -> 3000

    uint16_t resultA = getMaxRpm(context.current, context.page4, context.page6, context.page9);
    TEST_ASSERT_EQUAL_UINT16(3000, resultA);

    // Part B: flat shift clamps to absolute clutchEngagedRPM
    context.current.launchingHard = false;
    context.current.flatShiftingHard = true;
    context.current.clutchEngagedRPM = 2500; // absolute RPM clamp
    // Ensure other limits would be higher (force engine protect active to produce > 2500)
    context.current.engineProtect.boostCut = true;
    context.page4.engineProtectMaxRPM = 40; // 4000

    uint16_t resultB = getMaxRpm(context.current, context.page4, context.page6, context.page9);
    TEST_ASSERT_EQUAL_UINT16(2500, resultB);
}

static void test_checkRpmLimit_disabled(void) {
    engineProtection_test_context_t context;

    context.setRpmActive(HARD_REV_FIXED);
    context.page6.engineProtectType = PROTECT_CUT_OFF;
    TEST_ASSERT_FALSE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));

    context.setRpmActive(HARD_REV_COOLANT);
    context.page6.engineProtectType = PROTECT_CUT_OFF;
    TEST_ASSERT_FALSE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));

    context.setRpmActive(HARD_REV_COOLANT);
    context.page6.engineProtectType = PROTECT_CUT_IGN;
    TEST_ASSERT_FALSE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));

    context.setRpmActive(HARD_REV_COOLANT);
    context.page6.engineProtectType = PROTECT_CUT_FUEL;
    TEST_ASSERT_FALSE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));

    context.setRpmActive(HARD_REV_COOLANT);
    context.page6.engineProtectType = PROTECT_CUT_BOTH;
    TEST_ASSERT_FALSE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));

    // soft limit active and RPM < soft rev lim should not trigger
    context.setRpmActive(HARD_REV_FIXED);
    setRpm(context.current, RPM_COARSE.toUser(context.page4.SoftRevLim-1U));
    softLimitTime = context.page4.SoftLimMax + 1;
    TEST_ASSERT_FALSE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));
}

static void test_checkRpmLimit_fixed_and_softlimit(void) {
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);

    // below hard limit
    setRpm(context.current, RPM_COARSE.toUser(context.page4.HardRevLim - 1U));
    softLimitTime = 0;
    TEST_ASSERT_FALSE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));

    // equal to hard limit triggers
    context.setRpmActive(HARD_REV_FIXED);
    TEST_ASSERT_TRUE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));

    // soft limit active and RPM >= soft rev lim should trigger
    context.setRpmActive(HARD_REV_FIXED);
    setRpm(context.current, RPM_COARSE.toUser(context.page4.SoftRevLim));
    softLimitTime = context.page4.SoftLimMax + 1;
    TEST_ASSERT_TRUE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));

    // soft limit equality should not trigger
    context.setRpmActive(HARD_REV_FIXED);
    setRpm(context.current, RPM_COARSE.toUser(context.page4.SoftRevLim));
    softLimitTime = context.page4.SoftLimMax;
    TEST_ASSERT_FALSE(checkRpmLimit(context.current, context.page4, context.page6, context.page9));
}

static void test_checkCoolantLimit_disabled(void) {
    engineProtection_test_context_t context;

    context.setCoolantActive();
    context.page6.engineProtectType = PROTECT_CUT_OFF;
    TEST_ASSERT_FALSE(checkCoolantLimit(context.current, context.page6, context.page9));

    context.setCoolantActive();
    context.page9.hardRevMode = HARD_REV_FIXED;
    TEST_ASSERT_FALSE(checkCoolantLimit(context.current, context.page6, context.page9));
}

static void test_checkCoolantLimit_trigger_and_equal(void) {
    engineProtection_test_context_t context;

    context.setCoolantActive();
    context.page6.engineProtectType = PROTECT_CUT_IGN;
    TEST_ASSERT_TRUE(checkCoolantLimit(context.current, context.page6, context.page9));

    context.setCoolantActive();
    context.page6.engineProtectType = PROTECT_CUT_FUEL;
    TEST_ASSERT_TRUE(checkCoolantLimit(context.current, context.page6, context.page9));

    context.setCoolantActive();
    context.page6.engineProtectType = PROTECT_CUT_BOTH;
    TEST_ASSERT_TRUE(checkCoolantLimit(context.current, context.page6, context.page9));

    context.setCoolantActive();
    setRpm(context.current, RPM_COARSE.toUser(coolantProtectTable.values[0]));
    TEST_ASSERT_FALSE(checkCoolantLimit(context.current, context.page6, context.page9));
}

static void test_checkEngineProtection(void) {
    {
        engineProtection_test_context_t context;
        context.setOilPressureActive();
        context.setBoostActive();
        context.setAfrActive();
        context.setCoolantActive();
        auto flags = checkEngineProtection(context.current, context.page4, context.page6, context.page9, context.page10);
        TEST_ASSERT_TRUE(flags.boostCut);
        TEST_ASSERT_TRUE(flags.oil);
        TEST_ASSERT_TRUE(flags.afr);
        TEST_ASSERT_TRUE(flags.coolant);
        TEST_ASSERT_TRUE(flags.rpm); // Active when coolant active
    }

    {
        engineProtection_test_context_t context;
        context.setRpmActive(HARD_REV_FIXED);
        auto flags = checkEngineProtection(context.current, context.page4, context.page6, context.page9, context.page10);
        TEST_ASSERT_FALSE(flags.boostCut);
        TEST_ASSERT_FALSE(flags.oil);
        TEST_ASSERT_FALSE(flags.afr);
        TEST_ASSERT_FALSE(flags.coolant);
        TEST_ASSERT_TRUE(flags.rpm);
    }

    // Even if parameters would normally trigger, engineProtectType off disables checks
    {
        engineProtection_test_context_t context;
        context.setOilPressureActive();
        context.setBoostActive();
        context.setAfrActive();
        context.setCoolantActive();
        context.page6.engineProtectType = PROTECT_CUT_OFF;
        auto flags = checkEngineProtection(context.current, context.page4, context.page6, context.page9, context.page10);
        TEST_ASSERT_FALSE(flags.boostCut);
        TEST_ASSERT_FALSE(flags.oil);
        TEST_ASSERT_FALSE(flags.afr);
        TEST_ASSERT_FALSE(flags.coolant);
        TEST_ASSERT_FALSE(flags.rpm);
    }
}

static void test_calculateFuelIgnitionChannelCut_nosync(void)
{
    engineProtection_test_context_t context;
    resetInternalState();
    context.decoderStatus.syncStatus = SyncStatus::None;

    auto onOff = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(SchedulerCutStatus::Full, onOff.status);
}

static void test_calculateFuelIgnitionChannelCut_staging_complete_all_on(void)
{
    engineProtection_test_context_t context;
    context.setBeyondStaging();

    auto onOff = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(SchedulerCutStatus::None, onOff.status);
}

static void test_calculateFuelIgnitionChannelCut_hardcut_full_ignition_only(void)
{
    engineProtection_test_context_t context;
    context.setHardCutFull();
    context.setRpmActive(HARD_REV_FIXED);
    
    auto onOff = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.fuelChannels); // fuel remains on
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.ignitionChannels); // ignition cut
    TEST_ASSERT_EQUAL(SchedulerCutStatus::Full, onOff.status);
}

static void test_calculateFuelIgnitionChannelCut_hardcut_full_fuel_only(void)
{
    engineProtection_test_context_t context;
    context.setHardCutFull();
    context.setRpmActive(HARD_REV_FIXED);

    context.page6.engineProtectType = PROTECT_CUT_FUEL;
    
    auto onOff = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.fuelChannels); // fuel cut
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.ignitionChannels); // ignition on
    TEST_ASSERT_EQUAL(SchedulerCutStatus::Full, onOff.status);
}

static void test_calculateFuelIgnitionChannelCut_hardcut_full_both(void)
{
    engineProtection_test_context_t context;
    context.setHardCutFull();
    context.setRpmActive(HARD_REV_FIXED);

    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    auto onOff = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(SchedulerCutStatus::Full, onOff.status);
}

static void test_calculateFuelIgnitionChannelCut_rolling_cut_ignition_only(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.current.maxIgnOutputs = 1;
    context.current.maxInjOutputs = 1;

    auto onOff = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_BITS_HIGH(0x01, onOff.fuelChannels);
    TEST_ASSERT_BITS_LOW(0x01, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(SchedulerCutStatus::Full, onOff.status);
}

static void test_calculateFuelIgnitionChannelCut_rolling_cut_both(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    context.current.maxIgnOutputs = 1;
    context.current.maxInjOutputs = 1;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    auto onOff = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_BITS_LOW(0x01, onOff.fuelChannels);
    TEST_ASSERT_BITS_LOW(0x01, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(SchedulerCutStatus::Full, onOff.status);
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

    auto onOff = context.calculateFuelIgnitionChannelCut();
    // At least the lower 4 bits should be cleared -> 0xF0
    TEST_ASSERT_BITS_LOW(0x0F, onOff.fuelChannels);
    TEST_ASSERT_BITS_LOW(0x0F, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(SchedulerCutStatus::Full, onOff.status);
}

static void test_calculateFuelIgnitionChannelCut_fullcut_updates_rollingCutLastRev(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();
    setRpm(context.current, RPM_COARSE.toUser(context.page4.HardRevLim) + SIGNED_RPM_MEDIUM.toUser(rollingCutTable.axis[0]) + 1U);

    rollingCutLastRev = 0;
    auto onOff = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_EQUAL_UINT32(context.current.startRevolutions, rollingCutLastRev);
    TEST_ASSERT_EQUAL(context.current.schedulerCutState.status, onOff.status);
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

    auto cut = context.calculateFuelIgnitionChannelCut();

    TEST_ASSERT_EQUAL_HEX8(cut.fuelChannels, cut.ignitionChannels);
    TEST_ASSERT_EQUAL_UINT8(0, cut.ignitionChannelsPending);
    TEST_ASSERT_EQUAL(SchedulerCutStatus::Rolling, cut.status);
}

static void test_calculateFuelIgnitionChannelCut_no_rolling_cut_does_not_update_lastRev(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    setRpm(context.current, (context.page4.HardRevLim*100U) + SIGNED_RPM_MEDIUM.toUser(rollingCutTable.axis[0]) - 1); // below threshold
    
    rollingCutLastRev = 0;
    auto cut = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_EQUAL_UINT32(0U, rollingCutLastRev);
    TEST_ASSERT_EQUAL(SchedulerCutStatus::None, cut.status);
}

static void test_useRollingCut(void)
{
    engineProtection_test_context_t context;
    constexpr uint16_t maxRPM = 5000U;
    
    // Test with HARD_CUT_FULL
    context.setHardCutRolling();
    context.page2.hardCutType = HARD_CUT_FULL;
    setRpm(context.current, maxRPM);
    TEST_ASSERT_FALSE(useRollingCut(context.current, context.page2, maxRPM));
    setRpm(context.current, maxRPM + SIGNED_RPM_MEDIUM.toUser(rollingCutTable.axis[2]));
    TEST_ASSERT_FALSE(useRollingCut(context.current, context.page2, maxRPM));

    context.setHardCutRolling();
    setRpm(context.current, maxRPM + SIGNED_RPM_MEDIUM.toUser(rollingCutTable.axis[0]) - 1); // below threshold    
    TEST_ASSERT_FALSE(useRollingCut(context.current, context.page2, maxRPM));
    setRpm(context.current, maxRPM + SIGNED_RPM_MEDIUM.toUser(rollingCutTable.axis[0])); // exactly at threshold - should NOT trigger (needs >)
    TEST_ASSERT_FALSE(useRollingCut(context.current, context.page2, maxRPM));
    setRpm(context.current, maxRPM + SIGNED_RPM_MEDIUM.toUser(rollingCutTable.axis[0])+1); // just above threshold - cut
    TEST_ASSERT_TRUE(useRollingCut(context.current, context.page2, maxRPM));

    setRpm(context.current, maxRPM); // At max - no rolling cut
    TEST_ASSERT_FALSE(useRollingCut(context.current, context.page2, maxRPM));
    setRpm(context.current, maxRPM+1); // Above max - no rolling cut
    TEST_ASSERT_FALSE(useRollingCut(context.current, context.page2, maxRPM));
    setRpm(context.current, maxRPM - 1); // just below threshold - cut
    TEST_ASSERT_TRUE(useRollingCut(context.current, context.page2, maxRPM));
}

static void test_calcRollingCutRevolutions(void)
{
    engineProtection_test_context_t context;
    
    // Create a lookup of all possible configurations
    // 2 stroke types × 2 ignition types × 2 fuel types = 8 combinations
    
    // Config 1: 2-stroke, seq ign, seq fuel -> 1
    context.page2.strokes = TWO_STROKE;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.page2.injLayout = INJ_SEQUENTIAL;
    TEST_ASSERT_EQUAL_UINT8(1, calcRollingCutRevolutions(context.page2, context.page4));
    
    // Config 2: 2-stroke, non-seq ign, seq fuel -> 2
    context.page4.sparkMode = IGN_MODE_WASTED;
    TEST_ASSERT_EQUAL_UINT8(2, calcRollingCutRevolutions(context.page2, context.page4));
    
    // Config 3: 2-stroke, seq ign, non-seq fuel -> 2
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.page2.injLayout = INJ_BANKED;
    TEST_ASSERT_EQUAL_UINT8(2, calcRollingCutRevolutions(context.page2, context.page4));
    
    // Config 4: 2-stroke, non-seq ign, non-seq fuel -> 2
    context.page4.sparkMode = IGN_MODE_WASTED;
    TEST_ASSERT_EQUAL_UINT8(2, calcRollingCutRevolutions(context.page2, context.page4));
    
    // Config 5: 4-stroke, seq ign, seq fuel -> 2
    context.page2.strokes = FOUR_STROKE;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.page2.injLayout = INJ_SEQUENTIAL;
    TEST_ASSERT_EQUAL_UINT8(2, calcRollingCutRevolutions(context.page2, context.page4));
    
    // Config 6: 4-stroke, non-seq ign, seq fuel -> 4
    context.page4.sparkMode = IGN_MODE_WASTED;
    TEST_ASSERT_EQUAL_UINT8(4, calcRollingCutRevolutions(context.page2, context.page4));
    
    // Config 7: 4-stroke, seq ign, non-seq fuel -> 4
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.page2.injLayout = INJ_BANKED;
    TEST_ASSERT_EQUAL_UINT8(4, calcRollingCutRevolutions(context.page2, context.page4));
    
    // Config 8: 4-stroke, non-seq ign, non-seq fuel -> 4
    context.page4.sparkMode = IGN_MODE_WASTED;
    TEST_ASSERT_EQUAL_UINT8(4, calcRollingCutRevolutions(context.page2, context.page4));
}

static void test_calcRollingCutPercentage(void)
{
    engineProtection_test_context_t context;
    context.setHardCutRolling();    
    constexpr uint16_t maxRPM = 5000U;
    
    setRpm(context.current, maxRPM);
    TEST_ASSERT_EQUAL_UINT8(101U, calcRollingCutPercentage(context.current, maxRPM));
    setRpm(context.current, maxRPM+1);
    TEST_ASSERT_EQUAL_UINT8(101U, calcRollingCutPercentage(context.current, maxRPM));
    setRpm(context.current, maxRPM+1000);
    TEST_ASSERT_EQUAL_UINT8(101U, calcRollingCutPercentage(context.current, maxRPM));
    
    setRpm(context.current, maxRPM+SIGNED_RPM_MEDIUM.toUser(rollingCutTable.axis[2]));
    TEST_ASSERT_EQUAL_UINT8(rollingCutTable.values[2], calcRollingCutPercentage(context.current, maxRPM));
    
    // Test division underflow.
    setRpm(context.current, maxRPM+(INT8_MIN*11));
    TEST_ASSERT_EQUAL_UINT8(rollingCutTable.values[0], calcRollingCutPercentage(context.current, maxRPM));

    // RPM just below max -> table lookup
    setRpm(context.current, maxRPM-1U);
    TEST_ASSERT_GREATER_THAN(0, calcRollingCutPercentage(context.current, maxRPM));
}

static void test_channelOn_without_pending(void)
{
    // When pendingIgnitionCut=false, both ignition and fuel should be turned on
    statuses::scheduler_cut_t cutState = { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x00, .fuelChannels = 0x00, .status = SchedulerCutStatus::None };
    cutState = channelOn(cutState, false, 0);
    TEST_ASSERT_EQUAL_HEX8(0x01, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x01, cutState.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0x00, cutState.ignitionChannelsPending);

    // Test that different channels can be turned on independently  
    cutState = channelOn(cutState, false, 2);
    TEST_ASSERT_EQUAL_HEX8(0x05, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x05, cutState.fuelChannels);
    
    cutState = channelOn(cutState, false, 5);
    TEST_ASSERT_EQUAL_HEX8(0x25, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x25, cutState.fuelChannels);
}

static void test_channelOn_with_pending(void)
{
    // When pendingIgnitionCut=true and fuel is off, ignition should be set as pending
    statuses::scheduler_cut_t cutState = { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x00, .fuelChannels = 0x00, .status = SchedulerCutStatus::None };
    
    cutState = channelOn(cutState, true, 0);
    TEST_ASSERT_EQUAL_HEX8(0x00, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x01, cutState.ignitionChannelsPending);
    TEST_ASSERT_EQUAL_HEX8(0x01, cutState.fuelChannels);

    // When pendingIgnitionCut=true but fuel is already on, ignition should turn on normally
    cutState = { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x00, .fuelChannels = 0xFF, .status = SchedulerCutStatus::None };
    cutState = channelOn(cutState, true, 0);
    TEST_ASSERT_EQUAL_HEX8(0x01, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x00, cutState.ignitionChannelsPending);
    TEST_ASSERT_EQUAL_HEX8(0xFF, cutState.fuelChannels);

    // Test pending logic for different channels
    cutState = { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x00, .fuelChannels = 0x00, .status = SchedulerCutStatus::None };
    cutState = channelOn(cutState, true, 3);
    TEST_ASSERT_EQUAL_HEX8(0x00, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x08, cutState.ignitionChannelsPending);  
    cutState = channelOn(cutState, true, 7);
    TEST_ASSERT_EQUAL_HEX8(0x00, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x88, cutState.ignitionChannelsPending);

    // Ensure turning on one channel doesn't affect other ignition channels
    cutState = { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x02, .fuelChannels = 0x02, .status = SchedulerCutStatus::None };  
    cutState = channelOn(cutState, false, 4);
    TEST_ASSERT_EQUAL_HEX8(0x12, cutState.ignitionChannels);  // 0x02 | 0x10 = 0x12
    TEST_ASSERT_EQUAL_HEX8(0x12, cutState.fuelChannels);

    // Ensure turning on one channel doesn't affect other fuel channels
    cutState = { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x00, .fuelChannels = 0x80, .status = SchedulerCutStatus::None };
    cutState = channelOn(cutState, false, 1);
    TEST_ASSERT_EQUAL_HEX8(0x02, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x82, cutState.fuelChannels);  // 0x80 | 0x02 = 0x82
}

static void test_channelOn_pending_then_fuel_on(void)
{
    // Test transition: pending ignition when fuel off, then fuel turns on
    statuses::scheduler_cut_t cutState = { .ignitionChannelsPending = 0x00, .ignitionChannels = 0x00, .fuelChannels = 0x00, .status = SchedulerCutStatus::None };
    
    // First call: pending ignition set
    cutState = channelOn(cutState, true, 1);
    TEST_ASSERT_EQUAL_HEX8(0x02, cutState.ignitionChannelsPending);
    TEST_ASSERT_EQUAL_HEX8(0x00, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x02, cutState.fuelChannels);
    
    // Second call: fuel is now on, so ignition should turn on immediately
    cutState = channelOn(cutState, true, 1);
    TEST_ASSERT_EQUAL_HEX8(0x02, cutState.ignitionChannels);
    TEST_ASSERT_EQUAL_HEX8(0x02, cutState.ignitionChannelsPending);  // unchanged
    TEST_ASSERT_EQUAL_HEX8(0x02, cutState.fuelChannels);
}

static void setSupportPendingIgnitionCut(engineProtection_test_context_t &context)
{
    context.page2.strokes = FOUR_STROKE;
    context.page4.sparkMode = IGN_MODE_WASTED;
    context.page2.injLayout = INJ_PAIRED;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;
}

static void test_supportPendingIgnitionCut(void)
{
    engineProtection_test_context_t context;

    // Positive tests
    setSupportPendingIgnitionCut(context);
    TEST_ASSERT_TRUE(supportPendingIgnitionCut(context.page2, context.page4, context.page6));
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    TEST_ASSERT_TRUE(supportPendingIgnitionCut(context.page2, context.page4, context.page6));
    context.page4.sparkMode = IGN_MODE_WASTED;
    context.page2.injLayout = INJ_SEQUENTIAL;
    TEST_ASSERT_TRUE(supportPendingIgnitionCut(context.page2, context.page4, context.page6));

    // Negative tests
    setSupportPendingIgnitionCut(context);
    context.page2.strokes = TWO_STROKE;
    TEST_ASSERT_FALSE(supportPendingIgnitionCut(context.page2, context.page4, context.page6));

    setSupportPendingIgnitionCut(context);
    context.page6.engineProtectType = PROTECT_CUT_FUEL;
    TEST_ASSERT_FALSE(supportPendingIgnitionCut(context.page2, context.page4, context.page6));
    setSupportPendingIgnitionCut(context);
    context.page6.engineProtectType = PROTECT_CUT_IGN;
    TEST_ASSERT_FALSE(supportPendingIgnitionCut(context.page2, context.page4, context.page6));

    setSupportPendingIgnitionCut(context);
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.page2.injLayout = INJ_SEQUENTIAL;
    TEST_ASSERT_FALSE(supportPendingIgnitionCut(context.page2, context.page4, context.page6));
}

// Deterministic RNG stubs for tests
static uint8_t deterministic_rand_low(void)  { return 1U; }   // always triggers (< cutPercent)
static uint8_t deterministic_rand_high(void) { return 255U; } // never triggers (>= cutPercent)
static uint8_t deterministic_rand_flipper(void)
{
    // This flips the "random" number between high & low on each call.
    static bool onOff = false;
    onOff = !onOff;
    return onOff ? deterministic_rand_low() : deterministic_rand_high();
}

static void test_applyRollingCutPercentage_all_cut(void)
{
    engineProtection_test_context_t context;

    // Inject deterministic RNG that always triggers cuts
    rollingCutRandFunc_override_t rngOverride(deterministic_rand_low);

    context.current.maxInjOutputs = 5;
    context.current.maxIgnOutputs = 4;
    context.current.schedulerCutState.fuelChannels = 0xFF;
    context.current.schedulerCutState.ignitionChannels = 0xFF;
    context.current.schedulerCutState.status = SchedulerCutStatus::None;    
    auto onOff = applyRollingCutPercentage(context.current, context.page6, 50U, false);

    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0x00, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(onOff.status, SchedulerCutStatus::Rolling);
}

static void test_applyRollingCutPercentage_all_on(void)
{
    engineProtection_test_context_t context;

    // Inject deterministic RNG that never triggers cuts
    rollingCutRandFunc_override_t rngOverride(deterministic_rand_high);

    context.current.schedulerCutState.fuelChannels = 0xFF;
    context.current.schedulerCutState.ignitionChannels = 0xFF;
    context.current.schedulerCutState.status = SchedulerCutStatus::None;    
    context.current.maxInjOutputs = 3;
    context.current.maxIgnOutputs = 2;
    auto onOff = applyRollingCutPercentage(context.current, context.page6, 50U, false);
    TEST_ASSERT_EQUAL_HEX8(0x07, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0x03, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(onOff.status, SchedulerCutStatus::Rolling);

    context.current.maxInjOutputs = 4;
    context.current.maxIgnOutputs = 5;
    onOff = applyRollingCutPercentage(context.current, context.page6, 50U, false);
    TEST_ASSERT_EQUAL_HEX8(0x0F, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0x1F, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(onOff.status, SchedulerCutStatus::Rolling);

    context.current.maxInjOutputs = 2;
    context.current.maxIgnOutputs = 1;
    onOff = applyRollingCutPercentage(context.current, context.page6, 50U, false);
    TEST_ASSERT_EQUAL_HEX8(0x03, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0x01, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(onOff.status, SchedulerCutStatus::Rolling);
}

static void test_applyRollingCutPercentage_half_on(void)
{
    engineProtection_test_context_t context;

    // Inject deterministic RNG that never triggers cuts
    rollingCutRandFunc_override_t rngOverride(deterministic_rand_flipper);

    context.current.schedulerCutState.fuelChannels = 0xFF;
    context.current.schedulerCutState.ignitionChannels = 0xFF;
    context.current.schedulerCutState.status = SchedulerCutStatus::None;
    context.current.maxInjOutputs = 5;
    context.current.maxIgnOutputs = 3;
    auto onOff = applyRollingCutPercentage(context.current, context.page6, 50U, false);
    TEST_ASSERT_EQUAL_HEX8(0x0A, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0x02, onOff.ignitionChannels);
    TEST_ASSERT_EQUAL(onOff.status, SchedulerCutStatus::Rolling);
}

static void test_applyPendingIgnitionCuts(void)
{
    engineProtection_test_context_t context;
    
    constexpr statuses::scheduler_cut_t cutStateInitial = { 
        .ignitionChannelsPending = 0x0F, 
        .ignitionChannels = 0x01,
        .fuelChannels = 0x0F, 
        .status = SchedulerCutStatus::None 
    };

    context.current.startRevolutions = rollingCutLastRev + 2U;
    statuses::scheduler_cut_t testSubject = applyPendingIgnitionCuts(cutStateInitial, context.current);
    TEST_ASSERT_EQUAL(testSubject.fuelChannels, testSubject.ignitionChannels);
    TEST_ASSERT_EQUAL(0, testSubject.ignitionChannelsPending);
    TEST_ASSERT_EQUAL(SchedulerCutStatus::Rolling, testSubject.status);

    // Not enough revolutions yet
    context.current.startRevolutions = rollingCutLastRev;
    testSubject = applyPendingIgnitionCuts(cutStateInitial, context.current);
    TEST_ASSERT_EQUAL(cutStateInitial.ignitionChannels, testSubject.ignitionChannels);
    TEST_ASSERT_EQUAL(cutStateInitial.ignitionChannelsPending, testSubject.ignitionChannelsPending);
    TEST_ASSERT_EQUAL(cutStateInitial.status, testSubject.status);
}

// Force all channels to be cut via deterministic low RNG
static void test_calculateFuelIgnitionChannelCut_rolling_cut_forced_all_channels_cut(void)
{
    engineProtection_test_context_t context;
    context.setRpmActive(HARD_REV_FIXED);
    context.setHardCutRolling();

    setRpm(context.current, context.current.RPM * 2U); // ensure full-cut condition if rpmDelta >= 0
    context.current.maxIgnOutputs = 4;
    context.current.maxInjOutputs = 4;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;

    // Inject deterministic RNG that always triggers cuts
    rollingCutRandFunc_override_t rngOverride(deterministic_rand_low);

    auto onOff = context.calculateFuelIgnitionChannelCut();
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
    setRpm(context.current, RPM_COARSE.toUser(context.page4.HardRevLim) - RPM_MEDIUM.toUser(rollingCutTable.axis[1] * -1)); // use mid axis

    // Inject deterministic RNG that never triggers cuts
    rollingCutRandFunc_override_t rngOverride(deterministic_rand_high);

    auto onOff = context.calculateFuelIgnitionChannelCut();

    // No channels should be cut
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.ignitionChannels);
}

// Test getMaxRpm with both launchingHard AND flatShiftingHard active
static void test_getMaxRpm_launch_and_flatshift_both_active(void)
{
    engineProtection_test_context_t context;
    context.page9.hardRevMode = HARD_REV_FIXED;
    context.page4.HardRevLim = RPM_COARSE.toRaw(8000);
    context.current.launchingHard = true;
    context.page6.lnchHardLim = RPM_COARSE.toRaw(5000U);
    context.current.flatShiftingHard = true;
    context.current.clutchEngagedRPM = 3000; // Even lower

    // Flat shift should win (lowest limit)
    TEST_ASSERT_EQUAL_UINT16(context.current.clutchEngagedRPM, getMaxRpm(context.current, context.page4, context.page6, context.page9));
}

// Test staging boundary at StgCycles
static void test_calculateFuelIgnitionChannelCut_at_staging_boundary(void)
{
    engineProtection_test_context_t context;
    context.setBeyondStaging();

    // Exactly at StgCycles should still cut
    context.current.startRevolutions = context.page4.StgCycles;
    auto onOff = context.calculateFuelIgnitionChannelCut();
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.fuelChannels);
    TEST_ASSERT_EQUAL_HEX8(0xFF, onOff.ignitionChannels);
}

// Test oil pressure protection timer reset when pressure recovers
static void test_checkOilPressureLimit_timer_reset_on_recovery(void)
{
    engineProtection_test_context_t context;
    resetInternalState();
    context.page10.oilPressureProtTime = 5; // 50ms delay
    unsigned long now = 2000;

    // Pressure below limit - arms timer
    context.setOilPressureActive();
    checkOilPressureLimit(context.current, context.page6, context.page10, now);
    uint32_t firstExpiry = oilProtEndTime;
    TEST_ASSERT_NOT_EQUAL(0U, firstExpiry);

    // Pressure recovers - timer should reset
    context.current.oilPressure = table2D_getValue(&oilPressureProtectTable, context.current.RPMdiv100) + 10;
    checkOilPressureLimit(context.current, context.page6, context.page10, now + 1);
    TEST_ASSERT_EQUAL_UINT32(0U, oilProtEndTime);

    // Pressure drops again - new timer armed
    context.current.oilPressure = table2D_getValue(&oilPressureProtectTable, context.current.RPMdiv100) - 5;
    checkOilPressureLimit(context.current, context.page6, context.page10, now + 2);
    uint32_t secondExpiry = oilProtEndTime;
    TEST_ASSERT_NOT_EQUAL(0U, secondExpiry);
    TEST_ASSERT_NOT_EQUAL(firstExpiry, secondExpiry); // Different expiry time
}

// Test AFR protection condition with all boundaries at limits
static void test_checkAFRLimit_all_boundaries_at_exact_limits(void)
{
    engineProtection_test_context_t context;
    context.page9.afrProtectEnabled = AFR_PROTECT_TABLE;
    context.page9.afrProtectMinMAP = 50;
    context.page9.afrProtectMinRPM = 10;
    context.page9.afrProtectMinTPS = 10;
    context.page9.afrProtectDeviation = 5;
    context.page9.afrProtectCutTime = 0;
    context.page6.engineProtectType = PROTECT_CUT_BOTH;
    context.page6.egoType = EGO_TYPE_WIDE;

    // Set all exactly at boundaries
    context.current.MAP = MAP.toUser(context.page9.afrProtectMinMAP);
    context.current.RPMdiv100 = context.page9.afrProtectMinRPM;
    context.current.TPS = context.page9.afrProtectMinTPS;
    context.current.O2 = context.current.afrTarget + (uint16_t)context.page9.afrProtectDeviation;

    // Should activate
    TEST_ASSERT_TRUE(checkAFRLimit(context.current, context.page6, context.page9, 1000));
    TEST_ASSERT_TRUE(checkAFRLimitActive);

    // Drop just one below: should clear conditions but keep checkAFRLimitActive true due to logic
    context.current.MAP = MAP.toUser(context.page9.afrProtectMinMAP) - 1;
    checkAFRLimit(context.current, context.page6, context.page9, 1001); // Call without TPS reduction
    // checkAFRLimitActive should remain true (condition not met but not clearing either)
    TEST_ASSERT_TRUE(checkAFRLimitActive);
}

void runAllTests(void)
{
    SET_UNITY_FILENAME() {

    RUN_TEST_P(test_checkOilPressureLimit_basic);
    RUN_TEST_P(test_checkOilPressureLimit_activate_when_time_expires);
    RUN_TEST_P(test_checkOilPressureLimit_timer_and_activation)
    RUN_TEST_P(test_checkOilPressureLimit_existing_engineProtect_forces_cut);
    RUN_TEST_P(test_checkOilPressureLimit_timer_resets_when_pressure_recovers);
    RUN_TEST_P(test_checkOilPressureLimit_timer_reset_on_recovery);
    RUN_TEST_P(test_checkBoostLimit_disabled_conditions);
    RUN_TEST_P(test_checkBoostLimit_activate_when_conditions_met);
    RUN_TEST_P(test_checkAFRLimit_disabled_conditions);
    RUN_TEST_P(test_checkAFRLimit_activate_after_delay_and_reactivate_on_tps);
    RUN_TEST_P(test_checkAFRLimit_immediate_cut_time_zero);
    RUN_TEST_P(test_checkAFRLimit_table_mode_boundary);
    RUN_TEST_P(test_checkAFRLimit_delay_boundary_robustness);
    RUN_TEST_P(test_checkAFRLimit_zero_deviation_fixed_mode);
    RUN_TEST_P(test_checkAFRLimit_all_boundaries_at_exact_limits);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_ignition_only);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_both);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_multi_channel_fullcut);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_fullcut_updates_rollingCutLastRev);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_pending_ignition_clears_deterministic);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_no_rolling_cut_does_not_update_lastRev);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_at_staging_boundary);
    RUN_TEST_P(test_checkRpmLimit_disabled);
    RUN_TEST_P(test_checkRpmLimit_fixed_and_softlimit);
    RUN_TEST_P(test_checkCoolantLimit_disabled);
    RUN_TEST_P(test_checkCoolantLimit_trigger_and_equal);
    RUN_TEST_P(test_checkEngineProtection);
    RUN_TEST_P(test_getHardRevLimit);
    RUN_TEST_P(test_applyEngineProtectionRevLimit);
    RUN_TEST_P(test_applyEngineProtectionRevLimit_multiple_protections_lowest_wins);
    RUN_TEST_P(test_applyHardLaunchRevLimit);
    RUN_TEST_P(test_applyFlatShiftRevLimit);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_nosync);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_staging_complete_all_on);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_hardcut_full_ignition_only);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_hardcut_full_fuel_only);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_hardcut_full_both);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_forced_all_channels_cut);
    RUN_TEST_P(test_calculateFuelIgnitionChannelCut_rolling_cut_forced_no_channel_cut);
    RUN_TEST_P(test_getMaxRpm_hard_fixed_limit);
    RUN_TEST_P(test_getMaxRpm_coolant_limit);
    RUN_TEST_P(test_getMaxRpm_engineProtectMaxRPM_applies);
    RUN_TEST_P(test_getMaxRpm_launch_and_flatshift_priority);
    RUN_TEST_P(test_getMaxRpm_launch_and_flatshift_both_active);
    RUN_TEST_P(test_useRollingCut);
    RUN_TEST_P(test_calcRollingCutRevolutions);
    RUN_TEST_P(test_calcRollingCutPercentage);
    RUN_TEST_P(test_channelOn_without_pending);
    RUN_TEST_P(test_channelOn_with_pending);
    RUN_TEST_P(test_channelOn_pending_then_fuel_on);
    RUN_TEST_P(test_supportPendingIgnitionCut);
    RUN_TEST_P(test_applyRollingCutPercentage_all_cut);
    RUN_TEST_P(test_applyRollingCutPercentage_all_on);
    RUN_TEST_P(test_applyRollingCutPercentage_half_on);
    RUN_TEST_P(test_applyPendingIgnitionCuts);
    }
}

TEST_HARNESS(runAllTests)
