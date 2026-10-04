#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testTSCommandHandler(void);

    testTSCommandHandler();
}

TEST_HARNESS(runAllTests)
