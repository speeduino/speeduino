#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testPinMapping(void);

    testPinMapping();
}

TEST_HARNESS(runAllTests)
