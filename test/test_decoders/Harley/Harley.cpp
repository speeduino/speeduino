#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

extern volatile unsigned long toothOneTime;
extern volatile unsigned long toothOneMinusOneTime;
extern volatile uint32_t toothLastToothTime;
extern volatile unsigned long toothLastMinusOneToothTime;
extern decoder_status_t decoderStatus;
extern unsigned int triggerToothAngle;
extern volatile int toothCurrentCount;

static void test_getCrankAngle(void)
{
  auto decoder = triggerSetup_Harley();

  // Make time->angle deterministic for tests
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
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

static void test_getRPM(void)
{
  auto decoder = triggerSetup_Harley();

  // Ensure sync present
  decoderStatus.syncStatus = SyncStatus::Full;

  // --- Cranking-like branch: currentStatus.RPM < configPage4.crankRPM*100
  currentStatus.setRpm(0);
  configPage4.crankRPM = 100; // 100*100 = 10000 > 0 -> take special branch
  // Choose tempToothAngle = 120 and gap such that final RPM = 1000
  triggerToothAngle = 120;
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 20000UL; // gap = 20000 -> toothTime=20000*36 -> yields 1000 RPM
  // toothOne values used by SetRevolutionTime; set harmless values
  toothOneMinusOneTime = 0UL;
  toothOneTime = 0UL;
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Running path: should call stdGetRPM(CRANK_SPEED)
  currentStatus.setRpm(2000);
  configPage4.crankRPM = 10; // 10*100 = 1000 < 2000 -> use stdGetRPM
  // Set toothOne* so stdGetRPM will compute 1000 RPM (revTime = 60000)
  currentStatus.revolutionTime = 12345UL; // ensure SetRevolutionTime will change
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL; // revTime=60000 -> 1000 RPM
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Fallback: when not synced, return zerro
  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT16(0, decoder.getRPM());
}

void testHarley(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}