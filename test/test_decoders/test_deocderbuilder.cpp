#include <unity.h>
#include "decoder_builder.h"
#include "../test_utils.h"

static void assert_decoder_builder(const decoder_builder_t& builder)
{
    decoder_t decoder = builder.build();

    TEST_ASSERT_NOT_NULL(decoder.primary.callback);
    TEST_ASSERT_NOT_NULL(decoder.secondary.callback);
    TEST_ASSERT_NOT_NULL(decoder.tertiary.callback);

    TEST_ASSERT_NOT_NULL(decoder.getRPM);
    TEST_ASSERT_NOT_NULL(decoder.getCrankAngle);
    TEST_ASSERT_NOT_NULL(decoder.setEndTeeth);
    TEST_ASSERT_NOT_NULL(decoder.reset);

    // Test these functions can be called without crashing
    decoder.primary.callback();
    decoder.secondary.callback();
    decoder.tertiary.callback();
    decoder.getRPM();
    decoder.getCrankAngle();
    decoder.setEndTeeth();
    decoder.reset();
}

static void test_ctor()
{
    assert_decoder_builder( decoder_builder_t() );
}

static uint8_t counter = 0;
static void triggerHandlerIncrement(void)
{
    counter++;
}

static void test_setPrimaryTrigger(void)
{
    auto builder = decoder_builder_t().setPrimaryTrigger( triggerHandlerIncrement, RISING );

    counter = 0;
    builder.build().primary.callback();
    TEST_ASSERT_EQUAL_UINT8( 1, counter );

    assert_decoder_builder( builder );

    builder.setPrimaryTrigger( nullptr, RISING );
    assert_decoder_builder( builder );
}

static void test_setSecondaryTrigger(void)
{
    auto builder = decoder_builder_t().setSecondaryTrigger( triggerHandlerIncrement, RISING );

    counter = 0;
    builder.build().secondary.callback();
    TEST_ASSERT_EQUAL_UINT8( 1, counter );

    assert_decoder_builder( builder );

    builder.setSecondaryTrigger( nullptr, RISING );
    assert_decoder_builder( builder );
}

static void test_setTertiaryTrigger(void)
{
    auto builder = decoder_builder_t().setTertiaryTrigger( triggerHandlerIncrement, RISING );

    counter = 0;
    builder.build().tertiary.callback();
    TEST_ASSERT_EQUAL_UINT8( 1, counter );

    assert_decoder_builder( builder );

    builder.setTertiaryTrigger( nullptr, RISING );
    assert_decoder_builder( builder );
}

static uint16_t incrementGetRPM(void)
{
    counter++;
    return 0;
}

static void test_setGetRPM(void)
{
    auto builder = decoder_builder_t().setGetRPM( incrementGetRPM );

    counter = 0;
    builder.build().getRPM();
    TEST_ASSERT_EQUAL_UINT8( 1, counter );

    assert_decoder_builder( builder );

    builder.setGetRPM( nullptr);
    assert_decoder_builder( builder );
}

static int incrementGetCrankAngle(void)
{
    counter++;
    return 0;
}

static void test_setGetCrankAngle(void)
{
    auto builder = decoder_builder_t().setGetCrankAngle( incrementGetCrankAngle );

    counter = 0;
    builder.build().getCrankAngle();
    TEST_ASSERT_EQUAL_UINT8( 1, counter );

    assert_decoder_builder( builder );

    builder.setGetRPM( nullptr);
    assert_decoder_builder( builder );
}

static void test_setSetEndTeeth(void)
{
    auto builder = decoder_builder_t().setSetEndTeeth( triggerHandlerIncrement );

    counter = 0;
    builder.build().setEndTeeth();
    TEST_ASSERT_EQUAL_UINT8( 1, counter );

    assert_decoder_builder( builder );

    builder.setSetEndTeeth( nullptr );
    assert_decoder_builder( builder );
}

static void test_setReset(void)
{
    auto builder = decoder_builder_t().setReset( triggerHandlerIncrement );

    counter = 0;
    builder.build().reset();
    TEST_ASSERT_EQUAL_UINT8( 1, counter );

    assert_decoder_builder( builder );

    builder.setReset( nullptr );
    assert_decoder_builder( builder );
}

void testDecoderBuilder(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST( test_ctor );
    RUN_TEST( test_setPrimaryTrigger );
    RUN_TEST( test_setSecondaryTrigger );
    RUN_TEST( test_setTertiaryTrigger );
    RUN_TEST( test_setGetRPM );
    RUN_TEST( test_setGetCrankAngle );
    RUN_TEST( test_setSetEndTeeth );
    RUN_TEST( test_setReset );
  }
}