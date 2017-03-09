#ifndef COMMS_H
#define COMMS_H
//These are the page numbers that the Tuner Studio serial protocol uses to transverse the different map and config pages.
#define veMapPage    1
#define veSetPage    2//Config Page 1
#define ignMapPage   3
#define ignSetPage   4//Config Page 2
#define afrMapPage   5
#define afrSetPage   6//Config Page 3
#define iacPage      7//Config Page 4
#define boostvvtPage 8
#define seqFuelPage  9
#define canbusPage   10//Config Page 10

#define packetSize   38

byte currentPage = 1;//Not the same as the speeduino config page numbers
boolean isMap = true;
unsigned long requestCount = 0; //The number of times the A command has been issued

const char pageTitles[] PROGMEM //This is being stored in the avr flash instead of SRAM which there is not very much of
  {
   "\nVolumetric Efficiancy Map\0"//This is an alternative to using a 2D array which would waste space because of the different lengths of the strings
   "\nPage 1 Config\0"//The configuration page titles' indexes are found by counting the chars
   "\nIgnition Map\0"//The map page titles' indexes are put into a var called currentTitleIndex. That represents the first char of each string.
   "\nPage 2 Config\0"
   "\nAir/Fuel Ratio Map\0"
   "\nPage 3 Config\0"
   "\nPage 4 Config\0"
   "\nBoost Map\0"
   "\nVVT Map\0"//No need to put a trailing null because it's the last string and the compliler does it for you.
   "\nPage 10 Config"
  };

void command();//This is the heart of the Command Line Interpeter.  All that needed to be done was to make it human readable.
void sendValues(int packetlength, byte portnum);
void receiveValue(int offset, byte newValue);
void saveConfig();
void sendPage(bool useChar);
void receiveCalibration(byte tableID);
void sendToothLog(bool useChar);
void testComm();

#endif // COMMS_H
