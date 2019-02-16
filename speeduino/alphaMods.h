#ifndef __ALPHA_MODS_H__
#define __ALPHA_MODS_H__

#include "globals.h"

struct alphaMods{
//***
  byte carSelect = 0; //0 - generic car; 1 - Corolla XRS; 2 - Hyundai Tiburon; 
  //3 - Subaru WRX; 4 - Audi A4 1.8T; 5 - 1988 Mazda RX7; 6- 98 Mustang V6; 7 - DCwerx; 255 - mods disabled
  
  
  
  /*bool DFCOwait; // waits to enable DFCO
  bool vvlOn = false;
  bool CELon = false;
  bool gaugeSweep = true;
  bool rollingALsoft = false;
  bool rollingALhard = false;
  bool rollingALtrigger = false;
  bool ACOn; //whether AC is on
  bool AcReq; // AC request
  bool highIdleReq;*/ //raises idle in open loop to evade stalling
  uint8_t highIdleCount = 0;// counts to wait for normal idle
  uint8_t DFCOcounter = 0;// counts cycles till dfco
  uint8_t alphaBools1 = B00000000;
  uint8_t alphaBools2 = B00000000;
  uint8_t vvlCorrection;
  uint8_t alphaNcorrection;
  uint8_t gCamTime = 0;
  
  //**
};

//defines for alphaBools1
#define BIT_AC_ON         0 //Position Report enabled
#define BIT_AC_REQ        1 
#define BIT_HIGH_IDLE     2
#define BIT_DFCO_WAIT     3
#define BIT_CEL_STATE     4
#define BIT_GAUGE_SWEEP   5
#define BIT_VVL_ON        6

//defines for alphaBools2
#define BIT_RLING_SOFT    0
#define BIT_RLING_HARD    1 
#define BIT_RLING_TRIG    2
#define BIT_GCAM_STATE    3
#define BIT_CRK_ALLOW     4
#define BIT_SKIP_TOOTH    5

struct alphaMods alphaVars;
extern struct alphaMods alphaVars; // from speeduino.ino

void ACControl();
void CELcontrol();
void vvlControl();
void fanControl2();
void highIdleFunc();
void DFCOwaitFunc();
void XRSgaugeCLT();
void alphaIdleMods();
void RPMdance();
void initialiseAlphaPins();
void alpha4hz();
void alphaCorrections();
static inline uint8_t correctionVVL();
static inline uint8_t correctionAlphaN();
uint16_t WOTdwellCorrection(uint16_t);
uint16_t boostAssist(uint16_t);
static inline int8_t correctionRollingAntiLag(int8_t);

static inline int8_t correctionZeroThrottleTiming(int8_t advance);
static inline bool correctionDFCO2();
static inline int8_t correctionTimingAlphaN(int8_t advance);
static inline int8_t correctionAtUpshift(int8_t advance);
void ghostCam();

void alphaTableSetup();

// Custom pins
 byte pinAC; // pin for AC clutch
 byte pinAcReq;
 byte pinFan2;
 byte pinCEL;
 byte pinVVL;
 byte pinACpress;
 byte pinACtemp;
 byte pinOilPress;
 byte pinCLTgauge;
 byte pinRollingAL;

#endif // __ALPHA_MODS_H__
