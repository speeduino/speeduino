#include <unity.h>
#include "src/PID/integerPID_ideal.h"
#include "../test_utils.h"

constexpr uint32_t NOW = 10000UL;

static void test_p_only_clamped_to_min(void)
{
    long input = 100;
    uint16_t output = 0;
    uint16_t setpoint = 200;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 1, 0, 0, DIRECT);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(NOW, 0));
    TEST_ASSERT_EQUAL(2000u, output); // min clamp 20% *100
}

static void test_p_only_clamped_to_max(void)
{
    long input = 0;
    uint16_t output = 0;
    uint16_t setpoint = 1000;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 100, 0, 0, DIRECT);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(NOW, 0));
    TEST_ASSERT_EQUAL(8000u, output); // max clamp 80% *100
}

static void test_sample_time_gate(void)
{
    long input = 500;
    uint16_t output = 0;
    uint16_t setpoint = 500;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 250;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 0, 0, 0, DIRECT);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(NOW, 0));
    // Running immediately again should be gated by sample time (likely false)
    TEST_ASSERT_FALSE(pid.Compute(NOW, 0));
}

static void test_ki_windup_limits(void)
{
    long input = 0;
    uint16_t output = 0;
    uint16_t setpoint = 1000;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 10, 10, 0, DIRECT);
    pid.SetOutputLimits(20, 80);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(NOW, 0));
    // output should stay within [2000, 8000] scale after clamping
    TEST_ASSERT_LESS_OR_EQUAL(8000u, output);
    TEST_ASSERT_GREATER_OR_EQUAL(2000u, output);
}

static void test_reverse_direction(void)
{
    long input = 100;
    uint16_t output = 0;
    uint16_t setpoint = 200;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pidDirect(&input, &output, &setpoint, &sensitivity, &sampleTime, 50, 0, 0, DIRECT);
    pidDirect.SetOutputLimits(0, 100);
    pidDirect.Initialize();

    TEST_ASSERT_TRUE(pidDirect.Compute(NOW, 0));
    uint16_t directOutput = output;

    integerPID_ideal pidReverse(&input, &output, &setpoint, &sensitivity, &sampleTime, 50, 0, 0, REVERSE);
    pidReverse.SetOutputLimits(0, 100);
    pidReverse.Initialize();
    TEST_ASSERT_TRUE(pidReverse.Compute(NOW, 0));
    uint16_t reverseOutput = output;

    TEST_ASSERT_LESS_OR_EQUAL(reverseOutput, directOutput);
}
static void test_feedforward_applied(void)
{
    long input = 500;
    uint16_t output = 0;
    uint16_t setpoint = 500;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 0, 0, 0, DIRECT);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(NOW, 5000));
    TEST_ASSERT_EQUAL(5000u, output); // no PID action, output equals feedforward (above min clamp)
}

static void test_set_output_limits_invalid_bounds_are_ignored(void)
{
    long input = 0;
    uint16_t output = 0;
    uint16_t setpoint = 1000;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 100, 0, 0, DIRECT);
    pid.SetOutputLimits(80, 20);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(NOW, 0));
    TEST_ASSERT_EQUAL(8000u, output); // invalid limits ignored, default max clamp remains 80%
}

static void test_initialize_resets_integral_and_error(void)
{
    long input = 0;
    uint16_t output = 0;
    uint16_t setpoint = 1000;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 1, 10, 0, DIRECT);
    pid.SetOutputLimits(0, 100);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(NOW, 0));

    input = 1000;
    pid.Initialize();
    TEST_ASSERT_TRUE(pid.Compute(NOW + 1U, 0));
    TEST_ASSERT_EQUAL(0u, output); // with zero error and reset integral, output should return to zero
}

static void test_derivative_term_changes_output_on_error_transition(void)
{
    long input = 0;
    uint16_t output = 0;
    uint16_t setpoint = 1000;
    uint16_t sensitivity = 50;
    uint8_t sampleTime = 0;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 1, 0, 1, DIRECT);
    pid.SetOutputLimits(0, 100);
    pid.Initialize();

    TEST_ASSERT_TRUE(pid.Compute(NOW, 0));
    uint16_t firstOutput = output;

    input = 100;
    TEST_ASSERT_TRUE(pid.Compute(NOW + 1U, 0));
    TEST_ASSERT_NOT_EQUAL(firstOutput, output);
}

// Run the PID for 50 and confirm it hits the setpoint
static void assert_pid_complete(integerPID_ideal &pid, long *pInput, uint16_t *pOutput, uint16_t setpoint, uint8_t sampleTime)
{
    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();

    char szMsg[64];
    for (uint16_t iteration=0; iteration<50U; ++iteration)
    {
        TEST_ASSERT_TRUE(pid.Compute(NOW+(iteration*sampleTime), 0));
        *pInput = *pOutput;

        snprintf(szMsg, _countof(szMsg)-1, "%" PRIu16 ", %" PRId32 ", %" PRId32, iteration, (int32_t)*pInput, (int32_t)*pOutput);
        UnityPrint(szMsg); UNITY_PRINT_EOL();
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(DIV_ROUND_CLOSEST(setpoint, 100, int32_t), setpoint, *pInput);
}

static void test_end_to_end_positive_positive_up(void) 
{
    long input = 30;
    uint16_t output = 0;
    uint16_t setpoint = 90;
    uint16_t sensitivity = 0;
    uint8_t sampleTime = 25;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 3, 2, 1, DIRECT);
    pid.SetOutputLimits(0, 255);
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, setpoint, sampleTime);
}

static void test_end_to_end_positive_positive_down(void) 
{
    long input = 90;
    uint16_t output = 0;
    uint16_t setpoint = 30;
    uint16_t sensitivity = 0;
    uint8_t sampleTime = 25;

    integerPID_ideal pid(&input, &output, &setpoint, &sensitivity, &sampleTime, 3, 2, 1, DIRECT);
    pid.SetOutputLimits(0, 255);
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, setpoint, sampleTime);
}

void testIntegerPID_ideal(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_p_only_clamped_to_min);
        RUN_TEST_P(test_p_only_clamped_to_max);
        RUN_TEST_P(test_sample_time_gate);
        RUN_TEST_P(test_ki_windup_limits);
        RUN_TEST_P(test_reverse_direction);
        RUN_TEST_P(test_feedforward_applied);
        RUN_TEST_P(test_set_output_limits_invalid_bounds_are_ignored);
        RUN_TEST_P(test_initialize_resets_integral_and_error);
        RUN_TEST_P(test_derivative_term_changes_output_on_error_transition);
        RUN_TEST_P(test_end_to_end_positive_positive_up);
        RUN_TEST_P(test_end_to_end_positive_positive_down);
        // Following do not make sense for integerPID_ideal since input & output are uint16_t
        // RUN_TEST_P(test_end_to_end_negative_negative_up);
        // RUN_TEST_P(test_end_to_end_negative_negative_down);
        // RUN_TEST_P(test_end_to_end_negative_positive);
        // RUN_TEST_P(test_end_to_end_positive_to_negative);       
    }
}