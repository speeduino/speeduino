#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile uint16_t toothCurrentCount;

  // helper to setup times and call
  auto call_angle = [&](auto &decoder, uint8_t toothNum, uint32_t deltaSinceLast, int trigAngle = 0){
    toothLastToothTime = 2000;
    toothLastMinusOneToothTime = toothLastToothTime - 500; // tooth interval = 500
    toothCurrentCount = toothNum;
    decoderStatus.toothAngleIsCorrect = true;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
    return decoder.pGetCrankAngle(toothLastToothTime + deltaSinceLast);
  };

  // Test 4-cylinder configuration
  configPage2.nCylinders = 4;
  auto decoder4 = triggerSetup_4G63();
  decoderStatus.syncStatus = SyncStatus::Full;

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
  decoderStatus.syncStatus = SyncStatus::Full;
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
  decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_INT16(41, call_angle(decoder4b, 1, dt, 10)); // 31 + 10

  // If not synced, should return 0
  decoderStatus.syncStatus = SyncStatus::None;
  TEST_ASSERT_EQUAL_INT16(0, decoder4b.pGetCrankAngle(toothLastToothTime + dt));
}

static void test_getRPM(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile unsigned long toothLastMinusOneToothTime;
  extern volatile unsigned long toothOneTime;
  extern volatile unsigned long toothOneMinusOneTime;
  extern volatile uint16_t triggerToothAngle;

  auto decoder = triggerSetup_4G63();

  // Cranking path: currentStatus.RPM < currentStatus.crankRPM
  currentStatus.setRpm(0);
  currentStatus.crankRPM = 400;
  decoderStatus.syncStatus = SyncStatus::Full;

  // Simulate two tooth times 500us apart -> delta = 500
  toothLastToothTime = 2000;
  toothLastMinusOneToothTime = 1500;

  // For a 70 degree tooth gap (one of the 4G63 cranking gaps)
  triggerToothAngle = 70;
  TEST_ASSERT_EQUAL_UINT16(23333, decoder.getRPM()); // 70 * 6e6 / (500*36) = 23333

  // For a 110 degree tooth gap
  triggerToothAngle = 110;
  TEST_ASSERT_EQUAL_UINT16(36666, decoder.getRPM()); // 110 * 6e6 / (500*36) = 36666

  // Running path: should return stdGetRPM(CAM_SPEED) -> defaults to currentStatus.RPM when no revolution update
  currentStatus.setRpm(1000);
  // Clear toothOne* to avoid UpdateRevolutionTimeFromTeeth changing revolutionTime
  toothOneTime = 0;
  toothOneMinusOneTime = 0;
  decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL_UINT16(1000, decoder.getRPM());
}

void test4G63(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
    RUN_TEST_P(test_getRPM);
  }
}