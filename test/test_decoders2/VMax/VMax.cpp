#include "../../test_utils.h"
#include "src/decoders/vmax.h"
#include "units.h"
#include "../../test_decoders/test_decoder_shared.h"

constexpr uint32_t BASE_1ST_TOOTH_TIME = 100000UL;

struct context_vmax_t
{
  statuses current = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(void)
  {
    decoderState = decoders::vmax::initialise(1000, current);
    current.decoder.primary._pin.setPin(2);
    current.decoder.primary.edge = CHANGE;
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::vmax::triggerPrimary(curTime, current, decoderState, page4);
  }

  void setup_primaryfilter(uint32_t lastGap, uint32_t filter)
  {
    decoderState.toothLastToothTime = BASE_1ST_TOOTH_TIME;
    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime - lastGap;
    decoderState.triggerFilterTime = filter;
  }

  uint32_t getRevolutionTime()
  {
    return decoders::vmax::getRevolutionTime(current, decoderState);
  }

  auto getCrankAngle(uint32_t currMicros)
  {
    return decoders::vmax::getCrankAngle(currMicros, decoderState, page4);
  }
};

static void test_getCrankAngle(void)
{
  context_vmax_t context;
  context.init();

  auto run_case = [&](int secTooth, int delta, int trigAngle, int16_t expected) {
    context.decoderState.toothLastToothTime = 2000;
    context.decoderState.secondaryToothCount = secTooth;
    context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    context.decoderState.decoderStatus.toothAngleIsCorrect = true;
    context.page4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, context.getCrankAngle(context.decoderState.toothLastToothTime + delta));
  };

  // timeToAngle(100) ~= 18 deg
  const int dt = 18;

  run_case(1, 100, 0, 0 + dt);
  run_case(2, 100, 0, 40 + dt);
  run_case(3, 100, 0, 110 + dt);
  run_case(4, 100, 0, 180 + dt);
  run_case(5, 100, 0, 220 + dt);
  run_case(6, 100, 0, 290 + dt);

  // trigger angle offset
  run_case(3, 100, 10, 110 + dt + 10);
}

static void test_getRevolutionTime(void)
{
  context_vmax_t context;
  context.init();

  // --- Cranking-style calculation when RPM below threshold: computes using last-tooth gap & angle
  context.current.setRpm(0);
  context.page4.crankRPM = 40; // 400 RPM
  context.current.crankRPM = RPM_MEDIUM.toUser(context.page4.crankRPM);
  context.current.startRevolutions = 0; // cranking
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.current.revolutionTime = 12345UL;
  // Set a non-zero triggerToothAngle so calculation yields non-zero result
  context.decoderState.triggerToothAngle = 10;
  context.decoderState.toothLastMinusOneToothTime = 1000UL;
  context.decoderState.toothLastToothTime = context.decoderState.toothLastMinusOneToothTime + 1667UL; // gap
  TEST_ASSERT_EQUAL_UINT32(1667UL*36UL, context.getRevolutionTime()); // 1667 * 360 / 10

  // --- No tooth angle yet (E.g. first tooth after sync): keep the published period
  context.decoderState.triggerToothAngle = 0;
  TEST_ASSERT_EQUAL_UINT32(12345UL, context.getRevolutionTime());
  context.decoderState.triggerToothAngle = 10;

  // --- If tooth times missing, keep the published period
  context.decoderState.toothLastMinusOneToothTime = 0;
  context.decoderState.toothLastToothTime = 0;
  TEST_ASSERT_EQUAL_UINT32(12345UL, context.getRevolutionTime());

  // --- Running path: use stdGetRevolutionTime via toothOne pair
  context.current.setRpm(2000);
  context.page4.crankRPM = 10; // 100 RPM
  context.current.crankRPM = RPM_MEDIUM.toUser(context.page4.crankRPM);
  context.current.startRevolutions = 1; // not cranking
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  context.decoderState.toothOneMinusOneTime = 1000UL;
  context.decoderState.toothOneTime = context.decoderState.toothOneMinusOneTime + 60000UL; // revTime = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, context.getRevolutionTime());

  // --- Sync lost: the speed is unknown
  context.decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, context.getRevolutionTime());
}

static void assert_getRevolutionTime_at_cranking_boundary(uint16_t rpm, uint32_t expected)
{
  context_vmax_t context;
  context.init();

  context.page4.crankRPM = 40; // Stored in RPM/10, so the boundary is 400 RPM
  context.current.crankRPM = RPM_MEDIUM.toUser(context.page4.crankRPM);
  context.current.setRpm(rpm);
  context.current.startRevolutions = 1;
  context.current.revolutionTime = UINT32_MAX;
  context.decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // Deliberately different results distinguish which calculation was selected:
  // last-tooth angle/gap -> 60000uS (1000 RPM); complete revolution -> 40000uS (1500 RPM).
  context.decoderState.triggerToothAngle = 120;
  context.decoderState.toothLastMinusOneToothTime = 1000UL;
  context.decoderState.toothLastToothTime = 21000UL;
  context.decoderState.toothOneMinusOneTime = 1000UL;
  context.decoderState.toothOneTime = 41000UL;
  TEST_ASSERT_EQUAL_UINT32(expected, context.getRevolutionTime());
}

static void test_getRevolutionTime_below_cranking_boundary(void)
{
  assert_getRevolutionTime_at_cranking_boundary(399, 60000UL);
}

static void test_getRevolutionTime_at_cranking_boundary(void)
{
  assert_getRevolutionTime_at_cranking_boundary(400, 40000UL);
}

static void test_getRevolutionTime_above_cranking_boundary(void)
{
  assert_getRevolutionTime_at_cranking_boundary(401, 40000UL);
}

static void test_triggerPrimary_accepts_valid_tooth(uint16_t toothCurrentCount, 
  uint16_t expectedTriggerToothAngle,
  uint16_t expectedSecondaryToothCount,
  uint16_t nextToothAngle)
{
  context_vmax_t context;
  context.page4.triggerFilter = TRIGGER_FILTER_LITE;
  context.init();
  constexpr uint32_t GAP = 1000;
  context.current.decoder.primary._pin._pin.setPinHigh();
  context.setup_primaryfilter(GAP, 100);
  context.decoderState.toothCurrentCount = toothCurrentCount;

  auto nextTime = calcNextTimePassPrimaryFilter(context.decoderState);
  context.triggerPrimary(nextTime);

  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT16(expectedSecondaryToothCount, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT16(expectedTriggerToothAngle, context.decoderState.triggerToothAngle);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastMinusOneToothTime);
  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothLastToothTime);
  uint32_t mult = nextToothAngle/10;
  uint32_t div  = expectedTriggerToothAngle/10;
  auto nextGap = ((nextTime-BASE_1ST_TOOTH_TIME)*mult)/div;
  auto nextFilter = nextGap/4;
  TEST_ASSERT_EQUAL_UINT32(nextFilter, context.decoderState.triggerFilterTime);
}

static void test_triggerPrimary_accepts_valid_tooth(void)
{
  test_triggerPrimary_accepts_valid_tooth(1, 70, 1, 40);
  test_triggerPrimary_accepts_valid_tooth(2, 40, 2, 70);
  test_triggerPrimary_accepts_valid_tooth(3, 70, 3, 70);
  test_triggerPrimary_accepts_valid_tooth(4, 70, 4, 40);
  test_triggerPrimary_accepts_valid_tooth(5, 40, 5, 70);
  test_triggerPrimary_accepts_valid_tooth(6, 70, 6, 70);
}

static void test_triggerPrimary_first_tooth_sets_sync(void)
{
  context_vmax_t context;
  context.init();
  context.current.decoder.primary._pin._pin.setPinHigh();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.toothCurrentCount = 1;

  auto nextTime = calcNextTimePassPrimaryFilter(context.decoderState);
  context.triggerPrimary(nextTime);

  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.secondaryToothCount);
  TEST_ASSERT_EQUAL_UINT16(70, context.decoderState.triggerToothAngle);
  TEST_ASSERT_EQUAL_UINT32(nextTime, context.decoderState.toothOneTime);
  TEST_ASSERT_EQUAL_UINT32(1, context.current.startRevolutions);
}

static void test_triggerPrimary_rejects_filtered_tooth(void)
{
  context_vmax_t context;
  context.init();
  context.current.decoder.primary._pin._pin.setPinHigh();
  context.setup_primaryfilter(1000, 1500);
  context.decoderState.toothCurrentCount = 3;

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState)-2);

  TEST_ASSERT_FALSE(context.decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL_UINT16(0, context.decoderState.secondaryToothCount);
}

static void test_triggerPrimary_without_tooth_sync_clears_filter(void)
{
  context_vmax_t context;
  context.init();
  context.current.decoder.primary._pin._pin.setPinHigh();
  context.setup_primaryfilter(1000, 100);

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));

  TEST_ASSERT_EQUAL_UINT32(0, context.decoderState.triggerFilterTime);
  TEST_ASSERT_EQUAL_UINT32(BASE_1ST_TOOTH_TIME, context.decoderState.toothLastToothTime);
}

static void test_triggerPrimary_wide_lobe_acquires_sync(void)
{
  context_vmax_t context;
  context.init();
  context.current.decoder.primary._pin._pin.setPinLow();
  context.decoderState.decoderStatus.validTrigger = true;
  context.decoderState.curGap2 = 10000;
  context.decoderState.lastGap = 500;
  context.decoderState.toothCurrentCount = 0;

  context.triggerPrimary(11001);

  TEST_ASSERT_EQUAL(SyncStatus::Full, context.decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(1001, context.decoderState.lastGap);
}

static void test_triggerPrimary_fallinglobe_syncerror(void)
{
  context_vmax_t context;
  context.init();
  context.current.decoder.primary._pin._pin.setPinLow();
  context.decoderState.decoderStatus.validTrigger = true;
  context.decoderState.curGap2 = 10000;
  context.decoderState.lastGap = 500;
  context.decoderState.toothCurrentCount = 6;

  context.triggerPrimary(11000);

  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT32(1, context.current.syncLossCounter);
}

static void test_triggerPrimary_fallinglobe_small(void)
{
  context_vmax_t context;
  context.init();
  context.current.decoder.primary._pin._pin.setPinLow();
  context.decoderState.decoderStatus.validTrigger = true;
  context.decoderState.curGap2 = 10000;
  context.decoderState.lastGap = 500;
  context.decoderState.toothCurrentCount = 5;

  context.triggerPrimary(11000);

  TEST_ASSERT_EQUAL_UINT16(6, context.decoderState.toothCurrentCount);
}

static void test_triggerPrimary_wide_lobe_out_of_position_counts_sync_error(void)
{
  context_vmax_t context;
  context.init();
  context.current.decoder.primary._pin._pin.setPinLow();
  context.decoderState.decoderStatus.validTrigger = true;
  context.decoderState.curGap2 = 10000;
  context.decoderState.lastGap = 500;
  context.decoderState.toothCurrentCount = 3;

  context.triggerPrimary(11001);

  TEST_ASSERT_EQUAL_UINT16(1, context.decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL_UINT16(1, context.current.syncLossCounter);
}

static void test_triggerPrimary_validTrigger_reset(void)
{
  context_vmax_t context;
  context.init();
  context.current.decoder.primary._pin._pin.setPinLow();
  context.setup_primaryfilter(1000, 100);
  context.decoderState.decoderStatus.validTrigger = false;

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
 
  TEST_ASSERT_TRUE(context.decoderState.decoderStatus.validTrigger);
}

static void test_triggerPrimary_firstpulse_resetfilter(void)
{
  context_vmax_t context;
  context.page4.triggerFilter = TRIGGER_FILTER_LITE;
  context.init();
  context.current.decoder.primary._pin._pin.setPinHigh();
  context.setup_primaryfilter(1000, 50001);
  context.decoderState.toothCurrentCount = 7;

  context.triggerPrimary(calcNextTimePassPrimaryFilter(context.decoderState));
 
  TEST_ASSERT_EQUAL(0, context.decoderState.triggerFilterTime );
}

void testVMax(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
    RUN_TEST_P(test_getRevolutionTime_below_cranking_boundary);
    RUN_TEST_P(test_getRevolutionTime_at_cranking_boundary);
    RUN_TEST_P(test_getRevolutionTime_above_cranking_boundary);

    RUN_TEST(test_triggerPrimary_accepts_valid_tooth);
    RUN_TEST(test_triggerPrimary_first_tooth_sets_sync);
    RUN_TEST(test_triggerPrimary_rejects_filtered_tooth);
    RUN_TEST(test_triggerPrimary_without_tooth_sync_clears_filter);
    RUN_TEST(test_triggerPrimary_wide_lobe_acquires_sync);
    RUN_TEST(test_triggerPrimary_wide_lobe_out_of_position_counts_sync_error);
    RUN_TEST(test_triggerPrimary_validTrigger_reset);
    RUN_TEST(test_triggerPrimary_firstpulse_resetfilter);
    RUN_TEST(test_triggerPrimary_fallinglobe_syncerror);
    RUN_TEST(test_triggerPrimary_fallinglobe_small);
  }
}