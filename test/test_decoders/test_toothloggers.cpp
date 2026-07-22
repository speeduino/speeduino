#include <unity.h>
#include "decoders.h"
#include "logger.h"
#include "globals.h"
#include "decoder_init.h"
#include "../test_utils.h"
#include "src/pins/boardInputPin.h"
#include "decoder_name.h"

extern decoder_status_t decoderStatus;
extern boardInputPin_t triggerPri_pin;
extern boardInputPin_t triggerSec_pin;
extern boardInputPin_t triggerThird_pin;

static void configurePinState(boardInputPin_t &p, uint8_t edge)
{
  if (edge == RISING)
  {
    if (p.isPinHigh())
    {
      p._pin.setPinLow();
    }
    p._pin.setPinHigh();
  }
  else if (edge == FALLING)
  {
    if (p.isPinLow())
    {
      p._pin.setPinHigh();
    }
    p._pin.setPinLow();
  }
  else if (edge == CHANGE)
  {
    if (p.isPinLow())
    {
      p._pin.setPinHigh();
    }
    else
    {
      p._pin.setPinLow();
    }
  }
}

extern volatile unsigned long triggerFilterTime;
static void configureDecoderForStartStop(uint8_t decoder)
{
    extern volatile byte toothSystemCount;
    extern volatile unsigned long toothLastToothRisingTime;
    extern volatile unsigned long toothLastSecToothRisingTime;
    extern volatile unsigned long toothLastToothTime;
    extern volatile unsigned long toothSystemLastToothTime;
    extern volatile uint16_t toothCurrentCount;

    if (decoder==DECODER_24X) {
        toothCurrentCount = 0U;
    } else if (decoder==DECODER_JEEP2000) {
        toothCurrentCount = 0U;
    } else if (decoder==DECODER_AUDI135) {
        toothSystemCount = 2U;
        toothSystemLastToothTime = micros() - triggerFilterTime;
        decoderStatus.syncStatus = SyncStatus::Full;
    } else if (decoder==DECODER_RENIX) {
        toothLastToothRisingTime = micros() - triggerFilterTime;
        toothLastSecToothRisingTime = toothLastToothRisingTime - triggerFilterTime;
    } else if (decoder==DECODER_ROVERMEMS) {
        toothLastToothTime = micros() - triggerFilterTime;
    }
}

static void assertTrigger(decoder_t decoder, uint8_t decoderNum, uint8_t edge, interrupt_t::callback_t isr)
{
    char szMsg[64];
    snprintf(szMsg, sizeof(szMsg), "Decoder %d, edge %d", decoderNum, edge);

    currentStatus.decoder = decoder;
    decoder.reset();
    configureDecoderForStartStop(decoderNum);

    configurePinState(triggerPri_pin, edge);
    isr();
    TEST_ASSERT_TRUE_MESSAGE(decoder.getStatus().validTrigger, szMsg);
}

static void assertStartStopPrimaryTrigger(decoder_t decoder, uint8_t decoderNum, uint8_t edge)
{
    currentStatus.decoder = decoder;

    // Test primary trigger function
    assertTrigger(decoder, decoderNum, edge, decoder.primary.callback);

    // Attach logger
    startToothLogger();

    // Test primary trigger function
    assertTrigger(decoder, decoderNum, edge, loggerPrimaryISR);

    // Detach logger
    stopToothLogger();

    // Test primary trigger function
    assertTrigger(decoder, decoderNum, edge, decoder.primary.callback);
}

// Used as pseudo parameter to support dynamic test naming.
static uint8_t decoderToTest;

static void test_start_stop_rising(void)
{
#if !defined(SIMULATOR) && defined(CORE_AVR)
  TEST_IGNORE_MESSAGE("Cannot run interrupt based tests on AVRboard");
#endif

    pinTrigger = 19; // Example pin number
    configPage4.TrigEdge = 0;
    currentStatus.initialisationComplete = false;
    auto decoder = buildDecoder(decoderToTest);

    assertStartStopPrimaryTrigger(decoder, decoderToTest, decoder.primary.edge);
}

static void test_start_stop_falling(void)
{
#if !defined(SIMULATOR) && defined(CORE_AVR)
  TEST_IGNORE_MESSAGE("Cannot run interrupt based tests on AVRboard");
#endif

    pinTrigger = 19; // Example pin number
    configPage4.TrigEdge = 1;
    auto decoder = buildDecoder(decoderToTest);

    assertStartStopPrimaryTrigger(decoder, decoderToTest, decoder.primary.edge);
}

static void test_start_stop_all(void)
{
    for (uint8_t decoder = 0; decoder < DECODER_MAX; ++decoder)
    {
        if (   DECODER_NGC!=decoder     // See test_start_stop_ngc
            && DECODER_RENIX!=decoder   // See issue #1347
            && DECODER_ROVERMEMS!=decoder) { // See issue #1348

          decoderToTest = decoder;
          char szName[128];

          snprintf(szName, sizeof(szName)-1, "test_start_stop_rising_%s", getDecoderName(decoder));
          UnityDefaultTestRun(test_start_stop_rising, szName, __LINE__);
          snprintf(szName, sizeof(szName)-1, "test_start_stop_falling_%s", getDecoderName(decoder));
          UnityDefaultTestRun(test_start_stop_falling, szName, __LINE__);
        }
    }
}

static void test_start_stop_ngc(void)
{
#if !defined(SIMULATOR) && defined(CORE_AVR)
  TEST_IGNORE_MESSAGE("Cannot run interrupt based tests on AVRboard");
#endif
  pinTrigger = 19; // Example pin number
  auto decoder = buildDecoder(DECODER_NGC);

  // The NGC decoder triggers on change, but only sets 
  // BIT_DECODER_VALID_TRIGGER on falling interrupts.
  configurePinState(triggerPri_pin, RISING);
  assertTrigger(decoder, DECODER_NGC, decoder.primary.edge, decoder.primary.callback);
  
  // Attach logger
  startToothLogger();

  // Test primary trigger function
  configurePinState(triggerPri_pin, RISING);
  assertTrigger(decoder, DECODER_NGC, decoder.primary.edge, loggerPrimaryISR);

  // Detach logger
  stopToothLogger();

  // Test primary trigger function
  configurePinState(triggerPri_pin, RISING);
  assertTrigger(decoder, DECODER_NGC, decoder.primary.edge, decoder.primary.callback);
}

void testToothLoggers(void)
{
  SET_UNITY_FILENAME() {
    test_start_stop_all();
    RUN_TEST(test_start_stop_ngc);
  }
}