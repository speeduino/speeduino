#pragma once

#if defined(NATIVE_BOARD)

#include <unity.h>
#include "../lib/ArduinoFake/FakeMega.h"

// Unity required functions
void setUp(void) 
{
    fakeMega();
}
void tearDown(void) {}

// Below is to ensure setUp is called before main()
// If not, static varaiables in the firmware tests may use ArduinoFake before it 
// is configured: this will cause a segfault.
struct setup_static_initializer {
    setup_static_initializer() {
        setUp();
    }
} setup_static_initializer_instance __attribute__ ((init_priority (65534)));

#define TEST_HARNESS(testRunner) \
int main(int argc, char **argv) \
{ \
    UNITY_BEGIN(); \
    testRunner(); \
    return UNITY_END(); \
}

#endif