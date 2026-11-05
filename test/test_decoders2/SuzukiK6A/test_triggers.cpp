#include "../../test_utils.h"
#include "test_context.h"
#include "scheduler_ignition_controller.h"
#include "../../test_decoders/test_decoder_shared.h"
#include "atomic.h"

static void test_triggerPrimary_rejects_filtered_pulse(void)
{
    context_k6a_t context;
    context.init();
    context.setup_primaryfilter(1000, 1500);
    context.current.startRevolutions = 1;

    context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1499);

    TEST_ASSERT_FALSE(context.decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastToothTime);
    TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME - 1000, context.decoderState.toothLastMinusOneToothTime);
}

static void test_triggerPrimary_startup_accepts_pulse_below_filter(void)
{
    context_k6a_t context;
    context.init();
    context.setup_primaryfilter(1000, 1500);

    context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1);

    TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME + 1, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_accepts_regular_tooth(void)
{
    context_k6a_t context;
    context.init();
    context.setup_primaryfilter(1000, 500);
    context.decoderState.toothCurrentCount = 1;
    context.current.startRevolutions = 1;

    context.triggerPrimary(BASE_1ST_TOOTH_TIME + 1000);

    TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastMinusOneToothTime);
    TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME + 1000, context.decoderState.toothLastToothTime);
    TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
}

static void test_triggerPrimary_acquires_sync_on_extra_tooth(void)
{
    context_k6a_t context;
    context.init();
    context.setup_primaryfilter(1000, 50);
    context.decoderState.curGap2 = 200;
    context.decoderState.curGap3 = 300;
    context.page4.triggerFilter = 2;
    context.current.startRevolutions = 1;

    context.triggerPrimary(BASE_1ST_TOOTH_TIME + 100);

    TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
    TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(6, context.decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(200, context.decoderState.triggerFilterTime);
}

static void test_triggerPrimary_wraps_revolution_after_sync(void)
{
    context_k6a_t context;
    context.init();
    context.setup_primaryfilter(1000, 500);
    context.decoderState.toothCurrentCount = 7;
    context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    context.current.startRevolutions = 1;

    context.triggerPrimary(BASE_1ST_TOOTH_TIME + 2000);

    TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT32(3, context.current.startRevolutions);
    TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothOneMinusOneTime);
    TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME + 2000, context.decoderState.toothOneTime);
}

static void test_triggerPrimary_loses_sync_after_tooth_range(void)
{
    context_k6a_t context;
    context.init();
    context.setup_primaryfilter(1000, 500);
    context.decoderState.toothCurrentCount = 8;
    context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    context.current.startRevolutions = 1;

    context.triggerPrimary(BASE_1ST_TOOTH_TIME + 2000);

    TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT16(1, context.current.syncLossCounter);
    TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.triggerFilterTime);
}

  static void test_triggerPrimary_loses_sync_when_short_tooth_gap_is_too_long(void)
  {
    // TODO: requires bug fix
  #if false
    context_k6a_t context;
    context.init();
    context.setup_primaryfilter(1000, 50);
    context.current.startRevolutions = 1;
    context.decoderState.toothCurrentCount = 4;
    context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    context.decoderState.curGap2 = 100;
    context.decoderState.curGap3 = 50;

    context.triggerPrimary(BASE_1ST_TOOTH_TIME + 200);

    TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(2, context.decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT16(1, context.current.syncLossCounter);
    TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.triggerFilterTime);
  #endif
  }

  static void test_triggerPrimary_loses_sync_when_long_tooth_gap_is_too_short(void)
  {
    // TODO: requires bug fix
  #if false
    context_k6a_t context;
    context.init();
    context.setup_primaryfilter(1000, 50);
    context.current.startRevolutions = 1;
    context.decoderState.toothCurrentCount = 1;
    context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    context.decoderState.curGap2 = 200;
    context.decoderState.curGap3 = 100;

    context.triggerPrimary(BASE_1ST_TOOTH_TIME + 100);

    TEST_ASSERT_EQUAL(SyncStatus::None, context.decoderState.decoderStatus.syncStatus);
    TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT16(1, context.current.syncLossCounter);
    TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.triggerFilterTime);
#endif
}

  static void assert_trigger_filter_modes(uint8_t targetTooth, uint32_t gap, uint32_t previousGap,
                      uint32_t gapBeforePrevious, const uint32_t (&expectedFilters)[4])
  {
    for (uint8_t filterMode = TRIGGER_FILTER_OFF; filterMode <= TRIGGER_FILTER_AGGRESSIVE; filterMode++)
    {
      context_k6a_t context;
      context.init();
      context.setup_primaryfilter(1000, 50);
      context.current.startRevolutions = 1;
      context.decoderState.toothCurrentCount = targetTooth - 1;
      context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
      context.decoderState.curGap2 = previousGap;
      context.decoderState.curGap3 = gapBeforePrevious;
      context.page4.triggerFilter = filterMode;

      context.triggerPrimary(BASE_1ST_TOOTH_TIME + gap);

      TEST_ASSERT_EQUAL_UINT16(targetTooth, context.decoderState.toothCurrentCount);
      TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
      TEST_ASSERT_EQUAL_UINT32(expectedFilters[filterMode], context.decoderState.triggerFilterTime);
    }
  }

  static void test_triggerPrimary_updates_filter_for_each_tooth_and_mode(void)
  {
    const uint32_t largeGapFilters[4] = { 0, 40, 60, 100 };
    const uint32_t tooth5Filters[4] = { 0, 20, 40, 60 };
    const uint32_t tooth6Filters[4] = { 0, 160, 320, 480 };
    const uint32_t tooth7Filters[4] = { 0, 40, 80, 120 };
    const uint32_t shortGapFilters[4] = { 0, 100, 200, 280 };

    assert_trigger_filter_modes(1, 160, 320, 160, shortGapFilters);
    assert_trigger_filter_modes(2, 320, 160, 160, largeGapFilters);
    assert_trigger_filter_modes(3, 160, 320, 160, shortGapFilters);
    assert_trigger_filter_modes(4, 320, 160, 160, largeGapFilters);
    assert_trigger_filter_modes(5, 160, 320, 160, tooth5Filters);
    assert_trigger_filter_modes(6, 160, 320, 160, tooth6Filters);
    assert_trigger_filter_modes(7, 320, 160, 160, tooth7Filters);
  }

static void setup_perTooth(context_k6a_t &context)
{
  context.init();
  setAngleConverterRevolutionTime(6000000UL);
  context.current.setRpm(3000);

  context.decoderState.toothCurrentCount = 1;
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

static void assert_pertoothign(context_k6a_t &context, uint16_t newCrankAngle)
{
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);
}

static void assert_pertoothign360(context_k6a_t &context)
{
  auto newCrankAngle = context.decoderState.toothAngles[context.decoderState.toothCurrentCount+1] + context.page4.triggerAngle;
  assert_pertoothign(context, newCrankAngle);
}

static void assert_pertoothign720(context_k6a_t &context)
{
  auto newCrankAngle = 360 + context.page4.triggerAngle + ( (context.decoderState.toothCurrentCount) * context.decoderState.triggerToothAngle );
  assert_pertoothign(context, newCrankAngle);
}

static void test_triggerPrimary_applies_per_tooth_timing(void)
{
  ATOMIC() {
    stopIgnitionSchedulers();

    context_k6a_t context;
    setup_perTooth(context);
    assert_pertoothign360(context);

    // context = context_k6a_t();
    // setup_perTooth(context);
    // context.page4.TrigSpeed = CRANK_SPEED;
    // context.page4.sparkMode = IGN_MODE_SEQUENTIAL;
    // context.decoderState.revolutionOne = true;
    // context.decoderState.ignitionEndTeeth[0] = context.page4.triggerTeeth + context.decoderState.toothCurrentCount+1;
    // assert_pertoothign720(context);
  }
}

void testSuzukiK6ATriggers(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_triggerPrimary_rejects_filtered_pulse);
    RUN_TEST_P(test_triggerPrimary_startup_accepts_pulse_below_filter);
    RUN_TEST_P(test_triggerPrimary_accepts_regular_tooth);
    RUN_TEST_P(test_triggerPrimary_acquires_sync_on_extra_tooth);
    RUN_TEST_P(test_triggerPrimary_wraps_revolution_after_sync);
    RUN_TEST_P(test_triggerPrimary_loses_sync_after_tooth_range);
    RUN_TEST_P(test_triggerPrimary_loses_sync_when_short_tooth_gap_is_too_long);
    RUN_TEST_P(test_triggerPrimary_loses_sync_when_long_tooth_gap_is_too_short);
    RUN_TEST_P(test_triggerPrimary_updates_filter_for_each_tooth_and_mode);
    RUN_TEST_P(test_triggerPrimary_applies_per_tooth_timing);
  }
}