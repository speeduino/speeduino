#ifdef SD_LOGGING
#include <SPI.h>
#include <SD.h>
#include "SD_logger.h"
#include "logger.h"

void initSD()
{
  //Init the SPI connection to the card reader
  //if (SD_card.begin(SD_CS_PIN)) 
  if (SD_card.init(SPI_HALF_SPEED, SD_CS_PIN))
  {
    //Check for usable FAT32 volume
    if (SD_volume.init(SD_card))
    {
      //Attempt to create a log file for writing
      //logFile = SD_card.open("datalog.csv", FILE_WRITE);
      //if(!logFile) { SD_status = SD_STATUS_ERROR_NO_WRITE; } //Cannot write to SD card
      //else { SD_status = SD_STATUS_READY;
      SD_status = SD_STATUS_READY;
    }
    else { SD_status = SD_STATUS_ERROR_NO_FS; }
  }   
  else { SD_status = SD_STATUS_ERROR_NO_CARD; }

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

}

void writeSDLogEntry()
{
    //uint8_t logEntry[LOG_ENTRY_SIZE];

    if(SD_status == SD_STATUS_READY)
    {
      //createSDLog(&logEntry);

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