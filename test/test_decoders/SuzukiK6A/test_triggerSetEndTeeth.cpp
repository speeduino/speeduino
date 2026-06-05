#include <unity.h>
#include "decoders.h"
#include "globals.h"
#include "../../test_utils.h"
#include "schedule_calcs.h"
#include "decoder_init.h"
#include "scheduler_ignition_controller.h"

static decoder_t test_setup_SuzukiK6A()
{
    //Setup a renix 44 tooth wheel
    configPage4.TrigPattern = DECODER_SUZUKI_K6A;
    configPage2.nCylinders = 3;

    return triggerSetup_SuzukiK6A();
}

extern uint16_t ignitionEndTeeth[IGN_CHANNELS];

//************************************** Begin the new ignition setEndTooth tests **************************************

static void test_k6A_newIgn_trig0_1()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=0
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule1.dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 0; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(3, ignitionEndTeeth[0]);
}

static void test_k6A_newIgn_trig90_1()
{
    //Test the set end tooth function. Conditions:

    //Advance: 10
    //triggerAngle=90
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule1.dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 90; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(3, ignitionEndTeeth[0]);
}

static void test_k6A_newIgn_trig180_1()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=180
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule1.dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 180; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(1, ignitionEndTeeth[0]);
}

static void test_k6A_newIgn_trig270_1()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=270
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule1.dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 270; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(1, ignitionEndTeeth[0]);
}

static void test_k6A_newIgn_trig360_1()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=360
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule1.dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 360; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(3, ignitionEndTeeth[0]);
}

static void test_k6A_newIgn_trigNeg90_1()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=-90
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule1.dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -90; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(1, ignitionEndTeeth[0]);
}

static void test_k6A_newIgn_trigNeg180_1()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=-180
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule1.dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -180; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(1, ignitionEndTeeth[0]);
}

static void test_k6A_newIgn_trigNeg270_1()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=-270
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule1.dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -270; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(3, ignitionEndTeeth[0]);
}

static void test_k6A_newIgn_trigNeg360_1()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=-360
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule1.dischargeAngle = 360 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -360; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(3, ignitionEndTeeth[0]);
}

// ******* CHannel 2 *******
static void test_k6A_newIgn_trig0_2()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=0
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule2.dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 0; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(1, ignitionEndTeeth[1]);
}

static void test_k6A_newIgn_trig90_2()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=90
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule2.dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 90; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(1, ignitionEndTeeth[1]);
}

static void test_k6A_newIgn_trig180_2()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=180
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule2.dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 180; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(3, ignitionEndTeeth[1]);
}

static void test_k6A_newIgn_trig270_2()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=270
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule2.dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 270; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(3, ignitionEndTeeth[1]);
}

void test_K6A_newIgn_trig366()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=360
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule2.dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = 360; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(1, ignitionEndTeeth[1]);
}

static void test_k6A_newIgn_trigNeg90_2()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=-90
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule2.dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -90; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(3, ignitionEndTeeth[1]);
}

static void test_k6A_newIgn_trigNeg180_2()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=-180
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule2.dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -180; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(3, ignitionEndTeeth[1]);
}

static void test_k6A_newIgn_trigNeg270_2()
{
    //Test the set end tooth function. Conditions:
     //Advance: 10
    //triggerAngle=-270
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule2.dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -270; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(1, ignitionEndTeeth[1]);
}

void test_K6A_newIgn_trigNeg366()
{
    //Test the set end tooth function. Conditions:
    //Advance: 10
    //triggerAngle=-360
    auto decoder = test_setup_SuzukiK6A();
    configPage4.sparkMode = IGN_MODE_SINGLE;
    ignitionSchedule2.dischargeAngle = 180 - 10; //Set 10 degrees advance
    configPage4.triggerAngle = -360; //No trigger offset
    
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(1, ignitionEndTeeth[1]);
}


void testSuzukiK6A_setEndTeeth()
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_k6A_newIgn_trig0_1);
    RUN_TEST(test_k6A_newIgn_trig90_1);
    RUN_TEST(test_k6A_newIgn_trig180_1);
    RUN_TEST(test_k6A_newIgn_trig270_1);
    RUN_TEST(test_k6A_newIgn_trig360_1);
    RUN_TEST(test_k6A_newIgn_trigNeg90_1);
    RUN_TEST(test_k6A_newIgn_trigNeg180_1);
    RUN_TEST(test_k6A_newIgn_trigNeg270_1);
    RUN_TEST(test_k6A_newIgn_trigNeg360_1);

    RUN_TEST(test_k6A_newIgn_trig0_2);
    RUN_TEST(test_k6A_newIgn_trig90_2);
    RUN_TEST(test_k6A_newIgn_trig180_2);
    RUN_TEST(test_k6A_newIgn_trig270_2);
    RUN_TEST(test_K6A_newIgn_trig366);
    RUN_TEST(test_k6A_newIgn_trigNeg90_2);
    RUN_TEST(test_k6A_newIgn_trigNeg180_2);
    RUN_TEST(test_k6A_newIgn_trigNeg270_2);
    RUN_TEST(test_K6A_newIgn_trigNeg366);
  }
}