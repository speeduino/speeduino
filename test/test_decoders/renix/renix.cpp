#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "scheduler.h"
#include "../../test_utils.h"
#include "decoder_init.h"
#include "scheduler_ignition_controller.h"

extern uint16_t ignitionEndTeeth[IGN_CHANNELS];
extern void calculateIgnitionAngles(IgnitionSchedule &schedule, uint16_t dwellAngle, int8_t advance);

static decoder_t test_setup_renix44(void)
{
    //Setup a renix 44 tooth wheel
    configPage4.TrigPattern = DECODER_RENIX;
    configPage2.nCylinders = 4;

    return triggerSetup_Renix();
}

static decoder_t test_setup_renix66()
{
    //Setup a renix 66 tooth wheel
    configPage4.TrigPattern = DECODER_RENIX;
    configPage2.nCylinders = 6;

    return triggerSetup_Renix();
}

//************************************** Begin the new ignition setEndTooth tests *************************************/

static void assert_setEndTeeth(uint8_t expected, decoder_t &decoder, IgnitionSchedule &schedule, uint8_t index, int8_t advance)
{
    schedule.dischargeAngle = 180 + advance; 
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(expected, ignitionEndTeeth[index]);
}

static void test_setEndTeeth_44_channel_1()
{
    auto decoder = test_setup_renix44();
    configPage4.sparkMode = IGN_MODE_SINGLE;

    configPage4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = 90;
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = 180;
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = 270;
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = 360;
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = -90;
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = -180;
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = -270;
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = -360;
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, 10);
}

static void test_setEndTeeth_66_channel_1()
{
    auto decoder = test_setup_renix66();
    configPage4.sparkMode = IGN_MODE_SINGLE;

    configPage4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = 90;
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = 180;
    assert_setEndTeeth(5, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(5, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(5, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = 270;
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = 360;
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = -90;
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = -180;
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(5, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(5, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = -270;
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 10);

    configPage4.triggerAngle = -360;
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, 10);
}

void testRenix()
{
  SET_UNITY_FILENAME() {
      RUN_TEST_P(test_setEndTeeth_44_channel_1);
      RUN_TEST_P(test_setEndTeeth_66_channel_1);
  }           
}