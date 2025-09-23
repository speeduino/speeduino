
#include "globals.h"

#if defined(NATIVE_CAN_AVAILABLE)
#include "aux_dbw.h"
#include "comms_CAN.h"

void initialiseDBW()
{

}

void dbwControl()
{
  
  //uint16_t targetThrottle = get3DTableValue(&dbwTable, currentStatus.TPS, currentStatus.RPM); //Lookup the target throttle plate opening
  //targetThrottle = map(targetThrottle, 0, 200, 0, configPage15.dbwTarget_max); //Map back to 0-1023 range

  uint16_t targetThrottle = 80;
  
  //Construct the CAN frame
  outMsg.len = 8;
  outMsg.id = 0x104; //Address used by both LDPerformance and DBWX2
  outMsg.buf[0] = lowByte(targetThrottle);
  outMsg.buf[1] = highByte(targetThrottle);
  outMsg.buf[2] = lowByte(currentStatus.RPM);  //lsb RPM
  outMsg.buf[3] = highByte(currentStatus.RPM); //msb RPM
  outMsg.buf[4] = currentStatus.gear;
  outMsg.buf[5] = 0x00;  //not used
  outMsg.buf[6] = 0x00;  //not used
  outMsg.buf[7] = 0x00;  //not used
  //Potential for a 2nd 8 byte frame for throttles 2-4, but not currently implemented
  CAN_write();
}

void dbwReceivePacket(uint16_t pedalPos, uint16_t throttlePos)
{
  currentStatus.TPS = map(pedalPos, 0, configPage15.dbwPedal_max, 0, 200); // Map the 0-1023 value to 0-100% with 0.5%
}

void dbwBlipThrottle(void)
{

}

#endif