#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testInit(void);
    extern void testControl(void);

    testInit();
    testControl();
}

TEST_HARNESS(runAllTests)
