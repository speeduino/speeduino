#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "scheduler.h"
#include "schedule_calcs.h"
#include "../../test_utils.h"
#include "scheduler_ignition_controller.h"

extern uint16_t ignitionEndTeeth[IGN_CHANNELS];

void test_ngc_newIgn_12_trig0_1()
{
    decoder_t decoder = triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 0; //No trigger offset
    
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);

    //Test again with 0 degrees advance
    calculateIgnitionAngles(ignitionSchedule1, 5, 0);
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);

    //Test again with 35 degrees advance
    calculateIgnitionAngles(ignitionSchedule1, 5, 35);
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(31, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trig90_1()
{
    decoder_t decoder = triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 90;
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(25, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trig180_1()
{
    decoder_t decoder = triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 180;
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trig270_1()
{
    decoder_t decoder = triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 270;
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trig360_1()
{
    decoder_t decoder = triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 360;
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trigNeg90_1()
{
    decoder_t decoder = triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -90;
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trigNeg180_1()
{
    decoder_t decoder = triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -180;
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trigNeg270_1()
{
    decoder_t decoder = triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -270;
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(25, ignitionEndTeeth[0]);
}

void test_ngc_newIgn_12_trigNeg360_1()
{
    decoder_t decoder = triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -360;
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);
}

void testNGC()
{
   SET_UNITY_FILENAME() {

    RUN_TEST(test_ngc_newIgn_12_trig0_1);
    RUN_TEST(test_ngc_newIgn_12_trig90_1);
    RUN_TEST(test_ngc_newIgn_12_trig180_1);
    RUN_TEST(test_ngc_newIgn_12_trig270_1);
    RUN_TEST(test_ngc_newIgn_12_trig360_1);
    RUN_TEST(test_ngc_newIgn_12_trigNeg90_1);
    RUN_TEST(test_ngc_newIgn_12_trigNeg180_1);
    RUN_TEST(test_ngc_newIgn_12_trigNeg270_1);
    RUN_TEST(test_ngc_newIgn_12_trigNeg360_1);
   }
}