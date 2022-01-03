#ifdef RTC_ENABLED
#include "rtc_common.h"
#include "globals.h"
#include RTC_LIB_H //Defined in each boards .h file

void initRTC()
{


}

uint8_t rtc_getSecond()
{
  uint8_t tempSecond = 0;
#ifdef RTC_ENABLED
  #if defined(CORE_TEENSY)
    tempSecond = second();
  #elif defined(CORE_STM32)
    tempSecond = rtc.getSeconds();
  #endif
#endif
  return tempSecond;
}

uint8_t rtc_getMinute()
{
  uint8_t tempMinute = 0;
#ifdef RTC_ENABLED
  #if defined(CORE_TEENSY)
    tempMinute = minute();
  #elif defined(CORE_STM32)
    tempMinute = rtc.getMinutes();
  #endif
#endif
  return tempMinute;
}

uint8_t rtc_getHour()
{
  uint8_t tempHour = 0;
#ifdef RTC_ENABLED
  #if defined(CORE_TEENSY)
    tempHour = hour();
  #elif defined(CORE_STM32)
    tempHour = rtc.getHours();
  #endif
#endif
  return tempHour;
}

uint8_t rtc_getDay()
{
  uint8_t tempDay = 0;
#ifdef RTC_ENABLED
  #if defined(CORE_TEENSY)
    tempDay = day();
  #elif defined(CORE_STM32)
    tempDay = rtc.getDay();
  #endif
#endif
  return tempDay;
}

uint8_t rtc_getDOW()
{
  uint8_t dow = 0;
#ifdef RTC_ENABLED
  #if defined(CORE_TEENSY)
    dow = weekday();
  #elif defined(CORE_STM32)
    dow = rtc.getWeekDay();
  #endif
#endif
  return dow;
}

uint8_t rtc_getMonth()
{
  uint8_t tempMonth = 0;
#ifdef RTC_ENABLED
  #if defined(CORE_TEENSY)
    tempMonth = month();
  #elif defined(CORE_STM32)
    tempMonth = rtc.getMonth();
  #endif
#endif
  return tempMonth;
}

uint16_t rtc_getYear()
{
  uint16_t tempYear = 0;
#ifdef RTC_ENABLED
  #if defined(CORE_TEENSY)
    tempYear = year();
  #elif defined(CORE_STM32)
    tempYear = rtc.getYear();
  #endif
#endif
  return tempYear;
}

void rtc_setTime(byte second, byte minute, byte hour, byte day, byte month, uint16_t year)
{
#ifdef RTC_ENABLED
  #if defined(CORE_TEENSY)
    setTime(second, minute, hour, day, month, year);
  #elif defined(CORE_STM32)
    rtc.setTime(hour, minute, second);
    rtc.setDate(day, month, year);
  #endif
#endif
}
#endif