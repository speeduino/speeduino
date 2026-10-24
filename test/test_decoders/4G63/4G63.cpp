#include "../test_utils.h"

void test4G63(void)
{
    extern void test4G63Initialise(void);
    extern void test4G63CrankAngle(void);
    extern void test4G63RevTime(void);
    extern void test4G63Triggers(void);
    extern void test4G63PerToothIgn(void);

    test4G63Initialise();
    test4G63CrankAngle();
    test4G63RevTime();
    test4G63Triggers();
    test4G63PerToothIgn();
}