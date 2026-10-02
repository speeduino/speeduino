#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "crankMaths.h"
#include "src/decoders/details/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{
  // helper to setup times and call
  auto call_angle = [&](auto &decoder, uint8_t toothNum, uint32_t deltaSinceLast, int trigAngle = 0){
    _decoderState.toothLastToothTime = 2000;
    _decoderState.toothLastMinusOneToothTime = _decoderState.toothLastToothTime - 500; // tooth interval = 500
    _decoderState.toothCurrentCount = toothNum;
    _decoderState.revolutionOne = false;
    _decoderState.decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    return decoder.pGetCrankAngle(_decoderState.toothLastToothTime + deltaSinceLast);
  };

  // Test 4-cylinder configuration
  configPage2.nCylinders = 4;
  auto decoder4 = triggerSetup_4G63();
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // timeToAngleIntervalTooth: delta=100 -> toothAngle*100 / toothTime = 180*100/500 = 36
  const int dt = 100;
  TEST_ASSERT_EQUAL_INT16(31, call_angle(decoder4, 1, dt)); // 715 +36 =751 -> 31
  TEST_ASSERT_EQUAL_INT16(141, call_angle(decoder4, 2, dt));
  TEST_ASSERT_EQUAL_INT16(211, call_angle(decoder4, 3, dt));
  TEST_ASSERT_EQUAL_INT16(321, call_angle(decoder4, 4, dt));
  TEST_ASSERT_EQUAL_INT16(391, call_angle(decoder4, 5, dt));
  TEST_ASSERT_EQUAL_INT16(501, call_angle(decoder4, 6, dt));
  TEST_ASSERT_EQUAL_INT16(571, call_angle(decoder4, 7, dt));
  TEST_ASSERT_EQUAL_INT16(681, call_angle(decoder4, 8, dt));

  // 6-cylinder configuration
  configPage2.nCylinders = 6;
  auto decoder6 = triggerSetup_4G63();
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_INT16(31, call_angle(decoder6, 1, dt));
  TEST_ASSERT_EQUAL_INT16(81, call_angle(decoder6, 2, dt));
  TEST_ASSERT_EQUAL_INT16(151, call_angle(decoder6, 3, dt));
  TEST_ASSERT_EQUAL_INT16(201, call_angle(decoder6, 4, dt));
  TEST_ASSERT_EQUAL_INT16(271, call_angle(decoder6, 5, dt));
  TEST_ASSERT_EQUAL_INT16(321, call_angle(decoder6, 6, dt));
  TEST_ASSERT_EQUAL_INT16(391, call_angle(decoder6, 7, dt));
  TEST_ASSERT_EQUAL_INT16(441, call_angle(decoder6, 8, dt));
  TEST_ASSERT_EQUAL_INT16(511, call_angle(decoder6, 9, dt));
  TEST_ASSERT_EQUAL_INT16(561, call_angle(decoder6, 10, dt));
  TEST_ASSERT_EQUAL_INT16(631, call_angle(decoder6, 11, dt));
  TEST_ASSERT_EQUAL_INT16(681, call_angle(decoder6, 12, dt));

  // trigger angle offset modifies result
  configPage2.nCylinders = 4;
  auto decoder4b = triggerSetup_4G63();
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_INT16(41, call_angle(decoder4b, 1, dt, 10)); // 31 + 10

  // If not synced, should return 0
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_INT16(0, decoder4b.pGetCrankAngle(_decoderState.toothLastToothTime + dt));
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_4G63();

  // Cranking path: currentStatus.RPM < currentStatus.crankRPM
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 400;
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;

  // Simulate two tooth times 500us apart -> delta = 500
  _decoderState.toothLastToothTime = 2000;
  _decoderState.toothLastMinusOneToothTime = 1500;

  // For a 70 degree tooth gap (one of the 4G63 cranking gaps)
  _decoderState.triggerToothAngle = 70;
  TEST_ASSERT_EQUAL_UINT32(2571UL, decoder.getRevolutionTime()); // 500 * 360 / 70 = 2571

  // For a 110 degree tooth gap
  _decoderState.triggerToothAngle = 110;
  TEST_ASSERT_EQUAL_UINT32(1636UL, decoder.getRevolutionTime()); // 500 * 360 / 110 = 1636

  // Running path: should return stdGetRevolutionTime(CAM_SPEED) -> defaults to currentStatus.revolutionTime when no revolution update
  currentStatus.setRpm(1000);
  currentStatus.revolutionTime = 60000UL;
  // Clear toothOne* so stdGetRevolutionTime can't calculate the revolution time
  _decoderState.toothOneTime = 0;
  _decoderState.toothOneMinusOneTime = 0;
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());

  // No sync: the speed is unknown
  _decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoder.getRevolutionTime());
}

void test4G63(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}