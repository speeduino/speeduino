#include "scheduledIO.h"
#include "scheduler.h"
#include "globals.h"
#include "timers.h"

#ifndef USE_MC33810
  inline void openInjector1() { *inj1_pin_port |= (inj1_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ1); }
  inline void closeInjector1() { *inj1_pin_port &= ~(inj1_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ1); }
  inline void openInjector2() { *inj2_pin_port |= (inj2_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ2); }
  inline void closeInjector2() { *inj2_pin_port &= ~(inj2_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ2); }
  inline void openInjector3() { *inj3_pin_port |= (inj3_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ3); }
  inline void closeInjector3() { *inj3_pin_port &= ~(inj3_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ3); }
  inline void openInjector4() { *inj4_pin_port |= (inj4_pin_mask); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ4); }
  inline void closeInjector4() { *inj4_pin_port &= ~(inj4_pin_mask);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ4); }
  inline void openInjector5() { *inj5_pin_port |= (inj5_pin_mask); }
  inline void closeInjector5() { *inj5_pin_port &= ~(inj5_pin_mask); }
  inline void openInjector6() { *inj6_pin_port |= (inj6_pin_mask); }
  inline void closeInjector6() { *inj6_pin_port &= ~(inj6_pin_mask); }
  inline void openInjector7() { *inj7_pin_port |= (inj7_pin_mask); }
  inline void closeInjector7() { *inj7_pin_port &= ~(inj7_pin_mask); }
  inline void openInjector8() { *inj8_pin_port |= (inj8_pin_mask); }
  inline void closeInjector8() { *inj8_pin_port &= ~(inj8_pin_mask); }
#else
#include "acc_mc33810.h"
  inline void openInjector1() { openInjector1_MC33810(); }
  inline void closeInjector1() { closeInjector1_MC33810(); }
  inline void openInjector2() { openInjector2_MC33810(); }
  inline void closeInjector2() { closeInjector2_MC33810(); }
  inline void openInjector3() { openInjector3_MC33810(); }
  inline void closeInjector3() { closeInjector3_MC33810(); }
  inline void openInjector4() { openInjector4_MC33810(); }
  inline void closeInjector4() { closeInjector4_MC33810(); }
  inline void openInjector5() { openInjector5_MC33810(); }
  inline void closeInjector5() { closeInjector5_MC33810(); }
  inline void openInjector6() { openInjector6_MC33810(); }
  inline void closeInjector6() { closeInjector6_MC33810(); }
  inline void openInjector7() { openInjector7_MC33810(); }
  inline void closeInjector7() { closeInjector7_MC33810(); }
  inline void openInjector8() { openInjector8_MC33810(); }
  inline void closeInjector8() { closeInjector8_MC33810(); }
#endif

// These are for Semi-Sequential and 5 Cylinder injection
void openInjector1and4() { openInjector1(); openInjector4(); }
void closeInjector1and4() { closeInjector1(); closeInjector4(); }
void openInjector2and3() { openInjector2(); openInjector3(); }
void closeInjector2and3() { closeInjector2(); closeInjector3(); }

void openInjector3and5() { openInjector3(); openInjector5(); }
void closeInjector3and5() { closeInjector3(); closeInjector5(); }

void openInjector2and5() { openInjector2(); openInjector5(); }
void closeInjector2and5() { closeInjector2(); closeInjector5(); }
void openInjector3and6() { openInjector3(); openInjector6(); }
void closeInjector3and6() { closeInjector3(); closeInjector6(); }

void openInjector1and5() { openInjector1(); openInjector5(); }
void closeInjector1and5() { closeInjector1(); closeInjector5(); }
void openInjector2and6() { openInjector2(); openInjector6(); }
void closeInjector2and6() { closeInjector2(); closeInjector6(); }
void openInjector3and7() { openInjector3(); openInjector7(); }
void closeInjector3and7() { closeInjector3(); closeInjector7(); }
void openInjector4and8() { openInjector4(); openInjector8(); }
void closeInjector4and8() { closeInjector4(); closeInjector8(); }

#ifndef USE_MC33810
  inline void beginCoil1Charge() { digitalWrite(pinCoil1, coilHIGH); tachoOutputFlag = READY; }
  inline void endCoil1Charge() { digitalWrite(pinCoil1, coilLOW); }

  inline void beginCoil2Charge() { digitalWrite(pinCoil2, coilHIGH); tachoOutputFlag = READY; }
  inline void endCoil2Charge() { digitalWrite(pinCoil2, coilLOW); }

  inline void beginCoil3Charge() { digitalWrite(pinCoil3, coilHIGH); tachoOutputFlag = READY; }
  inline void endCoil3Charge() { digitalWrite(pinCoil3, coilLOW); }

  inline void beginCoil4Charge() { digitalWrite(pinCoil4, coilHIGH); tachoOutputFlag = READY; }
  inline void endCoil4Charge() { digitalWrite(pinCoil4, coilLOW); }

  inline void beginCoil5Charge() { digitalWrite(pinCoil5, coilHIGH); tachoOutputFlag = READY; }
  inline void endCoil5Charge() { digitalWrite(pinCoil5, coilLOW); }

  inline void beginCoil6Charge() { digitalWrite(pinCoil6, coilHIGH); tachoOutputFlag = READY; }
  inline void endCoil6Charge() { digitalWrite(pinCoil6, coilLOW); }

  inline void beginCoil7Charge() { digitalWrite(pinCoil7, coilHIGH); tachoOutputFlag = READY; }
  inline void endCoil7Charge() { digitalWrite(pinCoil7, coilLOW); }

  inline void beginCoil8Charge() { digitalWrite(pinCoil8, coilHIGH); tachoOutputFlag = READY; }
  inline void endCoil8Charge() { digitalWrite(pinCoil8, coilLOW); }

  //The below 3 calls are all part of the rotary ignition mode
  inline void beginTrailingCoilCharge() { digitalWrite(pinCoil2, coilHIGH); }
  inline void endTrailingCoilCharge1() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port |= ign3_pin_mask; } //Sets ign3 (Trailing select) high
  inline void endTrailingCoilCharge2() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port &= ~(ign3_pin_mask); } //sets ign3 (Trailing select) low

  //As above but for ignition (Wasted COP mode)
  void beginCoil1and3Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil3, coilHIGH); tachoOutputFlag = READY; }
  void endCoil1and3Charge()   { digitalWrite(pinCoil1, coilLOW);  digitalWrite(pinCoil3, coilLOW);  }
  void beginCoil2and4Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil4, coilHIGH); tachoOutputFlag = READY; }
  void endCoil2and4Charge()   { digitalWrite(pinCoil2, coilLOW);  digitalWrite(pinCoil4, coilLOW);  }

  //For 6cyl wasted COP mode)
  void beginCoil1and4Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil4, coilHIGH); tachoOutputFlag = READY; }
  void endCoil1and4Charge()   { digitalWrite(pinCoil1, coilLOW);  digitalWrite(pinCoil4, coilLOW);  }
  void beginCoil2and5Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil5, coilHIGH); tachoOutputFlag = READY; }
  void endCoil2and5Charge()   { digitalWrite(pinCoil2, coilLOW);  digitalWrite(pinCoil5, coilLOW);  }
  void beginCoil3and6Charge() { digitalWrite(pinCoil3, coilHIGH); digitalWrite(pinCoil6, coilHIGH); tachoOutputFlag = READY; }
  void endCoil3and6Charge()   { digitalWrite(pinCoil3, coilLOW);  digitalWrite(pinCoil6, coilLOW);  }

  //For 8cyl wasted COP mode)
  void beginCoil1and5Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil5, coilHIGH); tachoOutputFlag = READY; }
  void endCoil1and5Charge()   { digitalWrite(pinCoil1, coilLOW);  digitalWrite(pinCoil5, coilLOW);  }
  void beginCoil2and6Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil6, coilHIGH); tachoOutputFlag = READY; }
  void endCoil2and6Charge()   { digitalWrite(pinCoil2, coilLOW);  digitalWrite(pinCoil6, coilLOW);  }
  void beginCoil3and7Charge() { digitalWrite(pinCoil3, coilHIGH); digitalWrite(pinCoil7, coilHIGH); tachoOutputFlag = READY; }
  void endCoil3and7Charge()   { digitalWrite(pinCoil3, coilLOW);  digitalWrite(pinCoil7, coilLOW);  }
  void beginCoil4and8Charge() { digitalWrite(pinCoil4, coilHIGH); digitalWrite(pinCoil8, coilHIGH); tachoOutputFlag = READY; }
  void endCoil4and8Charge()   { digitalWrite(pinCoil4, coilLOW);  digitalWrite(pinCoil8, coilLOW);  }
#else
  //This really should be combined with the above code at some point, but need to write a neat macro to handle coilHIGH vs coilLOW
  inline void beginCoil1Charge() { coil1High(); tachoOutputFlag = READY; }
  inline void endCoil1Charge() { coil1Low(); }

  inline void beginCoil2Charge() { coil2High(); tachoOutputFlag = READY; }
  inline void endCoil2Charge() { coil2Low(); }

  inline void beginCoil3Charge() { coil3High(); tachoOutputFlag = READY; }
  inline void endCoil3Charge() { coil3Low(); }

  inline void beginCoil4Charge() { coil4High(); tachoOutputFlag = READY; }
  inline void endCoil4Charge() { coil4Low(); }

  inline void beginCoil5Charge() { coil5High(); tachoOutputFlag = READY; }
  inline void endCoil5Charge() { coil5Low(); }

  inline void beginCoil6Charge() { coil6High(); tachoOutputFlag = READY; }
  inline void endCoil6Charge() { coil6Low(); }

  inline void beginCoil7Charge() { coil7High(); tachoOutputFlag = READY; }
  inline void endCoil7Charge() { coil7Low(); }

  inline void beginCoil8Charge() { coil8High(); tachoOutputFlag = READY; }
  inline void endCoil8Charge() { coil8Low(); }

  //The below 3 calls are all part of the rotary ignition mode
  inline void beginTrailingCoilCharge() { coil2High(); }
  inline void endTrailingCoilCharge1() { coil2Low(); coil3High(); } //Sets ign3 (Trailing select) high
  inline void endTrailingCoilCharge2() { coil2Low(); coil3Low(); } //sets ign3 (Trailing select) low

  //As above but for ignition (Wasted COP mode)
  void beginCoil1and3Charge() { beginCoil1Charge(); beginCoil3Charge(); }
  void endCoil1and3Charge()   { endCoil1Charge();  endCoil3Charge(); }
  void beginCoil2and4Charge() { beginCoil2Charge(); beginCoil4Charge(); }
  void endCoil2and4Charge()   { endCoil2Charge(); endCoil4Charge(); }

  //For 6cyl wasted COP mode)
  void beginCoil1and4Charge() { beginCoil1Charge(); beginCoil4Charge(); }
  void endCoil1and4Charge()   { endCoil1Charge();  endCoil4Charge();  }
  void beginCoil2and5Charge() { beginCoil2Charge(); beginCoil5Charge(); }
  void endCoil2and5Charge()   { endCoil2Charge();  endCoil5Charge(); }
  void beginCoil3and6Charge() { beginCoil3Charge(); beginCoil6Charge(); }
  void endCoil3and6Charge()   { endCoil3Charge();  endCoil6Charge();  }

  //For 8cyl wasted COP mode)
  void beginCoil1and5Charge() { beginCoil1Charge(); beginCoil5Charge(); }
  void endCoil1and5Charge()   { endCoil1Charge();  endCoil5Charge();  }
  void beginCoil2and6Charge() { beginCoil2Charge(); beginCoil6Charge(); }
  void endCoil2and6Charge()   { endCoil2Charge();  endCoil6Charge();  }
  void beginCoil3and7Charge() { beginCoil3Charge(); beginCoil7Charge(); }
  void endCoil3and7Charge()   { endCoil3Charge();  endCoil7Charge();  }
  void beginCoil4and8Charge() { beginCoil4Charge(); beginCoil8Charge(); }
  void endCoil4and8Charge()   { endCoil4Charge();  endCoil8Charge();  }
#endif

void nullCallback() { return; }
