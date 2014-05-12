/*
This is called when a command is received over serial from TunerStudio / Megatune
It parses the command and calls the relevant function
A detailed description of each call can be found at: http://www.msextra.com/doc/ms1extra/COM_RS232.htm
*/
void command()
{
  switch (Serial.read()) 
  {
      case 'A': // send 22 bytes of realtime values
        sendValues(22);
        break;

      case 'B': // Burn current values to eeprom
        writeConfig();
        break;

      case 'C': // test communications. This is used by Tunerstudio to see whether there is an ECU on a given serial port
        testComm();
        break;

      case 'P': // set the current page
        //A 2nd byte of data is required after the 'P' specifying the new page number. 
        //This loop should never need to run as the byte should already be in the buffer, but is here just in case
        while (Serial.available() == 0) { }
        currentPage = Serial.read();
        break; 

      case 'R': // send 39 bytes of realtime values
        sendValues(39);
        break;    

      case 'S': // send code version
        Serial.print(signature);
        break;

      case 'Q': // send code version
        Serial.print(signature);
        //Serial.write("Speeduino_0_2");
        break;

      case 'V': // send VE table and constants
        sendPage();
        break;

      case 'W': // receive new VE or constant at 'W'+<offset>+<newbyte>
        byte offset;
        while (Serial.available() == 0) { }
        offset = Serial.read();
        while (Serial.available() == 0) { }
        
        receiveValue(offset, Serial.read());
	break;

      case 't': // receive new Calibration info. "t", 0, <tble_idx> <data array>. This is an MS2/Extra command, NOT part of MS1 spec
        byte tableID;
        
        EEPROM.write(260,'2');
        EEPROM.write(261,'2');
        EEPROM.write(262,'2');
        int y;
        y=263;
        while (y<300)
        {
          while (Serial.available() == 0) { }
          EEPROM.write(y, Serial.read());
          y++;
        }       

	break;

      case 'Z': //Totally non-standard testing function. Will be removed once calibration testing is completed
        for(int x=260; x<300; x++)
        {
          Serial.write(EEPROM.read(x));
        }
        Serial.write('2');
        Serial.flush();
	break;

      default:
	break;
  } 
}

/*
This function returns the current values of a fixed group of variables
*/
void sendValues(int length)
{
  byte packetSize = 26;
  byte response[packetSize];
  
  response[0] = currentStatus.secl; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
  response[1] = currentStatus.squirt; //Squirt Bitfield
  response[2] = currentStatus.engine; //Engine Status Bitfield - NOT YET WORKING
  response[3] = 0x00; //baro
  response[4] = currentStatus.MAP; //map
  response[5] = 0x00; //mat
  response[6] = 0x00; //Coolant ADC
  response[7] = currentStatus.tpsADC; //TPS (Raw 0-255)
  response[8] = 0x00; //battery voltage
  response[9] = 0x00; //O2
  response[10] = 0x00; //Exhaust gas correction (%)
  response[11] = 0x00; //Air Correction (%)
  response[12] = 0x00; //Warmup enrichment (%)
  response[13] = div(currentStatus.RPM, 100).quot; //rpm / 100
  response[14] = div(currentStatus.PW, 100).quot; //Pulsewidth 1 divided by 10 (in ms)
  response[15] = 0x00; //acceleration enrichment (ms)
  response[16] = 0x00; //Barometer correction (%)
  response[17] = 0x00; //Total GammaE (%)
  response[18] = currentStatus.VE; //Current VE 1 (%)
  response[19] = configPage1.tpsMin; //Pulsewidth 2 divided by 10 (in ms)
  response[20] = configPage1.tpsMax; //Current VE 2 (%)
  response[21] = currentStatus.tpsDOT; //TPS DOT
  response[22] = currentStatus.advance;
  response[23] = currentStatus.TPS; // TPS (0% to 100%)
  //Need to split the int loopsPerSecond value into 2 bytes
  response[24] = highByte(currentStatus.loopsPerSecond); //(byte)((currentStatus.loopsPerSecond >> 8) & 0xFF);
  response[25] = lowByte(currentStatus.loopsPerSecond); //(byte)(currentStatus.loopsPerSecond & 0xFF);

  Serial.write(response, (size_t)packetSize);
  Serial.flush();
  return; 
}

void receiveValue(byte offset, byte newValue)
{
  
  byte* pnt_configPage;

  switch (currentPage) 
  {
      case vePage:
        pnt_configPage = (byte *)&configPage1; //Setup a pointer to the relevant config page
        if (offset < 64) //New value is part of the fuel map
        {
          fuelTable.values[7-offset/8][offset%8] = newValue;
          return;
        }
        else if (offset < 80) //New value is one of the X or Y axis bins
        {
          //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
          if (offset < 72) 
          { 
            //X Axis
            fuelTable.axisX[(offset-64)] = (int(newValue) * 100); //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct
          }
          else
          { 
            //Y Axis
            offset = 7-(offset-72); //Need to do a translation to flip the order (Due to us using (0,0) in the top left rather than bottom right
            fuelTable.axisY[offset] = int(newValue);
          }
          return;
        }
        else //New value is one of the remaining config items
        {
          //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
          if( offset < page_size)
          {
            *(pnt_configPage + byte(offset - 80)) = newValue; //Need to subtract 80 because the map and bins (Which make up 80 bytes) aren't part of the config pages
          } 
          return;
        }
        break;
        
      case ignPage: //Ignition settings page (Page 2)
        pnt_configPage = (byte *)&configPage2;
        if (offset < 64) //New value is part of the ignition map
        {
          ignitionTable.values[7-offset/8][offset%8] = newValue;
          return;
        }
        else if (offset < 80) //New value is one of the X or Y axis bins
        {
          //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
          if (offset < 72) 
          { 
            //X Axis
            ignitionTable.axisX[(offset-64)] = int(newValue) * int(100); //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct
          }
          else
          { 
            
            //Y Axis
            offset = 7-(offset-72); //Need to do a translation to flip the order 
            ignitionTable.axisY[offset] = int(newValue);
            
          }
          return;
        }
        else //New value is one of the remaining config items
        {
          //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
          if( offset < page_size)
          {
            *(pnt_configPage + byte(offset - 80)) = newValue; //Need to subtract 80 because the map and bins (Which make up 80 bytes) aren't part of the config pages
          }
          return;
        }
        break;
      
      default:
	break;
  }
}

/*
sendPage() packs the data within the current page (As set with the 'P' command) 
into a buffer and sends it.
Note that some translation of the data is required to lay it out in the way Megasqurit / TunerStudio expect it
*/
void sendPage()
{
  byte response[page_size];
  byte offset;
  byte* pnt_configPage;
  
  switch (currentPage) 
  {
      case vePage:
        //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
        //MS format has origin (0,0) in the bottom left corner, we use the top left for efficiency reasons
        for(byte x=0;x<64;x++) { response[x] = fuelTable.values[7-x/8][x%8]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged
        for(byte x=64;x<72;x++) { response[x] = byte(fuelTable.axisX[(x-64)] / 100); } //RPM Bins for VE table (Need to be dvidied by 100)
        for(byte y=72;y<80;y++) { response[y] = byte(fuelTable.axisY[7-(y-72)]); } //MAP or TPS bins for VE table 
        
        //All other bytes can simply be copied from the config table
        pnt_configPage = (byte *)&configPage1; //Create a pointer to Page 1 in memory
        offset = 80; //Offset is based on the amount already copied above (table + bins)
        
        for(byte x=offset; x<page_size; x++)
        { 
          response[x] = *(pnt_configPage + byte(x - offset)); //Each byte is simply the location in memory of configPage1 + the offset + the variable number (x)
        }
        Serial.write((byte *)&response, sizeof(response));
        Serial.flush();
        break;
        
      case ignPage:
        //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
        for(byte x=0;x<64;x++) { response[x] = ignitionTable.values[7-x/8][x%8]; }
        for(byte x=64;x<72;x++) { response[x] = byte(ignitionTable.axisX[(x-64)] / 100); }
        for(byte y=72;y<80;y++) { response[y] = byte(ignitionTable.axisY[7-(y-72)]); }
        
        //All other bytes can simply be copied from the config table
        pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
        offset = 80; //Offset is based on the amount already copied above (table + bins)
        for(byte x=offset; x<page_size; x++)
        { 
          response[x] = *(pnt_configPage + byte(x - offset)); //Each byte is simply the location in memory of configPage2 + the offset + the variable number (x)
        }
        Serial.write((byte *)&response, sizeof(response)); 
        Serial.flush();
        break;
        
      default:
	break;
  }

  return; 
}


/*
This function is used to store calibration data sent by Tuner Studio. 
*/
void receiveCalibration()
{
  
}

void testComm()
{
  Serial.write(1);
  return; 
}
