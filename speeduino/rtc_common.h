#ifndef RTC_H
#define RTC_H

void initRTC();
uint8_t rtc_getSecond();
uint8_t rtc_getMinute();
uint8_t rtc_getHour();
uint8_t rtc_getDay();
uint8_t rtc_getDOW();
uint8_t rtc_getMonth();
uint16_t rtc_getYear();
void rtc_setTime(byte, byte, byte, byte, byte, uint16_t);



#endif