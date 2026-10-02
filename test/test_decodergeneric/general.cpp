#include "../test_utils.h"
#include "src/decoders/details/decoder_state.h"

extern decoders::detail::state_t _decoderState;
extern bool sharedEngineIsRunning(uint32_t curTime);

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
  }
}
