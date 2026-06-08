#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testProgrammableIOControl(void);
    extern void testProgrammableIOControlStateT(void);
    extern void testCompOperation(void);
    
    testProgrammableIOControl();
    testProgrammableIOControlStateT();
    testCompOperation();
}

TEST_HARNESS(runAllTests)
