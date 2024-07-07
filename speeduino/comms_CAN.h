#ifndef COMMS_CAN_H
#define COMMS_CAN_H
#if defined(NATIVE_CAN_AVAILABLE)

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

#define CAN_WBO_RUSEFI 1

#define TS_CAN_OFFSET 0x100

void initCAN();
int CAN_read();
void CAN_write();
void sendBMWCluster();
void sendVAGCluster();
void receiveCANwbo();
void DashMessages(uint16_t DashMessageID);
void can_Command(void);
void obd_response(uint8_t therequestedPID , uint8_t therequestedPIDlow, uint8_t therequestedPIDhigh);
void readAuxCanBus();

extern CAN_message_t outMsg;
extern CAN_message_t inMsg;

#endif
#endif // COMMS_CAN_H
