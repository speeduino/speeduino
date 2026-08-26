#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testAcInit(void);
    extern void testAcControl(void);
    extern void testAcControlDetails(void);

    testAcInit();
    testAcControl();
    testAcControlDetails();
}

TEST_HARNESS(runAllTests)
