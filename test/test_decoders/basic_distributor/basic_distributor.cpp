#include "../test_utils.h"

void testBasicDistributor(void)
{
    extern void testBasicDistribCrankAngle(void);
    extern void testBasicDistribRevTime(void);

    testBasicDistribCrankAngle();
    testBasicDistribRevTime();
}