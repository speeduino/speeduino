#include <Arduino.h>
#include <EEPROM.h>

void interrupts(){}
void noInterrupts(){}

void digitalWrite(...){}
void analogWrite(...){}
void pinMode(...){}
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

// div_result div(int x, int y){return {x/y};}
// div_result ldiv(int x, int y){return {x/y};}

volatile uint8_t TCNT2;
volatile uint8_t ADPS0;
volatile uint8_t ADPS1;
volatile uint8_t ADPS2;
volatile uint8_t ADCSRA;
volatile uint8_t OCIE1C;
volatile uint8_t OCIE1B;
volatile uint8_t OCIE1A;
volatile uint8_t TIMSK1;
volatile uint8_t TCNT1;
volatile uint8_t OCR1C;
volatile uint8_t OCR1B;
volatile uint8_t OCR1A;
volatile uint8_t OCIE4A;
volatile uint8_t OCIE4B;
volatile uint8_t OCIE4C;
volatile uint8_t TIMSK4;
volatile uint16_t OCR4A;
volatile uint16_t OCR4B;
volatile uint16_t OCR4C;
volatile uint16_t TCNT4;
volatile uint8_t OCIE5C;
volatile uint8_t OCIE5B;
volatile uint8_t OCIE5A;
volatile uint8_t TIMSK5;
volatile uint16_t TCNT5;
volatile uint16_t OCR5C;
volatile uint16_t OCR5B;
volatile uint16_t OCR5A;
volatile uint8_t OCIE3C;
volatile uint8_t OCIE3B;
volatile uint8_t OCIE3A;
volatile uint8_t TIMSK3;
volatile uint16_t OCR3C;
volatile uint16_t OCR3B;
volatile uint16_t OCR3A;
volatile uint16_t TCNT3;
volatile uint8_t TIFR5;
volatile uint8_t OCF5C;
volatile uint8_t OCF5B;
volatile uint8_t OCF5A;
volatile uint8_t OCF4A;
volatile uint8_t OCF4B;
volatile uint8_t OCF4C;
volatile uint8_t TOV4;
volatile uint8_t ICF4;
volatile uint8_t TIFR4;
volatile uint8_t TCCR4B;
volatile uint8_t TCCR4A;
volatile uint8_t ICF5;
volatile uint8_t TOV5;
volatile uint8_t CS11;
volatile uint8_t CS10;
volatile uint8_t TCCR5A;
volatile uint8_t TCCR5B;
volatile uint8_t ICF3;
volatile uint8_t TOV3;
volatile uint8_t OCF3C;
volatile uint8_t OCF3B;
volatile uint8_t OCF3A;
volatile uint8_t TIFR3;
volatile uint8_t TCCR3A;
volatile uint8_t TCCR3B;
volatile uint8_t TCCR3C;
volatile uint8_t TOV2;
volatile uint8_t OCF2B;
volatile uint8_t OCF2A;
volatile uint8_t CS21;
volatile uint8_t CS20;
volatile uint8_t CS22;
volatile uint8_t TIFR2;
volatile uint8_t TCCR2A;
volatile uint8_t TCCR2B;
volatile uint8_t TIMSK2;
volatile uint8_t ICF1;
volatile uint8_t TOV1;
volatile uint8_t OCF1C;
volatile uint8_t OCF1B;
volatile uint8_t OCF1A;
volatile uint8_t TCCR1A;
volatile uint8_t TCCR1B;
volatile uint8_t TIFR1;
volatile uint8_t CS12;

int __heap_start;
int *__brkval;

EEPROMClass EEPROM;