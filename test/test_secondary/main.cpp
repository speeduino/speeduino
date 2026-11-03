#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllSecondaryTests(void)
{
    extern void test_calculateSecondaryFuel(void);
    extern void test_calculateSecondarySpark(void);

    test_calculateSecondaryFuel();
    test_calculateSecondarySpark();
}

TEST_HARNESS(runAllSecondaryTests)
