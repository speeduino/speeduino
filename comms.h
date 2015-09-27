#ifndef COMMS_H
#define COMMS_H

#define veMapPage    1
#define veSetPage    2
#define ignMapPage   3
#define ignSetPage   4
#define afrMapPage   5
#define afrSetPage   6
#define iacPage      7
#define boostvvtPage 8

byte currentPage;

void command();
void sendValues();
void receiveValue(int offset, byte newValue);
void saveConfig();
void sendPage();
void receiveCalibration(byte tableID);
void sendToothLog(bool useChar);
void testComm();

#endif // COMMS_H
