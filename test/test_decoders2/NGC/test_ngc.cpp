#include "../../test_utils.h"
#include "src/decoders/ngc.h"
#include "scheduler_ignition_controller.h"
#include "../../test_decoders/test_decoder_shared.h"

constexpr uint32_t BASE_1ST_TOOTH_TIME = 100000UL;
constexpr uint32_t BASE_2ND_TOOTH_TIME = 200000UL;

struct context_ngc_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(uint8_t cylinders)
  {
    page2.nCylinders = cylinders;
    decoderState = decoders::ngc::initialise(page2, page4);
    current.decoder.primary._pin.setPin(2);
    current.decoder.secondary._pin.setPin(3);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::ngc::triggerPrimary(curTime, current, decoderState, page2, page4);
  }

  void triggerSecondary4(uint32_t curTime)
  {
    decoders::ngc::triggerSecondary4(curTime, current, decoderState, page2, page4);
  }
  
  void triggerSecondary68(uint32_t curTime)
  {
    decoders::ngc::triggerSecondary68(curTime, decoderState, page2, page4);
  }

  uint32_t getRevolutionTime()
  {
    return decoders::ngc::getRevolutionTime(current, decoderState, page4);
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

static void test_getRevolutionTime(void)
{
  context_ngc_t context;
  context.init(4);

  // Ensure staging allows cranking calculation
  context.page4.StgCycles = 0;
  context.current.crankRPM = 400;

  // --- Cranking path: tooth-angle correct -> use crankingGetRevolutionTime(36, CRANK_SPEED)
  context.current.setRpm(context.current.crankRPM/2U);
  context.current.startRevolutions = 0; // cranking
  context.decoderState.decoderStatus.toothAngleIsCorrect = true;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.current.revolutionTime = 99999UL;
  context.decoderState.toothLastMinusOneToothTime = 1000UL;
  context.decoderState.toothLastToothTime = context.decoderState.toothLastMinusOneToothTime + 1667UL; // gap ~=1667 -> revTime ~=60012
  TEST_ASSERT_EQUAL_UINT32(1667UL*36UL, context.getRevolutionTime());

  // --- If tooth angle not correct, return context.current.revolutionTime
  context.decoderState.decoderStatus.toothAngleIsCorrect = false;
  TEST_ASSERT_EQUAL_UINT32(99999UL, context.getRevolutionTime());

  // --- Running path: use stdGetRevolutionTime(CRANK_SPEED)
  context.current.setRpm(context.current.crankRPM*2U);
  context.current.startRevolutions = 1; // not cranking
  context.decoderState.decoderStatus.toothAngleIsCorrect = true;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.current.revolutionTime = 12345UL;
  context.decoderState.toothOneMinusOneTime = 1000UL;
  context.decoderState.toothOneTime = context.decoderState.toothOneMinusOneTime + 60000UL; // revTime = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, context.getRevolutionTime());

  // --- Fallback: when sync lost, stdGetRevolutionTime returns context.current.revolutionTime
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(12345UL, context.getRevolutionTime());
}

static void test_triggerPrimary_rising_edge_only_records_time(void)
{
  context_ngc_t context;
  context.init(4);
  context.decoderState.toothCurrentCount = 6;
  context.current.decoder.primary._pin._pin.setPinHigh();
  context.setup_primaryfilter(1000, 100);

  auto nextTime = calcNextTimePassPrimaryFilter(context.decoderState);
  context.triggerPrimary(nextTime);

  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothLastToothRisingTime);
  TEST_ASSERT_EQUAL_UINT16(6, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_rejects_filtered_pulse(void)
{
  context_ngc_t context;
  context.init(4);
  context.decoderState.toothCurrentCount = 4;
  context.current.decoder.primary._pin._pin.setPinLow();
  context.setup_primaryfilter(1000, 100);

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)-2);

  TEST_ASSERT_EQUAL_UINT16(4, context.decoderState.toothCurrentCount);
  TEST_ASSERT_FALSE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_accepts_regular_tooth(void)
{
  context_ngc_t context;
  context.init(4);
  context.decoderState.toothCurrentCount = 1;
  context.current.decoder.primary._pin._pin.setPinLow();
  context.setup_primaryfilter(1000, 100);

  auto nextTime = calcNextTimePassPrimaryFilter(context.decoderState);
  context.triggerPrimary(nextTime);

  TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.toothCurrentCount);
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.toothAngleIsCorrect);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
}

static void test_triggerPrimary_missing_tooth_high_polarity(void)
{
  context_ngc_t context;
  context.init(4);
  context.decoderState.toothCurrentCount = 16;
  context.current.decoder.primary._pin._pin.setPinLow();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothLastToothRisingTime = context.decoderState.toothLastToothTime+1000;

  auto nextTime = context.decoderState.toothLastToothRisingTime + 1500;
  context.triggerPrimary(nextTime);

  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_FALSE(context.decoderState.decoderStatus.toothAngleIsCorrect);
  TEST_ASSERT_EQUAL_UINT32(nextTime - (context.decoderState.toothLastToothTime - context.decoderState.toothLastMinusOneToothTime), context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_missing_tooth_low_polarity(void)
{
  context_ngc_t context;
  context.init(4);
  context.decoderState.toothCurrentCount = 16;
  context.current.decoder.primary._pin._pin.setPinLow();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothLastToothRisingTime = context.decoderState.toothLastToothTime+1800;

  context.triggerPrimary(context.decoderState.toothLastToothRisingTime+700);

  TEST_ASSERT_EQUAL_UINT16(19, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_FALSE(context.decoderState.decoderStatus.toothAngleIsCorrect);
}

static void test_triggerSecondary4_ignores_non_sequential_mode(void)
{
  context_ngc_t context;
  context.init(4);
  context.setup_ign_inj(IGN_MODE_WASTED, INJ_PAIRED);
  context.setup_secondaryfilter(1000, 100);
  context.triggerSecondary4(calcNextTimePassSecondaryFilter(context.decoderState));

  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary4_rising_edge_only_records_time(void)
{
  context_ngc_t context;
  context.init(4);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.current.decoder.secondary._pin._pin.setPinHigh();
  context.decoderState.secondaryToothCount = 3;
  context.setup_secondaryfilter(1000, 100);
  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary4(nextTime);

  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothLastSecToothRisingTime);
  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT16(3, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary4_rejects_filtered_pulse(void)
{
  context_ngc_t context;
  context.init(4);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.current.decoder.secondary._pin._pin.setPinLow();
  context.setup_secondaryfilter(1000, 1000);
  context.decoderState.secondaryToothCount = 2;

  context.triggerSecondary4(calcNextTimePassSecondaryFilter(context.decoderState)-2);

  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary4_regular_tooth(void)
{
  context_ngc_t context;
  context.init(4);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.current.decoder.secondary._pin._pin.setPinLow();
  context.decoderState.secondaryToothCount = 1;
  context.setup_secondaryfilter(1000, 100);

  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary4(nextTime);

  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, context.decoderState.toothLastMinusOneSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(25, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary4_unknown_tooth(void)
{
  context_ngc_t context;
  context.init(4);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.current.decoder.secondary._pin._pin.setPinLow();
  context.decoderState.secondaryToothCount = 0;
  context.setup_secondaryfilter(1000, 100);

  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary4(nextTime);

  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, context.decoderState.toothLastMinusOneSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(100, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.secondaryToothCount);
}

static void setup_triggerSecondary4_missing_tooth(context_ngc_t &context, bool highOrLow, uint16_t toothCount)
{
  context.init(4);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.current.decoder.secondary._pin._pin.setPinLow();
  context.decoderState.toothLastSecToothRisingTime = BASE_2ND_TOOTH_TIME+(highOrLow ? 500 : 1200);
  context.decoderState.secondaryToothCount = toothCount;
  context.setup_secondaryfilter(1000, 100);
}

static void test_triggerSecondary4_missing_tooth(bool highOrLow, uint16_t toothCount, uint16_t expectedCount)
{
  context_ngc_t context;
  setup_triggerSecondary4_missing_tooth(context, highOrLow, toothCount);

  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState)+1500;
  context.triggerSecondary4(nextTime);

  TEST_ASSERT_EQUAL_UINT16(expectedCount, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothLastSecToothTime);
}

static void test_triggerSecondary4_high_missing_tooth(void)
{
  test_triggerSecondary4_missing_tooth(true, 7, 1);
  test_triggerSecondary4_missing_tooth(true, 0, 1);
  test_triggerSecondary4_missing_tooth(true, 1, 0);
}

static void test_triggerSecondary4_low_missing_tooth(void)
{
  test_triggerSecondary4_missing_tooth(false, 4, 5);
  test_triggerSecondary4_missing_tooth(false, 0, 5);
  test_triggerSecondary4_missing_tooth(false, 1, 0);
}

static void setup_triggerSecondary68_long_gap(context_ngc_t &context,
                                               uint8_t cylinders,
                                               uint8_t currentCount,
                                               uint8_t previousCount)
{
  context.init(cylinders);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.setup_secondaryfilter(1000, 100);
  context.setup_primaryfilter(1000, 100);
  context.decoderState.secondaryToothCount = currentCount;
  context.decoderState.checkSyncToothCount = previousCount;
}

static void test_triggerSecondary68_ignores_non_sequential_mode(void)
{
  context_ngc_t context;
  context.init(6);
  context.setup_ign_inj(IGN_MODE_WASTED, INJ_PAIRED);
  context.setup_secondaryfilter(1000, 100);

  context.triggerSecondary68(calcNextTimePassSecondaryFilter(context.decoderState));

  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, context.decoderState.toothLastSecToothTime);
}

static void test_triggerSecondary68_rejects_filtered_pulse(void)
{
  context_ngc_t context;
  setup_triggerSecondary68_long_gap(context, 6, 2, 1);
  context.setup_secondaryfilter(1000, 2000);

  context.triggerSecondary68(calcNextTimePassSecondaryFilter(context.decoderState)-500);

  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT8(2, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.checkSyncToothCount);
}

static void test_triggerSecondary68_counts_regular_cam_tooth(void)
{
  context_ngc_t context;
  context.init(6);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.setup_secondaryfilter(1000, 100);
  context.setup_primaryfilter(1000, 100);
  context.decoderState.secondaryToothCount = 2;

  auto nextTime = calcNextTimePassSecondaryFilter(context.decoderState);
  context.triggerSecondary68(nextTime);

  TEST_ASSERT_EQUAL_UINT8(3, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(25, context.decoderState.triggerSecFilterTime);
}

static void test_triggerSecondary68_acquires_six_cylinder_group(void)
{
  context_ngc_t context;
  setup_triggerSecondary68_long_gap(context, 6, 2, 1);

  // The pulse advances the current group count to 3, matching group 1's pattern.
  context.triggerSecondary68(103000);

  TEST_ASSERT_EQUAL_UINT8(3, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT8(2, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL_UINT32(103000, context.decoderState.toothLastSecToothTime);
}

static void test_triggerSecondary68_acquires_eight_cylinder_group(void)
{
  context_ngc_t context;
  setup_triggerSecondary68_long_gap(context, 8, 2, 2);

  // The pulse advances the current group count to 3, matching group 4's pattern.
  context.triggerSecondary68(103000);

  TEST_ASSERT_EQUAL_UINT8(6, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT8(2, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary68_advances_known_group(void)
{
  context_ngc_t context;
  setup_triggerSecondary68_long_gap(context, 6, 1, 3);
  context.decoderState.toothSystemCount = 2;

  // The pulse advances the count to 2, matching the next group's pattern.
  context.triggerSecondary68(103000);

  TEST_ASSERT_EQUAL_UINT8(2, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.secondaryToothCount);
}

static void test_triggerSecondary68_advances_existing_group(void)
{
  context_ngc_t context;
  context.init(6);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.setup_secondaryfilter(1000, 100);
  context.setup_primaryfilter(1000, 100);

  context.decoderState.toothSystemCount = 1;
  context.decoderState.secondaryToothCount = 1; // Matches toothAngles[2].
  context.decoderState.checkSyncToothCount = 2;

  context.triggerSecondary68(BASE_2ND_TOOTH_TIME + 3000); // 3x primary gap.

  TEST_ASSERT_EQUAL_UINT8(2, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.checkSyncToothCount);
  TEST_ASSERT_EQUAL_UINT8(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.triggerSecFilterTime);
}

static void test_setEndTeeth_skips_missing_teeth_and_offsets_sequential(void)
{
  context_ngc_t context;
  context.init(4);

  const auto originalAngle = ignitionSchedule1.dischargeAngle;
  ignitionSchedule1.dischargeAngle = 180;

  context.setup_ign_inj(IGN_MODE_WASTED, INJ_PAIRED);
  decoders::ngc::setEndTeeth(context.decoderState, context.page4);
  const uint16_t crankTooth = context.decoderState.ignitionEndTeeth[0];

  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.page4.TrigSpeed = CRANK_SPEED;
  ignitionSchedule1.dischargeAngle = 180 + 360;
  decoders::ngc::setEndTeeth(context.decoderState, context.page4);
  const uint16_t sequentialTooth = context.decoderState.ignitionEndTeeth[0];

  TEST_ASSERT_EQUAL_UINT16(crankTooth + 36, sequentialTooth);

  ignitionSchedule1.dischargeAngle = originalAngle;
}

static void setup_sequential_primary_missing_tooth(context_ngc_t &context,
                                                    uint8_t cylinders,
                                                    uint16_t camCount,
                                                    uint8_t systemCount,
                                                    bool highMissingTooth)
{
  context.init(cylinders);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);

  context.current.decoder.primary._pin._pin.setPinLow();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothLastToothRisingTime = context.decoderState.toothLastToothTime + (highMissingTooth ? 1000 : 1800);
  context.decoderState.toothCurrentCount = 16;
  context.decoderState.secondaryToothCount = camCount;
  context.decoderState.toothSystemCount = systemCount;
}

static void test_triggerPrimary_sequential(context_ngc_t &context,
                                                  uint8_t cylinders,
                                                    uint16_t camCount,
                                                    uint8_t systemCount,
                                                    bool highMissingTooth)
{
  ;
  setup_sequential_primary_missing_tooth(context, cylinders, camCount, systemCount, highMissingTooth);

  context.triggerPrimary(context.decoderState.toothLastToothRisingTime+(highMissingTooth ? 1500 : 700));

  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT16(highMissingTooth ? 1 : 19, context.decoderState.toothCurrentCount);
}

static void test_triggerPrimary_sequential_revZero(uint8_t cylinders,
                                                    uint16_t camCount,
                                                    uint8_t systemCount,
                                                    bool highMissingTooth)
{
  context_ngc_t context;
  test_triggerPrimary_sequential(context, cylinders, camCount, systemCount, highMissingTooth);

  TEST_ASSERT_FALSE(context.decoderState.revolutionOne);
}

static void test_triggerPrimary_sequential_four_cylinder_revZero(void)
{
  test_triggerPrimary_sequential_revZero(4, 1, 0, true);
  test_triggerPrimary_sequential_revZero(4, 2, 0, true);
  test_triggerPrimary_sequential_revZero(4, 4, 0, false);
}

static void test_triggerPrimary_sequential_revOne(uint8_t cylinders,
                                                    uint16_t camCount,
                                                    uint8_t systemCount,
                                                    bool highMissingTooth)
{
  context_ngc_t context;
  test_triggerPrimary_sequential(context, cylinders, camCount, systemCount, highMissingTooth);

  TEST_ASSERT_TRUE(context.decoderState.revolutionOne);
}

static void test_triggerPrimary_sequential_four_cylinder_revOne(void)
{
  test_triggerPrimary_sequential_revOne(4, 5, 0, true);
  test_triggerPrimary_sequential_revOne(4, 7, 0, false);
}

static void test_triggerPrimary_sequential_six_cylinder_revZero(void)
{
  test_triggerPrimary_sequential_revZero(6, 0, 1, true);
  test_triggerPrimary_sequential_revZero(6, 0, 2, true);
  test_triggerPrimary_sequential_revZero(6, 0, 2, false);
  test_triggerPrimary_sequential_revZero(6, 0, 3, false);
}

static void test_triggerPrimary_sequential_six_cylinder_revOne(void)
{
  test_triggerPrimary_sequential_revOne(6, 0, 4, true);
  test_triggerPrimary_sequential_revOne(6, 0, 5, true);
  test_triggerPrimary_sequential_revOne(6, 0, 5, false);
  test_triggerPrimary_sequential_revOne(6, 0, 6, false);
}

static void test_triggerPrimary_sequential_eight_cylinder_revZero(void)
{
  test_triggerPrimary_sequential_revZero(8, 0, 1, true);
  test_triggerPrimary_sequential_revZero(8, 0, 2, true);
  test_triggerPrimary_sequential_revZero(8, 0, 3, false);
  test_triggerPrimary_sequential_revZero(8, 0, 4, false);
}

static void test_triggerPrimary_sequential_eight_cylinder_revOne(void)
{
  test_triggerPrimary_sequential_revOne(8, 0, 5, true);
  test_triggerPrimary_sequential_revOne(8, 0, 6, true);
  test_triggerPrimary_sequential_revOne(8, 0, 7, false);
  test_triggerPrimary_sequential_revOne(8, 0, 8, false);
}

static void test_triggerPrimary_sequential_invalid_pattern_loses_sync(uint8_t cylinders,
                                                    uint16_t camCount,
                                                    uint8_t systemCount,
                                                    bool highMissingTooth)
{
  {
    context_ngc_t context;
    setup_sequential_primary_missing_tooth(context, cylinders, camCount, systemCount, highMissingTooth);
    context.decoderState.decoderStatus.syncStatus = SyncStatus::None;
    context.triggerPrimary(context.decoderState.toothLastToothRisingTime+(highMissingTooth ? 1500 : 700));
    TEST_ASSERT_EQUAL(SyncStatus::Partial, context.decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(0, context.current.syncLossCounter);
  }
  {
    context_ngc_t context;
    setup_sequential_primary_missing_tooth(context, cylinders, camCount, systemCount, highMissingTooth);
    context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    context.triggerPrimary(context.decoderState.toothLastToothRisingTime+(highMissingTooth ? 1500 : 700));
    TEST_ASSERT_EQUAL(SyncStatus::Partial, context.decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(1, context.current.syncLossCounter);
  }
}

static void test_triggerPrimary_sequential_invalid_pattern_loses_sync(void)
{
  test_triggerPrimary_sequential_invalid_pattern_loses_sync(4, 3, 0, true);
  test_triggerPrimary_sequential_invalid_pattern_loses_sync(4, 3, 0, false);
  test_triggerPrimary_sequential_invalid_pattern_loses_sync(6, 0, 3, true);
  test_triggerPrimary_sequential_invalid_pattern_loses_sync(6, 0, 7, false);
  test_triggerPrimary_sequential_invalid_pattern_loses_sync(8, 0, 3, true);
  test_triggerPrimary_sequential_invalid_pattern_loses_sync(8, 0, 9, false);
}

static void setup_per_tooth_primary(context_ngc_t &context)
{
  context.init(4);
  context.page2.perToothIgn = true;
  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.decoder.primary._pin._pin.setPinLow();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothCurrentCount = 1;
}

static void test_triggerPrimary_per_tooth_running(void)
{
  context_ngc_t context;
  setup_per_tooth_primary(context);
  context.page4.triggerAngle = 5;

  context.triggerPrimary(101000);

  // The per-tooth call receives crank angle 10 + triggerAngle (15 degrees).
  TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.toothCurrentCount);
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(101000, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_per_tooth_sequential_crank_speed(void)
{
  context_ngc_t context;
  setup_per_tooth_primary(context);
  context.setup_ign_inj(IGN_MODE_SEQUENTIAL, INJ_PAIRED);
  context.page4.TrigSpeed = CRANK_SPEED;
  context.decoderState.revolutionOne = true;
  context.page4.triggerAngle = 5;

  context.triggerPrimary(101000);

  // This path adds 360 degrees before calling checkPerToothTiming.
  TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.toothCurrentCount);
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(101000, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_per_tooth_skipped_while_cranking(void)
{
  context_ngc_t context;
  setup_per_tooth_primary(context);
  context.current.rotationStatus = EngineRotationStatus::Cranking;

  context.triggerPrimary(101000);

  TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.toothCurrentCount);
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(101000, context.decoderState.toothLastToothTime);
}

void testNGC()
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getRevolutionTime);
    RUN_TEST(test_triggerPrimary_rising_edge_only_records_time);
    RUN_TEST(test_triggerPrimary_rejects_filtered_pulse);
    RUN_TEST(test_triggerPrimary_accepts_regular_tooth);
    RUN_TEST(test_triggerPrimary_missing_tooth_high_polarity);
    RUN_TEST(test_triggerPrimary_missing_tooth_low_polarity);
    RUN_TEST(test_triggerPrimary_sequential_four_cylinder_revZero);
    RUN_TEST(test_triggerPrimary_sequential_four_cylinder_revOne);
    RUN_TEST(test_triggerPrimary_sequential_six_cylinder_revZero);
    RUN_TEST(test_triggerPrimary_sequential_six_cylinder_revOne);
    RUN_TEST(test_triggerPrimary_sequential_eight_cylinder_revZero);
    RUN_TEST(test_triggerPrimary_sequential_eight_cylinder_revOne);
    RUN_TEST(test_triggerPrimary_sequential_invalid_pattern_loses_sync);
    RUN_TEST(test_triggerPrimary_per_tooth_running);
    RUN_TEST(test_triggerPrimary_per_tooth_sequential_crank_speed);
    RUN_TEST(test_triggerPrimary_per_tooth_skipped_while_cranking);
    RUN_TEST(test_triggerSecondary4_ignores_non_sequential_mode);
    RUN_TEST(test_triggerSecondary4_rising_edge_only_records_time);
    RUN_TEST(test_triggerSecondary4_rejects_filtered_pulse);
    RUN_TEST(test_triggerSecondary4_regular_tooth);
    RUN_TEST(test_triggerSecondary4_unknown_tooth);
    RUN_TEST(test_triggerSecondary4_high_missing_tooth);
    RUN_TEST(test_triggerSecondary4_low_missing_tooth);
    RUN_TEST(test_triggerSecondary68_ignores_non_sequential_mode);
    RUN_TEST(test_triggerSecondary68_rejects_filtered_pulse);
    RUN_TEST(test_triggerSecondary68_counts_regular_cam_tooth);
    RUN_TEST(test_triggerSecondary68_acquires_six_cylinder_group);
    RUN_TEST(test_triggerSecondary68_acquires_eight_cylinder_group);
    RUN_TEST(test_triggerSecondary68_advances_known_group);
    RUN_TEST(test_triggerSecondary68_advances_existing_group);
    RUN_TEST(test_setEndTeeth_skips_missing_teeth_and_offsets_sequential);
  }
}