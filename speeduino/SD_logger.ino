#ifdef SD_LOGGING
#include <SPI.h>
#include <SD.h>
#include "SD_logger.h"
#include "logger.h"
#include "rtc_common.h"

SdExFat sd;
ExFile logFile;
RingBuf<ExFile, RING_BUF_CAPACITY> rb;
uint8_t SD_status = SD_STATUS_OFF;
uint16_t currentLogFileNumber;
bool manualLogActive = false;

void initSD()
{
  //Set default state to ready. If any stage of the init fails, this will be changed
  SD_status = SD_STATUS_READY; 

  //Set the RTC callback. This is used to set the correct timestamp on file creation and sync operations
  FsDateTime::setCallback(dateTime);

  // Initialize the SD.
  if (!sd.begin(SD_CONFIG)) 
  {
    //sd.initErrorHalt(&Serial);
    //if (sdErrorCode() == SD_CARD_ERROR_CMD0) { SD_status = SD_STATUS_ERROR_NO_CARD;
    SD_status = SD_STATUS_ERROR_NO_CARD;
  }
  
  //Set the TunerStudio status varable
  setTS_SD_status();
}

bool createLogFile()
{
  //TunerStudio only supports 8.3 filename format. 
  char filenameBuffer[13]; //8 + 1 + 3 + 1
  bool returnValue = false;

  /*
  //Filename format is: YYYY-MM-DD_HH.MM.SS.csv
  char intBuffer[5];
  itoa(rtc_getYear(), intBuffer, 10);
  strcpy(filenameBuffer, intBuffer);
  strcat(filenameBuffer, "-");
  itoa(rtc_getMonth(), intBuffer, 10);
  strcat(filenameBuffer, intBuffer);
  strcat(filenameBuffer, "-");
  itoa(rtc_getDay(), intBuffer, 10);
  strcat(filenameBuffer, intBuffer);
  strcat(filenameBuffer, "_");
  itoa(rtc_getHour(), intBuffer, 10);
  strcat(filenameBuffer, intBuffer);
  strcat(filenameBuffer, ".");
  itoa(rtc_getMinute(), intBuffer, 10);
  strcat(filenameBuffer, intBuffer);
  strcat(filenameBuffer, ".");
  itoa(rtc_getSecond(), intBuffer, 10);
  strcat(filenameBuffer, intBuffer);
  strcat(filenameBuffer, ".csv");
  */
  if(currentLogFileNumber == 0)
  {
    //Lookup the next available file number
    currentLogFileNumber = getNextSDLogFileNumber();
  }
  //Create the filename
  sprintf(filenameBuffer, "%s%04d.%s", LOG_FILE_PREFIX, currentLogFileNumber, LOG_FILE_EXTENSION);

  //if (!logFile.open(LOG_FILENAME, O_RDWR | O_CREAT | O_TRUNC)) 
  if (logFile.open(filenameBuffer, O_RDWR | O_CREAT | O_TRUNC)) 
  {
    returnValue = true;
  }

  return returnValue;
}

uint16_t getNextSDLogFileNumber()
{
  uint16_t nextFileNumber = 1;
  char filenameBuffer[13]; //8 + 1 + 3 + 1
  sprintf(filenameBuffer, "%s%04d.%s", LOG_FILE_PREFIX, nextFileNumber, LOG_FILE_EXTENSION);

  //Lookup the next available file number
  while( (nextFileNumber < MAX_LOG_FILES) && (sd.exists(filenameBuffer)) )
  {
    nextFileNumber++;
    sprintf(filenameBuffer, "%s%04d.%s", LOG_FILE_PREFIX, nextFileNumber, LOG_FILE_EXTENSION);
  }

  return nextFileNumber;
}

bool getSDLogFileDetails(uint8_t* buffer, uint16_t logNumber)
{
  bool fileFound = false;

  if(logFile.isOpen()) { endSDLogging(); }

  char filenameBuffer[13]; //8 + 1 + 3 + 1
  sprintf(filenameBuffer, "%s%04d.%s", LOG_FILE_PREFIX, logNumber, LOG_FILE_EXTENSION);
  
  if(sd.exists(filenameBuffer))
  {
    fileFound = true;

    logFile = sd.open(filenameBuffer, O_RDONLY);
    //Copy the filename into the buffer. Note we do not copy the termination character or the fullstop
    for(byte i=0; i<12; i++)
    {
      //We don't copy the fullstop to the buffer
      //As TS requires 8.3 filenames, it's always in the same spot
      if(i < 8) { buffer[i] = filenameBuffer[i]; } //Everything before the fullstop
      else if(i > 8) { buffer[i-1] = filenameBuffer[i]; } //Everything after the fullstop
    }

    //Is File or ignore
    buffer[11] = 1;

    //No idea
    buffer[12] = 0;

    //5 bytes for FAT creation date/time
    uint16_t pDate = 0;
    uint16_t pTime = 0;
    logFile.getCreateDateTime(&pDate, &pTime);
    buffer[13] = 0; //Not sure what this byte is for yet
    buffer[14] = lowByte(pTime);
    buffer[15] = highByte(pTime);
    buffer[16] = lowByte(pDate);
    buffer[17] = highByte(pDate);

    //Sector number (4 bytes) - This byte order might be backwards
    uint32_t sector = logFile.firstSector();
    buffer[18] = ((sector) & 255);
    buffer[19] = ((sector >> 8) & 255);
    buffer[20] = ((sector >> 16) & 255);
    buffer[21] = ((sector >> 24) & 255);

    //Unsure on the below 6 bytes, possibly last accessed or modified date/time?
    buffer[22] = 0;
    buffer[23] = 0;
    buffer[24] = 0;
    buffer[25] = 0;
    buffer[26] = 0;
    buffer[27] = 0;

    //File size (4 bytes). Little endian
    uint32_t size = logFile.fileSize();
    buffer[28] = ((size) & 255);
    buffer[29] = ((size >> 8) & 255);
    buffer[30] = ((size >> 16) & 255);
    buffer[31] = ((size >> 24) & 255);

  }

  return fileFound;
}

void readSDSectors(uint8_t* buffer, uint32_t sectorNumber, uint16_t sectorCount)
{
  sd.card()->readSectors(sectorNumber, buffer, sectorCount);
}

void beginSDLogging()
{
  if(SD_status == SD_STATUS_READY)
  {
    SD_status = SD_STATUS_ACTIVE; //Set the status as being active so that entries will beging to be written. This will be updated below if there is an error

    // Open or create file - truncate existing file.
    if (!createLogFile()) 
    {
      SD_status = SD_STATUS_ERROR_NO_WRITE;
    }

    //Perform pre-allocation on card. This dramatically inproves write speed
    if (!logFile.preAllocate(SD_LOG_FILE_SIZE)) 
    {
      SD_status = SD_STATUS_ERROR_NO_SPACE;
    }

    //initialize the RingBuf.
    rb.begin(&logFile);

    //Write a header row
    writeSDLogHeader();
  }
}

void endSDLogging()
{
  if(SD_status > 0)
  {
    // Write any RingBuf data to file.
    rb.sync();
    logFile.truncate();
    logFile.rewind();
    logFile.close();

    SD_status = SD_STATUS_READY;
  }
}

void writeSDLogEntry()
{
  //Check if we're already running a log
  if(SD_status == SD_STATUS_READY)
  {
    //Log not currently running, check if it should be
    checkForSDStart();
  }

  if(SD_status == SD_STATUS_ACTIVE)
  {
    for(byte x=0; x<SD_LOG_NUM_FIELDS; x++)
    {
      rb.print(getReadableLogEntry(x));
      if(x < (SD_LOG_NUM_FIELDS - 1)) { rb.print(","); }
    }
    rb.println("");

    //Check if write to SD from ringbuffer is needed
    //We write to SD when there is more than 1 sector worth of data in the ringbuffer and there is not already a write being performed
    if( (rb.bytesUsed() >= SD_SECTOR_SIZE) && !logFile.isBusy() )
    {
      uint16_t bytesWritten = rb.writeOut(SD_SECTOR_SIZE); 
      //Make sure that the entire sector was written successfully
      if (SD_SECTOR_SIZE != bytesWritten) 
      {
        SD_status = SD_STATUS_ERROR_WRITE_FAIL;
      }
    }

    //Check whether we should stop logging
    checkForSDStop();
  }
  setTS_SD_status();
}

void writeSDLogHeader()
{
  for(byte x=0; x<SD_LOG_NUM_FIELDS; x++)
  {
    #ifdef CORE_AVR
      //This will probably never be used
      char buffer[30];
      strcpy_P(buffer, (char *)pgm_read_word(&(header_table[x])));
      rb.print(buffer);
    #else
      rb.print(header_table[x]);
    #endif
    if(x < (SD_LOG_NUM_FIELDS - 1)) { rb.print(","); }
  }
  rb.println("");
}

//Sets the status variable for TunerStudio
void setTS_SD_status()
{
  /*
  indicator = { sd_status & 1}, "No SD", "SD in",             white, black, green, black
   indicator = { sd_status & 4}, "SD ready", "SD ready",       white, black, green, black
   indicator = { sd_status & 8}, "SD Log", "SD Log",           white, black, green, black
   indicator = { sd_status & 16}, "SD Err", "SD Err",           white, black, red, black
   */
  //currentStatus.TS_SD_Status = SD_status;

  if( SD_status == SD_STATUS_ERROR_NO_CARD ) { BIT_CLEAR(currentStatus.TS_SD_Status, SD_STATUS_CARD_PRESENT); } // CARD is not present
  else { BIT_SET(currentStatus.TS_SD_Status, SD_STATUS_CARD_PRESENT); } // CARD present

  BIT_SET(currentStatus.TS_SD_Status, SD_STATUS_CARD_TYPE); // CARD is SDHC
  
  BIT_SET(currentStatus.TS_SD_Status, SD_STATUS_CARD_READY); // CARD is ready
  
  if( SD_status == SD_STATUS_ACTIVE ) { BIT_SET(currentStatus.TS_SD_Status, SD_STATUS_CARD_LOGGING); }// CARD is logging
  else { BIT_CLEAR(currentStatus.TS_SD_Status, SD_STATUS_CARD_LOGGING); }// CARD is not logging

  if( (SD_status >= SD_STATUS_ERROR_NO_FS) ) { BIT_SET(currentStatus.TS_SD_Status, SD_STATUS_CARD_ERROR); }// CARD has an error
  else { BIT_CLEAR(currentStatus.TS_SD_Status, SD_STATUS_CARD_ERROR); }// CARD has no error

  BIT_SET(currentStatus.TS_SD_Status, SD_STATUS_CARD_FS); // CARD has a FAT32 filesystem (Though this will be exFAT)
  BIT_CLEAR(currentStatus.TS_SD_Status, SD_STATUS_CARD_UNUSED); //Unused bit is always 0

}

/** 
 * Checks whether the SD logging should be started based on the logging trigger conditions
 */
void checkForSDStart()
{
  //Logging can only start if we're in the ready state
  //We must check the SD_status each time to prevent trying to init a new log file multiple times

  //Check for enable at boot
  if( (configPage13.onboard_log_trigger_boot) && (SD_status == SD_STATUS_READY) )
  {
    //Check that we're not already finished the logging
    if((millis() / 1000) < configPage13.onboard_log_tr1_duration)
    {
      beginSDLogging(); //Setup the log file, prallocation, header row
    }    
  }

  //Check for RPM based Enable
  if( (configPage13.onboard_log_trigger_RPM) && (SD_status == SD_STATUS_READY) )
  {
    if(currentStatus.RPMdiv100 >= configPage13.onboard_log_tr2_thr_on)
    {
      beginSDLogging(); //Setup the log file, prallocation, header row
    }
  }

  //Check for engine protection based enable
  if((configPage13.onboard_log_trigger_prot) && (SD_status == SD_STATUS_READY) )
  {

  }

  if( (configPage13.onboard_log_trigger_Vbat) && (SD_status == SD_STATUS_READY) )
  {

  }

  if(( configPage13.onboard_log_trigger_Epin) && (SD_status == SD_STATUS_READY) )
  {

  }
}

/** 
 * Checks whether the SD logging should be stopped, based on the logging trigger conditions
 */
void checkForSDStop()
{
  //Check the various conditions to see if we should stop logging
  bool log_boot = false;
  bool log_RPM = false;
  bool log_prot = false;
  bool log_Vbat = false;

  //Logging only needs to be stopped if already active
  if(SD_status == SD_STATUS_ACTIVE)
  {
    //Check for enable at boot
    if(configPage13.onboard_log_trigger_boot)
    {
      //Check if we're past the logging duration
      if((millis() / 1000) <= configPage13.onboard_log_tr1_duration)
      {
        log_boot = true;
      }
    }
    if(configPage13.onboard_log_trigger_RPM)
    {
      if(currentStatus.RPMdiv100 <= configPage13.onboard_log_tr2_thr_off)
      {
        log_RPM = true;
      }
    }
    if(configPage13.onboard_log_trigger_prot)
    {

    }
    if(configPage13.onboard_log_trigger_Vbat)
    {

    }

    //Check all conditions to see if we should stop logging
    if( (log_boot == false) && (log_RPM == false) && (log_prot == false) && (log_Vbat == false) && (manualLogActive == false) )
    {
      endSDLogging(); //Setup the log file, prallocation, header row
    }
  }

  
}

/** 
 * Similar to the @getTSLogEntry function, however this returns a full, unadjusted (ie human readable) log entry value. 
 * See logger.h for the field names and order
 * @param logIndex - The log index required. Note that this is NOT the byte number, but the index in the log
 * @return Raw, unadjusted value of the log entry. No offset or multiply is applied like it is with the TS log
 */
void formatExFat()
{
  bool result = false;

  if (sd.cardBegin(SD_CONFIG)) 
  {
    if(sd.format()) 
    {
      if (sd.volumeBegin())
      {
        result = true;
      }
    }
  }

  if(result == false)
  {
    SD_status = SD_STATUS_ERROR_FORMAT_FAIL;
  }
}

// Call back for file timestamps.  Only called for file create and sync().
void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {
  
  // Return date using FS_DATE macro to format fields.
  //*date = FS_DATE(year(), month(), day());
  *date = FS_DATE(rtc_getYear(), rtc_getMonth(), rtc_getDay());

  // Return time using FS_TIME macro to format fields.
  *time = FS_TIME(rtc_getHour(), rtc_getMinute(), rtc_getSecond());
  
  // Return low time bits in units of 10 ms.
  *ms10 = rtc_getSecond() & 1 ? 100 : 0;
}

uint32_t sectorCount()
{
  return sd.card()->sectorCount();
}

#endif