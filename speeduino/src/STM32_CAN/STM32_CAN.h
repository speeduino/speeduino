/*
This is CAN library for STM32 to be used in Speeduino engine management system by pazi88.
The library is created because at least currently (year 2020) there is no CAN library in the STM32 core.
This library is mostly based on the STM32 CAN examples by nopnop2002 and it has been combined with few
things from Teensy FlexCAN library to make it compatible with the CAN features that exist in speeduino for Teensy.
Link to the nopnop2002 repository:
https://github.com/nopnop2002/Arduino-STM32-CAN
*/
#ifndef STM32_CAN_H
#define STM32_CAN_H

#if defined(ARDUINO_ARCH_STM32)
#include <Arduino.h>

#define STM32_CAN_TIR_TXRQ              (1U << 0U)  // Bit 0: Transmit Mailbox Request
#define STM32_CAN_RIR_RTR               (1U << 1U)  // Bit 1: Remote Transmission Request
#define STM32_CAN_RIR_IDE               (1U << 2U)  // Bit 2: Identifier Extension
#define STM32_CAN_TIR_RTR               (1U << 1U)  // Bit 1: Remote Transmission Request
#define STM32_CAN_TIR_IDE               (1U << 2U)  // Bit 2: Identifier Extension

#define CAN_EXT_ID_MASK                 0x1FFFFFFFU
#define CAN_STD_ID_MASK                 0x000007FFU

// This struct is directly copied from Teensy FlexCAN library to retain compatibility with it. Not all are in use with STM32.
// Source: https://github.com/tonton81/FlexCAN_T4/

typedef struct CAN_message_t {
  uint32_t id = 0;         // can identifier
  uint16_t timestamp = 0;  // time when message arrived
  uint8_t idhit = 0;       // filter that id came from
  struct {
    bool extended = 0;     // identifier is extended (29-bit)
    bool remote = 0;       // remote transmission request packet type
    bool overrun = 0;      // message overrun
    bool reserved = 0;
  } flags;
  uint8_t len = 8;         // length of data
  uint8_t buf[8] = { 0 };  // data
  int8_t mb = 0;           // used to identify mailbox reception
  uint8_t bus = 1;         // used to identify where the message came from when events() is used. CAN(1) and CAN(2) in use
  bool seq = 0;            // sequential frames
} CAN_message_t;

typedef const struct
{
  uint8_t TS2;
  uint8_t TS1;
  uint8_t BRP;
} CAN_bit_timing_config_t;

typedef enum CAN_PINS {DEF, ALT, ALT2,} CAN_PINS;

#if defined(ARDUINO_BLACK_F407VE) || defined(ARDUINO_BLACK_F407VG) \
   || defined(ARDUINO_BLACK_F407ZE) || defined(ARDUINO_BLACK_F407ZG)
CAN_bit_timing_config_t can_configs[6] = {{2, 12, 56}, {2, 12, 28}, {2, 13, 21}, {2, 11, 12}, {2, 11, 6}, {1, 5, 6}};
#elif defined(ARDUINO_BLUEPILL_F103C8) || defined(ARDUINO_BLUEPILL_F103CB)
CAN_bit_timing_config_t can_configs[6] = {{2, 13, 45}, {2, 15, 20}, {2, 13, 18}, {2, 13, 9}, {2, 15, 4}, {2, 15, 2}};
#endif
//TODO: add bit timings for F401 and F411. The bit timings are clock frequency dependent
//also if support for other STM32 models is added, the bit timings need to be calculated for those too.

class STM32_CAN {
  private: 
    void CANSetGpio(GPIO_TypeDef * addr, uint8_t index, uint8_t speed = 3);
    void CANSetFilter(uint8_t index, uint8_t scale, uint8_t mode, uint8_t fifo, uint32_t bank1, uint32_t bank2);
    uint8_t CANMsgAvail();
   
  public:
    STM32_CAN();
    void begin();
    void setBaudRate(uint32_t baud);
    int write(CAN_message_t &CAN_tx_msg);
    int read(CAN_message_t &CAN_rx_msg);
    void enableFIFO(bool status = 1);
    void setTX(CAN_PINS pin = DEF);
    void setRX(CAN_PINS pin = DEF);
};

extern STM32_CAN Can0;

#endif
#endif