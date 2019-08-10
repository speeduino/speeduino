#ifndef CANCOMMS_H
#define CANCOMMS_H

#define NEW_CAN_PACKET_SIZE   75
#define CAN_PACKET_SIZE   75

uint8_t currentcanCommand;
uint8_t currentCanPage = 1;//Not the same as the speeduino config page numbers
uint8_t nCanretry = 0;      //no of retrys
uint8_t cancmdfail = 0;     //command fail yes/no
uint8_t canlisten = 0;
uint8_t Lbuffer[8];         //8 byte buffer to store incomng can data
uint8_t Gdata[9];
uint8_t Glow, Ghigh;

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  HardwareSerial &CANSerial = Serial3;
#elif defined(CORE_STM32)
  #if defined(ARDUINO_ARCH_STM32) && !defined(_VARIANT_ARDUINO_STM32_) // STM32GENERIC core
    SerialUART &CANSerial = Serial2;
  #else //libmaple core aka STM32DUINO
    HardwareSerial &CANSerial = Serial2;
  #endif
#elif defined(CORE_TEENSY)
  HardwareSerial &CANSerial = Serial2;
#endif

void canCommand();//This is the heart of the Command Line Interpeter.  All that needed to be done was to make it human readable.
void sendcanValues(uint16_t offset, uint16_t packetLength, byte cmd, byte portNum);
void sendCancommand(uint8_t cmdtype , uint16_t canadddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress);

#endif // CANCOMMS_H
