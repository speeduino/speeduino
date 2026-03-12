#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "scheduler.h"
#include "schedule_calcs.h"
#include "../../test_utils.h"
#include "scheduler_ignition_controller.h"

extern uint16_t ignitionEndTeeth[IGN_CHANNELS];

void test_fordst170_newIgn_12_trig0_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 12/1
    //Advance: 10
    //triggerAngle=0
    decoder_t decoder = triggerSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 0; //No trigger offset
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
  
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);

    //Test again with 0 degrees advance
    calculateIgnitionAngles(ignitionSchedule1, 5, 0);

    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(35, ignitionEndTeeth[0]);

    //Test again with 35 degrees advance
    calculateIgnitionAngles(ignitionSchedule1, 5, 35);

    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(31, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trig90_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 12/1
    //Advance: 10
    //triggerAngle=90
    decoder_t decoder = triggerSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 90; //No trigger offset
    calculateIgnitionAngles(ignitionSchedule1, 5, 35);

    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(22, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trig180_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=180
    decoder_t decoder = triggerSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 180; //No trigger offset
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
 
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trig270_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=270
    decoder_t decoder = triggerSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 270; //No trigger offset
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);

    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trig360_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=360
    decoder_t decoder = triggerSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 360; //No trigger offset
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trigNeg90_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-90
    decoder_t decoder = triggerSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -90; //No trigger offset
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);

    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(7, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trigNeg180_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-180
    decoder_t decoder = triggerSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -180; //No trigger offset
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);

    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(16, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trigNeg270_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-270
    decoder_t decoder = triggerSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -270; //No trigger offset
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(25, ignitionEndTeeth[0]);
}

void test_fordst170_newIgn_12_trigNeg360_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=-360
    decoder_t decoder = triggerSetup_FordST170();
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -360; //No trigger offset
    calculateIgnitionAngles(ignitionSchedule1, 5, 10);

    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(34, ignitionEndTeeth[0]);
}

void testFordST170()
{
    SET_UNITY_FILENAME() {

    RUN_TEST(test_fordst170_newIgn_12_trig0_1);
    RUN_TEST(test_fordst170_newIgn_12_trig90_1);
    RUN_TEST(test_fordst170_newIgn_12_trig180_1);
    RUN_TEST(test_fordst170_newIgn_12_trig270_1);
    RUN_TEST(test_fordst170_newIgn_12_trig360_1);
    RUN_TEST(test_fordst170_newIgn_12_trigNeg90_1);
    RUN_TEST(test_fordst170_newIgn_12_trigNeg180_1);
    RUN_TEST(test_fordst170_newIgn_12_trigNeg270_1);
    RUN_TEST(test_fordst170_newIgn_12_trigNeg360_1);
    
    }
}