#pragma once

#if defined(NATIVE_BOARD)

#include <unity.h>
#include <single_header/standalone/fakeit.hpp>
#include <sstream>
#include "../lib/ArduinoFake/FakeMega.h"

static std::stringstream serialBuffer;

// Unity required functions
void setUp(void) 
{
    fakeMega(serialBuffer, serialBuffer);
}
#if !defined(CUSTOM_TEARDOWN)
void tearDown(void) {}
#endif

// Below is to ensure setUp is called before main()
// If not, static variables in the firmware tests may use ArduinoFake before it 
// is configured: this will cause a segfault.
struct setup_static_initializer {
    setup_static_initializer() {
        setUp();
    }
} setup_static_initializer_instance __attribute__ ((init_priority (65534)));

int mainWrapper(int argc, char **argv, void (*testRunner)(void)) {
    try {
        UNITY_BEGIN();
        testRunner();
        UNITY_END();
    }
    catch (fakeit::FakeitException &err) {
        std::cerr << err;
    }
    catch (std::exception &err) {
        std::cerr << err.what();
    }
    catch (...) {
        std::cerr << "Unknown error occurred";
    }
    return 0;
}

#define TEST_HARNESS(testRunner) \
int main(int argc, char **argv) \
{ \
    return mainWrapper(argc, argv, testRunner); \
}

#endif