

//These functions simply trigger the injector/coil driver off or on.
//NOTE: squirt status is changed as per http://www.msextra.com/doc/ms1extra/COM_RS232.htm#Acmd
/*
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  //For the AVR chips, use the faster bit flipping method of switching pins
  void ignitionSetter(byte *port, bool startCharge)
  {
    if(
  }

  void openInjector1() { *inj1_pin_port |= (inj1_pin_mask); ; BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ1); }
  void closeInjector1() { *inj1_pin_port &= ~(inj1_pin_mask);  BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ1); }
  void beginCoil1Charge() { *ign1_pin_port |= (ign1_pin_mask); BIT_SET(currentStatus.spark, 0); digitalWrite(pinTachOut, LOW); }
  void endCoil1Charge() { *ign1_pin_port &= ~(ign1_pin_mask); BIT_CLEAR(currentStatus.spark, 0); }

  void openInjector2() { *inj2_pin_port |= (inj2_pin_mask); ; BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ2); }
  void closeInjector2() { *inj2_pin_port &= ~(inj2_pin_mask);  BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ2); }
  void beginCoil2Charge() { *ign2_pin_port |= (ign2_pin_mask); BIT_SET(currentStatus.spark, 1); digitalWrite(pinTachOut, LOW); }
  void endCoil2Charge() { *ign2_pin_port &= ~(ign2_pin_mask); BIT_CLEAR(currentStatus.spark, 1);}

  void openInjector3() { *inj3_pin_port |= (inj3_pin_mask); ; BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ3); }
  void closeInjector3() { *inj3_pin_port &= ~(inj3_pin_mask);  BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ3); }
  void beginCoil3Charge() { *ign3_pin_port |= (ign3_pin_mask); BIT_SET(currentStatus.spark, 2); digitalWrite(pinTachOut, LOW); }
  void endCoil3Charge() { *ign3_pin_port &= ~(ign3_pin_mask); BIT_CLEAR(currentStatus.spark, 2);}

  void openInjector4() { *inj4_pin_port |= (inj4_pin_mask); ; BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ4); }
  void closeInjector4() { *inj4_pin_port &= ~(inj4_pin_mask);  BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ4); }
  void beginCoil4Charge() { *ign4_pin_port |= (ign4_pin_mask); BIT_SET(currentStatus.spark, 3); digitalWrite(pinTachOut, LOW); }
  void endCoil4Charge() { *ign4_pin_port &= ~(ign4_pin_mask); BIT_CLEAR(currentStatus.spark, 3);}

#else
*/

volatile bool tachoAlt = true;
#define TACH_PULSE_HIGH() *tach_pin_port |= (tach_pin_mask)
#define TACH_PULSE_LOW() if( (configPage1.tachoDiv == 0) || tachoAlt ) *tach_pin_port &= ~(tach_pin_mask); tachoAlt = !tachoAlt

  inline void openInjector1() { digitalWrite(pinInjector1, HIGH); BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ1); }
  inline void closeInjector1() { digitalWrite(pinInjector1, LOW); BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ1); }
  inline void beginCoil1Charge() { digitalWrite(pinCoil1, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil1Charge() { digitalWrite(pinCoil1, coilLOW); TACH_PULSE_HIGH(); }

  inline void openInjector2() { digitalWrite(pinInjector2, HIGH); BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ2); } //Sets the relevant pin HIGH and changes the current status bit for injector 2 (2nd bit of currentStatus.squirt)
  inline void closeInjector2() { digitalWrite(pinInjector2, LOW); BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ2); }
  inline void beginCoil2Charge() { digitalWrite(pinCoil2, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil2Charge() { digitalWrite(pinCoil2, coilLOW); TACH_PULSE_HIGH(); }

  inline void openInjector3() { digitalWrite(pinInjector3, HIGH); BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ3); } //Sets the relevant pin HIGH and changes the current status bit for injector 3 (3rd bit of currentStatus.squirt)
  inline void closeInjector3() { digitalWrite(pinInjector3, LOW); BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ3); }
  inline void beginCoil3Charge() { digitalWrite(pinCoil3, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil3Charge() { digitalWrite(pinCoil3, coilLOW); TACH_PULSE_HIGH(); }

  inline void openInjector4() { digitalWrite(pinInjector4, HIGH); BIT_SET(currentStatus.squirt, BIT_SQUIRT_INJ4); } //Sets the relevant pin HIGH and changes the current status bit for injector 4 (4th bit of currentStatus.squirt)
  inline void closeInjector4() { digitalWrite(pinInjector4, LOW); BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_INJ4); }
  inline void beginCoil4Charge() { digitalWrite(pinCoil4, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil4Charge() { digitalWrite(pinCoil4, coilLOW); TACH_PULSE_HIGH(); }

  inline void openInjector5() { digitalWrite(pinInjector5, HIGH); }
  inline void closeInjector5() { digitalWrite(pinInjector5, LOW); }
  inline void beginCoil5Charge() { digitalWrite(pinCoil5, coilHIGH); TACH_PULSE_LOW(); }
  inline void endCoil5Charge() { digitalWrite(pinCoil5, coilLOW); TACH_PULSE_HIGH(); }

//#endif


//Combination functions for semi-sequential injection
void openInjector1and4() { digitalWrite(pinInjector1, HIGH); digitalWrite(pinInjector4, HIGH); BIT_SET(currentStatus.squirt, 0); }
void closeInjector1and4() { digitalWrite(pinInjector1, LOW); digitalWrite(pinInjector4, LOW);BIT_CLEAR(currentStatus.squirt, 0); }
void openInjector2and3() { digitalWrite(pinInjector2, HIGH); digitalWrite(pinInjector3, HIGH); BIT_SET(currentStatus.squirt, 1); }
void closeInjector2and3() { digitalWrite(pinInjector2, LOW); digitalWrite(pinInjector3, LOW); BIT_CLEAR(currentStatus.squirt, 1); }
//Below functions are used for 5 cylinder support
void openInjector3and5() { digitalWrite(pinInjector3, HIGH); digitalWrite(pinInjector5, HIGH); BIT_SET(currentStatus.squirt, 0); }
void closeInjector3and5() { digitalWrite(pinInjector3, LOW); digitalWrite(pinInjector5, LOW);BIT_CLEAR(currentStatus.squirt, 0); }

//As above but for ignition (Wasted COP mode)
void beginCoil1and3Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil3, coilHIGH); digitalWrite(pinTachOut, LOW); }
void endCoil1and3Charge() { digitalWrite(pinCoil1, coilLOW); digitalWrite(pinCoil3, coilLOW); }
void beginCoil2and4Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil4, coilHIGH); digitalWrite(pinTachOut, LOW); }
void endCoil2and4Charge() { digitalWrite(pinCoil2, coilLOW); digitalWrite(pinCoil4, coilLOW); }

void nullCallback() { return; }

