#include <unity.h>
#include "src/PID/integerPID.h"
#include "../test_utils.h"

static void test_integerPID_manual_mode_compute_false(void)
{
    long input = 10;
    long output = 8;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetMode(MANUAL);

    TEST_ASSERT_FALSE(pid.Compute(true));
    TEST_ASSERT_EQUAL(8, output);
}

static void test_integerPID_auto_mode_p_on_error(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(true));
    // Expected: kp scaled to 320, error=10, output=3200>>10=3
    TEST_ASSERT_EQUAL(3, output);
}

static void test_integerPID_output_limits_clamp(void)
{
    long input = 1;
    long output = 0;
    long setpoint = 1000;

    integerPID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(0, 10);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(true));
    TEST_ASSERT_EQUAL(10, output);
}

static void test_integerPID_input_zero_failsafe(void)
{
    long input = 0;
    long output = 3;
    long setpoint = 100;

    integerPID pid(&input, &output, &setpoint, 10, 10, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_FALSE(pid.Compute(true, 0));
    TEST_ASSERT_EQUAL(3, output);
}

static void test_integerPID_reverse_direction(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 10, 0, 0, REVERSE);
    pid.SetSampleTime(1);
    pid.SetOutputLimits(-255, 255);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(true));
    TEST_ASSERT_LESS_THAN(0, output);  // negative output expected in reverse
}

static void test_integerPID_feedforward_term(void)
{
    long input = 10;
    long output = 0;
    long setpoint = 20;

    integerPID pid(&input, &output, &setpoint, 0, 0, 0, DIRECT);
    pid.SetSampleTime(1);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute(true, 15));
    TEST_ASSERT_EQUAL(15, output);
}

void testIntegerPID(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_integerPID_manual_mode_compute_false);
        RUN_TEST_P(test_integerPID_auto_mode_p_on_error);
        RUN_TEST_P(test_integerPID_output_limits_clamp);
        RUN_TEST_P(test_integerPID_reverse_direction);
        RUN_TEST_P(test_integerPID_feedforward_term);
        RUN_TEST_P(test_integerPID_input_zero_failsafe);
    }
}