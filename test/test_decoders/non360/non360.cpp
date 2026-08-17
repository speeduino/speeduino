#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;

  // Configure a simple non360 wheel: 8 teeth, no multiplier
  configPage4.triggerTeeth = 8;
  configPage4.TrigAngMul = 1;
  configPage4.triggerAngle = 0;

  auto decoder = triggerSetup_non360();

  // Deterministic angle conversion
  setAngleConverterRevolutionTime(2000);

  toothLastToothTime = 10000;

  // tooth 1 -> base 0
  toothCurrentCount = 1;
  TEST_ASSERT_EQUAL(18, decoder.pGetCrankAngle(toothLastToothTime + 100));

  // tooth 0 -> treated as last tooth (triggerTeeth)
  toothCurrentCount = 0;
  TEST_ASSERT_EQUAL(351, decoder.pGetCrankAngle(toothLastToothTime + 200));

  // Test with TrigAngMul >1 and different tooth count
  configPage4.triggerTeeth = 10;
  configPage4.TrigAngMul = 2;
  configPage4.triggerAngle = 0;
  decoder = triggerSetup_non360(); // recompute triggerToothAngle
  toothLastToothTime = 20000;
  toothCurrentCount = 3;
  TEST_ASSERT_EQUAL(81, decoder.pGetCrankAngle(toothLastToothTime + 50));

  // Wrap-around: force triggerAngle large so result >=720
  configPage4.triggerTeeth = 8;
  configPage4.TrigAngMul = 1;
  configPage4.triggerAngle = 500;
  decoder = triggerSetup_non360();
  toothLastToothTime = 30000;
  toothCurrentCount = 8; // base = 7*45 = 315 -> +500 = 815 >=720 -> subtract 720 -> 95
  TEST_ASSERT_EQUAL(97, decoder.pGetCrankAngle(toothLastToothTime + 10));
}

void testNon360(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}