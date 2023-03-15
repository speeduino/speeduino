/** \file comms.h
 * @brief File for handling all serial requests 
 * @author Josh Stewart
 * 
 * This file contains all the functions associated with serial comms.
 * This includes sending of live data, sending/receiving current page data, sending CRC values of pages, receiving sensor calibration data etc
 * 
 */

#ifndef LEGACY_COMMS_H
#define LEGACY_COMMS_H

#ifndef DISABLE_LEGACY_COMMS

extern byte inProgressLength;

void legacySerialCommand(void);//This is the heart of the Command Line Interpreter.  All that needed to be done was to make it human readable.
void sendValuesLegacy(void);
void sendPage(void);
void sendPageASCII(void);
void receiveCalibration(byte tableID);
void testComm(void);
void sendToothLog_legacy(byte startOffset);
void sendCompositeLog_legacy(byte startOffset);

#endif // DISABLE_LEGACY_COMMS

#endif // LEGACY_COMMS_H
