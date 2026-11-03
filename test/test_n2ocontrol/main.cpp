#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testInit(void);
    extern void testN2oControl(void);

    testInit();
    testN2oControl();
}

TEST_HARNESS(runAllTests)
