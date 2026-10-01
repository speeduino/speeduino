#include "../../test_utils.h"
#include "src/decoders/missingTooth.h"
#include "scheduler_ignition_controller.h"

static int16_t test_crank_angle(uint32_t curTime)
{
    (void)curTime;
    return 90;
}

static void test_primary_rejects_filtered_trigger(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 800;
    decoderState.toothLastToothTime = 900;
    decoderState.triggerFilterTime = 100;

    decoders::missing_tooth::triggerPrimary(950, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT32(50, decoderState.curGap);
    TEST_ASSERT_EQUAL_UINT16(0, decoderState.toothCurrentCount);
    TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(900, decoderState.toothLastToothTime);
}

static void test_primary_initializes_tooth_timestamps(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoders::missing_tooth::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(0, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
}

static void test_primary_updates_regular_tooth(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 5;
    decoderState.triggerActualTeeth = 36;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    current.setRpm(3000);

    decoders::missing_tooth::triggerPrimary(1500, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(6, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1500, decoderState.toothLastToothTime);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.toothAngleIsCorrect);
}

static void test_primary_syncs_on_missing_tooth_gap(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 5;
    decoderState.triggerActualTeeth = 36;
    decoderState.decoderStatus.syncStatus = SyncStatus::None;

    decoders::missing_tooth::triggerPrimary(2000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
    TEST_ASSERT_FALSE(decoderState.decoderStatus.toothAngleIsCorrect);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(2000, decoderState.toothLastToothTime);
}

static void test_primary_loses_sync_on_early_gap(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 5;
    decoderState.triggerActualTeeth = 36;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;

    decoders::missing_tooth::triggerPrimary(2000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(6, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT8(1, current.syncLossCounter);
    TEST_ASSERT_EQUAL_UINT32(500, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
}

static void test_primary_does_not_detect_gap_at_threshold(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 800;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 4;
    decoderState.triggerActualTeeth = 36;
    decoderState.decoderStatus.syncStatus = SyncStatus::None;
    page4.triggerMissingTeeth = 2;

    decoders::missing_tooth::triggerPrimary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT32(400, decoderState.curGap);
    TEST_ASSERT_EQUAL_UINT32(400, decoderState.targetGap);
    TEST_ASSERT_EQUAL_UINT16(5, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastToothTime);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.toothAngleIsCorrect);
}

static void test_primary_recovers_after_tooth_count_overflow_at_cam_speed(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 800;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 36;
    decoderState.triggerActualTeeth = 36;
    decoderState.toothOneMinusOneTime = 600;
    decoderState.toothOneTime = 700;
    decoderState.secondaryToothCount = 3;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    page4.triggerMissingTeeth = 2;
    page4.trigPatternSec = SEC_TRIGGER_SINGLE;
    page4.TrigSpeed = CAM_SPEED;
    page4.sparkMode = IGN_MODE_WASTED;

    decoders::missing_tooth::triggerPrimary(1100, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(2, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(700, decoderState.toothOneMinusOneTime);
    TEST_ASSERT_EQUAL_UINT32(1100, decoderState.toothOneTime);
    TEST_ASSERT_EQUAL_UINT16(0, decoderState.secondaryToothCount);
    TEST_ASSERT_EQUAL_UINT32(0, decoderState.triggerFilterTime);
    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_FALSE(decoderState.decoderStatus.toothAngleIsCorrect);
}

static void test_primary_sequential_sync_waits_for_secondary_trigger(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 5;
    decoderState.triggerActualTeeth = 36;
    decoderState.decoderStatus.syncStatus = SyncStatus::None;
    page2.strokes = FOUR_STROKE;
    page4.triggerMissingTeeth = 1;
    page4.sparkMode = IGN_MODE_SEQUENTIAL;
    page4.TrigSpeed = CRANK_SPEED;

    decoders::missing_tooth::triggerPrimary(2000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(SyncStatus::Partial, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
}

static void test_primary_sequential_sync_completes_after_secondary_trigger(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 5;
    decoderState.triggerActualTeeth = 36;
    decoderState.secondaryToothCount = 1;
    decoderState.decoderStatus.syncStatus = SyncStatus::None;
    page2.strokes = FOUR_STROKE;
    page4.triggerMissingTeeth = 1;
    page4.sparkMode = IGN_MODE_SEQUENTIAL;
    page4.TrigSpeed = CRANK_SPEED;

    decoders::missing_tooth::triggerPrimary(2000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
}

static void test_primary_per_tooth_ignition_adjusts_timing(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 2000;
    current.setRpm(3000);
    page2.perToothIgn = true;
    page2.strokes = FOUR_STROKE;
    page4.triggerAngle = 10;
    page4.triggerTeeth = 36;
    page4.sparkMode = IGN_MODE_WASTED;
    decoderState.toothLastMinusOneToothTime = 800;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 2;
    decoderState.triggerActualTeeth = 36;
    decoderState.triggerToothAngle = 10;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.ignitionEndTeeth[0] = 3;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 9999;

    decoders::missing_tooth::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_NOT_EQUAL(100, ignitionSchedule1._compare);
}

static void test_primary_skips_per_tooth_ignition_while_cranking(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Cranking;
    current.startRevolutions = 2000;
    current.setRpm(500);
    page2.perToothIgn = true;
    page4.triggerAngle = 10;
    decoderState.toothLastMinusOneToothTime = 800;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 2;
    decoderState.triggerActualTeeth = 36;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.ignitionEndTeeth[0] = 3;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 500;

    decoders::missing_tooth::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);
}

static void test_secondary_rejects_filtered_trigger(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};
    config10 page10{};

    decoderState.toothLastSecToothTime = 1000;
    decoderState.triggerSecFilterTime = 100;
    page4.trigPatternSec = SEC_TRIGGER_SINGLE;

    decoders::missing_tooth::triggerSecondary(1050, current, decoderState, page4, page6, page10);

    TEST_ASSERT_EQUAL_UINT32(50, decoderState.curGap2);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT16(0, decoderState.secondaryToothCount);
}

static void test_secondary_initializes_single_tooth_trigger(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};
    config10 page10{};

    page4.trigPatternSec = SEC_TRIGGER_SINGLE;

    decoders::missing_tooth::triggerSecondary(1200, current, decoderState, page4, page6, page10);

    TEST_ASSERT_EQUAL_UINT32(0, decoderState.curGap2);
    TEST_ASSERT_EQUAL_UINT32(1200, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.secondaryToothCount);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_secondary_poll_updates_filter_without_resetting_revolution(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};
    config10 page10{};

    decoderState.toothLastSecToothTime = 1000;
    decoderState.secondaryToothCount = 3;
    page4.trigPatternSec = SEC_TRIGGER_POLL;

    decoders::missing_tooth::triggerSecondary(1400, current, decoderState, page4, page6, page10);

    TEST_ASSERT_EQUAL_UINT32(400, decoderState.curGap2);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(200, decoderState.triggerSecFilterTime);
    TEST_ASSERT_EQUAL_UINT16(3, decoderState.secondaryToothCount);
    TEST_ASSERT_FALSE(decoderState.revolutionOne);
}

static void test_secondary_4_1_increments_on_regular_tooth(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};
    config10 page10{};

    decoderState.toothLastMinusOneSecToothTime = 800;
    decoderState.toothLastSecToothTime = 1000;
    decoderState.secondaryToothCount = 2;
    page4.trigPatternSec = SEC_TRIGGER_4_1;

    decoders::missing_tooth::triggerSecondary(1250, current, decoderState, page4, page6, page10);

    TEST_ASSERT_EQUAL_UINT32(300, decoderState.targetGap2);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(1250, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(62, decoderState.triggerSecFilterTime);
    TEST_ASSERT_EQUAL_UINT16(3, decoderState.secondaryToothCount);
    TEST_ASSERT_FALSE(decoderState.revolutionOne);
}

static void test_secondary_4_1_detects_missing_gap(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};
    config10 page10{};

    decoderState.toothLastMinusOneSecToothTime = 800;
    decoderState.toothLastSecToothTime = 1000;
    decoderState.secondaryToothCount = 2;
    page4.trigPatternSec = SEC_TRIGGER_4_1;

    decoders::missing_tooth::triggerSecondary(1300, current, decoderState, page4, page6, page10);

    TEST_ASSERT_EQUAL_UINT32(300, decoderState.targetGap2);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(1300, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT32(0, decoderState.triggerSecFilterTime);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.secondaryToothCount);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_secondary_toyota_3_resets_revolution_on_second_tooth(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};
    config10 page10{};

    decoderState.toothLastSecToothTime = 1000;
    decoderState.secondaryToothCount = 1;
    page4.trigPatternSec = SEC_TRIGGER_TOYOTA_3;

    decoders::missing_tooth::triggerSecondary(1400, current, decoderState, page4, page6, page10);

    TEST_ASSERT_EQUAL_UINT32(100, decoderState.triggerSecFilterTime);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT16(2, decoderState.secondaryToothCount);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_secondary_4_1_recovers_after_excess_tooth_count(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};
    config10 page10{};

    decoderState.toothLastMinusOneSecToothTime = 800;
    decoderState.toothLastSecToothTime = 1000;
    decoderState.secondaryToothCount = 4;
    page4.trigPatternSec = SEC_TRIGGER_4_1;

    decoders::missing_tooth::triggerSecondary(1050, current, decoderState, page4, page6, page10);

    TEST_ASSERT_EQUAL_UINT32(300, decoderState.targetGap2);
    TEST_ASSERT_EQUAL_UINT32(1050, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.secondaryToothCount);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_secondary_toyota_3_keeps_first_tooth_in_current_revolution(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};
    config10 page10{};

    decoderState.toothLastSecToothTime = 1000;
    decoderState.secondaryToothCount = 0;
    page4.trigPatternSec = SEC_TRIGGER_TOYOTA_3;

    decoders::missing_tooth::triggerSecondary(1400, current, decoderState, page4, page6, page10);

    TEST_ASSERT_EQUAL_UINT32(100, decoderState.triggerSecFilterTime);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastSecToothTime);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.secondaryToothCount);
    TEST_ASSERT_FALSE(decoderState.revolutionOne);
}

static void test_secondary_records_vvt1_angle_when_enabled(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};
    config10 page10{};

    current.decoder.pGetCrankAngle = &test_crank_angle;
    decoderState.toothLastSecToothTime = 1000;
    decoderState.revolutionOne = true;
    page4.trigPatternSec = SEC_TRIGGER_SINGLE;
    page4.triggerAngle = 30;
    page4.ANGLEFILTER_VVT = 0;
    page6.vvtEnabled = 1;

    decoders::missing_tooth::triggerSecondary(1400, current, decoderState, page4, page6, page10);

    TEST_ASSERT_EQUAL_INT16(120, current.vvt1.angle);
}

static void test_tertiary_rejects_filtered_trigger(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};

    decoderState.toothLastThirdToothTime = 1000;
    decoderState.triggerThirdFilterTime = 100;

    decoders::missing_tooth::triggerTertiary(1050, current, decoderState, page4, page6);

    TEST_ASSERT_EQUAL_UINT32(50, decoderState.curGap3);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastThirdToothTime);
    TEST_ASSERT_EQUAL_UINT32(100, decoderState.triggerThirdFilterTime);
    TEST_ASSERT_EQUAL_INT16(0, current.vvt2.angle);
}

static void test_tertiary_initializes_timestamp_on_startup(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};

    decoders::missing_tooth::triggerTertiary(1200, current, decoderState, page4, page6);

    TEST_ASSERT_EQUAL_UINT32(0, decoderState.curGap3);
    TEST_ASSERT_EQUAL_UINT32(1200, decoderState.toothLastThirdToothTime);
    TEST_ASSERT_EQUAL_UINT32(0, decoderState.triggerThirdFilterTime);
}

static void test_tertiary_updates_filter_and_vvt2_angle(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};
    config6 page6{};

    current.decoder.pGetCrankAngle = &test_crank_angle;
    decoderState.toothLastThirdToothTime = 1000;
    page4.triggerAngle = 30;
    page4.vvt2CL0DutyAng = 10;
    page4.ANGLEFILTER_VVT = 0;
    page6.vvtMode = VVT_MODE_CLOSED_LOOP;

    decoders::missing_tooth::triggerTertiary(1400, current, decoderState, page4, page6);

    TEST_ASSERT_EQUAL_UINT32(400, decoderState.curGap3);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastThirdToothTime);
    TEST_ASSERT_EQUAL_UINT32(100, decoderState.triggerThirdFilterTime);
    TEST_ASSERT_EQUAL_INT16(100, current.vvt2.angle);
}

void testMissingToothTriggers(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_primary_rejects_filtered_trigger);
    RUN_TEST_P(test_primary_initializes_tooth_timestamps);
    RUN_TEST_P(test_primary_updates_regular_tooth);
    RUN_TEST_P(test_primary_syncs_on_missing_tooth_gap);
    RUN_TEST_P(test_primary_loses_sync_on_early_gap);
    RUN_TEST_P(test_primary_does_not_detect_gap_at_threshold);
    RUN_TEST_P(test_primary_recovers_after_tooth_count_overflow_at_cam_speed);
    RUN_TEST_P(test_primary_sequential_sync_waits_for_secondary_trigger);
    RUN_TEST_P(test_primary_sequential_sync_completes_after_secondary_trigger);
    RUN_TEST_P(test_primary_per_tooth_ignition_adjusts_timing);
    RUN_TEST_P(test_primary_skips_per_tooth_ignition_while_cranking);
    RUN_TEST_P(test_secondary_rejects_filtered_trigger);
    RUN_TEST_P(test_secondary_initializes_single_tooth_trigger);
    RUN_TEST_P(test_secondary_poll_updates_filter_without_resetting_revolution);
    RUN_TEST_P(test_secondary_4_1_increments_on_regular_tooth);
    RUN_TEST_P(test_secondary_4_1_detects_missing_gap);
    RUN_TEST_P(test_secondary_toyota_3_resets_revolution_on_second_tooth);
    RUN_TEST_P(test_secondary_4_1_recovers_after_excess_tooth_count);
    RUN_TEST_P(test_secondary_toyota_3_keeps_first_tooth_in_current_revolution);
    RUN_TEST_P(test_secondary_records_vvt1_angle_when_enabled);
    RUN_TEST_P(test_tertiary_rejects_filtered_trigger);
    RUN_TEST_P(test_tertiary_initializes_timestamp_on_startup);
    RUN_TEST_P(test_tertiary_updates_filter_and_vvt2_angle);
}