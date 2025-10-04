/** \file comms.h
 * @brief File for handling all serial requests 
 * @author Josh Stewart
 * 
 * This file contains all the functions associated with serial comms.
 * This includes sending of live data, sending/receiving current page data, sending CRC values of pages, receiving sensor calibration data etc
 * 
 */

#ifndef COMMS_LEGACY_H
#define COMMS_LEGACY_H

extern bool firstCommsRequest; /**< The number of times the A command has been issued. This is used to track whether a reset has recently been performed on the controller */
extern byte logItemsTransmitted;
extern byte inProgressLength;

void legacySerialCommand(commsInterface *); //Command parser
void legacySerialHandler(byte cmd, Stream &targetPort, SerialStatus &targetStatusFlag);
void sendValues(uint16_t offset, uint16_t packetLength, byte cmd, Stream &targetPort, SerialStatus &targetStatusFlag);
void sendValues(uint16_t offset, uint16_t packetLength, byte cmd, Stream &targetPort, SerialStatus &targetStatusFlag, uint8_t (*logFunction)(uint16_t));
void sendValuesLegacy(void);
void sendPage(void);
void sendPageASCII(void);
void receiveCalibration(byte tableID);
void testComm(void);
void sendToothLog_legacy(commsInterface *comms, byte startOffset);
void sendCompositeLog_legacy(commsInterface *comms, byte startOffset);

#endif // COMMS_LEGACY_H
