#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

// used by the ROVER MEMS pattern
// #define ID_TOOTH_PATTERN 0 // have we identified teeth to skip for calculating RPM?
#define SKIP_TOOTH1 1
#define SKIP_TOOTH2 2
#define SKIP_TOOTH3 3
#define SKIP_TOOTH4 4

extern int16_t toothAngles[24];

static void test_getRPM(void)
{
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;
  extern volatile unsigned int toothCurrentCount;
  extern decoder_status_t decoderStatus;

  auto decoder = triggerSetup_RoverMEMS();

  currentStatus.crankRPM = 400;

  // Make skip-tooth definitions deterministic for this test
  toothAngles[SKIP_TOOTH1] = 100;
  toothAngles[SKIP_TOOTH2] = 101;
  toothAngles[SKIP_TOOTH3] = 102;
  toothAngles[SKIP_TOOTH4] = 103;

  // Ensure staging allows cranking calculation
  configPage4.StgCycles = 0;

  // --- Cranking path: tooth not a skip tooth -> crankingGetRPM(36)
  currentStatus.setRpm(currentStatus.crankRPM/2U);
  currentStatus.startRevolutions = 0; // cranking
  decoderStatus.syncStatus = SyncStatus::Full;
  toothCurrentCount = 1; // not a skip tooth
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 1667UL; // gap -> revTime ~=1667*36 ~=60012 -> ~1000 RPM
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- If at a skip tooth, return currentStatus.RPM
  toothCurrentCount = (unsigned int)toothAngles[SKIP_TOOTH1];
  TEST_ASSERT_EQUAL_UINT16(currentStatus.RPM, decoder.getRPM());
  toothCurrentCount = (unsigned int)toothAngles[SKIP_TOOTH2];
  TEST_ASSERT_EQUAL_UINT16(currentStatus.RPM, decoder.getRPM());
  toothCurrentCount = (unsigned int)toothAngles[SKIP_TOOTH3];
  TEST_ASSERT_EQUAL_UINT16(currentStatus.RPM, decoder.getRPM());
  toothCurrentCount = (unsigned int)toothAngles[SKIP_TOOTH4];
  TEST_ASSERT_EQUAL_UINT16(currentStatus.RPM, decoder.getRPM());

  // --- Running path: stdGetRPM(CRANK_SPEED)
  currentStatus.setRpm(currentStatus.crankRPM*2U);
  currentStatus.startRevolutions = 1; // not cranking
  decoderStatus.syncStatus = SyncStatus::Full;
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL; // revTime = 60000 -> 1000 RPM
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Fallback: when sync lost, stdGetRPM will return currentStatus.RPM (or crankingGetRPM returns currentStatus.RPM)
  decoderStatus.syncStatus = SyncStatus::None;
  currentStatus.setRpm(777);
  TEST_ASSERT_EQUAL_UINT16(777U, decoder.getRPM());
}

void testRoverMems(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRPM);
  }
}