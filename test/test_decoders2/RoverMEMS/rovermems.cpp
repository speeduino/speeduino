#include "../../test_utils.h"
#include "src/decoders/rovermems.h"
#include "scheduler_ignition_controller.h"
#include "../../test_decoders/test_decoder_shared.h"
#include "atomic.h"

constexpr uint32_t BASE_1ST_TOOTH_TIME = 100000UL;
constexpr uint32_t BASE_2ND_TOOTH_TIME = 200000UL;

static int16_t rovermems_test_crank_angle(uint32_t)
{
  return 200;
}

struct context_rovermems_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  config6 page6 = {};
  config10 page10 = {};
  decoders::shared::state_t decoderState;

  void init(void)
  {
    decoderState = decoders::rovermems::initialise(page4);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::rovermems::triggerPrimary(curTime, current, decoderState, page2, page4);
  }

  void triggerSecondary(uint32_t curTime)
  {
    decoders::rovermems::triggerSecondary(curTime, current, decoderState, page4, page6, page10);
  }
  
  void setEndTeeth(void)
  {
    decoders::rovermems::setEndTeeth(decoderState, page4);
  }

  uint32_t getRevolutionTime()
  {
    return decoders::rovermems::getRevolutionTime(current, decoderState, page4);
  }

  void setup_primaryfilter(uint32_t lastGap, uint32_t filter)
  {
    decoderState.toothLastToothTime = BASE_1ST_TOOTH_TIME;
    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime - lastGap;
    decoderState.triggerFilterTime = filter;
  }

  void setup_secondaryfilter(uint32_t lastGap, uint32_t filter)
  {
    decoderState.toothLastSecToothTime = BASE_2ND_TOOTH_TIME;
    decoderState.toothLastMinusOneSecToothTime = decoderState.toothLastSecToothTime - lastGap;
    decoderState.triggerSecFilterTime = filter;
  }

  void setup_ign_inj(uint8_t ignMode, uint8_t injLayout)
  {
    page4.sparkMode = ignMode;
    page2.injLayout = injLayout;
  }
};

#define SKIP_TOOTH1 1
#define SKIP_TOOTH2 2
#define SKIP_TOOTH3 3
#define SKIP_TOOTH4 4

static void test_triggerPrimary_rejects_filtered_pulse(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_primaryfilter(1000, 1000);

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 500);

  TEST_ASSERT_EQUAL_UINT32(500, context.decoderState.curGap);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME - 1000, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.roverMEMSTeethSeen);
}

static void test_triggerPrimary_tracks_regular_tooth(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothCurrentCount = 1;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(1, context.decoderState.roverMEMSTeethSeen);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME + 1000, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_tracks_missing_gap(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothCurrentCount = 1;
  context.decoderState.roverMEMSTeethSeen = 1;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 2000);

  TEST_ASSERT_EQUAL_UINT16(3, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(5, context.decoderState.roverMEMSTeethSeen);
  TEST_ASSERT_EQUAL_UINT32(100, context.decoderState.triggerFilterTime);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME + 2000, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_accepts_first_tooth_without_history(void)
{
  context_rovermems_t context;
  context.init();
  context.decoderState.toothLastToothTime = BASE_1ST_TOOTH_TIME;
  context.decoderState.triggerFilterTime = 100;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME + 1000, context.decoderState.toothLastToothTime);
}

static void assert_primary_pattern(uint32_t pattern, uint8_t expectedPattern, uint8_t skipTooth1,
                                   uint8_t skipTooth2, uint8_t skipTooth3, uint8_t skipTooth4,
                                   uint8_t missingTeeth)
{
  context_rovermems_t context;
  context.init();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothCurrentCount = 35;
  context.decoderState.roverMEMSTeethSeen = pattern >> 1;
  context.decoderState.secondaryToothCount = 1;
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page4.trigPatternSec = SEC_TRIGGER_SINGLE;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL_UINT8(expectedPattern, context.decoderState.toothAngles[0]);
  TEST_ASSERT_EQUAL_UINT8(skipTooth1, context.decoderState.toothAngles[SKIP_TOOTH1]);
  TEST_ASSERT_EQUAL_UINT8(skipTooth2, context.decoderState.toothAngles[SKIP_TOOTH2]);
  TEST_ASSERT_EQUAL_UINT8(skipTooth3, context.decoderState.toothAngles[SKIP_TOOTH3]);
  TEST_ASSERT_EQUAL_UINT8(skipTooth4, context.decoderState.toothAngles[SKIP_TOOTH4]);
  TEST_ASSERT_EQUAL_UINT8(missingTeeth, context.page4.triggerMissingTeeth);
  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT16(1, context.current.startRevolutions);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.secondaryToothCount);
}

static void test_triggerPrimary_identifies_supported_wheel_patterns(void)
{
  assert_primary_pattern(0b11111101111111011111111110111111UL, 5, 1, 11, 19, 30, 4);
  assert_primary_pattern(0b11011101111111111111101101111111UL, 4, 8, 11, 25, 27, 4);
  assert_primary_pattern(0b11011011111111111111011101111111UL, 3, 8, 10, 24, 27, 4);
  assert_primary_pattern(0b11111101111101111111111110111101UL, 2, 1, 12, 17, 29, 4);
  assert_primary_pattern(0b11111111111101111111111111111101UL, 1, 1, 18, 0, 0, 2);
}

static void prepare_primary_common_pattern(context_rovermems_t &context)
{
  context.init();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothCurrentCount = 35;
  context.decoderState.roverMEMSTeethSeen = 0b11111111111101111111111111111101UL >> 1;
  context.page4.trigPatternSec = SEC_TRIGGER_SINGLE;
  context.page4.TrigSpeed = CRANK_SPEED;
}

static void test_triggerPrimary_partial_sync_branches(void)
{
  context_rovermems_t context;
  prepare_primary_common_pattern(context);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL(SyncStatus::Partial, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT16(1, context.current.startRevolutions);

  context = context_rovermems_t();
  prepare_primary_common_pattern(context);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);

  context = context_rovermems_t();
  prepare_primary_common_pattern(context);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.page2.injLayout = INJ_SEQUENTIAL;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL(SyncStatus::Partial, context.decoderState.decoderStatus.syncStatus);

  context = context_rovermems_t();
  prepare_primary_common_pattern(context);
  context.page4.sparkMode = IGN_MODE_WASTED;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL(SyncStatus::Partial, context.decoderState.decoderStatus.syncStatus);

  context = context_rovermems_t();
  prepare_primary_common_pattern(context);
  context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
  context.page4.TrigSpeed = CAM_SPEED;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
}

static void test_triggerPrimary_loses_sync_on_unknown_wheel_pattern(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothCurrentCount = 37;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT16(1, context.current.syncLossCounter);
  TEST_ASSERT_EQUAL_UINT16(38, context.decoderState.toothCurrentCount);
}

static void test_triggerSecondary_rejects_filtered_pulse(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_secondaryfilter(1000, 1000);
  context.page4.trigPatternSec = SEC_TRIGGER_SINGLE;

  context.triggerSecondary(BASE_2ND_TOOTH_TIME + 500);

  TEST_ASSERT_EQUAL_UINT32(500, context.decoderState.curGap2);
  TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_FALSE(context.decoderState.revolutionOne);
}

static void test_triggerSecondary_single_tooth_cam(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_secondaryfilter(1000, 100);
  context.page4.trigPatternSec = SEC_TRIGGER_SINGLE;

  context.triggerSecondary(BASE_2ND_TOOTH_TIME + 2000);

  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME + 2000, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_TRUE(context.decoderState.revolutionOne);
  TEST_ASSERT_EQUAL_UINT32(1000, context.decoderState.triggerSecFilterTime);
}

static void test_triggerSecondary_multi_tooth_pattern(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_secondaryfilter(1000, 100);
  context.page4.trigPatternSec = SEC_TRIGGER_5_3_2;
  context.decoderState.targetGap2 = 1500;
  context.decoderState.secondaryToothCount = 2;

  context.triggerSecondary(BASE_2ND_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL_UINT16(3, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT32(500, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL_UINT32(1500, context.decoderState.targetGap2);
}

static void test_triggerSecondary_multi_tooth_gap_sets_revolution(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_secondaryfilter(1000, 100);
  context.page4.trigPatternSec = SEC_TRIGGER_5_3_2;
  context.decoderState.targetGap2 = 1500;
  context.decoderState.secondaryToothCount = 5;
  context.decoderState.toothCurrentCount = 10;

  context.triggerSecondary(BASE_2ND_TOOTH_TIME + 2000);

  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT16(28, context.decoderState.toothCurrentCount);
  TEST_ASSERT_FALSE(context.decoderState.revolutionOne);
}

static void test_triggerSecondary_initializes_first_tooth(void)
{
  context_rovermems_t context;
  context.init();
  context.decoderState.curGap = 450;

  context.triggerSecondary(BASE_2ND_TOOTH_TIME + 300000);

  TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.curGap2);
  TEST_ASSERT_EQUAL_UINT32(900, context.decoderState.targetGap2);
  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME + 300000, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary_records_vvt_angle(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_secondaryfilter(1000, 100);
  context.current.decoder.pGetCrankAngle = &rovermems_test_crank_angle;
  context.page4.trigPatternSec = SEC_TRIGGER_SINGLE;
  context.page4.triggerAngle = 30;
  context.page6.vvtEnabled = 1;
  context.page6.vvtMode = VVT_MODE_CLOSED_LOOP;
  context.page10.vvtCLMinAng = 20;

  context.triggerSecondary(BASE_2ND_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL_INT16(150, context.current.vvt1.angle);

  context = context_rovermems_t();
  context.init();
  context.setup_secondaryfilter(1000, 100);
  context.current.decoder.pGetCrankAngle = &rovermems_test_crank_angle;
  context.page4.trigPatternSec = SEC_TRIGGER_SINGLE;
  context.page4.triggerAngle = 30;
  context.page6.vvtEnabled = 1;
  context.page6.vvtMode = VVT_MODE_OPEN_LOOP;

  context.triggerSecondary(BASE_2ND_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL_INT16(170, context.current.vvt1.angle);
}

static void test_triggerSecondary_records_vvt_for_multi_tooth_cam(void)
{
  context_rovermems_t context;
  context.init();
  context.setup_secondaryfilter(1000, 100);
  context.current.decoder.pGetCrankAngle = &rovermems_test_crank_angle;
  context.decoderState.secondaryToothCount = 5;
  context.decoderState.targetGap2 = 1500;
  context.page4.trigPatternSec = SEC_TRIGGER_5_3_2;
  context.page4.triggerAngle = 30;
  context.page6.vvtEnabled = 1;
  context.page6.vvtMode = VVT_MODE_CLOSED_LOOP;
  context.page10.vvtCLMinAng = 20;

  context.triggerSecondary(BASE_2ND_TOOTH_TIME + 1000);

  TEST_ASSERT_EQUAL_UINT16(6, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_INT16(150, context.current.vvt1.angle);
}

static void assert_multi_tooth_gap_alignment(uint16_t previousToothCount, uint16_t toothCount,
                                             uint16_t expectedToothCount, bool expectedRevolutionOne)
{
  context_rovermems_t context;
  context.init();
  context.setup_secondaryfilter(1000, 100);
  context.decoderState.targetGap2 = 1500;
  context.decoderState.secondaryToothCount = previousToothCount;
  context.decoderState.toothCurrentCount = toothCount;
  context.page4.trigPatternSec = SEC_TRIGGER_5_3_2;

  context.triggerSecondary(BASE_2ND_TOOTH_TIME + 2000);

  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT16(expectedToothCount, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(expectedRevolutionOne, context.decoderState.revolutionOne);
}

static void test_triggerSecondary_multi_tooth_gap_alignment_boundaries(void)
{
  assert_multi_tooth_gap_alignment(5, 19, 19, false);
  assert_multi_tooth_gap_alignment(3, 18, 0, true);
  assert_multi_tooth_gap_alignment(3, 17, 17, true);
  assert_multi_tooth_gap_alignment(2, 18, 36, true);
  assert_multi_tooth_gap_alignment(2, 19, 19, true);
  assert_multi_tooth_gap_alignment(0, 10, 10, false);
}

static void assert_rover_setEndTeeth(int triggerAngle, uint8_t sparkMode, uint8_t trigSpeed, 
  const uint16_t (&dischargeAngles)[4], const uint16_t (&expected)[4], const int16_t (&skipTeeth)[4])
{
  context_rovermems_t context;
  context.init();

  context.decoderState.toothAngles[1] = skipTeeth[0];
  context.decoderState.toothAngles[2] = skipTeeth[1];
  context.decoderState.toothAngles[3] = skipTeeth[2];
  context.decoderState.toothAngles[4] = skipTeeth[3];

  context.page4.triggerAngle = triggerAngle;
  context.page4.sparkMode = sparkMode;
  context.page4.TrigSpeed = trigSpeed;

  ignitionSchedule1.dischargeAngle = dischargeAngles[0];
  ignitionSchedule2.dischargeAngle = dischargeAngles[1];
  ignitionSchedule3.dischargeAngle = dischargeAngles[2];
  ignitionSchedule4.dischargeAngle = dischargeAngles[3];

  context.setEndTeeth();

  for (uint8_t i = 0; i < 4; i++)
  {
    TEST_ASSERT_EQUAL_UINT16(expected[i], context.decoderState.ignitionEndTeeth[i]);
  }
}

static void test_getRevolutionTime(void)
{
  context_rovermems_t context;
  context.init();

  context.current.crankRPM = 400;

  // Make skip-tooth definitions deterministic for this test
  context.decoderState.toothAngles[SKIP_TOOTH1] = 100;
  context.decoderState.toothAngles[SKIP_TOOTH2] = 101;
  context.decoderState.toothAngles[SKIP_TOOTH3] = 102;
  context.decoderState.toothAngles[SKIP_TOOTH4] = 103;

  // Ensure staging allows cranking calculation
  context.page4.StgCycles = 0;

  // --- Cranking path: tooth not a skip tooth -> crankingGetRevolutionTime(36)
  context.current.setRpm(context.current.crankRPM/2U);
  context.current.startRevolutions = 0; // cranking
  context.current.revolutionTime = 99999UL;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothCurrentCount = 1; // not a skip tooth
  context.decoderState.toothLastMinusOneToothTime = 1000UL;
  context.decoderState.toothLastToothTime = context.decoderState.toothLastMinusOneToothTime + 1667UL; // gap -> revTime ~=1667*36 ~=60012
  TEST_ASSERT_EQUAL_UINT32(1667UL*36UL, context.getRevolutionTime());

  // --- If at a skip tooth, return context.current.revolutionTime
  context.decoderState.toothCurrentCount = (uint16_t)context.decoderState.toothAngles[SKIP_TOOTH1];
  TEST_ASSERT_EQUAL_UINT32(99999UL, context.getRevolutionTime());
  context.decoderState.toothCurrentCount = (uint16_t)context.decoderState.toothAngles[SKIP_TOOTH2];
  TEST_ASSERT_EQUAL_UINT32(99999UL, context.getRevolutionTime());
  context.decoderState.toothCurrentCount = (uint16_t)context.decoderState.toothAngles[SKIP_TOOTH3];
  TEST_ASSERT_EQUAL_UINT32(99999UL, context.getRevolutionTime());
  context.decoderState.toothCurrentCount = (uint16_t)context.decoderState.toothAngles[SKIP_TOOTH4];
  TEST_ASSERT_EQUAL_UINT32(99999UL, context.getRevolutionTime());

  // --- Running path: stdGetRevolutionTime(CRANK_SPEED)
  context.current.setRpm(context.current.crankRPM*2U);
  context.current.startRevolutions = 1; // not cranking
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothOneMinusOneTime = 1000UL;
  context.decoderState.toothOneTime = context.decoderState.toothOneMinusOneTime + 60000UL; // revTime = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, context.getRevolutionTime());

  // --- Fallback: when sync lost, stdGetRevolutionTime returns context.current.revolutionTime
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(99999UL, context.getRevolutionTime());
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


static void setup_perTooth(context_rovermems_t &context)
{
  context.init();
  setAngleConverterRevolutionTime(6000000UL);
  context.current.setRpm(3000);

  context.decoderState.toothCurrentCount = 7;
  context.decoderState.triggerToothAngle = 13;
  context.decoderState.toothSystemCount = 11;
  context.decoderState.ignitionEndTeeth[0] = context.decoderState.toothCurrentCount+1;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.setup_primaryfilter(1000, 500);
  context.page2.perToothIgn = true;

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.startRevolutions = 99;

  ignitionSchedule1._status = PENDING;
  ignitionSchedule1._counter = 100;
  ignitionSchedule1._compare = 100;
  ignitionSchedule1.chargeAngle = 577;
}

static void assert_pertoothign(context_rovermems_t &context, uint16_t newCrankAngle)
{
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);
}

static void assert_pertoothign360(context_rovermems_t &context)
{
  auto newCrankAngle = context.page4.triggerAngle + ( (context.decoderState.toothCurrentCount) * context.decoderState.triggerToothAngle );
  assert_pertoothign(context, newCrankAngle);
}

static void assert_pertoothign720(context_rovermems_t &context)
{
  auto newCrankAngle = 360 + context.page4.triggerAngle + ( (context.decoderState.toothCurrentCount) * context.decoderState.triggerToothAngle );
  assert_pertoothign(context, newCrankAngle);
}

static void test_triggerPrimary_applies_per_tooth_timing(void)
{
  context_rovermems_t context;

  ATOMIC() {
    stopIgnitionSchedulers();

    setup_perTooth(context);
    assert_pertoothign360(context);

    context = context_rovermems_t();
    setup_perTooth(context);
    context.page4.TrigSpeed = CRANK_SPEED;
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    context.decoderState.revolutionOne = true;
    context.decoderState.ignitionEndTeeth[0] = context.page4.triggerTeeth + context.decoderState.toothCurrentCount+1;
    assert_pertoothign720(context);
  }
}


void testRoverMems(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_triggerPrimary_rejects_filtered_pulse);
    RUN_TEST_P(test_triggerPrimary_tracks_regular_tooth);
    RUN_TEST_P(test_triggerPrimary_tracks_missing_gap);
    RUN_TEST_P(test_triggerPrimary_accepts_first_tooth_without_history);
    RUN_TEST_P(test_triggerPrimary_identifies_supported_wheel_patterns);
    RUN_TEST_P(test_triggerPrimary_partial_sync_branches);
    RUN_TEST_P(test_triggerPrimary_loses_sync_on_unknown_wheel_pattern);
    RUN_TEST_P(test_triggerPrimary_applies_per_tooth_timing);
    RUN_TEST_P(test_triggerSecondary_rejects_filtered_pulse);
    RUN_TEST_P(test_triggerSecondary_single_tooth_cam);
    RUN_TEST_P(test_triggerSecondary_multi_tooth_pattern);
    RUN_TEST_P(test_triggerSecondary_multi_tooth_gap_sets_revolution);
    RUN_TEST_P(test_triggerSecondary_initializes_first_tooth);
    RUN_TEST_P(test_triggerSecondary_records_vvt_angle);
    RUN_TEST_P(test_triggerSecondary_records_vvt_for_multi_tooth_cam);
    RUN_TEST_P(test_triggerSecondary_multi_tooth_gap_alignment_boundaries);
    RUN_TEST_P(test_getRevolutionTime);
    RUN_TEST_P(test_setEndTeeth);
  }
}