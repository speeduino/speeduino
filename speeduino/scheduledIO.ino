
volatile bool tachoAlt = true;
#define TACH_PULSE_HIGH() *tach_pin_port |= (tach_pin_mask)
#define TACH_PULSE_LOW() if( (configPage1.tachoDiv == 0) || tachoAlt ) { *tach_pin_port &= ~(tach_pin_mask); tachoAlt = !tachoAlt; }

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



//As above but for ignition (Wasted COP mode)
void beginCoil1and3Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil3, coilHIGH); TACH_PULSE_LOW();  }
void endCoil1and3Charge()   { digitalWrite(pinCoil1, coilLOW);  digitalWrite(pinCoil3, coilLOW);  TACH_PULSE_HIGH(); }
void beginCoil2and4Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil4, coilHIGH); TACH_PULSE_LOW();  }
void endCoil2and4Charge()   { digitalWrite(pinCoil2, coilLOW);  digitalWrite(pinCoil4, coilLOW);  TACH_PULSE_HIGH(); }

void nullCallback() { return; }
