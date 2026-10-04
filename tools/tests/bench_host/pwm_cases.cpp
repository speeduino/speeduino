// Normal PWM must keep its phase/compare schedule while writes to claimed pins
// are blocked. Exercise the actual Fan/Boost ISR bodies and the real ownership API.
static void testNormalPwmDuringBenchWithPolarity(bool inverted) {
 boostOutput.pin.setPin(pinBoost, OUTPUT);
 boostOutput.maxDuty=1000;
 boostOutput.setTargetDuty(80); // 400 ticks on, 600 ticks off.
 _fanPwm.pin.setPin(pinFan, OUTPUT);
 _fanPwm.pin.setInverted(inverted);
 _fanPwm.maxDuty=1000;
 _fanPwm.setTargetDuty(0);
 _fanPwm.setTargetDuty(80);
 assert(arm(3)==0 && aux(1,1)==0 && aux(2,1)==0);
 for(unsigned i=0;i<4;++i) {
  boostCounter=boostCompare;fanCounter=fanCompare;
  boostInterrupt();fanInterrupt();
  assert(boostCompare==uint16_t(boostCounter+400));
  assert(fanCompare==uint16_t(fanCounter+400));
  assert(boostOutput.pin.isPinHigh() && _fanPwm.pin.isPinHigh());
  assert(gpio[pinBoost] && gpio[pinFan]);
  boostCounter=boostCompare;fanCounter=fanCompare;
  boostInterrupt();fanInterrupt();
  assert(boostCompare==uint16_t(boostCounter+600));
  assert(fanCompare==uint16_t(fanCounter+600));
  assert(boostOutput.pin.isPinLow() && !_fanPwm.pin.isPinHigh());
  assert(gpio[pinBoost] && gpio[pinFan]); // The tester still owns the high level.
 }
 // Off retains ownership too, including direct 100% / 0% controller writes.
 assert(aux(1,0)==0 && aux(2,0)==0);
 boostOutput.setTargetDuty(200);_fanPwm.setTargetDuty(200);
 assert(!gpio[pinBoost] && !gpio[pinFan]);
 boostOutput.setTargetDuty(0);_fanPwm.setTargetDuty(0);
 assert(!gpio[pinBoost] && !gpio[pinFan]);
 // A non-claimed controller continues driving its output during the same test.
 boostOutput.pin.setPin(100, OUTPUT);
 boostOutput.setTargetDuty(200);assert(gpio[100]);
 boostOutput.setTargetDuty(0);assert(!gpio[100]);
 boostOutput.pin.setPin(pinBoost, OUTPUT);
 boostOutput.setTargetDuty(80);
 _fanPwm.setTargetDuty(80);
 boostInterrupt();fanInterrupt();
 assert(!gpio[pinBoost] && !gpio[pinFan]);
 stop();
 // Release restores physical writes without reinitialising the controller pins.
 boostOutput.setTargetDuty(200);_fanPwm.setTargetDuty(200);
 assert(gpio[pinBoost] && gpio[pinFan]==!inverted);
 boostOutput.setTargetDuty(0);_fanPwm.setTargetDuty(0);
 assert(!gpio[pinBoost] && gpio[pinFan]==inverted);
}

void testNormalPwmDuringBench() {
 testNormalPwmDuringBenchWithPolarity(false);
 testNormalPwmDuringBenchWithPolarity(true);
}
