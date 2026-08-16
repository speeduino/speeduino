#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "scheduler.h"
#include "../../test_utils.h"
#include "scheduler_ignition_controller.h"

static decoder_t test_setup_dualwheel_12_1()
{
    //Setup a 12-1 wheel
    configPage4.triggerTeeth = 12;
    //configPage4.triggerMissingTeeth = 1;
    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return triggerSetup_DualWheel();
}

extern uint16_t ignitionEndTeeth[IGN_CHANNELS];

//************************************** Begin the new ignition setEndTooth tests **************************************

static void assert_setEndTeeth(uint8_t expected, decoder_t &decoder, IgnitionSchedule &schedule, uint8_t index, int8_t advance)
{
    schedule.dischargeAngle = 180 + advance; 
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(expected, ignitionEndTeeth[index]);
}

static void test_dualwheel_newIgn_12_1_trig0_1()
{
    configPage4.sparkMode = IGN_MODE_WASTED;
    auto decoder = test_setup_dualwheel_12_1();

    configPage4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(5, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(7, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = 90;
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = 180;
    assert_setEndTeeth(12, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(12, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = 270;
    assert_setEndTeeth(9, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(9, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(10, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = 360;
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(7, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = -90;
    assert_setEndTeeth(8, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(9, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(10, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = -180;
    assert_setEndTeeth(11, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(12, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = -270;
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = -360;
    assert_setEndTeeth(5, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(7, decoder, ignitionSchedule1, 0, 30);
}

void testDualWheel()
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_dualwheel_newIgn_12_1_trig0_1);
  }
}