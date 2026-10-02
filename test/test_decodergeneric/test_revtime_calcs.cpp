#include "../test_utils.h"
#include "src/decoders/details/rev_time_calcs.h"

using namespace decoders::detail;

static void test_stdGetRevolutionTime(void)
{
  statuses current;
  state_t decoderState;
  
  // Prepare state so the revolution time can be calculated
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  current.startRevolutions = 1; // not cranking
  current.setRpm(2000);
  current.revolutionTime = 12345UL;

  // Set tooth times such that revTime = 60000us
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 60000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(current, decoderState,false));

  // Cam-speed: diff must be doubled so shifting gives same revTime
  decoderState.toothOneMinusOneTime = 2000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 120000UL; // >>1 -> 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(current, decoderState,true));

  // micros() wrapped between the 2 tooth #1 times
  decoderState.toothOneMinusOneTime = UINT32_MAX - 999UL;
  decoderState.toothOneTime = 59000UL; // 60000uS after decoderState.toothOneMinusOneTime
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(current, decoderState,false));
  decoderState.toothOneMinusOneTime = 2000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 120000UL;

  // Querying the revolution time must not change the current status
  TEST_ASSERT_EQUAL_UINT32(12345UL, current.revolutionTime);
  TEST_ASSERT_EQUAL_UINT16(2000U, current.RPM);

  // Fallback paths: when the revolution time can't be calculated, return current.revolutionTime
  // Cranking
  current.startRevolutions = 0;
  current.crankRPM = 400;
  current.setRpm(current.crankRPM-1U);
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(current, decoderState,true));
  current.startRevolutions = 1;
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(current, decoderState,true));

  // No tooth #1 history
  decoderState.toothOneMinusOneTime = 0UL;
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(current, decoderState,true));
  decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(current, decoderState,true));
  decoderState.toothOneMinusOneTime = 2000UL;

  // No sync
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(current, decoderState,false));
}

static void test_crankingGetRevolutionTime(void)
{
  statuses current;
  state_t decoderState;
  config4 page4;
  
  // Ensure staging condition met
  page4.StgCycles = 0;
  current.startRevolutions = 0;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  current.revolutionTime = 99999UL;

  // Crank-speed case: choose gap*teeth = 60000us
  decoderState.toothLastMinusOneToothTime = 1000UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 15000UL; // gap = 15000
  // totalTeeth = 4 -> revTime = 15000*4 = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, crankingGetRevolutionTime(current, decoderState, page4, 4, CRANK_SPEED));

  // micros() wrapped between the 2 tooth times
  decoderState.toothLastMinusOneToothTime = UINT32_MAX - 999UL;
  decoderState.toothLastToothTime = 14000UL; // gap = 15000
  TEST_ASSERT_EQUAL_UINT32(60000UL, crankingGetRevolutionTime(current, decoderState, page4, 4, CRANK_SPEED));

  // Cam-speed case: use totalTeeth=8 and isCam=true so (gap*8)>>1 = 60000
  decoderState.toothLastMinusOneToothTime = 2000UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 15000UL; // gap = 15000
  TEST_ASSERT_EQUAL_UINT32(60000UL, crankingGetRevolutionTime(current, decoderState, page4, 8, CAM_SPEED));

  // Querying the revolution time must not change the current status
  TEST_ASSERT_EQUAL_UINT32(99999UL, current.revolutionTime);

  // Fallback: when staging not met or sync lost, should return current.revolutionTime
  page4.StgCycles = 1;
  TEST_ASSERT_EQUAL_UINT32(99999UL, crankingGetRevolutionTime(current, decoderState, page4, 4, CRANK_SPEED));
  page4.StgCycles = 0;
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(99999UL, crankingGetRevolutionTime(current, decoderState, page4, 4, CRANK_SPEED));
}

void testRevTimeCalcs(void)
{
    unity_filename_guard_t guard(__FILE__);
  
    RUN_TEST_P(test_stdGetRevolutionTime);
    RUN_TEST_P(test_crankingGetRevolutionTime);
}
