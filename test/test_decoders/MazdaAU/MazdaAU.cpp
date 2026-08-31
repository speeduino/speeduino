#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile int toothCurrentCount;

  auto decoder = triggerSetup_MazdaAU();

  auto setup_case = [&](int toothNum, int trigAngle) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothNum;
    decoderStatus.toothAngleIsCorrect = true;
    decoderStatus.syncStatus = SyncStatus::Full;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
  };

  auto run_case = [&](int toothNum, int16_t expected, int trigAngle = 0) {
    setup_case(toothNum, trigAngle);
    TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(toothLastToothTime + 100));
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
  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL(0, decoder.pGetCrankAngle(toothLastToothTime + 100));
  setup_case(1, 0);
  decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL(0, decoder.pGetCrankAngle(toothLastToothTime + 100));
}

static void test_getRPM(void)
{
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;
  extern decoder_status_t decoderStatus;

  auto decoder = triggerSetup_MazdaAU();

  // Ensure sync present
  decoderStatus.syncStatus = SyncStatus::Full;

  // --- Cranking branch: currentStatus.RPM < currentStatus.crankRPM
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 200;
  // ensure SetRevolutionTime will update
  currentStatus.revolutionTime = 12345UL;
  // gap such that revTime = 36 * gap = 6,480,000 -> RPM = (108 * MICROS_PER_MIN)/6,480,000 = 1000
  toothLastMinusOneToothTime = 1000UL;
  toothLastToothTime = toothLastMinusOneToothTime + 180000UL; // gap = 180000
  // triggerToothAngle set by decoder setup to 108; assert expected RPM
  TEST_ASSERT_EQUAL_UINT16(337, decoder.getRPM());

  // --- Running path: uses stdGetRPM(CRANK_SPEED)
  currentStatus.setRpm(2000);
  // ensure SetRevolutionTime will update in stdGetRPM
  currentStatus.revolutionTime = 12345UL;
  toothOneMinusOneTime = 1000UL;
  toothOneTime = toothOneMinusOneTime + 60000UL; // revTime = 60000 -> 1000 RPM
  TEST_ASSERT_EQUAL_UINT16(1000U, decoder.getRPM());

  // --- Fallback: when not full sync, should return 0
  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_UINT16(0U, decoder.getRPM());

}

void testMazdaAU(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}