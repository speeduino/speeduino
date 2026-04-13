#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllPIDTests(void)
{
    extern void testPID(void);
    extern void testIntegerPID(void);
    extern void testIntegerPID_ideal(void);

    testPID();
    testIntegerPID();
    testIntegerPID_ideal();
}

TEST_HARNESS(runAllPIDTests)
