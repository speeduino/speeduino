/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
can_comms was originally contributed by Darren Siepka
*/

/*
secondserial_command is called when a command is received from the secondary serial port
It parses the command and calls the relevant function.

can_command is called when a command is received by the onboard/attached canbus module
It parses the command and calls the relevant function.

sendcancommand is called when a command is to be sent either to serial3 
,to the external Can interface, or to the onboard/attached can interface
*/
#include "globals.h"
#include "comms_secondary.h"
#include "maths.h"
#include "errors.h"
#include "utilities.h"
#include "comms_legacy.h"
#include "logger.h"
#include "page_crc.h"

uint8_t currentSecondaryCommand;

#if ( defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) )
  HardwareSerial &secondarySerial = Serial3;
#elif defined(CORE_STM32)
  #ifndef HAVE_HWSERIAL2 //Hack to get the code to compile on BlackPills
    #define Serial2 Serial1
  #endif
  #if defined(STM32GENERIC) // STM32GENERIC core
    SerialUART &secondarySerial = Serial2;
  #else //libmaple core aka STM32DUINO
    HardwareSerial &secondarySerial = Serial2;
  #endif
#elif defined(CORE_TEENSY)
  HardwareSerial &secondarySerial = Serial2;
#endif

void secondserial_Command(void)
{
  #if defined(secondarySerial_AVAILABLE)
  if ( serialSecondaryStatusFlag == SERIAL_INACTIVE )  { currentSecondaryCommand = secondarySerial.read(); }

  switch (currentSecondaryCommand)
  {
    case 'A': 
      // sends a fixed 75 bytes of data. Used by Real Dash (Among others)
      if(configPage9.secondarySerialProtocol == SECONDARY_SERIAL_PROTO_GENERIC_FIXED) { sendValues(0, CAN_PACKET_SIZE, 0x31, secondarySerial, serialSecondaryStatusFlag, &getLegacySecondarySerialLogEntry); } // Send values using the legacy fixed byte order
      else { sendValues(0, CAN_PACKET_SIZE, 0x31, secondarySerial, serialSecondaryStatusFlag); } //send values to serial3 using the order in the ini file
      break;

    case 'b': // New EEPROM burn command to only burn a single page at a time
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 'B': // AS above but for the serial compatibility mode. 
      BIT_SET(currentStatus.status4, BIT_STATUS4_COMMS_COMPAT); //Force the compat mode
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 'd': // Send a CRC32 hash of a given page
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 'G': // this is the reply command sent by the Can interface
      serialSecondaryStatusFlag = SERIAL_COMMAND_INPROGRESS_LEGACY;
      byte destcaninchannel;
      if (secondarySerial.available() >= 9)
      {
        serialSecondaryStatusFlag = SERIAL_INACTIVE;
        uint8_t cmdSuccessful = secondarySerial.read();        //0 == fail,  1 == good.
        destcaninchannel = secondarySerial.read();  // the input channel that requested the data value
        if (cmdSuccessful != 0)
        {                                 // read all 8 bytes of data.
          uint8_t Gdata[9];
          uint8_t Glow, Ghigh;

          for (byte Gx = 0; Gx < 8; Gx++) // first two are the can address the data is from. next two are the can address the data is for.then next 1 or two bytes of data
          {
            Gdata[Gx] = secondarySerial.read();
          }
          Glow = Gdata[(configPage9.caninput_source_start_byte[destcaninchannel]&7)];
          if ((BIT_CHECK(configPage9.caninput_source_num_bytes,destcaninchannel) > 0))  //if true then num bytes is 2
          {
            if ((configPage9.caninput_source_start_byte[destcaninchannel]&7) < 8)   //you can't have a 2 byte value starting at byte 7(8 on the list)
            {
              Ghigh = Gdata[((configPage9.caninput_source_start_byte[destcaninchannel]&7)+1)];
            }
            else { Ghigh = 0; }
          }
        else
        {
          Ghigh = 0;
        }

        currentStatus.canin[destcaninchannel] = (Ghigh<<8) | Glow;
      }

        else{}  //continue as command request failed and/or data/device was not available

      }
      break;

    case 'k':   //placeholder for new can interface (toucan etc) commands

        break;
        
    case 'L':
    {
      //uint8_t Llength;
      while (secondarySerial.available() == 0) { }
      uint8_t canListen = secondarySerial.read();

      if (canListen == 0)
      {
        //command request failed and/or data/device was not available
        break;
      }

      while (secondarySerial.available() == 0) { }
      /*
      Unclear what the below is trying to achieve. Commenting out for now to avoid compiler warnings for unused variables
      Llength = secondarySerial.read();             // next the number of bytes expected value
      uint8_t Lbuffer[8];                     //8 byte buffer to store incoming can data

      for (uint8_t Lcount = 0; Lcount <Llength ;Lcount++)
      {
        while (secondarySerial.available() == 0){}
        // receive all x bytes into "Lbuffer"
        Lbuffer[Lcount] = secondarySerial.read();
      }
      */
      break;
    }
      
    case 'n': // sends the bytes of realtime values from the NEW CAN list
      //sendValues(0, NEW_CAN_PACKET_SIZE, 0x32, secondarySerial, serialSecondaryStatusFlag); //send values to serial3
      if(configPage9.secondarySerialProtocol == SECONDARY_SERIAL_PROTO_GENERIC_FIXED) { sendValues(0, NEW_CAN_PACKET_SIZE, 0x32, secondarySerial, serialSecondaryStatusFlag, &getLegacySecondarySerialLogEntry); } // Send values using the legacy fixed byte order
      else { sendValues(0, NEW_CAN_PACKET_SIZE, 0x32, secondarySerial, serialSecondaryStatusFlag); } //send values to serial3 using the order in the ini file
      break;

    case 'p':
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 'Q': // send code version
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
       break;

    case 'r': //New format for the optimised OutputChannels over CAN
      legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag);
      break;

    case 's': // send the "a" stream code version
      secondarySerial.print(F("Speeduino csx02019.8"));
      break;

    case 'S': // send code version
      if(configPage9.secondarySerialProtocol == SECONDARY_SERIAL_PROTO_MSDROID) { legacySerialHandler('Q', secondarySerial, serialSecondaryStatusFlag); } //Note 'Q', this is a workaround for msDroid
      else { legacySerialHandler(currentSecondaryCommand, secondarySerial, serialSecondaryStatusFlag); }
      
      break;

    case 'Z': //dev use
       break;

    default:
       break;
  }
  #endif
}

void can_Command(void)
{
 //int currentcanCommand = inMsg.id;
 #if defined (NATIVE_CAN_AVAILABLE)
      // currentStatus.canin[12] = (inMsg.id);
 if ( (inMsg.id == uint16_t(configPage9.obd_address + 0x100))  || (inMsg.id == 0x7DF))      
  {
    // The address is the speeduino specific ecu canbus address 
    // or the 0x7df(2015 dec) broadcast address
    if (inMsg.buf[1] == 0x01)
      {
        // PID mode 0 , realtime data stream
        obd_response(inMsg.buf[1], inMsg.buf[2], 0);     // get the obd response based on the data in byte2
        outMsg.id = (0x7E8);       //((configPage9.obd_address + 0x100)+ 8);  
        Can0.write(outMsg);       // send the 8 bytes of obd data   
      }
    if (inMsg.buf[1] == 0x22)
      {
        // PID mode 22h , custom mode , non standard data
        obd_response(inMsg.buf[1], inMsg.buf[2], inMsg.buf[3]);     // get the obd response based on the data in byte2
        outMsg.id = (0x7E8); //configPage9.obd_address+8);
        Can0.write(outMsg);       // send the 8 bytes of obd data
      }
  }
 if (inMsg.id == uint16_t(configPage9.obd_address + 0x100))      
  {
    // The address is only the speeduino specific ecu canbus address    
    if (inMsg.buf[1] == 0x09)
      {
       // PID mode 9 , vehicle information request
       if (inMsg.buf[2] == 02)
         {
          //send the VIN number , 17 char long VIN sent in 5 messages.
         }
      else if (inMsg.buf[2] == 0x0A)
         {
          //code 20: send 20 ascii characters with ECU name , "ECU -SpeeduinoXXXXXX" , change the XXXXXX ONLY as required.  
         }
      }
  }
#endif  
}  
    
// this routine sends a request(either "0" for a "G" , "1" for a "L" , "2" for a "R" to the Can interface or "3" sends the request via the actual local canbus
void sendCancommand(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress)
{
#if defined(secondarySerial_AVAILABLE)
    switch (cmdtype)
    {
      case 0:
        secondarySerial.print("G");
        secondarySerial.write(canaddress);  //tscanid of speeduino device
        secondarySerial.write(candata1);    // table id
        secondarySerial.write(candata2);    //table memory offset
        break;

      case 1:                      //send request to listen for a can message
        secondarySerial.print("L");
        secondarySerial.write(canaddress);  //11 bit canaddress of device to listen for
        break;

     case 2:                                          // requests via serial3
        secondarySerial.print("R");                         //send "R" to request data from the sourcecanAddress whose value is sent next
        secondarySerial.write(candata1);                    //the currentStatus.current_caninchannel
        secondarySerial.write(lowByte(sourcecanAddress) );       //send lsb first
        secondarySerial.write(highByte(sourcecanAddress) );
        break;

     case 3:
        //send to truecan send routine
        //canaddress == speeduino canid, candata1 == canin channel dest, paramgroup == can address  to request from
        //This section is to be moved to the correct can output routine later
        #if defined(NATIVE_CAN_AVAILABLE)
        outMsg.id = (canaddress);
        outMsg.len = 8;
        outMsg.buf[0] = 0x0B ;  //11;   
        outMsg.buf[1] = 0x15;
        outMsg.buf[2] = candata1;
        outMsg.buf[3] = 0x24;
        outMsg.buf[4] = 0x7F;
        outMsg.buf[5] = 0x70;
        outMsg.buf[6] = 0x9E;
        outMsg.buf[7] = 0x4D;
        Can0.write(outMsg);
        #endif
        break;

     default:
        break;
    }
#else
  UNUSED(cmdtype);
  UNUSED(canaddress);
  UNUSED(candata1);
  UNUSED(candata2);
  UNUSED(sourcecanAddress);
#endif
}

#if defined(NATIVE_CAN_AVAILABLE)
// This routine builds the realtime data into packets that the obd requesting device can understand. This is only used by teensy and stm32 with onboard canbus
void obd_response(uint8_t PIDmode, uint8_t requestedPIDlow, uint8_t requestedPIDhigh)
{ 
//only build the PID if the mcu has onboard/attached can 

  uint16_t obdcalcA;    //used in obd calcs
  uint16_t obdcalcB;    //used in obd calcs 
  uint16_t obdcalcC;    //used in obd calcs 
  uint16_t obdcalcD;    //used in obd calcs
  uint32_t obdcalcE32;    //used in calcs 
  uint32_t obdcalcF32;    //used in calcs 
  uint16_t obdcalcG16;    //used in calcs
  uint16_t obdcalcH16;    //used in calcs  

  outMsg.len = 8;
  
if (PIDmode == 0x01)
  {
     //currentStatus.canin[13] = therequestedPIDlow; 
   switch (requestedPIDlow)
         {
          case 0:       //PID-0x00 PIDs supported 01-20  
            outMsg.buf[0] =  0x06;    // sending 6 bytes
            outMsg.buf[1] =  0x41;    // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
            outMsg.buf[2] =  0x00;    // PID code
            outMsg.buf[3] =  0x08;   //B0000 1000   1-8
            outMsg.buf[4] =  B01111110;   //9-16
            outMsg.buf[5] =  B10100000;   //17-24
            outMsg.buf[6] =  B00010001;   //17-32
            outMsg.buf[7] =  B00000000;   
          break;

          case 5:      //PID-0x05 Engine coolant temperature , range is -40 to 215 deg C , formula == A-40
            outMsg.buf[0] =  0x03;                 // sending 3 bytes
            outMsg.buf[1] =  0x41;                 // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
            outMsg.buf[2] =  0x05;                 // pid code
            outMsg.buf[3] =  (byte)(currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);   //the data value A
            outMsg.buf[4] =  0x00;                 //the data value B which is 0 as unused
            outMsg.buf[5] =  0x00; 
            outMsg.buf[6] =  0x00; 
            outMsg.buf[7] =  0x00;
          break;

          case 10:        // PID-0x0A , Fuel Pressure (Gauge) , range is 0 to 765 kPa , formula == A / 3)
            uint16_t temp_fuelpressure;
            // Fuel pressure is in PSI. PSI to kPa is 6.89475729, but that needs to be divided by 3 for OBD2 formula. So 2.298.... 2.3 is close enough, so that in fraction.
            temp_fuelpressure = (currentStatus.fuelPressure * 23) / 10;
            outMsg.buf[0] =  0x03;    // sending 3 byte
            outMsg.buf[1] =  0x41;    // 
            outMsg.buf[2] =  0x0A;    // pid code
            outMsg.buf[3] =  lowByte(temp_fuelpressure);
            outMsg.buf[4] =  0x00;
            outMsg.buf[5] =  0x00; 
            outMsg.buf[6] =  0x00; 
            outMsg.buf[7] =  0x00;
          break;

          case 11:        // PID-0x0B , MAP , range is 0 to 255 kPa , Formula == A
            outMsg.buf[0] =  0x03;    // sending 3 byte
            outMsg.buf[1] =  0x41;    // 
            outMsg.buf[2] =  0x0B;    // pid code
            outMsg.buf[3] =  lowByte(currentStatus.MAP);    // absolute map
            outMsg.buf[4] =  0x00;
            outMsg.buf[5] =  0x00; 
            outMsg.buf[6] =  0x00; 
            outMsg.buf[7] =  0x00;
          break;

          case 12:        // PID-0x0C , RPM  , range is 0 to 16383.75 rpm , Formula == 256A+B / 4
            uint16_t temp_revs; 
            temp_revs = currentStatus.RPM << 2 ;      //
            outMsg.buf[0] = 0x04;                        // sending 4 byte
            outMsg.buf[1] = 0x41;                        // 
            outMsg.buf[2] = 0x0C;                        // pid code
            outMsg.buf[3] = highByte(temp_revs);         //obdcalcB; A
            outMsg.buf[4] = lowByte(temp_revs);          //obdcalcD; B
            outMsg.buf[5] = 0x00; 
            outMsg.buf[6] = 0x00; 
            outMsg.buf[7] = 0x00;
          break;

          case 13:        //PID-0x0D , Vehicle speed , range is 0 to 255 km/h , formula == A 
            outMsg.buf[0] =  0x03;                       // sending 3 bytes
            outMsg.buf[1] =  0x41;                       // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
            outMsg.buf[2] =  0x0D;                       // pid code
            outMsg.buf[3] =  lowByte(currentStatus.vss); // A
            outMsg.buf[4] =  0x00;                       // B
            outMsg.buf[5] =  0x00; 
            outMsg.buf[6] =  0x00; 
            outMsg.buf[7] =  0x00;
          break;

          case 14:      //PID-0x0E , Ignition Timing advance, range is -64 to 63.5 BTDC , formula == A/2 - 64 
            int8_t temp_timingadvance;
            temp_timingadvance = ((currentStatus.advance + 64) << 1);
            //obdcalcA = ((timingadvance + 64) <<1) ; //((timingadvance + 64) *2)
            outMsg.buf[0] =  0x03;                     // sending 3 bytes
            outMsg.buf[1] =  0x41;                     // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
            outMsg.buf[2] =  0x0E;                     // pid code
            outMsg.buf[3] =  temp_timingadvance;       // A
            outMsg.buf[4] =  0x00;                     // B
            outMsg.buf[5] =  0x00; 
            outMsg.buf[6] =  0x00; 
            outMsg.buf[7] =  0x00;
          break;

          case 15:      //PID-0x0F , Inlet air temperature , range is -40 to 215 deg C, formula == A-40 
            outMsg.buf[0] =  0x03;                                                         // sending 3 bytes
            outMsg.buf[1] =  0x41;                                                         // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
            outMsg.buf[2] =  0x0F;                                                         // pid code
            outMsg.buf[3] =  (byte)(currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET);   // A
            outMsg.buf[4] =  0x00;                                                         // B
            outMsg.buf[5] =  0x00; 
            outMsg.buf[6] =  0x00; 
            outMsg.buf[7] =  0x00;
         break;

         case 17:  // PID-0x11 , 
           // TPS percentage, range is 0 to 100 percent, formula == 100/256 A 
           uint16_t temp_tpsPC;
           temp_tpsPC = currentStatus.TPS;
           obdcalcA = (temp_tpsPC <<8) / 100;     // (tpsPC *256) /100;
           if (obdcalcA > 255){ obdcalcA = 255;}
           outMsg.buf[0] =  0x03;                    // sending 3 bytes
           outMsg.buf[1] =  0x41;                    // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
           outMsg.buf[2] =  0x11;                    // pid code
           outMsg.buf[3] =  obdcalcA;                // A
           outMsg.buf[4] =  0x00;                    // B
           outMsg.buf[5] =  0x00; 
           outMsg.buf[6] =  0x00; 
           outMsg.buf[7] =  0x00;
         break;
  
         case 19:      //PID-0x13 , oxygen sensors present, A0-A3 == bank1 , A4-A7 == bank2 , 
           uint16_t O2present;
           O2present = B00000011 ;       //realtimebufferA[24];         TEST VALUE !!!!!
           outMsg.buf[0] =  0x03;           // sending 3 bytes
           outMsg.buf[1] =  0x41;           // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
           outMsg.buf[2] =  0x13;           // pid code
           outMsg.buf[3] =  O2present ;     // A
           outMsg.buf[4] =  0x00;           // B
           outMsg.buf[5] =  0x00; 
           outMsg.buf[6] =  0x00; 
           outMsg.buf[7] =  0x00;
         break;

         case 28:      // PID-0x1C obd standard
           uint16_t obdstandard;
           obdstandard = 7;              // This is OBD2 / EOBD
           outMsg.buf[0] =  0x03;           // sending 3 bytes
           outMsg.buf[1] =  0x41;           // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
           outMsg.buf[2] =  0x1C;           // pid code
           outMsg.buf[3] =  obdstandard;    // A
           outMsg.buf[4] =  0x00;           // B
           outMsg.buf[5] =  0x00; 
           outMsg.buf[6] =  0x00; 
           outMsg.buf[7] =  0x00;
         break;
  
        case 32:      // PID-0x20 PIDs supported [21-40]
          outMsg.buf[0] =  0x06;          // sending 4 bytes
          outMsg.buf[1] =  0x41;          // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
          outMsg.buf[2] =  0x20;          // pid code
          outMsg.buf[3] =  B00011000;     // 33-40
          outMsg.buf[4] =  B00000000;     //41 - 48
          outMsg.buf[5] =  B00100000;     //49-56
          outMsg.buf[6] =  B00000001;     //57-64
          outMsg.buf[7] = 0x00;
        break;
   
        case 36:      // PID-0x24 O2 sensor2, AB: fuel/air equivalence ratio, CD: voltage ,  Formula == (2/65536)(256A +B) , 8/65536(256C+D) , Range is 0 to <2 and 0 to >8V 
          //uint16_t O2_1e ;
          //int16_t O2_1v ; 
          obdcalcH16 = configPage2.stoich/10 ;            // configPage2.stoich(is *10 so 14.7 is 147)
          obdcalcE32 = currentStatus.O2/10;            // afr(is *10 so 25.5 is 255) , needs a 32bit else will overflow
          obdcalcF32 = (obdcalcE32<<8) / obdcalcH16;      //this is same as (obdcalcE32/256) / obdcalcH16 . this calculates the ratio      
          obdcalcG16 = (obdcalcF32 *32768)>>8;          
          obdcalcA = highByte(obdcalcG16);
          obdcalcB = lowByte(obdcalcG16);       

          obdcalcF32 = currentStatus.O2ADC ;             //o2ADC is wideband volts to send *100    
          obdcalcG16 = (obdcalcF32 *20971)>>8;          
          obdcalcC = highByte(obdcalcG16);
          obdcalcD = lowByte(obdcalcG16);
    
          outMsg.buf[0] =  0x06;    // sending 4 bytes
          outMsg.buf[1] =  0x41;    // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
          outMsg.buf[2] =  0x24;    // pid code
          outMsg.buf[3] =  obdcalcA;   // A
          outMsg.buf[4] =  obdcalcB;   // B
          outMsg.buf[5] =  obdcalcC;   // C
          outMsg.buf[6] =  obdcalcD;   // D
          outMsg.buf[7] =  0x00;
        break;

        case 37:      //O2 sensor2, AB fuel/air equivalence ratio, CD voltage ,  2/65536(256A +B) ,8/65536(256C+D) , range is 0 to <2 and 0 to >8V
          //uint16_t O2_2e ;
          //int16_t O2_2V ; 
          obdcalcH16 = configPage2.stoich/10 ;            // configPage2.stoich(is *10 so 14.7 is 147)
          obdcalcE32 = currentStatus.O2_2/10;            // afr(is *10 so 25.5 is 255) , needs a 32bit else will overflow
          obdcalcF32 = (obdcalcE32<<8) / obdcalcH16;      //this is same as (obdcalcE32/256) / obdcalcH16 . this calculates the ratio      
          obdcalcG16 = (obdcalcF32 *32768)>>8;          
          obdcalcA = highByte(obdcalcG16);
          obdcalcB = lowByte(obdcalcG16);       

          obdcalcF32 = currentStatus.O2_2ADC ;             //o2_2ADC is wideband volts to send *100    
          obdcalcG16 = (obdcalcF32 *20971)>>8;          
          obdcalcC = highByte(obdcalcG16);
          obdcalcD = lowByte(obdcalcG16);
    
          outMsg.buf[0] =  0x06;    // sending 4 bytes
          outMsg.buf[1] =  0x41;    // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
          outMsg.buf[2] =  0x25;    // pid code
          outMsg.buf[3] =  obdcalcA;   // A
          outMsg.buf[4] =  obdcalcB;   // B
          outMsg.buf[5] =  obdcalcC;   // C
          outMsg.buf[6] =  obdcalcD;   // D 
          outMsg.buf[7] =  0x00;
        break;

        case 51:      //PID-0x33 Absolute Barometric pressure , range is 0 to 255 kPa , formula == A
          outMsg.buf[0] =  0x03;                  // sending 3 bytes
          outMsg.buf[1] =  0x41;                  // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
          outMsg.buf[2] =  0x33;                  // pid code
          outMsg.buf[3] =  currentStatus.baro ;   // A
          outMsg.buf[4] =  0x00;                  // B which is 0 as unused
          outMsg.buf[5] =  0x00; 
          outMsg.buf[6] =  0x00; 
          outMsg.buf[7] =  0x00;
        break;
   
        case 64:      // PIDs supported [41-60]  
          outMsg.buf[0] =  0x06;    // sending 4 bytes
          outMsg.buf[1] =  0x41;    // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
          outMsg.buf[2] =  0x40;    // pid code
          outMsg.buf[3] =  B01000100;    // 65-72dec
          outMsg.buf[4] =  B00000000;    // 73-80
          outMsg.buf[5] =  B01000000;   //  81-88
          outMsg.buf[6] =  B00010000;   //  89-96
          outMsg.buf[7] =  0x00;
        break;

        case 66:      //control module voltage, 256A+B / 1000 , range is 0 to 65.535v
          uint16_t temp_ecuBatt;
          temp_ecuBatt = currentStatus.battery10;   // create a 16bit temp variable to do the math
          obdcalcA = temp_ecuBatt*100;              // should be *1000 but ecuBatt is already *10
          outMsg.buf[0] =  0x04;                       // sending 4 bytes
          outMsg.buf[1] =  0x41;                       // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
          outMsg.buf[2] =  0x42;                       // pid code
          outMsg.buf[3] =  highByte(obdcalcA) ;        // A
          outMsg.buf[4] =  lowByte(obdcalcA) ;         // B
          outMsg.buf[5] =  0x00; 
          outMsg.buf[6] =  0x00; 
          outMsg.buf[7] =  0x00;
        break;

        case 70:        //PID-0x46 Ambient Air Temperature , range is -40 to 215 deg C , formula == A-40
          uint16_t temp_ambientair;
          temp_ambientair = 11;              // TEST VALUE !!!!!!!!!!
          obdcalcA = temp_ambientair + 40 ;    // maybe later will be (byte)(currentStatus.AAT + CALIBRATION_TEMPERATURE_OFFSET)
          outMsg.buf[0] =  0x03;             // sending 3 byte
          outMsg.buf[1] =  0x41;             // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
          outMsg.buf[2] =  0x46;             // pid code
          outMsg.buf[3] =  obdcalcA;         // A 
          outMsg.buf[4] =  0x00;
          outMsg.buf[5] =  0x00; 
          outMsg.buf[6] =  0x00; 
          outMsg.buf[7] =  0x00;
        break;

        case 82:        //PID-0x52 Ethanol fuel % , range is 0 to 100% , formula == (100/255)A
          outMsg.buf[0] =  0x03;                       // sending 3 byte
          outMsg.buf[1] =  0x41;                       // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc. 
          outMsg.buf[2] =  0x52;                       // pid code
          outMsg.buf[3] =  currentStatus.ethanolPct;   // A
          outMsg.buf[4] =  0x00;
          outMsg.buf[5] =  0x00; 
          outMsg.buf[6] =  0x00; 
          outMsg.buf[7] =  0x00;
        break;

        case 92:        //PID-0x5C Engine oil temperature , range is -40 to 210 deg C , formula == A-40
          uint16_t temp_engineoiltemp;
          temp_engineoiltemp = 40;              // TEST VALUE !!!!!!!!!! 
          obdcalcA = temp_engineoiltemp+40 ;    // maybe later will be (byte)(currentStatus.EOT + CALIBRATION_TEMPERATURE_OFFSET)
          outMsg.buf[0] =  0x03;                // sending 3 byte
          outMsg.buf[1] =  0x41;                // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc. 
          outMsg.buf[2] =  0x5C;                // pid code
          outMsg.buf[3] =  obdcalcA ;           // A
          outMsg.buf[4] =  0x00;
          outMsg.buf[5] =  0x00; 
          outMsg.buf[6] =  0x00; 
          outMsg.buf[7] =  0x00;
        break;

        case 96:       //PIDs supported [61-80]  
          outMsg.buf[0] =  0x06;    // sending 4 bytes
          outMsg.buf[1] =  0x41;    // Same as query, except that 40h is added to the mode value. So:41h = show current data ,42h = freeze frame ,etc.
          outMsg.buf[2] =  0x60;    // pid code
          outMsg.buf[3] =  0x00;    // B0000 0000
          outMsg.buf[4] =  0x00;    // B0000 0000
          outMsg.buf[5] =  0x00;    // B0000 0000
          outMsg.buf[6] =  0x00;    // B0000 0000
          outMsg.buf[7] =  0x00;
        break;

        default:
        break;
     }
    } 
  else if (PIDmode == 0x22)
    {
     // these are custom PID  not listed in the SAE std .
     if (requestedPIDhigh == 0x77)
       {
        if ((requestedPIDlow >= 0x01) && (requestedPIDlow <= 0x10))
             {   
                 // PID 0x01 (1 dec) to 0x10 (16 dec)
                 // Aux data / can data IN Channel 1 - 16  
                 outMsg.buf[0] =  0x06;                                               // sending 8 bytes
                 outMsg.buf[1] =  0x62;                                               // Same as query, except that 40h is added to the mode value. So:62h = custom mode
                 outMsg.buf[2] =  requestedPIDlow;                                 // PID code
                 outMsg.buf[3] =  0x77;                                               // PID code
                 outMsg.buf[4] =  lowByte(currentStatus.canin[requestedPIDlow]);   // A
                 outMsg.buf[5] =  highByte(currentStatus.canin[requestedPIDlow]);  // B
                 outMsg.buf[6] =  0x00;                                               // C
                 outMsg.buf[7] =  0x00;                                               // D
            }
       }
     // this allows to get any value out of current status array.
     else if (requestedPIDhigh == 0x78)
       {
          int16_t tempValue;
          tempValue = ProgrammableIOGetData(requestedPIDlow);
          outMsg.buf[0] =  0x06;                 // sending 6 bytes
          outMsg.buf[1] =  0x62;                 // Same as query, except that 40h is added to the mode value. So:62h = custom mode
          outMsg.buf[2] =  requestedPIDlow;      // PID code
          outMsg.buf[3] =  0x78;                 // PID code
          outMsg.buf[4] =  lowByte(tempValue);   // A
          outMsg.buf[5] =  highByte(tempValue);  // B
          outMsg.buf[6] =  0x00; 
          outMsg.buf[7] =  0x00;
      }
    }
}

void readAuxCanBus()
{
  for (int i = 0; i < 16; i++)
  {
    if (inMsg.id == (configPage9.caninput_source_can_address[i] + 0x100)) //Filters frame ID
    {

      if (!BIT_CHECK(configPage9.caninput_source_num_bytes, i))
      {
        // Gets the one-byte value from the Data Field.
        currentStatus.canin[i] = inMsg.buf[configPage9.caninput_source_start_byte[i]];
      }

      else
      {

        if (configPage9.caninputEndianess == 1)
        {
          //Gets the two-byte value from the Data Field in Litlle Endian.
          currentStatus.canin[i] = ((inMsg.buf[configPage9.caninput_source_start_byte[i]]) | (inMsg.buf[configPage9.caninput_source_start_byte[i] + 1] << 8));
        }
        else
        {
          //Gets the two-byte value from the Data Field in Big Endian.
          currentStatus.canin[i] = ((inMsg.buf[configPage9.caninput_source_start_byte[i]] << 8) | (inMsg.buf[configPage9.caninput_source_start_byte[i] + 1]));
        }
      }
    }
  }
}
#endif
