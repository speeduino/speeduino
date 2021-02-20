#include "globals.h"
#ifdef SD_LOGGING
#include "SD_logger.h"
#include "Arduino.h"
#include "src/STM32SD/bsp_sd.h"
#include "rtc_common.h"

//Private functions
void logger_updateLogdataCSV();
void logger_updateLogdataBIN();
void logger_addField(const char fieldname[], uint16_t fieldLength, bool lastValue);

//Private variables
File logger_logFile;
uint8_t LogBufferBIN[128]; 
char logger_LogBufferCSV[SD_LOGGER_BUFFER_SIZE]; 
char logger_LogBufferCSVfield[32];
char logger_fileName[32];
uint16_t logger_bufferIndex;
uint16_t logger_bufferswritten;
uint32_t logger_totalBytesWritten;
bool logger_fileNeedsFlush;
char CSVseparator;

#ifdef RTC_ENABLED
//This function is called by the fatfs library to get the current time if a file is created or modified to update the file time
uint32_t get_fattime (void){
  uint32_t time;
  time = (41<< 25) | (((uint32_t)rtc_getMonth())<< 21) | (((uint32_t)rtc_getDay())<< 16) | (((uint32_t)rtc_getHour())<< 11) | (((uint32_t)rtc_getMinute())<< 5) | (((uint32_t)rtc_getSecond())>>1);
  return time;
}
#endif

/**
  * @brief  Get information if sd card is busy or ready to receive new data.
  * @retval true if ready else false
  */
uint8_t logger_getCardState()
{
#ifdef __BSP_SD_H
    return BSP_SD_GetCardState();
#elif
    //need a way to get this info specific for each platform.
    return false;
#endif
}

/**
  * @brief  Add field names to the first line of the CSV file.
  * @retval true if ready else false
  */
void logger_addCSVFieldName(const char fieldname[], uint16_t fieldLength, bool lastValue)
{
    memcpy(&logger_LogBufferCSV[logger_bufferIndex], fieldname, fieldLength);
    logger_bufferIndex += fieldLength;
    if(!lastValue){
      logger_LogBufferCSV[logger_bufferIndex] = CSVseparator;
      logger_bufferIndex += 1;
    }else{
      logger_LogBufferCSV[logger_bufferIndex] = '\n';
      logger_bufferIndex += 1;
    }
}

void logger_init()
{ 
    if (configPage13.onboard_log_file_style){
        currentStatus.TS_SD_Status = SD_STATUS_OFF; 
        //Init the sdcard.
        if (SD.begin(SD_CS_PIN)) 
        {
            currentStatus.TS_SD_Status |= SD_STATUS_CARD_READY;
        }   
        else { currentStatus.TS_SD_Status |= SD_STATUS_ERROR_NO_WRITE; }   
    }
}

void logger_openLogFile()
{
    //Attempt to create a log file for writing only if there is no file open
    if(!(currentStatus.TS_SD_Status & SD_STATUS_FS_READY)){
        if(configPage13.onboard_log_file_style == LOGGER_CSV)
        {
            //Create file name
            sprintf(logger_fileName, "%02d%02d%02d-%02d%02d%02d.csv", rtc_getYear()-2000, rtc_getMonth(), rtc_getDay(), rtc_getHour(), rtc_getMinute(), rtc_getSecond());

            //open file on sdcard
            logger_logFile = SD.open(logger_fileName, FILE_WRITE);
            if(logger_logFile) { currentStatus.TS_SD_Status |= SD_STATUS_FS_READY; }
            else { currentStatus.TS_SD_Status &= ~SD_STATUS_FS_READY; }
                    
            //Write a CSV header to the sd writing buffer if the file is opened succesfully
            if(currentStatus.TS_SD_Status & SD_STATUS_FS_READY)
            {
                //set the seperation character for CSV
                if (configPage13.onboard_log_csv_separator == LOGGER_CSV_SEPARATOR_COMMA){CSVseparator = ',';}
                if (configPage13.onboard_log_csv_separator == LOGGER_CSV_SEPARATOR_SEMICOLON){CSVseparator = ';';}
                if (configPage13.onboard_log_csv_separator == LOGGER_CSV_SEPARATOR_TAP){CSVseparator = '\t';}
                if (configPage13.onboard_log_csv_separator == LOGGER_CSV_SEPARATOR_SPACE){CSVseparator = ' ';}
            
                //write all fields to the buffer until NULL character is encountered.
                uint16_t i = 0;
                while (ptr_fields[i]!=NULL)
                {
                logger_addCSVFieldName(ptr_fields[i], strlen(ptr_fields[i]), false);
                i++;
                }
                logger_addCSVFieldName(ptr_fields[i], strlen(ptr_fields[i]), true);
                
            }
        }
        if(configPage13.onboard_log_file_style == LOGGER_BINARY)
        {
            //Create file name
            sprintf(logger_fileName, "%02d%02d%02d-%02d%02d%02d.bin", rtc_getYear()-2000, rtc_getMonth(), rtc_getDay(), rtc_getHour(), rtc_getMinute(), rtc_getSecond());

            //open file on sdcard
            logger_logFile = SD.open(logger_fileName, FILE_WRITE);
            if(logger_logFile) { currentStatus.TS_SD_Status |= SD_STATUS_FS_READY; }
            else { currentStatus.TS_SD_Status &= ~SD_STATUS_FS_READY; }


            //TODO Create megalogviewer file format header                     

        }
    }
}

//this function needs to be called to close the file. When the board loses power 
//this function is not called and the file is not closed poperly.This creates 
//orphan sectors on the File system.
void logger_closeLogFile()
{   
    //get a time reading for time out purposes
    uint32_t millisstart = millis();
    while(logger_getCardState())
    {   
        //if timeout occurs set error bit and exit
        if((millis()-millisstart)>SD_LOGGER_CLOSE_FILE_TOUT)
        {
            currentStatus.TS_SD_Status = SD_STATUS_ERROR_NO_WRITE;
            return;}
    };

    //write buffer to sdcard before closing
    logger_logFile.write(logger_LogBufferCSV, logger_bufferIndex);  
    logger_bufferIndex = 0;
    if(logger_logFile){
        logger_logFile.close();
        logger_bufferIndex = 0;
    }
    currentStatus.TS_SD_Status &= ~SD_STATUS_FS_READY;
    currentStatus.TS_SD_Status &= ~SD_STATUS_LOGGING;
}

void logger_writeLogEntry()
{
    uint16_t bytes_written = 0;
    
    //only run logger if file is acutally open.
    if(currentStatus.TS_SD_Status & SD_STATUS_FS_READY)
    {
        if(configPage13.onboard_log_file_style==LOGGER_CSV)
        {
            currentStatus.TS_SD_Status |= SD_STATUS_LOGGING;
            //Create CSV line fron current status struct.
            logger_updateLogdataCSV();
        }
        if(configPage13.onboard_log_file_style==LOGGER_BINARY)
        {
            currentStatus.TS_SD_Status |= SD_STATUS_LOGGING;
            //Create CSV line fron current status struct.
            logger_updateLogdataBIN();
        }


        //Log data to sd card if there is more data in the buffer than the trigger bytes.
        if ((logger_bufferIndex > SD_LOGGER_WRITE_TRIG)){

            //if buffer is filling up stop logging to prevent buffer overflow.
            if (logger_bufferIndex >= SD_LOGGER_BUFFER_SIZE-128)
            {
                currentStatus.TS_SD_Status = SD_STATUS_ERROR_NO_WRITE;

                //Try to close the file else all data is lossed. 
                logger_closeLogFile();
            }

            //Only try to write data to the sdcard when the card is ready.
            if (logger_getCardState()==0)
            {
                //Only write data to sdcard if no flush is needed at this moment
                if (!logger_fileNeedsFlush)
                {
                    bytes_written = logger_logFile.write(&logger_LogBufferCSV[0], SD_LOGGER_WRITE_TRIG); 
                    logger_bufferIndex -= bytes_written;
                    logger_totalBytesWritten += bytes_written;  
                    memcpy(&logger_LogBufferCSV, &logger_LogBufferCSV[bytes_written], logger_bufferIndex);
                    logger_bufferswritten++;
                //File flush needed. so this instead.     
                }else
                {   
                    //Flush the file, all size and last modified data is written to the file
                    logger_logFile.flush();
                    logger_fileNeedsFlush = false;
                }
            }
            //trigger a file flush on next write of datalogger.
            if (logger_bufferswritten>SD_LOGGER_FLUSH_FILE_TRIG)
            {   
                logger_fileNeedsFlush = true;
                logger_bufferswritten = 0;
            }           
        }
        
           
    }else{
        currentStatus.TS_SD_Status &= !SD_STATUS_LOGGING;
    }

}
void updateCSVField(long value, bool lastValue)
{
    //Make string out of the values 
    itoa(value,logger_LogBufferCSVfield,10);
    uint16_t length = strlen(logger_LogBufferCSVfield);

    //Copy string from temp buffer to logbuffer 
    memcpy(&logger_LogBufferCSV[logger_bufferIndex], &logger_LogBufferCSVfield, length);

    //increase logbuffer index to new position 
    logger_bufferIndex += length;

    //end of line charter needs to be added after last value.
    if (lastValue)
    {   
        logger_LogBufferCSV[logger_bufferIndex] = '\n';
        logger_bufferIndex += 1;
    }else
    {
        //Every field has a seperator except for the last field, add this to the logbuffer too
        logger_LogBufferCSV[logger_bufferIndex] = CSVseparator;
        logger_bufferIndex += 1;
    }
}

void logger_updateLogdataCSV()
{
    // There is no convient way to access al struct memebers, therefore is the current implementation.
    updateCSVField(currentStatus.hasSync, false);
    updateCSVField(currentStatus.RPM, false);
    updateCSVField(currentStatus.MAP, false);
    updateCSVField(currentStatus.TPS, false);
    updateCSVField(currentStatus.tpsDOT, false);
    updateCSVField(currentStatus.mapDOT, false);
    updateCSVField(currentStatus.rpmDOT, false);
    updateCSVField(currentStatus.VE1, false);
    updateCSVField(currentStatus.VE2, false);
    updateCSVField(currentStatus.O2, false);
    updateCSVField(currentStatus.O2_2, false);
    updateCSVField(currentStatus.coolant, false);
    updateCSVField(currentStatus.IAT, false);
    updateCSVField(currentStatus.dwell, false);
    updateCSVField(currentStatus.battery10, false);
    updateCSVField(currentStatus.advance, false);
    updateCSVField(currentStatus.advance1, false);
    updateCSVField(currentStatus.advance2, false);
    updateCSVField(currentStatus.corrections, false);
    updateCSVField(currentStatus.AEamount, false);
    updateCSVField(currentStatus.egoCorrection, false);
    updateCSVField(currentStatus.wueCorrection, false);
    updateCSVField(currentStatus.batCorrection, false);
    updateCSVField(currentStatus.iatCorrection, false);
    updateCSVField(currentStatus.baroCorrection, false);
    updateCSVField(currentStatus.launchCorrection, false);
    updateCSVField(currentStatus.flexCorrection, false);
    updateCSVField(currentStatus.fuelTempCorrection, false);
    updateCSVField(currentStatus.flexIgnCorrection, false);
    updateCSVField(currentStatus.afrTarget, false);
    updateCSVField(currentStatus.idleDuty, false);
    updateCSVField(currentStatus.CLIdleTarget, false);
    updateCSVField(currentStatus.idleUpActive, false);
    updateCSVField(currentStatus.CTPSActive, false);
    updateCSVField(currentStatus.fanOn, false);
    updateCSVField(currentStatus.ethanolPct, false);
    updateCSVField(currentStatus.fuelTemp, false);
    updateCSVField(currentStatus.AEEndTime, false);
    updateCSVField(currentStatus.status1, false);
    updateCSVField(currentStatus.spark, false);
    updateCSVField(currentStatus.spark2, false);
    updateCSVField(currentStatus.engine, false);
    updateCSVField(currentStatus.PW1, false);
    updateCSVField(currentStatus.PW2, false);
    updateCSVField(currentStatus.PW3, false);
    updateCSVField(currentStatus.PW4, false);
    updateCSVField(currentStatus.PW5, false);
    updateCSVField(currentStatus.PW6, false);
    updateCSVField(currentStatus.PW7, false);
    updateCSVField(currentStatus.PW8, false);
    updateCSVField(currentStatus.runSecs, false);
    updateCSVField(currentStatus.secl, false);
    updateCSVField(currentStatus.loopsPerSecond, false);
    updateCSVField(currentStatus.launchingSoft, false);
    updateCSVField(currentStatus.launchingHard, false);
    updateCSVField(freeRam(), false);
    updateCSVField(currentStatus.startRevolutions, false);
    updateCSVField(currentStatus.boostTarget, false);
    updateCSVField(currentStatus.testOutputs, false);
    updateCSVField(currentStatus.testActive, false);
    updateCSVField(currentStatus.boostDuty, false);
    updateCSVField(currentStatus.idleLoad, false);
    updateCSVField(currentStatus.status3, false);
    updateCSVField(currentStatus.flexBoostCorrection, false);
    updateCSVField(currentStatus.nitrous_status, false);
    updateCSVField(currentStatus.fuelLoad, false);
    updateCSVField(currentStatus.fuelLoad2, false);
    updateCSVField(currentStatus.ignLoad, false);
    updateCSVField(currentStatus.fuelPumpOn, false);
    updateCSVField(currentStatus.syncLossCounter, false);
    updateCSVField(currentStatus.knockRetard, false);
    updateCSVField(currentStatus.knockActive, false);
    updateCSVField(currentStatus.toothLogEnabled, false);
    updateCSVField(currentStatus.compositeLogEnabled, false);
    updateCSVField(currentStatus.vvt1Angle, false);
    updateCSVField(currentStatus.vvt1Angle, false);
    updateCSVField(currentStatus.vvt1TargetAngle, false);
    updateCSVField(currentStatus.vvt1Duty, false);
    updateCSVField(currentStatus.injAngle, false);
    updateCSVField(currentStatus.ASEValue, false);
    updateCSVField(currentStatus.vss, false);
    updateCSVField(currentStatus.idleUpOutputActive, false);
    updateCSVField(currentStatus.gear, false);
    updateCSVField(currentStatus.fuelPressure, false);
    updateCSVField(currentStatus.oilPressure, false);
    updateCSVField(currentStatus.engineProtectStatus, false);
    updateCSVField(currentStatus.wmiPW, true);
}

void logger_updateLogdataBIN()
{
  for(byte x=0; x<116; x++)
  {
      logger_LogBufferCSV[logger_bufferIndex] = getStatusEntry(x);
      logger_bufferIndex ++;
  }
}
#endif