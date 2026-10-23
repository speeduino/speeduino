#include "../test_utils.h"
#include "src/decoders/basicDistributor.h"

static void test_getRevolutionTime(void)
{
  // Configure a 4-tooth distributor (4-cylinder cam-spaced)
  config2 page2={};
  config4 page4={};
  statuses current={};
  page2.nCylinders = 4;
  page2.strokes = FOUR_STROKE;
  auto decoderState = decoders::basic_distributor::initialise(page2);

  // Prepare for cranking path: RPM < crankRPM
  page4.StgCycles = 0;
  current.startRevolutions = 1;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  current.setRpm(0);
  current.crankRPM = 400;
  current.revolutionTime = 99999UL;
  decoderState.toothLastMinusOneToothTime = 1000UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 30000UL; // gap=30000 -> revTime = gap*totalTeeth/2 = 30000*4/2 = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::basic_distributor::getRevolutionTime(current, decoderState, page2, page4));

  // Running path: should return stdGetRevolutionTime(CAM_SPEED) -> current.revolutionTime when no toothOne* update
  current.setRpm(2000);
  decoderState.toothOneMinusOneTime = 0;
  decoderState.toothOneTime = 0;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_UINT32(99999UL, decoders::basic_distributor::getRevolutionTime(current, decoderState, page2, page4));

  page2.strokes = TWO_STROKE;
  TEST_ASSERT_EQUAL_UINT32(99999UL, decoders::basic_distributor::getRevolutionTime(current, decoderState, page2, page4));

  // If not synced, should return current.revolutionTime
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  current.setRpm(555);
  TEST_ASSERT_EQUAL_UINT32(99999UL, decoders::basic_distributor::getRevolutionTime(current, decoderState, page2, page4));
}

void testBasicDistribRevTime(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_getRevolutionTime);
}