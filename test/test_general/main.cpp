#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testPinMapping(void);
    extern void testResetControl(void);
    extern void testStatuses(void);

    testPinMapping();
    testResetControl();
    testStatuses();
}

TEST_HARNESS(runAllTests)
