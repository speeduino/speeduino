#include <unity.h>
#include "../../test_utils.h"
#include "decoders.h"
#include "globals.h"
#include "crankMaths.h"
#include "src/decoders/decoder_state.h"

extern decoders::detail::state_t _decoderState;

static void test_k6a_getCrankAngle_tooth(uint8_t toothNum, uint16_t expectedCrankAngle, uint16_t expectedToothAngle) {
    decoder_t decoder = triggerSetup_SuzukiK6A();
    CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
    configPage4.triggerAngle = 0U;

    uint32_t currMicros = 5000;
    _decoderState.toothLastToothTime = currMicros - 150U;
    _decoderState.toothCurrentCount = toothNum;
    setAngleConverterRevolutionTime(1);
    TEST_ASSERT_INT16_WITHIN(1, expectedCrankAngle, decoder.pGetCrankAngle(currMicros));
    TEST_ASSERT_EQUAL(expectedToothAngle, _decoderState.triggerToothAngle);
}

static void test_k6a_getCrankAngle_tooth0(void) {
    // Zero isn't a valid tooth, but just in case....
    test_k6a_getCrankAngle_tooth(0, 650, 90);
}

static void test_k6a_getCrankAngle_tooth1(void) {
    test_k6a_getCrankAngle_tooth(1, 0, 70);
}

static void test_k6a_getCrankAngle_tooth2(void) {
    test_k6a_getCrankAngle_tooth(2, 170, 170);
}

static void test_k6a_getCrankAngle_tooth3(void) {
    test_k6a_getCrankAngle_tooth(3, 240, 70);
}

static void test_k6a_getCrankAngle_tooth4(void) {
    test_k6a_getCrankAngle_tooth(4, 410, 170);
}


static void test_k6a_getCrankAngle_tooth5(void) {
    test_k6a_getCrankAngle_tooth(5, 480, 70);
}

static void test_k6a_getCrankAngle_tooth6(void) {
    test_k6a_getCrankAngle_tooth(6, 515, 35);
}

static void test_k6a_getCrankAngle_tooth7(void) {
    test_k6a_getCrankAngle_tooth(7, 650, 135);
}

static void test_k6a_getCrankAngle_tooth8(void) {
    // 8 isn't a valid tooth, but just in case....
    test_k6a_getCrankAngle_tooth(8, 0, 70);
}

static void test_getRevolutionTime(void)
{
  auto decoder = triggerSetup_SuzukiK6A();

  // Standard calculation at cam speed: half the time between the last 2 tooth #1
  _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
  currentStatus.startRevolutions = 1; // not cranking
  _decoderState.toothOneMinusOneTime = 1000UL;
  _decoderState.toothOneTime = _decoderState.toothOneMinusOneTime + 120000UL;
  TEST_ASSERT_EQUAL_UINT32(60000UL, decoder.getRevolutionTime());
}

void testSuzukiK6A_getCrankAngle()
{
    SET_UNITY_FILENAME() {

        RUN_TEST_P(test_k6a_getCrankAngle_tooth0);
        RUN_TEST_P(test_k6a_getCrankAngle_tooth1);
        RUN_TEST_P(test_k6a_getCrankAngle_tooth2);
        RUN_TEST_P(test_k6a_getCrankAngle_tooth3);
        RUN_TEST_P(test_k6a_getCrankAngle_tooth4);
        RUN_TEST_P(test_k6a_getCrankAngle_tooth5);
        RUN_TEST_P(test_k6a_getCrankAngle_tooth6);
        RUN_TEST_P(test_k6a_getCrankAngle_tooth7);
        RUN_TEST_P(test_k6a_getCrankAngle_tooth8);
        RUN_TEST_P(test_getRevolutionTime);
    }
}