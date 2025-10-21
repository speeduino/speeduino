#ifndef SCHEDULEDIO_H
#define SCHEDULEDIO_H

#include <Arduino.h>

void openInjector1(void);
void closeInjector1(void);

void openInjector2(void);
void closeInjector2(void);

void openInjector3(void);
void closeInjector3(void);

void openInjector4(void);
void closeInjector4(void);

void openInjector5(void);
void closeInjector5(void);

void openInjector6(void);
void closeInjector6(void);

void openInjector7(void);
void closeInjector7(void);

void openInjector8(void);
void closeInjector8(void);

// These are for Semi-Sequential and 5 Cylinder injection
void openInjector1and3(void);
void closeInjector1and3(void);
void openInjector2and4(void);
void closeInjector2and4(void);
void openInjector1and4(void);
void closeInjector1and4(void);
void openInjector2and3(void);
void closeInjector2and3(void);

void openInjector3and5(void);
void closeInjector3and5(void);

void openInjector2and5(void);
void closeInjector2and5(void);
void openInjector3and6(void);
void closeInjector3and6(void);

void openInjector1and5(void);
void closeInjector1and5(void);
void openInjector2and6(void);
void closeInjector2and6(void);
void openInjector3and7(void);
void closeInjector3and7(void);
void openInjector4and8(void);
void closeInjector4and8(void);

void injector1Toggle(void);
void injector2Toggle(void);
void injector3Toggle(void);
void injector4Toggle(void);
void injector5Toggle(void);
void injector6Toggle(void);
void injector7Toggle(void);
void injector8Toggle(void);

void beginCoil1Charge(void);
void endCoil1Charge(void);

void beginCoil2Charge(void);
void endCoil2Charge(void);

void beginCoil3Charge(void);
void endCoil3Charge(void);

void beginCoil4Charge(void);
void endCoil4Charge(void);

void beginCoil5Charge(void);
void endCoil5Charge(void);

void beginCoil6Charge(void);
void endCoil6Charge(void);

void beginCoil7Charge(void);
void endCoil7Charge(void);

void beginCoil8Charge(void);
void endCoil8Charge(void);

//The following functions are used specifically for the trailing coil on rotary engines. They are separate as they also control the switching of the trailing select pin
void beginTrailingCoilCharge(void);
void endTrailingCoilCharge1(void);
void endTrailingCoilCharge2(void);

//And the combined versions of the above for simplicity
void beginCoil1and3Charge(void);
void endCoil1and3Charge(void);
void beginCoil2and4Charge(void);
void endCoil2and4Charge(void);

//For 6-cyl cop
void beginCoil1and4Charge(void);
void endCoil1and4Charge(void);
void beginCoil2and5Charge(void);
void endCoil2and5Charge(void);
void beginCoil3and6Charge(void);
void endCoil3and6Charge(void);

//For 8-cyl cop
void beginCoil1and5Charge(void);
void endCoil1and5Charge(void);
void beginCoil2and6Charge(void);
void endCoil2and6Charge(void);
void beginCoil3and7Charge(void);
void endCoil3and7Charge(void);
void beginCoil4and8Charge(void);
void endCoil4and8Charge(void);

void coil1Toggle(void);
void coil2Toggle(void);
void coil3Toggle(void);
void coil4Toggle(void);
void coil5Toggle(void);
void coil6Toggle(void);
void coil7Toggle(void);
void coil8Toggle(void);

void tachoOutputOn(void);
void tachoOutputOff(void);

//Macros are used to define how each injector control system functions. These are then called by the master openInjectx() function.
//The DIRECT macros (ie individual pins) are defined below. Others should be defined in their relevant acc_x.h file
#define openInjector1_DIRECT()  { *inj1_pin_port |= (inj1_pin_mask); currentStatus.isInj1Open = true; }
#define closeInjector1_DIRECT() { *inj1_pin_port &= ~(inj1_pin_mask);  currentStatus.isInj1Open = false; }
#define openInjector2_DIRECT()  { *inj2_pin_port |= (inj2_pin_mask); currentStatus.isInj2Open = true; }
#define closeInjector2_DIRECT() { *inj2_pin_port &= ~(inj2_pin_mask);  currentStatus.isInj2Open = false; }
#define openInjector3_DIRECT()  { *inj3_pin_port |= (inj3_pin_mask); currentStatus.isInj3Open = true; }
#define closeInjector3_DIRECT() { *inj3_pin_port &= ~(inj3_pin_mask);  currentStatus.isInj3Open = false; }
#define openInjector4_DIRECT()  { *inj4_pin_port |= (inj4_pin_mask); currentStatus.isInj4Open = true; }
#define closeInjector4_DIRECT() { *inj4_pin_port &= ~(inj4_pin_mask);  currentStatus.isInj4Open = false; }
#define openInjector5_DIRECT()  { *inj5_pin_port |= (inj5_pin_mask); }
#define closeInjector5_DIRECT() { *inj5_pin_port &= ~(inj5_pin_mask); }
#define openInjector6_DIRECT()  { *inj6_pin_port |= (inj6_pin_mask); }
#define closeInjector6_DIRECT() { *inj6_pin_port &= ~(inj6_pin_mask); }
#define openInjector7_DIRECT()  { *inj7_pin_port |= (inj7_pin_mask); }
#define closeInjector7_DIRECT() { *inj7_pin_port &= ~(inj7_pin_mask); }
#define openInjector8_DIRECT()  { *inj8_pin_port |= (inj8_pin_mask); }
#define closeInjector8_DIRECT() { *inj8_pin_port &= ~(inj8_pin_mask); }

#define coil1Low_DIRECT()       (*ign1_pin_port &= ~(ign1_pin_mask))
#define coil1High_DIRECT()      (*ign1_pin_port |= (ign1_pin_mask))
#define coil2Low_DIRECT()       (*ign2_pin_port &= ~(ign2_pin_mask))
#define coil2High_DIRECT()      (*ign2_pin_port |= (ign2_pin_mask))
#define coil3Low_DIRECT()       (*ign3_pin_port &= ~(ign3_pin_mask))
#define coil3High_DIRECT()      (*ign3_pin_port |= (ign3_pin_mask))
#define coil4Low_DIRECT()       (*ign4_pin_port &= ~(ign4_pin_mask))
#define coil4High_DIRECT()      (*ign4_pin_port |= (ign4_pin_mask))
#define coil5Low_DIRECT()       (*ign5_pin_port &= ~(ign5_pin_mask))
#define coil5High_DIRECT()      (*ign5_pin_port |= (ign5_pin_mask))
#define coil6Low_DIRECT()       (*ign6_pin_port &= ~(ign6_pin_mask))
#define coil6High_DIRECT()      (*ign6_pin_port |= (ign6_pin_mask))
#define coil7Low_DIRECT()       (*ign7_pin_port &= ~(ign7_pin_mask))
#define coil7High_DIRECT()      (*ign7_pin_port |= (ign7_pin_mask))
#define coil8Low_DIRECT()       (*ign8_pin_port &= ~(ign8_pin_mask))
#define coil8High_DIRECT()      (*ign8_pin_port |= (ign8_pin_mask))

//Set the value of the coil pins to the coilHIGH or coilLOW state
#define coil1Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil1Low_DIRECT() : coil1High_DIRECT())
#define coil1StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil1High_DIRECT() : coil1Low_DIRECT())
#define coil2Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil2Low_DIRECT() : coil2High_DIRECT())
#define coil2StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil2High_DIRECT() : coil2Low_DIRECT())
#define coil3Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil3Low_DIRECT() : coil3High_DIRECT())
#define coil3StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil3High_DIRECT() : coil3Low_DIRECT())
#define coil4Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil4Low_DIRECT() : coil4High_DIRECT())
#define coil4StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil4High_DIRECT() : coil4Low_DIRECT())
#define coil5Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil5Low_DIRECT() : coil5High_DIRECT())
#define coil5StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil5High_DIRECT() : coil5Low_DIRECT())
#define coil6Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil6Low_DIRECT() : coil6High_DIRECT())
#define coil6StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil6High_DIRECT() : coil6Low_DIRECT())
#define coil7Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil7Low_DIRECT() : coil7High_DIRECT())
#define coil7StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil7High_DIRECT() : coil7Low_DIRECT())
#define coil8Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil8Low_DIRECT() : coil8High_DIRECT())
#define coil8StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil8High_DIRECT() : coil8Low_DIRECT())

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

#define coil1Toggle_DIRECT() (*ign1_pin_port ^= ign1_pin_mask )
#define coil2Toggle_DIRECT() (*ign2_pin_port ^= ign2_pin_mask )
#define coil3Toggle_DIRECT() (*ign3_pin_port ^= ign3_pin_mask )
#define coil4Toggle_DIRECT() (*ign4_pin_port ^= ign4_pin_mask )
#define coil5Toggle_DIRECT() (*ign5_pin_port ^= ign5_pin_mask )
#define coil6Toggle_DIRECT() (*ign6_pin_port ^= ign6_pin_mask )
#define coil7Toggle_DIRECT() (*ign7_pin_port ^= ign7_pin_mask )
#define coil8Toggle_DIRECT() (*ign8_pin_port ^= ign8_pin_mask )

#define injector1Toggle_DIRECT() (*inj1_pin_port ^= inj1_pin_mask )
#define injector2Toggle_DIRECT() (*inj2_pin_port ^= inj2_pin_mask )
#define injector3Toggle_DIRECT() (*inj3_pin_port ^= inj3_pin_mask )
#define injector4Toggle_DIRECT() (*inj4_pin_port ^= inj4_pin_mask )
#define injector5Toggle_DIRECT() (*inj5_pin_port ^= inj5_pin_mask )
#define injector6Toggle_DIRECT() (*inj6_pin_port ^= inj6_pin_mask )
#define injector7Toggle_DIRECT() (*inj7_pin_port ^= inj7_pin_mask )
#define injector8Toggle_DIRECT() (*inj8_pin_port ^= inj8_pin_mask )

void nullCallback(void);

typedef void (*voidVoidCallback)(void);

#endif
