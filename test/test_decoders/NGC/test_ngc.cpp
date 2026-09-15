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

static void test_getRPM(void)
{
    auto decoder = triggerSetup_NGC();

    // Ensure staging allows cranking calculation
    configPage4.StgCycles = 0;
    currentStatus.crankRPM = 400;

    // --- Cranking path: tooth-angle correct -> use crankingGetRPM(36, CRANK_SPEED)
    currentStatus.setRpm(currentStatus.crankRPM/2U);
    currentStatus.startRevolutions = 0; // cranking
    decoderStatus.toothAngleIsCorrect = true;
    decoderStatus.syncStatus = SyncStatus::Full;
    currentStatus.revolutionTime = 99999UL; // ensure SetRevolutionTime will update
    toothLastMinusOneToothTime = 1000UL;
    toothLastToothTime = toothLastMinusOneToothTime + 1667UL; // gap ~=1667 -> revTime ~=60012 -> ~1000 RPM
    TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

    // --- If tooth angle not correct, return currentStatus.RPM
    decoderStatus.toothAngleIsCorrect = false;
    TEST_ASSERT_EQUAL_UINT16(currentStatus.RPM, decoder.getRPM());

    // --- Running path: use stdGetRPM(CRANK_SPEED)
    currentStatus.setRpm(currentStatus.crankRPM*2U);
    currentStatus.startRevolutions = 1; // not cranking
    decoderStatus.toothAngleIsCorrect = true;
    decoderStatus.syncStatus = SyncStatus::Full;
    currentStatus.revolutionTime = 12345UL; // ensure update
    toothOneMinusOneTime = 1000UL;
    toothOneTime = toothOneMinusOneTime + 60000UL; // revTime = 60000 -> 1000 RPM
    TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

    // --- Fallback: when sync lost, stdGetRPM returns currentStatus.RPM
    decoderStatus.syncStatus = SyncStatus::None;
    currentStatus.setRpm(777);
    TEST_ASSERT_EQUAL_UINT16(777U, decoder.getRPM());
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
    RUN_TEST_P(test_getRPM);
   }
}