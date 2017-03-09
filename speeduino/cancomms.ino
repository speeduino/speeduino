/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
can_comms was originally contributed by Darren Siepka
*/

/*
can_command is called when a command is received over serial3 from the Can interface
It parses the command and calls the relevant function
sendcancommand is called when a comman d is to be sent via serial3 to the Can interface
*/

//#include "cancomms.h"
//#include "globals.h"
//#include "storage.h"

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)

void canCommand()
{
  switch (Serial3.read())
  {
    case 'A': // sends the bytes of realtime values
        sendValues(packetSize,3); //send values to serial3
        break;

    case 'G': // this is the reply command sent by the Can interface
        //uint8_t Gdata;
        while (Serial3.available() == 0) { }
        cancmdfail = Serial3.read();
        if (cancmdfail == 0)
        {
          //command request failed and/or data/device was not available
        }
        while (Serial3.available() == 0) { }
        //Gdata = Serial3.read();
        break;

    case 'L':
        uint8_t Llength;
        while (Serial3.available() == 0) { }
        canlisten = Serial3.read();
        if (canlisten == 0)
         {
          //command request failed and/or data/device was not available
          break;
         }
       while (Serial3.available() == 0) { }
       Llength= Serial3.read();              // next the number of bytes expected value
         for (uint8_t Lcount = 0; Lcount <Llength ;Lcount++)
                {
                  while (Serial3.available() == 0){}
                  // receive all x bytes into "Lbuffer"
                  Lbuffer[Lcount] = Serial3.read();
                }
       break;

    case 'S': // send code version
       for (unsigned int sig = 0; sig < sizeof(displaySignature) - 1; sig++){
           Serial3.write(displaySignature[sig]);
       }
       //Serial3.print("speeduino 201609-dev");
       break;

    case 'Q': // send code version
       for (unsigned int revn = 0; revn < sizeof( TSfirmwareVersion) - 1; revn++){
           Serial3.write( TSfirmwareVersion[revn]);
       }
       //Serial3.print("speeduino 201609-dev");
       break;

    default:
       break;
  }
}

// this routine sends a request(either "0" for a "G" or "1" for a "L" to the Can interface
void sendCancommand(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2)
{
    switch (cmdtype)
    {
      case 0:
        Serial3.print("G");
        Serial3.write(canaddress);  //tscanid of speeduino device
        Serial3.write(candata1);    // table id
        Serial3.write(candata2);    //table memory offset
     break;

     case 1:                      //send request to listen for a can message
        Serial3.print("L");
        Serial3.write(canaddress);  //11 bit canaddress of device to listen for
     break;
    }
}

#else
//Dummy functions for those that can't do Serial3
void canCommand() { return; }
void sendCancommand(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2) { return; }

#endif
