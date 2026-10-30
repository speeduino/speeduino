#include "../../test_utils.h"
#include "src/decoders/subaru6_7.h"
#include "../test_decoder_shared.h"
#include "atomic.h"
#include "scheduler_ignition_controller.h"

struct trigger_context_sub67_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState = decoders::subaru67::initialise();

  void triggerPrimary(uint32_t curTime)
  {
    decoders::subaru67::triggerPrimary(curTime, current, decoderState, page2, page4);
  }

  void triggerSecondary(uint32_t curTime)
  {
    decoders::subaru67::triggerSecondary(curTime, current, decoderState);
  }
};

static void setup_primary_gap(trigger_context_sub67_t &context, uint16_t toothCount, uint32_t lastTime)
{
  context.decoderState.toothCurrentCount = toothCount;
  context.decoderState.toothLastToothTime = lastTime;
  context.decoderState.toothOneMinusOneTime = 0;
  context.decoderState.triggerFilterTime = 1000;
}

static void test_triggerPrimary_gapcheck(void)
{
  trigger_context_sub67_t context;

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

static void test_triggerPrimary_updatesTimers(void)
{
  trigger_context_sub67_t context;

  context.decoderState.toothLastToothTime = 100;
  context.decoderState.toothCurrentCount = 3;
  context.decoderState.toothSystemCount = 5;
  context.decoderState.decoderStatus.validTrigger = false;

  auto nextTime = calcNextTimePassPrimaryFilter(context.decoderState);
  context.triggerPrimary(nextTime);
  TEST_ASSERT_EQUAL(100, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL(nextTime, context.decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL(4, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(6, context.decoderState.toothSystemCount);
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
}

static void test_triggerPrimary_toomanyteeth_syncloss(void)
{
  trigger_context_sub67_t context;

  context.decoderState.toothCurrentCount = 13;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full; 
  context.current.syncLossCounter = 4;

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));

  TEST_ASSERT_EQUAL(0, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(5, context.current.syncLossCounter);
}

static void test_triggerPrimary_Nsecondarytooth_havesync(uint16_t primaryTeeth, uint16_t secondaryTeeth)
{
  trigger_context_sub67_t context;

  context.decoderState.toothCurrentCount = primaryTeeth;
  context.decoderState.secondaryToothCount = secondaryTeeth;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None; 
  context.current.syncLossCounter = 4;

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(4, context.current.syncLossCounter);
  TEST_ASSERT_EQUAL(0, context.decoderState.secondaryToothCount);
}

static void test_triggerPrimary_Nsecondarytooth_nosync(uint16_t primaryTeeth, uint16_t secondaryTeeth)
{
  trigger_context_sub67_t context;

  context.decoderState.toothCurrentCount = primaryTeeth;
  context.decoderState.secondaryToothCount = secondaryTeeth;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full; 
  context.current.syncLossCounter = 4;

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(5, context.current.syncLossCounter);
  TEST_ASSERT_EQUAL(0, context.decoderState.secondaryToothCount);
}

static void test_triggerPrimary_1secondarytooth(void)
{
  test_triggerPrimary_Nsecondarytooth_havesync(4, 1);
  test_triggerPrimary_Nsecondarytooth_havesync(10, 1);
  test_triggerPrimary_Nsecondarytooth_nosync(5, 1);
}

static void test_triggerPrimary_2secondaryteeth(void)
{
  test_triggerPrimary_Nsecondarytooth_havesync(7, 2);
  test_triggerPrimary_Nsecondarytooth_nosync(5, 2);
}

static void test_triggerPrimary_3secondaryteeth(void)
{
  test_triggerPrimary_Nsecondarytooth_havesync(1, 3);
  test_triggerPrimary_Nsecondarytooth_nosync(5, 3);
}

static void test_triggerPrimary_Nsecondaryteeth(void)
{
  test_triggerPrimary_Nsecondarytooth_nosync(1, 4);
  test_triggerPrimary_Nsecondarytooth_nosync(5, 5);
}

static void test_triggerPrimary_2nd_sync_wraps_count(void)
{
  trigger_context_sub67_t context;

  context.decoderState.toothCurrentCount = 12;
  context.decoderState.secondaryToothCount = 0;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full; 
  context.current.startRevolutions = 7;
  context.decoderState.toothOneTime = 100;
  
  auto nextTime = calcNextTimePassPrimaryFilter(context.decoderState);
  context.triggerPrimary(nextTime);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(100, context.decoderState.toothOneMinusOneTime);
  TEST_ASSERT_EQUAL(nextTime, context.decoderState.toothOneTime);
  TEST_ASSERT_EQUAL(8, context.current.startRevolutions);
}

static void test_triggerPrimary_toothAngle(uint16_t primaryTooth, uint16_t expectedAngle)
{
  trigger_context_sub67_t context;

  context.decoderState.toothCurrentCount = primaryTooth;
  context.decoderState.secondaryToothCount = 0;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full; 
  
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_EQUAL(expectedAngle, context.decoderState.triggerToothAngle);
}

static void test_triggerPrimary_toothAngle(void)
{
  test_triggerPrimary_toothAngle(0, 55);
  test_triggerPrimary_toothAngle(1, 93);
  test_triggerPrimary_toothAngle(3, 170-115);
}

static void setup_perTooth(trigger_context_sub67_t &context)
{
  context.decoderState.toothCurrentCount = 4;
  context.decoderState.ignitionEndTeeth[0] = context.decoderState.toothCurrentCount+1;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  context.page2.perToothIgn = true;

  context.current.rotationStatus = EngineRotationStatus::Running;
  context.current.startRevolutions = 99;

  ignitionSchedule1._status = PENDING;
  ignitionSchedule1._counter = 100;
  ignitionSchedule1._compare = 100;
  ignitionSchedule1.chargeAngle = 577;
}

static void assert_pertoothign(trigger_context_sub67_t &context)
{
  auto newCrankAngle = context.decoderState.toothAngles[context.decoderState.toothCurrentCount];
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);
}

static void test_primary_applies_per_tooth_timing(void)
{
  trigger_context_sub67_t context;

  setAngleConverterRevolutionTime(6000000UL);
  context.current.setRpm(3000);

  ATOMIC() {
    stopIgnitionSchedulers();

    setup_perTooth(context);
    assert_pertoothign(context);

    setup_perTooth(context);
    context.decoderState.toothCurrentCount = 8;
    context.decoderState.ignitionEndTeeth[0] = context.decoderState.toothCurrentCount+1-6;
    assert_pertoothign(context);

    setup_perTooth(context);
    context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    assert_pertoothign(context);
  }
}

static void test_triggerSecondary_invalidteeth_nosyncloss(uint16_t sysToothCount)
{
  trigger_context_sub67_t context;

  context.decoderState.toothSystemCount = sysToothCount;
  context.decoderState.secondaryToothCount = 3;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full; 

  context.triggerSecondary(1000);

  TEST_ASSERT_EQUAL(0, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus); 
}

static void test_triggerSecondary_invalidteeth_nosyncloss(void)
{
  test_triggerSecondary_invalidteeth_nosyncloss(1);
  test_triggerSecondary_invalidteeth_nosyncloss(2);
}

static void test_triggerSecondary_invalidteeth_syncloss(void)
{
  trigger_context_sub67_t context;

  context.decoderState.toothSystemCount = 4;
  context.decoderState.secondaryToothCount = 3;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full; 

  context.triggerSecondary(1000);

  TEST_ASSERT_EQUAL(0, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus); 
}

static void setup_secondary_gap(trigger_context_sub67_t &context, uint16_t toothCount, uint32_t lastTime)
{
  context.decoderState.toothSystemCount = toothCount;
  context.decoderState.toothLastSecToothTime = lastTime;
  context.decoderState.triggerSecFilterTime = 1000;
}

static void test_triggerSecondary_gapcheck(void)
{
  trigger_context_sub67_t context;

  constexpr uint16_t TOOTH_COUNT = 3;
  constexpr uint32_t LAST_TOOTHTIME = 777777;

  setup_secondary_gap(context, TOOTH_COUNT, LAST_TOOTHTIME);
  context.triggerSecondary(calcNextTimePassSecondaryFilter(context.decoderState)-2);
  TEST_ASSERT_EQUAL(TOOTH_COUNT, context.decoderState.toothSystemCount);
  TEST_ASSERT_EQUAL(LAST_TOOTHTIME, context.decoderState.toothLastSecToothTime);

  setup_secondary_gap(context, TOOTH_COUNT, LAST_TOOTHTIME);
  context.triggerSecondary(calcNextTimePassSecondaryFilter(context.decoderState));
  TEST_ASSERT_NOT_EQUAL(TOOTH_COUNT, context.decoderState.toothSystemCount);
  TEST_ASSERT_NOT_EQUAL(LAST_TOOTHTIME, context.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL(0, context.decoderState.toothSystemCount);
}

static void test_triggerSecondary_updatesFilter(void)
{
  trigger_context_sub67_t context;

  constexpr uint32_t LAST_TOOTHTIME = 777777;

  setup_secondary_gap(context, 0, LAST_TOOTHTIME);
  context.decoderState.secondaryToothCount = 0;
  context.triggerSecondary(calcNextTimePassSecondaryFilter(context.decoderState));
  TEST_ASSERT_EQUAL(0, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL(1, context.decoderState.secondaryToothCount);

  setup_secondary_gap(context, 3, LAST_TOOTHTIME);
  context.decoderState.secondaryToothCount = 1;
  context.triggerSecondary(calcNextTimePassSecondaryFilter(context.decoderState));
  TEST_ASSERT_EQUAL(250, context.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL(2, context.decoderState.secondaryToothCount);
}

void testSubaru67Triggers(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST(test_triggerPrimary_gapcheck);
  RUN_TEST(test_triggerPrimary_updatesTimers);
  RUN_TEST(test_triggerPrimary_toomanyteeth_syncloss);
  RUN_TEST(test_triggerPrimary_1secondarytooth);
  RUN_TEST(test_triggerPrimary_2secondaryteeth);
  RUN_TEST(test_triggerPrimary_3secondaryteeth);
  RUN_TEST(test_triggerPrimary_Nsecondaryteeth);
  RUN_TEST(test_triggerPrimary_2nd_sync_wraps_count);
  RUN_TEST(test_triggerPrimary_toothAngle);
  RUN_TEST(test_primary_applies_per_tooth_timing);

  RUN_TEST(test_triggerSecondary_invalidteeth_nosyncloss);
  RUN_TEST(test_triggerSecondary_invalidteeth_syncloss);
  RUN_TEST(test_triggerSecondary_gapcheck);
  RUN_TEST(test_triggerSecondary_updatesFilter);
}
