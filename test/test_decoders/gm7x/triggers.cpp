#include "../../test_utils.h"
#include "src/decoders/gm7x.h"
#include "scheduler_ignition_controller.h"

static void test_primary_initializes_first_tooth(void)
{
    statuses current{};
    auto decoderState = decoders::gm7x::initialise();
    config2 page2{};
    config4 page4{};

    decoders::gm7x::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(0, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
    TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
}

static void test_primary_counts_regular_tooth(void)
{
    statuses current{};
    auto decoderState = decoders::gm7x::initialise();
    config2 page2{};
    config4 page4{};

    decoderState.curGap = 200;
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 1;

    decoders::gm7x::triggerPrimary(1300, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(100, decoderState.targetGap);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.toothAngleIsCorrect);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1300, decoderState.toothLastToothTime);
    TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
}

static void test_primary_syncs_on_short_reference_gap(void)
{
    statuses current{};
    auto decoderState = decoders::gm7x::initialise();
    config2 page2{};
    config4 page4{};

    decoderState.curGap = 400;
    decoderState.toothLastMinusOneToothTime = 1000;
    decoderState.toothLastToothTime = 1500;
    decoderState.toothCurrentCount = 2;

    decoders::gm7x::triggerPrimary(1600, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT32(100, decoderState.curGap);
    TEST_ASSERT_EQUAL_UINT32(200, decoderState.targetGap);
    TEST_ASSERT_EQUAL_UINT16(3, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_FALSE(decoderState.decoderStatus.toothAngleIsCorrect);
    TEST_ASSERT_EQUAL_UINT32(1, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(1500, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1600, decoderState.toothLastToothTime);
}

static void test_primary_wraps_after_seventh_tooth(void)
{
    statuses current{};
    auto decoderState = decoders::gm7x::initialise();
    config2 page2{};
    config4 page4{};

    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 7;
    decoderState.toothOneMinusOneTime = 200;
    decoderState.toothOneTime = 600;
    decoderState.decoderStatus.toothAngleIsCorrect = false;

    decoders::gm7x::triggerPrimary(1400, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(600, decoderState.toothOneMinusOneTime);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothOneTime);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.toothAngleIsCorrect);
    TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastToothTime);
}

static void test_primary_skips_per_tooth_adjustment_on_reference_tooth(void)
{
    statuses current{};
    auto decoderState = decoders::gm7x::initialise();
    config2 page2{};
    config4 page4{};

    page2.perToothIgn = true;
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 2;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 500;

    decoders::gm7x::triggerPrimary(1300, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(3, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);
}

static void test_primary_adjusts_per_tooth_timing_for_regular_tooth(void)
{
    statuses current{};
    auto decoderState = decoders::gm7x::initialise();
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 2000;
    current.setRpm(3000);
    page2.perToothIgn = true;
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 3;
    decoderState.ignitionEndTeeth[0] = 4;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 500;

    decoders::gm7x::triggerPrimary(1300, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(4, decoderState.toothCurrentCount);
    TEST_ASSERT_NOT_EQUAL(100, ignitionSchedule1._compare);
}

static void test_primary_uses_early_tooth_per_tooth_angle(void)
{
    statuses current{};
    auto decoderState = decoders::gm7x::initialise();
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 2000;
    current.setRpm(3000);
    page2.perToothIgn = true;
    page4.triggerAngle = 10;
    decoderState.curGap = 200;
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    decoderState.toothCurrentCount = 1;
    decoderState.ignitionEndTeeth[0] = 2;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 200;

    decoders::gm7x::triggerPrimary(1300, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(100 + angleToTimerTicks(200 - (60 + 42 + 10)), ignitionSchedule1._compare);
}

void testGM7XTriggers(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_primary_initializes_first_tooth);
    RUN_TEST_P(test_primary_counts_regular_tooth);
    RUN_TEST_P(test_primary_syncs_on_short_reference_gap);
    RUN_TEST_P(test_primary_wraps_after_seventh_tooth);
    RUN_TEST_P(test_primary_skips_per_tooth_adjustment_on_reference_tooth);
    RUN_TEST_P(test_primary_adjusts_per_tooth_timing_for_regular_tooth);
    RUN_TEST_P(test_primary_uses_early_tooth_per_tooth_angle);
}