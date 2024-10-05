#ifndef COMMS_CAN_H
#define COMMS_CAN_H

//For BMW e46/e39/e38, rover and mini other CAN instrument clusters
#define CAN_BMW_ASC1 0x153 //Rx message from ACS unit that includes speed
#define CAN_BMW_DME1 0x316 //Tx message that includes RPM
#define CAN_BMW_DME2 0x329 //Tx message that includes CLT and TPS
#define CAN_BMW_DME4 0x545 //Tx message that includes CLT and TPS
#define CAN_BMW_ICL2 0x613
#define CAN_BMW_ICL3 0x615

//For VAG CAN instrument clusters
#define CAN_VAG_RPM 0x280
#define CAN_VAG_VSS 0x5A0

//For Haltech IC-7 and IC-10 digital dashes
#define CAN_HALTECH_DATA1   0x360 //RPM, MAP, TPS, Coolant Pressure. 50Hz
#define CAN_HALTECH_DATA2   0x361 //Fuel Pressure, Oil Pressure, Load, Wastegate Pressure. 50Hz
#define CAN_HALTECH_DATA3   0x362 //Advance, INJ Stage 1/2 duty cycles. 50Hz
#define CAN_HALTECH_PW      0x364 //Pulsewidth 1-4. 50Hz
#define CAN_HALTECH_LAMBDA  0x368 //Lambda 1-4. 20Hz
#define CAN_HALTECH_TRIGGER 0x369 //Trigger Counter, sync level, sync error count. 20Hz
#define CAN_HALTECH_VSS     0x370 //VSS, current gear and inlet cam angles. 20Hz
#define CAN_HALTECH_DATA4   0x372 //Baro, BatteryV, Target boost. 10Hz
#define CAN_HALTECH_DATA5   0x3E0 //IAT, CLT, Fuel Temp, Oil Temp. 10Hz

#define CAN_BROADCAST_PROTOCOL_OFF      0
#define CAN_BROADCAST_PROTOCOL_BMW      1
#define CAN_BROADCAST_PROTOCOL_VAG      2
#define CAN_BROADCAST_PROTOCOL_HALTECH  3


#define CAN_WBO_RUSEFI 1
#define CAN_WBO_AEM 2

#define TS_CAN_OFFSET 0x100

#if defined(NATIVE_CAN_AVAILABLE)

void initCAN();
int CAN_read();
void CAN_write();
void sendCANBroadcast(uint8_t);
void receiveCANwbo();
void DashMessages(uint16_t DashMessageID);
void can_Command(void);
void obd_response(uint8_t therequestedPID , uint8_t therequestedPIDlow, uint8_t therequestedPIDhigh);
void readAuxCanBus();

extern CAN_message_t outMsg;
extern CAN_message_t inMsg;

#endif
#endif // COMMS_CAN_H
