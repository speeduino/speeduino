/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
  Processes the data on the serial buffer.
  Can be either a new command or a continuation of one that is already in progress:
    * cmdPending = If a command has started but is wairing on further data to complete
    * chunkPending = Specifically for the new receive value method where TS will send a known number of contiguous bytes to be written to a table
*/
void command()
{

  if (cmdPending == false) { currentCommand = Serial.read(); }

  switch (currentCommand)
  {

    case 'A': // send x bytes of realtime values
      sendValues(0, packetSize,0x30, 0);   //send values to serial0
      break;


    case 'B': // Burn current values to eeprom
      writeAllConfig();
      break;

    case 'b': // New EEPROM burn command to only burn a single page at a time
      cmdPending = true;

      if (Serial.available() >= 2)
      {
        Serial.read(); //Ignore the first table value, it's always 0
        writeConfig(Serial.read());
        cmdPending = false;
      }
      break;

    case 'C': // test communications. This is used by Tunerstudio to see whether there is an ECU on a given serial port
      testComm();
      break;

    case 'c': //Send the current loops/sec value
      Serial.write(lowByte(currentStatus.loopsPerSecond));
      Serial.write(highByte(currentStatus.loopsPerSecond));
      break;

    //The following can be used to show the amount of free memory

    case 'E': // receive command button commands
      cmdPending = true;

      if(Serial.available() >= 2)
      {
        cmdGroup = Serial.read();
        cmdValue = Serial.read();
        cmdCombined = word(cmdGroup, cmdValue);
        if (currentStatus.RPM == 0) { commandButtons(); }

        cmdPending = false;
      }
      break;

    case 'F': // send serial protocol version
      Serial.print("001");
      break;

    case 'L': // List the contents of current page in human readable form
      sendPage(true);
      break;

    case 'm': //Send the current free memory
      currentStatus.freeRAM = freeRam();
      Serial.write(lowByte(currentStatus.freeRAM));
      Serial.write(highByte(currentStatus.freeRAM));
      break;

    case 'N': // Displays a new line.  Like pushing enter in a text editor
      Serial.println();
      break;

    case 'P': // set the current page
      //A 2nd byte of data is required after the 'P' specifying the new page number.
      cmdPending = true;

      if (Serial.available() > 0)
      {
        currentPage = Serial.read();
        //This converts the ascii number char into binary. Note that this will break everyything if there are ever more than 48 pages (48 = asci code for '0')
        if (currentPage >= '0') { currentPage -= '0'; }
        // Detecting if the current page is a table/map
        if ( (currentPage == veMapPage) || (currentPage == ignMapPage) || (currentPage == afrMapPage) ) { isMap = true; }
        else { isMap = false; }
        cmdPending = false;
      }
      break;

    /*
    * New method for sending page values
    */
    case 'p':
      cmdPending = true;

      //6 bytes required:
      //2 - Page identifier
      //2 - offset
      //2 - Length
      if(Serial.available() >= 6)
      {
        byte offset1, offset2, length1, length2;
        int length;
        byte tempPage;

        Serial.read(); // First byte of the page identifier can be ignored. It's always 0
        tempPage = Serial.read();
        //currentPage = 1;
        offset1 = Serial.read();
        offset2 = Serial.read();
        valueOffset = word(offset2, offset1);
        length1 = Serial.read();
        length2 = Serial.read();
        length = word(length2, length1);
        for(int i = 0; i < length; i++)
        {
          Serial.write( getPageValue(tempPage, valueOffset + i) );
        }

        cmdPending = false;
      }
      break;

    case 'Q': // send code version
      Serial.print("speeduino 201710-dev");
      break;

    case 'r': //New format for the optimised OutputChannels
      cmdPending = true;
      byte cmd;
      if (Serial.available() >= 6)
      {
        tsCanId = Serial.read(); //Read the $tsCanId
        cmd = Serial.read(); // read the command

        uint16_t offset, length;
        if(cmd == 0x30) //Send output channels command 0x30 is 48dec
        {
          byte tmp;
          tmp = Serial.read();
          offset = word(Serial.read(), tmp);
          tmp = Serial.read();
          length = word(Serial.read(), tmp);
          sendValues(offset, length,cmd, 0);
        }
        else
        {
          //No other r/ commands should be called
        }
        cmdPending = false;
      }
      break;

    case 'S': // send code version
      Serial.print("Speeduino 2017.10-dev");
      currentStatus.secl = 0; //This is required in TS3 due to its stricter timings
      break;

    case 'T': //Send 256 tooth log entries to Tuner Studios tooth logger
      sendToothLog(false); //Sends tooth log values as ints
      break;

    case 't': // receive new Calibration info. Command structure: "t", <tble_idx> <data array>. This is an MS2/Extra command, NOT part of MS1 spec
      byte tableID;
      //byte canID;

      //The first 2 bytes sent represent the canID and tableID
      while (Serial.available() == 0) { }
      tableID = Serial.read(); //Not currently used for anything

      receiveCalibration(tableID); //Receive new values and store in memory
      writeCalibration(); //Store received values in EEPROM

      break;

    case 'V': // send VE table and constants in binary
      sendPage(false);
      break;

    case 'W': // receive new VE obr constant at 'W'+<offset>+<newbyte>
      cmdPending = true;

      if (isMap)
      {
        if(Serial.available() >= 3) // 1 additional byte is required on the MAP pages which are larger than 255 bytes
        {
          byte offset1, offset2;
          offset1 = Serial.read();
          offset2 = Serial.read();
          valueOffset = word(offset2, offset1);
          receiveValue(valueOffset, Serial.read());
          cmdPending = false;
        }
      }
      else
      {
        if(Serial.available() >= 2)
        {
          valueOffset = Serial.read();
          receiveValue(valueOffset, Serial.read());
          cmdPending = false;
        }
      }

      break;

    case 'w':
      cmdPending = true;

      if(chunkPending == false)
      {
        //This means it's a new request
        //7 bytes required:
        //2 - Page identifier
        //2 - offset
        //2 - Length (Should always be 1 until chunk write is setup)
        //1 - New value
        if(Serial.available() >= 7)
        {
          byte offset1, offset2, length1, length2;

          Serial.read(); // First byte of the page identifier can be ignored. It's always 0
          currentPage = Serial.read();
          //currentPage = 1;
          offset1 = Serial.read();
          offset2 = Serial.read();
          valueOffset = word(offset2, offset1);
          length1 = Serial.read(); // Length to be written (Should always be 1)
          length2 = Serial.read(); // Length to be written (Should always be 1)
          chunkSize = word(length2, length1);

          chunkPending = true;
          chunkComplete = 0;
        }
      }
      //This CANNOT be an else of the above if statement as chunkPending gets set to true above
      if(chunkPending == true)
      {
        while( (Serial.available() > 0) && (chunkComplete < chunkSize) )
        {
          receiveValue( (valueOffset + chunkComplete), Serial.read());
          chunkComplete++;
        }
        if(chunkComplete >= chunkSize) { cmdPending = false; chunkPending = false; }
      }
      break;

    case 'Z': //Totally non-standard testing function. Will be removed once calibration testing is completed. This function takes 1.5kb of program space! :S
    #ifndef SMALL_FLASH_MODE
      Serial.println(F("Coolant"));
      for (int x = 0; x < CALIBRATION_TABLE_SIZE; x++)
      {
        Serial.print(x);
        Serial.print(", ");
        Serial.println(cltCalibrationTable[x]);
      }
      Serial.println(F("Inlet temp"));
      for (int x = 0; x < CALIBRATION_TABLE_SIZE; x++)
      {
        Serial.print(x);
        Serial.print(", ");
        Serial.println(iatCalibrationTable[x]);
      }
      Serial.println(F("O2"));
      for (int x = 0; x < CALIBRATION_TABLE_SIZE; x++)
      {
        Serial.print(x);
        Serial.print(", ");
        Serial.println(o2CalibrationTable[x]);
      }
      Serial.println(F("WUE"));
      for (int x = 0; x < 10; x++)
      {
        Serial.print(configPage2.wueBins[x]);
        Serial.print(", ");
        Serial.println(configPage1.wueValues[x]);
      }
      Serial.flush();
    #endif
      break;

    case 'z': //Send 256 tooth log entries to a terminal emulator
      sendToothLog(true); //Sends tooth log values as chars
      break;

    case '?':
    #ifndef SMALL_FLASH_MODE
      Serial.println
      (F(
         "\n"
         "===Command Help===\n\n"
         "All commands are single character and are concatenated with their parameters \n"
         "without spaces."
         "Syntax:  <command>+<parameter1>+<parameter2>+<parameterN>\n\n"
         "===List of Commands===\n\n"
         "A - Displays 31 bytes of currentStatus values in binary (live data)\n"
         "B - Burn current map and configPage values to eeprom\n"
         "C - Test COM port.  Used by Tunerstudio to see whether an ECU is on a given serial \n"
         "    port. Returns a binary number.\n"
         "N - Print new line.\n"
         "P - Set current page.  Syntax:  P+<pageNumber>\n"
         "R - Same as A command\n"
         "S - Display signature number\n"
         "Q - Same as S command\n"
         "V - Display map or configPage values in binary\n"
         "W - Set one byte in map or configPage.  Expects binary parameters. \n"
         "    Syntax:  W+<offset>+<newbyte>\n"
         "t - Set calibration values.  Expects binary parameters.  Table index is either 0, \n"
         "    1, or 2.  Syntax:  t+<tble_idx>+<newValue1>+<newValue2>+<newValueN>\n"
         "Z - Display calibration values\n"
         "T - Displays 256 tooth log entries in binary\n"
         "r - Displays 256 tooth log entries\n"
         "? - Displays this help page"
       ));
     #endif

      break;

    default:
      break;
  }
}

/*
This function returns the current values of a fixed group of variables
*/
//void sendValues(int packetlength, byte portNum)
void sendValues(uint16_t offset, uint16_t packetLength, byte cmd, byte portNum)
{
  byte fullStatus[packetSize];

  if (portNum == 3)
  {
    //CAN serial
    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)|| defined(CORE_STM32) || defined (CORE_TEENSY) //ATmega2561 does not have Serial3
      if (offset == 0)
        {
          CANSerial.write("A");         //confirm cmd type
        }
      else
        {
      CANSerial.write("r");         //confirm cmd type
      CANSerial.write(cmd);
        }
    #endif
  }
  else
  {
    if(requestCount == 0) { currentStatus.secl = 0; }
    requestCount++;
  }

  currentStatus.spark ^= (-currentStatus.hasSync ^ currentStatus.spark) & (1 << BIT_SPARK_SYNC); //Set the sync bit of the Spark variable to match the hasSync variable

  fullStatus[0] = currentStatus.secl; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
  fullStatus[1] = currentStatus.status1; //status1 Bitfield
  fullStatus[2] = currentStatus.engine; //Engine Status Bitfield
  fullStatus[3] = (byte)(divu100(currentStatus.dwell)); //Dwell in ms * 10
  fullStatus[4] = lowByte(currentStatus.MAP); //2 bytes for MAP
  fullStatus[5] = highByte(currentStatus.MAP);
  fullStatus[6] = (byte)(currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET); //mat
  fullStatus[7] = (byte)(currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //Coolant ADC
  fullStatus[8] = currentStatus.batCorrection; //Battery voltage correction (%)
  fullStatus[9] = currentStatus.battery10; //battery voltage
  fullStatus[10] = currentStatus.O2; //O2
  fullStatus[11] = currentStatus.egoCorrection; //Exhaust gas correction (%)
  fullStatus[12] = currentStatus.iatCorrection; //Air temperature Correction (%)
  fullStatus[13] = currentStatus.wueCorrection; //Warmup enrichment (%)
  fullStatus[14] = lowByte(currentStatus.RPM); //rpm HB
  fullStatus[15] = highByte(currentStatus.RPM); //rpm LB
  fullStatus[16] = currentStatus.TAEamount; //acceleration enrichment (%)
  fullStatus[17] = currentStatus.corrections; //Total GammaE (%)
  fullStatus[18] = currentStatus.VE; //Current VE 1 (%)
  fullStatus[19] = currentStatus.afrTarget;
  fullStatus[20] = lowByte(currentStatus.PW1); //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
  fullStatus[21] = highByte(currentStatus.PW1); //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
  fullStatus[22] = currentStatus.tpsDOT; //TPS DOT
  fullStatus[23] = currentStatus.advance;
  fullStatus[24] = currentStatus.TPS; // TPS (0% to 100%)
  //Need to split the int loopsPerSecond value into 2 bytes
  fullStatus[25] = lowByte(currentStatus.loopsPerSecond);
  fullStatus[26] = highByte(currentStatus.loopsPerSecond);

  //The following can be used to show the amount of free memory
  currentStatus.freeRAM = freeRam();
  fullStatus[27] = lowByte(currentStatus.freeRAM); //(byte)((currentStatus.loopsPerSecond >> 8) & 0xFF);
  fullStatus[28] = highByte(currentStatus.freeRAM);

  fullStatus[29] = (byte)(currentStatus.boostTarget >> 1); //Divide boost target by 2 to fit in a byte
  fullStatus[30] = (byte)(currentStatus.boostDuty / 100);
  fullStatus[31] = currentStatus.spark; //Spark related bitfield

  //rpmDOT must be sent as a signed integer
  fullStatus[32] = lowByte(currentStatus.rpmDOT);
  fullStatus[33] = highByte(currentStatus.rpmDOT);

  fullStatus[34] = currentStatus.ethanolPct; //Flex sensor value (or 0 if not used)
  fullStatus[35] = currentStatus.flexCorrection; //Flex fuel correction (% above or below 100)
  fullStatus[36] = currentStatus.flexIgnCorrection; //Ignition correction (Increased degrees of advance) for flex fuel

  fullStatus[37] = currentStatus.idleLoad;
  fullStatus[38] = currentStatus.testOutputs;

  fullStatus[39] = currentStatus.O2_2; //O2
  fullStatus[40] = currentStatus.baro; //Barometer value

  fullStatus[41] = lowByte(currentStatus.canin[0]);
  fullStatus[42] = highByte(currentStatus.canin[0]);
  fullStatus[43] = lowByte(currentStatus.canin[1]);
  fullStatus[44] = highByte(currentStatus.canin[1]);
  fullStatus[45] = lowByte(currentStatus.canin[2]);
  fullStatus[46] = highByte(currentStatus.canin[2]);
  fullStatus[47] = lowByte(currentStatus.canin[3]);
  fullStatus[48] = highByte(currentStatus.canin[3]);
  fullStatus[49] = lowByte(currentStatus.canin[4]);
  fullStatus[50] = highByte(currentStatus.canin[4]);
  fullStatus[51] = lowByte(currentStatus.canin[5]);
  fullStatus[52] = highByte(currentStatus.canin[5]);
  fullStatus[53] = lowByte(currentStatus.canin[6]);
  fullStatus[54] = highByte(currentStatus.canin[6]);
  fullStatus[55] = lowByte(currentStatus.canin[7]);
  fullStatus[56] = highByte(currentStatus.canin[7]);
  fullStatus[57] = lowByte(currentStatus.canin[8]);
  fullStatus[58] = highByte(currentStatus.canin[8]);
  fullStatus[59] = lowByte(currentStatus.canin[9]);
  fullStatus[60] = highByte(currentStatus.canin[9]);
  fullStatus[61] = lowByte(currentStatus.canin[10]);
  fullStatus[62] = highByte(currentStatus.canin[10]);
  fullStatus[63] = lowByte(currentStatus.canin[11]);
  fullStatus[64] = highByte(currentStatus.canin[11]);
  fullStatus[65] = lowByte(currentStatus.canin[12]);
  fullStatus[66] = highByte(currentStatus.canin[12]);
  fullStatus[67] = lowByte(currentStatus.canin[13]);
  fullStatus[68] = highByte(currentStatus.canin[13]);
  fullStatus[69] = lowByte(currentStatus.canin[14]);
  fullStatus[70] = highByte(currentStatus.canin[14]);
  fullStatus[71] = lowByte(currentStatus.canin[15]);
  fullStatus[72] = highByte(currentStatus.canin[15]);

  fullStatus[73] = currentStatus.tpsADC;
  fullStatus[74] = getNextError();

  for(byte x=0; x<packetLength; x++)
  {
    if (portNum == 0) { Serial.write(fullStatus[offset+x]); }
    else if (portNum == 3){ CANSerial.write(fullStatus[offset+x]); }
  }

}

void receiveValue(int valueOffset, byte newValue)
{

  void* pnt_configPage;//This only stores the address of the value that it's pointing to and not the max size
  int tempOffset;

  switch (currentPage)
  {
    case veMapPage:
      if (valueOffset < 256) //New value is part of the fuel map
      {
        fuelTable.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          fuelTable.axisX[(valueOffset - 256)] = ((int)(newValue) * TABLE_RPM_MULTIPLIER); //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        }
        else if(valueOffset < 288)
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order (Due to us using (0,0) in the top left rather than bottom right
          fuelTable.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
          //This should never happen. It means there's an invalid offset value coming through
        }
      }
      break;

    case veSetPage:
      pnt_configPage = &configPage1; //Setup a pointer to the relevant config page
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[veSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case ignMapPage: //Ignition settings page (Page 2)
      if (valueOffset < 256) //New value is part of the ignition map
      {
        ignitionTable.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          ignitionTable.axisX[(valueOffset - 256)] = (int)(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct
        }
        else if(valueOffset < 288)
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order
          ignitionTable.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
      }
      break;

    case ignSetPage:
      pnt_configPage = &configPage2;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[ignSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case afrMapPage: //Air/Fuel ratio target settings page
      if (valueOffset < 256) //New value is part of the afr map
      {
        afrTable.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          afrTable.axisX[(valueOffset - 256)] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by megasquirt are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        }
        else
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order
          afrTable.axisY[tempOffset] = int(newValue) * TABLE_LOAD_MULTIPLIER;

        }
      }
      break;

    case afrSetPage:
      pnt_configPage = &configPage3;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[afrSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case boostvvtPage: //Boost and VVT maps (8x8)
      if (valueOffset < 64) //New value is part of the boost map
      {
        boostTable.values[7 - (valueOffset / 8)][valueOffset % 8] = newValue;
      }
      else if (valueOffset < 72) //New value is on the X (RPM) axis of the boost table
      {
        boostTable.axisX[(valueOffset - 64)] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      }
      else if (valueOffset < 80) //New value is on the Y (TPS) axis of the boost table
      {
        boostTable.axisY[(7 - (valueOffset - 72))] = int(newValue); //TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
      }
      else if (valueOffset < 144) //New value is part of the vvt map
      {
        tempOffset = valueOffset - 80;
        vvtTable.values[7 - (tempOffset / 8)][tempOffset % 8] = newValue;
      }
      else if (valueOffset < 152) //New value is on the X (RPM) axis of the vvt table
      {
        tempOffset = valueOffset - 144;
        vvtTable.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      }
      else if (valueOffset < 161) //New value is on the Y (Load) axis of the vvt table
      {
        tempOffset = valueOffset - 152;
        vvtTable.axisY[(7 - tempOffset)] = int(newValue); //TABLE_LOAD_MULTIPLIER is NOT used for vvt as it is TPS based (0-100)
      }
      break;

    case seqFuelPage:
      if (valueOffset < 36) { trim1Table.values[5 - (valueOffset / 6)][valueOffset % 6] = newValue; } //Trim1 values
      else if (valueOffset < 42) { trim1Table.axisX[(valueOffset - 36)] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the trim1 table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 48) { trim1Table.axisY[(5 - (valueOffset - 42))] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (TPS) axis of the boost table
      //Trim table 2
      else if (valueOffset < 84) { tempOffset = valueOffset - 48; trim2Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim2 map
      else if (valueOffset < 90) { tempOffset = valueOffset - 84; trim2Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 96) { tempOffset = valueOffset - 90; trim2Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 3
      else if (valueOffset < 132) { tempOffset = valueOffset - 96; trim3Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim2 map
      else if (valueOffset < 138) { tempOffset = valueOffset - 132; trim3Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 144) { tempOffset = valueOffset - 138; trim3Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 4
      else if (valueOffset < 180) { tempOffset = valueOffset - 144; trim4Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim2 map
      else if (valueOffset < 186) { tempOffset = valueOffset - 180; trim4Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 192) { tempOffset = valueOffset - 186; trim4Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table

      break;

    case canbusPage:
      pnt_configPage = &configPage10;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[currentPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case warmupPage:
      pnt_configPage = &configPage11;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[currentPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    default:
      break;
  }
  //if(Serial.available() > 16) { command(); }
}

/*
sendPage() packs the data within the current page (As set with the 'P' command)
into a buffer and sends it.
Note that some translation of the data is required to lay it out in the way Megasqurit / TunerStudio expect it
useChar - If true, all values are send as chars, this is for the serial command line interface. TunerStudio expects data as raw values, so this must be set false in that case
*/
void sendPage(bool useChar)
{
  void* pnt_configPage = &configPage1; //Default value is for safety only. Will be changed below if needed.
  struct table3D currentTable = fuelTable; //Default value is for safety only. Will be changed below if needed.
  byte currentTitleIndex = 0;// This corresponds to the count up to the first char of a string in pageTitles
  bool sendComplete = false; //Used to track whether all send operations are complete

  switch (currentPage)
  {
    case veMapPage:
        currentTitleIndex = 0;
        currentTable = fuelTable;
        break;

    case veSetPage:
        // currentTitleIndex = 27;
        if (useChar)
        {
          uint16_t* pnt16_configPage;
          // To Display Values from Config Page 1
          // When casting to the __FlashStringHelper type Serial.println uses the same subroutine as when using the F macro
          Serial.println((const __FlashStringHelper *)&pageTitles[27]);//27 is the index to the first char in the second sting in pageTitles
          // The following loop displays in human readable form of all byte values in config page 1 up to but not including the first array.
          // incrementing void pointers is cumbersome. Thus we have "pnt_configPage = (byte *)pnt_configPage + 1"
          for (pnt_configPage = &configPage1; pnt_configPage < &configPage1.wueValues[0]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
          for (byte x = 10; x; x--)// The x between the ';' has the same representation as the "x != 0" test or comparision
          {
            Serial.print(configPage1.wueValues[10 - x]);// This displays the values horizantially on the screen
            Serial.print(' ');
          }
          Serial.println();
          for (pnt_configPage = (byte *)&configPage1.wueValues[9] + 1; pnt_configPage < &configPage1.inj1Ang; pnt_configPage = (byte *)pnt_configPage + 1) {
            Serial.println(*((byte *)pnt_configPage));// This displays all the byte values between the last array up to but not including the first unsigned int on config page 1
          }
          // The following loop displays four unsigned ints
          for (pnt16_configPage = (uint16_t *)&configPage1.inj1Ang; pnt16_configPage < (uint16_t*)&configPage1.inj4Ang + 1; pnt16_configPage = (uint16_t*)pnt16_configPage + 1)
          { Serial.println(*((uint16_t *)pnt16_configPage)); }
          // Following loop displays byte values between the unsigned ints
          for (pnt_configPage = (uint16_t *)&configPage1.inj4Ang + 1; pnt_configPage < &configPage1.mapMax; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
          Serial.println(configPage1.mapMax);
          // Following loop displays remaining byte values of the page
          for (pnt_configPage = (uint16_t *)&configPage1.mapMax + 1; pnt_configPage < (byte *)&configPage1 + npage_size[veSetPage]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
          sendComplete = true;
        }
        else { pnt_configPage = &configPage1; } //Create a pointer to Page 1 in memory
        break;

    case ignMapPage:
        currentTitleIndex = 42;// the index to the first char of the third string in pageTitles
        currentTable = ignitionTable;
        break;

    case ignSetPage:
        //currentTitleIndex = 56;
        if (useChar)
        {
          //To Display Values from Config Page 2
          Serial.println((const __FlashStringHelper *)&pageTitles[56]);
          Serial.println(configPage2.triggerAngle);// configPsge2.triggerAngle is an int so just display it without complication
          // Following loop displays byte values after that first int up to but not including the first array in config page 2
          for (pnt_configPage = (int *)&configPage2 + 1; pnt_configPage < &configPage2.taeBins[0]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
          for (byte y = 2; y; y--)// Displaying two equal sized arrays
          {
            byte * currentVar;// A placeholder for each array
            if (y == 2) {
              currentVar = configPage2.taeBins;
            }
            else {
              currentVar = configPage2.taeValues;
            }

            for (byte j = 4; j; j--)
            {
              Serial.print(currentVar[4 - j]);
              Serial.print(' ');
            }
            Serial.println();
          }
          for (byte x = 10; x ; x--)
          {
            Serial.print(configPage2.wueBins[10 - x]);//Displaying array horizontally across screen
            Serial.print(' ');
          }
          Serial.println();
          Serial.println(configPage2.dwellLimit);// Little lonely byte stuck between two arrays. No complications just display it.
          for (byte x = 6; x; x--)
          {
            Serial.print(configPage2.dwellCorrectionValues[6 - x]);
            Serial.print(' ');
          }
          Serial.println();
          for (pnt_configPage = (byte *)&configPage2.dwellCorrectionValues[5] + 1; pnt_configPage < (byte *)&configPage2 + npage_size[ignSetPage]; pnt_configPage = (byte *)pnt_configPage + 1)
          {
            Serial.println(*((byte *)pnt_configPage));// Displaying remaining byte values of the page
          }
          sendComplete = true;
        }
        else { pnt_configPage = &configPage2; } //Create a pointer to Page 2 in memory
        break;

    case afrMapPage:
        currentTitleIndex = 71;//Array index to next string
        currentTable = afrTable;
        break;

    case afrSetPage:
        //currentTitleIndex = 91;
        if (useChar)
        {
          //To Display Values from Config Page 3
          Serial.println((const __FlashStringHelper *)&pageTitles[91]);//special typecasting to enable suroutine that the F macro uses
          for (pnt_configPage = &configPage3; pnt_configPage < &configPage3.voltageCorrectionBins[0]; pnt_configPage = (byte *)pnt_configPage + 1)
          {
            Serial.println(*((byte *)pnt_configPage));// Displaying byte values of config page 3 up to but not including the first array
          }
          for (byte y = 2; y; y--)// Displaying two equally sized arrays that are next to each other
          {
            byte * currentVar;
            if (y == 2) { currentVar = configPage3.voltageCorrectionBins; }
            else { currentVar = configPage3.injVoltageCorrectionValues; }

            for (byte x = 6; x; x--)
            {
              Serial.print(currentVar[6 - x]);
              Serial.print(' ');
            }
            Serial.println();
          }
          for (byte y = 2; y; y--)// and again
          {
            byte* currentVar;
            if (y == 2) { currentVar = configPage3.airDenBins; }
            else { currentVar = configPage3.airDenRates; }

            for (byte x = 9; x; x--)
            {
              Serial.print(currentVar[9 - x]);
              Serial.print(' ');
            }
            Serial.println();
          }
          // Following loop displays the remaining byte values of the page
          for (pnt_configPage = (byte *)&configPage3.airDenRates[8] + 1; pnt_configPage < (byte *)&configPage3 + npage_size[afrSetPage]; pnt_configPage = (byte *)pnt_configPage + 1)
          {
            Serial.println(*((byte *)pnt_configPage));
          }
          sendComplete = true;
        }
        else { pnt_configPage = &configPage3; } //Create a pointer to Page 3 in memory

        //Old configPage4 STARTED HERE!
        //currentTitleIndex = 106;
        //To Display Values from Config Page 4
        if (useChar)
        {
          Serial.println((const __FlashStringHelper *)&pageTitles[106]);// F macro hack
          for (byte y = 4; y; y--)// Display four equally sized arrays
          {
            byte * currentVar;
            switch (y)
            {
              case 1: currentVar = configPage3.iacBins; break;
              case 2: currentVar = configPage3.iacOLPWMVal; break;
              case 3: currentVar = configPage3.iacOLStepVal; break;
              case 4: currentVar = configPage3.iacCLValues; break;
              default: break;
            }
            for (byte x = 10; x; x--)
            {
              Serial.print(currentVar[10 - x]);
              Serial.print(' ');
            }
            Serial.println();
          }
          for (byte y = 3; y; y--)// Three equally sized arrays
          {
            byte * currentVar;
            switch (y)
            {
              case 1: currentVar = configPage3.iacCrankBins; break;
              case 2: currentVar = configPage3.iacCrankDuty; break;
              case 3: currentVar = configPage3.iacCrankSteps; break;
              default: break;
            }
            for (byte x = 4; x; x--)
            {
              Serial.print(currentVar[4 - x]);
              Serial.print(' ');
            }
            Serial.println();
          }
          // Following loop is for remaining byte value of page
          for (pnt_configPage = (byte *)&configPage3.iacCrankBins[3] + 1; pnt_configPage < (byte *)&configPage3 + npage_size[afrSetPage]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
          sendComplete = true;
        }
        else { pnt_configPage = &configPage3; } //Create a pointer to Page 4 in memory
        break;

    case boostvvtPage:
        if(useChar)
        {
          currentTable = boostTable;
          currentTitleIndex = 121;
        }
        else
        {
          //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
          byte response[160]; //Bit hacky, but the size is: (8x8 + 8 + 8) + (8x8 + 8 + 8) = 160

          //Boost table
          for (int x = 0; x < 64; x++) { response[x] = boostTable.values[7 - (x / 8)][x % 8]; }
          for (int x = 64; x < 72; x++) { response[x] = byte(boostTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 72; y < 80; y++) { response[y] = byte(boostTable.axisY[7 - (y - 72)]); }
          //VVT table
          for (int x = 0; x < 64; x++) { response[x + 80] = vvtTable.values[7 - (x / 8)][x % 8]; }
          for (int x = 64; x < 72; x++) { response[x + 80] = byte(vvtTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 72; y < 80; y++) { response[y + 80] = byte(vvtTable.axisY[7 - (y - 72)]); }
          Serial.write((byte *)&response, sizeof(response));
          sendComplete = true;
        }
        break;

    case seqFuelPage:
        if(useChar)
        {
          currentTable = trim1Table;
          for (int y = 0; y < currentTable.ySize; y++)
          {
            byte axisY = byte(currentTable.axisY[y]);
            if (axisY < 100)
            {
              Serial.write(" ");
              if (axisY < 10)
              {
                Serial.write(" ");
              }
            }
            Serial.print(axisY);// Vertical Bins
            Serial.write(" ");
            for (int x = 0; x < currentTable.xSize; x++)
            {
              byte value = currentTable.values[y][x];
              if (value < 100)
              {
                Serial.write(" ");
                if (value < 10)
                {
                  Serial.write(" ");
                }
              }
              Serial.print(value);
              Serial.write(" ");
            }
            Serial.println("");
          }
          sendComplete = true;
          //Do.... Something?
        }
        else
        {
          //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
          byte response[192]; //Bit hacky, but the size is: (6x6 + 6 + 6) * 4 = 192

          //trim1 table
          for (int x = 0; x < 36; x++) { response[x] = trim1Table.values[5 - (x / 6)][x % 6]; }
          for (int x = 36; x < 42; x++) { response[x] = byte(trim1Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 42; y < 48; y++) { response[y] = byte(trim1Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }
          //trim2 table
          for (int x = 0; x < 36; x++) { response[x + 48] = trim2Table.values[5 - (x / 6)][x % 6]; }
          for (int x = 36; x < 42; x++) { response[x + 48] = byte(trim2Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 42; y < 48; y++) { response[y + 48] = byte(trim2Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }
          //trim3 table
          for (int x = 0; x < 36; x++) { response[x + 96] = trim3Table.values[5 - (x / 6)][x % 6]; }
          for (int x = 36; x < 42; x++) { response[x + 96] = byte(trim3Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 42; y < 48; y++) { response[y + 96] = byte(trim3Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }
          //trim4 table
          for (int x = 0; x < 36; x++) { response[x + 144] = trim4Table.values[5 - (x / 6)][x % 6]; }
          for (int x = 36; x < 42; x++) { response[x + 144] = byte(trim4Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 42; y < 48; y++) { response[y + 144] = byte(trim4Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }
          Serial.write((byte *)&response, sizeof(response));
          sendComplete = true;
        }
        break;

    case canbusPage:
        //currentTitleIndex = 141;
        if (useChar)
        {
          //To Display Values from Config Page 10
          Serial.println((const __FlashStringHelper *)&pageTitles[141]);//special typecasting to enable suroutine that the F macro uses
          for (pnt_configPage = &configPage10; pnt_configPage < ((byte *)pnt_configPage + 128); pnt_configPage = (byte *)pnt_configPage + 1)
          {
            Serial.println(*((byte *)pnt_configPage));// Displaying byte values of config page 3 up to but not including the first array
          }
          sendComplete = true;
        }
        else { pnt_configPage = &configPage10; } //Create a pointer to Page 10 in memory
        break;

    case warmupPage:
        if (useChar)
        {
          sendComplete = true;
        }
        else { pnt_configPage = &configPage11; } //Create a pointer to Page 11 in memory
        break;

    default:
    #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
    #endif
        //Just set default Values to avoid warnings
        pnt_configPage = &configPage11;
        currentTable = fuelTable;
        sendComplete = true;
        break;
  }
  if(!sendComplete)
  {
    if (isMap)
    {
      if (useChar)
      {
        do //This is a do while loop that kicks in for the boostvvtPage
        {
          const char spaceChar = ' ';
          /*while(pageTitles[currentTitleIndex])
          {
           Serial.print(pageTitles[currentTitleIndex]);
           currentTitleIndex++;
          }*/
          Serial.println((const __FlashStringHelper *)&pageTitles[currentTitleIndex]);// F macro hack
          Serial.println();
          for (int y = 0; y < currentTable.ySize; y++)
          {
            byte axisY = byte(currentTable.axisY[y]);
            if (axisY < 100)
            {
              Serial.write(spaceChar);
              if (axisY < 10)
              {
                Serial.write(spaceChar);
              }
            }
            Serial.print(axisY);// Vertical Bins
            Serial.write(spaceChar);
            for (int i = 0; i < currentTable.xSize; i++)
            {
              byte value = currentTable.values[y][i];
              if (value < 100)
              {
                Serial.write(spaceChar);
                if (value < 10)
                {
                  Serial.write(spaceChar);
                }
              }
              Serial.print(value);
              Serial.write(spaceChar);
            }
            Serial.println();
          }
          Serial.print(F("    "));
          for (int x = 0; x < currentTable.xSize; x++)// Horizontal bins
          {
            byte axisX = byte(currentTable.axisX[x] / 100);
            if (axisX < 100)
            {
              Serial.write(spaceChar);
              if (axisX < 10)
              {
                Serial.write(spaceChar);
              }
            }
            Serial.print(axisX);
            Serial.write(spaceChar);
          }
          Serial.println();
          if(currentTitleIndex == 121) //Check to see if on boostTable
          {
            currentTitleIndex = 132; //Change over to vvtTable mid display
            currentTable = vvtTable;
          }
          else currentTitleIndex = 0;
        }while(currentTitleIndex == 132); //Should never loop unless going to display vvtTable
      } //use char
      else
      {
        //Need to perform a translation of the values[yaxis][xaxis] into the MS expected format
        //MS format has origin (0,0) in the bottom left corner, we use the top left for efficiency reasons
        byte response[MAP_PAGE_SIZE];

        for (int x = 0; x < 256; x++) { response[x] = currentTable.values[15 - (x / 16)][x % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        //loop();
        for (int x = 256; x < 272; x++) { response[x] = byte(currentTable.axisX[(x - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        //loop();
        for (int y = 272; y < 288; y++) { response[y] = byte(currentTable.axisY[15 - (y - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        //loop();
        Serial.write((byte *)&response, sizeof(response));
      }
    } //is map
    else
    {
      /*if(useChar)
      {
       while(pageTitles[currentTitleIndex])
       {
        Serial.print(pageTitles[currentTitleIndex]);
        currentTitleIndex++;
       }
       Serial.println();
       for(byte x=0;x<page_size;x++) Serial.println(*((byte *)pnt_configPage + x));
      }
      else
      {*/
      //All other bytes can simply be copied from the config table
      //byte response[npage_size[currentPage]];
      for (byte x = 0; x < npage_size[currentPage]; x++)
      {
        //response[x] = *((byte *)pnt_configPage + x);
        Serial.write(*((byte *)pnt_configPage + x)); //Each byte is simply the location in memory of the configPage + the offset + the variable number (x)
      }

      //Serial.write((byte *)&response, npage_size[currentPage]);
      // }
    } //isMap
  } //sendComplete
}

byte getPageValue(byte page, uint16_t valueAddress)
{
  void* pnt_configPage = &configPage1; //Default value is for safety only. Will be changed below if needed.
  uint16_t tempAddress;
  byte returnValue = 0;

  switch (page)
  {
    case veMapPage:
        if( valueAddress < 256) { returnValue = fuelTable.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(fuelTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(fuelTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;

    case veSetPage:
        pnt_configPage = &configPage1; //Create a pointer to Page 1 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case ignMapPage:
        if( valueAddress < 256) { returnValue = ignitionTable.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(ignitionTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(ignitionTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;

    case ignSetPage:
        pnt_configPage = &configPage2; //Create a pointer to Page 2 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case afrMapPage:
        if( valueAddress < 256) { returnValue = afrTable.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(afrTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(afrTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;

    case afrSetPage:
        pnt_configPage = &configPage3; //Create a pointer to Page 3 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case boostvvtPage:

        {
          //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
          if(valueAddress < 80)
          {
            //Boost table
            if(valueAddress < 64) { returnValue = boostTable.values[7 - (valueAddress / 8)][valueAddress % 8]; }
            else if(valueAddress < 72) { returnValue = byte(boostTable.axisX[(valueAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(valueAddress < 80) { returnValue = byte(boostTable.axisY[7 - (valueAddress - 72)]); }
          }
          else
          {
            tempAddress = valueAddress - 80;
            //VVT table
            if(tempAddress < 64) { returnValue = vvtTable.values[7 - (tempAddress / 8)][tempAddress % 8]; }
            else if(tempAddress < 72) { returnValue = byte(vvtTable.axisX[(tempAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 80) { returnValue = byte(vvtTable.axisY[7 - (tempAddress - 72)]); }
          }
        }
        break;

    case seqFuelPage:

        {
          //Need to perform a translation of the values[MAP/TPS][RPM] into the TS expected format
          if(valueAddress < 48)
          {
            //trim1 table
            if(valueAddress < 36) { returnValue = trim1Table.values[5 - (valueAddress / 6)][valueAddress % 6]; }
            else if(valueAddress < 42) { returnValue = byte(trim1Table.axisX[(valueAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(valueAddress < 48) { returnValue = byte(trim1Table.axisY[5 - (valueAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 96)
          {
            tempAddress = valueAddress - 48;
            //trim2 table
            if(tempAddress < 36) { returnValue = trim2Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim2Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim2Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 144)
          {
            tempAddress = valueAddress - 96;
            //trim3 table
            if(tempAddress < 36) { returnValue = trim3Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim3Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim3Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 192)
          {
            tempAddress = valueAddress - 144;
            //trim4 table
            if(tempAddress < 36) { returnValue = trim4Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim4Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim4Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
        }
        break;

    case canbusPage:
        pnt_configPage = &configPage10; //Create a pointer to Page 10 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case warmupPage:
        pnt_configPage = &configPage11; //Create a pointer to Page 11 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    default:
    #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
    #endif
        //Just set default Values to avoid warnings
        pnt_configPage = &configPage11;
        break;
  }
  return returnValue;
}

/*
This function is used to store calibration data sent by Tuner Studio.
*/
void receiveCalibration(byte tableID)
{
  byte* pnt_TargetTable; //Pointer that will be used to point to the required target table
  int OFFSET, DIVISION_FACTOR, BYTES_PER_VALUE, EEPROM_START;

  switch (tableID)
  {
    case 0:
      //coolant table
      pnt_TargetTable = (byte *)&cltCalibrationTable;
      OFFSET = CALIBRATION_TEMPERATURE_OFFSET; //
      DIVISION_FACTOR = 10;
      BYTES_PER_VALUE = 2;
      EEPROM_START = EEPROM_CALIBRATION_CLT;
      break;
    case 1:
      //Inlet air temp table
      pnt_TargetTable = (byte *)&iatCalibrationTable;
      OFFSET = CALIBRATION_TEMPERATURE_OFFSET;
      DIVISION_FACTOR = 10;
      BYTES_PER_VALUE = 2;
      EEPROM_START = EEPROM_CALIBRATION_IAT;
      break;
    case 2:
      //O2 table
      pnt_TargetTable = (byte *)&o2CalibrationTable;
      OFFSET = 0;
      DIVISION_FACTOR = 1;
      BYTES_PER_VALUE = 1;
      EEPROM_START = EEPROM_CALIBRATION_O2;
      break;

    default:
      OFFSET = 0;
      pnt_TargetTable = (byte *)&o2CalibrationTable;
      DIVISION_FACTOR = 1;
      BYTES_PER_VALUE = 1;
      EEPROM_START = EEPROM_CALIBRATION_O2;
      break; //Should never get here, but if we do, just fail back to main loop
  }

  //1024 value pairs are sent. We have to receive them all, but only use every second one (We only store 512 calibratino table entries to save on EEPROM space)
  //The values are sent as 2 byte ints, but we convert them to single bytes. Any values over 255 are capped at 255.
  int tempValue;
  byte tempBuffer[2];
  bool every2nd = true;
  int x;
  int counter = 0;
  pinMode(LED_BUILTIN, OUTPUT); //pinMode(13, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW); //digitalWrite(13, LOW);
  for (x = 0; x < 1024; x++)
  {
    //UNlike what is listed in the protocol documentation, the O2 sensor values are sent as bytes rather than ints
    if (BYTES_PER_VALUE == 1)
    {
      while ( Serial.available() < 1 ) {}
      tempValue = Serial.read();
    }
    else
    {
      while ( Serial.available() < 2 ) {}
      tempBuffer[0] = Serial.read();
      tempBuffer[1] = Serial.read();

      tempValue = div(int(word(tempBuffer[1], tempBuffer[0])), DIVISION_FACTOR).quot; //Read 2 bytes, convert to word (an unsigned int), convert to signed int. These values come through * 10 from Tuner Studio
      tempValue = ((tempValue - 32) * 5) / 9; //Convert from F to C
    }
    tempValue = tempValue + OFFSET;

    if (every2nd) //Only use every 2nd value
    {
      if (tempValue > 255) {
        tempValue = 255;  // Cap the maximum value to prevent overflow when converting to byte
      }
      if (tempValue < 0) {
        tempValue = 0;
      }

      pnt_TargetTable[(x / 2)] = (byte)tempValue;

      //From TS3.x onwards, the EEPROM must be written here as TS restarts immediately after the process completes which is before the EEPROM write completes
      int y = EEPROM_START + (x / 2);
      EEPROM.update(y, (byte)tempValue);

      every2nd = false;
      #if defined(CORE_STM32)
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      #else
        analogWrite(LED_BUILTIN, (counter % 50) ); //analogWrite(13, (counter % 50) );
      #endif
      counter++;
    }
    else {
      every2nd = true;
    }

  }

}

/*
Send 256 tooth log entries
 * if useChar is true, the values are sent as chars to be printed out by a terminal emulator
 * if useChar is false, the values are sent as a 2 byte integer which is readable by TunerStudios tooth logger
*/
void sendToothLog(bool useChar)
{
  //We need TOOTH_LOG_SIZE number of records to send to TunerStudio. If there aren't that many in the buffer then we just return and wait for the next call
  if (toothHistoryIndex >= TOOTH_LOG_SIZE) //Sanity check. Flagging system means this should always be true
  {
    unsigned int tempToothHistory[TOOTH_LOG_BUFFER]; //Create a temporary array that will contain a copy of what is in the main toothHistory array

    //Copy the working history into the temporary buffer array. This is done so that, if the history loops whilst the values are being sent over serial, it doesn't affect the values
    memcpy( (void*)tempToothHistory, (void*)toothHistory, sizeof(tempToothHistory) );
    toothHistoryIndex = 0; //Reset the history index

    //Loop only needs to go to half the buffer size
    if (useChar)
    {
      for (int x = 0; x < TOOTH_LOG_SIZE; x++)
      {
        Serial.println(tempToothHistory[x]);
      }
    }
    else
    {
      for (int x = 0; x < TOOTH_LOG_SIZE; x++)
      {
        Serial.write(highByte(tempToothHistory[x]));
        Serial.write(lowByte(tempToothHistory[x]));
      }
      BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    }
    toothLogRead = true;
  }
}

void testComm()
{
  Serial.write(1);
  return;
}

void commandButtons()
{
  switch (cmdCombined)
  {
    case 256: // cmd is stop
      BIT_CLEAR(currentStatus.testOutputs, 1);
      digitalWrite(pinInjector1, LOW);
      digitalWrite(pinInjector2, LOW);
      digitalWrite(pinInjector3, LOW);
      digitalWrite(pinInjector4, LOW);
      digitalWrite(pinCoil1, LOW);
      digitalWrite(pinCoil2, LOW);
      digitalWrite(pinCoil3, LOW);
      digitalWrite(pinCoil4, LOW);
      break;

    case 257: // cmd is enable
      // currentStatus.testactive = 1;
      BIT_SET(currentStatus.testOutputs, 1);
      break;
    case 513: // cmd group is for injector1 on actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){ digitalWrite(pinInjector1, HIGH); }
      break;
    case 514: // cmd group is for injector1 off actions
      if( BIT_CHECK(currentStatus.testOutputs, 1) ){digitalWrite(pinInjector1, LOW);}
      break;
    case 515: // cmd group is for injector1 50% dc actions
      //for (byte dcloop = 0; dcloop < 11; dcloop++)
      //{
      //  digitalWrite(pinInjector1, HIGH);
      //  delay(500);
      //  digitalWrite(pinInjector1, LOW);
      //  delay(500);
      //}
      break;
    case 516: // cmd group is for injector2 on actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ digitalWrite(pinInjector2, HIGH); }
      break;
    case 517: // cmd group is for injector2 off actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ digitalWrite(pinInjector2, LOW); }
      break;
    case 518: // cmd group is for injector2 50%dc actions

      break;
    case 519: // cmd group is for injector3 on actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinInjector3, HIGH); }
      break;
    case 520: // cmd group is for injector3 off actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinInjector3, LOW); }
      break;
    case 521: // cmd group is for injector3 50%dc actions

      break;
    case 522: // cmd group is for injector4 on actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ digitalWrite(pinInjector4, HIGH); }
      break;
    case 523: // cmd group is for injector4 off actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ digitalWrite(pinInjector4, LOW); }
      break;
    case 524: // cmd group is for injector4 50% dc actions

      break;
    case 769: // cmd group is for spark1 on actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil1, HIGH); }
      break;
    case 770: // cmd group is for spark1 off actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil1, LOW); }
      break;
    case 771: // cmd group is for spark1 50%dc actions

      break;
    case 772: // cmd group is for spark2 on actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil2, HIGH); }
      break;
    case 773: // cmd group is for spark2 off actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil2, LOW); }
      break;
    case 774: // cmd group is for spark2 50%dc actions

      break;
    case 775: // cmd group is for spark3 on actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil3, HIGH); }
      break;
    case 776: // cmd group is for spark3 off actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil3, LOW); }
      break;
    case 777: // cmd group is for spark3 50%dc actions

      break;
    case 778: // cmd group is for spark4 on actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil4, HIGH); }
      break;
    case 779: // cmd group is for spark4 off actions
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil4, LOW); }
      break;
    case 780: // cmd group is for spark4 50%dc actions

    default:
      break;
  }
}
