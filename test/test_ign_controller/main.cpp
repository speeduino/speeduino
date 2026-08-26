#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testIgnitionController(void);

    testIgnitionController();
}

TEST_HARNESS(runAllTests)
