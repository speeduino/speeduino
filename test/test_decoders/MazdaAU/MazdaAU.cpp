#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "crankMaths.h"
#include "src/decoders/details/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{
  auto decoder = triggerSetup_MazdaAU();

  auto setup_case = [&](int toothNum, int trigAngle) {
    _decoderState.toothLastToothTime = 2000;
    _decoderState.toothCurrentCount = toothNum;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
  };

  auto run_case = [&](int toothNum, int16_t expected, int trigAngle = 0) {
    setup_case(toothNum, trigAngle);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100));
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
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL(0, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100));
  setup_case(1, 0);
  _decoderState.decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL(0, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + 100));
}

static void test_getRevolutionTime(void)
{
  extern volatile uint16_t triggerToothAngle;

  auto decoder = triggerSetup_MazdaAU();

  // Ensure sync present
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // --- Cranking branch: currentStatus.RPM < currentStatus.crankRPM
  // Revolution time is calculated from the last tooth gap, whichever of the uneven gaps that is.
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 200;
  currentStatus.revolutionTime = 12345UL;
  // 108 degree gap of 180000uS -> revTime = 180000 * 360 / 108 = 600000 (100 RPM)
  _decoderState.triggerToothAngle = 108;
  _decoderState.toothLastMinusOneToothTime = 1000UL;
  _decoderState.toothLastToothTime = _decoderState.toothLastMinusOneToothTime + 180000UL;
  TEST_ASSERT_EQUAL_UINT32(600000UL, decoder.getRevolutionTime());
  // 72 degree gap of 120000uS -> revTime = 120000 * 360 / 72 = 600000 (100 RPM)
  _decoderState.triggerToothAngle = 72;
  _decoderState.toothLastToothTime = _decoderState.toothLastMinusOneToothTime + 120000UL;
  TEST_ASSERT_EQUAL_UINT32(600000UL, decoder.getRevolutionTime());

  // --- Running path: uses stdGetRevolutionTime(CRANK_SPEED)
  currentStatus.setRpm(2000);
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 60000UL; // revTime = 60000
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // --- Fallback: when not full sync, the speed is unknown
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoder.getRevolutionTime());
}

void testMazdaAU(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}