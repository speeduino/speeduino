#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testTSCommandHandler(void);
    extern void testPulsedCommandController(void);

    testTSCommandHandler();
    testPulsedCommandController();
}

TEST_HARNESS(runAllTests)
