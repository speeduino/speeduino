#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testProgrammableIOControl(void);

    testProgrammableIOControl();
}

TEST_HARNESS(runAllTests)
