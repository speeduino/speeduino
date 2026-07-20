#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testInit(void);
    extern void tesFanControl(void);
    extern void testOnOff(void);
    extern void testFanInterrupt(void);

    testInit();
    tesFanControl();
    testOnOff();
    testFanInterrupt();
}

TEST_HARNESS(runAllTests)
