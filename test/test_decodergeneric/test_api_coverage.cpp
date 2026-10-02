#include "decoder_init.h"
#include "globals.h"
#include "../test_utils.h"
#include "decoder_name.h"
#include "shared.h"
#include "src/pins/boardInputPin.h"

extern decoder_status_t decoderStatus;
extern volatile unsigned long toothOneTime;
extern volatile unsigned long toothOneMinusOneTime;
extern volatile uint32_t toothLastToothTime;
extern volatile unsigned long toothLastMinusOneToothTime;
extern uint16_t toothCurrentCount;
extern unsigned long MAX_STALL_TIME;

static void test_primary_trigger(decoder_t &decoder, uint8_t decoderNum)
{
    char szMsg[64] = {};
    snprintf(szMsg, _countof(szMsg), "%" PRIu8, decoderNum);

    if (DECODER_NGC==decoderNum)
    {
        configurePinState(decoder.primary._pin, FALLING);
    }
    else
    {
        configurePinState(decoder.primary._pin, decoder.primary.edge);
    }

    decoder.reset();
    configureStateForPrimaryTrigger(decoderNum, decoderStatus);
    TEST_ASSERT_FALSE_MESSAGE(decoder.getStatus().validTrigger, szMsg);

    decoder.primary.callback();
    if (DECODER_RENIX!=decoderNum   // See issue #1347
        && DECODER_ROVERMEMS!=decoderNum) { // See issue #1348
        TEST_ASSERT_TRUE_MESSAGE(decoder.getStatus().validTrigger, szMsg);
    }
}

static void setup_prebuild_state(void)
{
    pinNumbers.pinTrigger = 18;
    pinNumbers.pinTrigger2 = 19;
    pinNumbers.pinTrigger3 = 20;
    configPage4.TrigEdge = 1;
    configPage4.triggerTeeth = 31;
    configPage4.triggerAngle = 77;
    configPage4.TrigAngMul = 3;
    currentStatus.initialisationComplete = false;
}

static void test_primary_trigger(uint8_t decoderNum)
{
    setup_prebuild_state();
    currentStatus.decoder = buildDecoder(decoderNum);

    test_primary_trigger(currentStatus.decoder, decoderNum);
}

static void test_secondary_trigger_coverage(uint8_t decoderNum)
{
    setup_prebuild_state();
    auto decoder = buildDecoder(decoderNum);

    decoder.secondary.callback();

    TEST_PASS(); // Coverage only
}

static void test_tertiary_trigger_coverage(uint8_t decoderNum)
{
    setup_prebuild_state();
    auto decoder = buildDecoder(decoderNum);

    decoder.tertiary.callback();

    TEST_PASS(); // Coverage only
}

// Unlike older decoders, Optispark also requires private interval history.
// Establish it through real edges instead of fabricating a synchronized status.
static uint32_t primeOptispark8(void)
{
    extern void processOptispark8Edge(uint32_t now, bool pinHigh);
    const uint16_t edges[] = {86,100,176,180,266,290,356,360,446,480,536,540,626,670,716,720};
    uint32_t now = 1000;
    for (uint8_t n = 0; n < 32U; ++n)
    {
        const uint8_t i = n % 16U;
        now += (uint32_t)(i == 0U ? 86U : edges[i] - edges[i - 1U]) * 100U;
        processOptispark8Edge(now, ((i & 1U) == 0U) != (configPage4.TrigEdge != 0U));
    }
    return now;
}

static void test_getRpm_coverage(uint8_t decoderNum)
{
    setup_prebuild_state();
    auto decoder = buildDecoder(decoderNum);
    if (decoderNum == DECODER_OPTISPARK_8)
    {
        primeOptispark8();
        TEST_ASSERT_UINT16_WITHIN(1, 1667, decoder.getRPM());
        return;
    }

    currentStatus.revolutionTime = 3333;
    currentStatus.crankRPM = 400;
    currentStatus.setRpm(currentStatus.crankRPM*3U);
    decoderStatus.syncStatus = SyncStatus::Full; 
    toothLastMinusOneToothTime = 1111;
    toothOneMinusOneTime = toothLastMinusOneToothTime*5U;
    toothLastToothTime = 5555;
    toothOneTime = toothLastToothTime * 3U;
    TEST_ASSERT_NOT_EQUAL(0, decoder.getRPM());
}

static void test_getCrankAngle_coverage(uint8_t decoderNum)
{
    setup_prebuild_state();
    auto decoder = buildDecoder(decoderNum);
    if (decoderNum == DECODER_OPTISPARK_8)
    {
        const uint32_t now = primeOptispark8();
        TEST_ASSERT_EQUAL_INT16(configPage4.triggerAngle + 5, decoder.pGetCrankAngle(now + 500U));
        return;
    }

    currentStatus.revolutionTime = 3333;
    currentStatus.crankRPM = 400;
    currentStatus.setRpm(currentStatus.crankRPM*3U);
    decoderStatus.syncStatus = SyncStatus::Full; 
    toothLastMinusOneToothTime = 1111;
    toothOneMinusOneTime = toothLastMinusOneToothTime*5U;
    toothLastToothTime = 5555;
    toothOneTime = toothLastToothTime * 3U;
    toothCurrentCount = configPage4.triggerTeeth / 2U;

    TEST_ASSERT_NOT_EQUAL(0, decoder.getCrankAngle());
}

static void test_setEndTeeth_coverage(uint8_t decoderNum)
{
    setup_prebuild_state();
    auto decoder = buildDecoder(decoderNum);

    decoder.setEndTeeth();

    TEST_PASS(); // Coverage only
}

static void test_reset_coverage(uint8_t decoderNum)
{
    setup_prebuild_state();
    auto decoder = buildDecoder(decoderNum);

    decoder.reset();

    TEST_PASS(); // Coverage only
}

static void test_isEngineRunning_coverage(uint8_t decoderNum)
{
    setup_prebuild_state();
    auto decoder = buildDecoder(decoderNum);

    toothLastToothTime = 3333;
    TEST_ASSERT_TRUE(decoder.isEngineRunning(toothLastToothTime+(MAX_STALL_TIME/2U)));

    TEST_PASS(); // Coverage only
}

static uint8_t decoderToTest;
static void test_primary_trigger(void)
{
    test_primary_trigger(decoderToTest);
}
static void test_secondary_trigger_coverage(void)
{
    test_secondary_trigger_coverage(decoderToTest);
}
static void test_tertiary_trigger_coverage(void)
{
    test_tertiary_trigger_coverage(decoderToTest);
}
static void test_getRpm_coverage(void)
{
    test_getRpm_coverage(decoderToTest);
}
static void test_getCrankAngle_coverage(void)
{
    test_getCrankAngle_coverage(decoderToTest);
}
static void test_setEndTeeth_coverage(void)
{
    test_setEndTeeth_coverage(decoderToTest);
}
static void test_reset_coverage(void)
{
    test_reset_coverage(decoderToTest);
}
static void test_isEngineRunning_coverage(void)
{
    test_isEngineRunning_coverage(decoderToTest);
}

void testDecoderApiCoverage(void)
{
  SET_UNITY_FILENAME() {
    // Call every API method of evey decoder.
    for (uint8_t decoder = 0; decoder < DECODER_MAX; ++decoder)
    {
        decoderToTest = decoder;
        auto decoderName = getDecoderName(decoder);

        RUN_TEST_POSTFIX_P(test_primary_trigger, decoderName);
        RUN_TEST_POSTFIX_P(test_secondary_trigger_coverage, decoderName);
        RUN_TEST_POSTFIX_P(test_tertiary_trigger_coverage, decoderName);
        RUN_TEST_POSTFIX_P(test_getRpm_coverage, decoderName);
        RUN_TEST_POSTFIX_P(test_getCrankAngle_coverage, decoderName);
        RUN_TEST_POSTFIX_P(test_setEndTeeth_coverage, decoderName);
        RUN_TEST_POSTFIX_P(test_reset_coverage, decoderName);
        RUN_TEST_POSTFIX_P(test_isEngineRunning_coverage, decoderName);
    }
  }
}