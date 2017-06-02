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

void canCommand()
{
  currentcanCommand = CANSerial.read();

  switch (currentcanCommand)
  {
    case 'A': // sends the bytes of realtime values
        sendValues(0, packetSize,3); //send values to serial3
        break;

    case 'G': // this is the reply command sent by the Can interface
        //uint8_t Gdata;
        while (CANSerial.available() == 0) { }
        cancmdfail = CANSerial.read();

        if (cancmdfail != 0)
        {
          for (byte Gx = 0; Gx < 8; Gx++) //read all 8 bytes of data
            {
              while (CANSerial.available() == 0) { }
              Gdata[Gx] = CANSerial.read();
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
        while (CANSerial.available() == 0) { }
        canlisten = CANSerial.read();

        if (canlisten == 0)
         {
          //command request failed and/or data/device was not available
          break;
         }

         while (CANSerial.available() == 0) { }
         Llength= CANSerial.read();              // next the number of bytes expected value

         for (uint8_t Lcount = 0; Lcount <Llength ;Lcount++)
         {
           while (CANSerial.available() == 0){}
           // receive all x bytes into "Lbuffer"
           Lbuffer[Lcount] = CANSerial.read();
         }
         break;

    case 'r': //New format for the optimised OutputChannels
      byte cmd;

      if (CANSerial.available() >= 6)
      {
        CANSerial.read(); //Read the $tsCanId
        cmd = CANSerial.read();

        uint16_t offset, length;
        if(cmd == 0x30) //Send output channels command 0x30 is 48dec
        {
          byte tmp;
          tmp = CANSerial.read();
          offset = word(CANSerial.read(), tmp);
          tmp = CANSerial.read();
          length = word(CANSerial.read(), tmp);
          sendValues(offset, length, 3);
        }
        else
        {
          //No other r/ commands should be called
        }
      }
      break;

    case 'S': // send code version
       for (unsigned int sig = 0; sig < sizeof(displaySignature) - 1; sig++)
       {
         CANSerial.write(displaySignature[sig]);
       }
       //Serial3.print("speeduino 201609-dev");
       break;

    case 'Q': // send code version
       for (unsigned int revn = 0; revn < sizeof( TSfirmwareVersion) - 1; revn++)
       {
         CANSerial.write( TSfirmwareVersion[revn]);
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
        CANSerial.print("G");
        CANSerial.write(canaddress);  //tscanid of speeduino device
        CANSerial.write(candata1);    // table id
        CANSerial.write(candata2);    //table memory offset
        break;

      case 1:                      //send request to listen for a can message
        CANSerial.print("L");
        CANSerial.write(canaddress);  //11 bit canaddress of device to listen for
        break;

     case 2:
        CANSerial.print("R");                         //send "R" to request data from the parmagroup whos value is sent next
        CANSerial.write( lowByte(paramgroup) );       //send lsb first
        CANSerial.write( lowByte(paramgroup >> 8) );
        break;

     case 3:
        //send to truecan send routine
        break;

     default:
        break;
    }
}

