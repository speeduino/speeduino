#include "../test_harness_device.h"
#include "../test_harness_native.h"
#include "src/controllers/launch/launchController.h"

void runAllTests(void)
{
    extern void testLaunchControl(void);

    testLaunchControl();
}

TEST_HARNESS(runAllTests)
