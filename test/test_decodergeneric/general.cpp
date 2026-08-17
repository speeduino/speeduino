#include "../test_utils.h"
#include "decoders.h"
#include "globals.h"

extern bool sharedEngineIsRunning(uint32_t curTime);
extern uint16_t stdGetRPM(bool isCamTeeth);
extern int crankingGetRPM(byte totalTeeth, bool isCamTeeth);
extern volatile uint32_t toothLastToothTime;

static void test_stdGetRPM(void)
{  
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;
  extern decoder_status_t decoderStatus;

  // Prepare state so UpdateRevolutionTimeFromTeeth will succeed
  decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.startRevolutions = 1; // not cranking
  currentStatus.setRpm(2000);
  // ensure revolutionTime will be changed
  currentStatus.revolutionTime = 12345UL;

  // Set tooth times such that revTime = 60000us => 1000 RPM
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL;

  TEST_ASSERT_EQUAL_UINT16(1000U, stdGetRPM(false));

  // ensure revolutionTime will be changed
  currentStatus.revolutionTime = 12345UL;
  // Cam-speed: diff must be doubled so shifting gives same revTime
  toothOneMinusOneTime = 2000UL;
  toothOneTime = toothOneMinusOneTime + 120000UL; // >>1 -> 60000
  TEST_ASSERT_EQUAL_UINT16(1000U, stdGetRPM(true));

  // ensure revolutionTime will be changed
  currentStatus.revolutionTime = 12345UL;
  // Fallback path: when update can't run, return currentStatus.RPM
  decoderStatus.syncStatus = SyncStatus::None;
  currentStatus.setRpm(777);
  TEST_ASSERT_EQUAL_UINT16(777U, stdGetRPM(false));
}

static void test_crankingGetRPM(void)
{
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern decoder_status_t decoderStatus;

  // Ensure staging condition met
  configPage4.StgCycles = 0;
  currentStatus.startRevolutions = 0;
  decoderStatus.syncStatus = SyncStatus::Full;

  // Make revolutionTime different so SetRevolutionTime returns true
  currentStatus.revolutionTime = 99999UL;

  // Crank-speed case: choose gap*teeth = 60000us -> 1000 RPM
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 15000UL; // gap = 15000
  // totalTeeth = 4 -> revTime = 15000*4 = 60000
  TEST_ASSERT_EQUAL_INT(1000, crankingGetRPM(4, CRANK_SPEED));

  // Cam-speed case: use totalTeeth=8 and isCam=true so (gap*8)>>1 = 60000
  currentStatus.revolutionTime = 99999UL; // reset so SetRevolutionTime triggers
  toothLastMinusOneToothTime = 2000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 15000UL; // gap = 15000
  TEST_ASSERT_EQUAL_INT(1000, crankingGetRPM(8, CAM_SPEED));

  // Fallback: when sync lost or staging not met, should return currentStatus.RPM
  decoderStatus.syncStatus = SyncStatus::None;
  currentStatus.setRpm(555);
  TEST_ASSERT_EQUAL_INT(555, crankingGetRPM(4, CRANK_SPEED));
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
    RUN_TEST_P(test_stdGetRPM);
    RUN_TEST_P(test_crankingGetRPM);
  }
}
