#include "../test_utils.h"
#include "src/decoders/jeep2000.h"

static void test_getCrankAngle(void)
{
  statuses current={};
  config4 page4={};
  auto decoderState = decoders::jeep2000::initialise(100, current);

  auto run_case = [&](uint8_t toothNum, int16_t expected, int trigAngle = 0) {
    decoderState.toothLastToothTime = 2000;
    decoderState.toothCurrentCount = toothNum;
    decoderState.decoderStatus.toothAngleIsCorrect = true;
    page4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoders::jeep2000::getCrankAngle(decoderState.toothLastToothTime + 100, decoderState, page4));
  };

  // timeToAngle(100) ~= 18 deg
  const int dt = 18;

  // toothAngles from triggerSetup_Jeep2000
  // indices 1..12 map to array values
  run_case(1, 174 + dt);
  run_case(2, 194 + dt);
  run_case(3, 214 + dt);
  run_case(4, 234 + dt);
  run_case(5, 294 + dt);
  run_case(6, 314 + dt);
  run_case(7, 334 + dt);
  run_case(8, 354 + dt);
  run_case(9, 414 + dt);
  run_case(10, 434 + dt);
  run_case(11, 454 + dt);
  run_case(12, 474 + dt);

  // cam tooth (0) special case -> 114 + dt
  run_case(0, 114 + dt);

  // trigger angle offset
  run_case(1, 174 + dt + 10, 10);
}

static void test_primary_waits_for_first_revolution(void)
{
  statuses current{};
  config4 page4{};
  auto decoderState = decoders::jeep2000::initialise(100, current);

  decoders::jeep2000::triggerPrimary(200, current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT16(13, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::None, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(100, decoderState.toothLastToothTime);
  TEST_ASSERT_EQUAL_UINT32(0, current.startRevolutions);
}

static void test_primary_rejects_filtered_pulse(void)
{
  statuses current{};
  config4 page4{};
  current.initialisationComplete = true;
  auto decoderState = decoders::jeep2000::initialise(0, current);
  decoderState.toothCurrentCount = 1;
  decoderState.toothLastToothTime = 1000;
  decoderState.triggerFilterTime = 100;

  decoders::jeep2000::triggerPrimary(1050, current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT32(50, decoderState.curGap);
  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_FALSE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(1000, decoderState.toothLastToothTime);
}

static void test_primary_synchronizes_on_tooth_one(void)
{
  statuses current{};
  config4 page4{};
  current.initialisationComplete = true;
  auto decoderState = decoders::jeep2000::initialise(0, current);
  decoderState.toothCurrentCount = 0;
  decoderState.toothLastToothTime = 1000;

  decoders::jeep2000::triggerPrimary(1200, current, decoderState, page4);

  TEST_ASSERT_EQUAL_UINT16(1, decoderState.toothCurrentCount);
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoderState.decoderStatus.syncStatus);
  TEST_ASSERT_TRUE(decoderState.decoderStatus.validTrigger);
  TEST_ASSERT_EQUAL_UINT32(1, current.startRevolutions);
  TEST_ASSERT_EQUAL_UINT16(60, decoderState.triggerToothAngle);
  TEST_ASSERT_EQUAL_UINT32(1200, decoderState.toothOneTime);
  TEST_ASSERT_EQUAL_UINT32(1200, decoderState.toothLastToothTime);
}

static void test_primary_uses_variable_tooth_angles(void)
{
  statuses current{};
  config4 page4{};
  current.initialisationComplete = true;
  auto decoderState = decoders::jeep2000::initialise(0, current);
  decoderState.toothCurrentCount = 1;
  decoderState.toothLastToothTime = 1000;
  decoderState.triggerFilterTime = 0;
  const uint16_t expectedAngles[] = {20, 20, 20, 60, 20};

  for(size_t index = 0; index < sizeof(expectedAngles) / sizeof(expectedAngles[0]); ++index)
  {
    const uint32_t triggerTime = decoderState.toothLastToothTime + 100;
    decoders::jeep2000::triggerPrimary(triggerTime, current, decoderState, page4);

    TEST_ASSERT_EQUAL_UINT16(index + 2, decoderState.toothCurrentCount);
    TEST_ASSERT_EQUAL_UINT16(expectedAngles[index], decoderState.triggerToothAngle);
    TEST_ASSERT_EQUAL_UINT32(triggerTime, decoderState.toothLastToothTime);
  }
}

static void test_secondary(void)
{
  statuses current{};
  auto decoderState = decoders::jeep2000::initialise(0, current);

  decoderState.toothCurrentCount = 99;
  decoders::jeep2000::triggerSecondary(decoderState);;

  TEST_ASSERT_EQUAL(0, decoderState.toothCurrentCount);
}

static void test_initialise(void)
{
    {
        statuses current{};
        current.initialisationComplete = true;
        auto subject = decoders::jeep2000::initialise(5000, current);

        TEST_ASSERT_EQUAL(0, subject.toothCurrentCount);
        TEST_ASSERT_EQUAL(0, subject.toothLastToothTime);
    }

    {
        statuses current{};
        current.initialisationComplete = false;
        auto subject = decoders::jeep2000::initialise(5000, current);

        TEST_ASSERT_EQUAL(13, subject.toothCurrentCount);
        TEST_ASSERT_EQUAL(5000, subject.toothLastToothTime);
    }
}

void testJeep2000(void)
{
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_getCrankAngle);
  RUN_TEST_P(test_primary_waits_for_first_revolution);
  RUN_TEST_P(test_primary_rejects_filtered_pulse);
  RUN_TEST_P(test_primary_synchronizes_on_tooth_one);
  RUN_TEST_P(test_primary_uses_variable_tooth_angles);
  RUN_TEST_P(test_secondary);
  RUN_TEST_P(test_initialise);
}