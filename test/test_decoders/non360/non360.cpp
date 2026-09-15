#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

extern volatile uint32_t toothLastToothTime;
extern volatile int toothCurrentCount;
extern volatile unsigned long toothLastMinusOneToothTime;
extern volatile unsigned long toothOneTime;
extern volatile unsigned long toothOneMinusOneTime;
extern decoder_status_t decoderStatus;

static void test_getCrankAngle(void)
{
  // Configure a simple non360 wheel: 8 teeth, no multiplier
  configPage4.triggerTeeth = 8;
  configPage4.TrigAngMul = 1;
  configPage4.triggerAngle = 0;

  auto decoder = triggerSetup_non360();

  // Deterministic angle conversion
  setAngleConverterRevolutionTime(2000);

  toothLastToothTime = 10000;

  // tooth 1 -> base 0
  toothCurrentCount = 1;
  TEST_ASSERT_EQUAL(18, decoder.pGetCrankAngle(toothLastToothTime + 100));

  // tooth 0 -> treated as last tooth (triggerTeeth)
  toothCurrentCount = 0;
  TEST_ASSERT_EQUAL(351, decoder.pGetCrankAngle(toothLastToothTime + 200));

  // Test with TrigAngMul >1 and different tooth count
  configPage4.triggerTeeth = 10;
  configPage4.TrigAngMul = 2;
  configPage4.triggerAngle = 0;
  decoder = triggerSetup_non360(); // recompute triggerToothAngle
  toothLastToothTime = 20000;
  toothCurrentCount = 3;
  TEST_ASSERT_EQUAL(81, decoder.pGetCrankAngle(toothLastToothTime + 50));

  // Wrap-around: force triggerAngle large so result >=720
  configPage4.triggerTeeth = 8;
  configPage4.TrigAngMul = 1;
  configPage4.triggerAngle = 500;
  decoder = triggerSetup_non360();
  toothLastToothTime = 30000;
  toothCurrentCount = 8; // base = 7*45 = 315 -> +500 = 815 >=720 -> subtract 720 -> 95
  TEST_ASSERT_EQUAL(97, decoder.pGetCrankAngle(toothLastToothTime + 10));
}

static void test_getRPM(void)
{
  // Configure wheel and build decoder
  configPage4.triggerTeeth = 8;
  auto decoder = triggerSetup_non360();

  // Ensure staging allows cranking calculation
  configPage4.StgCycles = 0;

  // --- Cranking path: currentStatus.RPM < crankRPM -> crankingGetRPM
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 400;
  currentStatus.startRevolutions = 0; // cranking
  currentStatus.revolutionTime = UINT32_MAX; // Ensure this changes
  decoderStatus.syncStatus = SyncStatus::Full;
  toothCurrentCount = 1;
  toothLastMinusOneToothTime = 1000UL;
  // Choose gap so (gap * triggerTeeth) == 60000us -> 1000 RPM
  toothLastToothTime = toothLastMinusOneToothTime + (60000UL / configPage4.triggerTeeth); // 60000/8 = 7500
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Running path: use stdGetRPM(CRANK_SPEED) via toothOne pair
  currentStatus.setRpm(2000);
  currentStatus.startRevolutions = 1; // not cranking
  currentStatus.revolutionTime = UINT32_MAX; // Ensure this changes
  decoderStatus.syncStatus = SyncStatus::Full;
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL; // revTime = 60000 -> 1000 RPM
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Fallback: when sync lost, expect 0
  decoderStatus.syncStatus = SyncStatus::None;
  currentStatus.revolutionTime = UINT32_MAX; // Ensure this changes
  TEST_ASSERT_EQUAL_UINT16(0U, decoder.getRPM());

}

void testNon360(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}