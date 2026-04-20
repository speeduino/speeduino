#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testPinMapping(void);
    extern void testResetControl(void);
    extern void testProgrammableIOControl(void);

    testPinMapping();
    testResetControl();
    testProgrammableIOControl();
}

TEST_HARNESS(runAllTests)
