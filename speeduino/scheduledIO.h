#ifndef SCHEDULEDIO_H
#define SCHEDULEDIO_H

//If coil inverse is on, set the output low, else set it high
//#define beginCoil1Charge() { configPage4.IgInv == 1 ? *ign1_pin_port &= ~(ign1_pin_mask); : *ign1_pin_port |= (ign1_pin_mask); } TACH_PULSE_LOW();

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


#define openInjector1() *inj1_pin_port |= (inj1_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ1)
#define closeInjector1() *inj1_pin_port &= ~(inj1_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ1)
#define openInjector2() *inj2_pin_port |= (inj2_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ2)
#define closeInjector2() *inj2_pin_port &= ~(inj2_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ2)
#define openInjector3() *inj3_pin_port |= (inj3_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ3)
#define closeInjector3() *inj3_pin_port &= ~(inj3_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ3)
#define openInjector4() *inj4_pin_port |= (inj4_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ4)
#define closeInjector4() *inj4_pin_port &= ~(inj4_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ4)
#define openInjector5() *inj5_pin_port |= (inj5_pin_mask)
#define closeInjector5() *inj5_pin_port &= ~(inj5_pin_mask)
//Dynamic functions below
#define openInjector6() *inj6_pin_port |= (inj6_pin_mask);
#define closeInjector6() *inj6_pin_port &= ~(inj6_pin_mask);
#define openInjector7() *inj7_pin_port |= (inj7_pin_mask);
#define closeInjector7() *inj7_pin_port &= ~(inj7_pin_mask);
#define openInjector8() *inj8_pin_port |= (inj8_pin_mask);
#define closeInjector8() *inj8_pin_port &= ~(inj8_pin_mask);

#define openInjector1and4() openInjector1(); openInjector4()
#define closeInjector1and4() closeInjector1(); closeInjector4()
#define openInjector2and3() openInjector2(); openInjector3()
#define closeInjector2and3() closeInjector2(); closeInjector3()

//5 cylinder support doubles up injector 3 as being closese to inj 5 (Crank angle)
#define openInjector3and5() openInjector3(); openInjector5()
#define closeInjector3and5() closeInjector3(); closeInjector5()

#endif
