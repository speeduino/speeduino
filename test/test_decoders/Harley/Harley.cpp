#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;
      
  auto decoder = triggerSetup_Harley();

  // Make time->angle deterministic for tests
  setAngleConverterRevolutionTime(2000);

  // Base case: tooth 1 should map to 0 + triggerAngle
  configPage4.triggerAngle = 0;
  toothLastToothTime = 10000;
  toothCurrentCount = 1;
  {
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 100);
    int16_t expected = 0 + timeToAngle(100);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Tooth 2 should map to 157 + triggerAngle
  toothCurrentCount = 2;
  {
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 100);
    int16_t expected = 157 + timeToAngle(100);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Tooth 3 behaves like tooth 1 (reference tooth)
  toothCurrentCount = 3;
  {
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 100);
    int16_t expected = 0 + timeToAngle(100);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Tooth 4 with a triggerAngle offset
  configPage4.triggerAngle = 10;
  toothCurrentCount = 4;
  {
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 50);
    int16_t expected = 157 + 10 + timeToAngle(50);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Wrap-around behaviour: large triggerAngle should reduce result by 720 when >=720
  configPage4.triggerAngle = 600;
  toothCurrentCount = 2;
  {
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 10);
    int16_t expected = 157 + 600 + timeToAngle(10);
    if (expected >= 720) expected -= 720;
    TEST_ASSERT_EQUAL(expected, angle);
  }
}

void testHarley(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}