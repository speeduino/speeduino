#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testTSCommandHandler(void);
    extern void testPulsedCommandController(void);
    extern void testPrimarySerial(void);

    testTSCommandHandler();
    testPulsedCommandController();
    testPrimarySerial();
}

TEST_HARNESS(runAllTests)
