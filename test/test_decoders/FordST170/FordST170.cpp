#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "scheduler.h"
#include "../../test_utils.h"
#include "scheduler_ignition_controller.h"

extern uint16_t ignitionEndTeeth[IGN_CHANNELS];
extern decoder_status_t decoderStatus;
extern volatile uint32_t toothLastToothTime;
extern volatile int toothCurrentCount;
extern volatile bool revolutionOne;
extern void calculateIgnitionAngles(IgnitionSchedule &schedule, uint16_t dwellAngle, int8_t advance);

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

static void test_getCrankAngle(void)
{
    auto decoder = triggerSetup_FordST170();

    auto run_case = [&](int toothCount, bool revOne, int delta, int trigAngle, int16_t expected) {
        toothLastToothTime = 2000;
        toothCurrentCount = toothCount;
        revolutionOne = revOne;
        decoderStatus.syncStatus = SyncStatus::Full;
        decoderStatus.toothAngleIsCorrect = true;
        configPage4.triggerAngle = trigAngle;
        CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
        setAngleConverterRevolutionTime(2000);
        TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(toothLastToothTime + delta));
    };

    // timeToAngle(100) ~= 18 deg when revolution time = 2000
    const int dt = 18;

    // Basic teeth
    run_case(1, false, 100, 0, 0 + dt);
    run_case(2, false, 100, 0, 10 + dt);
    run_case(18, false, 100, 0, 170 + dt);
    run_case(36, false, 100, 0, 350 + dt);

    // Trigger angle offset
    run_case(3, false, 100, 90, (3 - 1) * 10 + 90 + dt);

    // Revolution one true should add 360 degrees
    configPage4.TrigSpeed = CAM_SPEED;
    run_case(1, true, 100, 0, 0 + dt);
    configPage4.TrigSpeed = CRANK_SPEED;
    run_case(1, true, 100, 0, 360 + 0 + dt);

    // Wrap-around when >720: expect subtraction of 720
    {
        int raw = (36 - 1) * 10 + dt + 360; // 350 + dt + 360
        int wrapped = raw >= 720 ? raw - 720 : raw;
        run_case(36, true, 100, 0, wrapped);
    }
}

static void test_getRPM(void)
{
  auto decoder = triggerSetup_FordST170();

  currentStatus.crankRPM = 400;

  // Running
  currentStatus.setRpm(currentStatus.crankRPM*2);
  auto rpm1 = decoder.getRPM();
  TEST_ASSERT_NOT_EQUAL(0, rpm1);

  // Cranking
  toothCurrentCount = 2;
  currentStatus.setRpm(currentStatus.crankRPM/2);
  TEST_ASSERT_NOT_EQUAL(rpm1, decoder.getRPM());
  TEST_ASSERT_NOT_EQUAL(0, decoder.getRPM());

  toothCurrentCount = 1;
  currentStatus.setRpm(currentStatus.crankRPM/2);
  TEST_ASSERT_EQUAL(currentStatus.RPM, decoder.getRPM());
}

void testFordST170()
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_fordst170_newIgn_12_trig0_1);
        RUN_TEST_P(test_fordst170_newIgn_12_trig90_1);
        RUN_TEST_P(test_fordst170_newIgn_12_trig180_1);
        RUN_TEST_P(test_fordst170_newIgn_12_trig270_1);
        RUN_TEST_P(test_fordst170_newIgn_12_trig360_1);
        RUN_TEST_P(test_fordst170_newIgn_12_trigNeg90_1);
        RUN_TEST_P(test_fordst170_newIgn_12_trigNeg180_1);
        RUN_TEST_P(test_fordst170_newIgn_12_trigNeg270_1);
        RUN_TEST_P(test_fordst170_newIgn_12_trigNeg360_1);
        RUN_TEST_P(test_getCrankAngle);   
        RUN_TEST_P(test_getRPM);         
    }
}