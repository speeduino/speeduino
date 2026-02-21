#include <unity.h>
#include "decoders.h"
#include "logger.h"
#include "globals.h"
#include "decoder_init.h"
#include "../test_utils.h"

extern decoder_status_t decoderStatus;

static void fireInterrupt(uint8_t pin, uint8_t edge)
{
  uint8_t currentState = digitalRead(pin);
  pinMode(pin, OUTPUT);
  if (edge == RISING)
  {
    if (currentState == HIGH)
    {
      digitalWrite(pin, LOW);
      TEST_ASSERT_EQUAL(LOW, digitalRead(pin));
    }
    digitalWrite(pin, HIGH);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(pin));
  }
  else if (edge == FALLING)
  {
    if (currentState == LOW)
    {
      digitalWrite(pin, HIGH);
      TEST_ASSERT_EQUAL(HIGH, digitalRead(pin));
    }
    digitalWrite(pin, LOW);
    TEST_ASSERT_EQUAL(LOW, digitalRead(pin));
  }
  else if (edge == CHANGE)
  {
    digitalWrite(pin, !currentState); // Toggle pin state
    TEST_ASSERT_EQUAL(!currentState, digitalRead(pin));
  }
  delay(0);
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

static void assertPrimaryTrigger(uint8_t decoder, uint8_t edge)
{
    char szMsg[64];
    snprintf(szMsg, sizeof(szMsg), "Decoder %d, edge %d", decoder, edge);

    getDecoder().reset();
    configureDecoderForStartStop(decoder);
    decoderStatus.validTrigger = false;
    delayMicroseconds(triggerFilterTime + 1);
    fireInterrupt(pinTrigger, edge);
#if defined(NATIVE_BOARD)
    TEST_MESSAGE("No interrupts on native board :-(");
#else
    TEST_ASSERT_TRUE_MESSAGE(decoderStatus.validTrigger, szMsg);
#endif
}

static void assertStartStopPrimaryTrigger(uint8_t decoder, uint8_t edge)
{
    // Test primary trigger function
    assertPrimaryTrigger(decoder, edge);

    // Attach logger
    startToothLogger();

    // Test primary trigger function
    assertPrimaryTrigger(decoder, edge);

    // Detach logger
    stopToothLogger();

    // Test primary trigger function
    assertPrimaryTrigger(decoder, edge);
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
    setDecoder(decoderToTest);

    assertStartStopPrimaryTrigger(decoderToTest, getDecoder().primary.edge);
}

static void test_start_stop_falling(void)
{
#if !defined(SIMULATOR) && defined(CORE_AVR)
  TEST_IGNORE_MESSAGE("Cannot run interrupt based tests on AVRboard");
#endif

    pinTrigger = 19; // Example pin number
    configPage4.TrigEdge = 1;
    setDecoder(decoderToTest);

    assertStartStopPrimaryTrigger(decoderToTest, getDecoder().primary.edge);
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

          snprintf(szName, sizeof(szName), "test_start_stop_rising_%d", decoder);
          UnityDefaultTestRun(test_start_stop_rising, szName, __LINE__);
          snprintf(szName, sizeof(szName), "test_start_stop_falling_%d", decoder);
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
  setDecoder(DECODER_NGC);

  // The NGC decoder triggers on change, but only sets 
  // BIT_DECODER_VALID_TRIGGER on falling interrupts.
  fireInterrupt(pinTrigger, RISING);
  assertPrimaryTrigger(DECODER_NGC, getDecoder().primary.edge);
  
  // Attach logger
  startToothLogger();

  // Test primary trigger function
  fireInterrupt(pinTrigger, RISING);
  assertPrimaryTrigger(DECODER_NGC, getDecoder().primary.edge);

  // Detach logger
  stopToothLogger();

  // Test primary trigger function
  fireInterrupt(pinTrigger, RISING);
  assertPrimaryTrigger(DECODER_NGC, getDecoder().primary.edge);
}

void testToothLoggers(void)
{
  SET_UNITY_FILENAME() {
    test_start_stop_all();
    RUN_TEST(test_start_stop_ngc);
  }
}