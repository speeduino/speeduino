#ifndef TPIC8101_H
#define TPIC8101_H
#if defined(CORE_TEENSY)&& defined(__IMXRT1062__)

#include <SPI.h>

volatile PORT_TYPE *tpic8101_cs_pin_port;
volatile PINMASK_TYPE tpic8101_cs_pin_mask;
volatile PORT_TYPE *tpic8101_int_hold_pin_port;
volatile PINMASK_TYPE tpic8101_int_hold_pin_mask;

#define SPU_SET_PRESCALAR_6MHz        0b01000100
#define SPU_SET_CHANNEL_2             0b11100001    /* Setting active channel to 2 */
#define SPU_SET_INTEGRATOR_TIME       0b11011111    /* Setting initial programmable integrator time constant to 600us */
#define SPU_SET_ADVANCED              0b01110001    /* Setting to be able to read integrator value from SPI */

#define BIT_KNOCK_TC        0
#define BIT_KNOCK_HZA       1
#define BIT_KNOCK_AP        2
#define BIT_KNOCK_HZB       3
#define BIT_KNOCK_HZC       4
#define BIT_KNOCK_HZD       5
#define BIT_KNOCK_UNUSED7   6
#define BIT_KNOCK_UNUSED8   7

#define BIT_KNOCK2_VALA     0
#define BIT_KNOCK2_VALB     1
#define BIT_KNOCK2_VALC     2
#define BIT_KNOCK2_VALD     3
#define BIT_KNOCK2_UNUSED1  4
#define BIT_KNOCK2_UNUSED2  5
#define BIT_KNOCK2_UNUSED3  6
#define BIT_KNOCK2_UNUSED4  7

void initTPIC8101(void);


#define TPIC_ACTIVE()          (digitalWrite(pinTPIC8101_CS, LOW))
#define TPIC_INACTIVE()        (digitalWrite(pinTPIC8101_CS, HIGH))
#define TPIC_START_WINDOW()    (digitalWrite(pinTPIC8101_INT, HIGH))
#define TPIC_STOP_WINDOW()     (digitalWrite(pinTPIC8101_INT, LOW))

/*
#define TPIC_ACTIVE()    noInterrupts(); *tpic8101_cs_pin_port &= ~(tpic8101_cs_pin_mask); interrupts()    //LOW
#define TPIC_INACTIVE()   noInterrupts(); *tpic8101_cs_pin_port |= (tpic8101_cs_pin_mask); interrupts()    //HIGH
#define TPIC_START_WINDOW()   noInterrupts(); *tpic8101_int_hold_pin_port |= (tpic8101_int_hold_pin_mask); interrupts()    //HIGH
#define TPIC_STOP_WINDOW()    noInterrupts(); *tpic8101_int_hold_pin_port &= ~(tpic8101_int_hold_pin_mask); interrupts()    //LOW
*/

uint8_t readTPICValue(void);
uint8_t sendTPICCommand(uint8_t);
uint8_t tcCalculator(int);
uint8_t getFrequency(byte);
uint8_t getProgGain(byte);
uint8_t msgtpic;

#define knock1IntHigh_TPIC8101() if(!BIT_CHECK(currentStatus.KNOCK, BIT_KNOCK_HZA)){msgtpic = sendTPICCommand(currentStatus.knockFreqBinA); BIT_SET(currentStatus.KNOCK, BIT_KNOCK_HZA); BIT_SET(currentStatus.KNOCK2, BIT_KNOCK2_VALA);} \
                                 if(!BIT_CHECK(currentStatus.KNOCK, BIT_KNOCK_HZB)){msgtpic = sendTPICCommand(currentStatus.knockFreqBinB); BIT_SET(currentStatus.KNOCK, BIT_KNOCK_HZB); BIT_SET(currentStatus.KNOCK2, BIT_KNOCK2_VALB);} \
                                 if(!BIT_CHECK(currentStatus.KNOCK, BIT_KNOCK_HZC)){msgtpic = sendTPICCommand(currentStatus.knockFreqBinC); BIT_SET(currentStatus.KNOCK, BIT_KNOCK_HZC); BIT_SET(currentStatus.KNOCK2, BIT_KNOCK2_VALC);} \
                                 if(!BIT_CHECK(currentStatus.KNOCK, BIT_KNOCK_HZD)){msgtpic = sendTPICCommand(currentStatus.knockFreqBinD); BIT_SET(currentStatus.KNOCK, BIT_KNOCK_HZD); BIT_SET(currentStatus.KNOCK2, BIT_KNOCK2_VALD);} \
                                 if(!BIT_CHECK(currentStatus.KNOCK, BIT_KNOCK_AP)){msgtpic = sendTPICCommand(currentStatus.knockGainBin); BIT_SET(currentStatus.KNOCK, BIT_KNOCK_AP);} \
                                 if(!BIT_CHECK(currentStatus.KNOCK, BIT_KNOCK_TC)){msgtpic = sendTPICCommand(currentStatus.knockTC);BIT_SET(currentStatus.KNOCK, BIT_KNOCK_TC);} \
                                 msgtpic = sendTPICCommand(SPU_SET_CHANNEL_2); TPIC_START_WINDOW()

#define knock2IntHigh_TPIC8101() msgtpic = sendTPICCommand(SPU_SET_CHANNEL_2); TPIC_START_WINDOW()
#define knock3IntHigh_TPIC8101() msgtpic = sendTPICCommand(SPU_SET_CHANNEL_2); TPIC_START_WINDOW()
#define knock4IntHigh_TPIC8101() msgtpic = sendTPICCommand(SPU_SET_CHANNEL_2); TPIC_START_WINDOW()

#define knock1IntLow_TPIC8101() TPIC_STOP_WINDOW(); currentStatus.knock1 = readTPICValue(); \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALA)){currentStatus.knock1A = currentStatus.knock1;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALB)){currentStatus.knock1B = currentStatus.knock1;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALC)){currentStatus.knock1C = currentStatus.knock1;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALD)){currentStatus.knock1D = currentStatus.knock1;}

#define knock2IntLow_TPIC8101() TPIC_STOP_WINDOW(); currentStatus.knock2 = readTPICValue(); \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALA)){currentStatus.knock2A = currentStatus.knock2;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALB)){currentStatus.knock2B = currentStatus.knock2;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALC)){currentStatus.knock2C = currentStatus.knock2;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALD)){currentStatus.knock2D = currentStatus.knock2;}

#define knock3IntLow_TPIC8101() TPIC_STOP_WINDOW(); currentStatus.knock3 = readTPICValue(); \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALA)){currentStatus.knock3A = currentStatus.knock3;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALB)){currentStatus.knock3B = currentStatus.knock3;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALC)){currentStatus.knock3C = currentStatus.knock3;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALD)){currentStatus.knock3D = currentStatus.knock3;}

#define knock4IntLow_TPIC8101() TPIC_STOP_WINDOW(); currentStatus.knock4 = readTPICValue(); \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALA)){currentStatus.knock4A = currentStatus.knock4;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALB)){currentStatus.knock4B = currentStatus.knock4;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALC)){currentStatus.knock4C = currentStatus.knock4;} \
                                                    if(BIT_CHECK(currentStatus.KNOCK2, BIT_KNOCK2_VALD)){currentStatus.knock4D = currentStatus.knock4;}

#endif //CORE_TEENSY
#endif //TPIC8101_H