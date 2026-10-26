#include "../test_utils.h"
#include "src/decoders/miata9905.h"
#include "scheduler_ignition_controller.h"
#include "atomic.h"

static uint32_t calcNextTimePassPrimaryFilter(const decoders::shared::state_t &decoderState)
{
  return decoderState.toothLastToothTime + decoderState.triggerFilterTime + 1;
}

static void test_triggerPrimary_gapcheck(void)
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  constexpr uint16_t TOOTH_COUNT = 5; // Arbitrary
  constexpr uint32_t LAST_TOOTHTIME = 777777;

  decoderState.toothCurrentCount = TOOTH_COUNT; // Start at some tooth count
  decoderState.toothLastToothTime = LAST_TOOTHTIME;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.triggerFilterTime = 1000;
  current.startRevolutions = 1;
  decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState)-2, current, decoderState, page2, page4);
  TEST_ASSERT_EQUAL(TOOTH_COUNT, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(LAST_TOOTHTIME, decoderState.toothLastToothTime);

  decoderState.toothCurrentCount = TOOTH_COUNT; // Start at some tooth count
  decoderState.toothLastToothTime = LAST_TOOTHTIME;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.triggerFilterTime = 1000;
  current.startRevolutions = 0;
  decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState)-2, current, decoderState, page2, page4);
  TEST_ASSERT_NOT_EQUAL(TOOTH_COUNT, decoderState.toothCurrentCount);
  TEST_ASSERT_NOT_EQUAL(LAST_TOOTHTIME, decoderState.toothLastToothTime);

  decoderState.toothCurrentCount = TOOTH_COUNT; // Start at some tooth count
  decoderState.toothLastToothTime = LAST_TOOTHTIME;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.triggerFilterTime = 1000;
  current.startRevolutions = 0;
  decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page2, page4);
  TEST_ASSERT_NOT_EQUAL(TOOTH_COUNT, decoderState.toothCurrentCount);
  TEST_ASSERT_NOT_EQUAL(LAST_TOOTHTIME, decoderState.toothLastToothTime);
}

static void test_triggerPrimary_noSync(void)
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  // Setup for no sync
  constexpr uint32_t FILTER_TIME=777777;
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  decoderState.toothCurrentCount = 5;
  decoderState.toothLastToothTime = 10000;
  decoderState.triggerFilterTime = FILTER_TIME;
  decoderState.triggerSecFilterTime = FILTER_TIME;

  // Call the function - should not increment count
  decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page2, page4);

  TEST_ASSERT_EQUAL(FILTER_TIME, decoderState.triggerFilterTime);
  TEST_ASSERT_EQUAL(FILTER_TIME, decoderState.triggerSecFilterTime);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
}

static void test_triggerPrimary_wrapCount(void)
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  constexpr uint32_t LAST_TOOTHTIME = 777777;

  current.startRevolutions = 1;
  decoderState.toothCurrentCount = decoderState.triggerActualTeeth;
  decoderState.toothLastToothTime = LAST_TOOTHTIME;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.triggerFilterTime = 1000;

  decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page2, page4);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(2, current.startRevolutions);
}

static void test_triggerPrimary_resyncCondition(void)
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  // Setup for resync condition: (syncStatus != Full) || (page4.useResync == true) is true
  // and secondaryToothCount == 2
  decoderState.decoderStatus.syncStatus = SyncStatus::None; // Fulfills the syncStatus check
  page4.useResync = true; // Fulfills the page4.useResync check
  decoderState.secondaryToothCount = 2;

  decoderState.toothCurrentCount = 5;
  decoderState.toothLastToothTime = 10000;
  decoderState.decoderStatus.toothAngleIsCorrect = true; // Assume angle is correct for primary trigger logic

  const uint32_t nextToothTime = 10000 + 100;

  // Call the function - should trigger resync logic
  decoders::miata9905::triggerPrimary(nextToothTime, current, decoderState, page2, page4);

  // Assert tooth count incremented AND sync status updated to Full
  TEST_ASSERT_EQUAL(6, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
}

static void test_primaryFilter(uint8_t filter, uint16_t toothNum, uint16_t expectedAngle, uint32_t expectedFilterTime)
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  constexpr uint32_t LAST_TOOTHTIME = 777777;

  current.startRevolutions = 1;
  current.setRpm(1500);

  page4.triggerFilter = filter;

  decoderState.toothCurrentCount = toothNum;
  decoderState.toothLastToothTime = LAST_TOOTHTIME;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.triggerFilterTime = 1000;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page2, page4);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL(expectedAngle, decoderState.triggerToothAngle);
  TEST_ASSERT_EQUAL(expectedFilterTime, decoderState.triggerFilterTime);
}

static void test_triggerPrimary_filter(uint8_t filter, uint32_t expectedFilterTime110, uint32_t expectedFilterTime70)
{
  test_primaryFilter(filter, 1, 110, expectedFilterTime110);
  test_primaryFilter(filter, 2, 70, expectedFilterTime70);
  test_primaryFilter(filter, 3, 110, expectedFilterTime110);
  test_primaryFilter(filter, 4, 70, expectedFilterTime70);
  test_primaryFilter(filter, 5, 110, expectedFilterTime110);
  test_primaryFilter(filter, 6, 70, expectedFilterTime70);
  test_primaryFilter(filter, 7, 110, expectedFilterTime110);
  test_primaryFilter(filter, 8, 70, expectedFilterTime70);
}

static void test_triggerPrimary_filters(void)
{
  test_triggerPrimary_filter(TRIGGER_FILTER_OFF, 0, 0);
  test_triggerPrimary_filter(TRIGGER_FILTER_LITE, 375, 1001);
  test_triggerPrimary_filter(TRIGGER_FILTER_MEDIUM, 500, 1251);
  test_triggerPrimary_filter(TRIGGER_FILTER_AGGRESSIVE, 281, 1376);
}

static void setup_perTooth(statuses &current, decoders::shared::state_t &decoderState, config2 &page2, config4 &page4)
{
  decoderState.toothCurrentCount = 1;
  decoderState.ignitionEndTeeth[0] = 2;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  page2.perToothIgn = true;
  page4.triggerAngle = 0;

  current.rotationStatus = EngineRotationStatus::Running;
  current.advance = 5;
  current.startRevolutions = 99;

  ignitionSchedule1._status = PENDING;
  ignitionSchedule1._counter = 100;
  ignitionSchedule1._compare = 100;
  ignitionSchedule1.chargeAngle = 200;
}

static void test_primary_applies_per_tooth_timing(void)
{
  statuses current{};
  auto decoderState = decoders::miata9905::initialise(1000, current);
  config2 page2{};
  config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    current.setRpm(3000);
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    ATOMIC() {
      stopIgnitionSchedulers();

      setup_perTooth(current, decoderState, page2, page4);
      decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page2, page4);
      TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(200 - 100), ignitionSchedule1._compare);

      setup_perTooth(current, decoderState, page2, page4);
      page4.sparkMode = IGN_MODE_SEQUENTIAL;
      decoderState.toothCurrentCount += page2.nCylinders;
      decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page2, page4);
      TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(200 - 100), ignitionSchedule1._compare);
    }
}

static void test_primary_skips_per_tooth_timing_when_disabled(void)
{
  statuses current{};
  auto decoderState = decoders::miata9905::initialise(1000, current);
  config2 page2{};
  config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    current.setRpm(3000);
    decoderState.toothLastMinusOneToothTime = 500;
    decoderState.toothLastToothTime = 1000;
    ATOMIC() {
      stopIgnitionSchedulers();

      setup_perTooth(current, decoderState, page2, page4);
      page2.perToothIgn = false;
      decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page2, page4);
      TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);

      setup_perTooth(current, decoderState, page2, page4);
      page4.triggerAngle = 1;
      decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page2, page4);
      TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);

      setup_perTooth(current, decoderState, page2, page4);
      current.advance = 0;
      decoders::miata9905::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page2, page4);
      TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);
    }
}

static void test_triggerSecondary_resetfiltertime()
{
  statuses current = {};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  current.rotationStatus = EngineRotationStatus::Cranking;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.triggerFilterTime = 7777;
  decoders::miata9905::triggerSecondary(0, current, decoderState);
  TEST_ASSERT_EQUAL(1500, decoderState.triggerFilterTime);

  current.rotationStatus = EngineRotationStatus::Running;
  decoderState.decoderStatus.syncStatus = SyncStatus::Partial;
  decoderState.triggerFilterTime = 7777;
  decoders::miata9905::triggerSecondary(0, current, decoderState);
  TEST_ASSERT_EQUAL(1500, decoderState.triggerFilterTime);

  current.rotationStatus = EngineRotationStatus::Running;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.triggerFilterTime = 7777;
  decoders::miata9905::triggerSecondary(0, current, decoderState);
  TEST_ASSERT_NOT_EQUAL(1500, decoderState.triggerFilterTime);
}

static uint32_t calcNextTimePassSecondaryFilter(const decoders::shared::state_t &decoderState)
{
  return decoderState.toothLastSecToothTime + decoderState.triggerSecFilterTime + 1;
}

static void test_triggerSecondary_gapcheck(void)
{
  statuses current = {};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  constexpr uint16_t TOOTH_COUNT = 5; // Arbitrary
  constexpr uint32_t LAST_TOOTHTIME = 777777;

  decoderState.secondaryToothCount = TOOTH_COUNT; // Start at some tooth count
  decoderState.toothLastSecToothTime = LAST_TOOTHTIME;
  decoderState.triggerSecFilterTime = 1000;
  decoders::miata9905::triggerSecondary(calcNextTimePassSecondaryFilter(decoderState)-2, current, decoderState);
  TEST_ASSERT_EQUAL(TOOTH_COUNT, decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL(LAST_TOOTHTIME, decoderState.toothLastSecToothTime);

  decoderState.secondaryToothCount = TOOTH_COUNT; // Start at some tooth count
  decoderState.toothLastSecToothTime = LAST_TOOTHTIME;
  decoderState.triggerSecFilterTime = 1000;
  decoders::miata9905::triggerSecondary(calcNextTimePassSecondaryFilter(decoderState), current, decoderState);
  TEST_ASSERT_NOT_EQUAL(TOOTH_COUNT, decoderState.secondaryToothCount);
  TEST_ASSERT_NOT_EQUAL(LAST_TOOTHTIME, decoderState.toothLastSecToothTime);
}

static void test_triggerSecondary_capture_vvt(void)
{
  statuses current = {};
  auto decoderState = decoders::miata9905::initialise(1000, current);

  constexpr uint32_t LAST_TOOTHTIME = 777777;
  decoderState.toothLastSecToothTime = LAST_TOOTHTIME;
  decoderState.triggerSecFilterTime = 1000;

  decoderState.lastVVTtime = 111111;
  decoderState.toothLastToothTime = calcNextTimePassSecondaryFilter(decoderState);
  decoderState.toothCurrentCount = 3;
  decoders::miata9905::triggerSecondary(decoderState.toothLastToothTime, current, decoderState);
  TEST_ASSERT_EQUAL(111111, decoderState.lastVVTtime);

  decoderState.toothLastToothTime = calcNextTimePassSecondaryFilter(decoderState)/2;
  decoderState.toothCurrentCount = 3;
  decoders::miata9905::triggerSecondary(decoderState.toothLastToothTime*2, current, decoderState);
  TEST_ASSERT_EQUAL(111111, decoderState.lastVVTtime);

  decoderState.toothLastToothTime = calcNextTimePassSecondaryFilter(decoderState)/2;
  decoderState.toothCurrentCount = 1;
  decoders::miata9905::triggerSecondary(decoderState.toothLastToothTime*2, current, decoderState);
  TEST_ASSERT_NOT_EQUAL(111111, decoderState.lastVVTtime);
}

void testMiata9905Triggers(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_triggerPrimary_gapcheck);
  RUN_TEST_P(test_triggerPrimary_wrapCount);
  RUN_TEST_P(test_triggerPrimary_noSync);
  RUN_TEST_P(test_triggerPrimary_resyncCondition);
  RUN_TEST_P(test_triggerPrimary_filters);
  RUN_TEST_P(test_primary_applies_per_tooth_timing);
  RUN_TEST_P(test_primary_skips_per_tooth_timing_when_disabled);
  RUN_TEST_P(test_triggerSecondary_resetfiltertime);
  RUN_TEST_P(test_triggerSecondary_gapcheck);
  RUN_TEST_P(test_triggerSecondary_capture_vvt);
}