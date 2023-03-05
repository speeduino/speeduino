#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "scheduler.h"
#include "schedule_calcs.h"
#include "../../test_utils.h"

extern uint16_t ignition1EndTooth;
extern uint16_t ignition2EndTooth;
extern uint16_t ignition3EndTooth;
extern uint16_t ignition4EndTooth;

void test_ngc_newIgn_12_trig0_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 0; //No trigger offset
    
    calculateIgnitionAngle(ignitionSchedule1, 5, 10);
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(34, ignition1EndTooth);

    //Test again with 0 degrees advance
    calculateIgnitionAngle(ignitionSchedule1, 5, 0);
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(34, ignition1EndTooth);

    //Test again with 35 degrees advance
    calculateIgnitionAngle(ignitionSchedule1, 5, 35);
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(31, ignition1EndTooth);
}

void test_ngc_newIgn_12_trig90_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 90;
    calculateIgnitionAngle(ignitionSchedule1, 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(25, ignition1EndTooth);
}

void test_ngc_newIgn_12_trig180_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 180;
    calculateIgnitionAngle(ignitionSchedule1, 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(16, ignition1EndTooth);
}

void test_ngc_newIgn_12_trig270_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 270;
    calculateIgnitionAngle(ignitionSchedule1, 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(7, ignition1EndTooth);
}

void test_ngc_newIgn_12_trig360_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = 360;
    calculateIgnitionAngle(ignitionSchedule1, 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(34, ignition1EndTooth);
}

void test_ngc_newIgn_12_trigNeg90_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -90;
    calculateIgnitionAngle(ignitionSchedule1, 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(7, ignition1EndTooth);
}

void test_ngc_newIgn_12_trigNeg180_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -180;
    calculateIgnitionAngle(ignitionSchedule1, 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(16, ignition1EndTooth);
}

void test_ngc_newIgn_12_trigNeg270_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -270;
    calculateIgnitionAngle(ignitionSchedule1, 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(25, ignition1EndTooth);
}

void test_ngc_newIgn_12_trigNeg360_1()
{
    triggerSetup_NGC();
    CRANK_ANGLE_MAX_IGN = 360;
    configPage4.sparkMode = IGN_MODE_WASTED;
    configPage4.triggerAngle = -360;
    calculateIgnitionAngle(ignitionSchedule1, 5, 10);
    
    triggerSetEndTeeth_NGC();
    TEST_ASSERT_EQUAL(34, ignition1EndTooth);
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