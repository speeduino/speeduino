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
    case 'A': // send 22 bytes of realtime values
      sendValues(22);
      break;

    case 'B': // Burn current values to eeprom
      writeConfig();
      break;

    case 'C': // test communications. This is used by Tunerstudio to see whether there is an ECU on a given serial port
      testComm();
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
      if (currentPage == veMapPage || currentPage == ignMapPage || currentPage == afrMapPage) {// Detecting if the current page is a table/map
        isMap = true;
      }
      else {
        isMap = false;
      }
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

    case 'V': // send VE table and constants in binary
      sendPage(false);
      break;

    case 'W': // receive new VE or constant at 'W'+<offset>+<newbyte>
      int offset;
      while (Serial.available() == 0) { }

      if (isMap)
      {
        byte offset1, offset2;
        offset1 = Serial.read();
        while (Serial.available() == 0) { }
        offset2 = Serial.read();
        offset = word(offset2, offset1);
      }
      else
      {
        offset = Serial.read();
      }
      while (Serial.available() == 0) { }

      receiveValue(offset, Serial.read());
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
void sendValues(int length)
{
  byte packetSize = 33;
  byte response[packetSize];

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
  response[11] = 0x00; //Air Correction (%)
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
  response[29] = (byte)(currentStatus.dwell / 100);
  response[30] = currentStatus.O2_2; //O2
  
  //rpmDOT must be sent as a signed integer
  response[31] = lowByte(currentStatus.rpmDOT);
  response[32] = highByte(currentStatus.rpmDOT);

  Serial.write(response, (size_t)packetSize);

  //if(Serial.available()) { command(); }
  //Serial.flush();
  return;
}

void receiveValue(int offset, byte newValue)
{

  void* pnt_configPage;//This only stores the address of the value that it's pointing to and not the max size

  switch (currentPage)
  {
    case veMapPage:
      if (offset < 256) //New value is part of the fuel map
      {
        fuelTable.values[15 - offset / 16][offset % 16] = newValue;
        return;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (offset < 272)
        {
          //X Axis
          fuelTable.axisX[(offset - 256)] = ((int)(newValue) * 100); //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct
        }
        else
        {
          //Y Axis
          offset = 15 - (offset - 272); //Need to do a translation to flip the order (Due to us using (0,0) in the top left rather than bottom right
          fuelTable.axisY[offset] = (int)(newValue);
        }
        return;
      }
      break;

    case veSetPage:
      pnt_configPage = &configPage1; //Setup a pointer to the relevant config page
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if ( offset < page_size)
      {
        *((byte *)pnt_configPage + (byte)offset) = newValue; //Need to subtract 80 because the map and bins (Which make up 80 bytes) aren't part of the config pages
      }
      break;

    case ignMapPage: //Ignition settings page (Page 2)
      if (offset < 256) //New value is part of the ignition map
      {
        ignitionTable.values[15 - offset / 16][offset % 16] = newValue;
        return;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (offset < 272)
        {
          //X Axis
          ignitionTable.axisX[(offset - 256)] = (int)(newValue) * int(100); //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct
        }
        else
        {
          //Y Axis
          offset = 15 - (offset - 272); //Need to do a translation to flip the order
          ignitionTable.axisY[offset] = (int)(newValue);
        }
        return;
      }

    case ignSetPage:
      pnt_configPage = &configPage2;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if ( offset < page_size)
      {
        *((byte *)pnt_configPage + (byte)offset) = newValue; //Need to subtract 80 because the map and bins (Which make up 80 bytes) aren't part of the config pages
      }
      break;

    case afrMapPage: //Air/Fuel ratio target settings page
      if (offset < 256) //New value is part of the afr map
      {
        afrTable.values[15 - offset / 16][offset % 16] = newValue;
        return;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (offset < 272)
        {
          //X Axis
          afrTable.axisX[(offset - 256)] = int(newValue) * int(100); //The RPM values sent by megasquirt are divided by 100, need to multiply it back by 100 to make it correct
        }
        else
        {
          //Y Axis
          offset = 15 - (offset - 272); //Need to do a translation to flip the order
          afrTable.axisY[offset] = int(newValue);

        }
        return;
      }

    case afrSetPage:
      pnt_configPage = &configPage3;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if ( offset < page_size)
      {
        *((byte *)pnt_configPage + (byte)offset) = newValue; //Need to subtract 80 because the map and bins (Which make up 80 bytes) aren't part of the config pages
      }
      break;

    case iacPage: //Idle Air Control settings page (Page 4)
      pnt_configPage = &configPage4;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if ( offset < page_size)
      {
        *((byte *)pnt_configPage + (byte)offset) = newValue;
      }
      break;
    case boostvvtPage: //Boost and VVT maps (8x8)
      if (offset < 64) //New value is part of the boost map
      {
        boostTable.values[7 - offset / 8][offset % 8] = newValue;
        return;
      }
      else if (offset < 72) //New value is on the X (RPM) axis of the boost table
      {
        boostTable.axisX[(offset - 64)] = int(newValue) * int(100); //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct
        return;
      }
      else if (offset < 80) //New value is on the Y (TPS) axis of the boost table
      {
        boostTable.axisY[(7 - (offset - 72))] = int(newValue);
        return;
      }
      else if (offset < 144) //New value is part of the vvt map
      {
        offset = offset - 80;
        vvtTable.values[7 - offset / 8][offset % 8] = newValue;
        return;
      }
      else if (offset < 152) //New value is on the X (RPM) axis of the vvt table
      {
        offset = offset - 144;
        vvtTable.axisX[offset] = int(newValue) * int(100); //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct
        return;
      }
      else //New value is on the Y (Load) axis of the vvt table
      {
        offset = offset - 152;
        vvtTable.axisY[(7 - offset)] = int(newValue);
        return;
      }
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
        if(!useChar)
        {
          //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format        
          byte response[160]; //Bit hacky, but the size is: (8x8 + 8 + 8) + (8x8 + 8 + 8) = 160

          //Boost table
          for (int x = 0; x < 64; x++) { response[x] = boostTable.values[7 - x / 8][x % 8]; }
          for (int x = 64; x < 72; x++) { response[x] = byte(boostTable.axisX[(x - 64)] / 100); }
          for (int y = 72; y < 80; y++) { response[y] = byte(boostTable.axisY[7 - (y - 72)]); }
          //VVT table
          for (int x = 0; x < 64; x++) { response[x + 80] = vvtTable.values[7 - x / 8][x % 8]; }
          for (int x = 64; x < 72; x++) { response[x + 80] = byte(vvtTable.axisX[(x - 64)] / 100); }
          for (int y = 72; y < 80; y++) { response[y + 80] = byte(vvtTable.axisY[7 - (y - 72)]); }
          Serial.write((byte *)&response, sizeof(response));
          return;
        }
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
      const char spaceChar = ' ';
      /*while(pageTitles[currentTitleIndex])
      {
       Serial.print(pageTitles[currentTitleIndex]);
       currentTitleIndex++;
      }*/
      Serial.println((const __FlashStringHelper *)&pageTitles[currentTitleIndex]);// F macro hack
      Serial.print(F("\n    "));
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
      for (int y = 0; y < currentTable.ySize; y++)
      {
        Serial.print(byte(currentTable.axisY[y]));// Vertical Bins
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
    }
    else
    {
      //Need to perform a translation of the values[yaxis][xaxis] into the MS expected format
      //MS format has origin (0,0) in the bottom left corner, we use the top left for efficiency reasons
      byte response[map_page_size];

      for (int x = 0; x < 256; x++) { response[x] = currentTable.values[15 - x / 16][x % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged
      for (int x = 256; x < 272; x++) { response[x] = byte(currentTable.axisX[(x - 256)] / 100); }  //RPM Bins for VE table (Need to be dvidied by 100)
      for (int y = 272; y < 288; y++) { response[y] = byte(currentTable.axisY[15 - (y - 272)]); } //MAP or TPS bins for VE table
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
    byte response[page_size];
    for (byte x = 0; x < page_size; x++)
    {
      response[x] = *((byte *)pnt_configPage + x); //Each byte is simply the location in memory of the configPage + the offset + the variable number (x)
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
  int OFFSET, DIVISION_FACTOR, BYTES_PER_VALUE;

  switch (tableID)
  {
    case 0:
      //coolant table
      pnt_TargetTable = (byte *)&cltCalibrationTable;
      OFFSET = CALIBRATION_TEMPERATURE_OFFSET; //
      DIVISION_FACTOR = 10;
      BYTES_PER_VALUE = 2;
      break;
    case 1:
      //Inlet air temp table
      pnt_TargetTable = (byte *)&iatCalibrationTable;
      OFFSET = CALIBRATION_TEMPERATURE_OFFSET;
      DIVISION_FACTOR = 10;
      BYTES_PER_VALUE = 2;
      break;
    case 2:
      //O2 table
      pnt_TargetTable = (byte *)&o2CalibrationTable;
      OFFSET = 0;
      DIVISION_FACTOR = 1;
      BYTES_PER_VALUE = 1;
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
      int y = EEPROM_CALIBRATION_O2 + counter;

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
}

void testComm()
{
  Serial.write(1);
  return;
}
