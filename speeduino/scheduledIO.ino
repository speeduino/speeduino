
volatile bool tachoAlt = true;
#define TACH_PULSE_HIGH() *tach_pin_port |= (tach_pin_mask)
#define TACH_PULSE_LOW() if( (configPage2.tachoDiv == 0) || tachoAlt ) { *tach_pin_port &= ~(tach_pin_mask); tachoAlt = !tachoAlt; }



  inline void beginCoil1Charge() { digitalWrite(pinCoil1, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil1Charge() { digitalWrite(pinCoil1, coilLOW); TACH_PULSE_HIGH(); }

  inline void beginCoil2Charge() { digitalWrite(pinCoil2, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil2Charge() { digitalWrite(pinCoil2, coilLOW); TACH_PULSE_HIGH(); }

  inline void beginCoil3Charge() { digitalWrite(pinCoil3, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil3Charge() { digitalWrite(pinCoil3, coilLOW); TACH_PULSE_HIGH(); }

  inline void beginCoil4Charge() { digitalWrite(pinCoil4, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil4Charge() { digitalWrite(pinCoil4, coilLOW); TACH_PULSE_HIGH(); }

  inline void beginCoil5Charge() { digitalWrite(pinCoil5, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil5Charge() { digitalWrite(pinCoil5, coilLOW); TACH_PULSE_HIGH(); }

  inline void beginCoil6Charge() { digitalWrite(pinCoil6, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil6Charge() { digitalWrite(pinCoil6, coilLOW); TACH_PULSE_HIGH(); }

  inline void beginCoil7Charge() { digitalWrite(pinCoil7, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil7Charge() { digitalWrite(pinCoil7, coilLOW); TACH_PULSE_HIGH(); }

  inline void beginCoil8Charge() { digitalWrite(pinCoil8, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil8Charge() { digitalWrite(pinCoil8, coilLOW); TACH_PULSE_HIGH(); }

  inline void beginTrailingCoilCharge() { digitalWrite(pinCoil2, coilHIGH); }
  inline void endTrailingCoilCharge1() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port |= ign3_pin_mask; } //Sets ign3 (Trailing select) high
  inline void endTrailingCoilCharge2() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port &= ~(ign3_pin_mask); } //sets ign3 (Trailing select) low

//As above but for ignition (Wasted COP mode)
void beginCoil1and3Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil3, coilHIGH); TACH_PULSE_LOW();  }
void endCoil1and3Charge()   { digitalWrite(pinCoil1, coilLOW);  digitalWrite(pinCoil3, coilLOW);  TACH_PULSE_HIGH(); }
void beginCoil2and4Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil4, coilHIGH); TACH_PULSE_LOW();  }
void endCoil2and4Charge()   { digitalWrite(pinCoil2, coilLOW);  digitalWrite(pinCoil4, coilLOW);  TACH_PULSE_HIGH(); }

void nullCallback() { return; }
