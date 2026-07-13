#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testInitialiseIdle(void);
    extern void testDisableIdle(void);

    testInitialiseIdle();
    testDisableIdle();
}

TEST_HARNESS(runAllTests)
