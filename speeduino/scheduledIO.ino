#include "scheduledIO.h"
#include "scheduler.h"
#include "globals.h"
#include "timers.h"

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
  inline void beginTrailingCoilCharge() { digitalWrite(pinCoil2, coilHIGH); }
  inline void endTrailingCoilCharge1() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port |= ign3_pin_mask; } //Sets ign3 (Trailing select) high
  inline void endTrailingCoilCharge2() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port &= ~(ign3_pin_mask); } //sets ign3 (Trailing select) low

  //As above but for ignition (Wasted COP mode)
  void beginCoil1and3Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil3, coilHIGH); tachoOutputFlag = READY; }
  void endCoil1and3Charge()   { digitalWrite(pinCoil1, coilLOW);  digitalWrite(pinCoil3, coilLOW);  }
  void beginCoil2and4Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil4, coilHIGH); tachoOutputFlag = READY; }
  void endCoil2and4Charge()   { digitalWrite(pinCoil2, coilLOW);  digitalWrite(pinCoil4, coilLOW);  }
#endif

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

void nullCallback() { return; }
