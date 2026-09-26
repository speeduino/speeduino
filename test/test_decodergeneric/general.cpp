#include "../test_utils.h"
#include "decoders.h"
#include "globals.h"

extern bool sharedEngineIsRunning(uint32_t curTime);
extern uint32_t stdGetRevolutionTime(bool isCamTeeth);
extern uint32_t crankingGetRevolutionTime(byte totalTeeth, bool isCamTeeth);
extern volatile uint32_t toothLastToothTime;

static void test_stdGetRevolutionTime(void)
{
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;
  extern decoder_status_t decoderStatus;

  // Prepare state so the revolution time can be calculated
  decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.startRevolutions = 1; // not cranking
  currentStatus.setRpm(2000);
  currentStatus.revolutionTime = 12345UL;

  // Set tooth times such that revTime = 60000us
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(false));

  // Cam-speed: diff must be doubled so shifting gives same revTime
  toothOneMinusOneTime = 2000UL;
  toothOneTime = toothOneMinusOneTime + 120000UL; // >>1 -> 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, stdGetRevolutionTime(true));

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
  toothOneMinusOneTime = 0UL;
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(true));
  toothOneMinusOneTime = toothOneTime;
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(true));
  toothOneMinusOneTime = 2000UL;

  // No sync
  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(12345UL, stdGetRevolutionTime(false));
}

static void test_crankingGetRevolutionTime(void)
{
  extern volatile uint32_t toothLastMinusOneToothTime;
  extern decoder_status_t decoderStatus;

  // Ensure staging condition met
  configPage4.StgCycles = 0;
  currentStatus.startRevolutions = 0;
  decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.revolutionTime = 99999UL;

  // Crank-speed case: choose gap*teeth = 60000us
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 15000UL; // gap = 15000
  // totalTeeth = 4 -> revTime = 15000*4 = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, crankingGetRevolutionTime(4, CRANK_SPEED));

  // Cam-speed case: use totalTeeth=8 and isCam=true so (gap*8)>>1 = 60000
  toothLastMinusOneToothTime = 2000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 15000UL; // gap = 15000
  TEST_ASSERT_EQUAL_UINT32(60000UL, crankingGetRevolutionTime(8, CAM_SPEED));

  // Querying the revolution time must not change the current status
  TEST_ASSERT_EQUAL_UINT32(99999UL, currentStatus.revolutionTime);

  // Fallback: when staging not met or sync lost, should return currentStatus.revolutionTime
  configPage4.StgCycles = 1;
  TEST_ASSERT_EQUAL_UINT32(99999UL, crankingGetRevolutionTime(4, CRANK_SPEED));
  configPage4.StgCycles = 0;
  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(99999UL, crankingGetRevolutionTime(4, CRANK_SPEED));
}

static void test_sharedEngineIsRunning(void)
{
    extern unsigned long MAX_STALL_TIME;
  
    MAX_STALL_TIME = 1000;
    toothLastToothTime = 0;
    TEST_ASSERT_TRUE(sharedEngineIsRunning(toothLastToothTime+MAX_STALL_TIME-1UL));
    TEST_ASSERT_FALSE(sharedEngineIsRunning(toothLastToothTime+MAX_STALL_TIME));
    TEST_ASSERT_FALSE(sharedEngineIsRunning(toothLastToothTime+MAX_STALL_TIME+1UL));

    // Simulate an interrupt for a pulse being triggered between a call
    // to micros() (1000) and the call to engineIsRunning(). The newer tooth
    // timestamp is accepted when it is within the stall interval.
    toothLastToothTime = 1500;
    TEST_ASSERT_TRUE(sharedEngineIsRunning(1000UL));

    TEST_ASSERT_TRUE(sharedEngineIsRunning(1499UL));
    TEST_ASSERT_TRUE(sharedEngineIsRunning(1500UL));
    TEST_ASSERT_TRUE(sharedEngineIsRunning(1501UL));

    TEST_ASSERT_FALSE(sharedEngineIsRunning(toothLastToothTime+MAX_STALL_TIME));

    // A recent tooth remains valid across rollover, but expires normally.
    toothLastToothTime = UINT32_MAX - 500UL;
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
