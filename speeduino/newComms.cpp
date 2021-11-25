/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * Process Incoming and outgoing serial communications.
 */
#include "globals.h"
#include "newComms.h"
#include "cancomms.h"
#include "storage.h"
#include "maths.h"
#include "utilities.h"
#include "decoders.h"
#include "TS_CommandButtonHandler.h"
#include "errors.h"
#include "pages.h"
#include "page_crc.h"
#include "logger.h"
#include "comms.h"
#include "src/FastCRC/FastCRC.h"
#ifdef RTC_ENABLED
  #include "rtc_common.h"
#endif
#ifdef SD_LOGGING
  #include "SD_logger.h"
#endif

uint16_t serialPayloadLength = 0;
bool serialReceivePending = false; /**< Whether or not a serial request has only been partially received. This occurs when a the length has been received in the serial buffer, but not all of the payload or CRC has yet been received. */
uint16_t serialBytesReceived = 0;
uint32_t serialCRC = 0; 
uint8_t serialPayload[SERIAL_BUFFER_SIZE]; /**< Pointer to the serial payload buffer. */
#ifdef RTC_ENABLED
  uint8_t serialSDTransmitPayload[SD_FILE_TRANSMIT_BUFFER_SIZE];
  uint16_t SDcurrentDirChunk;
  uint32_t SDreadStartSector;
  uint32_t SDreadNumSectors;
  uint32_t SDreadCompletedSectors = 0;
#endif

/** Processes the incoming data on the serial buffer based on the command sent.
Can be either data for a new command or a continuation of data for command that is already in progress:
- cmdPending = If a command has started but is wairing on further data to complete
- chunkPending = Specifically for the new receive value method where TS will send a known number of contiguous bytes to be written to a table

Comands are single byte (letter symbol) commands.
*/
void parseSerial()
{

  //Check for an existing legacy command in progress
  if(cmdPending == true)
  {
    command();
    return;
  }

  if (serialReceivePending == false)
  { 
    serialBytesReceived = 0; //Reset the number of bytes received as we're starting a new command

    //New command received
    //Need at least 2 bytes to read the length of the command
    serialReceivePending = true; //Flag the serial receive as being in progress
    byte lowByte = Serial.read();

    //Check if the command is legacy using the call/response mechanism
    if((lowByte >= 'A') && (lowByte <= 'z') )
    {
      //Handle legacy cases here
      serialReceivePending = false; //Make sure new serial handling does not interfere with legacy handling
      legacySerial = true;
      currentCommand = lowByte;
      command();
    }
    else
    {
      while(Serial.available() == 0) { } //Wait for the 2nd byte to be received (This will almost never happen)

      byte highByte = Serial.read();
      serialPayloadLength = word(lowByte, highByte);
      serialBytesReceived = 2;
      cmdPending = false; // Make sure legacy handling does not interfere with new serial handling

      //serialReceivePayload = (uint8_t *)malloc(serialPayloadLength);
    }
  }

  //If there is a serial receive in progress, read as much from the buffer as possible or until we receive all bytes
  while( (Serial.available() > 0) && (serialReceivePending == true) )
  {
    if (serialBytesReceived < (serialPayloadLength + SERIAL_LEN_SIZE) )
    {
      serialPayload[(serialBytesReceived - SERIAL_LEN_SIZE)] = Serial.read();
      serialBytesReceived++;
    }
    else if (Serial.available() >= SERIAL_CRC_LENGTH)
    {
      uint32_t crc1 = Serial.read();
      uint32_t crc2 = Serial.read();
      uint32_t crc3 = Serial.read();
      uint32_t crc4 = Serial.read();
      serialCRC = (crc1<<24) | (crc2<<16) | (crc3<<8) | crc4;
      
      serialReceivePending = false; //The serial receive is now complete

      //Test the CRC
      uint32_t receivedCRC = CRC32.crc32(serialPayload, serialPayloadLength);
      //receivedCRC++;
      if(serialCRC != receivedCRC)
      {
        //CRC Error. Need to send an error message
        sendSerialReturnCode(SERIAL_RC_CRC_ERROR);
      }
      else
      {
        //CRC is correct. Process the command
        processSerialCommand();
      }
      //free(serialReceivePayload); //Finally free the memory from the payload buffer
    }
  }
}

void sendSerialReturnCode(byte returnCode)
{
  Serial.write((uint8_t)0);
  Serial.write((uint8_t)1); //Size is always 1

  Serial.write(returnCode);

  //Calculate and send CRC
  uint32_t CRC32_val = CRC32.crc32(&returnCode, 1);
  Serial.write( ((CRC32_val >> 24) & 255) );
  Serial.write( ((CRC32_val >> 16) & 255) );
  Serial.write( ((CRC32_val >> 8) & 255) );
  Serial.write( (CRC32_val & 255) );
}

void sendSerialPayload(void *payload, uint16_t payloadLength)
{
  //uint16_t totalPayloadLength = payloadLength + SERIAL_CRC_LENGTH;
  uint16_t totalPayloadLength = payloadLength;
  Serial.write(totalPayloadLength >> 8);
  Serial.write(totalPayloadLength);

  //Need to handle serial buffer being full. This is just for testing
  for(int i = 0; i < payloadLength; i++)
  {
    Serial.write(((uint8_t*)payload)[i]);
  }

  //Calculate and send CRC
  uint32_t CRC32_val = CRC32.crc32((uint8_t*)payload, payloadLength);
  Serial.write( ((CRC32_val >> 24) & 255) );
  Serial.write( ((CRC32_val >> 16) & 255) );
  Serial.write( ((CRC32_val >> 8) & 255) );
  Serial.write( (CRC32_val & 255) );
}

void processSerialCommand()
{
  currentCommand = serialPayload[0];

  switch (currentCommand)
  {
    /*
    Should not happen with the new mode
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
    */

    case 'A': // send x bytes of realtime values
      //sendValues(0, LOG_ENTRY_SIZE, 0x31, 0);   //send values to serial0
      generateLiveValues(0, LOG_ENTRY_SIZE); 
      break;

    /*
    Should not happen with the new mode
    case 'B': // Burn current values to eeprom
      writeAllConfig();
      break;
    */

    case 'b': // New EEPROM burn command to only burn a single page at a time
      writeConfig(serialPayload[2]); //Read the table number and perform burn. Note that byte 1 in the array is unused
      sendSerialReturnCode(SERIAL_RC_BURN_OK);
      break;

    case 'C': // test communications. This is used by Tunerstudio to see whether there is an ECU on a given serial port
    {
      uint8_t tempPayload[] = {SERIAL_RC_OK, currentStatus.secl};
      sendSerialPayload(&tempPayload, 2);
      break;
    }

    /*
    Should not happen with the new mode
    case 'c': //Send the current loops/sec value
      Serial.write(lowByte(currentStatus.loopsPerSecond));
      Serial.write(highByte(currentStatus.loopsPerSecond));
      break;
    */

    case 'E': // receive command button commands
    {
      byte cmdGroup = serialPayload[1];
      byte cmdValue = serialPayload[2];
      uint16_t cmdCombined = word(cmdGroup, cmdValue);

      if ( ((cmdCombined >= TS_CMD_INJ1_ON) && (cmdCombined <= TS_CMD_IGN8_50PC)) || (cmdCombined == TS_CMD_TEST_ENBL) || (cmdCombined == TS_CMD_TEST_DSBL) )
      {
        //Hardware test buttons
        if (currentStatus.RPM == 0) { TS_CommandButtonsHandler(cmdCombined); }
      }
      else if( (cmdCombined >= TS_CMD_VSS_60KMH) && (cmdCombined <= TS_CMD_VSS_RATIO6) )
      {
        //VSS Calibration commands
        TS_CommandButtonsHandler(cmdCombined);
      }
      else if( (cmdCombined >= TS_CMD_STM32_REBOOT) && (cmdCombined <= TS_CMD_STM32_BOOTLOADER) )
      {
        //STM32 DFU mode button
        TS_CommandButtonsHandler(cmdCombined);
      }
      sendSerialReturnCode(SERIAL_RC_OK);
      break;
    }

    case 'F': // send serial protocol version
    {
      byte serialVersion[] = {SERIAL_RC_OK, '0', '0', '2'};
      sendSerialPayload(&serialVersion, 4);
      break;
    }

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

      sendSerialReturnCode(SERIAL_RC_OK);
      break;

    case 'h': //Stop the tooth logger
      currentStatus.toothLogEnabled = false;

      //Disconnect the logger interrupts and attach the normal ones
      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), triggerHandler, primaryTriggerEdge );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), triggerSecondaryHandler, secondaryTriggerEdge );
      sendSerialReturnCode(SERIAL_RC_OK);
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

      sendSerialReturnCode(SERIAL_RC_OK);
      break;

    case 'j': //Stop the composite logger
      currentStatus.compositeLogEnabled = false;

      //Disconnect the logger interrupts and attach the normal ones
      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), triggerHandler, primaryTriggerEdge );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), triggerSecondaryHandler, secondaryTriggerEdge );
      sendSerialReturnCode(SERIAL_RC_OK);
      break;

    case 'd': // Send a CRC32 hash of a given page
    {
      uint32_t CRC32_val = calculatePageCRC32( serialPayload[2] );
      uint8_t payloadCRC32[5];
      
      //First byte is the flag
      payloadCRC32[0] = SERIAL_RC_OK;

      //Split the 4 bytes of the CRC32 value into individual bytes and send
      payloadCRC32[1] =  ((CRC32_val >> 24) & 255);
      payloadCRC32[2] = ((CRC32_val >> 16) & 255);
      payloadCRC32[3] = ((CRC32_val >> 8) & 255);
      payloadCRC32[4] = (CRC32_val & 255);
      
      sendSerialPayload( &payloadCRC32, 5);

      break;
    }
      

    /*
    * New method for sending page values (MS command equivalent is 'r')
    */
    case 'p':
    {
      //6 bytes required:
      //2 - Page identifier
      //2 - offset
      //2 - Length
      byte offset1, offset2, length1, length2;
      int length;
      byte tempPage;

      tempPage = serialPayload[2];
      offset1 = serialPayload[3];
      offset2 = serialPayload[4];
      valueOffset = word(offset2, offset1);
      length1 = serialPayload[5];
      length2 = serialPayload[6];
      length = word(length2, length1);

      //Setup the transmit buffer
      //serialTransmitPayload = (byte*) malloc(length + 1);
      serialPayload[0] = SERIAL_RC_OK;
      for(int i = 0; i < length; i++)
      {
        serialPayload[i+1] = getPageValue(tempPage, valueOffset + i);
      }
      sendSerialPayload(&serialPayload, (length + 1));
      //free(serialTransmitPayload);
      break;
    }

    case 'Q': // send code version
    {
      char productString[21] = { SERIAL_RC_OK, 's','p','e','e','d','u','i','n','o',' ','2','0','2','1','0','9','-','d','e','v'} ; //Note no null terminator in array and statu variable at the start
      sendSerialPayload(&productString, 21);
      break;
    }

    case 'r': //New format for the optimised OutputChannels
    {
      uint8_t cmd = serialPayload[2];
      uint16_t offset = word(serialPayload[4], serialPayload[3]);
      uint16_t length = word(serialPayload[6], serialPayload[5]);
      uint16_t SD_arg1 = word(serialPayload[3], serialPayload[4]);
      uint16_t SD_arg2 = word(serialPayload[5], serialPayload[6]);

      if(cmd == 0x30) //Send output channels command 0x30 is 48dec
      {
        generateLiveValues(offset, length);
        sendSerialPayload(&serialPayload, (length + 1));
      }
#ifdef RTC_ENABLED
      else if(cmd == SD_RTC_PAGE) //Request to read SD card RTC
      {
        serialPayload[0] = SERIAL_RC_OK;
        serialPayload[1] = rtc_getSecond(); //Seconds
        serialPayload[2] = rtc_getMinute(); //Minutes
        serialPayload[3] = rtc_getHour(); //Hours
        serialPayload[4] = rtc_getDOW(); //Day of week
        serialPayload[5] = rtc_getDay(); //Day of month
        serialPayload[6] = rtc_getMonth(); //Month
        serialPayload[7] = highByte(rtc_getYear()); //Year
        serialPayload[8] = lowByte(rtc_getYear()); //Year
        sendSerialPayload(&serialPayload, 9);

      }
      else if(cmd == SD_READWRITE_PAGE) //Request SD card extended parameters
      {
        //SD read commands use the offset and length fields to indicate the request type
        if((SD_arg1 == SD_READ_STAT_ARG1) && (SD_arg2 == SD_READ_STAT_ARG2))
        {
          //Read the status of the SD card
          
          serialPayload[0] = SERIAL_RC_OK;

          serialPayload[1] = currentStatus.TS_SD_Status;
          //serialPayload[1] = 5;
          serialPayload[2] = 0;

          //All other values are 2 bytes   
          //Sector size     
          serialPayload[3] = 2;
          serialPayload[4] = 0;

          //Max blocks (4 bytes)
          serialPayload[5] = 0;
          serialPayload[6] = 0x20; //1gb dummy card
          serialPayload[7] = 0;
          serialPayload[8] = 0;

          //Max roots (Number of files)
          serialPayload[9] = 0;
          serialPayload[10] = 1;

          //Dir Start (4 bytes)
          serialPayload[11] = 0;
          serialPayload[12] = 0;
          serialPayload[13] = 0;
          serialPayload[14] = 0;

          //Unkown purpose for last 2 bytes
          serialPayload[15] = 0;
          serialPayload[16] = 0;

          sendSerialPayload(&serialPayload, 17);

        }
        else if((SD_arg1 == SD_READ_DIR_ARG1) && (SD_arg2 == SD_READ_DIR_ARG2))
        {
          //Send file details
          serialPayload[0] = SERIAL_RC_OK;

          uint16_t logFileNumber = (SDcurrentDirChunk * 16) + 1;
          uint8_t filesInCurrentChunk = 0;
          uint16_t payloadIndex = 1;
          while((filesInCurrentChunk < 16) && (getSDLogFileDetails(&serialPayload[payloadIndex], logFileNumber) == true))
          {
            logFileNumber++;
            filesInCurrentChunk++;
            payloadIndex += 32;
          }
          serialPayload[payloadIndex] = lowByte(SDcurrentDirChunk);
          serialPayload[payloadIndex + 1] = highByte(SDcurrentDirChunk);
          //Serial.print("Index:");
          //Serial.print(payloadIndex);

          sendSerialPayload(&serialPayload, (payloadIndex + 2));

        }

      }
      else if(cmd == SD_READFILE_PAGE)
      {
        //Fetch data from file
        if(SD_arg2 == SD_READ_COMP_ARG2)
        {
          //arg1 is the block number to return
          serialSDTransmitPayload[0] = SERIAL_RC_OK;
          serialSDTransmitPayload[1] = highByte(SD_arg1);
          serialSDTransmitPayload[2] = lowByte(SD_arg1);

          uint32_t currentSector = SDreadStartSector + (SD_arg1 * 4);
          
          int32_t numSectorsToSend = 0;
          if(SDreadNumSectors > SDreadCompletedSectors)
          {
            numSectorsToSend = SDreadNumSectors - SDreadCompletedSectors;
            if(numSectorsToSend > 4) //Maximum of 4 sectors at a time
            {
              numSectorsToSend = 4;
            }
          }
          SDreadCompletedSectors += numSectorsToSend;
          
          if(numSectorsToSend <= 0) { sendSerialReturnCode(SERIAL_RC_OK); }
          else
          {
            readSDSectors(&serialSDTransmitPayload[3], currentSector, numSectorsToSend); 
            sendSerialPayload(&serialSDTransmitPayload, (numSectorsToSend * SD_SECTOR_SIZE + 3));
          }
        }
      }
#endif
      else
      {
        //No other r/ commands should be called
      }
      cmdPending = false;
    
      break;
    }

    case 'S': // send code version
    {
      byte productString[] = { SERIAL_RC_OK, 'S', 'p', 'e', 'e', 'd', 'u', 'i', 'n', 'o', ' ', '2', '0', '2', '1', '.', '0', '9', '-', 'd', 'e', 'v'};
      //productString = F("Speeduino 2021.09-dev");
      sendSerialPayload(&productString, sizeof(productString));
      currentStatus.secl = 0; //This is required in TS3 due to its stricter timings
      break;
    }
      

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

        if(currentStatus.toothLogEnabled == true) { generateToothLog(0); } //Sends tooth log values as ints
        else if (currentStatus.compositeLogEnabled == true) { generateCompositeLog(0); }

        cmdPending = false;
      }

      

      break;

    case 't': // receive new Calibration info. Command structure: "t", <tble_idx> <data array>.
      byte tableID;
      //byte canID;

      //The first 2 bytes sent represent the canID and tableID
      while (Serial.available() == 0) { }
      tableID = Serial.read(); //Not currently used for anything

      receiveCalibrationNew(tableID); //Receive new values and store in memory
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


    case 'M':
    {
      //New write command
      //7 bytes required:
      //2 - Page identifier
      //2 - offset
      //2 - Length
      //1 - 1st New value
      byte offset1, offset2, length1, length2;

      uint8_t currentPage = serialPayload[2];
      offset1 = serialPayload[3];
      offset2 = serialPayload[4];
      uint16_t valueOffset = word(offset2, offset1);
      length1 = serialPayload[5];
      length2 = serialPayload[6];
      uint16_t chunkSize = word(length2, length1);

      for(uint16_t i = 0; i < chunkSize; i++)
      {
        setPageValue(currentPage, (valueOffset + i), serialPayload[7 + i]);
      }
      sendSerialReturnCode(SERIAL_RC_OK);
      break;
    }

    case 'w':
    {
      uint8_t cmd = serialPayload[2];
      uint16_t SD_arg1 = word(serialPayload[3], serialPayload[4]);
      uint16_t SD_arg2 = word(serialPayload[5], serialPayload[6]);

#ifdef RTC_ENABLED
      if(cmd == SD_READWRITE_PAGE)
        { 
          if((SD_arg1 == SD_WRITE_DO_ARG1) && (SD_arg2 == SD_WRITE_DO_ARG2))
          {
            /*
            SD DO command. Single byte of data where the commands are:
            0 Reset
            1 Reset
            2 Stop logging
            3 Start logging
            4 Load status variable
            5 Init SD card
            */
            uint8_t command = serialPayload[7];
            if(command == 2) { endSDLogging(); manualLogActive = false; }
            else if(command == 3) { beginSDLogging(); manualLogActive = true; }
            else if(command == 4) { setTS_SD_status(); }
            //else if(command == 5) { initSD(); }
            
            sendSerialReturnCode(SERIAL_RC_OK);
          }
          else if((SD_arg1 == SD_WRITE_DIR_ARG1) && (SD_arg2 == SD_WRITE_DIR_ARG2))
          {
            //Begin SD directory read. Value in payload represents the directory chunk to read
            //Directory chunks are each 16 files long
            SDcurrentDirChunk = word(serialPayload[7], serialPayload[8]);
            sendSerialReturnCode(SERIAL_RC_OK);
          }
          else if((SD_arg1 == SD_WRITE_SEC_ARG1) && (SD_arg2 == SD_WRITE_SEC_ARG2))
          {
            //SD write sector command
          }
          else if((SD_arg1 == SD_ERASEFILE_ARG1) && (SD_arg2 == SD_ERASEFILE_ARG2))
          {
            //Erase file command
            //First 4 bytes are the log number in ASCII
            /*
            char log1 = Serial.read();
            char log2 = Serial.read();
            char log3 = Serial.read();
            char log4 = Serial.read();
            */

            //Next 2 bytes are the directory block no
            Serial.read();
            Serial.read();
          }
          else if((SD_arg1 == SD_SPD_TEST_ARG1) && (SD_arg2 == SD_SPD_TEST_ARG2))
          {
            //Perform a speed test on the SD card
            //First 4 bytes are the sector number to write to
            uint32_t sector;
            uint8_t sector1 = serialPayload[7];
            uint8_t sector2 = serialPayload[8];
            uint8_t sector3 = serialPayload[9];
            uint8_t sector4 = serialPayload[10];
            sector = (sector1 << 24) | (sector2 << 16) | (sector3 << 8) | sector4;


            //Last 4 bytes are the number of sectors to test
            uint32_t testSize;
            uint8_t testSize1 = serialPayload[11];
            uint8_t testSize2 = serialPayload[12];
            uint8_t testSize3 = serialPayload[13];
            uint8_t testSize4 = serialPayload[14];
            testSize = (testSize1 << 24) | (testSize2 << 16) | (testSize3 << 8) | testSize4; 

            sendSerialReturnCode(SERIAL_RC_OK);

          }
          else if((SD_arg1 == SD_WRITE_COMP_ARG1) && (SD_arg2 == SD_WRITE_COMP_ARG2))
          {
            //Prepare to read a 2024 byte chunk of data from the SD card
            uint8_t sector1 = serialPayload[7];
            uint8_t sector2 = serialPayload[8];
            uint8_t sector3 = serialPayload[9];
            uint8_t sector4 = serialPayload[10];
            //SDreadStartSector = (sector1 << 24) | (sector2 << 16) | (sector3 << 8) | sector4;
            SDreadStartSector = (sector4 << 24) | (sector3 << 16) | (sector2 << 8) | sector1;
            //SDreadStartSector = sector4 | (sector3 << 8) | (sector2 << 16) | (sector1 << 24);

            //Next 4 bytes are the number of sectors to write
            uint8_t sectorCount1 = serialPayload[11];
            uint8_t sectorCount2 = serialPayload[12];
            uint8_t sectorCount3 = serialPayload[13];
            uint8_t sectorCount4 = serialPayload[14];
            SDreadNumSectors = (sectorCount1 << 24) | (sectorCount2 << 16) | (sectorCount3 << 8) | sectorCount4;

            //Reset the sector counter
            SDreadCompletedSectors = 0;

            sendSerialReturnCode(SERIAL_RC_OK);
          }
        }
        else if(cmd == SD_RTC_PAGE)
        {
          cmdPending = false;
          //Used for setting RTC settings
          if((SD_arg1 == SD_RTC_WRITE_ARG1) && (SD_arg2 == SD_RTC_WRITE_ARG2))
          {
            //Set the RTC date/time
            byte second = serialPayload[7];
            byte minute = serialPayload[8];
            byte hour = serialPayload[9];
            //byte dow = serialPayload[10]; //Not used
            byte day = serialPayload[11];
            byte month = serialPayload[12];
            uint16_t year = word(serialPayload[14], serialPayload[13]);
            rtc_setTime(second, minute, hour, day, month, year);
            sendSerialReturnCode(SERIAL_RC_OK);
          }
        }
#endif
      break;
    }

    default:
      break;
  }
}

/** Send a status record back to tuning/logging SW.
 * This will "live" information from @ref currentStatus struct.
 * @param offset - Start field number
 * @param packetLength - Length of actual message (after possible ack/confirm headers)
 * E.g. tuning sw command 'A' (Send all values) will send data from field number 0, LOG_ENTRY_SIZE fields.
 */
//void sendValues(int packetlength, byte portNum)
void generateLiveValues(uint16_t offset, uint16_t packetLength)
{  
  if(requestCount == 0) { currentStatus.secl = 0; }
  requestCount++;

  currentStatus.spark ^= (-currentStatus.hasSync ^ currentStatus.spark) & (1U << BIT_SPARK_SYNC); //Set the sync bit of the Spark variable to match the hasSync variable

  serialPayload[0] = SERIAL_RC_OK;
  for(byte x=0; x<packetLength; x++)
  {
    serialPayload[x+1] = getTSLogEntry(offset+x); 
  }
  // Reset any flags that are being used to trigger page refreshes
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_VSS_REFRESH);

}

namespace 
{

  inline void send_table_values(table_value_iterator it)
  {
    while (!it.at_end())
    {
      auto row = *it;
      Serial.write(&*row, row.size());
      ++it;
    }
  }

  inline void send_table_axis(table_axis_iterator it)
  {
    while (!it.at_end())
    {
      Serial.write((byte)*it);
      ++it;
    }
  }

}

/** Processes an incoming stream of calibration data (for CLT, IAT or O2) from TunerStudio.
 * Result is store in EEPROM and memory.
 * 
 * @param tableID - calibration table to process. 0 = Coolant Sensor. 1 = IAT Sensor. 2 = O2 Sensor.
 */
void receiveCalibrationNew(byte tableID)
{
  void* pnt_TargetTable_values; //Pointer that will be used to point to the required target table values
  uint16_t* pnt_TargetTable_bins;   //Pointer that will be used to point to the required target table bins
  int OFFSET, DIVISION_FACTOR;

  switch (tableID)
  {
    case 0:
      //coolant table
      pnt_TargetTable_values = (uint16_t *)&cltCalibration_values;
      pnt_TargetTable_bins = (uint16_t *)&cltCalibration_bins;
      OFFSET = CALIBRATION_TEMPERATURE_OFFSET; //
      DIVISION_FACTOR = 10;
      break;
    case 1:
      //Inlet air temp table
      pnt_TargetTable_values = (uint16_t *)&iatCalibration_values;
      pnt_TargetTable_bins = (uint16_t *)&iatCalibration_bins;
      OFFSET = CALIBRATION_TEMPERATURE_OFFSET;
      DIVISION_FACTOR = 10;
      break;
    case 2:
      //O2 table
      //pnt_TargetTable = (byte *)&o2CalibrationTable;
      pnt_TargetTable_values = (uint8_t *)&o2Calibration_values;
      pnt_TargetTable_bins = (uint16_t *)&o2Calibration_bins;
      OFFSET = 0;
      DIVISION_FACTOR = 1;
      break;

    default:
      OFFSET = 0;
      pnt_TargetTable_values = (uint16_t *)&iatCalibration_values;
      pnt_TargetTable_bins = (uint16_t *)&iatCalibration_bins;
      DIVISION_FACTOR = 10;
      break; //Should never get here, but if we do, just fail back to main loop
  }

  int16_t tempValue;
  byte tempBuffer[2];

  if(tableID == 2)
  {
    //O2 calibration. Comes through as 1024 8-bit values of which we use every 32nd
    for (int x = 0; x < 1024; x++)
    {
      while ( Serial.available() < 1 ) {}
      tempValue = Serial.read();

      if( (x % 32) == 0)
      {
        ((uint8_t*)pnt_TargetTable_values)[(x/32)] = (byte)tempValue; //O2 table stores 8 bit values
        pnt_TargetTable_bins[(x/32)] = (x);
      }
      
    }
  }
  else
  {
    //Temperature calibrations are sent as 32 16-bit values
    for (uint16_t x = 0; x < 32; x++)
    {
      while ( Serial.available() < 2 ) {}
      tempBuffer[0] = Serial.read();
      tempBuffer[1] = Serial.read();

      tempValue = (int16_t)(word(tempBuffer[1], tempBuffer[0])); //Combine the 2 bytes into a single, signed 16-bit value
      tempValue = div(tempValue, DIVISION_FACTOR).quot; //TS sends values multipled by 10 so divide back to whole degrees. 
      tempValue = ((tempValue - 32) * 5) / 9; //Convert from F to C
      
      //Apply the temp offset and check that it results in all values being positive
      tempValue = tempValue + OFFSET;
      if (tempValue < 0) { tempValue = 0; }

      
      ((uint16_t*)pnt_TargetTable_values)[x] = tempValue; //Both temp tables have 16-bit values
      pnt_TargetTable_bins[x] = (x * 32U);
      writeCalibration();
    }
  }

  writeCalibration();
}

/** Send 256 tooth log entries to serial.
 * if useChar is true, the values are sent as chars to be printed out by a terminal emulator
 * if useChar is false, the values are sent as a 2 byte integer which is readable by TunerStudios tooth logger
*/
void generateToothLog(byte startOffset)
{
  //We need TOOTH_LOG_SIZE number of records to send to TunerStudio. If there aren't that many in the buffer then we just return and wait for the next call
  if (BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY)) //Sanity check. Flagging system means this should always be true
  {
      for (int x = startOffset; x < TOOTH_LOG_SIZE; x++)
      {
        //Check whether the tx buffer still has space
        if(Serial.availableForWrite() < 4) 
        { 
          //tx buffer is full. Store the current state so it can be resumed later
          inProgressOffset = x;
          toothLogSendInProgress = true;
          return;
        }
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
      toothLogSendInProgress = false;
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

void generateCompositeLog(byte startOffset)
{
  if (BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY)) //Sanity check. Flagging system means this should always be true
  {
      if(startOffset == 0) { inProgressCompositeTime = 0; }
      for (int x = startOffset; x < TOOTH_LOG_SIZE; x++)
      {
        //Check whether the tx buffer still has space
        if(Serial.availableForWrite() < 4) 
        { 
          //tx buffer is full. Store the current state so it can be resumed later
          inProgressOffset = x;
          compositeLogSendInProgress = true;
          return;
        }

        inProgressCompositeTime += toothHistory[toothHistorySerialIndex]; //This combined runtime (in us) that the log was going for by this record)
        
        Serial.write(inProgressCompositeTime >> 24);
        Serial.write(inProgressCompositeTime >> 16);
        Serial.write(inProgressCompositeTime >> 8);
        Serial.write(inProgressCompositeTime);

        Serial.write(compositeLogHistory[toothHistorySerialIndex]); //The status byte (Indicates the trigger edge, whether it was a pri/sec pulse, the sync status)

        if(toothHistorySerialIndex == (TOOTH_LOG_BUFFER-1)) { toothHistorySerialIndex = 0; }
        else { toothHistorySerialIndex++; }
      }
      BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
      toothHistoryIndex = 0;
      toothHistorySerialIndex = 0;
      compositeLastToothTime = 0;
      cmdPending = false;
      compositeLogSendInProgress = false;
      inProgressCompositeTime = 0;
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