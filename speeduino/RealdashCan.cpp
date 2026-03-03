 
#include "RealdashCan.h"



RdCanSender::RdCanSender()
{
}

 
void RdCanSender::sendRdCanFrame(uint16_t id, uint16_t byte1, uint16_t byte2, uint16_t byte3, uint16_t byte4,uint16_t byte5,uint16_t byte6,uint16_t byte7,uint16_t byte8)
   {
     memcpy(frameData, &byte1, 1);
     memcpy(frameData + 1, &byte2, 1);
     memcpy(frameData + 2, &byte3, 1);
     memcpy(frameData + 3, &byte4, 1);
     memcpy(frameData + 4, &byte5, 1);
     memcpy(frameData + 5, &byte6, 1);
     memcpy(frameData + 6, &byte7, 1);
     memcpy(frameData + 7, &byte8, 1);
     SendCANFrameToSerial(id, frameData);
   }

 void RdCanSender::SendCANFrameToSerial(unsigned long canFrameId, const byte *frameData)
   {
    
     { 
       // the 4 byte identifier at the beginning of each CAN frame
       // this is required for RealDash to 'catch-up' on ongoing stream of CAN frames
       const byte serialBlockTag[4] = {0x44, 0x33, 0x22, 0x11};
       Serial3.write(serialBlockTag, 4);       

       // the CAN frame id number (as 32bit little endian value)
       Serial3.write((const byte *)&canFrameId, 4);
              
       //  CAN frame payload
       Serial3.write(frameData, 8);
    
     }
   }
