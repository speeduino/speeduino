#ifndef CANCOMMS_H
#define CANCOMMS_H
//These are the page numbers that the Tuner Studio serial protocol uses to transverse the different map and config pages.
#define veMapPage    1


uint8_t currentCanPage = 1;//Not the same as the speeduino config page numbers
uint8_t nCanretry = 0;      //no of retrys
uint8_t cancmdfail = 0;     //command fail yes/no 
uint8_t canlisten = 0;      
uint8_t Lbuffer[8];         //8 byte buffer to store incomng can data

  
void canCommand();//This is the heart of the Command Line Interpeter.  All that needed to be done was to make it human readable.
void sendCancommand(uint8_t cmdtype , uint16_t canadddress, uint8_t candata1, uint8_t candata2);
void testCanComm();

#endif // CANCOMMS_H
