#include <Arduino.h>


#include "tests_init.h"
#include <unity.h>

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();    // IMPORTANT LINE!

    pinMode(LED_BUILTIN, OUTPUT);
    
}

uint8_t i = 0;
uint8_t max_blinks = 5;

void loop() 
{
  testInitialisation();

  UNITY_END(); // stop unit testing
}