#pragma once

#include <iostream>

#include "avr/io.h"
#include "avr/interrupt.h"

#define abs(x) (x < 0 ? -x : x)
#define F (const char*)
#define PROGMEM

enum PIN_LEVEL {LOW, HIGH};
enum PIN_MODE {INPUT,OUTPUT,INPUT_PULLUP};
enum ANALOG_PIN {A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A13,A14,A15};
enum PIN_CHANGE {RISING,FALLING,CHANGE};

typedef unsigned char byte;
typedef char __FlashStringHelper;

void interrupts();
void noInterrupts();

void digitalWrite(...);
void analogWrite(...);
void pinMode(...);
void attachInterrupt(...);
void detachInterrupt(...);
void bitWrite(...);

int millis();
int micros();

int digitalRead(...);
int analogRead(...);
int map(...);
int bitRead(...);
int digitalPinToInterrupt(...);
int digitalPinToBitMask(...);
int word(int i, int j);
int lowByte(int i);
int highByte(int i);

volatile uint8_t* portInputRegister(...);
volatile uint8_t* portOutputRegister(...);
volatile uint8_t* digitalPinToPort(...);

void* malloc(int s);
void* realloc(void* p, int s);

struct div_result {int quot;};
// #ifndef CORE_MAIN
// div_result div(int x, int y);
// #endif
// div_result ldiv(int x, int y);

class HardwareSerial
{
public:
    void begin(...){}
    int write(...){return 0;}
    int read(...){return 0;}
    int print(...){return 0;}
    int println(...){return 0;}
    int available(){return 0;}
    void flush(...){}
    bool operator&& (bool) {return true;}
};
static HardwareSerial Serial;
static HardwareSerial Serial3;