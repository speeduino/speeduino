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
    extern void testAuxiliaries(void);
    extern void testUpdates(void);
    extern void testIdle(void);

    testPinMapping();
    testResetControl();
    testPolling();
    testTimers();
    testLogger();
    testTSCommandHandler();
    testAuxiliaries();
    testUpdates();
    testIdle();
}

TEST_HARNESS(runAllTests)
