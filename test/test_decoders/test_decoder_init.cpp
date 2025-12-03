#include <unity.h>
#include "decoder_init.h"
#include "decoder_builder.h"
#include "../test_utils.h"

static void test_init_all(void)
{
    auto defaultDecoder = decoder_builder_t().build();
    for (uint8_t decoder = 0; decoder < DECODER_MAX; ++decoder) {
        setDecoder(decoder);

        char szMsg[64];
        snprintf(szMsg, sizeof(szMsg), "Decoder %d, primary", decoder);

        // Should have at least a primary trigger
        TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().primary.callback, defaultDecoder.primary.callback, szMsg);
        snprintf(szMsg, sizeof(szMsg), "Decoder %d, secondary", decoder);
        if (getDecoder().secondary.isValid())
        {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().secondary.callback, defaultDecoder.secondary.callback, szMsg);
        } else
        {
            TEST_ASSERT_EQUAL_MESSAGE(getDecoder().secondary.callback, defaultDecoder.secondary.callback, szMsg);
        }
        snprintf(szMsg, sizeof(szMsg), "Decoder %d, tertiary", decoder);
        if (getDecoder().tertiary.isValid())
        {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().tertiary.callback, defaultDecoder.secondary.callback, szMsg);
        }
        else
        {
            TEST_ASSERT_EQUAL_MESSAGE(getDecoder().tertiary.callback, defaultDecoder.secondary.callback, szMsg);
        }
        snprintf(szMsg, sizeof(szMsg), "Decoder %d, getRPM", decoder);
        TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().getRPM, defaultDecoder.getRPM, szMsg);
        snprintf(szMsg, sizeof(szMsg), "Decoder %d, getCrankAngle", decoder);
        TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().getCrankAngle, defaultDecoder.getCrankAngle, szMsg);
        snprintf(szMsg, sizeof(szMsg), "Decoder %d, setEndTeeth", decoder);
        if (decoder!=5 && decoder!=6 && decoder!=7 && decoder!=8 && decoder!=10 && decoder!=11 && decoder!=14 && decoder!=15 && decoder!=23 && decoder!=27)
        {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().setEndTeeth, defaultDecoder.setEndTeeth, szMsg);
        }
        else
        {
            TEST_ASSERT_EQUAL_MESSAGE(getDecoder().setEndTeeth, defaultDecoder.setEndTeeth, szMsg);
        }
        snprintf(szMsg, sizeof(szMsg), "Decoder %d, reset", decoder);
        TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().reset, defaultDecoder.reset, szMsg);
        snprintf(szMsg, sizeof(szMsg), "Decoder %d, isEngineRunning", decoder);
        TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().isEngineRunning, defaultDecoder.isEngineRunning, szMsg);
    }
}

void testDecoderInit(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST( test_init_all );
  }
}