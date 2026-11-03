#include "../test_utils.h"
#include "src/decoders/gm24x.h"

static void test_primary_waits_for_first_revolution_at_startup(void)
{
    statuses current{};
    auto decoderState = decoders::gm24x::initialise(1234, current);

    decoders::gm24x::triggerPrimary(2000, current, decoderState);

    TEST_ASSERT_EQUAL_UINT16(25, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT32(1234, decoderState.toothLastToothTime);
    TEST_ASSERT_EQUAL_UINT32(0, current.startRevolutions);
}

static void test_primary_synchronizes_on_first_tooth(void)
{
    statuses current{};
    current.initialisationComplete = true;
    auto decoderState = decoders::gm24x::initialise(1000, current);
    decoderState.toothLastToothTime = 1000;

    decoders::gm24x::triggerPrimary(1200, current, decoderState);

    TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
    TEST_ASSERT_EQUAL_UINT32(1, current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(1200, decoderState.toothOneTime);
    TEST_ASSERT_EQUAL_UINT32(1200, decoderState.toothLastToothTime);
    TEST_ASSERT_EQUAL_UINT16(15, decoderState.triggerToothAngle);
}

static void test_primary_calculates_variable_tooth_angles(void)
{
    statuses current{};
    current.initialisationComplete = true;
    auto decoderState = decoders::gm24x::initialise(0, current);
    decoderState.toothCurrentCount = 1;
    decoderState.toothLastToothTime = 1000;
    const uint16_t expectedToothAngles[] = {6, 15, 15, 15, 15, 24, 6, 15, 15, 24, 15, 6, 15, 24, 15, 15, 6, 24, 6, 24, 15, 15, 15};

    for(size_t index = 0; index < sizeof(expectedToothAngles) / sizeof(expectedToothAngles[0]); ++index)
    {
        const uint32_t triggerTime = decoderState.toothLastToothTime + 100;
        decoders::gm24x::triggerPrimary(triggerTime, current, decoderState);

        TEST_ASSERT_EQUAL_UINT16(index + 2, decoderState.toothCurrentCount);
        TEST_ASSERT_EQUAL_UINT16(expectedToothAngles[index], decoderState.triggerToothAngle);
        TEST_ASSERT_EQUAL_UINT32(triggerTime, decoderState.toothLastToothTime);
    }
}

static void test_secondaryTrigger(void)
{
    statuses current{};
    current.initialisationComplete = true;
    auto decoderState = decoders::gm24x::initialise(0, current);

    decoderState.toothCurrentCount = 99;
    decoderState.revolutionOne = false;

    decoders::gm24x::triggerSecondary(decoderState);
   
    TEST_ASSERT_EQUAL(0, decoderState.toothCurrentCount);
    TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_initialise(void)
{
    {
        statuses current{};
        current.initialisationComplete = true;
        auto subject = decoders::gm24x::initialise(5000, current);

        TEST_ASSERT_EQUAL(0, subject.toothCurrentCount);
        TEST_ASSERT_EQUAL(0, subject.toothLastToothTime);
    }

    {
        statuses current{};
        current.initialisationComplete = false;
        auto subject = decoders::gm24x::initialise(5000, current);

        TEST_ASSERT_EQUAL(25, subject.toothCurrentCount);
        TEST_ASSERT_EQUAL(5000, subject.toothLastToothTime);
    }
}

void test24X(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_primary_waits_for_first_revolution_at_startup);
    RUN_TEST_P(test_primary_synchronizes_on_first_tooth);
    RUN_TEST_P(test_primary_calculates_variable_tooth_angles);
    RUN_TEST_P(test_secondaryTrigger);
    RUN_TEST_P(test_initialise);
}