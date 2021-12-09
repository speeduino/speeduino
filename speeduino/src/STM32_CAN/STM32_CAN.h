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

#if defined(STM32F407xx) || defined(STM32F1xx) || defined(STM32F405xx)
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

typedef enum CAN_PINS {DEF, ALT, ALT_2,} CAN_PINS;

//STM32 has only 3 TX mailboxes
typedef enum CAN_MAILBOX {
  MB0 = 0,
  MB1 = 1,
  MB2 = 2
} CAN_MAILBOX;

#ifndef CAN2
typedef enum CAN_CHANNEL {_CAN1,} CAN_CHANNEL;
#elif defined CAN2
typedef enum CAN_CHANNEL {_CAN1, _CAN2,} CAN_CHANNEL;
#endif

//Bit timings depend on the APB1 clock speed and need to be calculated based on that.
//APB1 at 42MHz:
#if defined(STM32F407xx) || defined(STM32F405xx)
CAN_bit_timing_config_t can_configs[6] = {{2, 12, 56}, {2, 12, 28}, {2, 13, 21}, {2, 11, 12}, {2, 11, 6}, {1, 5, 6}};
//APB1 at 36MHz
#elif defined(STM32F1xx)
CAN_bit_timing_config_t can_configs[6] = {{2, 13, 45}, {2, 15, 20}, {2, 13, 18}, {2, 13, 9}, {2, 15, 4}, {2, 15, 2}};
//APB1 at 45MHz
#elif defined(STM32F446xx)
CAN_bit_timing_config_t can_configs[6] = {{2, 12, 60}, {2, 12, 30}, {2, 12, 24}, {2, 12, 12}, {2, 12, 6}, {1, 7, 5}};
//If support for more APB1 clock speeds is needed, use this calculator: http://www.bittiming.can-wiki.info/
#endif

class STM32_CAN {
  const CAN_CHANNEL _channel;
  const CAN_PINS _pins;

  private: 
    void CANSetGpio(GPIO_TypeDef * addr, uint8_t index, uint8_t speed = 3);
    void CANSetFilter(uint8_t index, uint8_t scale, uint8_t mode, uint8_t fifo, uint32_t bank1, uint32_t bank2);
    void writeTxMailbox(uint8_t mb_num, CAN_message_t &CAN_tx_msg);
    uint8_t CANMsgAvail();
    void SetTXRX();

  public:
    STM32_CAN(const CAN_CHANNEL channel, CAN_PINS pins) : _channel (channel), _pins (pins) { };
    void begin();
    void setBaudRate(uint32_t baud);
    int write(CAN_message_t &CAN_tx_msg);  // use any available mailbox for transmitting
    int write(CAN_MAILBOX mb_num, CAN_message_t &CAN_tx_msg); // use a single mailbox for transmitting
    int read(CAN_message_t &CAN_rx_msg);
    void enableFIFO(bool status = 1);
};

#endif
#endif