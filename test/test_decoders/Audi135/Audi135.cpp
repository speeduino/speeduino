#include "../test_utils.h"
#include "src/decoders/audi135.h"

static void test_getCrankAngle(void)
{
  // Configure and create decoder
  config4 page4={};
  auto decoderState = decoders::audi135::initialise();

  auto run_case = [&](int toothCount, bool revOne, int delta, int trigAngle, int16_t expected) {
    decoderState.toothLastToothTime = 2000;
    decoderState.toothCurrentCount = toothCount;
    decoderState.revolutionOne = revOne;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.decoderStatus.toothAngleIsCorrect = true;
    page4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoders::audi135::getCrankAngle(decoderState.toothLastToothTime + delta, decoderState, page4));
  };

  // timeToAngle(100) ~= 18 deg when revolution time = 2000
  const int dt = 18;

  // Basic checks
  run_case(1, false, 100, 0, 0 + dt);
  run_case(3, false, 100, 0, (3 - 1) * 8 + dt); // tooth 3 -> 16 + dt
  run_case(10, false, 100, 0, (10 - 1) * 8 + dt); // tooth 10 -> 72 + dt

  // Last-tooth and zero-case (zero treated as 45)
  run_case(45, false, 100, 0, (45 - 1) * 8 + dt);
  run_case(0, false, 100, 0, (45 - 1) * 8 + dt);

  // trigger angle offset
  run_case(3, false, 100, 5, (3 - 1) * 8 + 5 + dt);

  // sequential/revolution cases
  run_case(1, true, 100, 0, 360 + 0 + dt);
  // Case where adding 360 wraps above 720: tooth 45 + revOne -> (352+dt+360)-720
  {
    int raw = (45 - 1) * 8 + dt + 360;
    int wrapped = raw >= 720 ? raw - 720 : raw;
    run_case(45, true, 100, 0, wrapped);
  }
}

static void test_primary_tracks_pulses_before_sync(void)
{
  statuses current{};
  config4 page4{};
  auto decoderState = decoders::audi135::initialise();
  decoderState.toothCurrentCount = 13;
  decoderState.toothLastToothTime = 1000;

  decoders::audi135::triggerPrimary(5000, current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT16(13, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(5000, decoderState.toothLastToothTime);
}

static void test_primary_waits_for_every_third_tooth_when_synced(void)
{
  statuses current{};
  config4 page4{};
  auto decoderState = decoders::audi135::initialise();
  TEST_ASSERT_EQUAL_UINT16(8, decoderState.triggerToothAngle);
  current.startRevolutions = 1;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 1;
  decoderState.toothSystemCount = 0;
  decoderState.toothSystemLastToothTime = 1000;
  decoderState.toothLastToothTime = 900;
  decoderState.triggerFilterTime = 0;
  decoderState.toothAngles[0] = 174;
  decoderState.toothAngles[1] = 194;

  decoders::audi135::triggerPrimary(1100, current, decoderState, page4);
  TEST_ASSERT_EQUAL_UINT8(1, decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);

  decoders::audi135::triggerPrimary(1200, current, decoderState, page4);
  TEST_ASSERT_EQUAL_UINT8(2, decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);

  decoders::audi135::triggerPrimary(1300, current, decoderState, page4);
  TEST_ASSERT_EQUAL_UINT8(0, decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT16(2, decoderState.toothCurrentCount);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(1300, decoderState.toothSystemLastToothTime);
  TEST_ASSERT_EQUAL_UINT32(1300, decoderState.toothLastToothTime);
}

static void test_primary_rejects_gap_at_filter_threshold(void)
{
  statuses current{};
  config4 page4{};
  auto decoderState = decoders::audi135::initialise();
  current.startRevolutions = 1;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothSystemLastToothTime = 1000;
  decoderState.toothSystemCount = 2;
  decoderState.toothCurrentCount = 10;
  decoderState.triggerFilterTime = 100;

  decoders::audi135::triggerPrimary(1100, current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT32(100, decoderState.curGap);
  TEST_ASSERT_EQUAL_UINT8(2, decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT16(10, decoderState.toothCurrentCount);
  TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothSystemLastToothTime);
}

static void test_primary_wraps_after_tooth_45(void)
{
  statuses current{};
  config4 page4{};
  auto decoderState = decoders::audi135::initialise();
  TEST_ASSERT_EQUAL_UINT16(8, decoderState.triggerToothAngle);
  current.startRevolutions = 4;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 45;
  decoderState.toothSystemCount = 2;
  decoderState.toothSystemLastToothTime = 1000;
  decoderState.toothLastToothTime = 900;
  decoderState.toothOneMinusOneTime = 200;
  decoderState.toothOneTime = 600;
  decoderState.triggerFilterTime = 0;
  decoderState.revolutionOne = false;

  decoders::audi135::triggerPrimary(1100, current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT8(0, decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(5, current.startRevolutions);
  TEST_ASSERT_TRUE(decoderState.revolutionOne);
  TEST_ASSERT_EQUAL_UINT32(600, decoderState.toothOneMinusOneTime);
  TEST_ASSERT_EQUAL_UINT32(1100, decoderState.toothOneTime);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
}

static void test_secondary_acquires_sync(void)
{
  statuses current{};
  config4 page4{};
  auto decoderState = decoders::audi135::initialise();
  decoderState.toothCurrentCount = 17;
  decoderState.toothSystemCount = 1;
  decoderState.revolutionOne = false;

  decoders::audi135::triggerSecondary(current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT16(0, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT8(3, decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_secondary_resync_resets_tooth_counts(void)
{
  statuses current{};
  config4 page4{};
  page4.useResync = 1;
  auto decoderState = decoders::audi135::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 17;
  decoderState.toothSystemCount = 1;

  decoders::audi135::triggerSecondary(current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT16(0, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT8(3, decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_secondary_resets_tooth_count_early_in_run(void)
{
  statuses current{};
  config4 page4{};
  auto decoderState = decoders::audi135::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 17;
  decoderState.toothSystemCount = 1;
  current.startRevolutions = 99;

  decoders::audi135::triggerSecondary(current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT16(0, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT8(1, decoderState.toothSystemCount);
  TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_secondary_preserves_tooth_45_early_in_run(void)
{
  statuses current{};
  config4 page4{};
  auto decoderState = decoders::audi135::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 45;
  current.startRevolutions = 99;

  decoders::audi135::triggerSecondary(current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT16(45, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

static void test_secondary_preserves_tooth_count_after_100_revolutions(void)
{
  statuses current{};
  config4 page4{};
  auto decoderState = decoders::audi135::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothCurrentCount = 17;
  decoderState.toothSystemCount = 1;
  current.startRevolutions = 100;

  decoders::audi135::triggerSecondary(current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT16(17, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT8(1, decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_TRUE(decoderState.revolutionOne);
}

void testAudi135(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getCrankAngle);
  RUN_TEST_P(test_primary_tracks_pulses_before_sync);
  RUN_TEST_P(test_primary_waits_for_every_third_tooth_when_synced);
  RUN_TEST_P(test_primary_rejects_gap_at_filter_threshold);
  RUN_TEST_P(test_primary_wraps_after_tooth_45);
  RUN_TEST_P(test_secondary_acquires_sync);
  RUN_TEST_P(test_secondary_resync_resets_tooth_counts);
  RUN_TEST_P(test_secondary_resets_tooth_count_early_in_run);
  RUN_TEST_P(test_secondary_preserves_tooth_45_early_in_run);
  RUN_TEST_P(test_secondary_preserves_tooth_count_after_100_revolutions);
}