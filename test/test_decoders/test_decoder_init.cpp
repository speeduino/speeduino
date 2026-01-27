#include <unity.h>
#include "bit_manip.h"
#include "decoders.h"
#include "decoder_init.h"
#include "decoder_builder.h"
#include "../test_utils.h"
#include "globals.h"

static void test_init(void)
{
    auto defaultDecoder = decoder_builder_t().build();
    // Should have at least a primary trigger
    TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().primary.callback, defaultDecoder.primary.callback, "primary");

    // Secondary is optional
    if (getDecoder().secondary.isValid())
    {
        TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().secondary.callback, defaultDecoder.secondary.callback, "secondary");
    }
    else
    {
        TEST_ASSERT_EQUAL_MESSAGE(getDecoder().secondary.callback, defaultDecoder.secondary.callback, "secondary");
    }
    TEST_ASSERT_EQUAL_MESSAGE(getDecoder().secondary.isValid(), getDecoder().getFeatures().hasSecondary, "hasSecondary");
    
    // Tertiary is optional
    if (getDecoder().tertiary.isValid())
    {
        TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().tertiary.callback, defaultDecoder.tertiary.callback, "tertiary");
    }
    else
    {
        TEST_ASSERT_EQUAL_MESSAGE(getDecoder().tertiary.callback, defaultDecoder.tertiary.callback, "tertiary");
    }
    
    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().getRPM, defaultDecoder.getRPM, "getRPM");
    
    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().getCrankAngle, defaultDecoder.getCrankAngle, "getCrankAngle");

    // Per tooth ignition is optional
    if (getDecoder().getFeatures().supportsPerToothIgnition)
    {
        TEST_ASSERT_TRUE(configPage2.perToothIgn);
        TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().setEndTeeth, defaultDecoder.setEndTeeth, "setEndTeeth");
    }
    else
    {
        TEST_ASSERT_FALSE(configPage2.perToothIgn);
        TEST_ASSERT_EQUAL_MESSAGE(getDecoder().setEndTeeth, defaultDecoder.setEndTeeth, "setEndTeeth");
    }

    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().reset, defaultDecoder.reset, "reset");

    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().isEngineRunning, defaultDecoder.isEngineRunning, "isEngineRunning");

    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().getStatus, defaultDecoder.getStatus, "getStatus");

    // Mandatory
    TEST_ASSERT_NOT_EQUAL_MESSAGE(getDecoder().getFeatures, defaultDecoder.getFeatures, "getFeatures");
}

static void test_init_all(void)
{
    configPage2.nCylinders = 4; // Needed to prevent division by zero on Renix.
    for (uint8_t decoder = 0; decoder < DECODER_MAX; ++decoder) {
        char szName[128];
        snprintf(szName, sizeof(szName), "test_init_%d", decoder);
        configPage2.perToothIgn = true;
        setDecoder(decoder);
        UnityDefaultTestRun(test_init, szName, __LINE__);
    }
}

void testDecoderInit(void)
{
  SET_UNITY_FILENAME() {
    test_init_all();
  }
}