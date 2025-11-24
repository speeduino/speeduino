#include <unity.h>
#include "decoders.h"
#include "logger.h"
#include "globals.h"
#include "../../test_utils.h"

extern decoder_status_t decoderStatus;

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

static void assertPrimaryTrigger(void)
{
    decoderStatus.validTrigger = false;
    delayMicroseconds(triggerFilterTime + 1);
    fireInterrupt(pinTrigger, CHANGE);
#if defined(NATIVE_BOARD)
    TEST_MESSAGE("No interrupts on native board :-(");
#else
    TEST_ASSERT_TRUE(decoderStatus.validTrigger);
#endif
}

static void testVMax_primary_trigger_rising(void)
{
    // Setup VMax decoder
    pinTrigger = 19; // Example pin number
    configPage4.TrigEdge = 0;
    initialiseDecoder(DECODER_VMAX);

    // Test primary trigger function
    assertPrimaryTrigger();

    // Attach logger
    startToothLogger();

    // Test primary trigger function
    assertPrimaryTrigger();

    // Detach logger
    stopToothLogger();

    // Test primary trigger function
    assertPrimaryTrigger();
}

static void testVMax_primary_trigger_falling(void)
{
    // Setup VMax decoder
    pinTrigger = 19; // Example pin number
    configPage4.TrigEdge = 1;
    initialiseDecoder(DECODER_VMAX);

    // Test primary trigger function
    assertPrimaryTrigger();

    // Attach logger
    startToothLogger();

    // Test primary trigger function
    assertPrimaryTrigger();

    // Detach logger
    stopToothLogger();

    // Test primary trigger function
    assertPrimaryTrigger();
}

void testVMax()
{
  SET_UNITY_FILENAME() {
    RUN_TEST(testVMax_primary_trigger_rising);
    RUN_TEST(testVMax_primary_trigger_falling);
  }
}