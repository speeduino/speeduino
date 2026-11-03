#include "../test_utils.h"
#include "src/decoders/4g63.h"

static void test_getCrankAngle(void)
{
    config2 page2={};
    config4 page4={};
    statuses current={};

  // helper to setup times and call
  auto call_angle = [&](auto &state, uint8_t toothNum, uint32_t deltaSinceLast, int trigAngle = 0){
    state.toothLastToothTime = 2000;
    state.toothLastMinusOneToothTime = state.toothLastToothTime - 500; // tooth interval = 500
    state.toothCurrentCount = toothNum;
    state.revolutionOne = false;
    state.decoderStatus.toothAngleIsCorrect = true;
    page4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    return decoders::fourg63::getCrankAngle(state.toothLastToothTime + deltaSinceLast, state, page4);
  };

  // Test 4-cylinder configuration
  page2.nCylinders = 4;
  auto decoder4 = decoders::fourg63::initialise(1000, current, page2);
  decoder4.decoderStatus.syncStatus = SyncStatus::Full;

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
  page2.nCylinders = 6;
  auto decoder6 = decoders::fourg63::initialise(1000, current, page2);
  decoder6.decoderStatus.syncStatus = SyncStatus::Full;
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
  page2.nCylinders = 4;
  auto decoder4b = decoders::fourg63::initialise(1000, current, page2);
  decoder4b.decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_INT16(41, call_angle(decoder4b, 1, dt, 10)); // 31 + 10

  // If not synced, should return 0
  decoder4b.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_INT16(0, decoders::fourg63::getCrankAngle(decoder4b.toothLastToothTime + dt, decoder4b, page4));
}

void test4G63CrankAngle(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_getCrankAngle);
}