#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "units.h"
#include "src/decoders/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{
  auto decoder = triggerSetup_Vmax();

  auto run_case = [&](int secTooth, int delta, int trigAngle, int16_t expected) {
    _decoderState.toothLastToothTime = 2000;
    _decoderState.secondaryToothCount = secTooth;
    _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + delta));
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
  auto decoder = triggerSetup_Vmax();

  // --- Cranking-style calculation when RPM below threshold: computes using last-tooth gap & angle
  currentStatus.setRpm(0);
  configPage4.crankRPM = 40; // 400 RPM
  currentStatus.crankRPM = RPM_MEDIUM.toUser(configPage4.crankRPM);
  currentStatus.startRevolutions = 0; // cranking
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.revolutionTime = 12345UL;
  // Set a non-zero triggerToothAngle so calculation yields non-zero result
  _decoderState.triggerToothAngle = 10;
  _decoderState.toothLastMinusOneToothTime = 1000UL;
  _decoderState.toothLastToothTime = _decoderState.toothLastMinusOneToothTime + 1667UL; // gap
  TEST_ASSERT_EQUAL_UINT32(1667UL*36UL, decoder.getRevolutionTime()); // 1667 * 360 / 10

  // --- No tooth angle yet (E.g. first tooth after sync): keep the published period
  _decoderState.triggerToothAngle = 0;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());
  _decoderState.triggerToothAngle = 10;

  // --- If tooth times missing, keep the published period
  _decoderState.toothLastMinusOneToothTime = 0;
  _decoderState.toothLastToothTime = 0;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoder.getRevolutionTime());

  // --- Running path: use stdGetRevolutionTime via toothOne pair
  currentStatus.setRpm(2000);
  configPage4.crankRPM = 10; // 100 RPM
  currentStatus.crankRPM = RPM_MEDIUM.toUser(configPage4.crankRPM);
  currentStatus.startRevolutions = 1; // not cranking
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL; // revTime = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // --- Sync lost: the speed is unknown
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoder.getRevolutionTime());
}

static void assert_getRevolutionTime_at_cranking_boundary(uint16_t rpm, uint32_t expected)
{
  auto decoder = triggerSetup_Vmax();
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

void testVMax(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
    RUN_TEST_P(test_getRevolutionTime_below_cranking_boundary);
    RUN_TEST_P(test_getRevolutionTime_at_cranking_boundary);
    RUN_TEST_P(test_getRevolutionTime_above_cranking_boundary);
  }
}