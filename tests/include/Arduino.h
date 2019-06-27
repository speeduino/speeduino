#pragma once

#include <iostream>
#include <string>
#include <list>

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
  std::list<char> input_buffer;
public:
  void put_char(char c)
  {
    input_buffer.push_back(c);
  }
  void begin(...)
  {
    // input_buffer.clear();
  }
  int write(byte data)
  {
    std::cout << data;
    return 1;
  }
  int write(byte* data, int length=1)
  {
    for(int i = 0; i < length; i++)
      write(data[i]);
    return length;
  }
  int write(const char* data, int length=1)
  {
    return write((byte*)data, length);
  }
  char read()
  {
    if(input_buffer.empty()) return 0;

    char c = input_buffer.front();
    input_buffer.pop_front();
    return c;
  }
  int print(std::string message = "")
  {
    std::cout << message;
    return message.size();
  }
  int print(int data)
  {
    std::cout << data;
    return 1;
  }
  int println(std::string message = "")
  {
    std::cout << message << std::endl;
    return message.size();
  }
  int println(int data)
  {
    std::cout << data << std::endl;
    return 1;
  }
  int available(){return input_buffer.size();}
  void flush(){}
  bool operator&& (bool) {return true;}
};
extern HardwareSerial Serial;
extern HardwareSerial Serial3;
