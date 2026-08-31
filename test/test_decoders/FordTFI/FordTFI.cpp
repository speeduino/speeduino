#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

extern decoder_status_t decoderStatus;
extern volatile uint32_t toothLastToothTime;
extern volatile int toothCurrentCount;
extern volatile unsigned long toothLastMinusOneToothTime;
extern volatile unsigned long toothOneTime;
extern volatile unsigned long toothOneMinusOneTime;

static void test_getCrankAngle(void)
{
  auto run_case = [&](decoder_t &decoder, uint8_t toothCount, uint8_t trigTeeth, int16_t delta, int16_t trigAngle, int16_t expected) {
    // trigTeeth is number of cylinders (configPage2.nCylinders)
    configPage2.nCylinders = trigTeeth;
    toothLastToothTime = 2000;
    toothCurrentCount = toothCount;
    decoderStatus.syncStatus = SyncStatus::Full;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(toothLastToothTime + delta));
  };

  // timeToAngle(100) ~= 18 deg when revolution time = 2000
  constexpr int16_t dt = 18;

  // 4-cylinder tests (triggerToothAngle = 720/4 = 180)
  configPage2.nCylinders = 4;
  decoder_t dec4 = triggerSetup_FordTFI();
  run_case(dec4, 1, 4, 100, 0, 0 + dt);
  run_case(dec4, 2, 4, 100, 0, 180 + dt);
  run_case(dec4, 3, 4, 100, 0, 360 + dt);
  // toothCurrentCount == 0 treated as 2
  run_case(dec4, 0, 4, 100, 0, 180 + dt);

  // trigger offset
  run_case(dec4, 2, 4, 100, 10, 180 + 10 + dt);

  // 6-cylinder tests (triggerToothAngle = 120)
  configPage2.nCylinders = 6;
  decoder_t dec6 = triggerSetup_FordTFI();
  run_case(dec6, 1, 6, 100, 0, 0 + dt);
  run_case(dec6, 3, 6, 100, 0, 240 + dt);
  run_case(dec6, 6, 6, 100, 0, 600 + dt);

  // 8-cylinder tests (triggerToothAngle = 90)
  configPage2.nCylinders = 8;
  decoder_t dec8 = triggerSetup_FordTFI();
  run_case(dec8, 1, 8, 100, 0, 0 + dt);
  run_case(dec8, 4, 8, 100, 0, 270 + dt);
  run_case(dec8, 8, 8, 100, 0, 630 + dt);

  // Negative delta
#if !defined(NATIVE_BOARD)
  run_case(dec8, 8, 8, -100, 0, 630 - dt);
#endif
}

static void test_getRPM(void)
{
  auto decoder = triggerSetup_FordTFI();

  // Prepare for cranking path: RPM < crankRPM
  configPage4.StgCycles = 0;
  currentStatus.startRevolutions = 1;
  decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 400;
  // Ensure SetRevolutionTime will update
  currentStatus.revolutionTime = 99999UL;
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 30000UL; // gap=30000 -> revTime = gap*totalTeeth/2 = 30000*4/2 = 60000
  TEST_ASSERT_EQUAL_UINT16(500, decoder.getRPM());

  // Running path: should return stdGetRPM(CAM_SPEED) -> currentStatus.RPM when no toothOne* update
  currentStatus.setRpm(2000);
  toothOneMinusOneTime = 0;
  toothOneTime = 0;
  decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_UINT16(2000U, decoder.getRPM());

  // If not synced, should return currentStatus.RPM
  decoderStatus.syncStatus = SyncStatus::None;
  currentStatus.setRpm(555);
  TEST_ASSERT_EQUAL_UINT16(555U, decoder.getRPM());
}

void testFordTFI(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}