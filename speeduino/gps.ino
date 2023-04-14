#include "globals.h"
#include "gps.h"
#include "maths.h"
#include "src/MicroNMEA/MicroNMEA.h"

/*
  For now only teensy is supported on Serial3, should use ~500 bytes of volatile memory
*/

#if ( defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) )
  //TBD
  /*
  Uses 96.4% (7895 bytes RAM) - 86.8% FLASH (ORIGINAL USES 94.2% (7716 bytes RAM) - 83.5% FLASH)
  HardwareSerial &gps = Serial3;
  char nmeaBuffer[100];
  bool gpsOnAUX = false;
  MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

  int32_t gpsLat;
  int32_t gpsLong;
  int32_t gpsAltitude;
  int32_t gpsSpeed;
  */
#elif defined(CORE_STM32)
  //TBD
#elif defined(CORE_TEENSY)
  HardwareSerial &gps = Serial3;
  char nmeaBuffer[100];
  
  MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

  int32_t gpsLat;
  int32_t gpsLong;
  int32_t gpsAltitude;
  int32_t gpsSpeed;
#endif

bool gpsOnAUX = false;

void gpsSerialReceive(void)
{
  #if defined(GPSSerial_AVAILABLE)
    //Read all buffer if more than 1 char is received, process data at each char received
    while (gps.available()>=0)
    { 
      char gpsChar = gps.read();
      bool gpsProcessEsit = nmea.process(gpsChar);

      //If received valid NMEA then update data
      if(gpsProcessEsit && nmea.isValid())
      {
        gpsLat = nmea.getLatitude(); //int32 value for latitude in 1/1e6 deg (one millionth degree) -->auxIn 10 H & 11 L
        gpsLong = nmea.getLongitude();//int32 value for longitude in 1/1e6 deg (one millionth degree) --> auxIn 12 H & 13 L

        if(nmea.getAltitude(gpsAltitude)) gpsAltitude /= 1000; //Altitude in m, if not valid go to 0 --> auxIn14 
        else gpsAltitude=0;

        gpsSpeed = nmea.getSpeed()/185.2; //int32 speed in 1/1e3 knots (divided by 185.2 to get tenth kph) --> auxIn15 16bits


      }
    }
  #endif
}




