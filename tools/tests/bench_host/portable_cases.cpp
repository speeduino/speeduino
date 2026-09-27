uint8_t packet[64];
uint8_t command(uint8_t op,uint16_t length=2) {
  packet[0]='N';packet[1]=op;injectorBenchCommand(packet,length);return packet[0];
}
uint8_t arm(uint8_t kind) {packet[2]=kind;return command(10,3);}
void settings(bool coil,uint16_t channel) {
  uint8_t *p=staging+(coil ? 8 : 0);
  injector_bench::write16(p,channel);
  injector_bench::write16(p+2,1000);
  injector_bench::write16(p+4,10000);
  injector_bench::write16(p+6,2);
}
int main() {
  using namespace injector_bench;
  Schedule *fuel[]={&fuelSchedule1,&fuelSchedule2,&fuelSchedule3,&fuelSchedule4,&fuelSchedule5,&fuelSchedule6,&fuelSchedule7,&fuelSchedule8};
  Schedule *ign[]={&ignitionSchedule1,&ignitionSchedule2,&ignitionSchedule3,&ignitionSchedule4,&ignitionSchedule5,&ignitionSchedule6,&ignitionSchedule7,&ignitionSchedule8};
  void (*inj[])()={openInjector1,openInjector2,openInjector3,openInjector4,openInjector5,openInjector6,openInjector7,openInjector8};
  void (*coil[])()={beginCoil1Charge,beginCoil2Charge,beginCoil3Charge,beginCoil4Charge,beginCoil5Charge,beginCoil6Charge,beginCoil7Charge,beginCoil8Charge};
  for(unsigned i=0;i<8;++i) {fuel[i]->_pStartCallback=inj[i];ign[i]->_pStartCallback=coil[i];}
  for(bool ignition: {false,true}) {
    const unsigned count=ignition ? IGN_CHANNELS : INJ_CHANNELS;
    assert(arm(ignition ? 2 : 1)==0);
    settings(ignition,0);assert(command(ignition ? 7 : 1)==0);
    assert(pulseMask==uint8_t((1U<<count)-1U));
    timer.CNT=timer.compare;
    assert(injectorBenchHandleTimer(fuelSchedule1));
    for(unsigned i=0;i<8;++i) assert((ignition ? coilLevels[i] : levels[i])==(i<count));
    assert(command(0)==0);
    for(unsigned i=0;i<8;++i) assert(!coilLevels[i] && !levels[i]);
    assert(!injectorBenchHandleTimer(fuelSchedule1));
    if(count<8) {
      assert(arm(ignition ? 2 : 1)==0);
      settings(ignition,count+1);assert(command(ignition ? 7 : 1)==0x84);
      assert(command(0)==0);
    }
  }
  // All omits missing physical pins; an explicit missing selection is rejected.
  pinNumbers.injectorPins[INJ_CHANNELS-1]=NOT_A_PIN;
  assert(arm(1)==0);settings(false,0);assert(command(1)==0);
  assert(pulseMask==uint8_t((1U<<(INJ_CHANNELS-1))-1U));assert(command(0)==0);
  assert(arm(1)==0);settings(false,INJ_CHANNELS);assert(command(1)==0x84);assert(command(0)==0);
  pinNumbers.injectorPins[INJ_CHANNELS-1]=INJ_CHANNELS;
  // Trigger callbacks follow the chosen board, including a tertiary trigger.
  pinTrigger=70;pinTrigger2=71;pinTrigger3=72;
  currentStatus.decoder.tertiary.valid=true;
  assert(arm(1)==0);
  for(unsigned pin=70;pin<=72;++pin) assert(triggerCallbacks[pin]==triggerAbort && triggerEdges[pin]==CHANGE);
  triggerCallbacks[72]();assert(reason==2 && !enabledKind);
  assert(command(0)==0);
  for(unsigned pin=70;pin<=72;++pin) assert(triggerCallbacks[pin]==normalTrigger && triggerEdges[pin]==2);
  currentStatus.decoder.primary.valid=false;assert(arm(1)==0x84);
  currentStatus.decoder.primary.valid=true;
  // Compare deadlines crossing the 16-bit counter boundary still produce one edge.
  timer.CNT=65530;assert(arm(1)==0);settings(false,1);assert(command(1)==0);
  assert(timer.compare==uint16_t(65530U+1000U));timer.CNT=timer.compare;
  injectorBenchHandleTimer(fuelSchedule1);assert(levels[0]);assert(command(0)==0);
#ifdef MC33810_SUPPORT
  pinNumbers.pinMC33810_1_CS=110;pinNumbers.pinMC33810_2_CS=111;
  assert(arm(2)==0);settings(true,0);assert(command(7)==0);
  timer.CNT=timer.compare;injectorBenchHandleTimer(fuelSchedule1);
  for(unsigned i=0;i<IGN_CHANNELS;++i) assert(coilLevels[i]);
  assert(command(0)==0);
  pinNumbers.pinMC33810_2_CS=NOT_A_PIN;
  assert(arm(2)==0);settings(true,1);assert(command(7)==0);assert(command(0)==0);
  pinNumbers.pinMC33810_2_CS=111;
  assert(arm(3)==0);pinFan=111;packet[2]=1;packet[3]=1;
  assert(command(6,4)==0x84);assert(command(0)==0);pinFan=31;
  if(IGN_CHANNELS>1) {
    pinNumbers.mc33810IgnBits[1]=pinNumbers.mc33810IgnBits[0];
    assert(arm(2)==0);settings(true,0);assert(command(7)==0x84);assert(command(0)==0);
  }
  // AUX must never claim SPI bus pins even when their engine channels are unused.
  currentStatus.injOutputs.primary=1;currentStatus.maxIgnOutputs=1;pinFan=2;
  assert(arm(3)==0);packet[2]=1;packet[3]=1;assert(command(6,4)==0x84);assert(command(0)==0);
#endif
  printf("PASS: portable timer/trigger/selection checks (%u injectors, %u coils)\n",INJ_CHANNELS,IGN_CHANNELS);
}
