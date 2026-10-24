#include "../test_utils.h"

void test4G63(void)
{
    extern void test4G63CrankAngle(void);
    extern void test4G63RevTime(void);

    test4G63CrankAngle();
    test4G63RevTime();
}