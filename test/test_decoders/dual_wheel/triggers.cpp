#include "../../test_utils.h"
#include "src/decoders/dualWheel.h"
#include "scheduler_ignition_controller.h"

using namespace decoders::shared;

static void test_primary_rejects_filtered_trigger(void)
{
    statuses current;
    state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 3;
    decoderState.triggerFilterTime = 100;

    decoders::dual_wheel::triggerPrimary(950, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT32(50, decoderState.curGap);
    TEST_ASSERT_EQUAL_UINT16(3, decoderState.toothCurrentCount);
    TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(900, decoderState.toothLastToothTime);
}

static void test_primary_accepts_trigger_and_updates_timestamps(void)
{
    statuses current;
    state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 800;
    decoderState.toothLastToothTime = 900;

    decoders::dual_wheel::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(900, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
}

static void test_primary_wraps_tooth_count_and_tracks_cam_revolutions(void)
{
    statuses current;
    config2 page2{};
    config4 page4{};

    page4.triggerTeeth = 4;
    page4.TrigSpeed = CAM_SPEED;
    state_t decoderState = decoders::dual_wheel::initialise(page4);
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = page4.triggerTeeth;
    decoderState.toothOneMinusOneTime = 600;
    decoderState.toothOneTime = 700;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.triggerFilterTime = 400;

    decoders::dual_wheel::triggerPrimary(1500, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
    TEST_ASSERT_EQUAL_UINT32(2, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(700, decoderState.toothOneMinusOneTime);
    TEST_ASSERT_EQUAL_UINT32(1500, decoderState.toothOneTime);
}

static void test_primary_wraps_tooth_count_and_tracks_crank_revolutions(void)
{
    statuses current;
    config2 page2{};
    config4 page4{};

    page4.triggerTeeth = 4;
    page4.TrigSpeed = CRANK_SPEED;
    state_t decoderState = decoders::dual_wheel::initialise(page4);
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = page4.triggerTeeth;
    decoderState.toothOneMinusOneTime = 600;
    decoderState.toothOneTime = 700;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.triggerFilterTime = 400;

    decoders::dual_wheel::triggerPrimary(1500, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
    TEST_ASSERT_EQUAL_UINT32(1, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(700, decoderState.toothOneMinusOneTime);
    TEST_ASSERT_EQUAL_UINT32(1500, decoderState.toothOneTime);
}

static void test_primary_adjusts_pending_ignition_on_matching_tooth(void)
{
    statuses current;
    state_t decoderState{};
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 20;
    page2.perToothIgn = true;
    page4.triggerAngle = 10;
    page4.triggerTeeth = 12;
    page4.sparkMode = IGN_MODE_WASTED;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 1;
    decoderState.triggerToothAngle = 30;
    decoderState.ignitionEndTeeth[0] = 2;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 180;

    decoders::dual_wheel::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_NOT_EQUAL(100, ignitionSchedule1._compare);
}

static void test_primary_skips_per_tooth_ignition_while_cranking(void)
{
    statuses current;
    state_t decoderState{};
    config2 page2{};
    config4 page4{};

    current.rotationStatus = EngineRotationStatus::Cranking;
    page2.perToothIgn = true;
    page4.triggerTeeth = 12;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 1;
    decoderState.ignitionEndTeeth[0] = 2;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 180;

    decoders::dual_wheel::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);
}

static void test_primary_sequential_second_revolution_uses_offset_tooth_and_angle(void)
{
    statuses current;
    state_t decoderState{};
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 20;
    CRANK_ANGLE_MAX_IGN = 720;
    page2.perToothIgn = true;
    page4.triggerAngle = 10;
    page4.triggerTeeth = 12;
    page4.sparkMode = IGN_MODE_SEQUENTIAL;
    page4.TrigSpeed = CRANK_SPEED;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 1;
    decoderState.triggerToothAngle = 30;
    decoderState.revolutionOne = true;
    decoderState.ignitionEndTeeth[0] = 14;

    stopIgnitionSchedulers();
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 500;

    decoders::dual_wheel::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
    TEST_ASSERT_UINT32_WITHIN(5, ignitionSchedule1._counter + angleToTimerTicks(100), ignitionSchedule1._compare);
}

static void test_primary_sequential_first_revolution_uses_unoffset_tooth_and_angle(void)
{
    statuses current;
    state_t decoderState{};
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 20;
    CRANK_ANGLE_MAX_IGN = 720;
    page2.perToothIgn = true;
    page4.triggerAngle = 10;
    page4.triggerTeeth = 12;
    page4.sparkMode = IGN_MODE_SEQUENTIAL;
    page4.TrigSpeed = CRANK_SPEED;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 1;
    decoderState.triggerToothAngle = 30;
    decoderState.revolutionOne = false;
    decoderState.ignitionEndTeeth[0] = 2;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 180;

    decoders::dual_wheel::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_FALSE(decoderState.revolutionOne);
    TEST_ASSERT_UINT32_WITHIN(5, ignitionSchedule1._counter + angleToTimerTicks(140), ignitionSchedule1._compare);
}

static void test_secondary_rejects_filtered_trigger_and_updates_filter(void)
{
    statuses current;
    state_t decoderState{};
    config4 page4{};

    current.revolutionTime = 6000;
    decoderState.toothLastSecToothTime = 1000;
    decoderState.triggerSecFilterTime = 100;
    decoderState.toothCurrentCount = 2;

    decoders::dual_wheel::triggerSecondary(1050, current, decoderState, page4);

    TEST_ASSERT_EQUAL_UINT32(50, decoderState.curGap2);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(3000, decoderState.triggerSecFilterTime);
    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
}

static void test_secondary_initializes_sync_on_accepted_trigger(void)
{
    statuses current;
    state_t decoderState{};
    config4 page4{};

    page4.triggerTeeth = 4;
    decoderState.toothLastSecToothTime = 100000;
    decoderState.triggerSecFilterTime = 100;

    decoders::dual_wheel::triggerSecondary(2000000, current, decoderState, page4);

    TEST_ASSERT_EQUAL_UINT32(1900000, decoderState.curGap2);
    TEST_ASSERT_EQUAL_UINT32(2000000, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(475000, decoderState.triggerSecFilterTime);
    TEST_ASSERT_EQUAL_UINT32(2000000, decoderState.toothLastToothTime);
    TEST_ASSERT_EQUAL_UINT32(500000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT16(4, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(0, decoderState.triggerFilterTime);
    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_secondary_counts_sync_loss_and_resynchronizes(void)
{
    statuses current;
    state_t decoderState{};
    config4 page4{};

    page4.triggerTeeth = 4;
    page4.useResync = 1;
    decoderState.toothLastSecToothTime = 1000;
    decoderState.triggerSecFilterTime = 100;
    decoderState.toothCurrentCount = 2;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    current.startRevolutions = 3;

    decoders::dual_wheel::triggerSecondary(1400, current, decoderState, page4);

    TEST_ASSERT_EQUAL_UINT32(400, decoderState.curGap2);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(1, current.syncLossCounter);
    TEST_ASSERT_EQUAL_UINT16(4, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

void testDualWheelTriggers(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_primary_rejects_filtered_trigger);
    RUN_TEST_P(test_primary_accepts_trigger_and_updates_timestamps);
    RUN_TEST_P(test_primary_wraps_tooth_count_and_tracks_cam_revolutions);
    RUN_TEST_P(test_primary_wraps_tooth_count_and_tracks_crank_revolutions);
    RUN_TEST_P(test_primary_adjusts_pending_ignition_on_matching_tooth);
    RUN_TEST_P(test_primary_skips_per_tooth_ignition_while_cranking);
    RUN_TEST_P(test_primary_sequential_second_revolution_uses_offset_tooth_and_angle);
    RUN_TEST_P(test_primary_sequential_first_revolution_uses_unoffset_tooth_and_angle);
    RUN_TEST_P(test_secondary_rejects_filtered_trigger_and_updates_filter);
    RUN_TEST_P(test_secondary_initializes_sync_on_accepted_trigger);
    RUN_TEST_P(test_secondary_counts_sync_loss_and_resynchronizes);
}