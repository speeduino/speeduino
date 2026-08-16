#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int secondaryToothCount;

  auto decoder = triggerSetup_Vmax();

  auto run_case = [&](int secTooth, int delta, int trigAngle, int16_t expected) {
    toothLastToothTime = 2000;
    secondaryToothCount = secTooth;
    decoderStatus.syncStatus = SyncStatus::Full;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(toothLastToothTime + delta));
  };

  // timeToAngle(100) ~= 18 deg
  const int dt = 18;

  run_case(1, 100, 0, 0 + dt);
  run_case(2, 100, 0, 40 + dt);
  run_case(3, 100, 0, 110 + dt);
  run_case(4, 100, 0, 180 + dt);
  run_case(5, 100, 0, 220 + dt);
  run_case(6, 100, 0, 290 + dt);

  // trigger angle offset
  run_case(3, 100, 10, 110 + dt + 10);
}

static void test_getRPM(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;
  extern volatile int secondaryToothCount;
  extern uint16_t triggerToothAngle;

  auto decoder = triggerSetup_Vmax();

  // --- Cranking-style calculation when RPM below threshold: computes using last-tooth gap * 36
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 400;
  configPage4.crankRPM = 4;
  currentStatus.startRevolutions = 0; // cranking
  decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
  // Set a non-zero triggerToothAngle so calculation yields non-zero result
  triggerToothAngle = 10;
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 1667UL; // gap
  unsigned long toothTime = (toothLastToothTime - toothLastMinusOneToothTime) * 36UL;
  uint16_t expected = (uint16_t)(((unsigned long)triggerToothAngle * (MICROS_PER_MIN/10U)) / toothTime);
  TEST_ASSERT_EQUAL_UINT16(expected, decoder.getRPM());

  // --- If tooth times missing, expect 0
  toothLastMinusOneToothTime = 0;
  toothLastToothTime = 0;
  currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
  TEST_ASSERT_EQUAL_UINT16(0U, decoder.getRPM());

  // --- Running path: use stdGetRPM via toothOne pair -> RpmFromRevolutionTimeUs
  currentStatus.setRpm(2000);
  currentStatus.crankRPM = 100; // ensure not considered cranking
  configPage4.crankRPM = 1;
  currentStatus.startRevolutions = 1; // not cranking
  decoderStatus.syncStatus = SyncStatus::Full;
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL; // revTime = 60000 -> 1000 RPM
  currentStatus.revolutionTime = UINT32_MAX; // To trigger a change
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Sync lost -> expect 0
  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT16(0U, decoder.getRPM());

}

void testVMax(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}