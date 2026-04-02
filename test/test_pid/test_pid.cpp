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

    unsigned long now = 1000000;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL(12, output); // 255*50/1000 integer scaling yields 12

    pid.SetControllerDirection(REVERSE);
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_LESS_THAN(0, output); // sign changed
}

static void test_pid_set_sample_time_rescaling(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 10, 1, 0, DIRECT);
    pid.SetMode(AUTOMATIC);

    pid.SetSampleTime(200); // changes ki/kd scaling
    unsigned long now = 1000000;
    TEST_ASSERT_TRUE(pid.Compute(now));
}

static void test_pid_output_limits_in_auto_scope(void)
{
    long input = 0;
    long output = 100;
    long setpoint = 500;

    PID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetMode(AUTOMATIC);
    pid.SetOutputLimits(0, 100); // inAuto path updates output and ITerm

    unsigned long now = 1000000;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_LESS_OR_EQUAL(100, output);
}

static void test_pid_manual_mode_compute_false(void)
{
    long input = 0;
    long output = 42;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 100, 0, 0, DIRECT);
    pid.SetMode(MANUAL);

    unsigned long now = 1000000;
    TEST_ASSERT_FALSE(pid.Compute(now));
    TEST_ASSERT_EQUAL(42, output);
}

static void test_pid_auto_mode_proportional(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 255, 0, 0, DIRECT);
    pid.SetMode(AUTOMATIC);

    unsigned long now = 1000000;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL(25, output);  // 255 * 100 / 1000 = 25

    input = 100;
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
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

    unsigned long now = 1000000;
    TEST_ASSERT_TRUE(pid.Compute(now));
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

    unsigned long now = 1000000;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL(-255, output);
}

static void test_pid_set_sample_time_scaling_integral(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 0, 10, 0, DIRECT);
    pid.SetMode(AUTOMATIC);

    unsigned long now = 1000000;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL(1, output); 

    pid.SetSampleTime(200);
    output = 0;
    now += 200;

    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL(1, output); 
}

static void test_pid_set_sample_time_scaling_derivative(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 0;

    PID pid(&input, &output, &setpoint, 0, 0, 10, DIRECT);
    pid.SetMode(AUTOMATIC);

    unsigned long now = 1000000;
    TEST_ASSERT_TRUE(pid.Compute(now));
    input = 100;
    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL(0, output); // dInput 100, kd=1 => -100/1000=0

    pid.SetSampleTime(200);
    input = 200;
    now += 200;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL(0, output); // kd halved
}

static void test_pid_integral_accumulation(void)
{
    long input = 0;
    long output = 0;
    long setpoint = 100;

    PID pid(&input, &output, &setpoint, 0, 10, 0, DIRECT);
    pid.SetMode(AUTOMATIC);

    unsigned long now = 1000000;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL(1, output);

    now += 100;
    TEST_ASSERT_TRUE(pid.Compute(now));
    TEST_ASSERT_EQUAL(2, output);
}

void testPID(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_pid_manual_mode_compute_false);
        RUN_TEST_P(test_pid_auto_mode_proportional);
        RUN_TEST_P(test_pid_output_limits_clamp);
        RUN_TEST_P(test_pid_reverse_direction);
        RUN_TEST_P(test_pid_integral_accumulation);
        RUN_TEST_P(test_pid_set_sample_time_scaling_derivative);
        RUN_TEST_P(test_pid_set_sample_time_scaling_integral);
        RUN_TEST_P(test_pid_output_limits_in_auto_scope);
        RUN_TEST_P(test_pid_set_sample_time_rescaling);
        RUN_TEST_P(test_pid_mode_transitions_and_controller_direction);
    }
}