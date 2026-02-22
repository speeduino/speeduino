/** \file comms.h
 * @brief File for handling all serial requests 
 * @author Josh Stewart
 * 
 * This file contains all the functions associated with serial comms.
 * This includes sending of live data, sending/receiving current page data, sending CRC values of pages, receiving sensor calibration data etc
 * 
 */

#ifndef COMMS_H
#define COMMS_H

extern Stream *pPrimarySerial;
#define primarySerial (*pPrimarySerial)

extern uint32_t serialReceiveStartTime; //!< The time in milliseconds at which the serial receive started. Used for calculating whether a timeout has occurred

/**
 * @brief The serial receive pump. Should be called whenever the serial port
 * has data available to read.
 */
void serialReceive(void);

/** @brief The serial transmit pump. Should be called when ::serialStatusFlag indicates a transmit
 * operation is in progress */
void serialTransmit(void);

/** @brief Checks whether the current serial command should be timed out 
 * 
 * @return true if the serial command has been waiting too long
*/
bool isRxTimeout(void);

/**
 * @brief During serial comms, defer storage writes
 * 
 * Serial comms can send data quicker than we can write it to permanent storage.
 * This is used to manually throttle the writes so that we don't stall the main loop. 
 * 
 * @param time Absolute time in µS 
 */
void setStorageWriteTimeout(uint32_t time);

/** @brief Test if the timeout set by @ref setStorageWriteTimeout has expired */
bool storageWriteTimeoutExpired(void);

#endif // COMMS_H
