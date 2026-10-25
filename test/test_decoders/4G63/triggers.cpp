#include "../test_utils.h"
#include "src/decoders/4g63.h"
#include "scheduler_ignition_controller.h"

namespace decoders {
    namespace fourg63 {
        extern bool is70DegTooth(uint16_t toothNum);
    }
}

static void assert_synced_filter(uint8_t cylinderCount, uint8_t filterMode, uint16_t initialToothCount, uint16_t expectedToothAngle, uint32_t expectedFilterTime, uint16_t rpm = 3000)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = cylinderCount;
    page4.triggerFilter = filterMode;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    current.startRevolutions = 1;
    current.setRpm(rpm);
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.triggerFilterTime = 0;
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = initialToothCount;

    decoders::fourg63::triggerPrimary(1800, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(initialToothCount + 1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT16(expectedToothAngle, decoderState.triggerToothAngle);
    TEST_ASSERT_EQUAL_UINT32(expectedFilterTime, decoderState.triggerFilterTime);
}

static void assert_unsynced_pin_state(uint8_t cylinderCount, bool primaryHigh, bool secondaryHigh, bool initialRevolutionOne, uint16_t expectedToothCount, bool expectedRevolutionOne, SyncStatus expectedSyncStatus)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = cylinderCount;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    current.startRevolutions = 1;
    current.decoder.primary._pin.setPin(2);
    current.decoder.secondary._pin.setPin(3);
    if(primaryHigh) { current.decoder.primary._pin._pin.setPinHigh(); }
    else { current.decoder.primary._pin._pin.setPinLow(); }
    if(secondaryHigh) { current.decoder.secondary._pin._pin.setPinHigh(); }
    else { current.decoder.secondary._pin._pin.setPinLow(); }
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 1;
    decoderState.revolutionOne = initialRevolutionOne;

    decoders::fourg63::triggerPrimary(3000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(expectedToothCount, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(expectedRevolutionOne, decoderState.revolutionOne);
    TEST_ASSERT_EQUAL(expectedSyncStatus, decoderState.decoderStatus.syncStatus);
}

static void assert_primary_per_tooth_schedule(uint8_t sparkMode, uint16_t endTooth)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page4.sparkMode = sparkMode;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    setAngleConverterRevolutionTime(6000000UL);
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 2000;
    current.setRpm(3000);
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.triggerFilterTime = 0;
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.ignitionEndTeeth[0] = endTooth;
    
    stopIgnitionSchedulers();
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 600;

    page2.perToothIgn = false;
    page4.triggerAngle = 0;
    page2.nCylinders = 4;
    current.advance = 20;
    decoders::fourg63::triggerPrimary(1800, current, decoderState, page2, page4);
    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);

    page2.perToothIgn = true;
    page4.triggerAngle = 1;
    page2.nCylinders = 4;
    current.advance = 20;
    decoders::fourg63::triggerPrimary(1800, current, decoderState, page2, page4);
    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);

    page2.perToothIgn = true;
    page4.triggerAngle = 0;
    page2.nCylinders = 3;
    current.advance = 20;
    decoders::fourg63::triggerPrimary(1800, current, decoderState, page2, page4);
    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);

    page2.perToothIgn = true;
    page4.triggerAngle = 0;
    page2.nCylinders = 4;
    current.advance = 0;
    decoders::fourg63::triggerPrimary(1800, current, decoderState, page2, page4);
    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);

    page2.perToothIgn = true;
    page4.triggerAngle = 0;
    page2.nCylinders = 4;
    current.advance = 20;
    decoderState.toothCurrentCount = page2.nCylinders;
    decoders::fourg63::triggerPrimary(1800, current, decoderState, page2, page4);

    TEST_ASSERT_NOT_EQUAL(100, ignitionSchedule1._compare);
}

static void assert_secondary_primary_pin_sync(uint8_t cylinderCount, uint16_t toothCount, bool primaryHigh, SyncStatus expectedSyncStatus)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = cylinderCount;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    decoderState.toothCurrentCount = toothCount;
    decoderState.toothLastSecToothTime = 1000;
    decoderState.triggerSecFilterTime = 0;
    current.setRpm(3000);
    current.decoder.primary._pin.setPin(2);
    if(primaryHigh) { current.decoder.primary._pin._pin.setPinHigh(); }
    else { current.decoder.primary._pin._pin.setPinLow(); }

    decoders::fourg63::triggerSecondary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(expectedSyncStatus, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastSecToothTime);
}

static void test_primary_accepts_startup_pulse_before_filter(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = 4;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);

    decoders::fourg63::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(1, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
}

static void test_primary_rejects_filtered_pulse(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = 4;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    current.startRevolutions = 1;
    decoderState.toothLastToothTime = 1000;

    decoders::fourg63::triggerPrimary(1200, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT32(200, decoderState.curGap);
    TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT16(99, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
}

static void test_primary_syncs_six_cylinder_on_secondary_edge(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = 6;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    current.startRevolutions = 1;
    current.decoder.primary._pin.setPin(2);
    current.decoder.secondary._pin.setPin(3);
    current.decoder.primary._pin._pin.setPinLow();
    current.decoder.secondary._pin._pin.setPinHigh();
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.curGap = 500;
    decoderState.toothCurrentCount = 1;
    decoderState.revolutionOne = true;

    decoders::fourg63::triggerPrimary(3000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
}

static void test_primary_updates_synced_tooth_filter(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = 4;
    page4.triggerFilter = TRIGGER_FILTER_LITE;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    current.startRevolutions = 1;
    current.setRpm(3000);
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.triggerFilterTime = 0;
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 1;

    decoders::fourg63::triggerPrimary(1500, current, decoderState, page2, page4);
    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT16(110, decoderState.triggerToothAngle);
    TEST_ASSERT_EQUAL_UINT32(187, decoderState.triggerFilterTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1500, decoderState.toothLastToothTime);

    page2.nCylinders = 6;
    decoderState.triggerFilterTime = 0;
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 1;
    decoders::fourg63::triggerPrimary(1500, current, decoderState, page2, page4);
    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT16(50, decoderState.triggerToothAngle);
    TEST_ASSERT_EQUAL_UINT32(250, decoderState.triggerFilterTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1500, decoderState.toothLastToothTime);
}

static void test_primary_wraps_tooth_count_and_updates_tooth_one_time(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = 4;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    current.startRevolutions = 3;
    current.setRpm(3000);
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 8;
    decoderState.toothOneMinusOneTime = 200;
    decoderState.toothOneTime = 600;

    decoders::fourg63::triggerPrimary(3000, current, decoderState, page2, page4);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(4, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(600, decoderState.toothOneMinusOneTime);
    TEST_ASSERT_EQUAL_UINT32(3000, decoderState.toothOneTime);

    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 0;
    decoderState.toothOneMinusOneTime = 200;
    decoderState.toothOneTime = 600;
    decoders::fourg63::triggerPrimary(3000, current, decoderState, page2, page4);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(5, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(600, decoderState.toothOneMinusOneTime);
    TEST_ASSERT_EQUAL_UINT32(3000, decoderState.toothOneTime);
}

static void test_primary_medium_and_aggressive_filter_modes(void)
{
    assert_synced_filter(4, TRIGGER_FILTER_MEDIUM, 2, 70, 1000);
    assert_synced_filter(4, TRIGGER_FILTER_MEDIUM, 1, 110, 400);
    assert_synced_filter(6, TRIGGER_FILTER_MEDIUM, 2, 70, 400);
    assert_synced_filter(6, TRIGGER_FILTER_MEDIUM, 1, 50, 600);
    assert_synced_filter(4, TRIGGER_FILTER_AGGRESSIVE, 2, 70, 1100);
    assert_synced_filter(4, TRIGGER_FILTER_AGGRESSIVE, 1, 110, 225);
    assert_synced_filter(6, TRIGGER_FILTER_AGGRESSIVE, 2, 70, 400);
    assert_synced_filter(6, TRIGGER_FILTER_AGGRESSIVE, 1, 50, 800);
}

static void test_primary_lite_filter_70_degree_tooth(void)
{
    assert_synced_filter(4, TRIGGER_FILTER_LITE, 2, 70, 800);
    assert_synced_filter(4, TRIGGER_FILTER_MEDIUM, 2, 70, 800, 1399);
    assert_synced_filter(4, TRIGGER_FILTER_AGGRESSIVE, 2, 70, 800, 1399);
    assert_synced_filter(6, TRIGGER_FILTER_LITE, 2, 70, 200);
    assert_synced_filter(6, TRIGGER_FILTER_MEDIUM, 2, 70, 200, 1399);
    assert_synced_filter(6, TRIGGER_FILTER_AGGRESSIVE, 2, 70, 200, 1399);
}

static void test_primary_no_filter_even_tooth_angles(void)
{
    assert_synced_filter(4, TRIGGER_FILTER_OFF, 1, 110, 0);
    assert_synced_filter(6, TRIGGER_FILTER_OFF, 1, 50, 0);
}

static void test_primary_unsynced_pin_combinations(void)
{
    assert_unsynced_pin_state(4, true, true, false, 2, true, SyncStatus::None);
    assert_unsynced_pin_state(4, true, false, true, 2, false, SyncStatus::None);
    assert_unsynced_pin_state(4, false, false, true, 1, true, SyncStatus::None);
    assert_unsynced_pin_state(4, false, true, true, 5, true, SyncStatus::None);
}

static void test_primary_adjusts_per_tooth_ignition(void)
{
    assert_primary_per_tooth_schedule(IGN_MODE_WASTED, 1);
    assert_primary_per_tooth_schedule(IGN_MODE_SEQUENTIAL, 5);
}

static void test_secondary_rejects_filtered_pulse(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    decoderState.toothLastSecToothTime = 1000;
    decoderState.triggerSecFilterTime = 100;

    decoders::fourg63::triggerSecondary(1050, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT32(50, decoderState.curGap2);
    TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(100, decoderState.triggerSecFilterTime);
}

static void test_secondary_accepts_pulse_and_updates_filters(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    decoderState.toothLastSecToothTime = 1000;
    decoderState.triggerSecFilterTime = 0;

    decoders::fourg63::triggerSecondary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT32(400, decoderState.curGap2);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(100, decoderState.triggerSecFilterTime);
    TEST_ASSERT_EQUAL_UINT32(1500, decoderState.triggerFilterTime);
}

static void test_secondary_syncs_four_cylinder_at_tooth_five(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = 4;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    decoderState.toothLastSecToothTime = 1000;
    decoderState.toothCurrentCount = 5;
    decoderState.triggerSecFilterTime = 0;
    current.decoder.primary._pin.setPin(2);
    current.decoder.primary._pin._pin.setPinLow();

    decoders::fourg63::triggerSecondary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(100, decoderState.triggerSecFilterTime);
}

static void test_secondary_loses_sync_on_unexpected_resync_pulse(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = 4;
    page4.useResync = 1;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.toothCurrentCount = 3;
    decoderState.toothLastSecToothTime = 1000;
    decoderState.triggerSecFilterTime = 0;
    current.setRpm(3000);
    current.decoder.primary._pin.setPin(2);
    current.decoder.primary._pin._pin.setPinHigh();

    decoders::fourg63::triggerSecondary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT8(1, current.syncLossCounter);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastSecToothTime);
}

static void test_secondary_sync_acquisition_uses_primary_level_and_cylinder_count(void)
{
    assert_secondary_primary_pin_sync(4, 8, true, SyncStatus::Full);
    assert_secondary_primary_pin_sync(6, 7, true, SyncStatus::Full);
    assert_secondary_primary_pin_sync(4, 7, true, SyncStatus::None);
}

static void test_secondary_keeps_full_sync_when_resync_is_disabled(void)
{
    statuses current{};
    config2 page2{};
    config4 page4{};
    page2.nCylinders = 4;
    auto decoderState = decoders::fourg63::initialise(0, current, page2);
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.toothCurrentCount = 3;
    decoderState.toothLastSecToothTime = 1000;
    decoderState.triggerSecFilterTime = 0;
    current.setRpm(3000);
    current.decoder.primary._pin.setPin(2);
    current.decoder.primary._pin._pin.setPinHigh();

    decoders::fourg63::triggerSecondary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT8(0, current.syncLossCounter);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
}

static void test_is70DegTooth(void)
{
    for (auto toothNum: { 1, 3, 5, 7, 9, 11} )
    {
        TEST_ASSERT_TRUE(decoders::fourg63::is70DegTooth(toothNum));
    }
    for (auto toothNum: { 0, 2, 4, 6, 8, 10, 12} )
    {
        TEST_ASSERT_FALSE(decoders::fourg63::is70DegTooth(toothNum));
    }    
}
void test4G63Triggers(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_primary_accepts_startup_pulse_before_filter);
    RUN_TEST_P(test_primary_rejects_filtered_pulse);
    RUN_TEST_P(test_primary_syncs_six_cylinder_on_secondary_edge);
    RUN_TEST_P(test_primary_updates_synced_tooth_filter);
    RUN_TEST_P(test_primary_wraps_tooth_count_and_updates_tooth_one_time);
    RUN_TEST_P(test_primary_medium_and_aggressive_filter_modes);
    RUN_TEST_P(test_primary_lite_filter_70_degree_tooth);
    RUN_TEST_P(test_primary_no_filter_even_tooth_angles);
    RUN_TEST_P(test_primary_unsynced_pin_combinations);
    RUN_TEST_P(test_primary_adjusts_per_tooth_ignition);
    RUN_TEST_P(test_secondary_rejects_filtered_pulse);
    RUN_TEST_P(test_secondary_accepts_pulse_and_updates_filters);
    RUN_TEST_P(test_secondary_syncs_four_cylinder_at_tooth_five);
    RUN_TEST_P(test_secondary_loses_sync_on_unexpected_resync_pulse);
    RUN_TEST_P(test_secondary_sync_acquisition_uses_primary_level_and_cylinder_count);
    RUN_TEST_P(test_secondary_keeps_full_sync_when_resync_is_disabled);
    RUN_TEST_P(test_is70DegTooth);
}