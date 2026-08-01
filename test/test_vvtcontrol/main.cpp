#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testVvtControl(void);
    extern void testVvtInterrupt(void);
    extern void testInit(void);
    extern void testVvtOutputChannel(void);

    testVvtControl();
    testVvtInterrupt();
    testInit();
    testVvtOutputChannel();
}

TEST_HARNESS(runAllTests)
