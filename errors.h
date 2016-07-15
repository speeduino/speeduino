#ifndef ERRORS_H
#define ERRORS_H

/*
 * Up to 64 different error codes may be defined (6 bits)
 */
#define ERR_NONE        0 //No error
#define ERR_UNKNOWN     1 //Unknown error
#define ERR_IAT_SHORT   2 //Inlet sensor shorted
#define ERR_IAT_GND     3 //Inlet sensor grounded
#define ERR_CLT_SHORT   4 //Coolant sensor shorted
#define ERR_CLT_GND     5 //Coolant Sensor grounded
#define ERR_O2_SHORT    6 //O2 sensor shorted
#define ERR_O2_GND      7 //O2 sensor grounded
#define ERR_TPS_SHORT   8 //TPS shorted (Is potentially valid)
#define ERR_TPS_GND     9 //TPS grounded (Is potentially valid)
#define ERR_BAT_HIGH    10 //Battery voltage is too high
#define ERR_BAT_LOW     11 //Battery voltage is too low

#define MAX_ERRORS  4 //The number of errors the system can hold simultaneously. Should be a power of 2

/*
 * This struct is a single byte in length and is sent to TS
 * The first 2 bits are used to define the current error (0-3)
 * The remaining 6 bits are used to give the error number
 */
struct packedError
{
  byte errorNum : 2;
  byte errorID  : 6;
};

byte getNextError();
byte setError(byte);

byte errorCount = 0;
byte errorCodes[4];


#endif
