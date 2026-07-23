#include "decoders.h"
#include "logger.h"
#include "globals.h"
#include "decoder_init.h"
#include "../test_utils.h"
#include "decoder_name.h"
#include "shared.h"

extern decoder_status_t decoderStatus;
extern volatile unsigned long triggerFilterTime;

static const char* edgeName(uint8_t edge)
{
  if (edge==RISING)
  {
    static const char name[]="RISING";
    return name;
  }
  if (edge==FALLING)
  {
    static const char name[]="FALLING";
    return name;
  }
  if (edge==CHANGE)
  {
    static const char name[]="CHANGE";
    return name;
  }
  static const char name[]="Unknown";
  return name;
}

static void assertValidTrigger(const decoder_t &decoder, uint8_t decoderNum, uint8_t testEdge)
{
  // We only expect a valid trigger when the decoder trigger conditions are met.
  bool expected = currentStatus.decoder.primary.isTriggered();
  // The NGC decoder triggers on change, but only sets 
  // decoderStatus.validTrigger on falling interrupts.
  if (DECODER_NGC==decoderNum)
  {
    expected = testEdge==FALLING;
  }

  TEST_ASSERT_EQUAL(expected, decoder.getStatus().validTrigger);
}

static void test_toothLogger(decoder_t &decoder, uint8_t decoderNum, uint8_t testEdge)
{
  currentStatus.decoder = decoder;
    
  // Attach logger
  startToothLogger();

  configurePinState(currentStatus.decoder.primary._pin, testEdge);

  currentStatus.decoder.reset();
  configureStateForPrimaryTrigger(decoderNum, decoderStatus);

  loggerPrimaryISR();
  assertValidTrigger(currentStatus.decoder, decoderNum, testEdge);

  // Detach logger
  stopToothLogger();
}

static void test_toothLogger(uint8_t decoderNum, uint8_t configSetting, uint8_t testEdge)
{
#if !defined(SIMULATOR) && defined(CORE_AVR)
  TEST_IGNORE_MESSAGE("Cannot run interrupt based tests on AVRboard");
#endif

  pinNumbers.pinTrigger = 19; // Example pin number
  configPage4.TrigEdge = configSetting;
  currentStatus.initialisationComplete = false;
  auto decoder = buildDecoder(decoderNum);

  test_toothLogger(decoder, decoderNum, testEdge);
}

// Used as pseudo parameter to support dynamic test naming.
static uint8_t decoderToTest;
static uint8_t decoderConfigEdge;
static uint8_t triggerEdge;
static void test_toothLogger(void)
{
  test_toothLogger(decoderToTest, decoderConfigEdge, triggerEdge);
}

static void test_start_stop_toothLogger(void)
{
  for (decoderToTest = 0; decoderToTest < DECODER_MAX; ++decoderToTest)
  {
    auto decoderName = getDecoderName(decoderToTest);
    if (DECODER_RENIX!=decoderToTest   // See issue #1347
        && DECODER_ROVERMEMS!=decoderToTest) // See issue #1348
    {
      for(decoderConfigEdge=0; decoderConfigEdge<2; ++decoderConfigEdge)
      {
        for (triggerEdge=CHANGE; triggerEdge<=RISING; ++triggerEdge)
        {
          char szPostfix[128] = {};
          snprintf(szPostfix, _countof(szPostfix)-1, "%s_%" PRIu8 "_%s", decoderName, decoderConfigEdge, edgeName(triggerEdge));
          RUN_TEST_POSTFIX_P(test_toothLogger, szPostfix);
        }
      }
    }
  }
}

void testToothLoggers(void)
{
  SET_UNITY_FILENAME() {
    test_start_stop_toothLogger();
  }
}