#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testAuxControl(void);

    testAuxControl();
}

TEST_HARNESS(runAllTests)
