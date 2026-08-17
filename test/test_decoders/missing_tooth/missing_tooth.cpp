#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "scheduler.h"
#include "../../test_utils.h"
#include "scheduler_ignition_controller.h"

static decoder_t test_setup_36_1()
{
    //Setup a 36-1 wheel
    configPage4.triggerTeeth = 36;
    configPage4.triggerMissingTeeth = 1;
    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return triggerSetup_missingTooth();
}

static decoder_t test_setup_60_2()
{
    //Setup a 60-2 wheel
    configPage4.triggerTeeth = 60;
    configPage4.triggerMissingTeeth = 2;
    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return triggerSetup_missingTooth();
}

extern uint16_t ignitionEndTeeth[IGN_CHANNELS];

//************************************** Begin the new ignition setEndTooth tests **************************************

static void assert_setEndTeeth(uint8_t expected, decoder_t &decoder, IgnitionSchedule &schedule, uint8_t index, int8_t advance)
{
    schedule.dischargeAngle = 360 + advance; 
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(expected, ignitionEndTeeth[index]);
}

static void test_missingtooth_newIgn_36_1()
{
    decoder_t decoder = test_setup_36_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    
    configPage4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(34, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(35, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(35, decoder, ignitionSchedule1, 0, 10);
    
    configPage4.triggerAngle = 90;
    assert_setEndTeeth(25, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(26, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(27, decoder, ignitionSchedule1, 0, 10);
    
    configPage4.triggerAngle = 180;
    assert_setEndTeeth(16, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(17, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(18, decoder, ignitionSchedule1, 0, 10);
    
    configPage4.triggerAngle = 270;
    assert_setEndTeeth(7, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(8, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(9, decoder, ignitionSchedule1, 0, 10);
    
    configPage4.triggerAngle = 360;
    assert_setEndTeeth(34, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(35, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(35, decoder, ignitionSchedule1, 0, 10);
    
    configPage4.triggerAngle = -90;
    assert_setEndTeeth(7, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(8, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(9, decoder, ignitionSchedule1, 0, 10);
    
    configPage4.triggerAngle = -180;
    assert_setEndTeeth(16, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(17, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(18, decoder, ignitionSchedule1, 0, 10);
    
    configPage4.triggerAngle = -270;
    assert_setEndTeeth(25, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(26, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(27, decoder, ignitionSchedule1, 0, 10);
    
    configPage4.triggerAngle = -360;
    assert_setEndTeeth(34, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(35, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(35, decoder, ignitionSchedule1, 0, 10);
}

static void test_missingtooth_newIgn_60_2()
{
    decoder_t decoder = test_setup_60_2();
    configPage4.sparkMode = IGN_MODE_WASTED;
    
    configPage4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(57, decoder, ignitionSchedule1, 0, -7);
    assert_setEndTeeth(58, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(58, decoder, ignitionSchedule1, 0, 6);
    
    configPage4.triggerAngle = 90;
    assert_setEndTeeth(43, decoder, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(44, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(45, decoder, ignitionSchedule1, 0, 6);
    
    configPage4.triggerAngle = 180;
    assert_setEndTeeth(28, decoder, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(29, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(30, decoder, ignitionSchedule1, 0, 6);
    
    configPage4.triggerAngle = 270;
    assert_setEndTeeth(13, decoder, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(14, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(15, decoder, ignitionSchedule1, 0, 6);
    
    configPage4.triggerAngle = 360;
    assert_setEndTeeth(58, decoder, ignitionSchedule1, 0, -7);
    assert_setEndTeeth(58, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(58, decoder, ignitionSchedule1, 0, 6);
    
    configPage4.triggerAngle = -90;
    assert_setEndTeeth(13, decoder, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(14, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(15, decoder, ignitionSchedule1, 0, 6);
    
    configPage4.triggerAngle = -180;
    assert_setEndTeeth(28, decoder, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(29, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(30, decoder, ignitionSchedule1, 0, 6);
    
    configPage4.triggerAngle = -270;
    assert_setEndTeeth(43, decoder, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(44, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(45, decoder, ignitionSchedule1, 0, 6);
    
    configPage4.triggerAngle = -360;
    assert_setEndTeeth(57, decoder, ignitionSchedule1, 0, -7);
    assert_setEndTeeth(58, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(58, decoder, ignitionSchedule1, 0, 6);
}

static void test_getCrankAngle(void)
{
    extern decoder_status_t decoderStatus;
    extern volatile unsigned long toothLastToothTime;
    extern volatile int toothCurrentCount;
    extern volatile bool revolutionOne;

    decoder_t decoder = test_setup_36_1();

    auto run_case = [&](int toothCount, bool revOne, int delta, int trigAngle, int16_t expected) {
        toothLastToothTime = 2000;
        toothCurrentCount = toothCount;
        revolutionOne = revOne;
        decoderStatus.syncStatus = SyncStatus::Full;
        decoderStatus.toothAngleIsCorrect = true;
        configPage4.triggerAngle = trigAngle;
        setAngleConverterRevolutionTime(2000);
        int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + delta);
        TEST_ASSERT_EQUAL(expected, angle);
    };

    // For 36-1 wheel: triggerToothAngle = 10 degrees. timeToAngle(100) ~= 18 deg
    const int dt = 18;

    // Basic teeth
    run_case(1, false, 100, 0, 0 + dt);
    run_case(2, false, 100, 0, 10 + dt);
    run_case(10, false, 100, 0, 90 + dt);
    run_case(35, false, 100, 0, 340 + dt);

    // Trigger angle offset
    run_case(1, false, 100, 90, 90 + dt);

    // Revolution one true should add 360 degrees
    configPage4.TrigSpeed = CAM_SPEED;
    run_case(1, true, 100, 0, 0 + dt);
    configPage4.TrigSpeed = CRANK_SPEED;
    run_case(1, true, 100, 0, 360 + 0 + dt);
}

void testMissingTooth()
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_missingtooth_newIgn_36_1);
        RUN_TEST_P(test_missingtooth_newIgn_60_2);
        RUN_TEST_P(test_getCrankAngle);
    }
}