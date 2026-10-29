#include "../test_utils.h"
#include "src/decoders/nissan360.h"
#include "../test_decoder_shared.h"
#include "atomic.h"
#include "scheduler_ignition_controller.h"

struct trigger_context_nis360_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState = decoders::nissan360::initialise();

  void triggerPrimary(uint32_t curTime)
  {
    decoders::nissan360::triggerPrimary(curTime, current, decoderState, page2, page4);
  }

  void triggerSecondary(uint32_t curTime)
  {
    decoders::nissan360::triggerSecondary(curTime, current, decoderState, page2, page4);
  }
};

static void setup_primary_gap(trigger_context_nis360_t &context, uint16_t toothCount, uint32_t lastTime)
{
  context.decoderState.toothCurrentCount = toothCount;
  context.decoderState.toothLastToothTime = lastTime;
  context.decoderState.toothOneMinusOneTime = 0;
  context.decoderState.triggerFilterTime = 1000;
}

static void test_triggerPrimary_gapcheck(void)
{
  trigger_context_nis360_t context;

  constexpr uint16_t TOOTH_COUNT = 5; // Arbitrary
  constexpr uint32_t LAST_TOOTHTIME = 777777;

  setup_primary_gap(context, TOOTH_COUNT, LAST_TOOTHTIME);
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)-2);
  TEST_ASSERT_EQUAL(TOOTH_COUNT, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(LAST_TOOTHTIME, context.decoderState.toothLastToothTime);

  setup_primary_gap(context, TOOTH_COUNT, LAST_TOOTHTIME);
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_NOT_EQUAL(TOOTH_COUNT, context.decoderState.toothCurrentCount);
  TEST_ASSERT_NOT_EQUAL(LAST_TOOTHTIME, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_wrapCount(void)
{
  trigger_context_nis360_t context;

  setup_primary_gap(context, 360, 777777UL);
  context.current.startRevolutions = 1;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(2, context.current.startRevolutions);
}

static void test_triggerPrimary_updatesTimers(void)
{
  trigger_context_nis360_t context;

  context.decoderState.toothLastToothTime = 100;

  context.triggerPrimary(1500);
  TEST_ASSERT_EQUAL(100, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL(1500, context.decoderState.toothLastToothTime);
}

static void setup_perTooth(trigger_context_nis360_t &context)
{
  context.decoderState.toothCurrentCount = 49;
  context.decoderState.ignitionEndTeeth[0] = context.decoderState.toothCurrentCount+1;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.page2.perToothIgn = true;

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.startRevolutions = 99;

  ignitionSchedule1._status = PENDING;
  ignitionSchedule1._counter = 100;
  ignitionSchedule1._compare = 100;
  ignitionSchedule1.chargeAngle = 200;
}

static void test_primary_applies_per_tooth_timing(void)
{
  trigger_context_nis360_t context;

  setAngleConverterRevolutionTime(6000000UL);
  context.current.setRpm(3000);
  context.decoderState.toothLastMinusOneToothTime = 500;
  context.decoderState.toothLastToothTime = 1000;
  ATOMIC() {
    stopIgnitionSchedulers();

    setup_perTooth(context);
    CRANK_ANGLE_MAX_IGN = 360;
    auto newCrankAngle = context.decoderState.toothCurrentCount * 2U;
    context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
    TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);

    setup_perTooth(context);
    context.decoderState.toothCurrentCount = 359;
    context.decoderState.ignitionEndTeeth[0] = (context.decoderState.toothCurrentCount+1)/2;
    newCrankAngle = context.decoderState.toothCurrentCount * 2U;
    ignitionSchedule1.chargeAngle = 400;
    TEST_ASSERT_LESS_THAN(newCrankAngle, CRANK_ANGLE_MAX_IGN);
    newCrankAngle -= CRANK_ANGLE_MAX_IGN;
    context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
    TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);
  }
}

static void test_primary_skips_per_tooth_timing_when_disabled(void)
{
  trigger_context_nis360_t context;

  setAngleConverterRevolutionTime(6000000UL);
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  context.current.setRpm(3000);
  context.decoderState.toothLastMinusOneToothTime = 500;
  context.decoderState.toothLastToothTime = 1000;
  ATOMIC() {
    stopIgnitionSchedulers();

    setup_perTooth(context);
    context.page2.perToothIgn = false;
    context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);
  }
}

static auto setup_secondary(uint8_t cylinders, bool resyc, uint16_t duration)
{
  trigger_context_nis360_t context;

  context.page2.nCylinders = cylinders;
  context.page4.useResync = resyc;
  context.decoderState.toothCurrentCount = 23;
  context.decoderState.secondaryToothCount = context.decoderState.toothCurrentCount - duration;

  return context;
}

static void test_triggerSecondary_sync(uint8_t cylinders, uint16_t windowWidth, uint16_t expectedCount)
{
  auto context = setup_secondary(cylinders, false, windowWidth);
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None;

  context.triggerSecondary(2000);

  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(expectedCount, context.decoderState.toothCurrentCount);
}

static void test_triggerSecondary_window_16_tooth_N4(void)
{
  test_triggerSecondary_sync(4, 15, 16);
  test_triggerSecondary_sync(4, 16, 16);
  test_triggerSecondary_sync(4, 17, 16);
}

static void test_triggerSecondary_window_12_tooth_N4(void)
{
  test_triggerSecondary_sync(4, 11, 102);
  test_triggerSecondary_sync(4, 12, 102);
  test_triggerSecondary_sync(4, 13, 102);
}

static void test_triggerSecondary_window_8_tooth_N4(void)
{
  test_triggerSecondary_sync(4, 7, 188);
  test_triggerSecondary_sync(4, 8, 188);
  test_triggerSecondary_sync(4, 9, 188);
}

static void test_triggerSecondary_window_4_tooth_N4(void)
{
  test_triggerSecondary_sync(4, 3, 274);
  test_triggerSecondary_sync(4, 4, 274);
  test_triggerSecondary_sync(4, 5, 274);
}

static void test_triggerSecondary_window_4_tooth_N6(void)
{
  test_triggerSecondary_sync(6, 3, 124);
  test_triggerSecondary_sync(6, 4, 124);
  test_triggerSecondary_sync(6, 5, 124);
}

static void test_triggerSecondary_window_8_tooth_N8(void)
{
  test_triggerSecondary_sync(8, 6, 56);
  test_triggerSecondary_sync(8, 7, 56);
  test_triggerSecondary_sync(8, 8, 56);
}

static void test_triggerSecondary_invalid_window_N4(uint16_t windowWidth)
{
  auto context = setup_secondary(4, false, windowWidth);

  context.triggerSecondary(2000);

  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(1, context.current.syncLossCounter);
}

static void test_triggerSecondary_invalid_window_N4(void)
{
  test_triggerSecondary_invalid_window_N4(1);
  test_triggerSecondary_invalid_window_N4(6);
  test_triggerSecondary_invalid_window_N4(10);
  test_triggerSecondary_invalid_window_N4(14);
  test_triggerSecondary_invalid_window_N4(18);
}

static void test_triggerSecondary_invalid_window_NOther(uint8_t cylinders, uint16_t windowWidth)
{
  auto context = setup_secondary(cylinders, false, windowWidth);

  context.triggerSecondary(2000);

  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(23, context.decoderState.toothCurrentCount);
}

static void test_triggerSecondary_invalid_window_N6(void)
{
  test_triggerSecondary_invalid_window_NOther(6, 2);
  test_triggerSecondary_invalid_window_NOther(6, 6);
}

static void test_triggerSecondary_invalid_window_N8(void)
{
  test_triggerSecondary_invalid_window_NOther(8, 5);
  test_triggerSecondary_invalid_window_NOther(8, 9);
}

static void test_triggerSecondary_resync(uint8_t cylinders, uint16_t windowWidth, uint16_t expectedCount)
{
  auto context = setup_secondary(cylinders, true, windowWidth);
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.triggerSecondary(2000);

  // No change
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(expectedCount, context.decoderState.toothCurrentCount);
}

static void test_triggerSecondary_resync_N4(void)
{
  test_triggerSecondary_resync(4, 14, 23);
  test_triggerSecondary_resync(4, 15, 16);
  test_triggerSecondary_resync(4, 17, 16);
  test_triggerSecondary_resync(4, 18, 23);
}

static void test_triggerSecondary_resync_N6(void)
{
  test_triggerSecondary_resync(6, 2, 23);
  test_triggerSecondary_resync(6, 3, 124);
  test_triggerSecondary_resync(6, 5, 124);
  test_triggerSecondary_resync(6, 6, 23);
}

static void test_triggerSecondary_resync_N8(void)
{
  test_triggerSecondary_resync(8, 6, 23);
}

static void test_triggerSecondary_already_synced_no_resync(void)
{
  auto context = setup_secondary(4, false, 10);
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.triggerSecondary(2000);

  TEST_ASSERT_EQUAL(23, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
}

void testNissan360Triggers(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_triggerPrimary_gapcheck);
  RUN_TEST_P(test_triggerPrimary_wrapCount);
  RUN_TEST_P(test_triggerPrimary_updatesTimers);
  RUN_TEST_P(test_primary_applies_per_tooth_timing);
  RUN_TEST_P(test_primary_skips_per_tooth_timing_when_disabled);

  RUN_TEST(test_triggerSecondary_window_16_tooth_N4);
  RUN_TEST(test_triggerSecondary_window_12_tooth_N4);
  RUN_TEST(test_triggerSecondary_window_8_tooth_N4);
  RUN_TEST(test_triggerSecondary_window_4_tooth_N4);
  RUN_TEST(test_triggerSecondary_window_4_tooth_N6);
  RUN_TEST(test_triggerSecondary_window_8_tooth_N8);
  RUN_TEST(test_triggerSecondary_invalid_window_N4);
  RUN_TEST(test_triggerSecondary_invalid_window_N6);
  RUN_TEST(test_triggerSecondary_invalid_window_N8);
  RUN_TEST(test_triggerSecondary_resync_N4);
  RUN_TEST(test_triggerSecondary_resync_N6);
  RUN_TEST(test_triggerSecondary_resync_N8);
  RUN_TEST(test_triggerSecondary_already_synced_no_resync);
}