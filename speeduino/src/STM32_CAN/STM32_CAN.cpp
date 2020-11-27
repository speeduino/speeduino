#include "STM32_CAN.h"

STM32_CAN::STM32_CAN(){
  RCC->APB1ENR |= 0x2000000UL;           // Enable CAN1 clock 
  // RCC->APB1ENR |= 0x4000000UL;           // Enable CAN2 clock
  //TODO: add way to use CAN2 too.
}

uint8_t STM32_CAN::CANMsgAvail()
{
  int channel = 1;
  if (channel == 1) {
    // Check for pending FIFO 0 messages
    return CAN1->RF0R & 0x3UL;
  } // end CAN1

  if (channel == 2) {
    // Check for pending FIFO 0 messages
    return CAN2->RF0R & 0x3UL;
  } // end CAN2
}

void STM32_CAN::CANSetGpio(GPIO_TypeDef * addr, uint8_t index, uint8_t speed )
{
    uint8_t _index2 = index * 2;
    uint8_t _index4 = index * 4;
    uint8_t ofs = 0;
    uint8_t setting;

    if (index > 7) {
      _index4 = (index - 8) * 4;
      ofs = 1;
    }

    uint32_t mask;
    mask = 0xF << _index4;
    addr->AFR[ofs]  &= ~mask;         // Reset alternate function
    setting = 0x9;                    // AF9
    mask = setting << _index4;
    addr->AFR[ofs]  |= mask;          // Set alternate function

    mask = 0x3 << _index2;
    addr->MODER   &= ~mask;           // Reset mode
    setting = 0x2;                    // Alternate function mode
    mask = setting << _index2;
    addr->MODER   |= mask;            // Set mode

    mask = 0x3 << _index2;
    addr->OSPEEDR &= ~mask;           // Reset speed
    setting = speed;
    mask = setting << _index2;
    addr->OSPEEDR |= mask;            // Set speed

    mask = 0x1 << index;
    addr->OTYPER  &= ~mask;           // Reset Output push-pull

    mask = 0x3 << _index2;
    addr->PUPDR   &= ~mask;           // Reset port pull-up/pull-down
}

void STM32_CAN::CANSetFilter(uint8_t index, uint8_t scale, uint8_t mode, uint8_t fifo, uint32_t bank1, uint32_t bank2) {
  if (index > 27) return;

  CAN1->FA1R &= ~(0x1UL<<index);               // Deactivate filter

  if (scale == 0) {
    CAN1->FS1R &= ~(0x1UL<<index);             // Set filter to Dual 16-bit scale configuration
  } else {
    CAN1->FS1R |= (0x1UL<<index);              // Set filter to single 32 bit configuration
  }
    if (mode == 0) {
    CAN1->FM1R &= ~(0x1UL<<index);             // Set filter to Mask mode
  } else {
    CAN1->FM1R |= (0x1UL<<index);              // Set filter to List mode
  }

  if (fifo == 0) {
    CAN1->FFA1R &= ~(0x1UL<<index);            // Set filter assigned to FIFO 0
  } else {
    CAN1->FFA1R |= (0x1UL<<index);             // Set filter assigned to FIFO 1
  }

  CAN1->sFilterRegister[index].FR1 = bank1;    // Set filter bank registers1
  CAN1->sFilterRegister[index].FR2 = bank2;    // Set filter bank registers2

  CAN1->FA1R |= (0x1UL<<index);                // Activate filter

}

void STM32_CAN::begin()
{
  int channel = 1;
  if (channel == 1)
  {
    // CAN1
    RCC->AHB1ENR |= 0x8;                 // Enable GPIOD clock 
    CANSetGpio(GPIOD, 0);                // Set PD0
    CANSetGpio(GPIOD, 1);                // Set PD1

    CAN1->MCR |= 0x1UL;                    // Require CAN1 to Initialization mode 
    while (!(CAN1->MSR & 0x1UL));          // Wait for Initialization mode

    //CAN1->MCR = 0x51UL;                  // Hardware initialization(No automatic retransmission)
    CAN1->MCR = 0x41UL;                    // Hardware initialization(With automatic retransmission)
  }
  else if (channel == 2)
  {
    // CAN2
    RCC->AHB1ENR |= 0x2;                 // Enable GPIOB clock 
    CANSetGpio(GPIOB, 12);               // Set PB12
    CANSetGpio(GPIOB, 13);               // Set PB13

    CAN2->MCR |= 0x1UL;                    // Require CAN2 to Initialization mode
    while (!(CAN2->MSR & 0x1UL));          // Wait for Initialization mode

    //CAN2->MCR = 0x51UL;                  // Hardware initialization(No automatic retransmission)
    CAN2->MCR = 0x41UL;                    // Hardware initialization(With automatic retransmission)
  }
}

int STM32_CAN::write(CAN_message_t &CAN_tx_msg)
{
  volatile int count = 0;
  uint32_t out = 0;
  int channel = 1; //temporary botch to use only CAN1. Eventually this needs to come from somewhere.

  if (CAN_tx_msg.flags.extended == true) { // Extended frame format
      out = ((CAN_tx_msg.id & CAN_EXT_ID_MASK) << 3U) | STM32_CAN_TIR_IDE;
  }
  else {                                    // Standard frame format
      out = ((CAN_tx_msg.id & CAN_STD_ID_MASK) << 21U);
  }
  // Remote frame
  if (CAN_tx_msg.flags.remote == true) {
      out |= STM32_CAN_TIR_RTR;
  }

  if (channel == 1)
  {
    CAN1->sTxMailBox[0].TDTR &= ~(0xF);
    CAN1->sTxMailBox[0].TDTR |= CAN_tx_msg.len & 0xFUL;
    CAN1->sTxMailBox[0].TDLR  = (((uint32_t) CAN_tx_msg.buf[3] << 24) |
                                 ((uint32_t) CAN_tx_msg.buf[2] << 16) |
                                 ((uint32_t) CAN_tx_msg.buf[1] <<  8) |
                                 ((uint32_t) CAN_tx_msg.buf[0]      ));
    CAN1->sTxMailBox[0].TDHR  = (((uint32_t) CAN_tx_msg.buf[7] << 24) |
                                 ((uint32_t) CAN_tx_msg.buf[6] << 16) |
                                 ((uint32_t) CAN_tx_msg.buf[5] <<  8) |
                                 ((uint32_t) CAN_tx_msg.buf[4]      ));

    // Send Go
    CAN1->sTxMailBox[0].TIR = out | STM32_CAN_TIR_TXRQ;

    // Wait until the mailbox is empty
    while(CAN1->sTxMailBox[0].TIR & 0x1UL && count++ < 1000000);

    if (CAN1->sTxMailBox[0].TIR & 0x1UL) {
      //Serial.println("transmit Fail");
      return -1; // transmit failed
    }
    else {
      return 1; // transmit ok
    }
  }
  if (channel == 2)
  {
    CAN2->sTxMailBox[0].TDTR &= ~(0xF);
    CAN2->sTxMailBox[0].TDTR |= CAN_tx_msg.len & 0xFUL;
    CAN2->sTxMailBox[0].TDLR  = (((uint32_t) CAN_tx_msg.buf[3] << 24) |
                                 ((uint32_t) CAN_tx_msg.buf[2] << 16) |
                                 ((uint32_t) CAN_tx_msg.buf[1] <<  8) |
                                 ((uint32_t) CAN_tx_msg.buf[0]      ));
    CAN2->sTxMailBox[0].TDHR  = (((uint32_t) CAN_tx_msg.buf[7] << 24) |
                                 ((uint32_t) CAN_tx_msg.buf[6] << 16) |
                                 ((uint32_t) CAN_tx_msg.buf[5] <<  8) |
                                 ((uint32_t) CAN_tx_msg.buf[4]      ));

    // Send Go
    CAN2->sTxMailBox[0].TIR = out | STM32_CAN_TIR_TXRQ;

    // Wait until the mailbox is empty
    while(CAN2->sTxMailBox[0].TIR & 0x1UL && count++ < 1000000);

    if (CAN2->sTxMailBox[0].TIR & 0x1UL) {
      //Serial.println("transmit Fail");
      return -1; // transmit failed
    }
    else {
      return 1; // transmit ok
    }
  }
}

int STM32_CAN::read(CAN_message_t &CAN_rx_msg)
{
  int channel = 1; //temporary botch to use only CAN1. Eventually this needs to come from somewhere.
  if (CANMsgAvail()) {
    if(channel == 1) {
      uint32_t id = CAN1->sFIFOMailBox[0].RIR;
      if ((id & STM32_CAN_RIR_IDE) == 0) { // Standard frame format
          CAN_rx_msg.flags.extended = false;
          CAN_rx_msg.id = (CAN_STD_ID_MASK & (id >> 21U));
      } 
      else {                               // Extended frame format
          CAN_rx_msg.flags.extended = true;
          CAN_rx_msg.id = (CAN_EXT_ID_MASK & (id >> 3U));
      }

      if ((id & STM32_CAN_RIR_RTR) == 0) {  // Data frame
          CAN_rx_msg.flags.remote = false;
      }
      else {                                // Remote frame
          CAN_rx_msg.flags.remote = true;
      }

      CAN_rx_msg.len = (CAN1->sFIFOMailBox[0].RDTR) & 0xFUL;

      CAN_rx_msg.buf[0] = 0xFFUL &  CAN1->sFIFOMailBox[0].RDLR;
      CAN_rx_msg.buf[1] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 8);
      CAN_rx_msg.buf[2] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 16);
      CAN_rx_msg.buf[3] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 24);
      CAN_rx_msg.buf[4] = 0xFFUL &  CAN1->sFIFOMailBox[0].RDHR;
      CAN_rx_msg.buf[5] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 8);
      CAN_rx_msg.buf[6] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 16);
      CAN_rx_msg.buf[7] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 24);

      CAN_rx_msg.bus = 1;

      CAN1->RF0R |= 0x20UL;
    } // end CAN1

    if(channel == 2) {
      uint32_t id = CAN2->sFIFOMailBox[0].RIR;
      if ((id & STM32_CAN_RIR_IDE) == 0) { // Standard frame format
          CAN_rx_msg.flags.extended = false;
          CAN_rx_msg.id = (CAN_STD_ID_MASK & (id >> 21U));
      }
      else {                               // Extended frame format
          CAN_rx_msg.flags.extended = true;
          CAN_rx_msg.id = (CAN_EXT_ID_MASK & (id >> 3U));
      }

      if ((id & STM32_CAN_RIR_RTR) == 0) {  // Data frame
          CAN_rx_msg.flags.remote = false;
      }
      else {                                // Remote frame
          CAN_rx_msg.flags.remote = true;
      }

      CAN_rx_msg.len = (CAN2->sFIFOMailBox[0].RDTR) & 0xFUL;
      
      CAN_rx_msg.buf[0] = 0xFFUL &  CAN2->sFIFOMailBox[0].RDLR;
      CAN_rx_msg.buf[1] = 0xFFUL & (CAN2->sFIFOMailBox[0].RDLR >> 8);
      CAN_rx_msg.buf[2] = 0xFFUL & (CAN2->sFIFOMailBox[0].RDLR >> 16);
      CAN_rx_msg.buf[3] = 0xFFUL & (CAN2->sFIFOMailBox[0].RDLR >> 24);
      CAN_rx_msg.buf[4] = 0xFFUL &  CAN2->sFIFOMailBox[0].RDHR;
      CAN_rx_msg.buf[5] = 0xFFUL & (CAN2->sFIFOMailBox[0].RDHR >> 8);
      CAN_rx_msg.buf[6] = 0xFFUL & (CAN2->sFIFOMailBox[0].RDHR >> 16);
      CAN_rx_msg.buf[7] = 0xFFUL & (CAN2->sFIFOMailBox[0].RDHR >> 24);

      CAN_rx_msg.bus = 2;

      CAN2->RF0R |= 0x20UL;
    } // END CAN2
    return 1; // message read
  }
  else {
    return 0; // no messages available
  }
}

void STM32_CAN::setBaudRate(uint32_t baud)
{
  int channel = 1; //temporary botch to use only CAN1. Eventually this needs to come from somewhere.

  //there must be better way to do this, but for now it's what it is.
  int bitrate;
  switch(baud)
    {
      case 50000:
        bitrate = 0;
        break;
      case 100000:
        bitrate = 1;
        break;
      case 125000:
        bitrate = 2;
        break;
      case 250000:
        bitrate = 3;
        break;
      case 500000:
         bitrate = 4;
        break;
      case 1000000:
         bitrate = 5;
        break;
      default:
        //Other baud rates aren't supported
        break;
    }
  // Set bit rates 
  if (channel == 1)
  {
    CAN1->BTR &= ~(((0x03) << 24) | ((0x07) << 20) | ((0x0F) << 16) | (0x1FF)); 
    CAN1->BTR |=  (((can_configs[bitrate].TS2-1) & 0x07) << 20) | (((can_configs[bitrate].TS1-1) & 0x0F) << 16) | ((can_configs[bitrate].BRP-1) & 0x1FF);

    // Configure Filters to default values
    CAN1->FMR  |=   0x1UL;                 // Set to filter initialization mode
    CAN1->FMR  &= 0xFFFFC0FF;              // Clear CAN2 start bank

    // bxCAN has 28 filters.
    // These filters are used for both CAN1 and CAN2.
    // STM32F405 has CAN1 and CAN2, so CAN2 filters are offset by 14
    CAN1->FMR  |= 0xE00;                   // Start bank for the CAN2 interface

    // Set filter 0
    // Single 32-bit scale configuration 
    // Two 32-bit registers of filter bank x are in Identifier Mask mode
    // Filter assigned to FIFO 0 
    // Filter bank register to all 0
    CANSetFilter(0, 1, 0, 0, 0x0UL, 0x0UL); 

    // Set filter 14
    // Single 32-bit scale configuration 
    // Two 32-bit registers of filter bank x are in Identifier Mask mode
    // Filter assigned to FIFO 0 
    // Filter bank register to all 0
    CANSetFilter(14, 1, 0, 0, 0x0UL, 0x0UL); 

    CAN1->FMR   &= ~(0x1UL);               // Deactivate initialization mode

    uint16_t TimeoutMilliseconds = 1000;
    bool can1 = false;
    CAN1->MCR   &= ~(0x1UL);               // Require CAN1 to normal mode 

    // Wait for normal mode
    // If the connection is not correct, it will not return to normal mode.
    for (uint16_t wait_ack = 0; wait_ack < TimeoutMilliseconds; wait_ack++) {
      if ((CAN1->MSR & 0x1UL) == 0) {
        can1 = true;
        break;
      }
      delayMicroseconds(1000);
    }
    if (can1) {
      //Serial.println("CAN1 initialize ok");
    } else {
      //Serial.println("CAN1 initialize fail!!");
    }
  }
  else if (channel == 2)
  {
    CAN2->BTR &= ~(((0x03) << 24) | ((0x07) << 20) | ((0x0F) << 16) | (0x1FF)); 
    CAN2->BTR |=  (((can_configs[bitrate].TS2-1) & 0x07) << 20) | (((can_configs[bitrate].TS1-1) & 0x0F) << 16) | ((can_configs[bitrate].BRP-1) & 0x1FF); 
	// Configure Filters to default values
    CAN2->FMR  |=   0x1UL;                 // Set to filter initialization mode
    CAN2->FMR  &= 0xFFFFC0FF;              // Clear CAN2 start bank

    // bxCAN has 28 filters.
    // These filters are used for both CAN1 and CAN2.
    // STM32F405 has CAN1 and CAN2, so CAN2 filters are offset by 14
    CAN2->FMR  |= 0xE00;                   // Start bank for the CAN2 interface

    // Set filter 0
    // Single 32-bit scale configuration 
    // Two 32-bit registers of filter bank x are in Identifier Mask mode
    // Filter assigned to FIFO 0 
    // Filter bank register to all 0
    CANSetFilter(0, 1, 0, 0, 0x0UL, 0x0UL); 

    // Set filter 14
    // Single 32-bit scale configuration 
    // Two 32-bit registers of filter bank x are in Identifier Mask mode
    // Filter assigned to FIFO 0 
    // Filter bank register to all 0
    CANSetFilter(14, 1, 0, 0, 0x0UL, 0x0UL);

    CAN2->FMR   &= ~(0x1UL);               // Deactivate initialization mode

    uint16_t TimeoutMilliseconds = 1000;
    bool can2 = false;
    CAN2->MCR   &= ~(0x1UL);               // Require CAN2 to normal mode  

    // Wait for normal mode
    // If the connection is not correct, it will not return to normal mode.
    for (uint16_t wait_ack = 0; wait_ack < TimeoutMilliseconds; wait_ack++) {
      if ((CAN2->MSR & 0x1UL) == 0) {
        can2 = true;
        break;
      }
      delayMicroseconds(1000);
    }
    //Serial.print("can2=");
    //Serial.println(can2);
    if (can2) {
      //Serial.println("CAN2 initialize ok");
    } else {
      //Serial.println("CAN2 initialize fail!!");
    }
  }
}

void STM32_CAN::enableFIFO(bool status)
{
  //Nothing to do here. The FIFO is on by default.
}

void STM32_CAN::setTX(CAN_PINS pin)
{
  //PD1 as default. PA12 is for USB so that can't be used if native USB connection is in use.
  if (pin == DEF) {
    // CAN1
    RCC->AHB1ENR |= 0x8;                 // Enable GPIOD clock 
    CANSetGpio(GPIOD, 1);                // Set PD1
  }
  //PB9 is alternative TX pin. 
  if (pin == ALT) {
    // CAN1
    RCC->AHB1ENR |= 0x2;                 // Enable GPIOB clock 
    CANSetGpio(GPIOB, 9);                // Set PB9
  }
  //PA12 is second alternative TX pin, but it can't be used if native USB connection is in use.
  if (pin == ALT2) {
    // CAN1
    RCC->AHB1ENR |= 0x1;                 // Enable GPIOA clock 
    CANSetGpio(GPIOA, 12);               // Set PA12
  }

}

void STM32_CAN::setRX(CAN_PINS pin)
{
  //PD0 as default. PA11 is for USB so that can't be used if native USB connection is in use.
  if (pin == DEF) {
    // CAN1
    RCC->AHB1ENR |= 0x8;                 // Enable GPIOD clock 
    CANSetGpio(GPIOD, 0);                // Set PD0
  }
  //PB8 is alternative RX pin. 
  if (pin == ALT) {
    // CAN1
    RCC->AHB1ENR |= 0x2;                 // Enable GPIOB clock 
    CANSetGpio(GPIOB, 8);                // Set PB8
  }
  //PA11 is second alternative RX pin, but it can't be used if native USB connection is in use.
  if (pin == ALT2) {
    // CAN1
    RCC->AHB1ENR |= 0x1;                 // Enable GPIOA clock 
    CANSetGpio(GPIOA, 11);               // Set PA11
  }
}