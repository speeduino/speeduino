#include <unity.h>
#include "../test_utils.h"
#include "decoders.h"

extern bool sharedEngineIsRunning(uint32_t curTime);

static void test_sharedEngineIsRunning(void)
{
    extern volatile unsigned long toothLastToothTime;
    extern unsigned long MAX_STALL_TIME;
  
    MAX_STALL_TIME = 1000;
    toothLastToothTime = 0;
    TEST_ASSERT_TRUE(sharedEngineIsRunning(toothLastToothTime+MAX_STALL_TIME-1UL));
    TEST_ASSERT_FALSE(sharedEngineIsRunning(toothLastToothTime+MAX_STALL_TIME));
    TEST_ASSERT_FALSE(sharedEngineIsRunning(toothLastToothTime+MAX_STALL_TIME+1UL));

    // Simulate an interrupt for a pulse being triggered between a call 
    // to micros() (1000) and the call to engineIsRunning()
    toothLastToothTime = 2500;
    TEST_ASSERT_TRUE(sharedEngineIsRunning(1000UL));

    TEST_ASSERT_TRUE(sharedEngineIsRunning(2499UL));
    TEST_ASSERT_TRUE(sharedEngineIsRunning(2500UL));
    TEST_ASSERT_TRUE(sharedEngineIsRunning(2501UL));

    TEST_ASSERT_FALSE(sharedEngineIsRunning(toothLastToothTime+MAX_STALL_TIME));
}

void testDecoder_General()
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_sharedEngineIsRunning);
  }
}