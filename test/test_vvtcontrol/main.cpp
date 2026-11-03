#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testVvtControl(void);
    extern void testVvtInterrupt(void);
    extern void testInit(void);
    extern void testPwmOutputChannel(void);

    testVvtControl();
    testVvtInterrupt();
    testInit();
    testPwmOutputChannel();
}

TEST_HARNESS(runAllTests)
