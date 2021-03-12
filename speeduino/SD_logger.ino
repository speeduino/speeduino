#include "globals.h"
#ifdef SD_LOGGING
#include "SD_logger.h"
#include "Arduino.h"
#include "src/coru/coru.h"
#include "rtc_common.h"

//Private functions
void logger_updateLogdataCSV();
void logger_updateLogdataBIN();
void logger_addCSVFieldName(const char fieldname[], uint16_t fieldLength, bool isLastValue);
void updateCSVField(double value, bool isLastValue);
void updateCSVField(int value, bool isLastValue);
double logger_calulateDutyCycle();

//Private variables
File logger_logFile;
char logger_FileBuffer[SD_LOGGER_BUFFER_SIZE]; 
char logger_FileBufferfield[32];
char logger_FileName[32];
char logger_FileExtension[5];
uint16_t logger_bufferIndex;
uint16_t logger_bufferswritten;
uint32_t logger_totalBytesWritten;
bool logger_fileNeedsFlush;
char CSVseparator;

//coroutines for the two functions used in real-time part of the system
//opening/closing and other file access can therefore be blocking!
//each coroutine function needs a buffer to copy the heap to. This take ~2kb
//https://github.com/geky/coru
coru_t AsyncWriteCoroutine;
coru_t AsyncFlushCoroutine;

uint16_t bytes_written = 0;
uint16_t indexOfValue = 0;

/**
 * @brief  Add csv field names on the first line of the file
 * @note   This adds the field names to the file buffer only 
 *          all fields are written to the file when logger_writeLogEntry() is atleasted called once,
 *          and enough data is in the file buffer
 * @param  fieldname[]: list of charter arrays as field names ternmined by NULL
 * @param  fieldLength: Length of the field to add
 * @param  isLastValue: Denotes that its the last field, in that case end of line is added instead of separator
 * @retval None
 */
void logger_addCSVFieldName(const char fieldname[], uint16_t fieldLength, bool isLastValue)
{
    memcpy(&logger_FileBuffer[logger_bufferIndex], fieldname, fieldLength);
    logger_bufferIndex += fieldLength;
    if(!isLastValue){
      logger_FileBuffer[logger_bufferIndex] = CSVseparator;
      logger_bufferIndex += 1;
    }else{
      logger_FileBuffer[logger_bufferIndex] = '\n';
      logger_bufferIndex += 1;
    }
}

/**
 * @brief  Initlilize sdcard sets error if not succesfull 
 * @note   
 * @retval None
 */
void logger_init()
{ 
    if (configPage13.onboard_log_file_style){
        currentStatus.TS_SD_Status = SD_STATUS_OFF; 
        //Init the sdcard.
        if (SD.begin(SD_CS_PIN)) 
        {
            int err_write = coru_create(&AsyncWriteCoroutine, AsyncWriteToLogFile, NULL, 2048);
            int err_flush = coru_create(&AsyncFlushCoroutine, AsyncFlushLogFile, NULL, 2048);
            if (err_write | err_flush) {
               currentStatus.TS_SD_Status = SD_STATUS_ERROR_NO_WRITE;
            }
            currentStatus.TS_SD_Status |= SD_STATUS_CARD_READY;
        }   
        else { currentStatus.TS_SD_Status = SD_STATUS_ERROR_NO_WRITE; }   
    }
}

/**
 * @brief  Opens a log file on the sdcard to start logging to. 
 * @note   
 * @retval None
 */
void logger_openLogFile()
{
    //Attempt to create a log file for writing only if there is no file open
    if(!(currentStatus.TS_SD_Status & SD_STATUS_FS_READY)){

        //Setup file extension for csv and put header in buffer is nessesary for file type..
        if(configPage13.onboard_log_file_style == LOGGER_CSV){
            strncpy(logger_FileExtension, ".csv", 5);
            
            //set the seperation character for CSV
            if (configPage13.onboard_log_csv_separator == LOGGER_CSV_SEPARATOR_COMMA){CSVseparator = ',';}
            if (configPage13.onboard_log_csv_separator == LOGGER_CSV_SEPARATOR_SEMICOLON){CSVseparator = ';';}
            if (configPage13.onboard_log_csv_separator == LOGGER_CSV_SEPARATOR_TAB){CSVseparator = '\t';}
            if (configPage13.onboard_log_csv_separator == LOGGER_CSV_SEPARATOR_SPACE){CSVseparator = ' ';}
        
            //write all fields to the file buffer until NULL field is encountered.
            uint16_t i = 0;
            while (ptr_fields[i]!=NULL)
            {
            logger_addCSVFieldName(ptr_fields[i], strlen(ptr_fields[i]), false);
            i++;
            }
            logger_addCSVFieldName(0, 0, true);               
            
        }

        //Setup file extension for binary file 
        if(configPage13.onboard_log_file_style == LOGGER_BINARY){strncpy(logger_FileExtension, ".bin", 5);}

        //Create file name if date-time format selected csv or bin
        if(configPage13.onboard_log_filenaming==LOGGER_FILENAMING_DATETIME){
            sprintf(logger_FileName, "%02d%02d%02d-%02d%02d%02d-log", rtc_getYear()-2000, rtc_getMonth(), rtc_getDay(), rtc_getHour(), rtc_getMinute(), rtc_getSecond());

            //Shot to make filenames without sprintf because of the overhead. Not succesfull yet.
            //Month march gets 3 in the name not the 03 we want. 
            // char Temp[6];               
            // itoa(rtc_getYear()-2000,Temp, 10);
            // strncat(logger_FileName, Temp, 6);
            // itoa(rtc_getMonth(),Temp, 10);
            // strncat(logger_FileName, Temp, 6);
            // itoa(rtc_getDay(),Temp, 10);
            // strncat(logger_FileName, Temp, 6);
            // strncat(logger_FileName, "-", 2);
            // itoa(rtc_getHour(),Temp, 10);
            // strncat(logger_FileName, Temp, 6);
            // itoa(rtc_getMinute(),Temp, 10);
            // strncat(logger_FileName, Temp, 6);
            // itoa(rtc_getSecond(),Temp, 10);
            // strncat(logger_FileName, Temp, 6);
            // strncat(logger_FileName, "_log", 6);

            strncat(logger_FileName,logger_FileExtension, 5);
        }

        //Create file name if overwrite format selected csv or bin
        if(configPage13.onboard_log_filenaming==LOGGER_FILENAMING_OVERWRITE)
        {
            strncat(logger_FileName, "Speeduino-log", 15);
            strncat(logger_FileName,logger_FileExtension, 5);
            if(SD.exists(logger_FileName))
            {
                SD.remove(logger_FileName);
            } 
        }

        //Create file name if seqential format selected csv or bin
        if(configPage13.onboard_log_filenaming==LOGGER_FILENAMING_SEQENTIAL)
        {
            for (size_t i = 0; i < 999; i++)
            {
                sprintf(logger_FileName, "Speeduino-log%03d", i);
                strncat(logger_FileName,logger_FileExtension, 5);
                if(!SD.exists(logger_FileName)){
                    break;
                } 
            }                   
        }

        //open file on sdcard for logging
        logger_logFile = SD.open(logger_FileName, FILE_WRITE);
        if(logger_logFile) { currentStatus.TS_SD_Status |= SD_STATUS_FS_READY; }
        else { currentStatus.TS_SD_Status &= ~SD_STATUS_FS_READY; }
    }                    
}


/**
 * @brief  Closes the log file that is open to stop logging 
 * @note   TODO call close log file when Vbat drops, and hopefully enough energy is left in capacitors to formally close the file
 * this function needs to be called to close the file. When the board loses power 
 * this function is not called and the file is not closed poperly.This creates orphan sectors on the File system.
 * @retval None
 */
void logger_closeLogFile()
{   
    //get a time reading for time out purposes
    uint32_t millisstart = millis();
 
    //if timeout occurs set error bit and exit
    if((millis()-millisstart)>SD_LOGGER_CLOSE_FILE_TOUT)
    {
        currentStatus.TS_SD_Status = SD_STATUS_ERROR_NO_WRITE;
        return;}

    //write buffer to sdcard before closing
    logger_logFile.write(logger_FileBuffer, logger_bufferIndex);  
    logger_bufferIndex = 0;
    if(logger_logFile){
        logger_logFile.close();
        logger_bufferIndex = 0;
    }
    currentStatus.TS_SD_Status &= ~SD_STATUS_FS_READY;
    currentStatus.TS_SD_Status &= ~SD_STATUS_LOGGING;
}

/**
 * @brief  Write logger data to media.
 * @note   Must be called enough times to prevent buffer overflow.
 * @retval None
 */
void logger_WriteBufferToDisk()
{
    //Only try if file is open
    if(currentStatus.TS_SD_Status & SD_STATUS_FS_READY){
        //Log data to sd card if there is more data in the buffer then the trigger bytes.
        if ((logger_bufferIndex > SD_LOGGER_WRITE_TRIG)){

            //if buffer is filling up stop logging to prevent buffer overflow.
            if (logger_bufferIndex >= SD_LOGGER_BUFFER_SIZE-SD_LOGGER_WRITE_TRIG)
            {
                currentStatus.TS_SD_Status = SD_STATUS_ERROR_NO_WRITE;

                //Try to close the file else all data is lossed. 
                logger_closeLogFile();
            }

            //Only write data to sdcard if no flush is needed at this moment
            if (!logger_fileNeedsFlush)
            {               
                //New async write to sdcard with coroutines.
                //When coru_resume returns CORU_ERR_AGAIN the function is waiting and called yield, need to call 
                //it again next run. When coru_resume returns != CORU_ERR_AGAIN the function is done and the buffer
                //is written to sdcard.
                if(coru_resume(&AsyncWriteCoroutine)!=CORU_ERR_AGAIN){
                    logger_bufferIndex -= bytes_written;
                    logger_totalBytesWritten += bytes_written;  
                    memcpy(&logger_FileBuffer, &logger_FileBuffer[bytes_written], logger_bufferIndex);
                    logger_bufferswritten++;  
                    
                    //destroy co routine because its done
                    coru_destroy(&AsyncWriteCoroutine);   
                    
                    //create new co routine for next write
                    coru_create(&AsyncWriteCoroutine, AsyncWriteToLogFile, NULL, 4096);              
                }

            //File flush needed. so this instead.     
            //Flush the file, all size and last modified data is written to the file
            }else{                               
                if(coru_resume(&AsyncFlushCoroutine)!=CORU_ERR_AGAIN){
                    logger_fileNeedsFlush = false;
                    
                    //destroy co routine because its done
                    coru_destroy(&AsyncFlushCoroutine);   
                    
                    //create new co routine for next flush
                    coru_create(&AsyncFlushCoroutine, AsyncFlushLogFile, NULL, 4096);    
                }
            }
            
            //trigger a file flush on next write of datalogger.
            if (logger_bufferswritten>SD_LOGGER_FLUSH_FILE_TRIG)
            {   
                logger_fileNeedsFlush = true;
                logger_bufferswritten = 0;
            }  
        }
    }
}


/**
 * @brief  Write a log entry to file buffer this function is called with 1,4,10,30hz depending on settings
 * @note   
 * @retval None
 */
void logger_writeLogEntry()
{   
    //Only run logger if file is acutally open.
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
        
    //Write data to disk  
    logger_WriteBufferToDisk();
           
    }else{
        currentStatus.TS_SD_Status &= !SD_STATUS_LOGGING;
    }

}

/**
 * @brief  Create a CSV style ASCII field for one value, terminated by the desired separator 
 * @note   This is the float version. There is also a integer version 
 * @param  value: The integer value to log (No floats in speeduino at the moment)
 * @param  isLastValue: Set to true if it is the last value of this log line.
 * @retval None
 */
void updateCSVField(double value, bool isLastValue)
{
    //Make string out of the values 
    dtostrf(value, 7 ,3,logger_FileBufferfield);
    writeCSVField(logger_FileBufferfield, strlen(logger_FileBufferfield), isLastValue);

}

/**
 * @brief  Create a CSV style ASCII field for one value, terminated by the desired separator 
 * @note   This is the integer version. There is also a float version
 * @param  value: The integer value to log (No floats in speeduino at the moment)
 * @param  isLastValue: Set to true if it is the last value of this log line.
 * @retval None
 */
void updateCSVField(int value, bool isLastValue)
{
    //Make string out of the values 
    itoa(value,logger_FileBufferfield,10);
    writeCSVField(logger_FileBufferfield, strlen(logger_FileBufferfield), isLastValue);

}
/**
 * @brief  writes the string to filebuffer for writing
 * @note   
 * @param  *value: value to write in chararray (string)
 * @param  length: length of usefull information in the string
 * @param  isLastValue: is tue ifit is the last value and line needs temination.
 * @retval None
 */
void writeCSVField(char *value, uint16_t length, bool isLastValue){
    //Copy string from temp buffer to logbuffer 
    memcpy(&logger_FileBuffer[logger_bufferIndex], &logger_FileBufferfield, length);

    //increase logbuffer index to new position 
    logger_bufferIndex += length;

    //end of line charter needs to be added after last value.
    if (isLastValue)
    {   
        logger_FileBuffer[logger_bufferIndex] = '\n';
        logger_bufferIndex += 1;
        indexOfValue++;
    }else
    {
        //Every field has a separator except for the last field, add this to the logbuffer too
        logger_FileBuffer[logger_bufferIndex] = CSVseparator;
        logger_bufferIndex += 1;
        indexOfValue = 0;
    }
}



/**
 * @brief  Calulate duty cyle from other values for datalogging (normally tuner studio does this)
 * @note   its done in double precision so its easier for csv logger to log accurately.
 * @retval returns duty cycle 
 */
double logger_calulateDutyCycle()
{
   uint32_t strokeMultipler = 2;
   if(configPage2.strokes==1){ strokeMultipler = 1;}else{ strokeMultipler = 2;};
   //TODO find right setting for squirts per cycle for calulation
   uint32_t pulseLimit   = ((60000000/currentStatus.RPM)*strokeMultipler)/currentStatus.nSquirts; 
   double dutyCycle    = (100.0*currentStatus.PW1)/pulseLimit;
  
   //TODO add staging duty
   //stgDutyCycle     = { rpm && stagingEnabled ? ( 100.0*pulseWidth3/pulseLimit ) : 0      }
   return dutyCycle;
}


/**
 * @brief  Adds the CSV fields as one line to the file buffer
 * @note   
 * @retval None
 */
void logger_updateLogdataCSV()
{
    // There is no convient way to access al struct memebers, therefore is the current implementation.
    updateCSVField(currentStatus.hasSync, false);
    updateCSVField(currentStatus.RPM, false);
    updateCSVField((int)currentStatus.MAP, false);
    updateCSVField(currentStatus.TPS, false);
    updateCSVField(currentStatus.tpsDOT, false);
    updateCSVField(currentStatus.mapDOT, false);
    updateCSVField(currentStatus.rpmDOT, false);
    updateCSVField(currentStatus.VE1, false);
    updateCSVField(currentStatus.VE2, false);
    updateCSVField((double)currentStatus.O2/10, false);
    updateCSVField((double)currentStatus.O2_2/10, false);
    updateCSVField(currentStatus.coolant, false);
    updateCSVField(currentStatus.IAT, false);
    updateCSVField((double)currentStatus.dwell/1000, false);
    updateCSVField((double)currentStatus.battery10/10, false);
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
    updateCSVField((double)currentStatus.afrTarget/10, false);
    updateCSVField(currentStatus.idleDuty, false);
    updateCSVField(currentStatus.CLIdleTarget, false);
    updateCSVField(currentStatus.idleUpActive, false);
    updateCSVField(currentStatus.CTPSActive, false);
    updateCSVField(currentStatus.fanOn, false);
    updateCSVField(currentStatus.ethanolPct, false);
    updateCSVField((double)currentStatus.fuelTemp/10, false);
    updateCSVField((int)currentStatus.AEEndTime, false);
    updateCSVField(currentStatus.status1, false);
    updateCSVField(currentStatus.spark, false);
    updateCSVField(currentStatus.spark2, false);
    updateCSVField(currentStatus.engine, false);
    updateCSVField((int)currentStatus.PW1, false);
    updateCSVField((int)currentStatus.PW2, false);
    updateCSVField((int)currentStatus.PW3, false);
    updateCSVField((int)currentStatus.PW4, false);
    updateCSVField((int)currentStatus.PW5, false);
    updateCSVField((int)currentStatus.PW6, false);
    updateCSVField((int)currentStatus.PW7, false);
    updateCSVField((int)currentStatus.PW8, false);
    updateCSVField(currentStatus.runSecs, false);
    updateCSVField(currentStatus.secl, false);
    updateCSVField((int)currentStatus.loopsPerSecond, false);
    updateCSVField(currentStatus.launchingSoft, false);
    updateCSVField(currentStatus.launchingHard, false);
    updateCSVField(freeRam(), false);
    updateCSVField((int)currentStatus.startRevolutions, false);
    updateCSVField(currentStatus.boostTarget, false);
    updateCSVField(currentStatus.testOutputs, false);
    updateCSVField(currentStatus.testActive, false);
    updateCSVField((double)currentStatus.boostDuty/100, false);
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
    updateCSVField((int)currentStatus.vvt1Angle, false);
    updateCSVField((int)currentStatus.vvt1Angle, false);
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
    updateCSVField(currentStatus.wmiPW, false);
    updateCSVField(logger_calulateDutyCycle(), false);
    
    //Time field is needed for mega logviewer 
    updateCSVField((double)millis()/1000, true);
}


/**
 * @brief  Adds the binary 'A' style data to the file buffer for logging 
 * @note   Writes all data in binary form the same as the 'A' command retrieves all real-time current status data
 * @retval None
 */
void logger_updateLogdataBIN()
{
  for(byte x=0; x<116; x++)
  {
      logger_FileBuffer[logger_bufferIndex] = getStatusEntry(x);
      logger_bufferIndex ++;
  }
}

/**
 * @brief  Helper function to encapsulate write sd card logger non-blocking with the help of coroutines 
 * @note   https://github.com/geky/coru
 * @param  arg: 
 * @retval None
 */
void AsyncWriteToLogFile(void* arg)
{
    bytes_written = logger_logFile.write(&logger_FileBuffer[0], SD_LOGGER_WRITE_TRIG);
}

/**
 * @brief  Helper function to encapsulate flush file on sd card non-blocking with the help of coroutines 
 * @note   https://github.com/geky/coru
 * @param  arg: 
 * @retval None
 */
void AsyncFlushLogFile(void* arg)
{
    logger_logFile.flush();
}

#endif