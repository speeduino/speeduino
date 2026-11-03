#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testBoostInit(void);
    extern void testBoostControl(void);
    extern void testBoostInterrupt(void);

    testBoostInit();
    testBoostControl();
    testBoostInterrupt();
}

TEST_HARNESS(runAllTests)
