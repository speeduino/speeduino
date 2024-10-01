#include <Arduino.h>
#include <unity.h>
#include <avr/sleep.h>

#define UNITY_EXCLUDE_DETAILS

extern void test_fastMap10Bit(void);
extern void test_map_sampling(void);

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
#if !defined(SIMULATOR)
    delay(2000);
#endif

    UNITY_BEGIN();    // IMPORTANT LINE!

    test_fastMap10Bit();
    test_map_sampling();
    
    UNITY_END(); // stop unit testing

#if defined(SIMULATOR)       // Tell SimAVR we are done
    cli();
    sleep_enable();
    sleep_cpu();
#endif   
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}