#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllSensorTests(void)
{
    extern void test_fastMap10Bit(void);
    extern void test_map_sampling(void);

    test_fastMap10Bit();
    test_map_sampling();
}

TEST_HARNESS(runAllSensorTests)
