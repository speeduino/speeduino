#if defined(STM32F407xx) || defined(STM32F103xB) || defined(STM32F405xx)
#include "STM32_CAN.h"

uint8_t STM32_CAN::CANMsgAvail()
{
  uint8_t msgAvail = CAN1->RF0R & 0x3UL; //Default value
  if (_channel == _CAN1) {
    // Check for pending FIFO 0 messages
    msgAvail = CAN1->RF0R & 0x3UL;
  }
  #if defined(CAN2)
  if (_channel == _CAN2) {
    // Check for pending FIFO 0 messages
    msgAvail = CAN2->RF0R & 0x3UL;
  }
  #endif
  return msgAvail;
}

void STM32_CAN::CANSetGpio(GPIO_TypeDef * addr, uint8_t index, uint8_t speed )
{
    #if defined(STM32F4xx)
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
    #endif
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
  volatile int timeout = 0;
  if (_channel == _CAN1)
  {
    // CAN1
    RCC->APB1ENR |= 0x2000000UL;           // Enable CAN1 clock 
  }
  #if defined(CAN2)
  else if (_channel == _CAN2)
  {
    // CAN2
    RCC->APB1ENR |= 0x4000000UL;           // Enable CAN2 clock
  }
  #endif
  SetTXRX();

  if (_channel == _CAN1)
  {
    CAN1->MCR |= 0x1UL;                                    // Require CAN1 to Initialization mode 
    while ((!(CAN1->MSR & 0x1UL)) && timeout++ < 100000);  // Wait for Initialization mode

    CAN1->MCR = 0x51UL;                    // Hardware initialization(No automatic retransmission)
    //CAN1->MCR = 0x41UL;                  // Hardware initialization(With automatic retransmission, not used because causes timer issues on speeduino)
  }
  #if defined(CAN2)
  else if (_channel == _CAN2)
  {
    // CAN2
    CAN2->MCR |= 0x1UL;                                    // Require CAN2 to Initialization mode
    while ((!(CAN2->MSR & 0x1UL)) && timeout++ < 100000);  // Wait for Initialization mode

    CAN2->MCR = 0x51UL;                    // Hardware initialization(No automatic retransmission)
    //CAN2->MCR = 0x41UL;                  // Hardware initialization(With automatic retransmission, not used because causes timer issues on speeduino)
  }
  #endif
}

void STM32_CAN::writeTxMailbox(uint8_t mb_num, CAN_message_t &CAN_tx_msg)
{
  uint32_t out = 0;
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

  if (_channel == _CAN1)
  {
    CAN1->sTxMailBox[mb_num].TDTR &= ~(0xF);
    CAN1->sTxMailBox[mb_num].TDTR |= CAN_tx_msg.len & 0xFUL;
    CAN1->sTxMailBox[mb_num].TDLR  = (((uint32_t) CAN_tx_msg.buf[3] << 24) |
                                      ((uint32_t) CAN_tx_msg.buf[2] << 16) |
                                      ((uint32_t) CAN_tx_msg.buf[1] <<  8) |
                                      ((uint32_t) CAN_tx_msg.buf[0]      ));
    CAN1->sTxMailBox[mb_num].TDHR  = (((uint32_t) CAN_tx_msg.buf[7] << 24) |
                                      ((uint32_t) CAN_tx_msg.buf[6] << 16) |
                                      ((uint32_t) CAN_tx_msg.buf[5] <<  8) |
                                      ((uint32_t) CAN_tx_msg.buf[4]      ));

    // Send Go
    CAN1->sTxMailBox[mb_num].TIR = out | STM32_CAN_TIR_TXRQ;
  }
  #if defined(CAN2)
  if (_channel == _CAN2)
  {
    CAN2->sTxMailBox[mb_num].TDTR &= ~(0xF);
    CAN2->sTxMailBox[mb_num].TDTR |= CAN_tx_msg.len & 0xFUL;
    CAN2->sTxMailBox[mb_num].TDLR  = (((uint32_t) CAN_tx_msg.buf[3] << 24) |
                                      ((uint32_t) CAN_tx_msg.buf[2] << 16) |
                                      ((uint32_t) CAN_tx_msg.buf[1] <<  8) |
                                      ((uint32_t) CAN_tx_msg.buf[0]      ));
    CAN2->sTxMailBox[mb_num].TDHR  = (((uint32_t) CAN_tx_msg.buf[7] << 24) |
                                      ((uint32_t) CAN_tx_msg.buf[6] << 16) |
                                      ((uint32_t) CAN_tx_msg.buf[5] <<  8) |
                                      ((uint32_t) CAN_tx_msg.buf[4]      ));
    // Send Go
    CAN2->sTxMailBox[mb_num].TIR = out | STM32_CAN_TIR_TXRQ;
  }
  #endif
}

int STM32_CAN::write(CAN_message_t &CAN_tx_msg)
{
  volatile int mailbox = 0;

  if (_channel == _CAN1)
  {
    // Check if one of the three mailboxes is empty
    while(CAN1->sTxMailBox[mailbox].TIR & 0x1UL &&  mailbox++ < 3);

    if (mailbox >= 3) {
      //Serial.println("transmit Fail");
      return -1; // transmit failed, no mailboxes available
    }
    else { // empty mailbox found, so it's ok to send new message
      writeTxMailbox(mailbox, CAN_tx_msg);
      return 1; // transmit done
    }
  }
  #if defined(CAN2)
  if (_channel == _CAN2)
  {
    // Check if one of the three mailboxes is empty
    while(CAN2->sTxMailBox[mailbox].TIR & 0x1UL &&  mailbox++ < 3);

    if (mailbox >= 3) {
      //Serial.println("transmit Fail");
      return -1; // transmit failed, no mailboxes available
    }
    else { // empty mailbox found, so it's ok to send new message
      writeTxMailbox(mailbox, CAN_tx_msg);
      return 1; // transmit done
    }
  }
  #endif
}

int STM32_CAN::write(CAN_MAILBOX mb_num, CAN_message_t &CAN_tx_msg)
{
  if (_channel == _CAN1)
  {
    if (CAN1->sTxMailBox[mb_num].TIR & 0x1UL) {
      //Serial.println("transmit Fail");
      return -1; // transmit failed, mailbox was not empty
    }
    else { // mailbox was empty, so it's ok to send new message
      writeTxMailbox(mb_num, CAN_tx_msg);
      return 1; // transmit done
    }
  }
  #if defined(CAN2)
  if (_channel == _CAN2)
  {
    if (CAN2->sTxMailBox[mb_num].TIR & 0x1UL) {
      //Serial.println("transmit Fail");
      return -1; // transmit failed, mailbox was not empty
    }
    else { // mailbox was empty, so it's ok to send new message
      writeTxMailbox(mb_num, CAN_tx_msg);
      return 1; // transmit done
    }
  }
  #endif
}

int STM32_CAN::read(CAN_message_t &CAN_rx_msg)
{
  if (CANMsgAvail()) {
    if(_channel == _CAN1) {
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
    #if defined(CAN2)
    if(_channel == _CAN2) {
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
    #endif
    return 1; // message read
  }
  else {
    return 0; // no messages available
  }
}

void STM32_CAN::setBaudRate(uint32_t baud)
{
  //there must be better way to do this, but for now it's what it is.
  int bitrate = 0; //Default to slowest baud rate
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
  if (_channel == _CAN1)
  {
    CAN1->BTR &= ~(((0x03) << 24) | ((0x07) << 20) | ((0x0F) << 16) | (0x1FF)); 
    CAN1->BTR |=  (((can_configs[bitrate].TS2-1) & 0x07) << 20) | (((can_configs[bitrate].TS1-1) & 0x0F) << 16) | ((can_configs[bitrate].BRP-1) & 0x1FF);
  }
  #if defined(CAN2)
  else if (_channel == _CAN2)
  {
    CAN2->BTR &= ~(((0x03) << 24) | ((0x07) << 20) | ((0x0F) << 16) | (0x1FF)); 
    CAN2->BTR |=  (((can_configs[bitrate].TS2-1) & 0x07) << 20) | (((can_configs[bitrate].TS1-1) & 0x0F) << 16) | ((can_configs[bitrate].BRP-1) & 0x1FF); 
  }
  #endif
  
    // Configure Filters to default values
    CAN1->FMR  |=   0x1UL;                 // Set to filter initialization mode
    CAN1->FMR  &= 0xFFFFC0FF;              // Clear CAN2 start bank

    // bxCAN has 28 filters.
    // These filters are used for both CAN1 and CAN2.
    #if defined(STM32F1xx)
    // STM32F1xx has only CAN1, so all 28 are used for CAN1
    CAN1->FMR  |= 0x1C << 8;              // Assign all filters to CAN1

    #elif defined(STM32F4xx)
    // STM32F4xx has CAN1 and CAN2, so CAN2 filters are offset by 14
    CAN1->FMR  |= 0xE00;                   // Start bank for the CAN2 interface
    #endif

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

  if (_channel == _CAN1)
  {
    CAN1->FMR   &= ~(0x1UL);               // Deactivate initialization mode
    CAN1->MCR   &= ~(0x1UL);               // Require CAN1 to normal mode 
  }
  #if defined(CAN2)
  else if (_channel == _CAN2)
  {
    CAN2->FMR   &= ~(0x1UL);               // Deactivate initialization mode
    CAN2->MCR   &= ~(0x1UL);               // Require CAN2 to normal mode  
  }
  #endif
}

void STM32_CAN::enableFIFO(bool status)
{
  //Nothing to do here. The FIFO is on by default.
}

void STM32_CAN::SetTXRX()
{
  if (_channel == _CAN1)  // CAN1
  {
    #if defined(STM32F4xx)
    //PD0/PD1 as default. PA11/PA12 is for USB so that can't be used if native USB connection is in use.
    if (_pins == DEF) {
      RCC->AHB1ENR |= 0x8;                 // Enable GPIOD clock 
      CANSetGpio(GPIOD, 1);                // Set PD1
      CANSetGpio(GPIOD, 0);                // Set PD0
    }
    //PB8/PB9 are alternative pins. 
    if (_pins == ALT) {
      RCC->AHB1ENR |= 0x2;                 // Enable GPIOB clock 
      CANSetGpio(GPIOB, 9);                // Set PB9
      CANSetGpio(GPIOB, 8);                // Set PB8
    }
    //PA11/PA12 are second alternative pins, but it can't be used if native USB connection is in use.
    if (_pins == ALT_2) {
      RCC->AHB1ENR |= 0x1;                 // Enable GPIOA clock 
      CANSetGpio(GPIOA, 12);               // Set PA12
      CANSetGpio(GPIOA, 11);               // Set PA11
    }
    #elif defined(STM32F1xx)
    RCC->APB2ENR |= 0x1UL;
    AFIO->MAPR   &= 0xFFFF9FFF;          // reset CAN remap
    //PA11/PA12 as default, because those are only ones available on all F1 models.
    if (_pins == DEF) {
      RCC->APB2ENR |= 0x4UL;           // Enable GPIOA clock
      AFIO->MAPR   &= 0xFFFF9FFF;          // reset CAN remap
                                           // CAN_RX mapped to PA11, CAN_TX mapped to PA12
      GPIOA->CRH   &= ~(0xFF000UL);        // Configure PA12(0b0000) and PA11(0b0000)
                                           // 0b0000
                                           //   MODE=00(Input mode)
                                           //   CNF=00(Analog mode)

      GPIOA->CRH   |= 0xB8FFFUL;           // Configure PA12(0b1011) and PA11(0b1000)
                                           // 0b1011
                                           //   MODE=11(Output mode, max speed 50 MHz) 
                                           //   CNF=10(Alternate function output Push-pull
                                           // 0b1000
                                           //   MODE=00(Input mode)
                                           //   CNF=10(Input with pull-up / pull-down)
                                     
      GPIOA->ODR |= 0x1UL << 12;           // PA12 Upll-up
    }
    //PB8/PB9 are alternative pins. 
    if (_pins == ALT) {
      AFIO->MAPR   |= 0x00004000;          // set CAN remap
                                           // CAN_RX mapped to PB8, CAN_TX mapped to PB9 (not available on 36-pin package)

      RCC->APB2ENR |= 0x8UL;               // Enable GPIOB clock
      GPIOB->CRH   &= ~(0xFFUL);           // Configure PB9(0b0000) and PB8(0b0000)
                                           // 0b0000
                                           //   MODE=00(Input mode)
                                           //   CNF=00(Analog mode)

      GPIOB->CRH   |= 0xB8UL;              // Configure PB9(0b1011) and PB8(0b1000)
                                           // 0b1011
                                           //   MODE=11(Output mode, max speed 50 MHz) 
                                           //   CNF=10(Alternate function output Push-pull
                                           // 0b1000
                                           //   MODE=00(Input mode)
                                           //   CNF=10(Input with pull-up / pull-down)
                                     
      GPIOB->ODR |= 0x1UL << 8;            // PB8 Upll-up
    }
    if (_pins == ALT_2) {
      AFIO->MAPR   |= 0x00005000;      // set CAN remap
                                       // CAN_RX mapped to PD0, CAN_TX mapped to PD1 (available on 100-pin and 144-pin package)

      RCC->APB2ENR |= 0x20UL;          // Enable GPIOD clock
      GPIOD->CRL   &= ~(0xFFUL);       // Configure PD1(0b0000) and PD0(0b0000)
                                       // 0b0000
                                       //   MODE=00(Input mode)
                                       //   CNF=00(Analog mode)

      GPIOD->CRH   |= 0xB8UL;          // Configure PD1(0b1011) and PD0(0b1000)
                                       // 0b1000
                                       //   MODE=00(Input mode)
                                       //   CNF=10(Input with pull-up / pull-down)
                                       // 0b1011
                                       //   MODE=11(Output mode, max speed 50 MHz) 
                                       //   CNF=10(Alternate function output Push-pull
                                     
      GPIOD->ODR |= 0x1UL << 0;        // PD0 Upll-up
    }
    #endif
  }
  #if defined(CAN2)
  else if (_channel == _CAN2)  // CAN2
  {
    //PB5/PB6 as default
    if (_pins == DEF) {
      RCC->AHB1ENR |= 0x2;                 // Enable GPIOB clock
      CANSetGpio(GPIOB, 6);                // Set PB6
      CANSetGpio(GPIOB, 5);                // Set PB5
    }
    //PB12/PB13 are alternative pins. 
    if (_pins == ALT) {
      RCC->AHB1ENR |= 0x2;                 // Enable GPIOB clock 
      CANSetGpio(GPIOB, 13);               // Set PB13
      CANSetGpio(GPIOB, 12);               // Set PB12
    }
  }
  #endif
}
#endif