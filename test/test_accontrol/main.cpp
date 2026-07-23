#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testAcInit(void);
    extern void testAcControl(void);

    testAcInit();
    testAcControl();
}

TEST_HARNESS(runAllTests)
