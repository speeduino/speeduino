#ifndef SCHEDULEDIO_H
#define SCHEDULEDIO_H

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

void tachoOutputOn(void);
void tachoOutputOff(void);

#endif
