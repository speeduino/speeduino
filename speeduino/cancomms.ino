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

void canCommand()
{
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)  
  currentcanCommand = Serial3.read();
#elif defined(CORE_STM32)
  currentcanCommand = Serial2.read();
#elif defined(CORE_TEENSY)
  currentcanCommand = Serial2.read();
#else return;  
#endif  

  switch (currentcanCommand)
  {
    case 'A': // sends the bytes of realtime values
        sendValues(0, packetSize,3); //send values to serial3
        break;

    case 'G': // this is the reply command sent by the Can interface
        //uint8_t Gdata;
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)       
        while (Serial3.available() == 0) { }
        cancmdfail = Serial3.read();
#elif defined(CORE_STM32)
        while (Serial2.available() == 0) { }
        cancmdfail = Serial2.read();
#elif defined(CORE_TEENSY)
        while (Serial2.available() == 0) { }
        cancmdfail = Serial2.read();
#else return;        
#endif
        if (cancmdfail != 0)
        {
          for (byte Gx = 0; Gx < 8; Gx++) //read all 8 bytes of data
            {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)             
              while (Serial3.available() == 0) { }
              Gdata[Gx] = Serial3.read();
#elif defined(CORE_STM32)
              while (Serial2.available() == 0) { }
              Gdata[Gx] = Serial2.read();
#elif defined(CORE_TEENSY)
              while (Serial2.available() == 0) { }
              Gdata[Gx] = Serial2.read();
#else return;              
#endif
            }

          Glow = Gdata[(configPage10.caninput_param_start_byte[currentStatus.current_caninchannel])];
          if (configPage10.caninput_param_num_bytes[currentStatus.current_caninchannel] == 2)
             {
              if ((configPage10.caninput_param_start_byte[currentStatus.current_caninchannel]) != 7)   //you cant have a 2 byte value starting at byte 7(8 on the list)
                 {
                  Ghigh = Gdata[((configPage10.caninput_param_start_byte[currentStatus.current_caninchannel])+1)];
                 }
             }
          else
             {
              Ghigh = 0;     
             }
                
          currentStatus.canin[currentStatus.current_caninchannel] = word(Ghigh, Glow);   
        }
        
        else{}  //continue as command request failed and/or data/device was not available  
        
        if (currentStatus.current_caninchannel <= 6)     // if channel is 0-7
           {
            currentStatus.current_caninchannel++;       //inc to next channel
           }
        else  
           {
            currentStatus.current_caninchannel = 0;      //reset to start
           }   
             
        break;

    case 'L':
        uint8_t Llength;
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        while (Serial3.available() == 0) { }
        canlisten = Serial3.read();
#elif defined(CORE_STM32)
        while (Serial2.available() == 0) { }
        canlisten = Serial2.read();
#elif defined(CORE_TEENSY)
        while (Serial2.available() == 0) { }
        canlisten = Serial2.read();
#else return;        
#endif
        if (canlisten == 0)
         {
          //command request failed and/or data/device was not available
          break;
         }
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)         
       while (Serial3.available() == 0) { }
       Llength= Serial3.read();              // next the number of bytes expected value
#elif defined(CORE_STM32)
       while (Serial2.available() == 0) { }
       Llength= Serial2.read();              // next the number of bytes expected value
#elif defined(CORE_TEENSY)       
       while (Serial2.available() == 0) { }
       Llength= Serial2.read();              // next the number of bytes expected value
#else return;       
#endif
         for (uint8_t Lcount = 0; Lcount <Llength ;Lcount++)
                {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)                  
                  while (Serial3.available() == 0){}
                  // receive all x bytes into "Lbuffer"
                  Lbuffer[Lcount] = Serial3.read();
#elif defined(CORE_STM32)
                  while (Serial2.available() == 0){}
                  // receive all x bytes into "Lbuffer"
                  Lbuffer[Lcount] = Serial2.read();
#elif defined(CORE_TEENSY)
                  while (Serial2.available() == 0){}
                  // receive all x bytes into "Lbuffer"
                  Lbuffer[Lcount] = Serial2.read();
#else return;                  
#endif
                }
       break;
       
    case 'r': //New format for the optimised OutputChannels
      byte cmd;
      byte tsCanId_sent;
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //ATmega2561 does not have Serial3      
      if (Serial3.available() < 6) { return; }
      tsCanId_sent = Serial3.read(); //Read the $tsCanId
      cmd = Serial3.read();
#elif defined(CORE_STM32)
      if (Serial2.available() < 6) { return; }
      tsCanId_sent = Serial2.read(); //Read the $tsCanId
      cmd = Serial2.read();
#elif defined(CORE_TEENSY)
      if (Serial2.available() < 6) { return; }
      tsCanId_sent = Serial2.read(); //Read the $tsCanId
      cmd = Serial2.read();
#else return;      
#endif
      uint16_t offset, length;
      if(cmd == 0x30) //Send output channels command 0x30 is 48dec
      {
        byte tmp;
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //ATmega2561 does not have Serial3
        tmp = Serial3.read();
        offset = word(Serial3.read(), tmp);
        tmp = Serial3.read();
        length = word(Serial3.read(), tmp);
#elif defined(CORE_STM32)
        tmp = Serial2.read();
        offset = word(Serial2.read(), tmp);
        tmp = Serial2.read();
        length = word(Serial2.read(), tmp);
#elif defined(CORE_TEENSY)
        tmp = Serial2.read();
        offset = word(Serial2.read(), tmp);
        tmp = Serial2.read();
        length = word(Serial2.read(), tmp);
#else return;        
#endif        
        sendValues(offset, length, 3);
      }
      else
      {
        //No other r/ commands should be called
      }
      break;
      
    case 'S': // send code version
       for (unsigned int sig = 0; sig < sizeof(displaySignature) - 1; sig++){
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //ATmega2561 does not have Serial3        
           Serial3.write(displaySignature[sig]);
#elif defined(CORE_STM32) 
          Serial2.write(displaySignature[sig]);
#elif defined(CORE_TEENSY)
          Serial2.write(displaySignature[sig]);
#else return;
#endif          
       }
       //Serial3.print("speeduino 201609-dev");
       break;

    case 'Q': // send code version
       for (unsigned int revn = 0; revn < sizeof( TSfirmwareVersion) - 1; revn++){
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //ATmega2561 does not have Serial3
           Serial3.write( TSfirmwareVersion[revn]);
#elif defined(CORE_STM32) 
           Serial2.write( TSfirmwareVersion[revn]);          
#elif defined(CORE_TEENSY)
           Serial2.write( TSfirmwareVersion[revn]);
#else return;
#endif           
       }
       //Serial3.print("speeduino 201609-dev");
       break;

    case 'Z': //dev use
       break;
       
    default:
       break;
  }
}

// this routine sends a request(either "0" for a "G" , "1" for a "L" , "2" for a "R" to the Can interface or "3" sends the request via the actual local canbus
void sendCancommand(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t paramgroup)
{
    switch (cmdtype)
    {
      case 0:
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //ATmega2561 does not have Serial3       
        Serial3.print("G");
        Serial3.write(canaddress);  //tscanid of speeduino device
        Serial3.write(candata1);    // table id
        Serial3.write(candata2);    //table memory offset
#elif defined(CORE_STM32)
        Serial2.print("G");
        Serial2.write(canaddress);  //tscanid of speeduino device
        Serial2.write(candata1);    // table id
        Serial2.write(candata2);    //table memory offset
#elif defined(CORE_TEENSY)
        Serial2.print("G");
        Serial2.write(canaddress);  //tscanid of speeduino device
        Serial2.write(candata1);    // table id
        Serial2.write(candata2);    //table memory offset
#else return;
#endif        
     break;

     case 1:                      //send request to listen for a can message
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //ATmega2561 does not have Serial3 
        Serial3.print("L");
        Serial3.write(canaddress);  //11 bit canaddress of device to listen for
#elif defined(CORE_STM32)
        Serial2.print("L");
        Serial2.write(canaddress);  //11 bit canaddress of device to listen for
#elif defined(CORE_TEENSY)        
        Serial2.print("L");
        Serial2.write(canaddress);  //11 bit canaddress of device to listen for

#else return;
#endif
     break;

     case 2:
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //ATmega2561 does not have Serial3     
        Serial3.print("R");                         //send "R" to request data from the parmagroup whos value is sent next
        Serial3.write( lowByte(paramgroup) );       //send lsb first
        Serial3.write( lowByte(paramgroup >> 8) );
#elif defined(CORE_STM32)
        Serial2.print("R");                         //send "R" to request data from the parmagroup whos value is sent next
        Serial2.write( lowByte(paramgroup) );       //send lsb first
        Serial2.write( lowByte(paramgroup >> 8) );
#elif defined(CORE_TEENSY)
        Serial2.print("R");                         //send "R" to request data from the parmagroup whos value is sent next
        Serial2.write( lowByte(paramgroup) );       //send lsb first
        Serial2.write( lowByte(paramgroup >> 8) );
#else return;
#endif
     break;

     case 3:
        //send to truecan send routine
#if defined(CORE_STM32)

#elif defined(CORE_TEENSY)
#else return;
#endif
     break;
    }
}

