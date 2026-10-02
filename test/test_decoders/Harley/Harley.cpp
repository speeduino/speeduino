#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "units.h"
#include "src/decoders/details/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{
  auto decoder = triggerSetup_Harley();

  // Make time->angle deterministic for tests
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  setAngleConverterRevolutionTime(2000);
    
  // Base case: tooth 1 should map to 0 + triggerAngle
  configPage4.triggerAngle = 0;
  _decoderState.toothLastToothTime = 10000;
  _decoderState.toothCurrentCount = 1;
  {
    int16_t angle = decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100);
    int16_t expected = 0 + timeToAngle(100);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Tooth 2 should map to 157 + triggerAngle
  _decoderState.toothCurrentCount = 2;
  {
    int16_t angle = decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100);
    int16_t expected = 157 + timeToAngle(100);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Tooth 3 behaves like tooth 1 (reference tooth)
  _decoderState.toothCurrentCount = 3;
  {
    int16_t angle = decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100);
    int16_t expected = 0 + timeToAngle(100);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Tooth 4 with a triggerAngle offset
  configPage4.triggerAngle = 10;
  _decoderState.toothCurrentCount = 4;
  {
    int16_t angle = decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 50);
    int16_t expected = 157 + 10 + timeToAngle(50);
    TEST_ASSERT_EQUAL(expected, angle);
  }

  // Wrap-around behaviour: large triggerAngle should reduce result by 720 when >=720
  configPage4.triggerAngle = 600;
  _decoderState.toothCurrentCount = 2;
  {
    int16_t angle = decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 10);
    int16_t expected = 157 + 600 + timeToAngle(10);
    if (expected >= 720) expected -= 720;
    TEST_ASSERT_EQUAL(expected, angle);
  }
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_Harley();

  // Ensure sync present
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // --- Cranking calculation below the configured RPM/10 threshold
  currentStatus.setRpm(0);
  configPage4.crankRPM = 100; // 1000 RPM
  currentStatus.crankRPM = RPM_MEDIUM.toUser(configPage4.crankRPM);
  currentStatus.revolutionTime = 12345UL;
  // Choose tempToothAngle = 120 and gap such that revTime = 20000 * 360 / 120 = 60000
  _decoderState.triggerToothAngle = 120;
  _decoderState.toothLastMinusOneToothTime = 1000UL;
  _decoderState.toothLastToothTime = _decoderState.toothLastMinusOneToothTime + 20000UL;
  _decoderState.toothOneMinusOneTime = 0UL;
  _decoderState.toothOneTime = 0UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // --- Tooth #1 has a zero tooth angle: keep the published period.
  // The tooth #1 times aren't used: they can span a stall (E.g. the first tooth after a restart)
  _decoderState.triggerToothAngle = 0;
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 5000000UL;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());

  // --- Running path: should call stdGetRevolutionTime(CRANK_SPEED)
  currentStatus.setRpm(2000);
  configPage4.crankRPM = 10; // 100 RPM
  currentStatus.crankRPM = RPM_MEDIUM.toUser(configPage4.crankRPM);
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL; // revTime=60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // --- Fallback: when not synced, the speed is unknown
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoder.getRevolutionTime());
}

static void assert_getRevolutionTime_at_cranking_boundary(uint16_t rpm, uint32_t expected)
{
  auto decoder = triggerSetup_Harley();
  configPage4.crankRPM = 40; // Stored in RPM/10, so the boundary is 400 RPM
  currentStatus.crankRPM = RPM_MEDIUM.toUser(configPage4.crankRPM);
  currentStatus.setRpm(rpm);
  currentStatus.startRevolutions = 1;
  currentStatus.revolutionTime = UINT32_MAX;
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // Deliberately different results distinguish which calculation was selected:
  // last-tooth angle/gap -> 60000uS (1000 RPM); complete revolution -> 40000uS (1500 RPM).
  _decoderState.triggerToothAngle = 120;
  _decoderState.toothLastMinusOneToothTime = 1000UL;
  _decoderState.toothLastToothTime = 21000UL;
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = 41000UL;
  TEST_ASSERT_EQUAL_UINT32(expected, decoder.getRevolutionTime());
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

void testHarley(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
    RUN_TEST_P(test_getRevolutionTime_below_cranking_boundary);
    RUN_TEST_P(test_getRevolutionTime_at_cranking_boundary);
    RUN_TEST_P(test_getRevolutionTime_above_cranking_boundary);
  }
}