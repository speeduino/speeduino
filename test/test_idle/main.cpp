#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testInitialiseIdle(void);
    extern void testDisableIdle(void);
    extern void testIdleControl(void);

    testInitialiseIdle();
    testDisableIdle();
    testIdleControl();
}

TEST_HARNESS(runAllTests)
