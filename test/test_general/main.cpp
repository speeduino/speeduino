#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testPinMapping(void);
    extern void testResetControl(void);
    extern void testPolling(void);
    extern void testTimers(void);
    extern void testLogger(void);
    extern void testTSCommandHandler(void);

    testPinMapping();
    testResetControl();
    testPolling();
    testTimers();
    testLogger();
    testTSCommandHandler();
}

TEST_HARNESS(runAllTests)
