#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testPinMapping(void);
    extern void testResetControl(void);
    extern void testTSCommandHandler(void);

    testPinMapping();
    testResetControl();
    testTSCommandHandler();
}

TEST_HARNESS(runAllTests)
