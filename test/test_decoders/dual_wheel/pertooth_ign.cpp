#include "../../test_utils.h"
#include "src/decoders/dualWheel.h"
#include "scheduler_ignition_controller.h"

using namespace decoders::shared;

static void assert_setEndTeeth(uint8_t expected, IgnitionSchedule &schedule, uint8_t index, int8_t advance, state_t &decoderState, const config4 &page4)
{
    schedule.dischargeAngle = 180 + advance; 
    decoders::dual_wheel::setEndTeeth(decoderState, page4);
    TEST_ASSERT_EQUAL(expected, decoderState.ignitionEndTeeth[index]);
}

static config4 setup_dualwheel_12_1(void)
{
    config4 page4 = {};

    //Setup a 12-1 wheel
    page4.triggerTeeth = 12;
    page4.TrigSpeed = CRANK_SPEED;
    page4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return page4;
}

static void test_perToothIgn_wasted()
{
    auto page4 = setup_dualwheel_12_1();
    auto decoderState = decoders::dual_wheel::initialise(page4);
    page4.sparkMode = IGN_MODE_WASTED;

    page4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(5, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(6, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(7, ignitionSchedule1, 0, 30, decoderState, page4);

    page4.triggerAngle = 90;
    assert_setEndTeeth(2, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(3, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(4, ignitionSchedule1, 0, 30, decoderState, page4);

    page4.triggerAngle = 180;
    assert_setEndTeeth(12, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(12, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(1, ignitionSchedule1, 0, 30, decoderState, page4);

    page4.triggerAngle = 270;
    assert_setEndTeeth(9, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(9, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(10, ignitionSchedule1, 0, 30, decoderState, page4);

    page4.triggerAngle = 360;
    assert_setEndTeeth(6, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(6, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(7, ignitionSchedule1, 0, 30, decoderState, page4);

    page4.triggerAngle = -90;
    assert_setEndTeeth(8, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(9, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(10, ignitionSchedule1, 0, 30, decoderState, page4);

    page4.triggerAngle = -180;
    assert_setEndTeeth(11, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(12, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(1, ignitionSchedule1, 0, 30, decoderState, page4);

    page4.triggerAngle = -270;
    assert_setEndTeeth(2, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(3, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(4, ignitionSchedule1, 0, 30, decoderState, page4);

    page4.triggerAngle = -360;
    assert_setEndTeeth(5, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(6, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(7, ignitionSchedule1, 0, 30, decoderState, page4);
}

static void test_perToothIgn_sequential()
{
    auto page4 = setup_dualwheel_12_1();
    auto decoderState = decoders::dual_wheel::initialise(page4);
    page4.sparkMode = IGN_MODE_SEQUENTIAL;
    page4.TrigSpeed = CRANK_SPEED;

    page4.triggerAngle = 360;
    assert_setEndTeeth(18, ignitionSchedule1, 0, -20, decoderState, page4);
    assert_setEndTeeth(18, ignitionSchedule1, 0, 0, decoderState, page4);
    assert_setEndTeeth(19, ignitionSchedule1, 0, 30, decoderState, page4);
}

void testDualWheelPerToothIgn(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_perToothIgn_wasted);
    RUN_TEST_P(test_perToothIgn_sequential);
}