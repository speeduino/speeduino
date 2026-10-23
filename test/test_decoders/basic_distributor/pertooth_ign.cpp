#include "../../test_utils.h"
#include "src/decoders/basicDistributor.h"
#include "scheduler_ignition_controller.h"

static void assert_end_teeth(uint8_t cylinderCount, int16_t dischargeAngle, const uint16_t expected[], size_t expectedCount)
{
    decoders::shared::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    page2.nCylinders = cylinderCount;
    ignitionSchedule1.dischargeAngle = dischargeAngle;

    decoders::basic_distributor::setEndTeeth(decoderState, page2, page4);

    for(size_t index = 0; index < expectedCount; ++index)
    {
        TEST_ASSERT_EQUAL_UINT16(expected[index], decoderState.ignitionEndTeeth[index]);
    }
}

static void test_set_end_teeth(void)
{
    const uint16_t fourCylindersEarly[] = {1, 2};
    const uint16_t fourCylindersLate[] = {2, 1};
    assert_end_teeth(4, 180, fourCylindersEarly, 2);
    assert_end_teeth(4, 181, fourCylindersLate, 2);
    assert_end_teeth(4, 0, fourCylindersLate, 2);

    const uint16_t threeOrSixCylindersEarly[] = {1, 2, 3};
    const uint16_t threeOrSixCylindersMiddle[] = {2, 3, 1};
    const uint16_t threeOrSixCylindersLate[] = {3, 1, 2};
    assert_end_teeth(3, 120, threeOrSixCylindersEarly, 3);
    assert_end_teeth(3, 121, threeOrSixCylindersMiddle, 3);
    assert_end_teeth(6, 240, threeOrSixCylindersMiddle, 3);
    assert_end_teeth(6, 241, threeOrSixCylindersLate, 3);
    assert_end_teeth(6, 0, threeOrSixCylindersLate, 3);

    const uint16_t eightCylindersFirst[] = {1, 2, 3, 4};
    const uint16_t eightCylindersSecond[] = {2, 3, 4, 1};
    const uint16_t eightCylindersThird[] = {3, 4, 1, 2};
    const uint16_t eightCylindersFourth[] = {4, 1, 2, 3};
    assert_end_teeth(8, 90, eightCylindersFirst, 4);
    assert_end_teeth(8, 91, eightCylindersSecond, 4);
    assert_end_teeth(8, 180, eightCylindersSecond, 4);
    assert_end_teeth(8, 181, eightCylindersThird, 4);
    assert_end_teeth(8, 270, eightCylindersThird, 4);
    assert_end_teeth(8, 271, eightCylindersFourth, 4);
    assert_end_teeth(8, 0, eightCylindersFourth, 4);
}

void testBasicDistribPerToothIgn(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_set_end_teeth);
}