#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testInitialiseIdle(void);
    extern void testDisableIdle(void);
    extern void testIdleControl(void);
    extern void testIdleControlOnOff(void);
    extern void testIdleInterrupt(void);

    testInitialiseIdle();
    testDisableIdle();
    testIdleControl();
    testIdleControlOnOff();
    testIdleInterrupt();
}

TEST_HARNESS(runAllTests)
