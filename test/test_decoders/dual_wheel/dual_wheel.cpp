#include "decoders.h"
#include "globals.h"
#include "scheduler.h"
#include "../../test_utils.h"
#include "scheduler_ignition_controller.h"

extern uint16_t ignitionEndTeeth[IGN_CHANNELS];
extern decoder_status_t decoderStatus;

static decoder_t test_setup_dualwheel_12_1()
{
    //Setup a 12-1 wheel
    configPage4.triggerTeeth = 12;
    //configPage4.triggerMissingTeeth = 1;
    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return triggerSetup_DualWheel();
}

//************************************** Begin the new ignition setEndTooth tests **************************************

static void assert_setEndTeeth(uint8_t expected, decoder_t &decoder, IgnitionSchedule &schedule, uint8_t index, int8_t advance)
{
    schedule.dischargeAngle = 180 + advance; 
    decoder.setEndTeeth();
    TEST_ASSERT_EQUAL(expected, ignitionEndTeeth[index]);
}

static void test_dualwheel_newIgn_12_1()
{
    configPage4.sparkMode = IGN_MODE_WASTED;
    auto decoder = test_setup_dualwheel_12_1();

    configPage4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(5, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(7, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = 90;
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = 180;
    assert_setEndTeeth(12, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(12, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = 270;
    assert_setEndTeeth(9, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(9, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(10, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = 360;
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(7, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = -90;
    assert_setEndTeeth(8, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(9, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(10, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = -180;
    assert_setEndTeeth(11, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(12, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(1, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = -270;
    assert_setEndTeeth(2, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(3, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(4, decoder, ignitionSchedule1, 0, 30);

    configPage4.triggerAngle = -360;
    assert_setEndTeeth(5, decoder, ignitionSchedule1, 0, -20);
    assert_setEndTeeth(6, decoder, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(7, decoder, ignitionSchedule1, 0, 30);
}

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;
  extern volatile bool revolutionOne;

  auto decoder = test_setup_dualwheel_12_1();

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

  // For a 12-tooth wheel triggerToothAngle = 30 degrees. timeToAngle(100) ~= 18 deg
  const int dt = 18;

  // Basic teeth 1..12 (no trigger offset, not on second revolution)
  run_case(1, false, 100, 0, 0 + dt);
  run_case(2, false, 100, 0, 30 + dt);
  run_case(3, false, 100, 0, 60 + dt);
  run_case(4, false, 100, 0, 90 + dt);
  run_case(5, false, 100, 0, 120 + dt);
  run_case(6, false, 100, 0, 150 + dt);
  run_case(7, false, 100, 0, 180 + dt);
  run_case(8, false, 100, 0, 210 + dt);
  run_case(9, false, 100, 0, 240 + dt);
  run_case(10, false, 100, 0, 270 + dt);
  run_case(11, false, 100, 0, 300 + dt);
  run_case(12, false, 100, 0, 330 + dt);

  // Secondary-last-tooth path: toothCurrentCount == 0 treated as 12
  run_case(0, false, 100, 0, 330 + dt);

  // Trigger angle offset
  run_case(3, false, 100, 10, 60 + 10 + dt);

  // Revolution one true should add 360 degrees
    configPage4.TrigSpeed = CAM_SPEED;
    run_case(1, true, 100, 0, 0 + dt);
    configPage4.TrigSpeed = CRANK_SPEED;
    run_case(1, true, 100, 0, 360 + 0 + dt);
}

static void test_getRPM(void)
{
  auto decoder = triggerSetup_DualWheel();

  decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.crankRPM = 400;
  currentStatus.setRpm(currentStatus.crankRPM*2);
  auto rpm1 = decoder.getRPM();
  TEST_ASSERT_NOT_EQUAL(0, rpm1);

  currentStatus.setRpm(currentStatus.crankRPM/2);
  TEST_ASSERT_NOT_EQUAL(rpm1, decoder.getRPM());
  TEST_ASSERT_NOT_EQUAL(0, decoder.getRPM());

  decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL(0, decoder.getRPM());

  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL(0, decoder.getRPM());
}

void testDualWheel()
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_dualwheel_newIgn_12_1);
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}