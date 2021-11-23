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
  char filenameBuffer[24];
  bool returnValue = false;

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

  //if (!logFile.open(LOG_FILENAME, O_RDWR | O_CREAT | O_TRUNC)) 
  if (logFile.open(filenameBuffer, O_RDWR | O_CREAT | O_TRUNC)) 
  {
    returnValue = true;
  }

  return returnValue;
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
  //Logging only needs to be stopped if already active
  if(SD_status == SD_STATUS_ACTIVE)
  {
    //Check for enable at boot
    if(configPage13.onboard_log_trigger_boot)
    {
      //Check if we're past the logging duration
      if((millis() / 1000) > configPage13.onboard_log_tr1_duration)
      {
        endSDLogging(); //Setup the log file, prallocation, header row
      }
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

#endif