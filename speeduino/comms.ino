/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
This is called when a command is received over serial from TunerStudio / Megatune
It parses the command and calls the relevant function
A detailed description of each call can be found at: http://www.msextra.com/doc/ms1extra/COM_RS232.htm
*/
//#include "comms.h"
//#include "globals.h"
//#include "storage.h"

void command()
{
  switch (Serial.read())
  {
    case 'A': // send x bytes of realtime values
      sendValues(packetSize, 0);   //send values to serial0
      break;

    case 'B': // Burn current values to eeprom
      writeConfig();
      break;

    case 'C': // test communications. This is used by Tunerstudio to see whether there is an ECU on a given serial port
      testComm();
      break;

    case 'E': // receive command button commands
      while (Serial.available() == 0) { }
      cmdGroup = Serial.read();
      while (Serial.available() == 0) { }
      cmdValue = Serial.read();
      cmdCombined = word(cmdGroup, cmdValue);
      if (currentStatus.RPM == 0) { commandButtons(); }

      break;

    case 'L': // List the contents of current page in human readable form
      sendPage(true);
      break;

    case 'N': // Displays a new line.  Like pushing enter in a text editor
      Serial.println();
      break;

    case 'P': // set the current page
      //A 2nd byte of data is required after the 'P' specifying the new page number.
      //This loop should never need to run as the byte should already be in the buffer, but is here just in case
      while (Serial.available() == 0) { }
      currentPage = Serial.read();
      if (currentPage >= '0') {//This converts the ascii number char into binary
        currentPage -= '0';
      }
      if (currentPage == veMapPage || currentPage == ignMapPage || currentPage == afrMapPage) { // Detecting if the current page is a table/map
        isMap = true;
      }
      else {
        isMap = false;
      }
      break;

    case 'R': // send 39 bytes of realtime values
      sendValues(39,0);
      break;

    case 'F': // send serial protocol version
      Serial.print("001");
      break;

    case 'S': // send code version
      Serial.print("Speeduino 2017.03");
      currentStatus.secl = 0; //This is required in TS3 due to its stricter timings
      break;

    case 'Q': // send code version
      Serial.print("speeduino 201703");
     break;

    case 'V': // send VE table and constants in binary
      sendPage(false);
      break;

    case 'W': // receive new VE obr constant at 'W'+<offset>+<newbyte>
      int valueOffset; //cannot use offset as a variable name, it is a reserved word for several teensy libraries
      while (Serial.available() == 0) { }

      if (isMap)
      {
        byte offset1, offset2;
        offset1 = Serial.read();
        while (Serial.available() == 0) { }
        offset2 = Serial.read();
        valueOffset = word(offset2, offset1);
      }
      else
      {
        valueOffset = Serial.read();
      }
      while (Serial.available() == 0) { }

      receiveValue(valueOffset, Serial.read());
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

    case 'Z': //Totally non-standard testing function. Will be removed once calibration testing is completed. This function takes 1.5kb of program space! :S
      digitalWrite(pinInjector1, HIGH);
      digitalWrite(pinInjector2, HIGH);
      delay(20);
      digitalWrite(pinInjector1, LOW);
      digitalWrite(pinInjector2, LOW);
      return;
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
      break;

    case 'T': //Send 256 tooth log entries to Tuner Studios tooth logger
      sendToothLog(false); //Sends tooth log values as ints
      break;

    case 'r': //Send 256 tooth log entries to a terminal emulator
      sendToothLog(true); //Sends tooth log values as chars
      break;



    case '?':
      Serial.println
      (F(
         "\n"
         "===Command Help===\n\n"
         "All commands are single character and are concatenated with their parameters \n"
         "without spaces. Some parameters are binary and cannot be entered through this \n"
         "prompt by conventional means. \n"
         "Syntax:  <command>+<parameter1>+<parameter2>+<parameterN>\n\n"
         "===List of Commands===\n\n"
         "A - Displays 31 bytes of currentStatus values in binary (live data)\n"
         "B - Burn current map and configPage values to eeprom\n"
         "C - Test COM port.  Used by Tunerstudio to see whether an ECU is on a given serial \n"
         "    port. Returns a binary number.\n"
         "L - Displays map page (aka table) or configPage values.  Use P to change page (not \n"
         "    every page is a map)\n"
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

      break;

    default:
      break;
  }
}

/*
This function returns the current values of a fixed group of variables
*/
void sendValues(int packetlength, byte portNum)
{
  byte response[packetlength];

  if (portNum == 3)
  {
    //CAN serial
    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //ATmega2561 does not have Serial3
      Serial3.write("A");         //confirm cmd type
      Serial3.write(packetlength);      //confirm no of byte to be sent
    #elif defined(CORE_STM32)
      Serial2.write("A");         //confirm cmd type
      Serial2.write(packetlength);      //confirm no of byte to be sent
    #endif
  }
  else
  {
    if(requestCount == 0) { currentStatus.secl = 0; }
    requestCount++;
  }

  currentStatus.spark ^= (-currentStatus.hasSync ^ currentStatus.spark) & (1 << BIT_SPARK_SYNC); //Set the sync bit of the Spark variable to match the hasSync variable

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
  response[16] = currentStatus.baro; //Barometer value
  response[17] = currentStatus.corrections; //Total GammaE (%)
  response[18] = currentStatus.VE; //Current VE 1 (%)
  response[19] = currentStatus.afrTarget;
  response[20] = (byte)(currentStatus.PW1 / 100); //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
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

  response[33] = currentStatus.ethanolPct; //Flex sensor value (or 0 if not used)
  response[34] = currentStatus.flexCorrection; //Flex fuel correction (% above or below 100)
  response[35] = currentStatus.flexIgnCorrection; //Ignition correction (Increased degrees of advance) for flex fuel
  response[36] = getNextError();
  response[37] = currentStatus.boostTarget;
  response[38] = currentStatus.boostDuty;
  response[39] = currentStatus.idleLoad;
  response[40] = currentStatus.testOutputs;

//cli();
  if (portNum == 0) { Serial.write(response, (size_t)packetlength); }
  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //ATmega2561 does not have Serial3
    else if (portNum == 3) { Serial3.write(response, (size_t)packetlength); }
  #elif defined(CORE_STM32)
    else if (portNum == 3) { Serial2.write(response, (size_t)packetlength); }
  #endif
//sei();
  return;
}

void receiveValue(int valueOffset, byte newValue)
{

  void* pnt_configPage;//This only stores the address of the value that it's pointing to and not the max size

  switch (currentPage)
  {
    case veMapPage:
      if (valueOffset < 256) //New value is part of the fuel map
      {
        fuelTable.values[15 - valueOffset / 16][valueOffset % 16] = newValue;
        return;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          fuelTable.axisX[(valueOffset - 256)] = ((int)(newValue) * TABLE_RPM_MULTIPLIER); //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        }
        else
        {
          //Y Axis
          valueOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order (Due to us using (0,0) in the top left rather than bottom right
          fuelTable.axisY[valueOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
        return;
      }
      break;

    case veSetPage:
      pnt_configPage = &configPage1; //Setup a pointer to the relevant config page
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < page_size)
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case ignMapPage: //Ignition settings page (Page 2)
      if (valueOffset < 256) //New value is part of the ignition map
      {
        ignitionTable.values[15 - valueOffset / 16][valueOffset % 16] = newValue;
        return;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          ignitionTable.axisX[(valueOffset - 256)] = (int)(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct
        }
        else
        {
          //Y Axis
          valueOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order
          ignitionTable.axisY[valueOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
        return;
      }

    case ignSetPage:
      pnt_configPage = &configPage2;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < page_size)
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case afrMapPage: //Air/Fuel ratio target settings page
      if (valueOffset < 256) //New value is part of the afr map
      {
        afrTable.values[15 - valueOffset / 16][valueOffset % 16] = newValue;
        return;
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
          valueOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order
          afrTable.axisY[valueOffset] = int(newValue) * TABLE_LOAD_MULTIPLIER;

        }
        return;
      }

    case afrSetPage:
      pnt_configPage = &configPage3;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < page_size)
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case iacPage: //Idle Air Control settings page (Page 4)
      pnt_configPage = &configPage4;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < page_size)
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;
    case boostvvtPage: //Boost and VVT maps (8x8)
      if (valueOffset < 64) //New value is part of the boost map
      {
        boostTable.values[7 - valueOffset / 8][valueOffset % 8] = newValue;
        return;
      }
      else if (valueOffset < 72) //New value is on the X (RPM) axis of the boost table
      {
        boostTable.axisX[(valueOffset - 64)] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        return;
      }
      else if (valueOffset < 80) //New value is on the Y (TPS) axis of the boost table
      {
        boostTable.axisY[(7 - (valueOffset - 72))] = int(newValue); //TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
        return;
      }
      else if (valueOffset < 144) //New value is part of the vvt map
      {
        valueOffset = valueOffset - 80;
        vvtTable.values[7 - valueOffset / 8][valueOffset % 8] = newValue;
        return;
      }
      else if (valueOffset < 152) //New value is on the X (RPM) axis of the vvt table
      {
        valueOffset = valueOffset - 144;
        vvtTable.axisX[valueOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        return;
      }
      else //New value is on the Y (Load) axis of the vvt table
      {
        valueOffset = valueOffset - 152;
        vvtTable.axisY[(7 - valueOffset)] = int(newValue); //TABLE_LOAD_MULTIPLIER is NOT used for vvt as it is TPS based (0-100)
        return;
      }
    case seqFuelPage:
      if (valueOffset < 36) { trim1Table.values[5 - valueOffset / 6][valueOffset % 6] = newValue; } //Trim1 values
      else if (valueOffset < 42) { trim1Table.axisX[(valueOffset - 36)] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the trim1 table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 48) { trim1Table.axisY[(5 - (valueOffset - 42))] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (TPS) axis of the boost table
      //Trim table 2
      else if (valueOffset < 84) { valueOffset = valueOffset - 48; trim2Table.values[5 - valueOffset / 6][valueOffset % 6] = newValue; } //New value is part of the trim2 map
      else if (valueOffset < 90) { valueOffset = valueOffset - 84; trim2Table.axisX[valueOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 96) { valueOffset = valueOffset - 90; trim2Table.axisY[(5 - valueOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 3
      else if (valueOffset < 132) { valueOffset = valueOffset - 96; trim3Table.values[5 - valueOffset / 6][valueOffset % 6] = newValue; } //New value is part of the trim2 map
      else if (valueOffset < 138) { valueOffset = valueOffset - 132; trim3Table.axisX[valueOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 144) { valueOffset = valueOffset - 138; trim3Table.axisY[(5 - valueOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 4
      else if (valueOffset < 180) { valueOffset = valueOffset - 144; trim4Table.values[5 - valueOffset / 6][valueOffset % 6] = newValue; } //New value is part of the trim2 map
      else if (valueOffset < 186) { valueOffset = valueOffset - 180; trim4Table.axisX[valueOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 192) { valueOffset = valueOffset - 186; trim4Table.axisY[(5 - valueOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table

      break;

    case canbusPage:
      pnt_configPage = &configPage10;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[currentPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
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
useChar - If true, all values are send as chars, this is for the serial command line interface. TunerStudio expects data as raw values, so this must be set false in that case
*/
void sendPage(bool useChar)
{
  void* pnt_configPage;
  struct table3D currentTable;
  byte currentTitleIndex = 0;// This corresponds to the count up to the first char of a string in pageTitles

  switch (currentPage)
  {
    case veMapPage:
      {
        currentTitleIndex = 0;
        currentTable = fuelTable;
        break;
      }

    case veSetPage:
      {
        // currentTitleIndex = 27;
        if (useChar)
        {
          // To Display Values from Config Page 1
          // When casting to the __FlashStringHelper type Serial.println uses the same subroutine as when using the F macro
          Serial.println((const __FlashStringHelper *)&pageTitles[27]);//27 is the index to the first char in the second sting in pageTitles
          // The following loop displays in human readable form of all byte values in config page 1 up to but not including the first array.
          // incrementing void pointers is cumbersome. Thus we have "pnt_configPage = (byte *)pnt_configPage + 1"
          for (pnt_configPage = &configPage1; pnt_configPage < &configPage1.wueValues[0]; pnt_configPage = (byte *)pnt_configPage + 1) Serial.println(*((byte *)pnt_configPage));
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
          for (pnt_configPage = &configPage1.inj1Ang; pnt_configPage < (unsigned int *)&configPage1.inj4Ang + 1; pnt_configPage = (unsigned int *)pnt_configPage + 1) Serial.println(*((unsigned int *)pnt_configPage));
          // Following loop displays byte values between the unsigned ints
          for (pnt_configPage = (unsigned int *)&configPage1.inj4Ang + 1; pnt_configPage < &configPage1.mapMax; pnt_configPage = (byte *)pnt_configPage + 1) Serial.println(*((byte *)pnt_configPage));
          Serial.println(configPage1.mapMax);
          // Following loop displays remaining byte values of the page
          for (pnt_configPage = (unsigned int *)&configPage1.mapMax + 1; pnt_configPage < (byte *)&configPage1 + page_size; pnt_configPage = (byte *)pnt_configPage + 1) Serial.println(*((byte *)pnt_configPage));
          return;
        }
        else pnt_configPage = &configPage1; //Create a pointer to Page 1 in memory
        break;
      }

    case ignMapPage:
      {
        currentTitleIndex = 42;// the index to the first char of the third string in pageTitles
        currentTable = ignitionTable;
        break;
      }

    case ignSetPage:
      {
        //currentTitleIndex = 56;
        if (useChar)
        {
          //To Display Values from Config Page 2
          Serial.println((const __FlashStringHelper *)&pageTitles[56]);
          Serial.println(configPage2.triggerAngle);// configPsge2.triggerAngle is an int so just display it without complication
          // Following loop displays byte values after that first int up to but not including the first array in config page 2
          for (pnt_configPage = (int *)&configPage2 + 1; pnt_configPage < &configPage2.taeBins[0]; pnt_configPage = (byte *)pnt_configPage + 1) Serial.println(*((byte *)pnt_configPage));
          for (byte y = 2; y; y--)// Displaying two equal sized arrays
          {
            byte * currentVar;// A placeholder for each array
            if (y == 2) {
              currentVar = configPage2.taeBins;
            }
            else {
              currentVar = configPage2.taeValues;
            }

            for (byte x = 4; x; x--)
            {
              Serial.print(currentVar[4 - x]);
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
          for (pnt_configPage = (byte *)&configPage2.dwellCorrectionValues[5] + 1; pnt_configPage < (byte *)&configPage2 + page_size; pnt_configPage = (byte *)pnt_configPage + 1)
          {
            Serial.println(*((byte *)pnt_configPage));// Displaying remaining byte values of the page
          }
          return;
        }
        else pnt_configPage = &configPage2; //Create a pointer to Page 2 in memory
        break;
      }

    case afrMapPage:
      {
        currentTitleIndex = 71;//Array index to next string
        currentTable = afrTable;
        break;
      }

    case afrSetPage:
      {
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
            if (y == 2) currentVar = configPage3.airDenBins;
            else currentVar = configPage3.airDenRates;
            for (byte x = 9; x; x--)
            {
              Serial.print(currentVar[9 - x]);
              Serial.print(' ');
            }
            Serial.println();
          }
          // Following loop displays the remaining byte values of the page
          for (pnt_configPage = (byte *)&configPage3.airDenRates[8] + 1; pnt_configPage < (byte *)&configPage3 + page_size; pnt_configPage = (byte *)pnt_configPage + 1)
          {
            Serial.println(*((byte *)pnt_configPage));
          }
          return;
        }
        else pnt_configPage = &configPage3; //Create a pointer to Page 3 in memory
        break;
      }

    case iacPage:
      {
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
              case 1: currentVar = configPage4.iacBins; break;
              case 2: currentVar = configPage4.iacOLPWMVal; break;
              case 3: currentVar = configPage4.iacOLStepVal; break;
              case 4: currentVar = configPage4.iacCLValues; break;
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
              case 1: currentVar = configPage4.iacCrankBins; break;
              case 2: currentVar = configPage4.iacCrankDuty; break;
              case 3: currentVar = configPage4.iacCrankSteps; break;
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
          for (pnt_configPage = (byte *)&configPage4.iacCrankBins[3] + 1; pnt_configPage < (byte *)&configPage4 + page_size; pnt_configPage = (byte *)pnt_configPage + 1) Serial.println(*((byte *)pnt_configPage));
          return;
        }
        else pnt_configPage = &configPage4; //Create a pointer to Page 4 in memory
        break;
      }

    case boostvvtPage:
      {
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
          for (int x = 0; x < 64; x++) { response[x] = boostTable.values[7 - x / 8][x % 8]; }
          for (int x = 64; x < 72; x++) { response[x] = byte(boostTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 72; y < 80; y++) { response[y] = byte(boostTable.axisY[7 - (y - 72)]); }
          //VVT table
          for (int x = 0; x < 64; x++) { response[x + 80] = vvtTable.values[7 - x / 8][x % 8]; }
          for (int x = 64; x < 72; x++) { response[x + 80] = byte(vvtTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 72; y < 80; y++) { response[y + 80] = byte(vvtTable.axisY[7 - (y - 72)]); }
          Serial.write((byte *)&response, sizeof(response));
          return;
        }
        break;
      }
    case seqFuelPage:
      {
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
            return;
          //Do.... Something?
        }
        else
        {
          //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
          byte response[192]; //Bit hacky, but the size is: (6x6 + 6 + 6) * 4 = 192

          //trim1 table
          for (int x = 0; x < 36; x++) { response[x] = trim1Table.values[5 - x / 6][x % 6]; }
          for (int x = 36; x < 42; x++) { response[x] = byte(trim1Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 42; y < 48; y++) { response[y] = byte(trim1Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }
          //trim2 table
          for (int x = 0; x < 36; x++) { response[x + 48] = trim2Table.values[5 - x / 6][x % 6]; }
          for (int x = 36; x < 42; x++) { response[x + 48] = byte(trim2Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 42; y < 48; y++) { response[y + 48] = byte(trim2Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }
          //trim3 table
          for (int x = 0; x < 36; x++) { response[x + 96] = trim3Table.values[5 - x / 6][x % 6]; }
          for (int x = 36; x < 42; x++) { response[x + 96] = byte(trim3Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 42; y < 48; y++) { response[y + 96] = byte(trim3Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }
          //trim4 table
          for (int x = 0; x < 36; x++) { response[x + 144] = trim4Table.values[5 - x / 6][x % 6]; }
          for (int x = 36; x < 42; x++) { response[x + 144] = byte(trim4Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
          for (int y = 42; y < 48; y++) { response[y + 144] = byte(trim4Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }
          Serial.write((byte *)&response, sizeof(response));
          return;
        }
        break;
      }

    case canbusPage:
      {
        //currentTitleIndex = 141;
        if (useChar)
        {
          //To Display Values from Config Page 10
          Serial.println((const __FlashStringHelper *)&pageTitles[141]);//special typecasting to enable suroutine that the F macro uses
          for (pnt_configPage = &configPage10; pnt_configPage < ((byte *)pnt_configPage + 128); pnt_configPage = (byte *)pnt_configPage + 1)
          {
            Serial.println(*((byte *)pnt_configPage));// Displaying byte values of config page 3 up to but not including the first array
          }
          return;
        }
        else pnt_configPage = &configPage10; //Create a pointer to Page 10 in memory
        break;
      }

    default:
      {
        Serial.println(F("\nPage has not been implemented yet. Change to another page."));
        return;
        break;
      }
  }
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
          for (int x = 0; x < currentTable.xSize; x++)
          {
            byte value = currentTable.values[y][x];
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
    }
    else
    {
      //Need to perform a translation of the values[yaxis][xaxis] into the MS expected format
      //MS format has origin (0,0) in the bottom left corner, we use the top left for efficiency reasons
      byte response[map_page_size];

      for (int x = 0; x < 256; x++) { response[x] = currentTable.values[15 - x / 16][x % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
      //loop();
      for (int x = 256; x < 272; x++) { response[x] = byte(currentTable.axisX[(x - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
      //loop();
      for (int y = 272; y < 288; y++) { response[y] = byte(currentTable.axisY[15 - (y - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
      //loop();
      Serial.write((byte *)&response, sizeof(response));
    }
  }
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
    byte response[npage_size[currentPage]];
    for (byte x = 0; x < npage_size[currentPage]; x++)
    {
      response[x] = *((byte *)pnt_configPage + x); //Each byte is simply the location in memory of the configPage + the offset + the variable number (x)
      //if ( (x & 31) == 1) { loop(); } //Every 32 loops, do a manual call to loop() to ensure that there is no misses
    }

    Serial.write((byte *)&response, sizeof(response));
    // }
  }
  return;
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
      return; //Should never get here, but if we do, just fail back to main loop
      //pnt_TargetTable = (table2D *)&o2CalibrationTable;
      //break;
  }

  //1024 value pairs are sent. We have to receive them all, but only use every second one (We only store 512 calibratino table entries to save on EEPROM space)
  //The values are sent as 2 byte ints, but we convert them to single bytes. Any values over 255 are capped at 255.
  int tempValue;
  byte tempBuffer[2];
  bool every2nd = true;
  int x;
  int counter = 0;
  pinMode(13, OUTPUT);

  digitalWrite(13, LOW);
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
      analogWrite(13, (counter % 50) );
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
  if (toothHistoryIndex < TOOTH_LOG_SIZE) {
    return;  //This should no longer ever occur since the flagging system was put in place
  }
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
    BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_TOOTHLOG1READY);
  }
  toothLogRead = true;
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
      if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinInjector1, HIGH);}
      break;
    case 514: // cmd group is for injector1 off actions
      if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinInjector1, LOW);}
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
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinInjector2, HIGH);}
      break;
    case 517: // cmd group is for injector2 off actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinInjector2, LOW);}
      break;
    case 518: // cmd group is for injector2 50%dc actions

      break;
    case 519: // cmd group is for injector3 on actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinInjector3, HIGH);}
      break;
    case 520: // cmd group is for injector3 off actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinInjector3, LOW);}
      break;
    case 521: // cmd group is for injector3 50%dc actions

      break;
    case 522: // cmd group is for injector4 on actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinInjector4, HIGH);}
      break;
    case 523: // cmd group is for injector4 off actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinInjector4, LOW);}
      break;
    case 524: // cmd group is for injector4 50% dc actions

      break;
    case 769: // cmd group is for spark1 on actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinCoil1, HIGH);}
      break;
    case 770: // cmd group is for spark1 off actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinCoil1, LOW);}
      break;
    case 771: // cmd group is for spark1 50%dc actions

      break;
    case 772: // cmd group is for spark2 on actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinCoil2, HIGH);}
      break;
    case 773: // cmd group is for spark2 off actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinCoil2, LOW);}
      break;
    case 774: // cmd group is for spark2 50%dc actions

      break;
    case 775: // cmd group is for spark3 on actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinCoil3, HIGH);}
      break;
    case 776: // cmd group is for spark3 off actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinCoil3, LOW);}
      break;
    case 777: // cmd group is for spark3 50%dc actions

      break;
    case 778: // cmd group is for spark4 on actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinCoil4, HIGH);}
      break;
    case 779: // cmd group is for spark4 off actions
        if(BIT_CHECK(currentStatus.testOutputs, 1)){digitalWrite(pinCoil4, LOW);}
      break;
    case 780: // cmd group is for spark4 50%dc actions

      break;
  }
}
