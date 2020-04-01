#ifndef SCHEDULEDIO_H
#define SCHEDULEDIO_H

inline void openInjector1();
inline void closeInjector1();

inline void openInjector2();
inline void closeInjector2();

inline void openInjector3();
inline void closeInjector3();

inline void openInjector4();
inline void closeInjector4();

inline void openInjector5();
inline void closeInjector5();

inline void openInjector6();
inline void closeInjector6();

inline void openInjector7();
inline void closeInjector7();

inline void openInjector8();
inline void closeInjector8();

// These are for Semi-Sequential and 5 Cylinder injection
void openInjector1and4();
void closeInjector1and4();
void openInjector2and3();
void closeInjector2and3();

void openInjector3and5();
void closeInjector3and5();

void openInjector2and5();
void closeInjector2and5();
void openInjector3and6();
void closeInjector3and6();

void openInjector1and5();
void closeInjector1and5();
void openInjector2and6();
void closeInjector2and6();
void openInjector3and7();
void closeInjector3and7();
void openInjector4and8();
void closeInjector4and8();

//If coil inverse is on, set the output low, else set it high
//#define beginCoil1Charge() { configPage4.IgInv == 1 ? coil1Low(); : coil1High(); } tachoOutputFlag = READY;
//#define beginCoil2Charge() { configPage4.IgInv == 1 ? coil2Low(); : coil2High(); } tachoOutputFlag = READY;
//#define beginCoil3Charge() { configPage4.IgInv == 1 ? coil3Low(); : coil3High(); } tachoOutputFlag = READY;
//#define beginCoil4Charge() { configPage4.IgInv == 1 ? coil4Low(); : coil4High(); } tachoOutputFlag = READY;
//#define beginCoil5Charge() { configPage4.IgInv == 1 ? coil5Low(); : coil5High(); } tachoOutputFlag = READY;
//#define beginCoil6Charge() { configPage4.IgInv == 1 ? coil6Low(); : coil6High(); } tachoOutputFlag = READY;
//#define beginCoil7Charge() { configPage4.IgInv == 1 ? coil7Low(); : coil7High(); } tachoOutputFlag = READY;
//#define beginCoil8Charge() { configPage4.IgInv == 1 ? coil8Low(); : coil8High(); } tachoOutputFlag = READY;

inline void beginCoil1Charge();
inline void endCoil1Charge();

inline void beginCoil2Charge();
inline void endCoil2Charge();

inline void beginCoil3Charge();
inline void endCoil3Charge();

inline void beginCoil4Charge();
inline void endCoil4Charge();

inline void beginCoil5Charge();
inline void endCoil5Charge();

inline void beginCoil6Charge();
inline void endCoil6Charge();

inline void beginCoil7Charge();
inline void endCoil7Charge();

inline void beginCoil8Charge();
inline void endCoil8Charge();

//The following functions are used specifically for the trailing coil on rotary engines. They are separate as they also control the switching of the trailing select pin
inline void beginTrailingCoilCharge();
inline void endTrailingCoilCharge1();
inline void endTrailingCoilCharge2();

//And the combined versions of the above for simplicity
void beginCoil1and3Charge();
void endCoil1and3Charge();
void beginCoil2and4Charge();
void endCoil2and4Charge();

//For 6-cyl cop
void beginCoil1and4Charge();
void endCoil1and4Charge();
void beginCoil2and5Charge();
void endCoil2and5Charge();
void beginCoil3and6Charge();
void endCoil3and6Charge();

//For 8-cyl cop
void beginCoil1and5Charge();
void endCoil1and5Charge();
void beginCoil2and6Charge();
void endCoil2and6Charge();
void beginCoil3and7Charge();
void endCoil3and7Charge();
void beginCoil4and8Charge();
void endCoil4and8Charge();
/*
#define beginCoil1and3Charge() beginCoil1Charge(); beginCoil3Charge()
#define endCoil1and3Charge()
#define beginCoil2and4Charge() beginCoil2Charge(); beginCoil4Charge()
#define endCoil2and4Charge();
*/

/*
#ifndef USE_MC33810
#define openInjector1() *inj1_pin_port |= (inj1_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ1)
#define closeInjector1() *inj1_pin_port &= ~(inj1_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ1)
#define openInjector2() *inj2_pin_port |= (inj2_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ2)
#define closeInjector2() *inj2_pin_port &= ~(inj2_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ2)
#define openInjector3() *inj3_pin_port |= (inj3_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ3)
#define closeInjector3() *inj3_pin_port &= ~(inj3_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ3)
#define openInjector4() *inj4_pin_port |= (inj4_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ4)
#define closeInjector4() *inj4_pin_port &= ~(inj4_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ4)
#define openInjector5() *inj5_pin_port |= (inj5_pin_mask);
#define closeInjector5() *inj5_pin_port &= ~(inj5_pin_mask);
#define openInjector6() *inj6_pin_port |= (inj6_pin_mask);
#define closeInjector6() *inj6_pin_port &= ~(inj6_pin_mask);
#define openInjector7() *inj7_pin_port |= (inj7_pin_mask);
#define closeInjector7() *inj7_pin_port &= ~(inj7_pin_mask);
#define openInjector8() *inj8_pin_port |= (inj8_pin_mask);
#define closeInjector8() *inj8_pin_port &= ~(inj8_pin_mask);

#else
#include "acc_mc33810.h"
#define openInjector1() openInjector1_MC33810()
#define closeInjector1() closeInjector1_MC33810()
#define openInjector2() openInjector2_MC33810()
#define closeInjector2() closeInjector2_MC33810()
#define openInjector3() openInjector3_MC33810()
#define closeInjector3() closeInjector3_MC33810()
#define openInjector4() openInjector4_MC33810()
#define closeInjector4() closeInjector4_MC33810()
#define openInjector5() openInjector5_MC33810()
#define closeInjector5() closeInjector5_MC33810()
#define openInjector6() openInjector6_MC33810()
#define closeInjector6() closeInjector6_MC33810()
#define openInjector7() openInjector7_MC33810()
#define closeInjector7() closeInjector7_MC33810()
#define openInjector8() openInjector8_MC33810()
#define closeInjector8() closeInjector8_MC33810()

#endif

#define openInjector1and4() openInjector1(); openInjector4()
#define closeInjector1and4() closeInjector1(); closeInjector4()
#define openInjector2and3() openInjector2(); openInjector3()
#define closeInjector2and3() closeInjector2(); closeInjector3()

//5 cylinder support doubles up injector 3 as being closese to inj 5 (Crank angle)
#define openInjector3and5() openInjector3(); openInjector5()
#define closeInjector3and5() closeInjector3(); closeInjector5()
*/

#ifndef USE_MC33810
#define coil1Low() (*ign1_pin_port &= ~(ign1_pin_mask))
#define coil1High() (*ign1_pin_port |= (ign1_pin_mask))
#define coil2Low() (*ign2_pin_port &= ~(ign2_pin_mask))
#define coil2High() (*ign2_pin_port |= (ign2_pin_mask))
#define coil3Low() (*ign3_pin_port &= ~(ign3_pin_mask))
#define coil3High() (*ign3_pin_port |= (ign3_pin_mask))
#define coil4Low() (*ign4_pin_port &= ~(ign4_pin_mask))
#define coil4High() (*ign4_pin_port |= (ign4_pin_mask))
#define coil5Low() (*ign5_pin_port &= ~(ign5_pin_mask))
#define coil5High() (*ign5_pin_port |= (ign5_pin_mask))
#define coil6Low() (*ign6_pin_port &= ~(ign6_pin_mask))
#define coil6High() (*ign6_pin_port |= (ign6_pin_mask))
#define coil7Low() (*ign7_pin_port &= ~(ign7_pin_mask))
#define coil7High() (*ign7_pin_port |= (ign7_pin_mask))
#define coil8Low() (*ign8_pin_port &= ~(ign8_pin_mask))
#define coil8High() (*ign8_pin_port |= (ign8_pin_mask))
#else
#define coil1Low() coil1Low_MC33810()
#define coil1High() coil1High_MC33810()
#define coil2Low() coil2Low_MC33810()
#define coil2High() coil2High_MC33810()
#define coil3Low() coil3Low_MC33810()
#define coil3High()coil3High_MC33810()
#define coil4Low() coil4Low_MC33810()
#define coil4High() coil4High_MC33810()
#define coil5Low() coil5Low_MC33810()
#define coil5High() coil5High_MC33810()
#define coil6Low() coil6Low_MC33810()
#define coil6High() coil6High_MC33810()
#define coil7Low() coil7Low_MC33810()
#define coil7High() coil7High_MC33810()
#define coil8Low() coil8Low_MC33810()
#define coil8High() coil8High_MC33810()
#endif

void nullCallback();

static byte coilHIGH = HIGH;
static byte coilLOW = LOW;

#endif
