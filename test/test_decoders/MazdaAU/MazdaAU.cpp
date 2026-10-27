#include "../test_utils.h"
#include "src/decoders/mazdaau.h"
#include "../test_decoder_shared.h"

static void test_getCrankAngle(void)
{
  config4 page4={};
  auto decoderState = decoders::mazdaau::initialise();

  auto setup_case = [&](int toothNum, int trigAngle) {
    decoderState.toothLastToothTime = 2000;
    decoderState.toothCurrentCount = toothNum;
    decoderState.decoderStatus.toothAngleIsCorrect = true;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    page4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
  };

  auto run_case = [&](int toothNum, int16_t expected, int trigAngle = 0) {
    setup_case(toothNum, trigAngle);
    TEST_ASSERT_EQUAL(expected, decoders::mazdaau::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));
  };

  // timeToAngle(100) ~= 18 deg
  const int dt = 18;

  // toothAngles from triggerSetup_MazdaAU: [348,96,168,276]
  run_case(1, 348 + dt);
  run_case(2, 96 + dt);
  run_case(3, 168 + dt);
  run_case(4, 276 + dt);

  // trigger angle offset
  run_case(2, 96 + dt + 10, 10);

  // Zero if not full sync
  setup_case(1, 0);
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL(0, decoders::mazdaau::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));
  setup_case(1, 0);
  decoderState.decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL(0, decoders::mazdaau::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));
}

static void test_getRevolutionTime(void)
{
  statuses current={};
  auto decoderState = decoders::mazdaau::initialise();

  // Ensure sync present
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // --- Cranking branch: current.RPM < current.crankRPM
  // Revolution time is calculated from the last tooth gap, whichever of the uneven gaps that is.
  current.setRpm(0);
  current.crankRPM = 200;
  current.revolutionTime = 12345UL;
  // 108 degree gap of 180000uS -> revTime = 180000 * 360 / 108 = 600000 (100 RPM)
  decoderState.triggerToothAngle = 108;
  decoderState.toothLastMinusOneToothTime = 1000UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 180000UL;
  TEST_ASSERT_EQUAL_UINT32(600000UL, decoders::mazdaau::getRevolutionTime(current, decoderState));
  // 72 degree gap of 120000uS -> revTime = 120000 * 360 / 72 = 600000 (100 RPM)
  decoderState.triggerToothAngle = 72;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 120000UL;
  TEST_ASSERT_EQUAL_UINT32(600000UL, decoders::mazdaau::getRevolutionTime(current, decoderState));

  // No valid tooth - use last published time
  current.revolutionTime = 999999UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime;
  TEST_ASSERT_EQUAL_UINT32(999999UL, decoders::mazdaau::getRevolutionTime(current, decoderState));

  // --- Running path: uses stdGetRevolutionTime(CRANK_SPEED)
  current.setRpm(2000);
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 60000UL; // revTime = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::mazdaau::getRevolutionTime(current, decoderState));

  // --- Fallback: when not full sync, the speed is unknown
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoders::mazdaau::getRevolutionTime(current, decoderState));
}

static void test_triggerPrimary_gapcheck(void)
{
  statuses current = {};
  config4 page4 = {};
  auto decoderState = decoders::mazdaau::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  constexpr uint16_t TOOTH_COUNT = 5; // Arbitrary
  constexpr uint32_t LAST_TOOTHTIME = 777777;

  decoderState.toothCurrentCount = TOOTH_COUNT; // Start at some tooth count
  decoderState.toothLastToothTime = LAST_TOOTHTIME;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.triggerFilterTime = 1000;
  decoders::mazdaau::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState)-2, current, decoderState, page4);
  TEST_ASSERT_EQUAL(TOOTH_COUNT, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(LAST_TOOTHTIME, decoderState.toothLastToothTime);

  decoderState.toothCurrentCount = TOOTH_COUNT; // Start at some tooth count
  decoderState.toothLastToothTime = LAST_TOOTHTIME;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.triggerFilterTime = 1000;
  decoders::mazdaau::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page4);
  TEST_ASSERT_NOT_EQUAL(TOOTH_COUNT, decoderState.toothCurrentCount);
  TEST_ASSERT_NOT_EQUAL(LAST_TOOTHTIME, decoderState.toothLastToothTime);
}

static void test_triggerPrimary_wrapCount(void)
{
  statuses current = {};
  config4 page4 = {};
  auto decoderState = decoders::mazdaau::initialise();

  constexpr uint32_t LAST_TOOTHTIME = 777777;

  current.startRevolutions = 1;
  decoderState.toothCurrentCount = 4;
  decoderState.toothLastToothTime = LAST_TOOTHTIME;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.triggerFilterTime = 1000;

  decoders::mazdaau::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page4);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(2, current.startRevolutions);
}

static void test_primaryTrigger_toothAngle(uint16_t toothNum, uint16_t expectedAngle, uint32_t expectedFilterTime)
{
  statuses current = {};
  config4 page4 = {};
  auto decoderState = decoders::mazdaau::initialise();
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  constexpr uint32_t LAST_TOOTHTIME = 777777;

  decoderState.toothCurrentCount = toothNum;
  decoderState.toothLastToothTime = LAST_TOOTHTIME;
  decoderState.toothOneMinusOneTime = 0;
  decoderState.triggerFilterTime = 1000;
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  decoders::mazdaau::triggerPrimary(calcNextTimePassPrimaryFilter(decoderState), current, decoderState, page4);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL(expectedAngle, decoderState.triggerToothAngle);
  TEST_ASSERT_EQUAL(expectedFilterTime, decoderState.triggerFilterTime);
}

static void test_primaryTrigger_toothAngle(void)
{
  test_primaryTrigger_toothAngle(1, 108, 375);
  test_primaryTrigger_toothAngle(2, 72, 1001);
  test_primaryTrigger_toothAngle(3, 108, 375);
  test_primaryTrigger_toothAngle(4, 72, 1001);
}

static void test_triggerSecondary_rollCounters(void)
{
  auto decoderState = decoders::mazdaau::initialise();

  decoderState.curGap2 = 100;
  decoderState.toothLastSecToothTime = 1000;
  decoderState.lastGap = 2000;
  
  decoders::mazdaau::triggerSecondary(1500, decoderState);
  TEST_ASSERT_EQUAL(100, decoderState.lastGap);
  TEST_ASSERT_EQUAL(500, decoderState.curGap2);
  TEST_ASSERT_EQUAL(1500, decoderState.toothLastSecToothTime);
}

static void test_triggerSecondary_toFullSyncOnTooth2(void)
{
  auto decoderState = decoders::mazdaau::initialise();

  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  decoderState.secondaryToothCount = 2;
  
  decoders::mazdaau::triggerSecondary(1500, decoderState);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(3, decoderState.secondaryToothCount);
}

static void setup_triggerSecondary_gapFilter(decoders::shared::state_t &decoderState)
{
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  decoderState.secondaryToothCount = 3;
  decoderState.curGap2 = 1000;
  decoderState.toothLastSecToothTime = 499;
}

static uint32_t calcTimeToPassSecondaryFilter(const decoders::shared::state_t &decoderState)
{
  return decoderState.toothLastSecToothTime + (decoderState.curGap2/2U) - 1;
}

static void test_triggerSecondary_gapFilter(void)
{
  auto decoderState = decoders::mazdaau::initialise();

  setup_triggerSecondary_gapFilter(decoderState);  
  decoders::mazdaau::triggerSecondary(calcTimeToPassSecondaryFilter(decoderState)+1, decoderState);
  TEST_ASSERT_EQUAL(1500, decoderState.triggerFilterTime);
  TEST_ASSERT_EQUAL(500, decoderState.targetGap);
  TEST_ASSERT_EQUAL(4, decoderState.secondaryToothCount);

  setup_triggerSecondary_gapFilter(decoderState);  
  decoders::mazdaau::triggerSecondary(calcTimeToPassSecondaryFilter(decoderState), decoderState);
  TEST_ASSERT_EQUAL(1500, decoderState.triggerFilterTime);
  TEST_ASSERT_EQUAL(500, decoderState.targetGap);
  TEST_ASSERT_EQUAL(3, decoderState.secondaryToothCount);
}

void testMazdaAU(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getCrankAngle);
  RUN_TEST_P(test_getRevolutionTime);

  RUN_TEST_P(test_triggerPrimary_gapcheck);
  RUN_TEST_P(test_triggerPrimary_wrapCount);
  RUN_TEST_P(test_primaryTrigger_toothAngle);
  RUN_TEST_P(test_triggerSecondary_rollCounters);
  RUN_TEST_P(test_triggerSecondary_toFullSyncOnTooth2);
  RUN_TEST_P(test_triggerSecondary_gapFilter);
}
