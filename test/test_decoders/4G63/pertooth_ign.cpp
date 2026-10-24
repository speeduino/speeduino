#include "../test_utils.h"
#include "src/decoders/4g63.h"

static void assert_end_teeth(uint8_t cylinderCount, uint8_t sparkMode, const uint16_t expected[4])
{
    config2 page2{};
    config4 page4{};
    decoders::shared::state_t decoderState{};
    page2.nCylinders = cylinderCount;
    page4.sparkMode = sparkMode;

    decoders::fourg63::setEndTeeth(decoderState, page2, page4);

    for(size_t index = 0; index < 4; ++index)
    {
        TEST_ASSERT_EQUAL_UINT16(expected[index], decoderState.ignitionEndTeeth[index]);
    }
}

static void test_four_cylinder_wasted_spark_end_teeth(void)
{
    const uint16_t expected[4] = {4, 2, 4, 2};
    assert_end_teeth(4, IGN_MODE_WASTED, expected);
}

static void test_four_cylinder_sequential_end_teeth(void)
{
    const uint16_t expected[4] = {8, 2, 4, 6};
    assert_end_teeth(4, IGN_MODE_SEQUENTIAL, expected);
}

static void test_six_cylinder_wasted_spark_end_teeth(void)
{
    const uint16_t expected[4] = {6, 2, 4, 2};
    assert_end_teeth(6, IGN_MODE_WASTED, expected);
}

static void test_six_cylinder_sequential_end_teeth(void)
{
    const uint16_t expected[4] = {8, 2, 4, 6};
    assert_end_teeth(6, IGN_MODE_SEQUENTIAL, expected);
}

void test4G63PerToothIgn(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_four_cylinder_wasted_spark_end_teeth);
    RUN_TEST_P(test_four_cylinder_sequential_end_teeth);
    RUN_TEST_P(test_six_cylinder_wasted_spark_end_teeth);
    RUN_TEST_P(test_six_cylinder_sequential_end_teeth);
}