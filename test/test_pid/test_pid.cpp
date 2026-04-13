#include <unity.h>
#include "src/PID/PID.h"
#include "../test_utils.h"

static void test_pid_mode_transitions_and_controller_direction(void)
{
    long input = 50;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetOutputLimits(-200, 200);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(12, output); // 255*50/1000 integer scaling yields 12

    pid.SetControllerDirection(REVERSE);
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_LESS_THAN(0, output); // sign changed
}

static void test_pid_output_limits_in_auto_scope(void)
{
    long input = 0;
    long output = 100;
    long setpoint = 500;

    PID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetMode(AUTOMATIC);
    pid.SetOutputLimits(0, 100); // inAuto path updates output and ITerm

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_LESS_OR_EQUAL(100, output);
}

static void test_pid_manual_mode_compute_false(void)
{
    long input = 0;
    long output = 42;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 100, 0, 0, DIRECT);
    pid.SetMode(MANUAL);

    TEST_ASSERT_FALSE(pid.Compute());
    TEST_ASSERT_EQUAL(42, output);
}

static void test_pid_auto_mode_proportional(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(25, output);  // 255 * 100 / 1000 = 25

    input = 100;
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(0, output);
}

static void test_pid_output_limits_clamp(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 1000;  // results in max output for Kp=255

    PID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetOutputLimits(0, 255);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(255, output);
}

static void test_pid_reverse_direction(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 1000;

    PID pid(&input, &output, &setpoint, 255, 0, 0, REVERSE);
    pid.SetOutputLimits(-255, 255);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(-255, output);
}

static void test_pid_set_output_limits_long_range(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 1000;

    PID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetMode(AUTOMATIC);

    pid.SetOutputLimits(0, 10);
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(10, output);

    pid.SetOutputLimits(-20, 20);
    input = 1000;
    setpoint = 0;
    pid.Initialize();
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(-20, output);

    pid.SetOutputLimits(100, 200);
    input = 0;
    setpoint = 0;
    pid.Initialize();
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(100, output);

    pid.SetOutputLimits(150, 150); // invalid min==max should be ignored
    input = 0;
    setpoint = 0;
    pid.Initialize();
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(100, output); // remains from previous in-range bounds
}

static void test_pid_integral_accumulation(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 0, 10, 0, DIRECT);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(1, output);

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(2, output);
}

static void test_pid_set_tunings_runtime_changes(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute());
    long originalOutput = output;

    pid.SetTunings(20, 0, 0); // Change Kp
    TEST_ASSERT_TRUE(pid.Compute());
    // Output should change due to new Kp
    TEST_ASSERT_NOT_EQUAL(originalOutput, output);
}

static void test_pid_initialize_resets_state(void)
{
    long input = 0;
    long output = 50;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 0, 10, 0, DIRECT);
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute()); // Accumulate some integral

    pid.Initialize(); // Reset state
    TEST_ASSERT_TRUE(pid.Compute());
    // After reset, output should be based on new state (integral reset)
    TEST_ASSERT_NOT_EQUAL(50, output); // Should not be the initial output
}

static void test_pid_set_output_limits_invalid_ignored(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetOutputLimits(50, 20); // Invalid: Min >= Max
    pid.SetMode(AUTOMATIC);

    TEST_ASSERT_TRUE(pid.Compute());
    // Output should still be computed normally (limits ignored, default 0-255)
    TEST_ASSERT_NOT_EQUAL(0, output);
}

static void test_pid_set_controller_direction_runtime_manual(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 10, 0, 0, DIRECT);
    pid.SetMode(MANUAL); // Manual mode
    pid.SetControllerDirection(REVERSE); // Should not affect in manual mode

    pid.SetMode(AUTOMATIC);
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_GREATER_THAN(0, output); // Reverse direction normally produces negative output
}

// Run the PID for maxIterations and confirm it hits the setpoint
static void assert_pid_complete(PID &pid, long *pInput, long *pOutput, long setpoint, uint16_t maxIterations)
{
    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();

    char szMsg[64];
    for (uint16_t iteration=0; iteration<maxIterations; ++iteration)
    {
        TEST_ASSERT_TRUE(pid.Compute());
        *pInput += *pOutput;

        snprintf(szMsg, _countof(szMsg)-1, "%" PRIu16 ", %" PRId32 ", %" PRId32, iteration, (int32_t)*pInput, (int32_t)*pOutput);
        UnityPrint(szMsg); UNITY_PRINT_EOL();
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(DIV_ROUND_CLOSEST(setpoint, 100, int32_t), setpoint, *pInput);
}

static void test_end_to_end_positive_positive_up(void) 
{
    long output = 0;
    long input = 900;
    long setpoint = 1500;

    PID pid(&input, &output, &setpoint, 100, 30, 50, DIRECT);
    pid.SetOutputLimits(-25, 25);
    pid.SetMode(AUTOMATIC);
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, setpoint, 50);
}

static void test_end_to_end_positive_positive_down(void) 
{
    long output = 0;
    long input = 1250;
    long setpoint = 900;

    PID pid(&input, &output, &setpoint, 100, 5, 5, DIRECT);
    pid.SetOutputLimits(-75, 75);
    pid.SetMode(AUTOMATIC);
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, setpoint, 100);
}

static void test_end_to_end_negative_negative_up(void) 
{
    long output = 0;
    long input = -1500;
    long setpoint = -900;

    PID pid(&input, &output, &setpoint, 100, 30, 50, DIRECT);
    pid.SetOutputLimits(-25, 25);
    pid.SetMode(AUTOMATIC);
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, setpoint, 50);
}

static void test_end_to_end_negative_negative_down(void) 
{
    long output = 0;
    long input = -900;
    long setpoint = -1500;

    PID pid(&input, &output, &setpoint, 100, 30, 50, DIRECT);
    pid.SetOutputLimits(-25, 25);
    pid.SetMode(AUTOMATIC);
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, setpoint, 50);
}

static void test_end_to_end_negative_positive(void) 
{
    long output = 0;
    long input = -199;
    long setpoint = 199;

    PID pid(&input, &output, &setpoint, 50, 1, 80, DIRECT);
    pid.SetOutputLimits(-75, 75);
    pid.SetMode(AUTOMATIC);
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, setpoint, 50);
}

static void test_end_to_end_positive_to_negative(void) 
{
    long output = 0;
    long input = 900;
    long setpoint = -1500;

    PID pid(&input, &output, &setpoint, 100, 30, 20, DIRECT);
    pid.SetOutputLimits(-25, 25);
    pid.SetMode(AUTOMATIC);
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, setpoint, 50);
}

void testPID(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_pid_manual_mode_compute_false);
        RUN_TEST_P(test_pid_auto_mode_proportional);
        RUN_TEST_P(test_pid_output_limits_clamp);
        RUN_TEST_P(test_pid_reverse_direction);
        RUN_TEST_P(test_pid_integral_accumulation);
        RUN_TEST_P(test_pid_output_limits_in_auto_scope);
        RUN_TEST_P(test_pid_mode_transitions_and_controller_direction);
        RUN_TEST_P(test_pid_set_output_limits_long_range);
        RUN_TEST_P(test_pid_set_tunings_runtime_changes);
        RUN_TEST_P(test_pid_initialize_resets_state);
        RUN_TEST_P(test_pid_set_output_limits_invalid_ignored);
        RUN_TEST_P(test_pid_set_controller_direction_runtime_manual);
        RUN_TEST_P(test_end_to_end_positive_positive_up);
        RUN_TEST_P(test_end_to_end_positive_positive_down);
        RUN_TEST_P(test_end_to_end_negative_negative_up);
        RUN_TEST_P(test_end_to_end_negative_negative_down);
        RUN_TEST_P(test_end_to_end_negative_positive);
        RUN_TEST_P(test_end_to_end_positive_to_negative);
    }
}