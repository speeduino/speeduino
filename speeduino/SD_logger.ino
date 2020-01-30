#ifdef SD_LOGGING
#include <SPI.h>
#include <SD.h>
#include "SD_logger.h"
#include "logger.h"

void initSD()
{
    //Init the SPI connection to the card reader
    if (SD.begin(SD_CS_PIN)) 
    {
        //Attempt to create a log file for writing
        logFile = SD.open("datalog.csv", FILE_WRITE);
        if(!logFile) { SD_status = SD_STATUS_ERROR_NO_WRITE; } //Cannot write to SD card
        else { SD_status = SD_STATUS_READY; }
    }   
    else { SD_status = SD_STATUS_ERROR_NO_CARD; }


    
}

void endSD()
{

}

void writeSDLog()
{
    uint8_t logEntry[LOG_ENTRY_SIZE];
    
}

#endif