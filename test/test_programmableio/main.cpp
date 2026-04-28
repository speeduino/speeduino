#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testProgrammableIOControl(void);
    extern void testProgrammableIOControlStateT(void);

    testProgrammableIOControl();
    testProgrammableIOControlStateT();
}

TEST_HARNESS(runAllTests)
