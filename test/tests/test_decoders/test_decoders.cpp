
#include <Arduino.h>
#include <unity.h>

#include "test_decoders.h"
#include "missing_tooth/missing_tooth.h"

void testDecoders()
{
    RUN_TEST(test_missingtooth_newIgn_1);
}