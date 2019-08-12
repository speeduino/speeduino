#include <iostream>
#include <map>

#include <Arduino.h>
#include <EEPROM.h>

bool hasInterrupts = false;
void interrupts()
{
    hasInterrupts = true;
}
void noInterrupts()
{
    hasInterrupts = false;
}

static std::map<unsigned, unsigned> digitalWriteStatus;
static std::map<unsigned, unsigned> analogWriteStatus;
static std::map<unsigned, unsigned> pinModeStatus;

void digitalWrite(unsigned pin, unsigned level)
{
    digitalWriteStatus[pin] = level;
}
void analogWrite(unsigned pin, unsigned level)
{
    analogWriteStatus[pin] = level;
}
void pinMode(unsigned pin, unsigned mode)
{
    pinModeStatus[pin] = mode;
}
void attachInterrupt(...){}
void detachInterrupt(...){}
void bitWrite(...){}

int millis(){return 1;}
int micros(){return 1;}

int digitalRead(...){return 1;}
int analogRead(...){return 1;}
int map(...){return 1;}
int bitRead(...){return 1;}
int digitalPinToInterrupt(...){return 1;}
int digitalPinToBitMask(...){return 1;}
int word(int i, int j){return i << 8 | j;}
int lowByte(int i){return i & 0xff;}
int highByte(int i){return (i >> 8) & 0xff;}

uint8_t my_int;

volatile uint8_t* portInputRegister(...){return &my_int;}
volatile uint8_t* portOutputRegister(...){return &my_int;}
volatile uint8_t* digitalPinToPort(...){return &my_int;}

void* malloc(int s)
{
    return new byte[s];
}
void* realloc(void* p, int s)
{
    byte* pb = (byte*)p;
    while(s--) *pb = 0;
    return pb;
}

uint8_t __avr_memory[512];

int __heap_start;
int *__brkval;

EEPROMClass EEPROM;
HardwareSerial Serial;
HardwareSerial Serial3;
