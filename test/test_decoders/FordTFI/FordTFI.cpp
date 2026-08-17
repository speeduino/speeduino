#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;

  auto run_case = [&](decoder_t &decoder, uint8_t toothCount, uint8_t trigTeeth, int16_t delta, int16_t trigAngle, int16_t expected) {
    // trigTeeth is number of cylinders (configPage2.nCylinders)
    configPage2.nCylinders = trigTeeth;
    toothLastToothTime = 2000;
    toothCurrentCount = toothCount;
    decoderStatus.syncStatus = SyncStatus::Full;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle((uint32_t)((int32_t)toothLastToothTime + delta)));
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

void testFordTFI(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}