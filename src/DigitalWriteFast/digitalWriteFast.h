/*
  Optimized digital functions for AVR microcontrollers
  by Watterott electronic (www.watterott.com)
  based on http://code.google.com/p/digitalwritefast
 */

#ifndef __digitalWriteFast_h_
#define __digitalWriteFast_h_ 1

#define ERROR_SEQUENCE 0b10101010 //digitalReadFast will return this value if pin number is not constant
// general macros/defines
#ifndef BIT_READ
# define BIT_READ(value, bit)            ((value) &   (1UL << (bit)))
#endif
#ifndef BIT_SET
# define BIT_SET(value, bit)             ((value) |=  (1UL << (bit)))
#endif
#ifndef BIT_CLEAR
# define BIT_CLEAR(value, bit)           ((value) &= ~(1UL << (bit)))
#endif
#ifndef BIT_WRITE
# define BIT_WRITE(value, bit, bitvalue) (bitvalue ? BIT_SET(value, bit) : BIT_CLEAR(value, bit))
#endif

#ifndef SWAP
#define SWAP(x,y) do{ (x)=(x)^(y); (y)=(x)^(y); (x)=(x)^(y); }while(0)
#endif

// workarounds for ARM microcontrollers
#if (!defined(__AVR__) || defined(ARDUINO_ARCH_SAM))
#ifndef PROGMEM
# define PROGMEM
#endif
#ifndef PGM_P
# define PGM_P const char *
#endif
#ifndef PSTR
# define PSTR(str) (str)
#endif

#ifndef memcpy_P
# define memcpy_P(dest, src, num) memcpy((dest), (src), (num))
#endif
#ifndef strcpy_P
# define strcpy_P(dst, src)       strcpy((dst), (src))
#endif
#ifndef strcat_P
# define strcat_P(dst, src)       strcat((dst), (src))
#endif
#ifndef strcmp_P
# define strcmp_P(a, b)           strcmp((a), (b))
#endif
#ifndef strcasecmp_P
# define strcasecmp_P(a, b)       strcasecmp((a), (b))
#endif
#ifndef strncmp_P
# define strncmp_P(a, b, n)       strncmp((a), (b), (n))
#endif
#ifndef strncasecmp_P
# define strncasecmp_P(a, b, n)   strncasecmp((a), (b), (n))
#endif
#ifndef strstr_P
# define strstr_P(a, b)           strstr((a), (b))
#endif
#ifndef strlen_P
# define strlen_P(a)              strlen((a))
#endif
#ifndef sprintf_P
# define sprintf_P(s, f, ...)     sprintf((s), (f), __VA_ARGS__)
#endif

#ifndef pgm_read_byte
# define pgm_read_byte(addr)      (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
# define pgm_read_word(addr)      (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
# define pgm_read_dword(addr)     (*(const unsigned long *)(addr))
#endif

#endif


// digital functions
// --- Arduino Mega ---
#if (defined(ARDUINO_AVR_MEGA) || \
       defined(ARDUINO_AVR_MEGA1280) || \
       defined(ARDUINO_AVR_MEGA2560) || \
       defined(__AVR_ATmega1280__) || \
       defined(__AVR_ATmega1281__) || \
       defined(__AVR_ATmega2560__) || \
       defined(__AVR_ATmega2561__))

#define UART_RX_PIN     (0) //PE0
#define UART_TX_PIN     (1) //PE1

#define I2C_SDA_PIN     (20)
#define I2C_SCL_PIN     (21)

#define SPI_HW_SS_PIN   (53) //PB0
#define SPI_HW_MOSI_PIN (51) //PB2
#define SPI_HW_MISO_PIN (50) //PB3
#define SPI_HW_SCK_PIN  (52) //PB1

#define __digitalPinToPortReg(P) \
(((P) >= 22 && (P) <= 29) ? &PORTA : \
((((P) >= 10 && (P) <= 13) || ((P) >= 50 && (P) <= 53)) ? &PORTB : \
(((P) >= 30 && (P) <= 37) ? &PORTC : \
((((P) >= 18 && (P) <= 21) || (P) == 38) ? &PORTD : \
((((P) >= 0 && (P) <= 3) || (P) == 5) ? &PORTE : \
(((P) >= 54 && (P) <= 61) ? &PORTF : \
((((P) >= 39 && (P) <= 41) || (P) == 4) ? &PORTG : \
((((P) >= 6 && (P) <= 9) || (P) == 16 || (P) == 17) ? &PORTH : \
(((P) == 14 || (P) == 15) ? &PORTJ : \
(((P) >= 62 && (P) <= 69) ? &PORTK : &PORTL))))))))))

#define __digitalPinToDDRReg(P) \
(((P) >= 22 && (P) <= 29) ? &DDRA : \
((((P) >= 10 && (P) <= 13) || ((P) >= 50 && (P) <= 53)) ? &DDRB : \
(((P) >= 30 && (P) <= 37) ? &DDRC : \
((((P) >= 18 && (P) <= 21) || (P) == 38) ? &DDRD : \
((((P) >= 0 && (P) <= 3) || (P) == 5) ? &DDRE : \
(((P) >= 54 && (P) <= 61) ? &DDRF : \
((((P) >= 39 && (P) <= 41) || (P) == 4) ? &DDRG : \
((((P) >= 6 && (P) <= 9) || (P) == 16 || (P) == 17) ? &DDRH : \
(((P) == 14 || (P) == 15) ? &DDRJ : \
(((P) >= 62 && (P) <= 69) ? &DDRK : &DDRL))))))))))

#define __digitalPinToPINReg(P) \
(((P) >= 22 && (P) <= 29) ? &PINA : \
((((P) >= 10 && (P) <= 13) || ((P) >= 50 && (P) <= 53)) ? &PINB : \
(((P) >= 30 && (P) <= 37) ? &PINC : \
((((P) >= 18 && (P) <= 21) || (P) == 38) ? &PIND : \
((((P) >= 0 && (P) <= 3) || (P) == 5) ? &PINE : \
(((P) >= 54 && (P) <= 61) ? &PINF : \
((((P) >= 39 && (P) <= 41) || (P) == 4) ? &PING : \
((((P) >= 6 && (P) <= 9) || (P) == 16 || (P) == 17) ? &PINH : \
(((P) == 14 || (P) == 15) ? &PINJ : \
(((P) >= 62 && (P) <= 69) ? &PINK : &PINL))))))))))

#define __digitalPinToBit(P) \
(((P) >=  7 && (P) <=  9) ? (P) - 3 : \
(((P) >= 10 && (P) <= 13) ? (P) - 6 : \
(((P) >= 22 && (P) <= 29) ? (P) - 22 : \
(((P) >= 30 && (P) <= 37) ? 37 - (P) : \
(((P) >= 39 && (P) <= 41) ? 41 - (P) : \
(((P) >= 42 && (P) <= 49) ? 49 - (P) : \
(((P) >= 50 && (P) <= 53) ? 53 - (P) : \
(((P) >= 54 && (P) <= 61) ? (P) - 54 : \
(((P) >= 62 && (P) <= 69) ? (P) - 62 : \
(((P) == 0 || (P) == 15 || (P) == 17 || (P) == 21) ? 0 : \
(((P) == 1 || (P) == 14 || (P) == 16 || (P) == 20) ? 1 : \
(((P) == 19) ? 2 : \
(((P) == 5 || (P) == 6 || (P) == 18) ? 3 : \
(((P) == 2) ? 4 : \
(((P) == 3 || (P) == 4) ? 5 : 7)))))))))))))))

// --- Other ---
#else

#define SPI_HW_SS_PIN   SS
#define SPI_HW_MOSI_PIN MOSI
#define SPI_HW_MISO_PIN MISO
#define SPI_HW_SCK_PIN  SCK

#endif
//#endif  //#ifndef digitalPinToPortReg


//ref: http://forum.arduino.cc/index.php?topic=140409.msg1054868#msg1054868
//void OutputsErrorIfCalled( void ) __attribute__ (( error( "Line: "__line__ "Variable used for digitalWriteFast") ));
void NonConstantUsed( void )  __attribute__ (( error("") )); 


#ifndef digitalWriteFast
#if (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
#define digitalWriteFast(P, V) \
if (__builtin_constant_p(P) && __builtin_constant_p(V)) { \
  BIT_WRITE(*__digitalPinToPortReg(P), __digitalPinToBit(P), (V)); \
} else { \
  NonConstantUsed(); \
}
#else
#define digitalWriteFast digitalWrite
#endif
#endif


#ifndef pinModeFast
#if (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
#define pinModeFast(P, V) \
if (__builtin_constant_p(P) && __builtin_constant_p(V)) { \
  BIT_WRITE(*__digitalPinToDDRReg(P), __digitalPinToBit(P), (V)); \
} else { \
  NonConstantUsed(); \
}
#else
#define pinModeFast pinMode
#endif
#endif


#ifndef digitalReadFast
#if (defined(__AVR__) || defined(ARDUINO_ARCH_AVR))
#define digitalReadFast(P) ( (byte) __digitalReadFast((P)) )
#define __digitalReadFast(P ) \
  (__builtin_constant_p(P) ) ? ( \
  ( BIT_READ(*__digitalPinToPINReg(P), __digitalPinToBit(P))) ) : \
  ERROR_SEQUENCE
#else
#define digitalReadFast digitalRead
#endif
#endif

#endif //__digitalWriteFast_h_

