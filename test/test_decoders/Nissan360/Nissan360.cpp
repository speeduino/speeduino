#include "../test_utils.h"
#include "src/decoders/nissan360.h"
#include "scheduler_ignition_controller.h"

static void assert_setEndTeeth(uint16_t expected, decoders::shared::state_t &decoderState, IgnitionSchedule &schedule, uint8_t index, int8_t advance, const config4 &page4)
{
    schedule.dischargeAngle = 360 + advance; 
    decoders::nissan360::setEndTeeth(decoderState, page4);
    TEST_ASSERT_EQUAL(expected, decoderState.ignitionEndTeeth[index]);
}

void test_setEndTeeth_channel1()
{
  config4 page4={};
  auto decoderState = decoders::nissan360::initialise();
  page4.sparkMode = IGN_MODE_WASTED;

  page4.triggerAngle = 0; //No trigger offset
  assert_setEndTeeth(171, decoderState, ignitionSchedule1, 0, -10, page4);
  assert_setEndTeeth(176, decoderState, ignitionSchedule1, 0, 0, page4);
  assert_setEndTeeth(158, decoderState, ignitionSchedule1, 0, -35, page4);

  page4.triggerAngle = 90;
  assert_setEndTeeth(126, decoderState, ignitionSchedule1, 0, -10, page4);

  page4.triggerAngle = 180;
  assert_setEndTeeth(81, decoderState, ignitionSchedule1, 0, -10, page4);

  page4.triggerAngle = 270;
  assert_setEndTeeth(36, decoderState, ignitionSchedule1, 0, -10, page4);

  page4.triggerAngle = 360;
  assert_setEndTeeth(351, decoderState, ignitionSchedule1, 0, -10, page4);

  page4.triggerAngle = -90;
  assert_setEndTeeth(216, decoderState, ignitionSchedule1, 0, -10, page4);

  page4.triggerAngle = -180;
  assert_setEndTeeth(261, decoderState, ignitionSchedule1, 0, -10, page4);

  page4.triggerAngle = -270;
  assert_setEndTeeth(306, decoderState, ignitionSchedule1, 0, -10, page4);

  page4.triggerAngle = -360;
  assert_setEndTeeth(351, decoderState, ignitionSchedule1, 0, -10, page4);
}

static void test_getCrankAngle(void)
{
  config4 page4={};
  auto decoderState = decoders::nissan360::initialise();

  auto run_case = [&](int toothNum, unsigned long elapsedDelta, int trigAngle, int16_t expected) {
      // Set deterministic tooth times so halfTooth is known
      decoderState.toothLastMinusOneToothTime = 1000;
      decoderState.toothLastToothTime = 1500; // halfTooth = 250
      decoderState.toothCurrentCount = toothNum;
      decoderState.decoderStatus.syncStatus = SyncStatus::Full;
      page4.triggerAngle = trigAngle;
      CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 360;
      setAngleConverterRevolutionTime(2000);
      TEST_ASSERT_EQUAL(expected, decoders::nissan360::getCrankAngle(decoderState.toothLastToothTime + elapsedDelta, decoderState, page4));
  };

  // halfTooth = (1500-1000)/2 = 250
  // Cases: elapsed <=250 -> no extra degree; elapsed >250 -> +1 degree
  run_case(1, 100, 0, 0);       // tooth 1, before half -> 0 deg
  run_case(1, 300, 0, 1);       // tooth 1, after half -> 1 deg
  run_case(2, 100, 0, 2);       // tooth 2 -> 2 deg
  run_case(36, 100, 90, 160);   // tooth 36: (35*2)=70 + 90 = 160
  run_case(180, 300, 0, 359);   // tooth 180: (179*2)=358 + 1 => 359 deg
  // Negative triggerAngle should wrap into positive range
  run_case(1, 100, -90, 270);   // -90 -> +360 => 270
}

static void test_getRevolutionTime(void)
{
  statuses current={};
  auto decoderState = decoders::nissan360::initialise();

  // --- Cranking path: startRevolutions < 2 -> revTime = gap * 180
  current.setRpm(0);
  current.crankRPM = 400;
  current.revolutionTime = 12345UL;
  current.startRevolutions = 0; // cranking
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothLastMinusOneToothTime = 1000UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 333UL; // gap = 333 -> revTime = 333*180
  TEST_ASSERT_EQUAL_UINT32(333UL*180UL, decoders::nissan360::getRevolutionTime(current, decoderState));

  current.revolutionTime = 12345UL;
  decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoders::nissan360::getRevolutionTime(current, decoderState));

  // --- Running path: use the toothOne pair and >>1 scaling
  current.setRpm(2000);
  current.startRevolutions = 2; // not cranking
  decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  decoderState.toothOneMinusOneTime = 1000UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 120000UL; // >>1 => 60000 uS revTime
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoders::nissan360::getRevolutionTime(current, decoderState));
  TEST_ASSERT_EQUAL_UINT32(120000UL, decoderState.MAX_STALL_TIME);

  current.revolutionTime = 12345UL;
  decoderState.toothOneTime = decoderState.toothOneMinusOneTime;
  TEST_ASSERT_EQUAL_UINT32(12345UL, decoders::nissan360::getRevolutionTime(current, decoderState));

  // --- Fallback: when sync is lost, the speed is unknown. The stall time is kept
  decoderState.decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT32(0UL, decoders::nissan360::getRevolutionTime(current, decoderState));
  TEST_ASSERT_EQUAL_UINT32(120000UL, decoderState.MAX_STALL_TIME);
}

void testNissan360()
{
  extern void testNissan360Triggers(void);

  testNissan360Triggers();

  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_setEndTeeth_channel1);
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRevolutionTime);
  }
}