#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "scheduler_ignition_controller.h"

// used by the ROVER MEMS pattern
// #define ID_TOOTH_PATTERN 0 // have we identified teeth to skip for calculating RPM?
#define SKIP_TOOTH1 1
#define SKIP_TOOTH2 2
#define SKIP_TOOTH3 3
#define SKIP_TOOTH4 4

extern int16_t toothAngles[24];
extern uint16_t ignitionEndTeeth[IGN_CHANNELS];

static void assert_rover_setEndTeeth(int triggerAngle, uint8_t sparkMode, uint8_t trigSpeed, 
  const uint16_t (&dischargeAngles)[4], const uint16_t (&expected)[4], const int16_t (&skipTeeth)[4])
{
  auto decoder = triggerSetup_RoverMEMS();

  toothAngles[1] = skipTeeth[0];
  toothAngles[2] = skipTeeth[1];
  toothAngles[3] = skipTeeth[2];
  toothAngles[4] = skipTeeth[3];

  configPage4.triggerAngle = triggerAngle;
  configPage4.sparkMode = sparkMode;
  configPage4.TrigSpeed = trigSpeed;

  ignitionSchedule1.dischargeAngle = dischargeAngles[0];
  ignitionSchedule2.dischargeAngle = dischargeAngles[1];
  ignitionSchedule3.dischargeAngle = dischargeAngles[2];
  ignitionSchedule4.dischargeAngle = dischargeAngles[3];

  decoder.setEndTeeth();

  for (uint8_t i = 0; i < 4; i++)
  {
    TEST_ASSERT_EQUAL_UINT16(expected[i], ignitionEndTeeth[i]);
  }
}

static void test_getRPM(void)
{
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;
  extern volatile unsigned int toothCurrentCount;
  extern decoder_status_t decoderStatus;

  auto decoder = triggerSetup_RoverMEMS();

  currentStatus.crankRPM = 400;

  // Make skip-tooth definitions deterministic for this test
  toothAngles[SKIP_TOOTH1] = 100;
  toothAngles[SKIP_TOOTH2] = 101;
  toothAngles[SKIP_TOOTH3] = 102;
  toothAngles[SKIP_TOOTH4] = 103;

  // Ensure staging allows cranking calculation
  configPage4.StgCycles = 0;

  // --- Cranking path: tooth not a skip tooth -> crankingGetRPM(36)
  currentStatus.setRpm(currentStatus.crankRPM/2U);
  currentStatus.startRevolutions = 0; // cranking
  decoderStatus.syncStatus = SyncStatus::Full;
  toothCurrentCount = 1; // not a skip tooth
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 1667UL; // gap -> revTime ~=1667*36 ~=60012 -> ~1000 RPM
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- If at a skip tooth, return currentStatus.RPM
  toothCurrentCount = (unsigned int)toothAngles[SKIP_TOOTH1];
  TEST_ASSERT_EQUAL_UINT16(currentStatus.RPM, decoder.getRPM());
  toothCurrentCount = (unsigned int)toothAngles[SKIP_TOOTH2];
  TEST_ASSERT_EQUAL_UINT16(currentStatus.RPM, decoder.getRPM());
  toothCurrentCount = (unsigned int)toothAngles[SKIP_TOOTH3];
  TEST_ASSERT_EQUAL_UINT16(currentStatus.RPM, decoder.getRPM());
  toothCurrentCount = (unsigned int)toothAngles[SKIP_TOOTH4];
  TEST_ASSERT_EQUAL_UINT16(currentStatus.RPM, decoder.getRPM());

  // --- Running path: stdGetRPM(CRANK_SPEED)
  currentStatus.setRpm(currentStatus.crankRPM*2U);
  currentStatus.startRevolutions = 1; // not cranking
  decoderStatus.syncStatus = SyncStatus::Full;
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL; // revTime = 60000 -> 1000 RPM
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Fallback: when sync lost, stdGetRPM will return currentStatus.RPM (or crankingGetRPM returns currentStatus.RPM)
  decoderStatus.syncStatus = SyncStatus::None;
  currentStatus.setRpm(777);
  TEST_ASSERT_EQUAL_UINT16(777U, decoder.getRPM());
}

static void test_setEndTeeth(void)
{
  const int16_t zeroSkipTeeth[4] = { 0U, 0U, 0U, 0U };

  // Baseline: no missing teeth are flagged, so the function should just reflect the discharge-angle math.
  const uint16_t baseAngles[4] = { 0U, 20U, 90U, 170U };
  const uint16_t baseExpected[4] = { 35U, 1U, 8U, 16U };
  assert_rover_setEndTeeth(0, IGN_MODE_WASTED, CRANK_SPEED, baseAngles, baseExpected, zeroSkipTeeth);
  assert_rover_setEndTeeth(0, IGN_MODE_WASTED, CAM_SPEED, baseAngles, baseExpected, zeroSkipTeeth);

  // Trigger-angle offsets wrap the base calculation correctly.
  const uint16_t offsetAngles[4] = { 180U, 250U, 340U, 450U };
  const uint16_t offsetExpected[4] = { 8U, 15U, 24U, 35U };
  assert_rover_setEndTeeth(90, IGN_MODE_SINGLE, CRANK_SPEED, offsetAngles, offsetExpected, zeroSkipTeeth);
  assert_rover_setEndTeeth(90, IGN_MODE_SINGLE, CAM_SPEED, offsetAngles, offsetExpected, zeroSkipTeeth);

  // Missing-tooth compensation in wasted spark must back off one tooth when the calculated end tooth lands on a known gap.
  const int16_t wastedSkipTeeth[4] = { 8, 16, 0, 0 };
  const uint16_t skipAngles[4] = { 90U, 170U, 260U, 350U };
  const uint16_t skipExpected[4] = { 7U, 15U, 25U, 34U };
  assert_rover_setEndTeeth(0, IGN_MODE_WASTED, CRANK_SPEED, skipAngles, skipExpected, wastedSkipTeeth);
  assert_rover_setEndTeeth(0, IGN_MODE_WASTED, CAM_SPEED, skipAngles, skipExpected, wastedSkipTeeth);

  // Sequential mode applies the same missing-tooth correction while also adding the 36-tooth offset for crank-speed sequential timing.
  const int16_t seqSkipTeeth[4] = { 8, 16, 44, 0 };
  const uint16_t seqAngles[4] = { 90U, 170U, 450U, 520U };
  const uint16_t seqExpectedCrankSpeed[4] = { 7U, 15U, 43U, 51U };
  assert_rover_setEndTeeth(0, IGN_MODE_SEQUENTIAL, CRANK_SPEED, seqAngles, seqExpectedCrankSpeed, seqSkipTeeth);
  const uint16_t seqExpectedCamSpeed[4] = { 7U, 15U, 43U-36U, 51U-36U };
  assert_rover_setEndTeeth(0, IGN_MODE_SEQUENTIAL, CAM_SPEED, seqAngles, seqExpectedCamSpeed, seqSkipTeeth);

  // Negative trigger-angle offsets still resolve to the same effective tooth numbers after wrapping.
  const uint16_t negOffsetAngles[4] = { 0U, 30U, 120U, 210U };
  const uint16_t negOffsetExpected[4] = { 8U, 11U, 20U, 29U };
  assert_rover_setEndTeeth(-90, IGN_MODE_WASTED, CRANK_SPEED, negOffsetAngles, negOffsetExpected, zeroSkipTeeth);
  assert_rover_setEndTeeth(-90, IGN_MODE_WASTED, CAM_SPEED, negOffsetAngles, negOffsetExpected, zeroSkipTeeth);
}

void testRoverMems(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRPM);
    RUN_TEST_P(test_setEndTeeth);
  }
}