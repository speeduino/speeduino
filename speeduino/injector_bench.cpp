#include "injector_bench.h"
#include "globals.h"
#include "injector_bench_logic.h"
#include "idle_bench_logic.h"
#include "idle.h"
#include "scheduler.h"
#include "scheduledIO_inj.h"
#include "scheduledIO_ign.h"
#include "scheduler_fuel_controller.h"
#include "scheduler_ignition_controller.h"
#include "decoder_init.h"
#include "decoders.h"
#include "init.h"
#include "src/pins/pinMapping.h"
#include <FastCRC.h>
#include "atomic.h"

static volatile uint8_t benchTelemetry=0;
uint8_t injectorBenchTelemetry() { return benchTelemetry; }
namespace {
using namespace injector_bench;
bool timerActive=false;
Sequence sequence;
volatile bool owned = false;
volatile uint16_t leaseMs = 0;
volatile uint16_t edgeBudgetMs = 0;
uint32_t remainingDelayUs=0;
volatile uint8_t reason = 0; // 0=none/stop, 1=link timeout, 2=trigger, 3=late ISR, 4=timer stalled, 5=pump time limit
// Two independent RAM settings blocks: injector, then coil (channel/on/period/count).
uint8_t staging[24] = {0,0, 0xE8,0x03, 0x10,0x27, 100,0, 0,0, 0xC4,0x09, 0x10,0x27, 100,0, 250,0, 0,0, 50,0, 0xE8,0x03};
// Enable is explicit and selects exactly one tester: injector/coil/aux/idle.
uint8_t enabledKind=0;
uint8_t pulseMask=0;
constexpr uint8_t AUX_COUNT=10, NO_AUX=255, WMI_ENABLE_TARGET=254;
uint16_t auxClaimed=0, auxOnMask=0, auxPwmMask=0, auxHighMask=0;
uint8_t auxPins[AUX_COUNT]={};
bool wmiOwned=false;
uint8_t wmiEnablePin=0;
uint16_t auxFrequency10=1000, auxPhase=0;
bool auxPhaseHigh=true;
bool coilSession=false;
bool pumpOwned=false, pumpLevel=false;
uint8_t pumpPin=0;
uint32_t pumpRemainingMs=0;
bool auxSession=false, auxRunning=false, auxComplete=false;
uint16_t auxRemainingMs=0;
bool idleSession=false, idleIsStepper=false, idleTwoPins=false, idlePhaseOn=false;
uint8_t idleMode=0, idleDuty=0, idleCurrentDuty=0, idleActiveLevel=1;
uint16_t idleCycleMs=1000;
bool idleCycleHigh=true;
output_bench::StepperTest idleSteps;
void idleOff() {
  if(idleIsStepper) {digitalWrite(pinNumbers.pinStepperStep,LOW);digitalWrite(pinNumbers.pinStepperEnable,HIGH);}
  else {
    // iacPWMdir reverses valve action,
    // not electrical shutdown: both coils must be LOW when the test stops.
    digitalWrite(pinNumbers.pinIdle1,LOW);
    if(idleTwoPins) digitalWrite(pinNumbers.pinIdle2,LOW);
  }
}
void idlePwmWrite(bool on) {
  digitalWrite(pinNumbers.pinIdle1,on ? idleActiveLevel : !idleActiveLevel);
  if(idleTwoPins) digitalWrite(pinNumbers.pinIdle2,on ? !idleActiveLevel : idleActiveLevel);
}
const uint8_t RC_OK=0, RC_RANGE=0x84, RC_BUSY=0x85;
// Preserve interrupt state in main-loop and ISR callers on each platform.
struct Lock {
#if defined(NATIVE_BOARD)
  TickEventGuard guard;
#elif defined(__AVR__)
  uint8_t mask;
  Lock() : mask(SREG) { cli(); }
  ~Lock() { SREG=mask; }
#else
  uint32_t mask;
  Lock() {
#if defined(CORE_TEENSY)
    __asm__ volatile("mrs %0, primask" : "=r" (mask) :: "memory");
#else
    mask=__get_PRIMASK();
#endif
    __disable_irq();
  }
  ~Lock() { if (!mask) __enable_irq(); }
#endif
};
bool running() { return auxSession ? auxRunning : sequence.running(); }
void pulseOn() { for(uint8_t i=0;i<8;++i) if(pulseMask & (1U<<i)) { if(coilSession) beginCoilTestCharge(i+1); else openInjector(i+1); } }
void pulseOff() { for(uint8_t i=0;i<8;++i) if(pulseMask & (1U<<i)) { if(coilSession) endCoilTestCharge(i+1); else closeInjector(i+1); } }
bool pinIsSensor(uint8_t pin) {
  return pin==pinNumbers.pinTPS || pin==pinNumbers.pinMAP || pin==pinNumbers.pinCLT
    || pin==pinNumbers.pinIAT || pin==pinNumbers.pinO2 || pin==pinNumbers.pinO2_2 || pin==pinNumbers.pinBat
    || (configPage2.flexEnabled && pin==pinNumbers.pinFlex);
}
void publish() {
  benchTelemetry = (enabledKind ? enabledKind-1U : 0U)
      | (owned ? 4U:0U) | (running() ? 8U:0U)
      | ((auxSession ? auxComplete : sequence.state==State::Complete) ? 16U:0U)
      | ((reason!=0) ? 32U:0U)
      | ((owned && enabledKind!=0) ? 64U:0U);

}
void finish(uint8_t why) {
  if(wmiOwned) digitalWrite(wmiEnablePin,LOW);
  if(pumpOwned) { digitalWrite(pumpPin,LOW); pumpLevel=false; }
  timerActive=false;
  FUEL1_TIMER_DISABLE();
  if (auxSession) { if(idleSession) {idleOff();if(!idleIsStepper)currentStatus.idleLoad=0;} else {
      for(uint8_t i=0;i<AUX_COUNT;++i) if(auxClaimed & (1U<<i))
        digitalWrite(auxPins[i],(auxHighMask & (1U<<i)) ? LOW : HIGH);
      auxOnMask=0;auxPwmMask=0;
    } auxRunning=false; auxComplete=(why==0); }
  else { pulseOff(); sequence.abort(); }
  reason=why;
  if(why) enabledKind=0; // A fault disarms actions; Disable releases ownership.
  publish();
}
void triggerAbort() {
  Lock lock;
  if (owned) finish(2);
}
// Borrow the idle fuel-1 compare channel instead of reserving another timer.
// Chunk long waits without generating an output edge at intermediate deadlines.
void scheduleDelay(uint32_t delayUs) {
  const uint32_t limit=(std::min)(uint32_t(60000),uint32_t(MAX_TIMER_PERIOD/2U));
  uint32_t chunk=delayUs>limit ? limit : delayUs;
  if(delayUs>chunk && delayUs-chunk<100U) chunk-=100U;
  remainingDelayUs=delayUs-chunk;
  edgeBudgetMs=chunk/1000U+3U;
  const COMPARE_TYPE elapsed=COMPARE_TYPE(FUEL1_COUNTER-FUEL1_COMPARE);
  const COMPARE_TYPE ticks=uS_TO_TIMER_COMPARE(chunk);
  if(!ticks || elapsed>=ticks) {finish(3);return;}
  SET_COMPARE(FUEL1_COMPARE, FUEL1_COMPARE+ticks);
}
void edgeISR() {
  Lock lock;
  if (!owned || !timerActive) return;
  if (!running()) { finish(reason); return; }
  // A delayed interrupt must not silently produce a long pulse or miss a reload.
  if (ticksToMicros(COMPARE_TYPE(FUEL1_COUNTER-FUEL1_COMPARE)) > 100U) { finish(3); return; }
  if(idleSession && !idleIsStepper) {
    if(!idlePhaseOn) idleCurrentDuty=idleMode==0 ? 0 : (idleMode==2 && !idleCycleHigh ? 0 : idleDuty);
    uint16_t next=10000;
    if(idleCurrentDuty==0 || idleCurrentDuty==100) {
      idlePhaseOn=false; idlePwmWrite(idleCurrentDuty==100);
    }
    else {
      idlePhaseOn=!idlePhaseOn;
      idlePwmWrite(idlePhaseOn);
      next=idlePhaseOn ? uint16_t(idleCurrentDuty)*100U : 10000U-uint16_t(idleCurrentDuty)*100U;
    }
    currentStatus.idleLoad=idleCurrentDuty;
    scheduleDelay(next);
    return;
  }
  if (auxSession) return; // Other auxiliary outputs use the millisecond timer.
  if(remainingDelayUs) { scheduleDelay(remainingDelayUs); return; }
  const uint32_t next=sequence.edge();
  if (sequence.state==State::On) pulseOn();
  else pulseOff();
  if (next==0) { finish(0); }
  else {
    scheduleDelay(next);
    publish();
  }
}
bool schedulesIdle() {
  FuelSchedule *fuel[]={&fuelSchedule1,
#if INJ_CHANNELS >= 2
    &fuelSchedule2,
#endif
#if INJ_CHANNELS >= 3
    &fuelSchedule3,
#endif
#if INJ_CHANNELS >= 4
    &fuelSchedule4,
#endif
#if INJ_CHANNELS >= 5
    &fuelSchedule5,
#endif
#if INJ_CHANNELS >= 6
    &fuelSchedule6,
#endif
#if INJ_CHANNELS >= 7
    &fuelSchedule7,
#endif
#if INJ_CHANNELS >= 8
    &fuelSchedule8,
#endif
  };
  IgnitionSchedule *ign[]={&ignitionSchedule1,
#if IGN_CHANNELS >= 2
    &ignitionSchedule2,
#endif
#if IGN_CHANNELS >= 3
    &ignitionSchedule3,
#endif
#if IGN_CHANNELS >= 4
    &ignitionSchedule4,
#endif
#if IGN_CHANNELS >= 5
    &ignitionSchedule5,
#endif
#if IGN_CHANNELS >= 6
    &ignitionSchedule6,
#endif
#if IGN_CHANNELS >= 7
    &ignitionSchedule7,
#endif
#if IGN_CHANNELS >= 8
    &ignitionSchedule8,
#endif
  };
  for(auto schedule:fuel) if(schedule->_status!=OFF) return false;
  for(auto schedule:ign) if(schedule->_status!=OFF) return false;
  return true;
}
bool auxFeatureEnabled(uint8_t target);
uint8_t auxPin(uint8_t target) {
  const uint8_t pins[]={pinNumbers.pinFuelPump,pinNumbers.pinFan,pinNumbers.pinBoost,pinNumbers.pinVVT_1,pinNumbers.pinVVT_2,pinNumbers.pinVVT_2,pinNumbers.pinAirConComp,pinNumbers.pinAirConFan,pinNumbers.pinIdle1,pinNumbers.pinIdle2};
  return target<AUX_COUNT ? pins[target] : 255;
}
bool configuredMask(bool coil,uint8_t &result) {
  const Schedule *fuel[]={&fuelSchedule1,
#if INJ_CHANNELS >= 2
    &fuelSchedule2,
#endif
#if INJ_CHANNELS >= 3
    &fuelSchedule3,
#endif
#if INJ_CHANNELS >= 4
    &fuelSchedule4,
#endif
#if INJ_CHANNELS >= 5
    &fuelSchedule5,
#endif
#if INJ_CHANNELS >= 6
    &fuelSchedule6,
#endif
#if INJ_CHANNELS >= 7
    &fuelSchedule7,
#endif
#if INJ_CHANNELS >= 8
    &fuelSchedule8,
#endif
  };
  const Schedule *ign[]={&ignitionSchedule1,
#if IGN_CHANNELS >= 2
    &ignitionSchedule2,
#endif
#if IGN_CHANNELS >= 3
    &ignitionSchedule3,
#endif
#if IGN_CHANNELS >= 4
    &ignitionSchedule4,
#endif
#if IGN_CHANNELS >= 5
    &ignitionSchedule5,
#endif
#if IGN_CHANNELS >= 6
    &ignitionSchedule6,
#endif
#if IGN_CHANNELS >= 7
    &ignitionSchedule7,
#endif
#if IGN_CHANNELS >= 8
    &ignitionSchedule8,
#endif
  };
  const uint8_t count=coil ? currentStatus.maxIgnOutputs : currentStatus.injOutputs.getTotalInjectors();
  result=0;
  if(!count || count>(coil ? IGN_CHANNELS : INJ_CHANNELS)) return false;
  for(uint8_t i=0;i<count;++i) {
    const auto cb=(coil ? ign[i] : fuel[i])->_pStartCallback;
    uint8_t mask=0;
    // Keep this lookup in flash code instead of allocating callback tables in SRAM.
    if(coil) {
      if(cb==beginCoil1Charge) mask=1;
      else if(cb==beginCoil2Charge) mask=2;
      else if(cb==beginCoil3Charge) mask=4;
      else if(cb==beginCoil4Charge) mask=8;
      else if(cb==beginCoil5Charge) mask=16;
      else if(cb==beginCoil6Charge) mask=32;
      else if(cb==beginCoil7Charge) mask=64;
      else if(cb==beginCoil8Charge) mask=128;
      else if(cb==beginCoil1and3Charge) mask=5;
      else if(cb==beginCoil2and4Charge) mask=10;
      else if(cb==beginCoil1and4Charge) mask=9;
      else if(cb==beginCoil2and5Charge) mask=18;
      else if(cb==beginCoil3and6Charge) mask=36;
      else if(cb==beginCoil1and5Charge) mask=17;
      else if(cb==beginCoil2and6Charge) mask=34;
      else if(cb==beginCoil3and7Charge) mask=68;
      else if(cb==beginCoil4and8Charge) mask=136;
      else if(cb==beginTrailingCoilCharge) mask=2;
    } else {
      if(cb==openInjector1) mask=1;
      else if(cb==openInjector2) mask=2;
      else if(cb==openInjector3) mask=4;
      else if(cb==openInjector4) mask=8;
      else if(cb==openInjector5) mask=16;
      else if(cb==openInjector6) mask=32;
      else if(cb==openInjector7) mask=64;
      else if(cb==openInjector8) mask=128;
      else if(cb==openInjector1and3) mask=5;
      else if(cb==openInjector2and4) mask=10;
      else if(cb==openInjector1and4) mask=9;
      else if(cb==openInjector2and3) mask=6;
      else if(cb==openInjector3and5) mask=20;
      else if(cb==openInjector2and5) mask=18;
      else if(cb==openInjector3and6) mask=36;
      else if(cb==openInjector1and5) mask=17;
      else if(cb==openInjector2and6) mask=34;
      else if(cb==openInjector3and7) mask=68;
      else if(cb==openInjector4and8) mask=136;
    }
    if(!mask) return false;
    result|=mask;
  }
  return true;
}
bool auxiliaryInputUsesPin(uint8_t pin) {
  if(configPage10.knock_mode==KNOCK_MODE_DIGITAL && pin==configPage10.knock_pin) return true;
  if(configPage10.knock_mode==KNOCK_MODE_ANALOG) {
    const uint8_t knockPin=configPage10.knock_pin>=47U ? pinTranslateAnalog(configPage10.knock_pin-47U) : A15;
    if(pin==knockPin) return true;
  }
  // Use the same input-bank selection and pin translation as initialiseADC().
  const bool external=configPage9.enable_secondarySerial==1U
      || (configPage9.enable_intcan==1U && configPage9.intcan_available==1U);
  for(uint8_t i=0;i<16;++i) {
    const uint8_t source=(configPage9.caninput_sel[i]>>(external ? 2 : 0)) & 3U;
    if(source==2 && pin==pinTranslateAnalog(configPage9.Auxinpina[i]&63U)) return true;
    if(source==3 && pin==((configPage9.Auxinpinb[i]&63U)+1U)) return true;
  }
  return false;
}
bool otherFunctionUsesPin(uint8_t pin,uint8_t target=NO_AUX,uint8_t idleGroup=0) {
  if(auxiliaryInputUsesPin(pin)) return true;
  for(uint8_t i=0;i<AUX_COUNT;++i) {
    if(i==target || (idleGroup==1 && i>=8)) continue;
    if(auxFeatureEnabled(i) && auxPin(i)==pin) return true;
  }
  if(target!=WMI_ENABLE_TARGET && configPage10.wmiEnabled && pin==pinNumbers.pinWMIEnabled) return true;
  if(idleGroup!=2 && (configPage6.iacAlgorithm==4 || configPage6.iacAlgorithm==5 || configPage6.iacAlgorithm==7)
      && (pin==pinNumbers.pinStepperDir || pin==pinNumbers.pinStepperStep || pin==pinNumbers.pinStepperEnable)) return true;
  const uint8_t protectedPins[]={pinNumbers.pinTachOut,pinNumbers.pinTrigger,pinNumbers.pinTrigger2,pinNumbers.pinTrigger3,
    pinNumbers.pinAirConRequest,pinNumbers.pinResetControl,pinNumbers.pinBaro,pinNumbers.pinEMAP,
    pinNumbers.pinFlex,pinNumbers.pinVSS,pinNumbers.pinLaunch,pinNumbers.pinIdleUp,pinNumbers.pinIdleUpOutput,
    pinNumbers.pinWMIEmpty,pinNumbers.pinWMIIndicator,pinNumbers.pinFuelPressure,pinNumbers.pinOilPressure,
    pinNumbers.pinIgnBypass,pinNumbers.pinCTPS};
  for(auto value:protectedPins) if(value==pin) return true;
  for(auto value:configPage13.outputPin) if(value>0 && value<128 && value==pin) return true;
  return (pin==pinNumbers.pinFuel2Input && configPage10.fuel2Mode==4)
      || (pin==pinNumbers.pinSpark2Input && configPage10.spark2Mode==4);
}
bool usesSpiDrivers() {
#ifdef MC33810_SUPPORT
  return pinNumbers.pinMC33810_1_CS!=NOT_A_PIN;
#else
  return false;
#endif
}
bool outputExists(uint8_t channel,bool coil) {
  if(!channel || channel>(coil ? IGN_CHANNELS : INJ_CHANNELS)) return false;
#ifdef MC33810_SUPPORT
  if(usesSpiDrivers()) {
    const uint8_t cs=channel>4 ? pinNumbers.pinMC33810_2_CS : pinNumbers.pinMC33810_1_CS;
    return cs!=NOT_A_PIN && cs<NUM_DIGITAL_PINS;
  }
#endif
  const uint8_t pin=coil ? pinNumbers.coilPins[channel-1] : pinNumbers.injectorPins[channel-1];
  return pin!=NOT_A_PIN && pin<NUM_DIGITAL_PINS;
}
bool pinUsable(uint16_t channel,bool coil=false) {
  if(!outputExists(channel,coil)) return false;
#ifdef MC33810_SUPPORT
  if(usesSpiDrivers()) {
    // These are logical driver channels. Never apply GPIO alias tests to their IDs.
    const uint8_t *bits=coil ? pinNumbers.mc33810IgnBits : pinNumbers.mc33810InjBits;
    const uint8_t bit=bits[channel-1];
    if(bit>7 || (coil ? bit<4 : bit>3)) return false;
    const uint8_t base=((channel-1)/4)*4;
    for(uint8_t i=base;i<base+4 && i<(coil ? IGN_CHANNELS : INJ_CHANNELS);++i)
      if(i!=channel-1 && bits[i]==bit) return false;
    if(otherFunctionUsesPin(pinNumbers.pinMC33810_1_CS)) return false;
    if(pinNumbers.pinMC33810_2_CS!=NOT_A_PIN && otherFunctionUsesPin(pinNumbers.pinMC33810_2_CS)) return false;
    for(auto pin:pinNumbers.injectorPins) if(pin!=NOT_A_PIN && otherFunctionUsesPin(pin)) return false;
    return true;
  }
#endif
  const uint8_t pin=coil ? pinNumbers.coilPins[channel-1] : pinNumbers.injectorPins[channel-1];
  if(pinIsReserved(pin) || pinIsSensor(pin)) return false;
  for(uint8_t i=0;i<(coil ? IGN_CHANNELS : INJ_CHANNELS);++i)
    if(i!=channel-1 && (coil ? pinNumbers.coilPins[i] : pinNumbers.injectorPins[i])==pin) return false;
  for(uint8_t i=0;i<(coil ? INJ_CHANNELS : IGN_CHANNELS);++i)
    if((coil ? pinNumbers.injectorPins[i] : pinNumbers.coilPins[i])==pin) return false;
  return !otherFunctionUsesPin(pin);
}
bool startBlocked() {
  return owned || millis()<5000 || currentStatus.RPM || currentStatus.decoder.isEngineRunning(micros())
      || currentStatus.decoder.getStatus().syncStatus!=SyncStatus::None
      || currentStatus.isTestModeActive || !schedulesIdle()
      || currentStatus.toothLogEnabled || currentStatus.compositeTriggerUsed;
}
void setTriggerHandlers(bool testing) {
  const interrupt_t *sources[]={&currentStatus.decoder.primary,&currentStatus.decoder.secondary,&currentStatus.decoder.tertiary};
  const uint8_t pins[]={pinNumbers.pinTrigger,pinNumbers.pinTrigger2,pinNumbers.pinTrigger3};
  for(uint8_t i=0;i<3;++i) if(sources[i]->isValid())
    attachInterrupt(digitalPinToInterrupt(pins[i]),testing ? triggerAbort : sources[i]->callback,testing ? CHANGE : sources[i]->edge);
}
void acquire() {
  leaseMs=2000;edgeBudgetMs=4;reason=0;owned=true;
  FUEL1_TIMER_DISABLE();
  setTriggerHandlers(true);
}
void startTimer(uint16_t firstUs) {
  FUEL1_TIMER_DISABLE();timerActive=true;remainingDelayUs=0;
  SET_COMPARE(FUEL1_COMPARE,FUEL1_COUNTER+uS_TO_TIMER_COMPARE(firstUs));
  FUEL1_TIMER_ENABLE();
}
// Do not allow a test selection to drive another output, an input or a reserved pin.
bool auxPinUsable(uint8_t target,uint8_t pin,uint8_t idleGroup=0) {
  if(pin==NOT_A_PIN || pin>=NUM_DIGITAL_PINS || pinIsReserved(pin) || pinIsSensor(pin)) return false;
  if(usesSpiDrivers() && (pinNumbers.injectorPins.isPinUsed(pin) || pinNumbers.coilPins.isPinUsed(pin))) return false;
#ifdef MC33810_SUPPORT
  if(usesSpiDrivers() && (pin==pinNumbers.pinMC33810_1_CS || pin==pinNumbers.pinMC33810_2_CS)) return false;
#endif
  uint8_t injMask=0,ignMask=0;
  if(!configuredMask(false,injMask) || !configuredMask(true,ignMask)) return false;
  for(uint8_t i=0;i<INJ_CHANNELS;++i) {
    if((injMask & (1U<<i)) && pinNumbers.injectorPins[i]==pin) return false;
  }
  for(uint8_t i=0;i<IGN_CHANNELS;++i) {
    if((ignMask & (1U<<i)) && pinNumbers.coilPins[i]==pin) return false;
  }
  return !otherFunctionUsesPin(pin,target,idleGroup);
}
bool idleFeatureEnabled() {
  const auto algorithm=configPage6.iacAlgorithm;
  return algorithm>=2 && algorithm<=7;
}
bool auxFeatureEnabled(uint8_t target) {
  switch(target) {
    case 0: return true; // Fuel pump has no feature-enable setting.
    case 1: return configPage2.fanEnable==1 || configPage2.fanEnable==2;
    case 2: return configPage6.boostEnabled;
    case 3: return configPage6.vvtEnabled;
    case 4: return configPage6.vvtEnabled && configPage10.vvt2Enabled && !configPage10.wmiEnabled;
    case 5: return configPage10.wmiEnabled && !configPage10.vvt2Enabled;
    case 6: return configPage15.airConEnable;
    case 7: return configPage15.airConEnable && configPage15.airConFanEnabled;
    case 8: return configPage6.iacAlgorithm==2 || configPage6.iacAlgorithm==3 || configPage6.iacAlgorithm==6;
    case 9: return auxFeatureEnabled(8) && configPage6.iacChannels;
    default: return false;
  }
}
uint8_t enable(uint8_t kind) {
  if(kind<1 || kind>4 || (kind==4 && !idleFeatureEnabled())) return RC_RANGE;
  Lock lock;
  if(owned) return enabledKind==kind && !reason ? RC_OK : RC_BUSY;
  if(startBlocked()) return RC_BUSY;
  if(!currentStatus.decoder.primary.isValid()) return RC_RANGE;
  const uint8_t triggers[]={pinNumbers.pinTrigger,pinNumbers.pinTrigger2,pinNumbers.pinTrigger3};
  const interrupt_t *sources[]={&currentStatus.decoder.primary,&currentStatus.decoder.secondary,&currentStatus.decoder.tertiary};
  for(uint8_t i=0;i<3;++i) if(sources[i]->isValid()) {
    const uint8_t pin=triggers[i];
    if(auxiliaryInputUsesPin(pin) || (configPage2.flexEnabled && pin==pinNumbers.pinFlex)
        || (isExternalVssMode(configPage2) && pin==pinNumbers.pinVSS)) return RC_RANGE;
  }
  enabledKind=kind;coilSession=false;pulseMask=0;
  sequence.state=State::Idle;
  auxClaimed=0;auxOnMask=0;auxPwmMask=0;auxHighMask=0;wmiOwned=false;
  auxSession=false;idleSession=false;auxRunning=false;auxComplete=false;
  acquire();
  if(kind==4) currentStatus.idleLoad=0;
  publish();return RC_OK;
}
bool ready(uint8_t kind) { return owned && enabledKind==kind && !reason; }
void writeAux(uint8_t target,bool on) {
  const bool high=(auxHighMask & (1U<<target))!=0;
  digitalWrite(auxPins[target],on ? high : !high);
}
uint8_t startAux(uint8_t target,uint8_t mode) {
  if(target>=AUX_COUNT || mode>2 || (mode==2 && (target==0 || target==6 || target==7))) return RC_RANGE;
  if(!auxFeatureEnabled(target)) return RC_RANGE;
  const uint16_t frequency10=read16(staging+22);
  if(mode==2 && (frequency10<10 || frequency10>2000)) return RC_RANGE;
  const uint8_t pin=auxPin(target);
  if(!auxPinUsable(target,pin)) return RC_RANGE;
  if(target==5 && !auxPinUsable(WMI_ENABLE_TARGET,pinNumbers.pinWMIEnabled)) return RC_RANGE;
  Lock lock;
  if(!ready(3)) return RC_BUSY;
  const uint16_t bit=1U<<target;
  if(!(auxClaimed & bit)) {
    auxPins[target]=pin;
    const bool high=target==1 ? !configPage6.fanInv :
      target==6 ? !configPage15.airConCompPol : target==7 ? !configPage15.airConFanPol : true;
    if(high) auxHighMask|=bit; else auxHighMask&=~bit;
    auxClaimed|=bit;
    writeAux(target,false);pinMode(auxPins[target],OUTPUT);
  }
  if(target==5) {
    if(!wmiOwned) {
      wmiEnablePin=pinNumbers.pinWMIEnabled;wmiOwned=true;
      digitalWrite(wmiEnablePin,LOW);pinMode(wmiEnablePin,OUTPUT);
    }
    digitalWrite(wmiEnablePin,mode ? HIGH : LOW);
  }
  auxSession=true;auxRunning=true;auxComplete=false;
  if(mode==0) {auxOnMask&=~bit;auxPwmMask&=~bit;writeAux(target,false);}
  else if(mode==1) {auxOnMask|=bit;auxPwmMask&=~bit;writeAux(target,true);}
  else {
    auxOnMask&=~bit;auxPwmMask|=bit;
    // Frequency changes are applied on a 50% command and shared by pulsed outputs.
    if(auxFrequency10!=frequency10) {auxFrequency10=frequency10;auxPhase=0;auxPhaseHigh=true;}
    for(uint8_t i=0;i<AUX_COUNT;++i) if(auxPwmMask & (1U<<i)) writeAux(i,auxPhaseHigh);
  }
  publish();return RC_OK;
}
void tickAuxOutputs() {
  if(!auxPwmMask) return;
  auxPhase+=2U*auxFrequency10;
  if(auxPhase>=10000U) {
    auxPhase-=10000U;auxPhaseHigh=!auxPhaseHigh;
    for(uint8_t i=0;i<AUX_COUNT;++i) if(auxPwmMask & (1U<<i)) writeAux(i,auxPhaseHigh);
  }
}
uint8_t startIdle(uint8_t mode) {
  const uint16_t home=read16(staging+16), position=read16(staging+18), duty=read16(staging+20);
  const uint8_t algorithm=configPage6.iacAlgorithm;
  const bool stepper=algorithm==4 || algorithm==5 || algorithm==7;
  const bool pwm=algorithm==2 || algorithm==3 || algorithm==6;
  if(mode>2 || (!stepper && !pwm)) return RC_RANGE;
  if(stepper) {
    if(home==0 || home>765 || configPage6.iacStepTime==0
        || (mode!=0 && (position>255 || position>=home || position>uint16_t(configPage9.iacMaxSteps)*3U))) return RC_RANGE;
    const uint8_t pins[]={pinNumbers.pinStepperDir,pinNumbers.pinStepperStep,pinNumbers.pinStepperEnable};
    for(uint8_t i=0;i<3;++i) {
      if(!auxPinUsable(NO_AUX,pins[i],2)) return RC_RANGE;
      for(uint8_t j=0;j<i;++j) if(pins[i]==pins[j]) return RC_RANGE;
    }
  }
  else {
    if(duty>100 || !auxPinUsable(NO_AUX,pinNumbers.pinIdle1,1)) return RC_RANGE;
    if(configPage6.iacChannels && (pinNumbers.pinIdle1==pinNumbers.pinIdle2 || !auxPinUsable(NO_AUX,pinNumbers.pinIdle2,1))) return RC_RANGE;
  }
  Lock lock;
  if(!ready(4) || running()) return RC_BUSY;
  // Normal idle is paused by Enable. End a pre-existing STEP pulse before homing.
  if(stepper) idleBenchRestorePosition(false,0);
  idleSession=true; idleIsStepper=stepper; idleTwoPins=configPage6.iacChannels;
  idleMode=mode; idleDuty=duty; idleCurrentDuty=0; idlePhaseOn=false;
  idleCycleMs=1000; idleCycleHigh=true;
  idleActiveLevel=!configPage6.iacPWMdir;
  auxSession=true; auxRunning=true; auxComplete=false; auxRemainingMs=30000;
  leaseMs=2000;edgeBudgetMs=4;idleOff();currentStatus.idleLoad=0;
  if(stepper) {
    pinMode(pinNumbers.pinStepperDir,OUTPUT);pinMode(pinNumbers.pinStepperStep,OUTPUT);pinMode(pinNumbers.pinStepperEnable,OUTPUT);
    idleSteps.start(home,position,mode,configPage6.iacStepTime,configPage9.iacCoolTime);
  }
  else {
    pinMode(pinNumbers.pinIdle1,OUTPUT);if(idleTwoPins)pinMode(pinNumbers.pinIdle2,OUTPUT);
    startTimer(1000);
  }
  publish();return RC_OK;
}
void tickIdle() {
  if(idleIsStepper) {
    idleSteps.tick();
    digitalWrite(pinNumbers.pinStepperDir,idleSteps.closing ? !configPage9.iacStepperInv : configPage9.iacStepperInv);
    digitalWrite(pinNumbers.pinStepperEnable,idleSteps.running ? LOW : HIGH);
    digitalWrite(pinNumbers.pinStepperStep,idleSteps.stepHigh ? HIGH : LOW);
    currentStatus.idleLoad=idleSteps.position;
    if(!idleSteps.running) finish(0);
  }
  else {
    if(idleMode==2 && --idleCycleMs==0) {idleCycleHigh=!idleCycleHigh;idleCycleMs=1000;}
    if(edgeBudgetMs) --edgeBudgetMs;
    if(!edgeBudgetMs) finish(4);
  }
}
uint8_t setPump(bool on) {
  Lock lock;
  if(!ready(1)) return RC_BUSY;
  if(!pumpOwned) {
    if(!auxFeatureEnabled(0) || !auxPinUsable(0,pinNumbers.pinFuelPump)) return RC_RANGE;
    pumpPin=pinNumbers.pinFuelPump; pumpOwned=true;
    digitalWrite(pumpPin,LOW); pinMode(pumpPin,OUTPUT);
  }
  if(on && !pumpLevel) pumpRemainingMs=120000;
  pumpLevel=on; digitalWrite(pumpPin,on ? HIGH : LOW);
  publish();
  return RC_OK;
}
uint8_t start(bool coil=false) {
  const uint8_t *settings=staging+(coil ? 8 : 0);
  const uint16_t on=read16(settings+2);
  const uint32_t period=uint32_t(read16(settings+4))*10U;
  // Require a real off interval; reject before subtraction to prevent wrapping.
  if (period <= on || period-on < 100U || period > 500000U) return RC_RANGE;
  const Settings s={read16(settings),on,period-on,read16(settings+6)};
  if(coil && (on>5000 || period<10000)) return RC_RANGE;
  if (!valid(s)) return RC_RANGE;
  uint8_t selected=0;
  if(s.channel==0) {for(uint8_t i=1;i<=8;++i) if(outputExists(i,coil)) selected|=uint8_t(1U<<(i-1));}
  else if(s.channel==9) {if(!configuredMask(coil,selected)) return RC_RANGE;}
  else selected=uint8_t(1U<<(s.channel-1));
  if(!selected) return RC_RANGE;
  for(uint8_t i=1;i<=8;++i) if((selected & (1U<<(i-1))) && !pinUsable(i,coil)) return RC_RANGE;
  Lock lock;
  if (!ready(coil ? 2 : 1) || running()) return RC_BUSY;
  pulseMask=selected;
  auxSession=false; coilSession=coil;
  sequence.start(s);
  leaseMs=2000;edgeBudgetMs=4;
  pulseOff();
  startTimer(1000); // first pulse after 1 ms
  publish();
  return RC_OK;
}
}

bool injectorBenchHandleTimer(FuelSchedule &schedule) {
  if(!owned || &schedule!=&fuelSchedule1) return false;
  edgeISR();return true;
}
bool injectorBenchOwnsOutputs() { return owned; }
bool injectorBenchOwnsPin(uint8_t pin) {
  if(!owned) return false;
  for(uint8_t i=0;i<(coilSession ? IGN_CHANNELS : INJ_CHANNELS);++i) if(!usesSpiDrivers() && (pulseMask & (1U<<i))
      && (coilSession ? pinNumbers.coilPins[i] : pinNumbers.injectorPins[i])==pin) return true;
  if(idleSession && (pin==pinNumbers.pinIdle1 || (idleTwoPins && pin==pinNumbers.pinIdle2)
      || (idleIsStepper && (pin==pinNumbers.pinStepperDir || pin==pinNumbers.pinStepperStep || pin==pinNumbers.pinStepperEnable)))) return true;
  if(wmiOwned && pin==wmiEnablePin) return true;
  if(pumpOwned && pin==pumpPin) return true;
  for(uint8_t i=0;i<AUX_COUNT;++i) if((auxClaimed & (1U<<i)) && auxPins[i]==pin) return true;
  return false;
}
void injectorBenchTick() {
  Lock lock;
  if (owned) {
    if (leaseMs) --leaseMs;
    if (!leaseMs) finish(1);
    else if(pumpLevel && --pumpRemainingMs==0) finish(5);
    else if(!running()) {}
    else if (idleSession && --auxRemainingMs==0) finish(0);
    else if(idleSession) tickIdle();
    else if (auxSession) tickAuxOutputs();
    else {
      if (edgeBudgetMs) --edgeBudgetMs;
      if (!edgeBudgetMs) finish(4);
    }
  }
}
void injectorBenchKeepAlive() { Lock lock; if (owned) leaseMs=2000; }
void injectorBenchStop() {
  Lock lock;
  if (owned) {
    finish(0);
    if(idleSession && idleIsStepper) idleBenchRestorePosition(idleSteps.positionKnown(),idleSteps.position);
    idleSession=false;
    // Remain inhibited until the user explicitly stops, even on completion/fault.
    currentStatus.decoder.reset();
    setTriggerHandlers(false);
    FUEL1_TIMER_ENABLE();
    sequence.state=State::Idle;
    owned=false;enabledKind=0;pulseMask=0;auxClaimed=0;reason=0;
    pumpOwned=false; pumpLevel=false;wmiOwned=false;
    auxSession=false; coilSession=false; auxRunning=false; auxComplete=false;
    publish();
  }
}
uint16_t injectorBenchCommand(uint8_t *p,uint16_t n) {
  using namespace injector_bench;
  uint8_t rc=RC_RANGE;
  if (n<2) { p[0]=rc; return 1; }
  const uint8_t op=p[1];
  if (op==0 && n==2) { injectorBenchStop(); rc=RC_OK; }
  else if (op==11 && n==2) {
    Lock lock;
    if(owned && enabledKind) {
      finish(0);sequence.state=State::Idle;auxComplete=false;publish();rc=RC_OK;
    }
    else rc=RC_BUSY;
  }
  else if (op==10 && n==3) rc=enable(p[2]);
  else if (op==1 && n==2) rc=start();
  else if (op==6 && n==4) rc=startAux(p[2],p[3]);
  else if (op==7 && n==2) rc=start(true);
  else if (op==9 && n==3) rc=startIdle(p[2]);
  else if (op==8 && n==3 && p[2]<=1) rc=setPump(p[2]!=0);
  else if (op==2 && n==2) {
    Lock lock;
    if (owned) leaseMs=2000;
    p[0]=RC_OK; p[1]=8; // protocol version: extended RAM page with auxiliary tests
    p[2]=uint8_t(auxSession ? (auxRunning ? State::On : (auxComplete ? State::Complete : State::Aborted)) : sequence.state); p[3]=reason;
    write16(p+4,sequence.settings.channel); write16(p+6,sequence.settings.onUs);
    write16(p+8,(sequence.settings.onUs+sequence.settings.offUs)/10U); write16(p+10,sequence.settings.count);
    write16(p+12,sequence.completed); p[14]=owned;
    return 15;
  }
  else if ((op==3 || op==4) && n>=6) {
    const uint16_t offset=read16(p+2), count=read16(p+4);
    if (range(offset,count,sizeof(staging)) && n==(op==3 ? 6U : 6U+count)) {
      if (op==3) { memcpy(p+1,staging+offset,count); p[0]=RC_OK; return count+1; }
      // Staging is not read by the timer ISR. start() snapshots it into
      // sequence.settings, so edits always apply to the next test only.
      memcpy(staging+offset,p+6,count);
      rc=RC_OK;
    }
  }
  else if (op==5 && n==2) {
    FastCRC32 crc;
    uint32_t value=crc.crc32(staging,sizeof(staging));
    p[0]=RC_OK;
    for (uint8_t i=0;i<4;++i) p[i+1]=uint8_t(value>>(24-8*i));
    return 5;
  }
  p[0]=rc;
  return 1;
}
