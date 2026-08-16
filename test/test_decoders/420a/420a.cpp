#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"
#include "crankMaths.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;

  auto decoder = triggerSetup_420a();

  auto run_case = [&](int toothNum, int16_t expected, int trigAngle = 0) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothNum;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    setAngleConverterRevolutionTime(2000);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(toothLastToothTime + 100));
  };

  // timeToAngle(100) ~= 18 deg
  const int dt = 18;

  run_case(1,  (711 + dt) - 720); // wraps to small positive angle
  run_case(2,  111 + dt);
  run_case(3,  131 + dt);
  run_case(4,  151 + dt);
  run_case(5,  171 + dt);
  run_case(6,  291 + dt);
  run_case(7,  311 + dt);
  run_case(8,  331 + dt);
  run_case(9,  351 + dt);
  run_case(10, 471 + dt);
  run_case(11, 491 + dt);
  run_case(12, 511 + dt);
  run_case(13, 531 + dt);
  run_case(14, 651 + dt);
  run_case(15, 671 + dt);
  run_case(16, 691 + dt);

  // trigger angle offset
  run_case(2, 111 + dt + 10, 10);
}

static void test_getRPM(void)
{
  auto decoder = triggerSetup_420a();

  currentStatus.crankRPM = 400;
  currentStatus.setRpm(currentStatus.crankRPM*2);
  auto rpm1 = decoder.getRPM();
  TEST_ASSERT_NOT_EQUAL(0, rpm1);

  currentStatus.setRpm(currentStatus.crankRPM/2);
  TEST_ASSERT_NOT_EQUAL(rpm1, decoder.getRPM());
  TEST_ASSERT_NOT_EQUAL(0, decoder.getRPM());
}

void test420a(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);    
  }
}