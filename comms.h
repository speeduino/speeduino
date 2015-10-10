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

byte currentPage = 1;
boolean isMap = true;
const char pageTitles[] PROGMEM
  {
   "\nVolumetric Efficiancy Map\0"
   "\nPage 1 Config\0"
   "\nIgnition Map\0"
   "\nPage 2 Config\0"
   "\nAir/Fuel Ratio Map\0"
   "\nPage 3 Config\0"
   "\nPage 4 Config"
  };
  
void command();
void sendValues();
void receiveValue(int offset, byte newValue);
void saveConfig();
void sendPage(bool useChar);
void receiveCalibration(byte tableID);
void sendToothLog(bool useChar);
void testComm();

#endif // COMMS_H
