#ifndef MICRONMEA_H
#define MICRONMEA_H

#define MICRONMEA_VERSION "2.0.6"
#include <limits.h>
#include <Arduino.h>

/**
 * @file MicroNMEA.h
 * @author Steve Marple
 */



/**
 * @class MicroNMEA
 * @brief Process MicroNMEA sentences from GPS and GNSS receivers.
 * @details The user is responsible to allocating the buffer that MicroNMEA uses. This
 * enables a static buffer to be used if desired so that `malloc()` is not required.
 * Values returned are integers, floating-point maths is not used.
 */
class MicroNMEA {
  public:

    static const char* skipField(const char* s);
    static uint16_t parseUnsignedInt(const char* s, uint8_t len);
    static int32_t parseFloat(const char* s, uint8_t log10Multiplier,
                           const char** eptr = nullptr, bool *resultValid = nullptr);
    static int32_t parseDegreeMinute(const char* s, uint8_t degWidth,
                                  const char** eptr = nullptr);
    static const char* parseToComma(const char* s, char *result = nullptr,
                                    int16_t len = 0);
    static const char* parseField(const char* s, char *result = nullptr,
                                  int16_t len = 0);
    static const char* generateChecksum(const char* s, char* checksum);
    static bool testChecksum(const char* s);

    /**
     * @brief Send a NMEA sentence to the GNSS receiver.
     *
     * @param s Stream to which the GNSS receiver is connected
     * @param sentence The NMEA sentence to send
     * @details The sentence must start with `$`; the checksum
     * and `\r\n` terminators will be appended automatically.
     * @return The GNSS stream
     */
    static Stream& sendSentence(Stream &s, const char* sentence);

    /**
     * @brief Default constructor
     * @details User **must** call setrBuffer() before use
     */
    MicroNMEA(void);


    /**
     * @brief Construct object and pass in the buffer allocated for MicroNMEA to use
     */
    MicroNMEA(void* buffer, uint8_t len);

    /**
     * @brief Set the buffer object
     *
     * @param buf Address of the buffer
     * @param len Number of bytes allocated
     */
    void setBuffer(void* buf, uint8_t len);

    // Clear all fix information. isValid() will return false, Year,
    // month and day will all be zero. Hour, minute and second time will
    // be set to 99. Speed, course and altitude will be set to
    // LONG_MIN; the altitude validity flag will be false. Latitude and
    // longitude will be set to 999 degrees.
    /**
     * @brief Clear all fix information
     * @details `isValid()` will return false, year,
     * month and day will all be zero. Hour, minute and second will
     * be set to 99. Speed, course and altitude will be set to
     * `LONG_MIN`; the altitude validity flag will be false. Latitude and
     * longitude will be set to 999 degrees.
     */
    void clear(void);

    /**
     * @brief Get the navigation system in use
     * @details `N` = GNSS, `P` = GPS, `L` = GLONASS, `A` = Galileo, `\0` = none
     * @return char
     */
    char getNavSystem(void) const {
      return _navSystem;
    }

    /**
     * @brief Get the number of satellites in use
     *
     * @return uint8_t
     */
    uint8_t getNumSatellites(void) const {
      return _numSat;
    }

    /**
     * @brief Get the horizontal dilution of precision (HDOP), in tenths
     * @details A HDOP value of 1.1 is returned as `11`
     * @return uint8_t
     */
    uint8_t getHDOP(void) const {
      return _hdop;
    }

    /**
     * @brief Inquire if latest fix is valid
     *
     * @return true Valid
     * @return false Not valid
     */
    bool isValid(void) const {
      return _isValid;
    }

    /**
     * @brief Get the latitude, in millionths of a degree
     * @details North is positive.
     * @return long
     */
    int32_t getLatitude(void) const {
      return _latitude;
    }

    /**
     * @brief Get the longitude, in millionths of a degree
     * @details East is positive.
     * @return long
     */
    int32_t getLongitude(void) const {
      return _longitude;
    }

    // Altitude in millimetres.
    /**
     * @brief Get the altitude in millmetres
     *
     * @param alt Reference to int32_t value where altitude is to be stored
     * @return true Altitude is valid
     * @return false Altitude not valid
     */
    bool getAltitude(int32_t &alt) const {
      if (_altitudeValid)
        alt = _altitude;
      return _altitudeValid;
    }

    /**
	 * @brief Get the height above WGS84 Geoid in millimetres.
	 *
	 * @return uint16_t year
	 * @param alt Reference to int32_t value where height is to be stored
	 * @return true Altitude is valid
	 * @return false Altitude not valid
	 */
	bool getGeoidHeight(int32_t &alt) const {
		if (_geoidHeightValid)
			alt = _geoidHeight;
		return _geoidHeightValid;
	}

	/**
     * @brief Get the year
     *
     * @return uint16_t year
     */
    uint16_t getYear(void) const {
      return _year;
    }

    /**
     * @brief Get the month (1 - 12 inclusive)
     *
     * @return uint8_t year
     */
    uint8_t getMonth(void) const {
      return _month;
    }

    /**
     * @brief Get the day of month (1 - 31 inclusive)
     *
     * @return uint8_t month
     */
    uint8_t getDay(void) const {
      return _day;
    }

    /**
     * @brief Get the hour
     *
     * @return uint8_t hour
     */
    uint8_t getHour(void) const {
      return _hour;
    }

    /**
     * @brief Get the minute
     *
     * @return uint8_t minute
     */
    uint8_t getMinute(void) const {
      return _minute;
    }

    /**
     * @brief Get the integer part of the second
     *
     * @return uint8_t second
     */
    uint8_t getSecond(void) const {
      return _second;
    }

    /**
     * @brief Get the hundredths part of the second
     *
     * @return uint8_t hundredths
     */
    uint8_t getHundredths(void) const {
      return _hundredths;
    }

    /**
     * @brief Get the speed
     *
     * @return uint8_t speed
     */
    int32_t getSpeed(void) const {
      return _speed;
    }

    /**
     * @brief Get the direction of travel
     * @return Direction in thousandths of a degree, clockwise from North
     */
    int32_t getCourse(void) const {
      return _course;
    }

    /**
     * @brief Instruct MicroNMEA to process a character
     *
     * @param c Character to process
     * @return true A complete non-empty sentence has been processed (may not be valid)
     * @return false End of sentence not detected
     */
    bool process(char c);

    /**
     * @brief Register a handler to be called when bad checksums are detected
     *
     * @param handler pointer to handler function
     */
    void setBadChecksumHandler(void (*handler)(MicroNMEA& nmea)) {
      _badChecksumHandler = handler;
    }

    /**
     * @brief Register a handler to be called when an unknown NMEA sentence is detected
     *
     * @param handler pointer to handler function
     */
    void setUnknownSentenceHandler(void (*handler)(MicroNMEA& nmea)) {
      _unknownSentenceHandler = handler;
    }

    /**
     * @brief Get NMEA sentence
     *
     * @return const char*
     */
    const char* getSentence(void) const {
      return _buffer;
    }

    // Talker ID for current MicroNMEA sentence
    char getTalkerID(void) const {
      return _talkerID;
    }

    // Message ID for current MicroNMEA sentence
    const char* getMessageID(void) const {
      return (const char*)_messageID;
    }


  protected:
    static inline bool isEndOfFields(char c) {
      return c == '*' || c == '\0' || c == '\r' || c == '\n';
    }

    const char* parseTime(const char* s);
    const char* parseDate(const char* s);
 
    bool processGGA(const char *s);
    bool processRMC(const char* s);

  private:
    // Sentence buffer and associated pointers
    // static const uint8_t _bufferLen = 83; // 82 + NULL
    // char _buffer[_bufferLen];
    uint8_t _bufferLen;
    char* _buffer;
    char *_ptr;

    // Information from current MicroNMEA sentence
    char _talkerID;
    char _messageID[6];

    // Variables parsed and kept for user
    char _navSystem;
    bool _isValid;
    int32_t _latitude, _longitude; // In millionths of a degree
    int32_t _altitude; // In millimetres
    bool _altitudeValid;
	int32_t _geoidHeight; // In millimetres
	bool _geoidHeightValid;
    int32_t _speed, _course;
    uint16_t _year;
    uint8_t _month, _day, _hour, _minute, _second, _hundredths;
    uint8_t _numSat;
    uint8_t _hdop;

    void (*_badChecksumHandler)(MicroNMEA &nmea);
    void (*_unknownSentenceHandler)(MicroNMEA &nmea);

};


#endif
