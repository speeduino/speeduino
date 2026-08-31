#include "decoders.h"
#include "decoder_init.h"
#include "globals.h"
#include "scheduler.h"
#include "test_utils.h"
#include "scheduler_ignition_controller.h"
#include "crankMaths.h"

extern volatile uint32_t toothLastToothTime;
extern volatile unsigned long toothLastMinusOneToothTime;
extern volatile int toothCurrentCount;
extern decoder_status_t decoderStatus;
extern uint16_t ignitionEndTeeth[IGN_CHANNELS];
extern volatile unsigned long toothLastMinusOneToothTime;
extern volatile unsigned long toothOneTime;
extern volatile unsigned long toothOneMinusOneTime;
extern void calculateIgnitionAngles(IgnitionSchedule &schedule, uint16_t dwellAngle, int8_t advance);

static void assert_setEndTeeth(uint16_t expected, decoder_t &decoder, IgnitionSchedule &schedule, uint8_t index, int8_t advance)
{
    schedule.dischargeAngle = 360 + advance; 
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(expected, ignitionEndTeeth[index]);
}

void test_setEndTeeth_channel1()
{
    decoder_t decoder = triggerSetup_Nissan360();
    configPage4.sparkMode = IGN_MODE_WASTED;

    configPage4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(171, decoder, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(176, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(158, decoder, ignitionSchedule1, 0, -35);

    configPage4.triggerAngle = 90;
    assert_setEndTeeth(126, decoder, ignitionSchedule1, 0, -10);

    configPage4.triggerAngle = 180;
    assert_setEndTeeth(81, decoder, ignitionSchedule1, 0, -10);

    configPage4.triggerAngle = 270;
    assert_setEndTeeth(36, decoder, ignitionSchedule1, 0, -10);

    configPage4.triggerAngle = 360;
    assert_setEndTeeth(351, decoder, ignitionSchedule1, 0, -10);

    configPage4.triggerAngle = -90;
    assert_setEndTeeth(216, decoder, ignitionSchedule1, 0, -10);

    configPage4.triggerAngle = -180;
    assert_setEndTeeth(261, decoder, ignitionSchedule1, 0, -10);

    configPage4.triggerAngle = -270;
    assert_setEndTeeth(306, decoder, ignitionSchedule1, 0, -10);

    configPage4.triggerAngle = -360;
    assert_setEndTeeth(351, decoder, ignitionSchedule1, 0, -10);
}

static void test_getCrankAngle(void)
{
    auto decoder = triggerSetup_Nissan360();

    auto run_case = [&](int toothNum, unsigned long elapsedDelta, int trigAngle, int16_t expected) {
        // Set deterministic tooth times so halfTooth is known
        toothLastMinusOneToothTime = 1000;
        toothLastToothTime = 1500; // halfTooth = 250
        toothCurrentCount = toothNum;
        decoderStatus.syncStatus = SyncStatus::Full;
        configPage4.triggerAngle = trigAngle;
        CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 360;
        setAngleConverterRevolutionTime(2000);
        TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(toothLastToothTime + elapsedDelta));
    };

    // halfTooth = (1500-1000)/2 = 250
    // Cases: elapsed <=250 -> no extra degree; elapsed >250 -> +1 degree
    run_case(1, 100, 0, 0);       // tooth 1, before half -> 0 deg
    run_case(1, 300, 0, 1);       // tooth 1, after half -> 1 deg
    run_case(2, 100, 0, 2);       // tooth 2 -> 2 deg
    run_case(36, 100, 90, 160);   // tooth 36: (35*2)=70 + 90 = 160
    run_case(180, 300, 0, 359);   // tooth 180: (179*2)=358 + 1 => 359 deg
    // Negative triggerAngle should wrap into positive range
    run_case(1, 100, -90, 270);   // -90 -> +360 => 270
}

static void test_getRPM(void)
{
        auto decoder = triggerSetup_Nissan360();

        // --- Cranking path: startRevolutions < 2 -> revTime = gap * 180
        currentStatus.setRpm(0);
        currentStatus.crankRPM = 400;
        currentStatus.startRevolutions = 0; // cranking
        decoderStatus.syncStatus = SyncStatus::Full;
        currentStatus.revolutionTime = 99999UL; // ensure SetRevolutionTime will update
        toothLastMinusOneToothTime = 1000UL;
        toothLastToothTime = toothLastMinusOneToothTime + 333UL; // gap = 333 -> revTime = 333*180
        uint32_t revTime = (toothLastToothTime - toothLastMinusOneToothTime) * 180UL;
        uint16_t expected = (uint16_t)((MICROS_PER_MIN + (revTime / 2U)) / revTime);
        TEST_ASSERT_EQUAL_UINT16(expected, decoder.getRPM());

        // --- Running path: use the toothOne pair and >>1 scaling
        currentStatus.setRpm(2000);
        currentStatus.startRevolutions = 2; // not cranking
        decoderStatus.syncStatus = SyncStatus::Full;
        toothOneMinusOneTime = 1000UL;
        toothOneTime = toothOneMinusOneTime + 120000UL; // >>1 => 60000 uS revTime -> 1000 RPM
        revTime = ((toothOneTime - toothOneMinusOneTime) >> 1);
        expected = (uint16_t)((MICROS_PER_MIN + (revTime / 2U)) / revTime);
        TEST_ASSERT_EQUAL_UINT16(expected, decoder.getRPM());

        // --- Fallback: when sync lost or tooth times not present, expect 0
        decoderStatus.syncStatus = SyncStatus::None;
        TEST_ASSERT_EQUAL_UINT16(0U, decoder.getRPM());

}

void testNissan360()
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_setEndTeeth_channel1);
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}