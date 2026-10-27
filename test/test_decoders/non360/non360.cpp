#include "../test_utils.h"
#include "src/decoders/non360.h"


static void test_getCrankAngle(void)
{
  // Configure a simple non360 wheel: 8 teeth, no multiplier
  config4 page4 = {};
  page4.triggerTeeth = 8;
  page4.TrigAngMul = 1;
  auto decoderState = decoders::non360::initialise(page4);

  // Deterministic angle conversion
  setAngleConverterRevolutionTime(2000);

  decoderState.toothLastToothTime = 10000;

  // tooth 1 -> base 0
  decoderState.toothCurrentCount = 1;
  TEST_ASSERT_EQUAL(18, decoders::non360::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));

  // tooth 0 -> treated as last tooth (triggerTeeth)
  decoderState.toothCurrentCount = 0;
  TEST_ASSERT_EQUAL(351, decoders::non360::getCrankAngle(decoderState.toothLastToothTime + 200, decoderState, page4));

  // Test with TrigAngMul >1 and different tooth count
  page4.triggerTeeth = 10;
  page4.TrigAngMul = 2;
  page4.triggerAngle = 0;
  decoderState = decoders::non360::initialise(page4); // recompute triggerToothAngle
  decoderState.toothLastToothTime = 20000;
  decoderState.toothCurrentCount = 3;
  TEST_ASSERT_EQUAL(81, decoders::non360::getCrankAngle(decoderState.toothLastToothTime + 50, decoderState, page4));

  // Wrap-around: force triggerAngle large so result >=720
  page4.triggerTeeth = 8;
  page4.TrigAngMul = 1;
  page4.triggerAngle = 500;
  decoderState = decoders::non360::initialise(page4);
  decoderState.toothLastToothTime = 30000;
  decoderState.toothCurrentCount = 8; // base = 7*45 = 315 -> +500 = 815 >=720 -> subtract 720 -> 95
  TEST_ASSERT_EQUAL(97, decoders::non360::getCrankAngle(decoderState.toothLastToothTime + 10, decoderState, page4));
}

static void test_getRevolutionTime(void)
{
  // Configure wheel and build decoder
  statuses current={};
  config4 page4 = {};
  page4.triggerTeeth = 8;
  // page4.TrigAngMul = 1;
  auto decoderState = decoders::non360::initialise(page4);

  // Ensure staging allows cranking calculation
  page4.StgCycles = 0;

  // --- Cranking path: current.RPM < crankRPM -> crankingGetRevolutionTime
  current.setRpm(0);
  current.crankRPM = 400;
  current.startRevolutions = 0; // cranking
  current.revolutionTime = UINT32_MAX;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 1;
  decoderState.toothLastMinusOneToothTime = 1000UL;
  // Choose gap so (gap * triggerTeeth) == 60000us
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + (60000UL / page4.triggerTeeth); // 60000/8 = 7500
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::non360::getRevolutionTime(current, decoderState, page4));

  // --- Running path: use stdGetRevolutionTime(CRANK_SPEED) via toothOne pair
  current.setRpm(2000);
  current.startRevolutions = 1; // not cranking
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 60000UL; // revTime = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::non360::getRevolutionTime(current, decoderState, page4));

  // --- Fallback: when sync lost, the speed is unknown
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoders::non360::getRevolutionTime(current, decoderState, page4));
}

void testNon360(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}