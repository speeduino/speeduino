#include "../device_test_harness.h"

void runAllSensorTests(void)
{
    extern void test_fastMap10Bit(void);
    extern void test_map_sampling(void);

    test_fastMap10Bit();
    test_map_sampling();
}

DEVICE_TEST(runAllSensorTests)
