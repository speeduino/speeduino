#if defined(CORE_TEENSY) && defined(__IMXRT1062__)
#include "tpic8101.h"
#include "globals.h"
#include "maths.h"
#include <SPI.h>

/**
 * @brief Calculates the integrator time constant in uS for knock as a function of engine speed
 * 
 * @param ti This is the knock measuring window in uS that is calculated at every loop
 * @return byte SPI parameter to configure TPIC8101
 */
uint8_t tcCalculator(int ti)
{
    uint16_t timeC;
    uint8_t timeCBin;
    timeC = (ti * 10UL) / 283UL; //Simplified formula from Tc = Tin / 2 x Pi x Vout(4.5)
    if (timeC <= 43U) { timeCBin = 0b11000000;  }                    //knockTC = 40
    else if ((timeC > 43U)  && (timeC <= 48U) ) { timeCBin = 0b11000001; } //knockTC = 45
    else if ((timeC > 48U)  && (timeC <= 53U) ) { timeCBin = 0b11000010; } //knockTC = 50
    else if ((timeC > 53U)  && (timeC <= 58U) ) { timeCBin = 0b11000011; } //knockTC = 55
    else if ((timeC > 58U)  && (timeC <= 63U) ) { timeCBin = 0b11000100; } //knockTC = 60
    else if ((timeC > 63U)  && (timeC <= 68U) ) { timeCBin = 0b11000101; } //knockTC = 65
    else if ((timeC > 68U)  && (timeC <= 73U) ) { timeCBin = 0b11000110; } //knockTC = 70
    else if ((timeC > 73U)  && (timeC <= 78U) ) { timeCBin = 0b11000111; } //knockTC = 75
    else if ((timeC > 78U)  && (timeC <= 85U) ) { timeCBin = 0b11001000; } //knockTC = 80
    else if ((timeC > 85U)  && (timeC <= 95U) ) { timeCBin = 0b11001001; } //knockTC = 90
    else if ((timeC > 95U)  && (timeC <= 105U)) { timeCBin = 0b11001010; } //knockTC = 100
    else if ((timeC > 105U) && (timeC <= 115U)) { timeCBin = 0b11001011; } //knockTC = 110
    else if ((timeC > 115U) && (timeC <= 125U)) { timeCBin = 0b11001100; } //knockTC = 120
    else if ((timeC > 125U) && (timeC <= 135U)) { timeCBin = 0b11001101; } //knockTC = 130
    else if ((timeC > 135U) && (timeC <= 145U)) { timeCBin = 0b11001110; } //knockTC = 140
    else if ((timeC > 145U) && (timeC <= 155U)) { timeCBin = 0b11001111; } //knockTC = 150
    else if ((timeC > 155U) && (timeC <= 170U)) { timeCBin = 0b11010000; } //knockTC = 160
    else if ((timeC > 170U) && (timeC <= 190U)) { timeCBin = 0b11010001; } //knockTC = 180
    else if ((timeC > 190U) && (timeC <= 210U)) { timeCBin = 0b11010010; } //knockTC = 200
    else if ((timeC > 210U) && (timeC <= 230U)) { timeCBin = 0b11010011; } //knockTC = 220
    else if ((timeC > 230U) && (timeC <= 250U)) { timeCBin = 0b11010100; } //knockTC = 240
    else if ((timeC > 250U) && (timeC <= 270U)) { timeCBin = 0b11010101; } //knockTC = 260
    else if ((timeC > 270U) && (timeC <= 290U)) { timeCBin = 0b11010110; } //knockTC = 280
    else if ((timeC > 290U) && (timeC <= 310U)) { timeCBin = 0b11010111; } //knockTC = 300
    else if ((timeC > 310U) && (timeC <= 340U)) { timeCBin = 0b11011000; } //knockTC = 320
    else if ((timeC > 340U) && (timeC <= 380U)) { timeCBin = 0b11011001; } //knockTC = 360
    else if ((timeC > 380U) && (timeC <= 420U)) { timeCBin = 0b11011010; } //knockTC = 400
    else if ((timeC > 420U) && (timeC <= 460U)) { timeCBin = 0b11011011; } //knockTC = 440
    else if ((timeC > 460U) && (timeC <= 500U)) { timeCBin = 0b11011100; } //knockTC = 480
    else if ((timeC > 500U) && (timeC <= 540U)) { timeCBin = 0b11011101; } //knockTC = 520
    else if ((timeC > 540U) && (timeC <= 580U)) { timeCBin = 0b11011110; } //knockTC = 560
    else if (timeC > 580U) { timeCBin = 0b11011111; }                     //knockTC = 600
    return timeCBin;
}

//Programmable gain is not RPM dependant. It is strongly dependant of the sensor
//sensitivity in mVpp @ specific frequency and therefore is set and forget!!

/**
 * @brief Get the Frequency object
 * 
 * @param kF Frequency in kHz from the dropdown list in TS config15
 * @return byte Returns the data necessary to configure TPIC8101 Freq
 */
uint8_t getFrequency(byte kF)
{
    uint8_t tempFreq;
    switch (kF)
    {
        case 0: tempFreq = 0b00000000; break; //1.22kHz
        case 1: tempFreq = 0b00000001; break; //1.26kHz
        case 2: tempFreq = 0b00000010; break; //1.31kHz
        case 3: tempFreq = 0b00000011; break; //1.35kHz
        case 4: tempFreq = 0b00000100; break; //1.40kHz
        case 5: tempFreq = 0b00000101; break; //1.45kHz
        case 6: tempFreq = 0b00000110; break; //1.51kHz
        case 7: tempFreq = 0b00000111; break; //1.57kHz
        case 8: tempFreq = 0b00001000; break; //1.63kHz
        case 9: tempFreq = 0b00001001; break; //1.71kHz
        case 10: tempFreq = 0b00001010; break; //1.78kHz
        case 11: tempFreq = 0b00001011; break; //1.87kHz
        case 12: tempFreq = 0b00001100; break; //1.96kHz
        case 13: tempFreq = 0b00001101; break; //2.07kHz
        case 14: tempFreq = 0b00001110; break; //2.18kHz
        case 15: tempFreq = 0b00001111; break; //2.31kHz
        case 16: tempFreq = 0b00010000; break; //2.46kHz
        case 17: tempFreq = 0b00010001; break; //2.54kHz
        case 18: tempFreq = 0b00010010; break; //2.62kHz
        case 19: tempFreq = 0b00010011; break; //2.71kHz
        case 20: tempFreq = 0b00010100; break; //2.81kHz
        case 21: tempFreq = 0b00010101; break; //2.92kHz
        case 22: tempFreq = 0b00010110; break; //3.03kHz
        case 23: tempFreq = 0b00010111; break; //3.15kHz
        case 24: tempFreq = 0b00011000; break; //3.28kHz
        case 25: tempFreq = 0b00011001; break; //3.43kHz
        case 26: tempFreq = 0b00011010; break; //3.59kHz
        case 27: tempFreq = 0b00011011; break; //3.76kHz
        case 28: tempFreq = 0b00011100; break; //3.95kHz
        case 29: tempFreq = 0b00011101; break; //4.16kHz
        case 30: tempFreq = 0b00011110; break; //4.39kHz
        case 31: tempFreq = 0b00011111; break; //4.66kHz
        case 32: tempFreq = 0b00100000; break; //4.95kHz
        case 33: tempFreq = 0b00100001; break; //5.12kHz
        case 34: tempFreq = 0b00100010; break; //5.29kHz
        case 35: tempFreq = 0b00100011; break; //5.48kHz
        case 36: tempFreq = 0b00100100; break; //5.68kHz
        case 37: tempFreq = 0b00100101; break; //5.90kHz
        case 38: tempFreq = 0b00100110; break; //6.12kHz
        case 39: tempFreq = 0b00100111; break; //6.37kHz
        case 40: tempFreq = 0b00101000; break; //6.64kHz
        case 41: tempFreq = 0b00101001; break; //6.94kHz
        case 42: tempFreq = 0b00101010; break; //7.27kHz
        case 43: tempFreq = 0b00101011; break; //7.63kHz
        case 44: tempFreq = 0b00101100; break; //8.02kHz
        case 45: tempFreq = 0b00101101; break; //8.46kHz
        case 46: tempFreq = 0b00101110; break; //8.95kHz
        case 47: tempFreq = 0b00101111; break; //9.50kHz
        case 48: tempFreq = 0b00110000; break; //10.12kHz
        case 49: tempFreq = 0b00110001; break; //10.46kHz
        case 50: tempFreq = 0b00110010; break; //10.83kHz
        case 51: tempFreq = 0b00110011; break; //11.22kHz
        case 52: tempFreq = 0b00110100; break; //11.65kHz
        case 53: tempFreq = 0b00110101; break; //12.10kHz
        case 54: tempFreq = 0b00110110; break; //12.60kHz
        case 55: tempFreq = 0b00110111; break; //13.14kHz
        case 56: tempFreq = 0b00111000; break; //13.72kHz
        case 57: tempFreq = 0b00111001; break; //14.36kHz
        case 58: tempFreq = 0b00111010; break; //15.07kHz
        case 59: tempFreq = 0b00111011; break; //15.84kHz
        case 60: tempFreq = 0b00111100; break; //16.71kHz
        case 61: tempFreq = 0b00111101; break; //17.67kHz
        case 62: tempFreq = 0b00111110; break; //18.76kHz
        case 63: tempFreq = 0b00111111; break; //19.98kHz
        default: break;
    }
    return tempFreq;
}

/**
 * @brief Get the Prog Gain object
 * 
 * @param pG Programmable gain from the dropdown list in TS config15
 * @return byte Returns the data necessary to configure TPIC8101 ProgGain
 */
uint8_t getProgGain(byte pG)
{
    uint8_t tempGain;
    switch (pG)
    {
        case 0: tempGain = 0b10000000; break; //2.000
        case 1: tempGain = 0b10000001; break; //1.882
        case 2: tempGain = 0b10000010; break; //1.778
        case 3: tempGain = 0b10000011; break; //1.684
        case 4: tempGain = 0b10000100; break; //1.600
        case 5: tempGain = 0b10000101; break; //1.523
        case 6: tempGain = 0b10000110; break; //1.455
        case 7: tempGain = 0b10000111; break; //1.391
        case 8: tempGain = 0b10001000; break; //1.333
        case 9: tempGain = 0b10001001; break; //1.280
        case 10: tempGain = 0b10001010; break; //1.231
        case 11: tempGain = 0b10001011; break; //1.185
        case 12: tempGain = 0b10001100; break; //1.143
        case 13: tempGain = 0b10001101; break; //1.063
        case 14: tempGain = 0b10001110; break; //1.000
        case 15: tempGain = 0b10001111; break; //0.944
        case 16: tempGain = 0b10010000; break; //0.895
        case 17: tempGain = 0b10010001; break; //0.850
        case 18: tempGain = 0b10010010; break; //0.810
        case 19: tempGain = 0b10010011; break; //0.773
        case 20: tempGain = 0b10010100; break; //0.739
        case 21: tempGain = 0b10010101; break; //0.708
        case 22: tempGain = 0b10010110; break; //0.680
        case 23: tempGain = 0b10010111; break; //0.654
        case 24: tempGain = 0b10011000; break; //0.630
        case 25: tempGain = 0b10011001; break; //0.607
        case 26: tempGain = 0b10011010; break; //0.586
        case 27: tempGain = 0b10011011; break; //0.567
        case 28: tempGain = 0b10011100; break; //0.548
        case 29: tempGain = 0b10011101; break; //0.500
        case 30: tempGain = 0b10011110; break; //0.471
        case 31: tempGain = 0b10011111; break; //0.444
        case 32: tempGain = 0b10100000; break; //0.421
        case 33: tempGain = 0b10100001; break; //0.400
        case 34: tempGain = 0b10100010; break; //0.381
        case 35: tempGain = 0b10100011; break; //0.364
        case 36: tempGain = 0b10100100; break; //0.348
        case 37: tempGain = 0b10100101; break; //0.333
        case 38: tempGain = 0b10100110; break; //0.320
        case 39: tempGain = 0b10100111; break; //0.308
        case 40: tempGain = 0b10101000; break; //0.296
        case 41: tempGain = 0b10101001; break; //0.286
        case 42: tempGain = 0b10101010; break; //0.276
        case 43: tempGain = 0b10101011; break; //0.267
        case 44: tempGain = 0b10101100; break; //0.258
        case 45: tempGain = 0b10101101; break; //0.250
        case 46: tempGain = 0b10101110; break; //0.236
        case 47: tempGain = 0b10101111; break; //0.222
        case 48: tempGain = 0b10110000; break; //0.211
        case 49: tempGain = 0b10110001; break; //0.200
        case 50: tempGain = 0b10110010; break; //0.190
        case 51: tempGain = 0b10110011; break; //0.182
        case 52: tempGain = 0b10110100; break; //0.174
        case 53: tempGain = 0b10110101; break; //0.167
        case 54: tempGain = 0b10110110; break; //0.160
        case 55: tempGain = 0b10110111; break; //0.154
        case 56: tempGain = 0b10111000; break; //0.148
        case 57: tempGain = 0b10111001; break; //0.143
        case 58: tempGain = 0b10111010; break; //0.138
        case 59: tempGain = 0b10111011; break; //0.133
        case 60: tempGain = 0b10111100; break; //0.129
        case 61: tempGain = 0b10111101; break; //0.125
        case 62: tempGain = 0b10111110; break; //0.118
        case 63: tempGain = 0b10111111; break; //0.111
        default: break;
    }
    return tempGain;
}

void initTPIC8101(void)
{
    pinMode(pinTPIC8101_CS, OUTPUT);
    pinMode(pinTPIC8101_INT, OUTPUT);
    TPIC_INACTIVE();
    TPIC_STOP_WINDOW();
    SPI.begin();
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1)); //SPI mode 0 does not work well with TPIC8101
    msgtpic = sendTPICCommand(SPU_SET_PRESCALAR_6MHz);
    uint8_t tmpFreq = getFrequency(configPage15.knockFreqA);
    msgtpic = sendTPICCommand(tmpFreq); currentStatus.knockFreqBinA = tmpFreq; BIT_SET(currentStatus.KNOCK, BIT_KNOCK_HZA); BIT_SET(currentStatus.KNOCK, BIT_KNOCK_HZB); BIT_SET(currentStatus.KNOCK, BIT_KNOCK_HZC); BIT_SET(currentStatus.KNOCK, BIT_KNOCK_HZD);
    uint8_t tmpGain = getProgGain(configPage15.knockGain);
    msgtpic = sendTPICCommand(tmpGain); currentStatus.knockGainBin = tmpGain; BIT_SET(currentStatus.KNOCK, BIT_KNOCK_AP);
    msgtpic = sendTPICCommand(SPU_SET_INTEGRATOR_TIME); currentStatus.knockTC = SPU_SET_INTEGRATOR_TIME; (currentStatus.KNOCK, BIT_KNOCK_TC);
    msgtpic = sendTPICCommand(SPU_SET_ADVANCED);
}

uint8_t readTPICValue(void)
{
    uint16_t tpic8101_int_last_bits = sendTPICCommand(SPU_SET_PRESCALAR_6MHz);
    uint16_t tpic8101_int_first_bits = sendTPICCommand(SPU_SET_CHANNEL_2);
    tpic8101_int_last_bits<<=2;
    tpic8101_int_first_bits |= tpic8101_int_last_bits;
    uint8_t tempReading = fastMap10Bit(tpic8101_int_first_bits, 0, 254);
    return tempReading;
}

uint8_t sendTPICCommand(uint8_t data)
{
    TPIC_ACTIVE();
    uint8_t response = SPI.transfer(data); //Transmit the actual data
    TPIC_INACTIVE();
    return response;
}
#endif //CORE_TEENSY