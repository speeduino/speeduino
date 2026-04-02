#include <unity.h>
#include "src/PID/integerPID_ideal.h"
#include "../test_utils.h"

static void test_integerPID_ideal_p_only_clamped_to_min(void)
{
    long input = 100;
    uint16_t output = 0;
    uint16_t setpoint = 200;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 1, 0, 0, DIRECT);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(0));
    TEST_ASSERT_EQUAL(2000u, output); // min clamp 20% *100
}

static void test_integerPID_ideal_p_only_clamped_to_max(void)
{
    long input = 0;
    uint16_t output = 0;
    uint16_t setpoint = 1000;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 100, 0, 0, DIRECT);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(0));
    TEST_ASSERT_EQUAL(8000u, output); // max clamp 80% *100
}

static void test_integerPID_ideal_sample_time_gate(void)
{
    long input = 500;
    uint16_t output = 0;
    uint16_t setpoint = 500;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 250;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 0, 0, 0, DIRECT);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(0));
    // Running immediately again should be gated by sample time (likely false)
    TEST_ASSERT_FALSE(pid.Compute(0));
}

static void test_integerPID_ideal_ki_windup_limits(void)
{
    long input = 0;
    uint16_t output = 0;
    uint16_t setpoint = 1000;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 10, 10, 0, DIRECT);
    pid.SetOutputLimits(20, 80);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(0));
    // output should stay within [2000, 8000] scale after clamping
    TEST_ASSERT_LESS_OR_EQUAL(8000u, output);
    TEST_ASSERT_GREATER_OR_EQUAL(2000u, output);
}

static void test_integerPID_ideal_reverse_direction(void)
{
    long input = 100;
    uint16_t output = 0;
    uint16_t setpoint = 200;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pidDirect(&input, &output, &setpoint, &sensitivity, &sampleTime, 50, 0, 0, DIRECT);
    pidDirect.SetOutputLimits(0, 100);
    pidDirect.Initialize();
    TEST_ASSERT_TRUE(pidDirect.Compute(0));
    uint16_t directOutput = output;

    integerPID_ideal pidReverse(&input, &output, &setpoint, &sensitivity, &sampleTime, 50, 0, 0, REVERSE);
    pidReverse.SetOutputLimits(0, 100);
    pidReverse.Initialize();
    TEST_ASSERT_TRUE(pidReverse.Compute(0));
    uint16_t reverseOutput = output;

    TEST_ASSERT_LESS_OR_EQUAL(reverseOutput, directOutput);
}
static void test_integerPID_ideal_feedforward_applied(void)
{
    long input = 500;
    uint16_t output = 0;
    uint16_t setpoint = 500;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 0, 0, 0, DIRECT);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(5000));
    TEST_ASSERT_EQUAL(5000u, output); // no PID action, output equals feedforward (above min clamp)
}

void testIntegerPID_ideal(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_integerPID_ideal_p_only_clamped_to_min);
        RUN_TEST_P(test_integerPID_ideal_p_only_clamped_to_max);
        RUN_TEST_P(test_integerPID_ideal_sample_time_gate);
        RUN_TEST_P(test_integerPID_ideal_ki_windup_limits);
        RUN_TEST_P(test_integerPID_ideal_reverse_direction);
        RUN_TEST_P(test_integerPID_ideal_feedforward_applied);
    }
}