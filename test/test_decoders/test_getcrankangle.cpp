#include "decoder_builder.h"
#include "decoder_init.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "decoder_name.h"

extern decoder_status_t decoderStatus;
extern volatile unsigned long toothLastToothTime;
extern volatile unsigned long toothLastMinusOneToothTime;
extern volatile uint16_t triggerToothAngle;
extern volatile uint8_t toothCurrentCount;
extern volatile bool revolutionOne; 

extern uint16_t timeToAngleIntervalTooth(uint32_t time);

static void setup_toothInterval(void)
{
    toothLastToothTime = 2000;
    toothLastMinusOneToothTime = toothLastToothTime-500;
    triggerToothAngle = 90;
    toothCurrentCount = 1;
    revolutionOne = false;    
    decoderStatus.toothAngleIsCorrect = true;
    setAngleConverterRevolutionTime(2000);
}

static uint8_t decoderToTest = 0;

static void test_getCrankAngle(void)
{
    auto decoder = buildDecoder(decoderToTest);
    setup_toothInterval();
    TEST_ASSERT_EQUAL_UINT16(95, decoder.pGetCrankAngle(toothLastToothTime+100));
}

static void test_timeToAngleIntervalTooth(void)
{
    setup_toothInterval();
    TEST_ASSERT_EQUAL_UINT16(18, timeToAngleIntervalTooth(100));

    setup_toothInterval();
    decoderStatus.toothAngleIsCorrect = true;
    TEST_ASSERT_UINT16_WITHIN(1, timeToAngle(9999), timeToAngleIntervalTooth(9999));
}

void testgetCrankAngle(void)
{
  SET_UNITY_FILENAME() {
    for (decoderToTest = 0; decoderToTest < DECODER_MAX; ++decoderToTest)
    {
        if (   (decoderToTest!=DECODER_HONDA_J32)
            && (decoderToTest!=DECODER_GM7X)
            && (decoderToTest!=DECODER_4G63)
            && (decoderToTest!=DECODER_24X)
            && (decoderToTest!=DECODER_JEEP2000)
            && (decoderToTest!=DECODER_MIATA_9905)
            && (decoderToTest!=DECODER_MAZDA_AU)
            && (decoderToTest!=DECODER_NISSAN_360)
            && (decoderToTest!=DECODER_SUBARU_67)
            && (decoderToTest!=DECODER_420A)
            && (decoderToTest!=DECODER_VMAX))
        {
            RUN_TEST_POSTFIX_P(test_getCrankAngle, getDecoderName(decoderToTest));
        }
    }
    RUN_TEST_P(test_timeToAngleIntervalTooth);
  }
}