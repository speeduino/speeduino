#ifndef GPS_H
#define GPS_H

  #define NMEA_PACKET_SIZE   75

  #if ( defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) )
  /*
    #define GPSSerial_AVAILABLE

    extern bool gpsOnAUX;
    extern HardwareSerial &gps;
    extern char nmeaBuffer[100];

    extern int32_t gpsLat;
    extern int32_t gpsLong;
    extern int32_t gpsAltitude;
    extern int32_t gpsSpeed;
  */
  #elif defined(CORE_STM32)
      //TBD
  #elif defined(CORE_TEENSY)
    #define GPSSerial_AVAILABLE


    extern HardwareSerial &gps;
    extern char nmeaBuffer[100];

    extern int32_t gpsLat;
    extern int32_t gpsLong;
    extern int32_t gpsAltitude;
    extern int32_t gpsSpeed;

  #endif

  extern bool gpsOnAUX;

#endif

/**
 * @brief The serial receive pump. Should be called whenever the serial port
 * has data available to read.
 */
void gpsSerialReceive(void);