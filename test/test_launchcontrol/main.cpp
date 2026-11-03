#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testLaunchControl(void);
    extern void testInit(void);

    testLaunchControl();
    testInit();
}

TEST_HARNESS(runAllTests)
