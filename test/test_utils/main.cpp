#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testStaticFor(void);

    testStaticFor();
}

TEST_HARNESS(runAllTests)
