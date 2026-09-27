#pragma once
#include <stdint.h>

namespace output_bench {
// Open-loop step count: no position sensor. DIR gets a full millisecond of
// setup before STEP rises. Pulse/cooling times come from the idle tune.
struct StepperTest {
  enum Phase : uint8_t { Ready, Setup, High, Cooling, Hold };
  Phase phase=Ready;
  uint16_t homeLeft=0, position=0, target=0, runTarget=0, waitMs=0;
  uint8_t mode=0, pulseMs=1, coolMs=1;
  bool running=false, homed=false, stepHigh=false, closing=true;
  void start(uint16_t home,uint16_t run,uint8_t action,uint8_t pulse,uint8_t cool) {
    homeLeft=home; position=0; target=run; runTarget=run; mode=action;
    pulseMs=pulse; coolMs=cool ? cool : 1; phase=Ready; waitMs=0;
    running=true; homed=false; stepHigh=false; closing=true;
  }
  void tick() {
    if(!running) return;
    if(waitMs && --waitMs) return;
    if(phase==Setup) {stepHigh=true; phase=High; waitMs=pulseMs; return;}
    if(phase==High) {
      stepHigh=false;
      if(homeLeft) --homeLeft;
      else if(closing) --position;
      else ++position;
      phase=Cooling; waitMs=coolMs; return;
    }
    if(phase==Hold) {target=target==0 ? runTarget : 0; phase=Ready;}
    if(!homeLeft) {
      homed=true;
      if(mode==0 || position==target) {
        if(mode!=2) {running=false;return;}
        if(phase!=Ready) {phase=Hold;waitMs=1000;return;}
        // Also handle a zero run target without issuing any spurious steps.
        if(position==target) {phase=Hold;waitMs=1000;return;}
      }
    }
    closing=homeLeft || position>target;
    phase=Setup; waitMs=1;
  }
  bool positionKnown() const { return homed && !stepHigh; }
};
}
