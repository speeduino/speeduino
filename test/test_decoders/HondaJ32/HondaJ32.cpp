#include "../test_utils.h"
#include "src/decoders/hondaj32.h"

static void test_getCrankAngle(void)
{
  config4 page4 = {};
  auto decoderState = decoders::hondaj32::initialise();

  auto run_case = [&](uint16_t toothNum, int16_t expected, int16_t triggerAngle = 0) {
    decoderState.toothLastToothTime = 2000;
    decoderState.toothCurrentCount = toothNum;
    decoderState.decoderStatus.toothAngleIsCorrect = true;
    page4.triggerAngle = triggerAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 360;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoders::hondaj32::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));
  };

  // Basic teeth
  // triggerToothAngle = 15 deg. timeToAngle(100) ~= 18 deg
  run_case(1, 15 + 18);
  run_case(2, 30 + 18);

  // Special teeth
  // tooth 14 -> 213 + 18
  run_case(14, 213 + 18);
  // tooth 22 -> 333 + 18
  run_case(22, 333 + 18);

  // Wrap-around: tooth 24 -> 24*15 + 18 = 378 -> subtract 360 => 18
  run_case(24, 18);

  // Tooth zero case
  run_case(0, 0 + 18);

  // Non-zero trigger angle offset
  run_case(1, 15 + 18 + 10, 10);
}

static void test_getRevolutionTime(void)
{
  statuses current = {};
  auto decoderState = decoders::hondaj32::initialise();

  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  current.revolutionTime = 12345UL;
  // No tooth #1 history yet: keep the published period
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoders::hondaj32::getRevolutionTime(current, decoderState));

  // The time between the last 2 tooth #1 (as recorded by the trigger handler)
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 60000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::hondaj32::getRevolutionTime(current, decoderState));

  // No sync: the tooth #1 times may be stale, so keep the published period
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoders::hondaj32::getRevolutionTime(current, decoderState));
}

static void test_primary_counts_regular_tooth_while_unsynced(void)
{
  statuses current{};
  auto decoderState = decoders::hondaj32::initialise();
  decoderState.toothLastToothTime = 1000;

  decoders::hondaj32::triggerPrimary(1100, current, decoderState);

  TEST_ASSERT_EQUAL_UINT32(100, decoderState.curGap);
  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(100, decoderState.lastGap);
  TEST_ASSERT_EQUAL_UINT32(1100, decoderState.toothLastToothTime);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
}

static void test_primary_counts_regular_tooth_while_synced(uint8_t initialTooth)
{
  statuses current{};
  auto decoderState = decoders::hondaj32::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothLastToothTime = 1000;
  decoderState.toothCurrentCount = initialTooth;

  decoders::hondaj32::triggerPrimary(1100, current, decoderState);

  TEST_ASSERT_EQUAL_UINT32(100, decoderState.curGap);
  TEST_ASSERT_EQUAL_UINT16(initialTooth+1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(100, decoderState.lastGap);
  TEST_ASSERT_EQUAL_UINT32(1100, decoderState.toothLastToothTime);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
}

static void test_primary_counts_regular_tooth_while_synced(void)
{
  for (auto initialTooth: {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, /*13, 14, */ 15, 16, 17, 18, 19, 20, /*21, 22,*/ 23, })
  {
    test_primary_counts_regular_tooth_while_synced(initialTooth);
  }
}

static void test_primary_resets_count_on_ambiguous_gap(void)
{
  statuses current{};
  auto decoderState = decoders::hondaj32::initialise();
  decoderState.toothCurrentCount = 8;
  decoderState.lastGap = 100;
  decoderState.toothLastToothTime = 1000;

  decoders::hondaj32::triggerPrimary(1400, current, decoderState);

  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT32(100, decoderState.lastGap);
  TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastToothTime);
}

static void test_primary_syncs_on_second_missing_gap(void)
{
  statuses current{};
  auto decoderState = decoders::hondaj32::initialise();
  decoderState.toothCurrentCount = 15;
  decoderState.lastGap = 100;
  decoderState.toothLastToothTime = 4600;

  decoders::hondaj32::triggerPrimary(5000, current, decoderState);

  TEST_ASSERT_EQUAL_UINT16(16, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT32(3500, decoderState.toothOneTime);
  TEST_ASSERT_EQUAL_UINT32(1100, decoderState.toothOneMinusOneTime);
  TEST_ASSERT_EQUAL_UINT32(5000, decoderState.toothLastToothTime);
}

static void test_primary_keeps_last_gap_for_special_teeth(uint8_t initialToothNum)
{
  statuses current{};
  auto decoderState = decoders::hondaj32::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = initialToothNum;
  decoderState.lastGap = 200;
  decoderState.toothLastToothTime = 1000;

  decoders::hondaj32::triggerPrimary(1500, current, decoderState);

  TEST_ASSERT_EQUAL_UINT16(initialToothNum+1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(200, decoderState.lastGap);
}

static void test_primary_keeps_last_gap_for_special_teeth(void)
{
  test_primary_keeps_last_gap_for_special_teeth(13);
  test_primary_keeps_last_gap_for_special_teeth(21);
}

static void test_primary_keeps_sync_on_expected_missing_gap(uint8_t initialToothNum)
{
  statuses current{};
  auto decoderState = decoders::hondaj32::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = initialToothNum;
  decoderState.lastGap = 200;
  decoderState.toothLastToothTime = 1000;

  decoders::hondaj32::triggerPrimary(1400, current, decoderState);

  TEST_ASSERT_EQUAL_UINT16(initialToothNum+2, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT32(200, decoderState.lastGap);
}

static void test_primary_keeps_sync_on_expected_missing_gap(void)
{
  test_primary_keeps_sync_on_expected_missing_gap(14);
  test_primary_keeps_sync_on_expected_missing_gap(22);
}

static void test_primary_loses_sync_on_missing_gap_too_short(void)
{
  statuses current{};
  auto decoderState = decoders::hondaj32::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 14;
  decoderState.lastGap = 200;
  decoderState.toothLastToothTime = 1000;

  decoders::hondaj32::triggerPrimary(1100, current, decoderState);

  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT32(100, decoderState.curGap);
}

static void test_primary_rolls_over_after_tooth_24(void)
{
  statuses current{};
  auto decoderState = decoders::hondaj32::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 24;
  decoderState.toothOneMinusOneTime = 200;
  decoderState.toothOneTime = 600;
  decoderState.toothLastToothTime = 1000;
  current.startRevolutions = 3;

  decoders::hondaj32::triggerPrimary(1400, current, decoderState);

  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(4, current.startRevolutions);
  TEST_ASSERT_EQUAL_UINT32(600, decoderState.toothOneMinusOneTime);
  TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothOneTime);
  TEST_ASSERT_EQUAL_UINT32(1400, decoderState.toothLastToothTime);
}

void testHondaJ32(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getCrankAngle);
  RUN_TEST_P(test_getRevolutionTime);
  RUN_TEST_P(test_primary_counts_regular_tooth_while_unsynced);
  RUN_TEST_P(test_primary_counts_regular_tooth_while_synced);
  RUN_TEST_P(test_primary_resets_count_on_ambiguous_gap);
  RUN_TEST_P(test_primary_syncs_on_second_missing_gap);
  RUN_TEST_P(test_primary_keeps_last_gap_for_special_teeth);
  RUN_TEST_P(test_primary_keeps_sync_on_expected_missing_gap);
  RUN_TEST_P(test_primary_loses_sync_on_missing_gap_too_short);
  RUN_TEST_P(test_primary_rolls_over_after_tooth_24);
}