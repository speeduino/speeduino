

#ifndef RealdashCan_h
#define RealdashCan_h
#include "Arduino.h"


class RdCanSender
{
public:
  RdCanSender();

  void sendRdCanFrame(uint16_t id, uint16_t byte1, uint16_t byte2, uint16_t byte3, uint16_t byte4,uint16_t byte5, uint16_t byte6, uint16_t byte7, uint16_t byte8);

private:
  void SendCANFrameToSerial(unsigned long canFrameId, const byte *frameData);

  uint8_t frameData[8];
};

#endif