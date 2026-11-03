#include "../test_utils.h"
#include "src/decoders/4g63.h"

static void test_initialises_four_cylinder_layout(void)
{
    statuses current{};
    config2 page2{};
    page2.nCylinders = 4;

    const int16_t expectedToothAngles[8] = {715, 105, 175, 285, 355, 465, 535, 645};
    auto decoderState = decoders::fourg63::initialise(1234, current, page2);

    TEST_ASSERT_EQUAL_UINT16(8, decoderState.triggerActualTeeth);
    TEST_ASSERT_EQUAL_UINT16(180, decoderState.triggerToothAngle);
    TEST_ASSERT_EQUAL_UINT32(366667, decoderState.MAX_STALL_TIME);
    TEST_ASSERT_EQUAL_UINT32(1500, decoderState.triggerFilterTime);
    TEST_ASSERT_EQUAL_UINT32(0, decoderState.toothLastToothTime);
    TEST_ASSERT_TRUE(decoderState.decoderFeatures.supportsSequential);
    TEST_ASSERT_TRUE(decoderState.decoderFeatures.hasFixedCrankingTiming);
    TEST_ASSERT_TRUE(decoderState.decoderFeatures.supportsPerToothIgnition);

    for(size_t index = 0; index < 8; ++index)
    {
        TEST_ASSERT_EQUAL_INT16(expectedToothAngles[index], decoderState.toothAngles[index]);
    }
}

static void test_initialises_six_cylinder_layout_and_startup_timestamp(void)
{
    statuses current{};
    config2 page2{};
    page2.nCylinders = 6;
    current.initialisationComplete = true;

    const int16_t expectedToothAngles[12] = {715, 45, 115, 165, 235, 285, 355, 405, 475, 525, 595, 645};
    auto decoderState = decoders::fourg63::initialise(1234, current, page2);

    TEST_ASSERT_EQUAL_UINT16(12, decoderState.triggerActualTeeth);
    TEST_ASSERT_EQUAL_UINT16(180, decoderState.triggerToothAngle);
    TEST_ASSERT_EQUAL_UINT32(366667, decoderState.MAX_STALL_TIME);
    TEST_ASSERT_EQUAL_UINT32(1500, decoderState.triggerFilterTime);
    TEST_ASSERT_EQUAL_UINT32(1234, decoderState.toothLastToothTime);

    for(size_t index = 0; index < 12; ++index)
    {
        TEST_ASSERT_EQUAL_INT16(expectedToothAngles[index], decoderState.toothAngles[index]);
    }
}

void test4G63Initialise(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_initialises_four_cylinder_layout);
    RUN_TEST_P(test_initialises_six_cylinder_layout_and_startup_timestamp);
}