#include <unity.h>
#include "src/PID/PidTuningParameters.h"
#include "../test_utils.h"

static void test_ctor(void)
{
    // Test default constructor
    PidTuningParameters defaultParams;
    TEST_ASSERT_EQUAL(1, defaultParams.Kp);
    TEST_ASSERT_EQUAL(1, defaultParams.Ki);
    TEST_ASSERT_EQUAL(1, defaultParams.Kd);

    // Test parameterized constructor
    PidTuningParameters customParams(10, 20, 30);
    TEST_ASSERT_EQUAL(10, customParams.Kp);
    TEST_ASSERT_EQUAL(20, customParams.Ki);
    TEST_ASSERT_EQUAL(30, customParams.Kd);
}

static void test_multiply(void)
{
    // Test scalar multiplication (params * scalar)
    PidTuningParameters baseParams(2, 4, 6);
    PidTuningParameters multiplied = baseParams * 3;
    TEST_ASSERT_EQUAL(6, multiplied.Kp);
    TEST_ASSERT_EQUAL(12, multiplied.Ki);
    TEST_ASSERT_EQUAL(18, multiplied.Kd);

    // Test scalar multiplication (scalar * params)
    PidTuningParameters multiplied2 = 2 * baseParams;
    TEST_ASSERT_EQUAL(4, multiplied2.Kp);
    TEST_ASSERT_EQUAL(8, multiplied2.Ki);
    TEST_ASSERT_EQUAL(12, multiplied2.Kd);

    // Test scalar multiplication with negative numbers (params * negative scalar)
    PidTuningParameters multipliedNeg = baseParams * (-2);
    TEST_ASSERT_EQUAL(-4, multipliedNeg.Kp);
    TEST_ASSERT_EQUAL(-8, multipliedNeg.Ki);
    TEST_ASSERT_EQUAL(-12, multipliedNeg.Kd);

    // Test scalar multiplication with negative numbers (negative scalar * params)
    PidTuningParameters multipliedNeg2 = (-3) * baseParams;
    TEST_ASSERT_EQUAL(-6, multipliedNeg2.Kp);
    TEST_ASSERT_EQUAL(-12, multipliedNeg2.Ki);
    TEST_ASSERT_EQUAL(-18, multipliedNeg2.Kd);
}

static void test_divide(void)
{
    // Test scalar division (params / scalar)
    PidTuningParameters baseParams(2, 4, 6);
    PidTuningParameters divided = baseParams / 2;
    TEST_ASSERT_EQUAL(1, divided.Kp);
    TEST_ASSERT_EQUAL(2, divided.Ki);
    TEST_ASSERT_EQUAL(3, divided.Kd);

    // Test scalar division (scalar / params) - results in zero due to integer division
    PidTuningParameters divided2 = 10 / baseParams;
    TEST_ASSERT_EQUAL(0, divided2.Kp);  // 2/10 = 0 (integer division)
    TEST_ASSERT_EQUAL(0, divided2.Ki);  // 4/10 = 0
    TEST_ASSERT_EQUAL(0, divided2.Kd);  // 6/10 = 0

    // Test scalar division with negative numbers (params / negative scalar)
    PidTuningParameters dividedNeg = baseParams / (-2);
    TEST_ASSERT_EQUAL(-1, dividedNeg.Kp);  // 2/-2 = -1
    TEST_ASSERT_EQUAL(-2, dividedNeg.Ki);  // 4/-2 = -2
    TEST_ASSERT_EQUAL(-3, dividedNeg.Kd);  // 6/-2 = -3

    // Test scalar division with negative numbers (negative scalar / params)
    PidTuningParameters dividedNeg2 = (-10) / baseParams;
    TEST_ASSERT_EQUAL(0, dividedNeg2.Kp);  // 2/-10 = 0 (integer division)
    TEST_ASSERT_EQUAL(0, dividedNeg2.Ki);  // 4/-10 = 0
    TEST_ASSERT_EQUAL(0, dividedNeg2.Kd);  // 6/-10 = 0
}

void testPidTuningParameters(void)
{
   SET_UNITY_FILENAME() {
        RUN_TEST_P(test_ctor);
        RUN_TEST_P(test_multiply);
        RUN_TEST_P(test_divide);
   }
}