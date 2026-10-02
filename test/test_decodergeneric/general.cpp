#include "../test_utils.h"
#include "src/decoders/details/decoder_state.h"

static void test_sharedEngineIsRunning(void)
{
    decoders::detail::state_t decoderState;

    decoderState.MAX_STALL_TIME = 1000;
    decoderState.toothLastToothTime = 0;
    TEST_ASSERT_TRUE(decoderState.toothWithinMaxStallTime(decoderState.toothLastToothTime+decoderState.MAX_STALL_TIME-1UL));
    TEST_ASSERT_FALSE(decoderState.toothWithinMaxStallTime(decoderState.toothLastToothTime+decoderState.MAX_STALL_TIME));
    TEST_ASSERT_FALSE(decoderState.toothWithinMaxStallTime(decoderState.toothLastToothTime+decoderState.MAX_STALL_TIME+1UL));

    // Simulate an interrupt for a pulse being triggered between a call
    // to micros() (1000) and the call to engineIsRunning(). The newer tooth
    // timestamp is accepted when it is within the stall interval.
    decoderState.toothLastToothTime = 1500;
    TEST_ASSERT_TRUE(decoderState.toothWithinMaxStallTime(1000UL));

    TEST_ASSERT_TRUE(decoderState.toothWithinMaxStallTime(1499UL));
    TEST_ASSERT_TRUE(decoderState.toothWithinMaxStallTime(1500UL));
    TEST_ASSERT_TRUE(decoderState.toothWithinMaxStallTime(1501UL));

    TEST_ASSERT_FALSE(decoderState.toothWithinMaxStallTime(decoderState.toothLastToothTime+decoderState.MAX_STALL_TIME));

    // A recent tooth remains valid across rollover, but expires normally.
    decoderState.toothLastToothTime = UINT32_MAX - 500UL;
    TEST_ASSERT_TRUE(decoderState.toothWithinMaxStallTime(400UL));  // 901 uS elapsed
    TEST_ASSERT_FALSE(decoderState.toothWithinMaxStallTime(600UL)); // 1101 uS elapsed
}

void testDecoder_General()
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_sharedEngineIsRunning);
  }
}
