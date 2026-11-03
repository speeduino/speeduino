#include "../../test_utils.h"
#include "src/decoders/basicDistributor.h"
#include "scheduler_ignition_controller.h"

using namespace decoders::basic_distributor;

static void test_primary_rejects_filtered_trigger(void)
{
    statuses current{};
    decoders::shared::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastToothTime = 900;
    decoderState.triggerFilterTime = 100;

    triggerPrimary(950, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT32(50, decoderState.curGap);
    TEST_ASSERT_EQUAL_UINT16(0, decoderState.toothCurrentCount);
    TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(900, decoderState.toothLastToothTime);
    TEST_ASSERT_EQUAL_UINT32(0, current.startRevolutions);
}

static void test_primary_initializes_sync_on_first_trigger(void)
{
    statuses current{};
    decoders::shared::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(1, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(0, decoderState.toothOneMinusOneTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothOneTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
}

static void test_primary_counts_teeth_and_updates_filter(void)
{
    statuses current{};
    decoders::shared::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 1;
    decoderState.triggerActualTeeth = 4;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    page4.triggerFilter = TRIGGER_FILTER_LITE;

    triggerPrimary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(100, decoderState.triggerFilterTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastToothTime);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(0, current.startRevolutions);
}

static void test_primary_wraps_tooth_count_at_revolution_boundary(void)
{
    statuses current{};
    decoders::shared::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 4;
    decoderState.triggerActualTeeth = 4;
    decoderState.toothOneMinusOneTime = 300;
    decoderState.toothOneTime = 600;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;

    triggerPrimary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(1, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(600, decoderState.toothOneMinusOneTime);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothOneTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastToothTime);
}

static void test_primary_loses_sync_when_tooth_count_exceeds_limit(void)
{
    statuses current{};
    decoders::shared::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 800;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 5;
    decoderState.triggerActualTeeth = 4;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;

    triggerPrimary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(5, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT8(1, current.syncLossCounter);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastToothTime);
}

static void test_primary_applies_per_tooth_timing_for_normalized_tooth(void)
{
    statuses current{};
    decoders::shared::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 2000;
    current.setRpm(3000);
    page2.perToothIgn = true;
    page4.triggerAngle = 10;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 2;
    decoderState.triggerActualTeeth = 4;
    decoderState.triggerToothAngle = 180;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.ignitionEndTeeth[0] = 1;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 500;

    triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(3, decoderState.toothCurrentCount);
    TEST_ASSERT_NOT_EQUAL(100, ignitionSchedule1._compare);
}

static void test_primary_skips_per_tooth_timing_when_disabled(void)
{
    statuses current{};
    decoders::shared::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 2000;
    current.setRpm(3000);
    page4.triggerAngle = 10;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 2;
    decoderState.triggerActualTeeth = 4;
    decoderState.triggerToothAngle = 180;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.ignitionEndTeeth[0] = 1;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 500;

    triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(3, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);
}


void testBasicDistribTriggers(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_primary_rejects_filtered_trigger);
    RUN_TEST_P(test_primary_initializes_sync_on_first_trigger);
    RUN_TEST_P(test_primary_counts_teeth_and_updates_filter);
    RUN_TEST_P(test_primary_wraps_tooth_count_at_revolution_boundary);
    RUN_TEST_P(test_primary_loses_sync_when_tooth_count_exceeds_limit);
    RUN_TEST_P(test_primary_applies_per_tooth_timing_for_normalized_tooth);
    RUN_TEST_P(test_primary_skips_per_tooth_timing_when_disabled);
}