#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "scheduler.h"
#include "../../test_utils.h"
#include "scheduler_ignition_controller.h"

extern volatile uint32_t toothLastToothTime;
extern volatile unsigned long toothLastMinusOneToothTime;
extern volatile unsigned long toothOneTime;
extern volatile unsigned long toothOneMinusOneTime;
extern decoder_status_t decoderStatus;
extern uint16_t ignitionEndTeeth[IGN_CHANNELS];
extern void calculateIgnitionAngles(IgnitionSchedule &schedule, uint16_t dwellAngle, int8_t advance);

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

static void test_getRevolutionTime(void)
{
    auto decoder = triggerSetup_NGC();

    // Ensure staging allows cranking calculation
    configPage4.StgCycles = 0;
    currentStatus.crankRPM = 400;

    // --- Cranking path: tooth-angle correct -> use crankingGetRevolutionTime(36, CRANK_SPEED)
    currentStatus.setRpm(currentStatus.crankRPM/2U);
    currentStatus.startRevolutions = 0; // cranking
    decoderStatus.toothAngleIsCorrect = true;
    decoderStatus.syncStatus = SyncStatus::Full;
    currentStatus.revolutionTime = 99999UL;
    toothLastMinusOneToothTime = 1000UL;
    toothLastToothTime = toothLastMinusOneToothTime + 1667UL; // gap ~=1667 -> revTime ~=60012
    TEST_ASSERT_EQUAL_UINT32(1667UL*36UL, decoder.getRevolutionTime());

    // --- If tooth angle not correct, return currentStatus.revolutionTime
    decoderStatus.toothAngleIsCorrect = false;
    TEST_ASSERT_EQUAL_UINT32(99999UL, decoder.getRevolutionTime());

    // --- Running path: use stdGetRevolutionTime(CRANK_SPEED)
    currentStatus.setRpm(currentStatus.crankRPM*2U);
    currentStatus.startRevolutions = 1; // not cranking
    decoderStatus.toothAngleIsCorrect = true;
    decoderStatus.syncStatus = SyncStatus::Full;
    currentStatus.revolutionTime = 12345UL;
    toothOneMinusOneTime = 1000UL;
    toothOneTime = toothOneMinusOneTime + 60000UL; // revTime = 60000
    TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

    // --- Fallback: when sync lost, stdGetRevolutionTime returns currentStatus.revolutionTime
    decoderStatus.syncStatus = SyncStatus::None;
    TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());
}

void testNGC()
{
   SET_UNITY_FILENAME() {

    RUN_TEST_P(test_ngc_newIgn_12_trig0_1);
    RUN_TEST_P(test_ngc_newIgn_12_trig90_1);
    RUN_TEST_P(test_ngc_newIgn_12_trig180_1);
    RUN_TEST_P(test_ngc_newIgn_12_trig270_1);
    RUN_TEST_P(test_ngc_newIgn_12_trig360_1);
    RUN_TEST_P(test_ngc_newIgn_12_trigNeg90_1);
    RUN_TEST_P(test_ngc_newIgn_12_trigNeg180_1);
    RUN_TEST_P(test_ngc_newIgn_12_trigNeg270_1);
    RUN_TEST_P(test_ngc_newIgn_12_trigNeg360_1);
    RUN_TEST_P(test_getRevolutionTime);
   }
}