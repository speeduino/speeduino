#ifndef COMMS_H
#define COMMS_H
//These are the page numbers that the Tuner Studio serial protocol uses to transverse the different map and config pages.
#define veMapPage    1
#define veSetPage    2//Config Page 1
#define ignMapPage   3
#define ignSetPage   4//Config Page 2
#define afrMapPage   5
#define afrSetPage   6//Config Page 3
#define boostvvtPage 7
#define seqFuelPage  8
#define canbusPage   9//Config Page 9
#define warmupPage   10 //Config Page 10

#define SERIAL_PACKET_SIZE   90 //Must match ochBlockSize in ini file

byte currentPage = 1;//Not the same as the speeduino config page numbers
bool isMap = true;
unsigned long requestCount = 0; //The number of times the A command has been issued
byte currentCommand;
bool cmdPending = false;
bool chunkPending = false;
uint16_t chunkComplete = 0;
uint16_t chunkSize = 0;
int valueOffset; //cannot use offset as a variable name, it is a reserved word for several teensy libraries
byte cmdGroup = 0;
byte cmdValue = 0;
int cmdCombined = 0;  //the cmdgroup as high byte and cmdvalue as low byte
byte tsCanId = 0;     // current tscanid requested

const char pageTitles[] PROGMEM //This is being stored in the avr flash instead of SRAM which there is not very much of
  {
   "\nVE Map\0"//This is an alternative to using a 2D array which would waste space because of the different lengths of the strings
   "\nPg 1 Config\0"// 21-The configuration page titles' indexes are found by counting the chars
   "\nIgnition Map\0"//35-The map page titles' indexes are put into a var called currentTitleIndex. That represents the first char of each string.
   "\nPg 2 Config\0" //48
   "\nAFR Map\0" //56
   "\nPg 3 Config\0" //69
   "\nPg 4 Config\0" //82
   "\nBoost Map\0" //93
   "\nVVT Map\0"//102-No need to put a trailing null because it's the last string and the compliler does it for you.
   "\nPg 10 Config"
  };

void command();//This is the heart of the Command Line Interpeter.  All that needed to be done was to make it human readable.
void sendValues(uint16_t offset, uint16_t packetlength,byte cmd, byte portnum);
void receiveValue(int offset, byte newValue);
void saveConfig();
void sendPage(bool useChar);
void receiveCalibration(byte tableID);
void sendToothLog(bool useChar);
void testComm();
void commandButtons();

#endif // COMMS_H
