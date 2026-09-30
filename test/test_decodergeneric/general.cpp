#include "../test_utils.h"
#include "decoders.h"
#include "globals.h"
#include "src/decoders/decoder_state.h"

extern decoders::detail::state_t _decoderState;

extern bool sharedEngineIsRunning(uint32_t curTime);
extern uint32_t stdGetRevolutionTime(bool isCamTeeth);
extern uint32_t crankingGetRevolutionTime(byte totalTeeth, bool isCamTeeth);

static void test_stdGetRevolutionTime(void)
{
  // Prepare state so the revolution time can be calculated
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.startRevolutions = 1; // not cranking
  currentStatus.setRpm(2000);
  currentStatus.revolutionTime = 12345UL;

  // Set tooth times such that revTime = 60000us
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(false));

  // Cam-speed: diff must be doubled so shifting gives same revTime
  _decoderState.toothOneMinusOneTime = 2000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 120000UL; // >>1 -> 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(true));

  // micros() wrapped between the 2 tooth #1 times
  _decoderState.toothOneMinusOneTime = UINT32_MAX - 999UL;
  _decoderState.toothOneTime = 59000UL; // 60000uS after _decoderState.toothOneMinusOneTime
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(false));
  _decoderState.toothOneMinusOneTime = 2000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 120000UL;

  // Querying the revolution time must not change the current status
  TEST_ASSERT_EQUAL_UINT32(12345UL, currentStatus.revolutionTime);
  TEST_ASSERT_EQUAL_UINT16(2000U, currentStatus.RPM);

  // Fallback paths: when the revolution time can't be calculated, return currentStatus.revolutionTime
  // Cranking
  currentStatus.startRevolutions = 0;
  currentStatus.crankRPM = 400;
  currentStatus.setRpm(currentStatus.crankRPM-1U);
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(true));
  currentStatus.startRevolutions = 1;
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(true));

  // No tooth #1 history
  _decoderState.toothOneMinusOneTime = 0UL;
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(true));
  _decoderState.toothOneMinusOneTime = _decoderState.toothOneTime;
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(true));
  _decoderState.toothOneMinusOneTime = 2000UL;

  // No sync
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(false));
}

static void test_crankingGetRevolutionTime(void)
{
  // Ensure staging condition met
  configPage4.StgCycles = 0;
  currentStatus.startRevolutions = 0;
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.revolutionTime = 99999UL;

  // Crank-speed case: choose gap*teeth = 60000us
  _decoderState.toothLastMinusOneToothTime = 1000UL;
  _decoderState.toothLastToothTime = _decoderState.toothLastMinusOneToothTime + 15000UL; // gap = 15000
  // totalTeeth = 4 -> revTime = 15000*4 = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, crankingGetRevolutionTime(4, CRANK_SPEED));

  // micros() wrapped between the 2 tooth times
  _decoderState.toothLastMinusOneToothTime = UINT32_MAX - 999UL;
  _decoderState.toothLastToothTime = 14000UL; // gap = 15000
  TEST_ASSERT_EQUAL_UINT32(60000UL, crankingGetRevolutionTime(4, CRANK_SPEED));

  // Cam-speed case: use totalTeeth=8 and isCam=true so (gap*8)>>1 = 60000
  _decoderState.toothLastMinusOneToothTime = 2000UL;
  _decoderState.toothLastToothTime = _decoderState.toothLastMinusOneToothTime + 15000UL; // gap = 15000
  TEST_ASSERT_EQUAL_UINT32(60000UL, crankingGetRevolutionTime(8, CAM_SPEED));

  // Querying the revolution time must not change the current status
  TEST_ASSERT_EQUAL_UINT32(99999UL, currentStatus.revolutionTime);

  // Fallback: when staging not met or sync lost, should return currentStatus.revolutionTime
  configPage4.StgCycles = 1;
  TEST_ASSERT_EQUAL_UINT32(99999UL, crankingGetRevolutionTime(4, CRANK_SPEED));
  configPage4.StgCycles = 0;
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(99999UL, crankingGetRevolutionTime(4, CRANK_SPEED));
}

static void test_sharedEngineIsRunning(void)
{
    _decoderState.MAX_STALL_TIME = 1000;
    _decoderState.toothLastToothTime = 0;
    TEST_ASSERT_TRUE(sharedEngineIsRunning(_decoderState.toothLastToothTime+_decoderState.MAX_STALL_TIME-1UL));
    TEST_ASSERT_FALSE(sharedEngineIsRunning(_decoderState.toothLastToothTime+_decoderState.MAX_STALL_TIME));
    TEST_ASSERT_FALSE(sharedEngineIsRunning(_decoderState.toothLastToothTime+_decoderState.MAX_STALL_TIME+1UL));

    // Simulate an interrupt for a pulse being triggered between a call
    // to micros() (1000) and the call to engineIsRunning(). The newer tooth
    // timestamp is accepted when it is within the stall interval.
    _decoderState.toothLastToothTime = 1500;
    TEST_ASSERT_TRUE(sharedEngineIsRunning(1000UL));

    TEST_ASSERT_TRUE(sharedEngineIsRunning(1499UL));
    TEST_ASSERT_TRUE(sharedEngineIsRunning(1500UL));
    TEST_ASSERT_TRUE(sharedEngineIsRunning(1501UL));

    TEST_ASSERT_FALSE(sharedEngineIsRunning(_decoderState.toothLastToothTime+_decoderState.MAX_STALL_TIME));

    // A recent tooth remains valid across rollover, but expires normally.
    _decoderState.toothLastToothTime = UINT32_MAX - 500UL;
    TEST_ASSERT_TRUE(sharedEngineIsRunning(400UL));  // 901 uS elapsed
    TEST_ASSERT_FALSE(sharedEngineIsRunning(600UL)); // 1101 uS elapsed
}

void testDecoder_General()
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_sharedEngineIsRunning);
    RUN_TEST_P(test_stdGetRevolutionTime);
    RUN_TEST_P(test_crankingGetRevolutionTime);
  }
}
