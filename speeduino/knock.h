#ifndef KNOCK_H
#define KNOCK_H

#include "globals.h"

static inline void launchKnockWindow();
static inline void getKnockValue();
void initialiseKnock();
static inline void determineRetard();
static inline uint8_t sendCmd(uint8_t);
void refreshKnockParameters(void);
volatile int knock_threshold = 0;
volatile bool knockRecoveryFirstStepDelay = false;
volatile uint32_t knockWindowStartDelay = 0;  // uSec
volatile uint32_t knockWindowDuration = 0;    // uSec
#define SEVERE_CNT 2

// SPI commands for TPIC8101
#define PS_SDO_STAT_CMD  0b01000110 // 8MHz in, SDO active
#define CHAN1_SEL_CMD    0b11100000 // channel 1
#define CHAN2_SEL_CMD    0b11100001 // channel 2
#define BPCF_CMD         0b00000000 // band pass center frequency SPI cmd
#define INT_GAIN_CMD     0b10000000 // Integrator gain
#define INT_TC_CMD       0b11000000 // Integrator Time constant
#define ADV_MODE_CMD     0b01110001 // switch to advanced mode
#define REQUEST_LOW_BYTE     0b01000110 // (when in advanced mode)
#define REQUEST_HIGH_BYTE    0b11100000 // (when in advanced mode)
SPISettings knockSettings;

// TPIC8101 parameter values
int timeConst[] = {\
40,45,50,55,60,65,70,75,80,90,100,110,120,130,140,150,\
160,180,200,220,240,260,280,300,320,360,400,440,480,520,560,600\
};
// the gainK array is inverted from that required by TPIC8101 to allow timeConst, gainK and bpFreq
// to be searched with same routine. The values are scaled up from the TPIC8101 table by 1000; 111 is gain of 0.111.
int gainK[] = {\
111,118,125,129,133,138,143,148,154,160,167,174,182,190,200,211,\
222,236,250,258,267,276,286,296,308,320,333,348,364,381,400,421,\
444,471,500,548,567,586,607,630,654,680,708,739,773,810,850,895,\
944,1000,1063,1143,1185,1231,1280,1333,1391,1455,1523,1600,1684,1778,1882,2000\
};
// The values in bpFreq are scaled down by 10, 122 is freq 1220Hz
int bpFreq[] = {\
122,126,131,135,140,145,151,157,163,171,178,187,196,207,218,231,\
246,254,262,271,281,292,303,315,328,343,359,376,395,416,439,466,\
495,512,529,548,568,590,612,637,664,694,727,763,802,846,895,950,\
1012,1046,1083,1122,1165,1210,1260,1314,1372,1436,1507,1584,1671,1767,1876,1998\
};
uint8_t getClosestIndex(int, int [], uint8_t);
uint8_t closestIndex(int, int, int [], int);

int16_t knockWindowSize = 0;  //The current crank angle delta that defines the knock window duration
int16_t knockWindowDelay = 0; //The current crank angle after end of ign dwell for a knock pulse to be valid

int16_t knockWindowGainFactor = 0;  // to control the sensor sensitivity as rpm (noise) increases
uint8_t band_pass_frequency_idx = 0;  // index from bpFreq array, used by TPIC8101
volatile uint8_t integrator_time_constant_idx = 0; // index from timeConst array, used by TPIC8101
volatile uint8_t rpmModGain_idx = 0;  // index from gainK array, used by TPIC8101
int integratorGain = 0;

#define OPEN_KNOCK_WINDOW() *knock_win_pin_port |= (knock_win_pin_mask) 
#define CLOSE_KNOCK_WINDOW() *knock_win_pin_port &= ~(knock_win_pin_mask)
#define CS0_ASSERT() digitalWrite(CS0, LOW)
#define CS0_RELEASE() digitalWrite(CS0, HIGH)

#endif
