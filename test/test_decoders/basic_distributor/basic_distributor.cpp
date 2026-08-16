#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile int toothCurrentCount;

  // Configure a 4-tooth distributor (4-cylinder cam-spaced)
  configPage2.nCylinders = 4;
  configPage4.triggerTeeth = 4;

  auto decoder = triggerSetup_BasicDistributor();

  auto run_case = [&](int toothCount, int delta, int trigAngle, int16_t expected) {
    toothLastToothTime = 2000;
    toothLastMinusOneToothTime = toothLastToothTime - (5*delta);
    toothCurrentCount = toothCount;
    decoderStatus.syncStatus = SyncStatus::Full;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(toothLastToothTime + delta));
  };

  // timeToAngleIntervalTooth(100) ~ 18 deg when revolution time = 2000
  const int dt = 36;

  run_case(1, 100, 0, 0 + dt);
  run_case(2, 100, 0, 180 + dt);
  run_case(3, 100, 0, 360 + dt);
  run_case(4, 100, 0, 540 + dt);

  // trigger offset
  run_case(1, 100, 30, 30 + dt);
}

static void test_getRPM(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;

  // Configure a 4-tooth distributor (4-cylinder cam-spaced)
  configPage2.nCylinders = 4;
  configPage2.strokes = FOUR_STROKE;
  auto decoder = triggerSetup_BasicDistributor();

  // Prepare for cranking path: RPM < crankRPM
  configPage4.StgCycles = 0;
  currentStatus.startRevolutions = 1;
  decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 400;
  // Ensure SetRevolutionTime will update
  currentStatus.revolutionTime = 99999UL;
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 30000UL; // gap=30000 -> revTime = gap*totalTeeth/2 = 30000*4/2 = 60000
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // Running path: should return stdGetRPM(CAM_SPEED) -> currentStatus.RPM when no toothOne* update
  currentStatus.setRpm(2000);
  toothOneMinusOneTime = 0;
  toothOneTime = 0;
  decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_UINT16(2000U, decoder.getRPM());

  configPage2.strokes = TWO_STROKE;
  TEST_ASSERT_EQUAL_UINT16(2000U, decoder.getRPM());

  // If not synced, should return currentStatus.RPM
  decoderStatus.syncStatus = SyncStatus::None;
  currentStatus.setRpm(555);
  TEST_ASSERT_EQUAL_UINT16(555U, decoder.getRPM());
}

void testBasicDistributor(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}