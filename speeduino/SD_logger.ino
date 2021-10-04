#ifdef SD_LOGGING
#include <SPI.h>
#include <SD.h>
#include "SD_logger.h"
#include "logger.h"

void initSD()
{
  //Set default state to ready. If any stage of the init fails, this will be changed
  SD_status = SD_STATUS_READY; 

  // Initialize the SD.
  if (!sd.begin(SD_CONFIG)) 
  {
    //sd.initErrorHalt(&Serial);
    //if (sdErrorCode() == SD_CARD_ERROR_CMD0) { SD_status = SD_STATUS_ERROR_NO_CARD;
    SD_status = SD_STATUS_ERROR_NO_CARD;
  }

  // Open or create file - truncate existing file.
  if (!logFile.open(LOG_FILENAME, O_RDWR | O_CREAT | O_TRUNC)) 
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
  if(SD_status == SD_STATUS_READY)
  {
    //logFile.println("Field 1,Field 2,Test");
  }
  
  //Set the TunerStudio status varable
  setTS_SD_status();
}

void endSD()
{
  // Write any RingBuf data to file.
  rb.sync();
  logFile.truncate();
  logFile.rewind();
  logFile.close();
}

void writeSDLogEntry()
{
  //uint8_t logEntry[LOG_ENTRY_SIZE];

  if(SD_status == SD_STATUS_READY)
  {
    for(byte x=0; x<SD_LOG_ENTRY_SIZE; x++)
    {
      rb.write(getLogEntry(x));
      rb.print(",");
    }
    rb.println("");
  }

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
  currentStatus.TS_SD_Status = 0;
  if(SD_status != SD_STATUS_ERROR_NO_CARD) { BIT_SET(currentStatus.TS_SD_Status, 0); } //Set bit for SD card being present
  if(SD_status == SD_STATUS_READY) { BIT_SET(currentStatus.TS_SD_Status, 2); } //Set bit for SD card being ready

}

#endif