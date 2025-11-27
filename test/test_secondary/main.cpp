#include "../device_test_harness.h"

void runAllSecondaryTests(void)
{
    extern void test_calculateSecondaryFuel(void);
    extern void test_calculateSecondarySpark(void);

    test_calculateSecondaryFuel();
    test_calculateSecondarySpark();
}

DEVICE_TEST(runAllSecondaryTests)
