#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;
      
  auto decoder = triggerSetup_HondaD17();

  // Use deterministic time->angle conversion
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  setAngleConverterRevolutionTime(2000);

  // Setup common deterministic state
  configPage4.triggerAngle = 0;
  toothLastToothTime = 5000;

  // toothCurrentCount == 1 -> angle = 0 + triggerAngle + timeToAngle(dt)
  toothCurrentCount = 1;
  TEST_ASSERT_EQUAL(18, decoder.pGetCrankAngle(toothLastToothTime + 100));

  // toothCurrentCount == 2 -> angle = 1*triggerToothAngle + triggerAngle + timeToAngle(dt)
  toothCurrentCount = 2;
  TEST_ASSERT_EQUAL(48, decoder.pGetCrankAngle(toothLastToothTime + 100));

  // toothCurrentCount == 0 -> treated as 13th tooth -> use 11*triggerToothAngle
  toothCurrentCount = 0;
  TEST_ASSERT_EQUAL(339, decoder.pGetCrankAngle(toothLastToothTime + 50));

  // Trigger angle offset and wrap-around behavior
  configPage4.triggerAngle = 500;
  toothCurrentCount = 12; // base = 11*triggerToothAngle = 330
  TEST_ASSERT_EQUAL(112, decoder.pGetCrankAngle(toothLastToothTime + 10));
}

static void test_getRPM(void)
{
  auto decoder = triggerSetup_HondaD17();
  TEST_ASSERT_NOT_EQUAL(0, decoder.getRPM());
}

void testHondaD17(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}