#include "../test_utils.h"

void testBasicDistributor(void)
{
    extern void testBasicDistribCrankAngle(void);
    extern void testBasicDistribRevTime(void);
    extern void testBasicDistribInitialise(void);
    extern void testBasicDistribTriggers(void);
    extern void testBasicDistribPerToothIgn(void);

    testBasicDistribCrankAngle();
    testBasicDistribRevTime();
    testBasicDistribInitialise();
    testBasicDistribTriggers();
    testBasicDistribPerToothIgn();
}