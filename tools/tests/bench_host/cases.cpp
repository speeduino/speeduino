void fireEdge() {timer.CNT=timer.compare;edgeISR();}
struct MockOutputPin {
 uint8_t pin=0;
 void setPin(uint8_t p,uint8_t) {pin=p;}
 void setPinHigh() {digitalWrite(pin,1);}
 void setPinLow() {digitalWrite(pin,0);}
};
uint8_t packet[64];
uint16_t cmd(uint8_t op,uint16_t n=2) {packet[0]='N';packet[1]=op;return injectorBenchCommand(packet,n);}
uint8_t arm(uint8_t kind) {packet[2]=kind;cmd(10,3);return packet[0];}
uint8_t pump(bool on) {packet[2]=on;cmd(8,3);return packet[0];}
uint8_t aux(uint8_t target,uint8_t mode) {packet[2]=target;packet[3]=mode;cmd(6,4);return packet[0];}
uint8_t idle(uint8_t mode) {packet[2]=mode;cmd(9,3);return packet[0];}
void stop() {cmd(0);assert(packet[0]==0 && !owned && !enabledKind);for(bool x:levels)assert(!x);for(bool x:coilLevels)assert(!x);}
void writeValue(uint16_t offset,uint16_t value) {
 injector_bench::write16(packet+2,offset);injector_bench::write16(packet+4,2);
 injector_bench::write16(packet+6,value);cmd(4,8);assert(packet[0]==0);
}
void configure(bool coil,uint16_t channel,uint16_t on,uint32_t period,uint16_t count) {
 uint16_t offset=coil ? 8 : 0;
 writeValue(offset,channel);writeValue(offset+2,on);writeValue(offset+4,period/10U);writeValue(offset+6,count);
}
void ticks(unsigned count,bool keep=true) {
 for(unsigned i=0;i<count;++i) {if(keep && i%500==0)injectorBenchKeepAlive();injectorBenchTick();}
}
void allIdle() {for(bool x:levels)assert(!x);for(bool x:coilLevels)assert(!x);}
int main() {
 using namespace injector_bench;
 // Reboot defaults match the example injector and coil bench settings.
 const uint16_t defaults[]={0,1000,10000,100,0,2500,10000,100};
 for(unsigned i=0;i<8;++i) assert(read16(staging+2*i)==defaults[i]);
 Schedule *fuelSchedules[]={&fuelSchedule1,&fuelSchedule2,&fuelSchedule3,&fuelSchedule4,&fuelSchedule5,&fuelSchedule6,&fuelSchedule7,&fuelSchedule8};
 Schedule *ignSchedules[]={&ignitionSchedule1,&ignitionSchedule2,&ignitionSchedule3,&ignitionSchedule4,&ignitionSchedule5,&ignitionSchedule6,&ignitionSchedule7,&ignitionSchedule8};
 void (*fuelCallbacks[])()={openInjector1,openInjector2,openInjector3,openInjector4,openInjector5,openInjector6,openInjector7,openInjector8};
 void (*ignCallbacks[])()={beginCoil1Charge,beginCoil2Charge,beginCoil3Charge,beginCoil4Charge,beginCoil5Charge,beginCoil6Charge,beginCoil7Charge,beginCoil8Charge};
 for(unsigned i=0;i<8;++i) {fuelSchedules[i]->_pStartCallback=fuelCallbacks[i];ignSchedules[i]->_pStartCallback=ignCallbacks[i];}
 for(uint8_t board: {uint8_t(0),uint8_t(14),uint8_t(60)}) {configPage2.pinMapping=board;assert(arm(1)==0);stop();}
 testNormalPwmDuringBench();
 // Nothing can drive outputs until an explicit mode-specific Enable.
 cmd(1);assert(packet[0]==0x85);cmd(7);assert(packet[0]==0x85);
 assert(pump(true)==0x85 && aux(0,1)==0x85 && idle(0)==0x85);allIdle();
 assert(arm(0)==0x84 && arm(5)==0x84);
 currentStatus.RPM=1;assert(arm(1)==0x85);currentStatus.RPM=0;
 engineRunning=true;assert(arm(1)==0x85);engineRunning=false;
 ignitionSchedule8._status=PENDING;assert(arm(1)==0x85);ignitionSchedule8._status=OFF;
 currentStatus.decoder.status.syncStatus=SyncStatus::Full;assert(arm(1)==0x85);currentStatus.decoder.status.syncStatus=SyncStatus::None;
 currentStatus.toothLogEnabled=true;assert(arm(1)==0x85);currentStatus.toothLogEnabled=false;
 // Pin alias admission and settings bounds are enforced before a pulse.
 assert(arm(1)==0);configure(false,1,1000,10000,2);
 pinFan=1;cmd(1);assert(packet[0]==0x84);pinFan=21;
 configure(false,10,1000,10000,2);cmd(1);assert(packet[0]==0x84);
 configure(false,1,1000,1000,2);cmd(1);assert(packet[0]==0x84);
 configure(false,1,1000,1099,2);cmd(1);assert(packet[0]==0x84);
 configure(false,1,20000,60000,65535);cmd(1);assert(packet[0]==0x84);stop();
 // Both single and All modes execute exactly count pulses per selected channel.
 const bool kinds[]={false,true}; for(bool coil:kinds) for(uint8_t channel=0;channel<=8;++channel) {
   assert(arm(coil ? 2 : 1)==0);
   configure(coil,channel,1000,10000,3);
   const unsigned before=coil ? chargesSeen[0] : opensSeen[0];
   cmd(coil ? 7 : 1);assert(packet[0]==0);
   for(unsigned i=0;i<3;++i) {
     fireEdge();assert(timer.ARR==999);
     for(unsigned n=0;n<8;++n) {
       assert((coil ? coilLevels[n] : levels[n])==(channel==0 || channel==n+1));
       assert(!(coil ? levels[n] : coilLevels[n]));
     }
     fireEdge();allIdle();if(i<2)assert(timer.ARR==8999);
   }
   assert(sequence.completed==3 && sequence.state==State::Complete);
   assert((coil ? chargesSeen[0] : opensSeen[0])==before+((channel==0 || channel==1) ? 3 : 0));
   stop();
 }
 // Staging writes never change a running test. Stop keeps enabled, Disable releases.
 assert(arm(1)==0);configure(false,1,750,5750,2);cmd(1);fireEdge();assert(levels[0]);
 configure(false,8,400,20400,3);assert(sequence.settings.channel==1 && sequence.settings.onUs==750);
 assert(arm(2)==0x85);cmd(1);assert(packet[0]==0x85);
 cmd(11);allIdle();assert(owned && enabledKind==1);
 cmd(1);assert(packet[0]==0);fireEdge();assert(levels[7] && timer.ARR==399);stop();
 assert(arm(2)==0);configure(true,1,5001,20000,1);cmd(7);assert(packet[0]==0x84);
 configure(true,1,1000,9999,1);cmd(7);assert(packet[0]==0x84);stop();
 // Faults shut all selected outputs down and disarm actions until Disable.
 assert(arm(2)==0);configure(true,0,2000,20000,2);cmd(7);fireEdge();triggerCallbacks[pinTrigger2]();
 allIdle();assert(!enabledKind && reason==2 && owned);cmd(7);assert(packet[0]==0x85);stop();
 assert(arm(1)==0);configure(false,0,750,5750,2);cmd(1);fireEdge();ticks(3);
 assert(reason==4 && !enabledKind);allIdle();stop();
 assert(arm(1)==0);cmd(1);timer.CNT=uint16_t(timer.compare+101);edgeISR();assert(reason==3);timer.CNT=0;stop();
 // Pump before/during pulses, completion and explicit Stop/Disable.
 assert(arm(1)==0 && pump(true)==0 && gpio[20]);
 configure(false,1,1000,10000,1);cmd(1);fireEdge();assert(gpio[20] && levels[0]);
 assert(pump(false)==0 && !gpio[20]);assert(pump(true)==0);fireEdge();assert(!gpio[20]);
 assert(pump(true)==0);stop();assert(!gpio[20]);
 assert(arm(1)==0 && pump(true)==0);ticks(2000,false);assert(reason==1 && !gpio[20] && !enabledKind);stop();
 // AUX permits independent simultaneous channels, never coil/injector tests.
 assert(arm(3)==0);assert(aux(0,2)==0x84 && aux(6,2)==0x84 && aux(7,2)==0x84);
 assert(aux(0,1)==0 && aux(2,1)==0 && aux(6,1)==0);
 assert(gpio[20] && gpio[22] && gpio[34]);
 assert(aux(2,0)==0 && !gpio[22] && gpio[20] && gpio[34]);
 writeValue(22,1000);assert(aux(3,2)==0 && aux(4,2)==0);assert(gpio[23] && gpio[24]);
 ticks(5);assert(!gpio[23] && !gpio[24]);ticks(5);assert(gpio[23] && gpio[24]);
 assert(aux(3,0)==0);
 ticks(31000);assert(enabledKind==3 && gpio[20] && gpio[34]); // no 30-second cutoff
 cmd(1);assert(packet[0]==0x85);stop();assert(!gpio[20] && !gpio[34] && !gpio[24]);
 // Shared frequency is fractional-capable. At 11.1 Hz: 222 half-period transitions / 10s.
 assert(arm(3)==0);writeValue(22,111);assert(aux(2,2)==0);
 unsigned transitions=0;
 for(unsigned i=0;i<10000;++i) {bool before=gpio[22];ticks(1);if(before!=gpio[22])++transitions;}
 assert(transitions==222);writeValue(22,0);assert(aux(2,2)==0x84);stop();
 configPage6.fanInv=1;configPage15.airConCompPol=1;
 assert(arm(3)==0 && aux(1,1)==0 && aux(6,1)==0);assert(!gpio[21] && !gpio[34]);
 triggerCallbacks[pinTrigger]();assert(gpio[21] && gpio[34] && !enabledKind);stop();
 configPage6.fanInv=0;configPage15.airConCompPol=0;
 assert(arm(3)==0);assert(aux(0,1)==0 && aux(2,1)==0);ticks(2000,false);
 assert(!gpio[20] && !gpio[22] && reason==1);stop();
 // Actual reported stepper regression: disabled table-switch pin aliases DIR.
 configPage6.iacAlgorithm=4;writeValue(16,3);writeValue(18,2);writeValue(20,25);
 pinFuel2Input=pinStepperDir;configPage10.fuel2Mode=0;
 assert(arm(4)==0);assert(currentStatus.idleLoad==0);assert(idle(1)==0);
 unsigned stepEdges=0;
 for(unsigned i=0;i<40 && auxRunning;++i) {
   bool before=gpio[pinStepperStep];ticks(1);
   if(!before && gpio[pinStepperStep]) {assert(gpio[pinStepperDir]==(stepEdges<3));++stepEdges;}
 }
 assert(stepEdges==5 && auxComplete && currentStatus.idleLoad==2);
 stop();assert(restoredPositionKnown && restoredPosition==2);
 configPage10.fuel2Mode=4;assert(arm(4)==0);assert(idle(1)==0x84);stop();
 configPage10.fuel2Mode=0;pinFuel2Input=51;
 // Home-only works independently of an invalid run-position value.
 assert(arm(4)==0);writeValue(18,200);assert(idle(0)==0);ticks(20);assert(auxComplete);stop();writeValue(18,2);
 assert(arm(4)==0);assert(idle(1)==0);ticks(2);assert(gpio[pinStepperStep]);
 triggerCallbacks[pinTrigger]();assert(!gpio[pinStepperStep] && gpio[pinStepperEnable]);stop();assert(!restoredPositionKnown);
 // Both complementary PWM outputs obey test duty and are de-energized on Disable.
 configPage6.iacAlgorithm=2;configPage6.iacChannels=1;
 assert(arm(4)==0 && idle(1)==0);fireEdge();assert(gpio[25] && !gpio[26] && timer.ARR==2499);
 fireEdge();assert(!gpio[25] && gpio[26] && timer.ARR==7499);
 writeValue(20,80);fireEdge();assert(timer.ARR==2499);stop();assert(!gpio[25] && !gpio[26]);
 configPage6.iacChannels=0;
 // Long periods must split into timer chunks without extra GPIO edges.
 const uint32_t longPeriods[]={65000UL,120000UL,240000UL,500000UL,60800UL,60810UL}; for(uint32_t period: longPeriods) {
   assert(arm(1)==0);configure(false,1,800,period,2);cmd(1);assert(packet[0]==0);
   fireEdge();assert(levels[0]);fireEdge();assert(!levels[0]);
   uint32_t elapsed=timer.ARR+1;
   while(remainingDelayUs) {fireEdge();assert(!levels[0]);elapsed+=timer.ARR+1;}
   assert(elapsed==period-800);fireEdge();assert(levels[0]);fireEdge();
   assert(sequence.completed==2);stop();
 }
 assert(arm(1)==0);configure(false,1,800,500010,2);cmd(1);assert(packet[0]==0x84);stop();
 // Watchdog must accept every long-delay chunk and still stop on a missed one.
 assert(arm(1)==0);configure(false,1,800,240000,2);cmd(1);fireEdge();fireEdge();
 while(remainingDelayUs) {ticks((timer.ARR+1)/1000);assert(!reason && !levels[0]);fireEdge();}
 ticks((timer.ARR+1)/1000);assert(!reason);fireEdge();assert(levels[0]);stop();
 assert(arm(1)==0);cmd(1);fireEdge();fireEdge();ticks(63);assert(reason==4);stop();
 assert(arm(1)==0);configure(false,1,800,500000,241);cmd(1);assert(packet[0]==0x84);stop();
 // VVT ownership guard blocks physical writes without stopping the other channel.
 BenchOutputPin<MockOutputPin> guarded,other;
 guarded.setPin(23,OUTPUT);other.setPin(24,OUTPUT);
 assert(arm(3)==0 && aux(3,1)==0);guarded.setPinLow();assert(gpio[23]);
 other.setPinHigh();assert(gpio[24]);other.setPinLow();assert(!gpio[24]);
 assert(aux(3,0)==0);guarded.setPinHigh();assert(!gpio[23]);stop();
 guarded.setPinHigh();assert(gpio[23]);guarded.setPinLow();
 // Disabled tune features are rejected by the ECU, not only by menu conditions.
 assert(arm(3)==0);
 configPage2.fanEnable=0;assert(aux(1,1)==0x84 && aux(1,2)==0x84);configPage2.fanEnable=1;
 configPage6.boostEnabled=0;assert(aux(2,1)==0x84);configPage6.boostEnabled=1;
 configPage6.vvtEnabled=0;assert(aux(3,1)==0x84 && aux(4,1)==0x84);configPage6.vvtEnabled=1;
 configPage10.vvt2Enabled=0;assert(aux(4,1)==0x84);
 configPage15.airConEnable=0;assert(aux(6,1)==0x84 && aux(7,1)==0x84);configPage15.airConEnable=1;
 configPage15.airConFanEnabled=0;assert(aux(7,1)==0x84);configPage15.airConFanEnabled=1;
 assert(aux(5,1)==0x84);stop();
 // WMI drives PWM and its enable relay together, without affecting another AUX output.
 configPage10.wmiEnabled=1;writeValue(22,1000);
 assert(arm(3)==0 && aux(0,1)==0 && aux(5,2)==0);
 assert(gpio[48] && injectorBenchOwnsPin(48) && injectorBenchOwnsPin(24));
 bool phase=gpio[24];ticks(5);assert(gpio[24]!=phase && gpio[48]);
 assert(aux(4,1)==0x84);assert(aux(5,0)==0 && !gpio[24] && !gpio[48] && gpio[20]);
 assert(aux(5,1)==0 && gpio[24] && gpio[48]);triggerCallbacks[pinTrigger]();assert(!gpio[24] && !gpio[48]);stop();
 assert(arm(3)==0 && aux(5,1)==0);ticks(2000,false);assert(!gpio[24] && !gpio[48]);stop();
 assert(arm(3)==0);pinWMIEnabled=pinVVT_2;assert(aux(5,1)==0x84);pinWMIEnabled=48;stop();
 configPage10.vvt2Enabled=1;assert(arm(3)==0);assert(aux(4,1)==0x84 && aux(5,1)==0x84);stop();
 configPage10.wmiEnabled=0;
 for(uint8_t algorithm=0;algorithm<2;++algorithm) {
   configPage6.iacAlgorithm=algorithm;assert(arm(4)==0x84 && idle(1)==0x84);
 }
 configPage6.iacAlgorithm=2;
 // Repurposed injector8 is reserved for fan, but usable via AUX if engine uses only 1-4.
 currentStatus.injOutputs.primary=4;pinFan=8;configPage2.fanEnable=1;
 assert(arm(1)==0);configure(false,8,800,10000,2);cmd(1);assert(packet[0]==0x84);
 configure(false,0,800,10000,2);cmd(1);assert(packet[0]==0x84);allIdle();
 configure(false,9,800,10000,2);cmd(1);assert(packet[0]==0 && pulseMask==15);fireEdge();
 for(unsigned i=0;i<8;++i) {assert(levels[i]==(i<4));}
 stop();
 assert(arm(3)==0 && aux(1,1)==0 && gpio[8]);stop();assert(!gpio[8]);
 currentStatus.injOutputs.primary=8;assert(arm(3)==0);assert(aux(1,1)==0x84);stop();pinFan=21;
 // The same protection applies to repurposed unused ignition outputs.
 currentStatus.maxIgnOutputs=2;pinBoost=18;
 assert(arm(2)==0);configure(true,8,2000,20000,2);cmd(7);assert(packet[0]==0x84);
 configure(true,0,2000,20000,2);cmd(7);assert(packet[0]==0x84);
 configure(true,9,2000,20000,2);cmd(7);assert(packet[0]==0 && pulseMask==3);stop();
 assert(arm(3)==0 && aux(2,1)==0);stop();pinBoost=22;
 // Resolve physical outputs from routing, not cylinder/scheduler count alone.
 uint8_t actual=0;
 currentStatus.injOutputs.primary=2;fuelSchedule1._pStartCallback=openInjector1and3;fuelSchedule2._pStartCallback=openInjector2and4;
 assert(configuredMask(false,actual) && actual==15);
 currentStatus.injOutputs.primary=4;fuelSchedule1._pStartCallback=openInjector1;fuelSchedule2._pStartCallback=openInjector2;fuelSchedule3._pStartCallback=openInjector3and5;
 assert(configuredMask(false,actual) && actual==31);
 fuelSchedule3._pStartCallback=openInjector3;currentStatus.injOutputs.secondary=4;assert(configuredMask(false,actual) && actual==255);
 currentStatus.injOutputs.secondary=0;currentStatus.injOutputs.primary=8;
 ignitionSchedule1._pStartCallback=beginCoil1and3Charge;ignitionSchedule2._pStartCallback=beginCoil2and4Charge;
 assert(configuredMask(true,actual) && actual==15);
 ignitionSchedule1._pStartCallback=beginCoil1Charge;ignitionSchedule2._pStartCallback=beginCoil1Charge;
 assert(configuredMask(true,actual) && actual==1);
 ignitionSchedule2._pStartCallback=nullptr;assert(!configuredMask(true,actual));
 for(unsigned i=0;i<8;++i) {ignSchedules[i]->_pStartCallback=ignCallbacks[i];}
 currentStatus.maxIgnOutputs=8;
 // Previously omitted auxiliary/programmable assignments block raw injector tests too.
 pinAirConComp=8;assert(arm(1)==0);configure(false,0,800,10000,2);cmd(1);assert(packet[0]==0x84);stop();pinAirConComp=34;
 configPage13.outputPin[0]=8;assert(arm(1)==0);cmd(1);assert(packet[0]==0x84);stop();configPage13.outputPin[0]=0;
 // Idle1 and Idle2 are independent PWM outputs; disabling one leaves the other running.
 configPage6.iacAlgorithm=2;configPage6.iacChannels=0;assert(arm(3)==0 && aux(9,1)==0x84);stop();
 configPage6.iacChannels=1;assert(arm(3)==0 && aux(8,1)==0 && aux(9,2)==0);
 assert(gpio[25] && injectorBenchOwnsPin(26));bool idlePhase=gpio[26];ticks(5);assert(gpio[25] && gpio[26]!=idlePhase);
 assert(aux(8,0)==0 && !gpio[25]);assert(auxPwmMask & (1U<<9));
 triggerCallbacks[pinTrigger]();assert(!gpio[25] && !gpio[26]);stop();
 configPage6.iacAlgorithm=4;assert(arm(3)==0 && aux(8,1)==0x84 && aux(9,1)==0x84);stop();
 configPage6.iacAlgorithm=2;configPage6.iacChannels=0;
 // Even a stale normal PWM ISR cannot overwrite an admitted injector/coil pin.
 assert(arm(1)==0);configure(false,1,800,10000,2);cmd(1);
 assert(injectorBenchOwnsPin(1) && !injectorBenchOwnsPin(2));
 BenchOutputPin<MockOutputPin> stalePwm;stalePwm.setPin(1,OUTPUT);
 gpio[1]=false;stalePwm.setPinHigh();assert(!gpio[1]);stop();
 // Input assignments cannot be driven by AUX or raw injector/coil tests.
 configPage10.knock_mode=KNOCK_MODE_DIGITAL;configPage10.knock_pin=8;
 assert(arm(1)==0);configure(false,8,1000,100000,100);cmd(1);assert(packet[0]==0x84);stop();
 configPage10.knock_pin=PD3;assert(arm(1)==0x84);configPage10.knock_mode=0;
 configPage9.caninput_sel[0]=3;configPage9.Auxinpinb[0]=pinBoost-1;
 assert(arm(3)==0 && aux(2,1)==0x84);stop();
 configPage9.enable_secondarySerial=1; // External bank is off, local bank is inactive.
 assert(arm(3)==0 && aux(2,1)==0);stop();
 configPage9.caninput_sel[0]=12;assert(arm(3)==0 && aux(2,1)==0x84);stop();
 configPage9.caninput_sel[0]=8;configPage9.Auxinpina[0]=1;pinBoost=65;
 assert(arm(3)==0 && aux(2,1)==0x84);stop();
 configPage9.caninput_sel[0]=0;configPage9.enable_secondarySerial=0;
 configPage10.knock_mode=KNOCK_MODE_ANALOG;configPage10.knock_pin=48;
 assert(arm(3)==0 && aux(2,1)==0x84);stop();configPage10.knock_mode=0;
 pinNumbers.pinO2_2=65;assert(arm(3)==0 && aux(2,1)==0x84);stop();pinNumbers.pinO2_2=127;pinBoost=22;
 // CRC-framed protocol bounds and full extended page.
 write16(packet+2,23);write16(packet+4,2);cmd(3,6);assert(packet[0]==0x84);
 write16(packet+2,65535);write16(packet+4,2);cmd(3,6);assert(packet[0]==0x84);
 write16(packet+2,0);write16(packet+4,24);assert(cmd(3,6)==25 && packet[0]==0);
 cmd(2);assert(packet[1]==8);mask=1;stop();assert(mask==1);mask=0;
 puts("PASS: explicit Enable, single/All pulses, multi-output AUX and frequency, independent Off, faults, pump, idle pin regression, homing and dual PWM");
}
