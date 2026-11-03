#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllSensorTests(void)
{
    extern void test_fastMap10Bit(void);
    extern void test_map_sampling(void);
    extern void test_baro(void);

    test_fastMap10Bit();
    test_map_sampling();
    test_baro();
}

TEST_HARNESS(runAllSensorTests)
