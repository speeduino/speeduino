#include "../test_utils.h"
#include "logger.h"
#include "decoder_init.h"
#include "decoders.h"

extern decoder_status_t decoderStatus;

static uint8_t decoderToTest;
static void test_start_stop(void)
{
    configPage4.triggerTeeth = 6; // Prevent division by zero
    configPage2.nCylinders = 4; // Needed to prevent division by zero.
    currentStatus.initialisationComplete = true;
#if defined(STM32F407xx)
    pinNumbers.pinTrigger = PD3; //The CAS pin
    pinNumbers.pinTrigger2 = PD4; //The Cam Sensor pin
    pinNumbers.pinTrigger2 = PD5; //The Cam Sensor pin
#else
    pinNumbers.pinTrigger = 18;
    pinNumbers.pinTrigger2 = 19;
    pinNumbers.pinTrigger3 = 20;
#endif
    currentStatus.decoder = buildDecoder(decoderToTest);
    currentStatus.decoder.primary.edge = CHANGE;
    currentStatus.decoder.secondary.edge = CHANGE;
    
    startToothLogger();
    TEST_ASSERT_TRUE(currentStatus.toothLogEnabled);
    TEST_ASSERT_EQUAL(0, currentStatus.compositeTriggerUsed);
    TEST_ASSERT_FALSE(currentStatus.isToothLog1Full);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(1, toothHistoryIndex);

    stopToothLogger();
    TEST_ASSERT_FALSE(currentStatus.toothLogEnabled);

    startCompositeLogger();
    TEST_ASSERT_FALSE(currentStatus.toothLogEnabled);
    TEST_ASSERT_EQUAL(2, currentStatus.compositeTriggerUsed);
    TEST_ASSERT_FALSE(currentStatus.isToothLog1Full);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(1, toothHistoryIndex);
    
    stopCompositeLogger();
    TEST_ASSERT_EQUAL(0, currentStatus.compositeTriggerUsed);
    
    startCompositeLoggerTertiary();
    TEST_ASSERT_FALSE(currentStatus.toothLogEnabled);
    TEST_ASSERT_EQUAL(3, currentStatus.compositeTriggerUsed);
    TEST_ASSERT_FALSE(currentStatus.isToothLog1Full);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(1, toothHistoryIndex);
    
    stopCompositeLoggerTertiary();
    TEST_ASSERT_EQUAL(0, currentStatus.compositeTriggerUsed);

    startCompositeLoggerCams();
    TEST_ASSERT_FALSE(currentStatus.toothLogEnabled);
    TEST_ASSERT_EQUAL(4, currentStatus.compositeTriggerUsed);
    TEST_ASSERT_FALSE(currentStatus.isToothLog1Full);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(1, toothHistoryIndex);
    
    stopCompositeLoggerCams();
    TEST_ASSERT_EQUAL(0, currentStatus.compositeTriggerUsed);

    TEST_PASS(); // Coverege only
}

void testStartStop(void)
{
  SET_UNITY_FILENAME()
  {
    for (uint8_t decoder = 0; decoder < DECODER_MAX; ++decoder)
    {
        if (DECODER_AUDI135!=decoder
            && DECODER_HARLEY!=decoder
            && DECODER_NGC!=decoder // Only produces a valid trigger when primary is LOW
            && DECODER_RENIX!=decoder   // See issue #1347
            && DECODER_ROVERMEMS!=decoder) { // See issue #1348
            decoderToTest = decoder;
            char szName[128];

            snprintf(szName, sizeof(szName), "test_start_stop_%d", decoder);
            UnityDefaultTestRun(test_start_stop, szName, __LINE__);
        }
    }
  }
}