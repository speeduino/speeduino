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

void Cancommand()
{
  switch (Serial3.read())
  {
    case 'A': // sends all the bytes of realtime values
      sendCanValues();
      break;

    case 'G': // this is the reply command sent by the Can interface
      uint8_t Gdata;
        while (Serial3.available() == 0) { }
        cancmdfail = Serial3.read();
        if (cancmdfail == 0)
         {
          //command request failed and/or data/device was not available
         }
        while (Serial3.available() == 0) { }
        Gdata= Serial3.read();
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
      Serial3.print("Speeduino 2016.09_canio");
      break;

    case 'Q': // send code version
      Serial3.print("speeduino 201609-dev_canio");
    break;

    default:
      break;
  }
}

/*
This function returns the current values of a fixed group of variables. if this list is changed so must the list in the Can interface to prevent errors
*/
void sendCanValues()
{
  uint8_t packetSize = 34;
  uint8_t response[packetSize];
Serial3.write("A");         //confirm cmd type
Serial3.write(packetSize);  //confirm no of byte to be sent
//now send the data
  response[0] = currentStatus.secl; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
  response[1] = currentStatus.squirt; //Squirt Bitfield
  response[2] = currentStatus.engine; //Engine Status Bitfield
  response[3] = (byte)(divu100(currentStatus.dwell)); //Dwell in ms * 10
  response[4] = (byte)(currentStatus.MAP >> 1); //map value is divided by 2
  response[5] = (byte)(currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET); //mat
  response[6] = (byte)(currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //Coolant ADC
  response[7] = currentStatus.tpsADC; //TPS (Raw 0-255)
  response[8] = currentStatus.battery10; //battery voltage
  response[9] = currentStatus.O2; //O2
  response[10] = currentStatus.egoCorrection; //Exhaust gas correction (%)
  response[11] = currentStatus.iatCorrection; //Air temperature Correction (%)
  response[12] = currentStatus.wueCorrection; //Warmup enrichment (%)
  response[13] = lowByte(currentStatus.RPM); //rpm HB
  response[14] = highByte(currentStatus.RPM); //rpm LB
  response[15] = currentStatus.TAEamount; //acceleration enrichment (%)
  response[16] = 0x00; //Barometer correction (%)
  response[17] = currentStatus.corrections; //Total GammaE (%)
  response[18] = currentStatus.VE; //Current VE 1 (%)
  response[19] = currentStatus.afrTarget;
  response[20] = (byte)(currentStatus.PW / 100); //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
  response[21] = currentStatus.tpsDOT; //TPS DOT
  response[22] = currentStatus.advance;
  response[23] = currentStatus.TPS; // TPS (0% to 100%)
  //Need to split the int loopsPerSecond value into 2 bytes
  response[24] = lowByte(currentStatus.loopsPerSecond);
  response[25] = highByte(currentStatus.loopsPerSecond);

  //The following can be used to show the amount of free memory
  currentStatus.freeRAM = freeRam();
  response[26] = lowByte(currentStatus.freeRAM); //(byte)((currentStatus.loopsPerSecond >> 8) & 0xFF);
  response[27] = highByte(currentStatus.freeRAM);

  response[28] = currentStatus.batCorrection; //Battery voltage correction (%)
  response[29] = currentStatus.spark; //Spark related bitfield
  response[30] = currentStatus.O2_2; //O2
  
  //rpmDOT must be sent as a signed integer
  response[31] = lowByte(currentStatus.rpmDOT);
  response[32] = highByte(currentStatus.rpmDOT);

  response[33] = currentStatus.flex; //Flex sensor value (or 0 if not used)
  
  Serial3.write(response, (size_t)packetSize);

  return;
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

