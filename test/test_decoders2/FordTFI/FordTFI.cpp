#include "../../test_utils.h"
#include "test_context.h"
#include "scheduler_ignition_controller.h"
#include "../../test_decoders/test_decoder_shared.h"
#include "atomic.h"

static void test_triggerPrimary_rejects_filtered_pulse(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.setup_primaryfilter(1000, 500);
  ctx.current.startRevolutions = 1;

  ctx.triggerPrimary(calcNextTimePassPrimaryFilter(ctx.decoderState)-2);

  TEST_ASSERT_FALSE(ctx.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT8(0, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(500, ctx.decoderState.triggerFilterTime);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, ctx.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_startup_bypasses_filter(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.setup_primaryfilter(1000, 1500);

  auto nextTime = calcNextTimePassPrimaryFilter(ctx.decoderState);
  ctx.triggerPrimary(nextTime);

  TEST_ASSERT_TRUE(ctx.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT8(1, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(0, ctx.decoderState.triggerFilterTime);
  TEST_ASSERT_EQUAL_UINT32(nextTime, ctx.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_accepts_unsynced_tooth_and_disables_filter(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.setup_primaryfilter(1000, 500);
  ctx.current.startRevolutions = 1;
  ctx.page4.triggerFilter = TRIGGER_FILTER_MEDIUM;

  auto nextTime = calcNextTimePassPrimaryFilter(ctx.decoderState);
  ctx.triggerPrimary(nextTime);

  TEST_ASSERT_TRUE(ctx.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT8(1, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(0, ctx.decoderState.triggerFilterTime);
  TEST_ASSERT_EQUAL_UINT32(nextTime, ctx.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_recalculates_filter_when_synced(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.setup_primaryfilter(1000, 1000);
  ctx.decoderState.toothCurrentCount = 1;
  ctx.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  ctx.current.startRevolutions = 1;
  ctx.page4.triggerFilter = TRIGGER_FILTER_MEDIUM;

  ctx.triggerPrimary(calcNextTimePassPrimaryFilter(ctx.decoderState));

  TEST_ASSERT_TRUE(ctx.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT8(2, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(500, ctx.decoderState.triggerFilterTime);
}

static void test_triggerPrimary_handles_timer_rollover(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.decoderState.toothLastToothTime = 0xFFFFFF00UL;
  ctx.decoderState.triggerFilterTime = 500;
  ctx.current.startRevolutions = 1;

  ctx.triggerPrimary(0x00000100UL);

  TEST_ASSERT_EQUAL_UINT32(512, ctx.decoderState.curGap);
  TEST_ASSERT_TRUE(ctx.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT8(1, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(0x00000100UL, ctx.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_revolution_wrap_preserves_recent_sync(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.setup_primaryfilter(1000, 100);
  ctx.decoderState.toothCurrentCount = 4;
  ctx.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  ctx.decoderState.lastSyncRevolution = 1;
  ctx.current.startRevolutions = 4;

  ctx.triggerPrimary(calcNextTimePassPrimaryFilter(ctx.decoderState));

  TEST_ASSERT_EQUAL(SyncStatus::Full, ctx.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT8(1, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(5, ctx.current.startRevolutions);
  TEST_ASSERT_EQUAL_UINT16(0, ctx.current.syncLossCounter);
}

static void test_triggerPrimary_revolution_wrap_loses_stale_sync(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.setup_primaryfilter(1000, 100);
  ctx.decoderState.toothCurrentCount = 4;
  ctx.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  ctx.decoderState.lastSyncRevolution = 0;
  ctx.current.startRevolutions = 4;

  ctx.triggerPrimary(calcNextTimePassPrimaryFilter(ctx.decoderState));

  TEST_ASSERT_EQUAL(SyncStatus::None, ctx.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT8(1, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(5, ctx.current.startRevolutions);
  TEST_ASSERT_EQUAL_UINT16(1, ctx.current.syncLossCounter);
}

static void setup_perTooth(context_ford_tfi_t &context)
{
  context.init(8);
  setAngleConverterRevolutionTime(6000000UL);
  context.current.setRpm(3000);

  context.decoderState.toothCurrentCount = 2;
  context.decoderState.triggerToothAngle = 13;
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

static void assert_pertoothign(context_ford_tfi_t &context, uint16_t newCrankAngle)
{
  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
  TEST_ASSERT_UINT32_WITHIN(10, ignitionSchedule1._counter + angleToTimerTicks(ignitionSchedule1.chargeAngle - newCrankAngle), ignitionSchedule1._compare);
}

static void assert_pertoothign360(context_ford_tfi_t &context)
{
  auto newCrankAngle = context.page4.triggerAngle + ( (context.decoderState.toothCurrentCount) * context.decoderState.triggerToothAngle );
  assert_pertoothign(context, newCrankAngle);
}

static void test_triggerPrimary_applies_per_tooth_timing(void)
{
  context_ford_tfi_t context;

  ATOMIC() {
    stopIgnitionSchedulers();

    setup_perTooth(context);
    assert_pertoothign360(context);
  }
}

static void prepare_signature_context(context_ford_tfi_t &ctx, SyncStatus syncStatus,
                                      uint8_t toothCount, uint32_t startRevolutions)
{
  ctx.init(4);
  ctx.decoderState.curGap = 1000;
  ctx.decoderState.curGap3 = 800;
  ctx.decoderState.lastGap = 1000;
  ctx.decoderState.toothLastSecToothTime = BASE_2ND_TOOTH_TIME;
  ctx.decoderState.triggerSecFilterTime = 1000;
  ctx.decoderState.decoderStatus.syncStatus = syncStatus;
  ctx.decoderState.toothCurrentCount = toothCount;
  ctx.current.startRevolutions = startRevolutions;
}

static void test_triggerSecondary_rejects_filtered_pulse(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.decoderState.curGap = 1000;
  ctx.decoderState.curGap3 = 800;
  ctx.decoderState.lastGap = 1000;
  ctx.decoderState.toothLastSecToothTime = BASE_2ND_TOOTH_TIME;
  ctx.decoderState.triggerSecFilterTime = 1000;

  ctx.triggerSecondary(BASE_2ND_TOOTH_TIME + 999);

  TEST_ASSERT_EQUAL_UINT32(999, ctx.decoderState.curGap2);
  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, ctx.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(500, ctx.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL_UINT32(800, ctx.decoderState.curGap3);
  TEST_ASSERT_EQUAL_UINT32(1000, ctx.decoderState.lastGap);
}

static void test_triggerSecondary_initializes_first_pulse(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.decoderState.curGap = 1000;

  ctx.triggerSecondary(BASE_2ND_TOOTH_TIME);

  TEST_ASSERT_EQUAL_UINT32(0, ctx.decoderState.curGap2);
  TEST_ASSERT_EQUAL_UINT32(BASE_2ND_TOOTH_TIME, ctx.decoderState.toothLastSecToothTime);
  TEST_ASSERT_EQUAL_UINT32(500, ctx.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL(SyncStatus::None, ctx.decoderState.decoderStatus.syncStatus);
}

static void test_triggerSecondary_acquires_signature_sync(void)
{
  context_ford_tfi_t ctx;
  prepare_signature_context(ctx, SyncStatus::None, 3, 1);

  ctx.triggerSecondary(BASE_2ND_TOOTH_TIME + 1200);

  TEST_ASSERT_EQUAL(SyncStatus::Full, ctx.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT8(2, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(0, ctx.decoderState.triggerFilterTime);
  TEST_ASSERT_EQUAL_UINT32(1, ctx.decoderState.lastSyncRevolution);
  TEST_ASSERT_EQUAL_UINT32(500, ctx.decoderState.triggerSecFilterTime);
  TEST_ASSERT_EQUAL_UINT32(800, ctx.decoderState.lastGap);
  TEST_ASSERT_EQUAL_UINT32(1200, ctx.decoderState.curGap3);
}

static void test_triggerSecondary_reports_signature_mismatch_without_resync(void)
{
  context_ford_tfi_t ctx;
  prepare_signature_context(ctx, SyncStatus::Full, 4, 3);

  ctx.triggerSecondary(BASE_2ND_TOOTH_TIME + 1200);

  TEST_ASSERT_EQUAL(SyncStatus::Full, ctx.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT8(4, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT16(1, ctx.current.syncLossCounter);
  TEST_ASSERT_EQUAL_UINT32(3, ctx.decoderState.lastSyncRevolution);
}

static void test_triggerSecondary_resyncs_after_signature_mismatch(void)
{
  context_ford_tfi_t ctx;
  prepare_signature_context(ctx, SyncStatus::Full, 4, 3);
  ctx.page4.useResync = 1;

  ctx.triggerSecondary(BASE_2ND_TOOTH_TIME + 1200);

  TEST_ASSERT_EQUAL(SyncStatus::Full, ctx.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT8(2, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT16(1, ctx.current.syncLossCounter);
  TEST_ASSERT_EQUAL_UINT32(3, ctx.decoderState.lastSyncRevolution);
}

static void test_triggerSecondary_resyncs_during_staging(void)
{
  context_ford_tfi_t ctx;
  prepare_signature_context(ctx, SyncStatus::Full, 4, 3);
  ctx.page4.StgCycles = 3;

  ctx.triggerSecondary(BASE_2ND_TOOTH_TIME + 1200);

  TEST_ASSERT_EQUAL(SyncStatus::Full, ctx.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT8(2, ctx.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT16(0, ctx.current.syncLossCounter);
}

static void test_triggerSecondary_handles_timer_rollover(void)
{
  context_ford_tfi_t ctx;
  ctx.init(4);
  ctx.decoderState.toothLastSecToothTime = 0xFFFFFF00UL;
  ctx.decoderState.triggerFilterTime = 500;
  ctx.current.startRevolutions = 1;

  ctx.triggerSecondary(0x00000100UL);

  TEST_ASSERT_EQUAL_UINT32(512, ctx.decoderState.curGap2);
  TEST_ASSERT_EQUAL_UINT32(0x00000100UL, ctx.decoderState.toothLastSecToothTime );
}

static void assert_set_end_teeth(uint8_t cylinders, uint16_t dischargeAngle,
                                 const uint16_t (&expected)[4])
{
  context_ford_tfi_t ctx;
  ctx.init(cylinders);
  ctx.page4.triggerAngle = 0;
  ignitionSchedule1.dischargeAngle = dischargeAngle;

  ctx.setEndTeeth();

  for (uint8_t channel = 0; channel < 4; channel++)
  {
    TEST_ASSERT_EQUAL_UINT16(expected[channel], ctx.decoderState.ignitionEndTeeth[channel]);
  }
}

static void test_setEndTeeth_maps_cylinder_ranges(void)
{
  const uint16_t fourCylinderOutsideRange[4] = { 2, 1, 0, 0 };
  const uint16_t fourCylinderInsideRange[4] = { 1, 2, 0, 0 };
  assert_set_end_teeth(4, 0, fourCylinderOutsideRange);
  assert_set_end_teeth(4, 180, fourCylinderInsideRange);
  assert_set_end_teeth(4, 181, fourCylinderOutsideRange);

  const uint16_t sixCylinderFirstRange[4] = { 1, 2, 3, 0 };
  const uint16_t sixCylinderMiddleRange[4] = { 2, 3, 1, 0 };
  const uint16_t sixCylinderLastRange[4] = { 3, 1, 2, 0 };
  assert_set_end_teeth(6, 0, sixCylinderLastRange);
  assert_set_end_teeth(6, 120, sixCylinderFirstRange);
  assert_set_end_teeth(6, 121, sixCylinderMiddleRange);
  assert_set_end_teeth(6, 240, sixCylinderMiddleRange);
  assert_set_end_teeth(6, 241, sixCylinderLastRange);

  const uint16_t eightCylinderFirstRange[4] = { 1, 2, 3, 4 };
  const uint16_t eightCylinderSecondRange[4] = { 2, 3, 4, 1 };
  const uint16_t eightCylinderThirdRange[4] = { 3, 4, 1, 2 };
  const uint16_t eightCylinderLastRange[4] = { 4, 1, 2, 3 };
  assert_set_end_teeth(8, 0, eightCylinderLastRange);
  assert_set_end_teeth(8, 90, eightCylinderFirstRange);
  assert_set_end_teeth(8, 91, eightCylinderSecondRange);
  assert_set_end_teeth(8, 180, eightCylinderSecondRange);
  assert_set_end_teeth(8, 181, eightCylinderThirdRange);
  assert_set_end_teeth(8, 270, eightCylinderThirdRange);
  assert_set_end_teeth(8, 271, eightCylinderLastRange);
}

static void test_getCrankAngle(void)
{
  auto run_case = [&](uint8_t toothCount, uint8_t trigTeeth, int16_t delta, int16_t trigAngle, int16_t expected) {
    // trigTeeth is number of cylinders (ctx.page2.nCylinders)
    context_ford_tfi_t ctx;
    ctx.init(trigTeeth);
    ctx.decoderState.toothLastToothTime = 2000;
    ctx.decoderState.toothCurrentCount = toothCount;
    ctx.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    ctx.decoderState.decoderStatus.toothAngleIsCorrect = true;
    ctx.page4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, ctx.getCrankAngle(ctx.decoderState.toothLastToothTime + delta));
  };

  // timeToAngle(100) ~= 18 deg when revolution time = 2000
  constexpr int16_t dt = 18;

  // 4-cylinder tests (triggerToothAngle = 720/4 = 180)
  run_case(1, 4, 100, 0, 0 + dt);
  run_case(2, 4, 100, 0, 180 + dt);
  run_case(3, 4, 100, 0, 360 + dt);
  // ctx.decoderState.toothCurrentCount == 0 treated as 2
  run_case(0, 4, 100, 0, 180 + dt);

  // trigger offset
  run_case(2, 4, 100, 10, 180 + 10 + dt);

  // 6-cylinder tests (triggerToothAngle = 120)
  run_case(1, 6, 100, 0, 0 + dt);
  run_case(3, 6, 100, 0, 240 + dt);
  run_case(6, 6, 100, 0, 600 + dt);

  // 8-cylinder tests (triggerToothAngle = 90)
  run_case(1, 8, 100, 0, 0 + dt);
  run_case(4, 8, 100, 0, 270 + dt);
  run_case(8, 8, 100, 0, 630 + dt);

  // Negative delta
#if !defined(NATIVE_BOARD)
  run_case(8, 8, -100, 0, 630 - dt);
#endif
}

static void test_getRevolutionTime(void)
{
  context_ford_tfi_t ctx;
  ctx.init(8);

  // Prepare for cranking path: RPM < crankRPM
  ctx.page4.StgCycles = 0;
  ctx.current.startRevolutions = 1;
  ctx.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  ctx.current.setRpm(0);
  ctx.current.crankRPM = 400;
  ctx.current.revolutionTime = 99999UL;
  ctx.decoderState.toothLastMinusOneToothTime = 1000UL;
  ctx.decoderState.toothLastToothTime = ctx.decoderState.toothLastMinusOneToothTime + 30000UL; // gap=30000 -> revTime = gap*totalTeeth/2 = 30000*8/2 = 120000
  TEST_ASSERT_EQUAL_UINT32(120000UL, ctx.getRevolutionTime());

  // Running path: should return stdGetRevolutionTime(CAM_SPEED) -> ctx.current.revolutionTime when no toothOne* update
  ctx.current.setRpm(2000);
  ctx.decoderState.toothOneMinusOneTime = 0;
  ctx.decoderState.toothOneTime = 0;
  ctx.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_UINT32(99999UL, ctx.getRevolutionTime());

  // If not synced, should return ctx.current.revolutionTime
  ctx.decoderState.decoderStatus.syncStatus = SyncStatus::None;
  ctx.current.setRpm(555);
  TEST_ASSERT_EQUAL_UINT32(99999UL, ctx.getRevolutionTime());
}

void testFordTFI(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_triggerPrimary_rejects_filtered_pulse);
    RUN_TEST_P(test_triggerPrimary_startup_bypasses_filter);
    RUN_TEST_P(test_triggerPrimary_accepts_unsynced_tooth_and_disables_filter);
    RUN_TEST_P(test_triggerPrimary_recalculates_filter_when_synced);
    RUN_TEST_P(test_triggerPrimary_handles_timer_rollover);
    RUN_TEST_P(test_triggerPrimary_revolution_wrap_preserves_recent_sync);
    RUN_TEST_P(test_triggerPrimary_revolution_wrap_loses_stale_sync);
    RUN_TEST_P(test_triggerPrimary_applies_per_tooth_timing);
    RUN_TEST_P(test_triggerSecondary_rejects_filtered_pulse);
    RUN_TEST_P(test_triggerSecondary_initializes_first_pulse);
    RUN_TEST_P(test_triggerSecondary_acquires_signature_sync);
    RUN_TEST_P(test_triggerSecondary_reports_signature_mismatch_without_resync);
    RUN_TEST_P(test_triggerSecondary_resyncs_after_signature_mismatch);
    RUN_TEST_P(test_triggerSecondary_resyncs_during_staging);
    RUN_TEST_P(test_triggerSecondary_handles_timer_rollover)
    RUN_TEST_P(test_setEndTeeth_maps_cylinder_ranges);
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}