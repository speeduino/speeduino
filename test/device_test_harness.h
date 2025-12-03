#pragma once

#if !defined(NATIVE_BOARD)

#include <unity.h>
#include <Arduino.h>
#if defined(SIMULATOR)
#include <avr/sleep.h>
#endif

void setup(void (*runAllTests)(void))
{
    pinMode(LED_BUILTIN, OUTPUT);

    // Wait for Serial Monitor connection
    // Note: waiting on !Serial doesn't work on STM32
#if !defined(SIMULATOR)
    delay(5000);
#endif
    while (!Serial) {
        ; // Wait for serial connection
    }

    UNITY_BEGIN();    // IMPORTANT LINE!

    runAllTests();

    // A small delay here helps STM32
#if !defined(SIMULATOR)
    delay(500);
#endif

    UNITY_END(); // stop unit testing

#if defined(SIMULATOR)       // Tell SimAVR we are done
    cli();
    sleep_enable();
    sleep_cpu();
#endif     
}

#define DEVICE_TEST(testRunner) \
void setup() \
{ \
    setup(testRunner); \
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}
#else
#define DEVICE_TEST(testRunner)
#endif
