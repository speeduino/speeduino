#include "scheduledIO.h"
#include "scheduler.h"
#include "globals.h"
#include "timers.h"

byte coilHIGH = HIGH;
byte coilLOW = LOW;

void beginCoil1Charge() { digitalWrite(pinCoil1, coilHIGH); tachoOutputFlag = READY; }
void endCoil1Charge() { digitalWrite(pinCoil1, coilLOW); }

void beginCoil2Charge() { digitalWrite(pinCoil2, coilHIGH); tachoOutputFlag = READY; }
void endCoil2Charge() { digitalWrite(pinCoil2, coilLOW); }

void beginCoil3Charge() { digitalWrite(pinCoil3, coilHIGH); tachoOutputFlag = READY; }
void endCoil3Charge() { digitalWrite(pinCoil3, coilLOW); }

void beginCoil4Charge() { digitalWrite(pinCoil4, coilHIGH); tachoOutputFlag = READY; }
void endCoil4Charge() { digitalWrite(pinCoil4, coilLOW); }

void beginCoil5Charge() { digitalWrite(pinCoil5, coilHIGH); tachoOutputFlag = READY; }
void endCoil5Charge() { digitalWrite(pinCoil5, coilLOW); }

void beginCoil6Charge() { digitalWrite(pinCoil6, coilHIGH); tachoOutputFlag = READY; }
void endCoil6Charge() { digitalWrite(pinCoil6, coilLOW); }

void beginCoil7Charge() { digitalWrite(pinCoil7, coilHIGH); tachoOutputFlag = READY; }
void endCoil7Charge() { digitalWrite(pinCoil7, coilLOW); }

void beginCoil8Charge() { digitalWrite(pinCoil8, coilHIGH); tachoOutputFlag = READY; }
void endCoil8Charge() { digitalWrite(pinCoil8, coilLOW); }

//The below 3 calls are all part of the rotary ignition mode
void beginTrailingCoilCharge() { digitalWrite(pinCoil2, coilHIGH); }
void endTrailingCoilCharge1() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port |= ign3_pin_mask; } //Sets ign3 (Trailing select) high
void endTrailingCoilCharge2() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port &= ~(ign3_pin_mask); } //sets ign3 (Trailing select) low

//As above but for ignition (Wasted COP mode)
void beginCoil1and3Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil3, coilHIGH); tachoOutputFlag = READY; }
void endCoil1and3Charge()   { digitalWrite(pinCoil1, coilLOW);  digitalWrite(pinCoil3, coilLOW);  }
void beginCoil2and4Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil4, coilHIGH); tachoOutputFlag = READY; }
void endCoil2and4Charge()   { digitalWrite(pinCoil2, coilLOW);  digitalWrite(pinCoil4, coilLOW);  }

void nullCallback() { return; }
