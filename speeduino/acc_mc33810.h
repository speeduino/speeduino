#ifndef MC33810_H
#define MC33810_H

#include <SPI.h>
#include "board_definition.h"

static const uint8_t MC33810_ONOFF_CMD = 0x30; //48 in decimal
static volatile uint8_t mc33810_1_requestedState; //Current binary state of the 1st ICs IGN and INJ values
static volatile uint8_t mc33810_2_requestedState; //Current binary state of the 2nd ICs IGN and INJ values
static volatile uint8_t mc33810_1_returnState; //Current binary state of the 1st ICs IGN and INJ values
static volatile uint8_t mc33810_2_returnState; //Current binary state of the 2nd ICs IGN and INJ values

void initMC33810(void);

void setMC33810_1_ACTIVE(void);
void setMC33810_1_INACTIVE(void);
void setMC33810_2_ACTIVE(void);
void setMC33810_2_INACTIVE(void);

//These are default values for which injector is attached to which output on the IC. 
//They may (Probably will) be changed during init by the board specific config in init.ino
extern uint8_t MC33810_BIT_INJ1;
extern uint8_t MC33810_BIT_INJ2;
extern uint8_t MC33810_BIT_INJ3;
extern uint8_t MC33810_BIT_INJ4;
extern uint8_t MC33810_BIT_INJ5;
extern uint8_t MC33810_BIT_INJ6;
extern uint8_t MC33810_BIT_INJ7;
extern uint8_t MC33810_BIT_INJ8;

extern uint8_t MC33810_BIT_IGN1;
extern uint8_t MC33810_BIT_IGN2;
extern uint8_t MC33810_BIT_IGN3;
extern uint8_t MC33810_BIT_IGN4;
extern uint8_t MC33810_BIT_IGN5;
extern uint8_t MC33810_BIT_IGN6;
extern uint8_t MC33810_BIT_IGN7;
extern uint8_t MC33810_BIT_IGN8;

#define openInjector1_MC33810() setMC33810_1_ACTIVE(); BIT_SET(mc33810_1_requestedState, MC33810_BIT_INJ1); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define openInjector2_MC33810() setMC33810_1_ACTIVE(); BIT_SET(mc33810_1_requestedState, MC33810_BIT_INJ2); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define openInjector3_MC33810() setMC33810_1_ACTIVE(); BIT_SET(mc33810_1_requestedState, MC33810_BIT_INJ3); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define openInjector4_MC33810() setMC33810_1_ACTIVE(); BIT_SET(mc33810_1_requestedState, MC33810_BIT_INJ4); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define openInjector5_MC33810() setMC33810_2_ACTIVE(); BIT_SET(mc33810_2_requestedState, MC33810_BIT_INJ5); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define openInjector6_MC33810() setMC33810_2_ACTIVE(); BIT_SET(mc33810_2_requestedState, MC33810_BIT_INJ6); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define openInjector7_MC33810() setMC33810_2_ACTIVE(); BIT_SET(mc33810_2_requestedState, MC33810_BIT_INJ7); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define openInjector8_MC33810() setMC33810_2_ACTIVE(); BIT_SET(mc33810_2_requestedState, MC33810_BIT_INJ8); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();

#define closeInjector1_MC33810() setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1_requestedState, MC33810_BIT_INJ1); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define closeInjector2_MC33810() setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1_requestedState, MC33810_BIT_INJ2); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define closeInjector3_MC33810() setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1_requestedState, MC33810_BIT_INJ3); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define closeInjector4_MC33810() setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1_requestedState, MC33810_BIT_INJ4); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define closeInjector5_MC33810() setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2_requestedState, MC33810_BIT_INJ5); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define closeInjector6_MC33810() setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2_requestedState, MC33810_BIT_INJ6); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define closeInjector7_MC33810() setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2_requestedState, MC33810_BIT_INJ7); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define closeInjector8_MC33810() setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2_requestedState, MC33810_BIT_INJ8); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();

#define coil1High_MC33810() setMC33810_1_ACTIVE(); BIT_SET(mc33810_1_requestedState, MC33810_BIT_IGN1); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define coil2High_MC33810() setMC33810_1_ACTIVE(); BIT_SET(mc33810_1_requestedState, MC33810_BIT_IGN2); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define coil3High_MC33810() setMC33810_1_ACTIVE(); BIT_SET(mc33810_1_requestedState, MC33810_BIT_IGN3); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define coil4High_MC33810() setMC33810_1_ACTIVE(); BIT_SET(mc33810_1_requestedState, MC33810_BIT_IGN4); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define coil5High_MC33810() setMC33810_2_ACTIVE(); BIT_SET(mc33810_2_requestedState, MC33810_BIT_IGN5); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define coil6High_MC33810() setMC33810_2_ACTIVE(); BIT_SET(mc33810_2_requestedState, MC33810_BIT_IGN6); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define coil7High_MC33810() setMC33810_2_ACTIVE(); BIT_SET(mc33810_2_requestedState, MC33810_BIT_IGN7); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define coil8High_MC33810() setMC33810_2_ACTIVE(); BIT_SET(mc33810_2_requestedState, MC33810_BIT_IGN8); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();

#define coil1Low_MC33810() setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1_requestedState, MC33810_BIT_IGN1); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define coil2Low_MC33810() setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1_requestedState, MC33810_BIT_IGN2); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define coil3Low_MC33810() setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1_requestedState, MC33810_BIT_IGN3); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define coil4Low_MC33810() setMC33810_1_ACTIVE(); BIT_CLEAR(mc33810_1_requestedState, MC33810_BIT_IGN4); mc33810_1_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_1_requestedState)); setMC33810_1_INACTIVE();
#define coil5Low_MC33810() setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2_requestedState, MC33810_BIT_IGN5); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define coil6Low_MC33810() setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2_requestedState, MC33810_BIT_IGN6); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define coil7Low_MC33810() setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2_requestedState, MC33810_BIT_IGN7); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();
#define coil8Low_MC33810() setMC33810_2_ACTIVE(); BIT_CLEAR(mc33810_2_requestedState, MC33810_BIT_IGN8); mc33810_2_returnState = SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810_2_requestedState)); setMC33810_2_INACTIVE();

#define coil1Charging_MC33810()      if(configPage4.IgInv == GOING_HIGH) { coil1Low_MC33810();  } else { coil1High_MC33810(); }
#define coil1StopCharging_MC33810()  if(configPage4.IgInv == GOING_HIGH) { coil1High_MC33810(); } else { coil1Low_MC33810();  }
#define coil2Charging_MC33810()      if(configPage4.IgInv == GOING_HIGH) { coil2Low_MC33810();  } else { coil2High_MC33810(); }
#define coil2StopCharging_MC33810()  if(configPage4.IgInv == GOING_HIGH) { coil2High_MC33810(); } else { coil2Low_MC33810();  }
#define coil3Charging_MC33810()      if(configPage4.IgInv == GOING_HIGH) { coil3Low_MC33810();  } else { coil3High_MC33810(); }
#define coil3StopCharging_MC33810()  if(configPage4.IgInv == GOING_HIGH) { coil3High_MC33810(); } else { coil3Low_MC33810();  }
#define coil4Charging_MC33810()      if(configPage4.IgInv == GOING_HIGH) { coil4Low_MC33810();  } else { coil4High_MC33810(); }
#define coil4StopCharging_MC33810()  if(configPage4.IgInv == GOING_HIGH) { coil4High_MC33810(); } else { coil4Low_MC33810();  }
#define coil5Charging_MC33810()      if(configPage4.IgInv == GOING_HIGH) { coil5Low_MC33810();  } else { coil5High_MC33810(); }
#define coil5StopCharging_MC33810()  if(configPage4.IgInv == GOING_HIGH) { coil5High_MC33810(); } else { coil5Low_MC33810();  }
#define coil6Charging_MC33810()      if(configPage4.IgInv == GOING_HIGH) { coil6Low_MC33810();  } else { coil6High_MC33810(); }
#define coil6StopCharging_MC33810()  if(configPage4.IgInv == GOING_HIGH) { coil6High_MC33810(); } else { coil6Low_MC33810();  }
#define coil7Charging_MC33810()      if(configPage4.IgInv == GOING_HIGH) { coil7Low_MC33810();  } else { coil7High_MC33810(); }
#define coil7StopCharging_MC33810()  if(configPage4.IgInv == GOING_HIGH) { coil7High_MC33810(); } else { coil7Low_MC33810();  }
#define coil8Charging_MC33810()      if(configPage4.IgInv == GOING_HIGH) { coil8Low_MC33810();  } else { coil8High_MC33810(); }
#define coil8StopCharging_MC33810()  if(configPage4.IgInv == GOING_HIGH) { coil8High_MC33810(); } else { coil8Low_MC33810();  }

#endif