#ifndef COMMS_H
#define COMMS_H

#define vePage    1
#define ignPage   2
#define afrPage   3
#define iacPage   4

byte currentPage;

void command();
void sendValues();
void receiveValue(byte offset, byte newValue);
void saveConfig();
void sendPage();
void receiveCalibration(byte tableID);
void sendToothLog(bool useChar);
void testComm();

#endif // COMMS_H
