#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;
  extern volatile bool revolutionOne;

  // Configure and create decoder
  auto decoder = triggerSetup_Audi135();

  auto run_case = [&](int toothCount, bool revOne, int delta, int trigAngle, int16_t expected) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothCount;
    revolutionOne = revOne;
    decoderStatus.syncStatus = SyncStatus::Full;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + delta);
    TEST_ASSERT_EQUAL(expected, angle);
  };

  // timeToAngle(100) ~= 18 deg when revolution time = 2000
  const int dt = 18;

  // Basic checks
  run_case(1, false, 100, 0, 0 + dt);
  run_case(3, false, 100, 0, (3 - 1) * 8 + dt); // tooth 3 -> 16 + dt
  run_case(10, false, 100, 0, (10 - 1) * 8 + dt); // tooth 10 -> 72 + dt

  // Last-tooth and zero-case (zero treated as 45)
  run_case(45, false, 100, 0, (45 - 1) * 8 + dt);
  run_case(0, false, 100, 0, (45 - 1) * 8 + dt);

  // trigger angle offset
  run_case(3, false, 100, 5, (3 - 1) * 8 + 5 + dt);

  // sequential/revolution cases
  run_case(1, true, 100, 0, 360 + 0 + dt);
  // Case where adding 360 wraps above 720: tooth 45 + revOne -> (352+dt+360)-720
  {
    int raw = (45 - 1) * 8 + dt + 360;
    int wrapped = raw >= 720 ? raw - 720 : raw;
    run_case(45, true, 100, 0, wrapped);
  }
}

static void test_getRPM(void)
{
  auto decoder = triggerSetup_Audi135();
  TEST_ASSERT_NOT_EQUAL(0, decoder.getRPM());
}

void testAudi135(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}