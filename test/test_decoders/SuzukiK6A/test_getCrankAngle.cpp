#include <unity.h>
#include "../../test_utils.h"
#include "decoders.h"
#include "init.h"
#include "globals.h"

static void test_k6a_getCrankAngle_tooth(uint8_t toothNum, uint16_t expectedCrankAngle, uint16_t expectedToothAngle) {
    decoder_t decoder = triggerSetup_SuzukiK6A();
    configPage4.triggerAngle = 0U;

    extern volatile unsigned long toothLastToothTime;
    toothLastToothTime = micros() - 150U;
    toothCurrentCount = toothNum;
    // Allow some variance since the algorithm relies on calling micros();
    TEST_ASSERT_INT16_WITHIN_MESSAGE(2, expectedCrankAngle, decoder.getCrankAngle(), "Crank Angle");
    TEST_ASSERT_EQUAL_MESSAGE(expectedToothAngle, triggerToothAngle, "Tooth Angle");
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

void testSuzukiK6A_getCrankAngle()
{
    SET_UNITY_FILENAME() {

        RUN_TEST(test_k6a_getCrankAngle_tooth0);
        RUN_TEST(test_k6a_getCrankAngle_tooth1);
        RUN_TEST(test_k6a_getCrankAngle_tooth2);
        RUN_TEST(test_k6a_getCrankAngle_tooth3);
        RUN_TEST(test_k6a_getCrankAngle_tooth4);
        RUN_TEST(test_k6a_getCrankAngle_tooth5);
        RUN_TEST(test_k6a_getCrankAngle_tooth6);
        RUN_TEST(test_k6a_getCrankAngle_tooth7);
        RUN_TEST(test_k6a_getCrankAngle_tooth8);

    }
}