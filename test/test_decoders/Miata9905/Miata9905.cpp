#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "crankMaths.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;

  auto decoder = triggerSetup_Miata9905();

  auto run_case = [&](int toothNum, int16_t expected, int trigAngle = 0) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothNum;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(toothLastToothTime + 100));
  };

  // timeToAngle(100) ~= 18 deg when revolution time is 2000us
  const int dt = 18;

  // toothAngles from triggerSetup_Miata9905 (indices 1..8)
  run_case(1, 710 + dt - 720); // wraps to small positive angle (710+18=728 -> 8)
  run_case(2, 100 + dt);
  run_case(3, 170 + dt);
  run_case(4, 280 + dt);
  run_case(5, 350 + dt);
  run_case(6, 460 + dt);
  run_case(7, 530 + dt);
  run_case(8, 640 + dt);

  // trigger angle offset
  run_case(2, 100 + dt + 10, 10);
}

static void test_getRPM(void)
{
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;
  extern decoder_status_t decoderStatus;

  auto decoder = triggerSetup_Miata9905();

  // --- Cranking branch: currentStatus.RPM < currentStatus.crankRPM and sync full
  decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 400;
  currentStatus.revolutionTime = 99999UL; // ensure SetRevolutionTime will update
  // gap such that tempRPM = 1000: triggerToothAngle=90, gap=15000 -> toothTime=15000*36
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 15000UL;
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Running path: should call stdGetRPM(CAM_SPEED)
  currentStatus.setRpm(2000);
  decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.revolutionTime = 12345UL; // ensure SetRevolutionTime will update
  toothOneMinusOneTime = 2000UL;
  toothOneTime = toothOneMinusOneTime + 120000UL; // >>1 -> 60000 -> 1000 RPM
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Fallback: when UpdateRevolutionTimeFromTeeth can't run, stdGetRPM returns currentStatus.RPM
  decoderStatus.syncStatus = SyncStatus::None;
  currentStatus.setRpm(555);
  TEST_ASSERT_EQUAL_UINT16(555U, decoder.getRPM());
}

void testMiata9905(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}