/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "globals.h"
#include "comms.h"
#include "cancomms.h"
#include "storage.h"
#include "maths.h"
#include "utils.h"
#include "decoders.h"
#include "TS_CommandButtonHandler.h"
#include "logger.h"
#include "errors.h"

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

    case 'a':
      cmdPending = true;

      if (Serial.available() >= 2)
      {
        Serial.read(); //Ignore the first value, it's always 0
        Serial.read(); //Ignore the second value, it's always 6
        sendValuesLegacy();
        cmdPending = false;
      }
      break;

    case 'A': // send x bytes of realtime values
      sendValues(0, LOG_ENTRY_SIZE, 0x31, 0);   //send values to serial0
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

    case 'd': // Send a CRC32 hash of a given page
      cmdPending = true;

      if (Serial.available() >= 2)
      {
        Serial.read(); //Ignore the first byte value, it's always 0
        uint32_t CRC32_val = calculateCRC32( Serial.read() );
        
        //Split the 4 bytes of the CRC32 value into individual bytes and send
        Serial.write( ((CRC32_val >> 24) & 255) );
        Serial.write( ((CRC32_val >> 16) & 255) );
        Serial.write( ((CRC32_val >> 8) & 255) );
        Serial.write( (CRC32_val & 255) );
        
        cmdPending = false;
      }
      break;

    case 'E': // receive command button commands
      cmdPending = true;

      if(Serial.available() >= 2)
      {
        byte cmdGroup = Serial.read();
        byte cmdValue = Serial.read();
        uint16_t cmdCombined = word(cmdGroup, cmdValue);

        if ( ((cmdCombined >= TS_CMD_INJ1_ON) && (cmdCombined <= TS_CMD_IGN8_50PC)) || (cmdCombined == TS_CMD_TEST_ENBL) || (cmdCombined == TS_CMD_TEST_DSBL) )
        {
          //Hardware test buttons
          if (currentStatus.RPM == 0) { TS_CommandButtonsHandler(cmdCombined); }
          cmdPending = false;
        }
        else if( (cmdCombined >= TS_CMD_VSS_60KMH) && (cmdCombined <= TS_CMD_VSS_RATIO6) )
        {
          //VSS Calibration commands
          TS_CommandButtonsHandler(cmdCombined);
          cmdPending = false;
        }
      }
      break;

    case 'F': // send serial protocol version
      Serial.print(F("001"));
      break;

    case 'H': //Start the tooth logger
      currentStatus.toothLogEnabled = true;
      currentStatus.compositeLogEnabled = false; //Safety first (Should never be required)
      BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
      toothHistoryIndex = 0;
      toothHistorySerialIndex = 0;

      //Disconnect the standard interrupt and add the logger version
      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), loggerPrimaryISR, CHANGE );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), loggerSecondaryISR, CHANGE );

      Serial.write(1); //TS needs an acknowledgement that this was received. I don't know if this is the correct response, but it seems to work
      break;

    case 'h': //Stop the tooth logger
      currentStatus.toothLogEnabled = false;

      //Disconnect the logger interrupts and attach the normal ones
      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), triggerHandler, primaryTriggerEdge );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), triggerSecondaryHandler, secondaryTriggerEdge );
      break;

    case 'J': //Start the composite logger
      currentStatus.compositeLogEnabled = true;
      currentStatus.toothLogEnabled = false; //Safety first (Should never be required)
      BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
      toothHistoryIndex = 0;
      toothHistorySerialIndex = 0;
      compositeLastToothTime = 0;

      //Disconnect the standard interrupt and add the logger version
      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), loggerPrimaryISR, CHANGE );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), loggerSecondaryISR, CHANGE );

      Serial.write(1); //TS needs an acknowledgement that this was received. I don't know if this is the correct response, but it seems to work
      break;

    case 'j': //Stop the composite logger
      currentStatus.compositeLogEnabled = false;

      //Disconnect the logger interrupts and attach the normal ones
      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), triggerHandler, primaryTriggerEdge );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), triggerSecondaryHandler, secondaryTriggerEdge );
      break;

    case 'L': // List the contents of current page in human readable form
      sendPageASCII();
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
      //This is a legacy function and is no longer used by TunerStudio. It is maintained for compatibility with other systems
      //A 2nd byte of data is required after the 'P' specifying the new page number.
      cmdPending = true;

      if (Serial.available() > 0)
      {
        currentPage = Serial.read();
        //This converts the ascii number char into binary. Note that this will break everyything if there are ever more than 48 pages (48 = asci code for '0')
        if ((currentPage >= '0') && (currentPage <= '9')) // 0 - 9
        {
          currentPage -= 48;
        }
        else if ((currentPage >= 'a') && (currentPage <= 'f')) // 10 - 15
        {
          currentPage -= 87;
        }
        else if ((currentPage >= 'A') && (currentPage <= 'F'))
        {
          currentPage -= 55;
        }
        
        // Detecting if the current page is a table/map
        if ( (currentPage == veMapPage) || (currentPage == ignMapPage) || (currentPage == afrMapPage) || (currentPage == fuelMap2Page) ) { isMap = true; }
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
      Serial.print(F("speeduino 202006-dev"));
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
      Serial.print(F("Speeduino 2020.06-dev"));
      currentStatus.secl = 0; //This is required in TS3 due to its stricter timings
      break;

    case 'T': //Send 256 tooth log entries to Tuner Studios tooth logger
      //6 bytes required:
      //2 - Page identifier
      //2 - offset
      //2 - Length
      cmdPending = true;
      if(Serial.available() >= 6)
      {
        Serial.read(); // First byte of the page identifier can be ignored. It's always 0
        Serial.read(); // First byte of the page identifier can be ignored. It's always 0
        Serial.read(); // First byte of the page identifier can be ignored. It's always 0
        Serial.read(); // First byte of the page identifier can be ignored. It's always 0
        Serial.read(); // First byte of the page identifier can be ignored. It's always 0
        Serial.read(); // First byte of the page identifier can be ignored. It's always 0

        if(currentStatus.toothLogEnabled == true) { sendToothLog(); } //Sends tooth log values as ints
        else if (currentStatus.compositeLogEnabled == true) { sendCompositeLog(); }

        cmdPending = false;
      }

      

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

    case 'U': //User wants to reset the Arduino (probably for FW update)
      if (resetControl != RESET_CONTROL_DISABLED)
      {
      #ifndef SMALL_FLASH_MODE
        if (!cmdPending) { Serial.println(F("Comms halted. Next byte will reset the Arduino.")); }
      #endif

        while (Serial.available() == 0) { }
        digitalWrite(pinResetControl, LOW);
      }
      else
      {
      #ifndef SMALL_FLASH_MODE
        if (!cmdPending) { Serial.println(F("Reset control is currently disabled.")); }
      #endif
      }
      break;

    case 'V': // send VE table and constants in binary
      sendPage();
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
        //2 - Length
        //1 - 1st New value
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
        //Serial.println(cltCalibrationTable[x]);
      }
      Serial.println(F("Inlet temp"));
      for (int x = 0; x < CALIBRATION_TABLE_SIZE; x++)
      {
        Serial.print(x);
        Serial.print(", ");
        //Serial.println(iatCalibrationTable[x]);
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
        Serial.print(configPage4.wueBins[x]);
        Serial.print(F(", "));
        Serial.println(configPage2.wueValues[x]);
      }
      Serial.flush();
    #endif
      break;

    case 'z': //Send 256 tooth log entries to a terminal emulator
      sendToothLog(); //Sends tooth log values as chars
      break;

    case '`': //Custom 16u2 firmware is making its presence known
      cmdPending = true;

      if (Serial.available() >= 1) {
        configPage4.bootloaderCaps = Serial.read();
        cmdPending = false;
      }
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
         "U - Prepare for firmware update. The next byte received will cause the Arduino to reset.\n"
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
  byte fullStatus[LOG_ENTRY_SIZE];

  if (portNum == 3)
  {
    //CAN serial
    #if defined(USE_SERIAL3)
      if (cmd == 30)
      {
        CANSerial.write("r");         //confirm cmd type
        CANSerial.write(cmd);
        
      }
      else if (cmd == 31)
      {
        CANSerial.write("A");         //confirm cmd type
      }
    #endif
  }
  else
  {
    if(requestCount == 0) { currentStatus.secl = 0; }
    requestCount++;
  }

  currentStatus.spark ^= (-currentStatus.hasSync ^ currentStatus.spark) & (1U << BIT_SPARK_SYNC); //Set the sync bit of the Spark variable to match the hasSync variable

  fullStatus[0] = currentStatus.secl; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
  fullStatus[1] = currentStatus.status1; //status1 Bitfield
  fullStatus[2] = currentStatus.engine; //Engine Status Bitfield
  fullStatus[3] = currentStatus.syncLossCounter;
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
  fullStatus[16] = (byte)(currentStatus.AEamount >> 1); //TPS acceleration enrichment (%) divided by 2 (Can exceed 255)
  fullStatus[17] = lowByte(currentStatus.corrections); //Total GammaE (%)
  fullStatus[18] = highByte(currentStatus.corrections); //Total GammaE (%)
  fullStatus[19] = currentStatus.VE1; //VE 1 (%)
  fullStatus[20] = currentStatus.VE2; //VE 2 (%)
  fullStatus[21] = currentStatus.afrTarget;
  fullStatus[22] = currentStatus.tpsDOT; //TPS DOT
  fullStatus[23] = currentStatus.advance;
  fullStatus[24] = currentStatus.TPS; // TPS (0% to 100%)
  //Need to split the int loopsPerSecond value into 2 bytes
  if(currentStatus.loopsPerSecond > 60000) { currentStatus.loopsPerSecond = 60000;}
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

  fullStatus[75] = lowByte(currentStatus.PW1); //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
  fullStatus[76] = highByte(currentStatus.PW1); //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
  fullStatus[77] = lowByte(currentStatus.PW2); //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
  fullStatus[78] = highByte(currentStatus.PW2); //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
  fullStatus[79] = lowByte(currentStatus.PW3); //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
  fullStatus[80] = highByte(currentStatus.PW3); //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
  fullStatus[81] = lowByte(currentStatus.PW4); //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.
  fullStatus[82] = highByte(currentStatus.PW4); //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.

  fullStatus[83] = currentStatus.status3;
  fullStatus[84] = currentStatus.engineProtectStatus;
  fullStatus[85] = lowByte(currentStatus.fuelLoad);
  fullStatus[86] = highByte(currentStatus.fuelLoad);
  fullStatus[87] = lowByte(currentStatus.ignLoad);
  fullStatus[88] = highByte(currentStatus.ignLoad);
  fullStatus[89] = lowByte(currentStatus.dwell);
  fullStatus[90] = highByte(currentStatus.dwell);
  fullStatus[91] = currentStatus.CLIdleTarget;
  fullStatus[92] = currentStatus.mapDOT;
  fullStatus[93] = (int8_t)currentStatus.vvtAngle;
  fullStatus[94] = currentStatus.vvtTargetAngle;
  fullStatus[95] = currentStatus.vvtDuty;
  fullStatus[96] = lowByte(currentStatus.flexBoostCorrection);
  fullStatus[97] = highByte(currentStatus.flexBoostCorrection);
  fullStatus[98] = currentStatus.baroCorrection;
  fullStatus[99] = currentStatus.VE; //Current VE (%). Can be equal to VE1 or VE2 or a calculated value from both of them
  fullStatus[100] = currentStatus.ASEValue; //Current ASE (%)
  fullStatus[101] = lowByte(currentStatus.vss);
  fullStatus[102] = highByte(currentStatus.vss);
  fullStatus[103] = currentStatus.gear;
  fullStatus[104] = currentStatus.fuelPressure;
  fullStatus[105] = currentStatus.oilPressure;

  for(byte x=0; x<packetLength; x++)
  {
    if (portNum == 0) { Serial.write(fullStatus[offset+x]); }
    #if defined(CANSerial_AVAILABLE)
      else if (portNum == 3){ CANSerial.write(fullStatus[offset+x]); }
    #endif
  }

  // Reset any flags that are being used to trigger page refreshes
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_VSS_REFRESH);

}

void sendValuesLegacy()
{
  uint16_t temp;
  int bytestosend = 112;

  bytestosend -= Serial.write(currentStatus.secl>>8);
  bytestosend -= Serial.write(currentStatus.secl);
  bytestosend -= Serial.write(currentStatus.PW1>>8);
  bytestosend -= Serial.write(currentStatus.PW1);
  bytestosend -= Serial.write(currentStatus.PW2>>8);
  bytestosend -= Serial.write(currentStatus.PW2);
  bytestosend -= Serial.write(currentStatus.RPM>>8);
  bytestosend -= Serial.write(currentStatus.RPM);

  temp = currentStatus.advance * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  bytestosend -= Serial.write(currentStatus.nSquirts);
  bytestosend -= Serial.write(currentStatus.engine);
  bytestosend -= Serial.write(currentStatus.afrTarget);
  bytestosend -= Serial.write(currentStatus.afrTarget); // send twice so afrtgt1 == afrtgt2
  bytestosend -= Serial.write(99); // send dummy data as we don't have wbo2_en1
  bytestosend -= Serial.write(99); // send dummy data as we don't have wbo2_en2

  temp = currentStatus.baro * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.MAP * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.IAT * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.coolant * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.TPS * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  bytestosend -= Serial.write(currentStatus.battery10>>8);
  bytestosend -= Serial.write(currentStatus.battery10);
  bytestosend -= Serial.write(currentStatus.O2>>8);
  bytestosend -= Serial.write(currentStatus.O2);
  bytestosend -= Serial.write(currentStatus.O2_2>>8);
  bytestosend -= Serial.write(currentStatus.O2_2);

  bytestosend -= Serial.write(99); // knock
  bytestosend -= Serial.write(99); // knock

  temp = currentStatus.egoCorrection * 10;
  bytestosend -= Serial.write(temp>>8); // egocor1
  bytestosend -= Serial.write(temp); // egocor1
  bytestosend -= Serial.write(temp>>8); // egocor2
  bytestosend -= Serial.write(temp); // egocor2

  temp = currentStatus.iatCorrection * 10;
  bytestosend -= Serial.write(temp>>8); // aircor
  bytestosend -= Serial.write(temp); // aircor

  temp = currentStatus.wueCorrection * 10;
  bytestosend -= Serial.write(temp>>8); // warmcor
  bytestosend -= Serial.write(temp); // warmcor

  bytestosend -= Serial.write(99); // accelEnrich
  bytestosend -= Serial.write(99); // accelEnrich
  bytestosend -= Serial.write(99); // tpsFuelCut
  bytestosend -= Serial.write(99); // tpsFuelCut
  bytestosend -= Serial.write(99); // baroCorrection
  bytestosend -= Serial.write(99); // baroCorrection

  temp = currentStatus.corrections * 10;
  bytestosend -= Serial.write(temp>>8); // gammaEnrich
  bytestosend -= Serial.write(temp); // gammaEnrich

  temp = currentStatus.VE * 10;
  bytestosend -= Serial.write(temp>>8); // ve1
  bytestosend -= Serial.write(temp); // ve1
  temp = currentStatus.VE2 * 10;
  bytestosend -= Serial.write(temp>>8); // ve2
  bytestosend -= Serial.write(temp); // ve2

  bytestosend -= Serial.write(99); // iacstep
  bytestosend -= Serial.write(99); // iacstep
  bytestosend -= Serial.write(99); // cold_adv_deg
  bytestosend -= Serial.write(99); // cold_adv_deg

  temp = currentStatus.tpsDOT * 10;
  bytestosend -= Serial.write(temp>>8); // TPSdot
  bytestosend -= Serial.write(temp); // TPSdot

  temp = currentStatus.mapDOT * 10;
  bytestosend -= Serial.write(temp >> 8); // MAPdot
  bytestosend -= Serial.write(temp); // MAPdot

  temp = currentStatus.dwell * 10;
  bytestosend -= Serial.write(temp>>8); // dwell
  bytestosend -= Serial.write(temp); // dwell

  bytestosend -= Serial.write(99); // MAF
  bytestosend -= Serial.write(99); // MAF
  bytestosend -= Serial.write(currentStatus.fuelLoad*10); // fuelload
  bytestosend -= Serial.write(99); // fuelcor
  bytestosend -= Serial.write(99); // fuelcor
  bytestosend -= Serial.write(99); // portStatus

  for(int i = 0; i < bytestosend; i++)
  {
    // send dummy data to fill remote's buffer
    Serial.write(99);
  }
}

void receiveValue(uint16_t valueOffset, byte newValue)
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
      fuelTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case veSetPage:
      pnt_configPage = &configPage2; //Setup a pointer to the relevant config page
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
          ignitionTable.axisX[(valueOffset - 256)] = (int)(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiple it back by 100 to make it correct
        }
        else if(valueOffset < 288)
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order
          ignitionTable.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
      }
      ignitionTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case ignSetPage:
      pnt_configPage = &configPage4;
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
          afrTable.axisX[(valueOffset - 256)] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        }
        else
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order
          afrTable.axisY[tempOffset] = int(newValue) * TABLE_LOAD_MULTIPLIER;

        }
      }
      afrTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case afrSetPage:
      pnt_configPage = &configPage6;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[afrSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
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
      //End of boost table
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
      else if (valueOffset < 160) //New value is on the Y (Load) axis of the vvt table
      {
        tempOffset = valueOffset - 152;
        vvtTable.axisY[(7 - tempOffset)] = int(newValue); //TABLE_LOAD_MULTIPLIER is NOT used for vvt as it is TPS based (0-100)
      }
      //End of vvt table
      else if (valueOffset < 224) //New value is part of the staging map
      {
        tempOffset = valueOffset - 160;
        stagingTable.values[7 - (tempOffset / 8)][tempOffset % 8] = newValue;
      }
      else if (valueOffset < 232) //New value is on the X (RPM) axis of the staging table
      {
        tempOffset = valueOffset - 224;
        stagingTable.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      }
      else if (valueOffset < 240) //New value is on the Y (Load) axis of the staging table
      {
        tempOffset = valueOffset - 232;
        stagingTable.axisY[(7 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER;
      }
      boostTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      vvtTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      stagingTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
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
      else if (valueOffset < 132) { tempOffset = valueOffset - 96; trim3Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim3 map
      else if (valueOffset < 138) { tempOffset = valueOffset - 132; trim3Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 144) { tempOffset = valueOffset - 138; trim3Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 4
      else if (valueOffset < 180) { tempOffset = valueOffset - 144; trim4Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim4 map
      else if (valueOffset < 186) { tempOffset = valueOffset - 180; trim4Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 192) { tempOffset = valueOffset - 186; trim4Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table

      trim1Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim2Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim3Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim4Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case canbusPage:
      pnt_configPage = &configPage9;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[currentPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case warmupPage:
      pnt_configPage = &configPage10;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[currentPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case fuelMap2Page:
      if (valueOffset < 256) //New value is part of the fuel map
      {
        fuelTable2.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          fuelTable2.axisX[(valueOffset - 256)] = ((int)(newValue) * TABLE_RPM_MULTIPLIER); //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        }
        else if(valueOffset < 288)
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order (Due to us using (0,0) in the top left rather than bottom right
          fuelTable2.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
          //This should never happen. It means there's an invalid offset value coming through
        }
      }
      fuelTable2.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    default:
      break;
  }
  //if(Serial.available() > 16) { command(); }
}

/**
 * @brief Packs the data within the current page (As set with the 'P' command) into a buffer and sends it.
 * 
 * Note that some translation of the data is required to lay it out in the way Megasqurit / TunerStudio expect it
 * Data is sent in binary format, as defined by in each page in the ini
 */
void sendPage()
{
  void* pnt_configPage = &configPage2; //Default value is for safety only. Will be changed below if needed.
  struct table3D currentTable = fuelTable; //Default value is for safety only. Will be changed below if needed.
  bool sendComplete = false; //Used to track whether all send operations are complete

  switch (currentPage)
  {
    case veMapPage:
      currentTable = fuelTable;
      break;

    case veSetPage:
      pnt_configPage = &configPage2; //Create a pointer to Page 1 in memory
      break;

    case ignMapPage:
      currentTable = ignitionTable;
      break;

    case ignSetPage:
      pnt_configPage = &configPage4; //Create a pointer to Page 2 in memory
      break;

    case afrMapPage:
      currentTable = afrTable;
      break;

    case afrSetPage:
      pnt_configPage = &configPage6; //Create a pointer to Page 3 in memory
      break;

    case boostvvtPage:
    {
      //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
      byte response[80]; //Bit hacky, but send 1 map at a time (Each map is 8x8, so 64 + 8 + 8)

      //Boost table
      for (int x = 0; x < 64; x++) { response[x] = boostTable.values[7 - (x / 8)][x % 8]; }
      for (int x = 64; x < 72; x++) { response[x] = byte(boostTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 72; y < 80; y++) { response[y] = byte(boostTable.axisY[7 - (y - 72)]); }
      Serial.write((byte *)&response, 80);
      //VVT table
      for (int x = 0; x < 64; x++) { response[x] = vvtTable.values[7 - (x / 8)][x % 8]; }
      for (int x = 64; x < 72; x++) { response[x] = byte(vvtTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 72; y < 80; y++) { response[y] = byte(vvtTable.axisY[7 - (y - 72)]); }
      Serial.write((byte *)&response, 80);
      //Staging table
      for (int x = 0; x < 64; x++) { response[x] = stagingTable.values[7 - (x / 8)][x % 8]; }
      for (int x = 64; x < 72; x++) { response[x] = byte(stagingTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 72; y < 80; y++) { response[y] = byte(stagingTable.axisY[7 - (y - 72)] / TABLE_LOAD_MULTIPLIER); }
      Serial.write((byte *)&response, 80);
      sendComplete = true;
      break;
    }
    case seqFuelPage:
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
      break;
    }
    case canbusPage:
      pnt_configPage = &configPage9; //Create a pointer to Page 9 in memory
      break;

    case warmupPage:
      pnt_configPage = &configPage10; //Create a pointer to Page 10 in memory
      break;

    case fuelMap2Page:
      currentTable = fuelTable2;
      break;

    default:
    #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
    #endif
        //Just set default Values to avoid warnings
        pnt_configPage = &configPage10;
        currentTable = fuelTable;
        sendComplete = true;
        break;
  }
  if(!sendComplete)
  {
    if (isMap)
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
    } //is map
    else
    {
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

/**
 * @brief Similar to sendPage(), however data is sent in human readable format
 * 
 * This is used for testing only (Not used by TunerStudio) in order to see current map and config data without the need for TunerStudio. 
 */
void sendPageASCII()
{
  void* pnt_configPage = &configPage2; //Default value is for safety only. Will be changed below if needed.
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
      uint16_t* pnt16_configPage;
      // To Display Values from Config Page 1
      // When casting to the __FlashStringHelper type Serial.println uses the same subroutine as when using the F macro
      Serial.println((const __FlashStringHelper *)&pageTitles[27]);//27 is the index to the first char in the second sting in pageTitles
      // The following loop displays in human readable form of all byte values in config page 1 up to but not including the first array.
      // incrementing void pointers is cumbersome. Thus we have "pnt_configPage = (byte *)pnt_configPage + 1"
      for (pnt_configPage = (byte *)&configPage2; pnt_configPage < &configPage2.wueValues[0]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      for (byte x = 10; x; x--)// The x between the ';' has the same representation as the "x != 0" test or comparision
      {
        Serial.print(configPage2.wueValues[10 - x]);// This displays the values horizantially on the screen
        Serial.print(F(" "));
      }
      Serial.println();
      for (pnt_configPage = (byte *)&configPage2.wueValues[9] + 1; pnt_configPage < &configPage2.injAng; pnt_configPage = (byte *)pnt_configPage + 1) {
        Serial.println(*((byte *)pnt_configPage));// This displays all the byte values between the last array up to but not including the first unsigned int on config page 1
      }
      // The following loop displays four unsigned ints
      for (pnt16_configPage = (uint16_t *)&configPage2.injAng; pnt16_configPage < (uint16_t*)&configPage2.injAng + 9; pnt16_configPage = (uint16_t*)pnt16_configPage + 1)
      { Serial.println(*((uint16_t *)pnt16_configPage)); }
      // Following loop displays byte values between the unsigned ints
      for (pnt_configPage = (uint16_t *)&configPage2.injAng + 9; pnt_configPage < &configPage2.mapMax; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      Serial.println(configPage2.mapMax);
      // Following loop displays remaining byte values of the page
      for (pnt_configPage = (uint16_t *)&configPage2.mapMax + 1; pnt_configPage < (byte *)&configPage2 + npage_size[veSetPage]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      sendComplete = true;
      break;

    case ignMapPage:
      currentTitleIndex = 42;// the index to the first char of the third string in pageTitles
      currentTable = ignitionTable;
      break;

    case ignSetPage:
      //To Display Values from Config Page 2
      Serial.println((const __FlashStringHelper *)&pageTitles[56]);
      Serial.println(configPage4.triggerAngle);// configPsge2.triggerAngle is an int so just display it without complication
      // Following loop displays byte values after that first int up to but not including the first array in config page 2
      for (pnt_configPage = (byte *)&configPage4 + 1; pnt_configPage < &configPage4.taeBins[0]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      for (byte y = 2; y; y--)// Displaying two equal sized arrays
      {
        byte * currentVar;// A placeholder for each array
        if (y == 2) {
          currentVar = configPage4.taeBins;
        }
        else {
          currentVar = configPage4.taeValues;
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
        Serial.print(configPage4.wueBins[10 - x]);//Displaying array horizontally across screen
        Serial.print(' ');
      }
      Serial.println();
      Serial.println(configPage4.dwellLimit);// Little lonely byte stuck between two arrays. No complications just display it.
      for (byte x = 6; x; x--)
      {
        Serial.print(configPage4.dwellCorrectionValues[6 - x]);
        Serial.print(' ');
      }
      Serial.println();
      for (pnt_configPage = (byte *)&configPage4.dwellCorrectionValues[5] + 1; pnt_configPage < (byte *)&configPage4 + npage_size[ignSetPage]; pnt_configPage = (byte *)pnt_configPage + 1)
      {
        Serial.println(*((byte *)pnt_configPage));// Displaying remaining byte values of the page
      }
      sendComplete = true;
      break;

    case afrMapPage:
      currentTitleIndex = 71;//Array index to next string
      currentTable = afrTable;
      break;

    case afrSetPage:
      //currentTitleIndex = 91;
      //To Display Values from Config Page 3
      Serial.println((const __FlashStringHelper *)&pageTitles[91]);//special typecasting to enable suroutine that the F macro uses
      for (pnt_configPage = (byte *)&configPage6; pnt_configPage < &configPage6.voltageCorrectionBins[0]; pnt_configPage = (byte *)pnt_configPage + 1)
      {
        Serial.println(*((byte *)pnt_configPage));// Displaying byte values of config page 3 up to but not including the first array
      }
      for (byte y = 2; y; y--)// Displaying two equally sized arrays that are next to each other
      {
        byte * currentVar;
        if (y == 2) { currentVar = configPage6.voltageCorrectionBins; }
        else { currentVar = configPage6.injVoltageCorrectionValues; }

        for (byte i = 6; i; i--)
        {
          Serial.print(currentVar[6 - i]);
          Serial.print(' ');
        }
        Serial.println();
      }
      for (byte y = 2; y; y--)// and again
      {
        byte* currentVar;
        if (y == 2) { currentVar = configPage6.airDenBins; }
        else { currentVar = configPage6.airDenRates; }

        for (byte i = 9; i; i--)
        {
          Serial.print(currentVar[9 - i]);
          Serial.print(' ');
        }
        Serial.println();
      }
      // Following loop displays the remaining byte values of the page
      for (pnt_configPage = (byte *)&configPage6.airDenRates[8] + 1; pnt_configPage < (byte *)&configPage6 + npage_size[afrSetPage]; pnt_configPage = (byte *)pnt_configPage + 1)
      {
        Serial.println(*((byte *)pnt_configPage));
      }
      sendComplete = true;

      //Old configPage4 STARTED HERE!
      //currentTitleIndex = 106;
      Serial.println((const __FlashStringHelper *)&pageTitles[106]);// F macro hack
      for (byte y = 4; y; y--)// Display four equally sized arrays
      {
        byte * currentVar;
        switch (y)
        {
          case 1: currentVar = configPage6.iacBins; break;
          case 2: currentVar = configPage6.iacOLPWMVal; break;
          case 3: currentVar = configPage6.iacOLStepVal; break;
          case 4: currentVar = configPage6.iacCLValues; break;
          default: break;
        }
        for (byte i = 10; i; i--)
        {
          Serial.print(currentVar[10 - i]);
          Serial.print(' ');
        }
        Serial.println();
      }
      for (byte y = 3; y; y--)// Three equally sized arrays
      {
        byte * currentVar;
        switch (y)
        {
          case 1: currentVar = configPage6.iacCrankBins; break;
          case 2: currentVar = configPage6.iacCrankDuty; break;
          case 3: currentVar = configPage6.iacCrankSteps; break;
          default: break;
        }
        for (byte i = 4; i; i--)
        {
          Serial.print(currentVar[4 - i]);
          Serial.print(' ');
        }
        Serial.println();
      }
      // Following loop is for remaining byte value of page
      for (pnt_configPage = (byte *)&configPage6.iacCrankBins[3] + 1; pnt_configPage < (byte *)&configPage6 + npage_size[afrSetPage]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      sendComplete = true;
      break;

    case boostvvtPage:
      currentTable = boostTable;
      currentTitleIndex = 121;
      break;

    case seqFuelPage:
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
        for (int i = 0; i < currentTable.xSize; i++)
        {
          byte value = currentTable.values[y][i];
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
      break;

    case canbusPage:
      //currentTitleIndex = 141;
      //To Display Values from Config Page 10
      Serial.println((const __FlashStringHelper *)&pageTitles[103]);//special typecasting to enable suroutine that the F macro uses
      for (pnt_configPage = &configPage9; pnt_configPage < ( (byte *)&configPage9 + npage_size[canbusPage]); pnt_configPage = (byte *)pnt_configPage + 1)
      {
        Serial.println(*((byte *)pnt_configPage));// Displaying byte values of config page 9 up to but not including the first array
      }
      sendComplete = true;
      break;

    case warmupPage:
      //NOT WRITTEN YET
      #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
      #endif
      sendComplete = true;
      break;

    case fuelMap2Page:
      currentTitleIndex = 117;// the index to the first char of the third string in pageTitles
      currentTable = fuelTable2;
      break;

    default:
    #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
    #endif
        //Just set default Values to avoid warnings
        pnt_configPage = &configPage10;
        currentTable = fuelTable;
        sendComplete = true;
        break;
  }
  if(!sendComplete)
  {
    if (isMap)
    {
      //This is a do while loop that kicks in for the boostvvtPage
      do {
        const char spaceChar = ' ';

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
        else { currentTitleIndex = 0; }
      } while(currentTitleIndex == 132); //Should never loop unless going to display vvtTable
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
    } //isMap
  } //sendComplete
}

/**
 * @brief Retrieves a single value from a memory page, with data aligned as per the ini file
 * 
 * @param page The page number to retrieve data from
 * @param valueAddress The address in the page that should be returned. This is as per the page definition in the ini
 * @return byte The requested value
 */
byte getPageValue(byte page, uint16_t valueAddress)
{
  void* pnt_configPage = &configPage2; //Default value is for safety only. Will be changed below if needed.
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
        pnt_configPage = &configPage2; //Create a pointer to Page 1 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case ignMapPage:
        if( valueAddress < 256) { returnValue = ignitionTable.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(ignitionTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(ignitionTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;

    case ignSetPage:
        pnt_configPage = &configPage4; //Create a pointer to Page 2 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case afrMapPage:
        if( valueAddress < 256) { returnValue = afrTable.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(afrTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(afrTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;

    case afrSetPage:
        pnt_configPage = &configPage6; //Create a pointer to Page 3 in memory
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
          else if(valueAddress < 160)
          {
            tempAddress = valueAddress - 80;
            //VVT table
            if(tempAddress < 64) { returnValue = vvtTable.values[7 - (tempAddress / 8)][tempAddress % 8]; }
            else if(tempAddress < 72) { returnValue = byte(vvtTable.axisX[(tempAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 80) { returnValue = byte(vvtTable.axisY[7 - (tempAddress - 72)]); }
          }
          else
          {
            tempAddress = valueAddress - 160;
            //Staging table
            if(tempAddress < 64) { returnValue = stagingTable.values[7 - (tempAddress / 8)][tempAddress % 8]; }
            else if(tempAddress < 72) { returnValue = byte(stagingTable.axisX[(tempAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 80) { returnValue = byte(stagingTable.axisY[7 - (tempAddress - 72)] / TABLE_LOAD_MULTIPLIER); }
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
        pnt_configPage = &configPage9; //Create a pointer to Page 10 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case warmupPage:
        pnt_configPage = &configPage10; //Create a pointer to Page 11 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case fuelMap2Page:
        if( valueAddress < 256) { returnValue = fuelTable2.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(fuelTable2.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(fuelTable2.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;

    default:
    #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
    #endif
        //Just set default Values to avoid warnings
        pnt_configPage = &configPage10;
        break;
  }
  return returnValue;
}

/**
 * @brief Processes an incoming stream of calibration data from TunerStudio. Result is store in EEPROM and memory
 * 
 * @param tableID Which calibration table to process. 0 = Coolant Sensor. 1 = IAT Sensor. 2 = O2 Sensor.
 */
void receiveCalibration(byte tableID)
{
  byte* pnt_TargetTable; //Pointer that will be used to point to the required target table
  int OFFSET, DIVISION_FACTOR, BYTES_PER_VALUE, EEPROM_START;

  switch (tableID)
  {
    case 0:
      //coolant table
      //pnt_TargetTable = (byte *)&cltCalibrationTable;
      OFFSET = CALIBRATION_TEMPERATURE_OFFSET; //
      DIVISION_FACTOR = 10;
      BYTES_PER_VALUE = 2;
      EEPROM_START = EEPROM_CALIBRATION_CLT;
      break;
    case 1:
      //Inlet air temp table
      //pnt_TargetTable = (byte *)&iatCalibrationTable;
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
  bool useLEDIndicator = false;
  if (pinIsOutput(LED_BUILTIN) == false)
  {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    useLEDIndicator = true;
  }

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
      //EEPROM.update(y, (byte)tempValue);
      storeCalibrationValue(y, (byte)tempValue);

      every2nd = false;
      if(useLEDIndicator == true)
      {
        #if defined(CORE_STM32)
          digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        #else
          analogWrite(LED_BUILTIN, (counter % 50) );
        #endif
      }
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
void sendToothLog()
{
  //We need TOOTH_LOG_SIZE number of records to send to TunerStudio. If there aren't that many in the buffer then we just return and wait for the next call
  if (BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY)) //Sanity check. Flagging system means this should always be true
  {
      for (int x = 0; x < TOOTH_LOG_SIZE; x++)
      {
        //Serial.write(highByte(toothHistory[toothHistorySerialIndex]));
        //Serial.write(lowByte(toothHistory[toothHistorySerialIndex]));
        Serial.write(toothHistory[toothHistorySerialIndex] >> 24);
        Serial.write(toothHistory[toothHistorySerialIndex] >> 16);
        Serial.write(toothHistory[toothHistorySerialIndex] >> 8);
        Serial.write(toothHistory[toothHistorySerialIndex]);

        if(toothHistorySerialIndex == (TOOTH_LOG_BUFFER-1)) { toothHistorySerialIndex = 0; }
        else { toothHistorySerialIndex++; }
      }
      BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
      cmdPending = false;
  }
  else 
  { 
    //TunerStudio has timed out, send a LOG of all 0s
    for(int x = 0; x < (4*TOOTH_LOG_SIZE); x++)
    {
      Serial.write(static_cast<byte>(0x00)); //GCC9 fix
    }
    cmdPending = false; 
  } 
}

void sendCompositeLog()
{
  if (BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY)) //Sanity check. Flagging system means this should always be true
  {
      uint32_t runTime = 0;
      for (int x = 0; x < TOOTH_LOG_SIZE; x++)
      {
        runTime += toothHistory[toothHistorySerialIndex]; //This combined runtime (in us) that the log was going for by this record)
        
        //Serial.write(highByte(runTime));
        //Serial.write(lowByte(runTime));
        Serial.write(runTime >> 24);
        Serial.write(runTime >> 16);
        Serial.write(runTime >> 8);
        Serial.write(runTime);

        //Serial.write(highByte(toothHistory[toothHistorySerialIndex]));
        //Serial.write(lowByte(toothHistory[toothHistorySerialIndex]));

        Serial.write(compositeLogHistory[toothHistorySerialIndex]); //The status byte (Indicates the trigger edge, whether it was a pri/sec pulse, the sync status)

        if(toothHistorySerialIndex == (TOOTH_LOG_BUFFER-1)) { toothHistorySerialIndex = 0; }
        else { toothHistorySerialIndex++; }
      }
      BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
      toothHistoryIndex = 0;
      toothHistorySerialIndex = 0;
      compositeLastToothTime = 0;
      cmdPending = false;
  }
  else 
  { 
    //TunerStudio has timed out, send a LOG of all 0s
    for(int x = 0; x < (5*TOOTH_LOG_SIZE); x++)
    {
      Serial.write(static_cast<byte>(0x00)); //GCC9 fix
    }
    cmdPending = false; 
  } 
}

void testComm()
{
  Serial.write(1);
  return;
}

