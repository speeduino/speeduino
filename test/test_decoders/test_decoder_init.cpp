#include <unity.h>
#include "bit_manip.h"
#include "decoders.h"
#include "decoder_init.h"
#include "decoder_builder.h"
#include "../test_utils.h"
#include "globals.h"

static void assert_decoder(const decoder_t &decoder)
{
    auto defaultDecoder = decoder_builder_t().build();

    // Secondary is optional
    if (decoder.secondary.edge!=TRIGGER_EDGE_NONE)
    {
        TEST_ASSERT_NOT_EQUAL_MESSAGE(decoder.secondary.callback, defaultDecoder.secondary.callback, "secondary");
    }
    else
    {
        TEST_ASSERT_EQUAL_MESSAGE(decoder.secondary.callback, defaultDecoder.secondary.callback, "secondary");
    }
    
    // Tertiary is optional
    if (decoder.tertiary.edge!=TRIGGER_EDGE_NONE)
    {
        TEST_ASSERT_NOT_EQUAL_MESSAGE(decoder.tertiary.callback, defaultDecoder.tertiary.callback, "tertiary");
    }
    else
    {
        TEST_ASSERT_EQUAL_MESSAGE(decoder.tertiary.callback, defaultDecoder.tertiary.callback, "tertiary");
    }
    
    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(decoder.getRPM, defaultDecoder.getRPM, "getRPM");
    
    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(decoder.getCrankAngle, defaultDecoder.getCrankAngle, "getCrankAngle");

    // Per tooth ignition is optional
    if (decoder.getFeatures().supportsPerToothIgnition)
    {
        // TEST_ASSERT_TRUE(configPage2.perToothIgn);
        TEST_ASSERT_NOT_EQUAL_MESSAGE(decoder.setEndTeeth, defaultDecoder.setEndTeeth, "setEndTeeth");
    }
    else
    {
        // TEST_ASSERT_FALSE(configPage2.perToothIgn);
        TEST_ASSERT_EQUAL_MESSAGE(decoder.setEndTeeth, defaultDecoder.setEndTeeth, "setEndTeeth");
    }

    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(decoder.reset, defaultDecoder.reset, "reset");

    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(decoder.isEngineRunning, defaultDecoder.isEngineRunning, "isEngineRunning");

    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(decoder.getStatus, defaultDecoder.getStatus, "getStatus");

    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(decoder.getFeatures, defaultDecoder.getFeatures, "getFeatures");
}

extern decoder_t buildDecoder(uint8_t decoder);

static uint8_t decoderIdentifier;
static void test_buildDecoder(void)
{
    configPage2.perToothIgn = true;
    assert_decoder(buildDecoder(decoderIdentifier));
}

static void test_buildDecoder_all(void)
{
    configPage2.nCylinders = 4; // Needed to prevent division by zero on Renix.
    for (uint8_t decoder = 0; decoder < DECODER_MAX; ++decoder) {
        char szName[128];
        snprintf(szName, sizeof(szName), "test_buildDecoder_%d", decoder);
        decoderIdentifier = decoder;
        // setDecoder(decoder);
        UnityDefaultTestRun(test_buildDecoder, szName, __LINE__);
    }
}

static void test_setDecoder_attachesInterrupts(void)
{
    pinTrigger = 11;
    pinTrigger2 = 12;
    pinTrigger3 = 13;

    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
    configPage10.vvt2Enabled = true;

    setDecoder(DECODER_MISSING_TOOTH);
    assert_decoder(getDecoder());
    
    TEST_ASSERT_TRUE(getDecoder().primary.isValid());
    TEST_ASSERT_TRUE(getDecoder().secondary.isValid());
    TEST_ASSERT_TRUE(getDecoder().tertiary.isValid());
}

static void test_setDecoder_TurnsOffPerToothIgn(void)
{
    configPage2.perToothIgn = true;
    setDecoder(DECODER_MISSING_TOOTH);
    assert_decoder(getDecoder());
    TEST_ASSERT_TRUE(configPage2.perToothIgn);

    configPage2.perToothIgn = true;
    setDecoder(DECODER_HONDA_J32);
    assert_decoder(getDecoder());
    TEST_ASSERT_FALSE(configPage2.perToothIgn);
}

void testDecoderInit(void)
{
  SET_UNITY_FILENAME() {
    test_buildDecoder_all();
    RUN_TEST(test_setDecoder_attachesInterrupts);
    RUN_TEST(test_setDecoder_TurnsOffPerToothIgn);
  }
}