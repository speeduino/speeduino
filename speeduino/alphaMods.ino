#include "alphaMods.h"
#include "errors.h"

#include "auxiliaries.h"
#include "timers.h"
#include "sensors.h"
#include "idle.h"
#include "table.h"

struct table2D zttAdvance;

void alphaTableSetup()
{
  // init ztt Advance table
  zttAdvance.xSize = 6;
  zttAdvance.valueSize = SIZE_BYTE;
  zttAdvance.axisX = configPage4.idleZTTBins;
  zttAdvance.values = configPage4.idleZTTValues;

}

//pin setup
void alphaPinSetup() {
  switch (alphaVars.carSelect) {
    case 0:
      pinAC; // pin for AC clutch
      pinAcReq;
      pinFan2;
      pinCEL;
      pinVVL;
      pinACpress;
      pinACtemp;
      pinRollingAL = 50;

    case 1:
      pinAC = 45; // pin for AC clutch
      pinAcReq = 26;
      pinFan2 = 49;
      pinCEL = 53;
      pinVVL = 6;
      pinACpress = 28;
      pinACtemp = A5;
      pinOilPress = 30;
      pinCLTgauge = 2;
      pinMode(pinAC, OUTPUT);
      pinMode(pinAcReq, INPUT);
      pinMode(pinCEL, OUTPUT);
      pinMode(pinVVL, OUTPUT);
      pinMode(pinACpress, INPUT_PULLUP);
      pinMode(pinACtemp, INPUT);
      pinMode(pinOilPress, INPUT_PULLUP);
      pinMode(pinCLTgauge, OUTPUT);
      break;
    case 2:
      pinAC = 44; // pin for AC clutch
      pinAcReq = 26;
      pinFan2 = 46;
      pinCEL = 30;
      //pinVVL = 6;
      pinACpress = 28;
      pinMode(pinAC, OUTPUT);
      pinMode(pinAcReq, INPUT);
      pinMode(pinCEL, OUTPUT);
      pinMode(pinFan2, OUTPUT);
      //pinMode(pinVVL, OUTPUT);
      pinMode(pinACpress, INPUT_PULLUP);
      break;
    case 4:
      pinAC = 45; // pin for AC clutch
      pinAcReq = 26;
      pinFan2 = 46;
      pinCEL = 53;
      //pinVVL = 6;
      pinACpress = 28;
      pinACtemp = A5;
      pinOilPress = 30;
      //pinCLTgauge = 2;
      pinMode(pinAC, OUTPUT);
      pinMode(pinAcReq, INPUT);
      pinMode(pinCEL, OUTPUT);
      pinMode(pinFan2, OUTPUT);
      //pinMode(pinVVL, OUTPUT);
      pinMode(pinACpress, INPUT_PULLUP);
      pinMode(pinACtemp, INPUT);
      pinMode(pinOilPress, INPUT_PULLUP);
    //pinMode(pinCLTgauge, OUTPUT);
    default:
      break;
  }
}

void initialiseAC()
{
  digitalWrite(pinAC, LOW); // initialize AC low
  BIT_CLEAR(alphaVars.alphaBools1, BIT_AC_ON);
}

void fanControl2()
{
  //fan2
  if ( ((currentStatus.coolant >= 85) && (currentStatus.RPM > 500)) || (BIT_CHECK(alphaVars.alphaBools1, BIT_AC_REQ)) ) {
    digitalWrite(pinFan2, fanHIGH);
    currentStatus.fanOn = true;
  }
  if ( ( (currentStatus.coolant <= 82) && (!BIT_CHECK(alphaVars.alphaBools1, BIT_AC_REQ))) || (currentStatus.RPM == 0) ) {
    digitalWrite(pinFan2, fanLOW);
    currentStatus.fanOn = false;
  }
}

void audiFanControl()
{
  if (loopCLT == 1) {
    digitalWrite(pinFan2, LOW);
  }
  else {
    if (currentStatus.coolant < 80) {
      if (loopCLT == 21) {
        digitalWrite(pinFan2, HIGH);
      }
    }
    else if ((currentStatus.coolant >= 80) && (currentStatus.coolant < 90)) {
      if (loopCLT == 101) {
        digitalWrite(pinFan2, HIGH);
      }
    }
    else {
      if (loopCLT == 151) {
        digitalWrite(pinFan2, HIGH);
      }
    }
  }
  /*if (currentStatus.coolant < 80){
    if (loopCLT == 1){
      digitalWrite(pinFan2,LOW);
     // Serial.print("LOW LoopCLT =");Serial.println(pinFan2);
    }
    else if (loopCLT == 21){
      digitalWrite(pinFan2,HIGH);
      //Serial.print("HIGH LoopCLT =");Serial.println(loopCLT);
    }
    }
    else if ((currentStatus.coolant >= 80) && (currentStatus.coolant < 90)){
    if (loopCLT == 100){
      digitalWrite(pinFan2,LOW);
    }
    else{
      digitalWrite(pinFan2,HIGH);
    }
    }
    else if (currentStatus.coolant >= 90){
    if (loopCLT < 150){
      digitalWrite(pinFan2,LOW);
    }
    else{
      digitalWrite(pinFan2,HIGH);
    }
    }*/
}

void ACControl()
{
  if ((BIT_CHECK(alphaVars.alphaBools1, BIT_AC_REQ)) && (currentStatus.TPS < 60) && (currentStatus.RPM > 600) && (currentStatus.RPM < 3600)) {
    digitalWrite(pinAC, HIGH);  // turn on AC compressor
    BIT_SET(alphaVars.alphaBools1, BIT_AC_ON);
  }
  else {
    digitalWrite(pinAC, LOW);  // shut down AC compressor
    BIT_CLEAR(alphaVars.alphaBools1, BIT_AC_ON);
  }
}

void CELcontrol()
{
  if ((mapErrorCount > 4) || (cltErrorCount > 4) || (iatErrorCount > 4)  || (errorCount > 1) || (currentStatus.RPM == 0)) {
    BIT_SET(alphaVars.alphaBools1, BIT_CEL_STATE);
  }
  else {
    BIT_CLEAR(alphaVars.alphaBools1, BIT_CEL_STATE);
  }
  if (BIT_CHECK(alphaVars.alphaBools1, BIT_CEL_STATE))  {
    digitalWrite(pinCEL, HIGH);
  }
  else {
    digitalWrite(pinCEL, LOW);
  }
}


void vvlControl()
{
  if ((currentStatus.RPM >= 5400) && (currentStatus.TPS > 80) && (currentStatus.coolant > 50))
  {
    if (!BIT_CHECK(alphaVars.alphaBools1, BIT_VVL_ON))
    {
      BIT_SET(alphaVars.alphaBools1, BIT_VVL_ON);
      digitalWrite(pinVVL, HIGH);
        //  Serial.println("VVL ON");
    }
  }
  else if ((currentStatus.RPM < 5300) && (currentStatus.TPS < 80)) {
    digitalWrite(pinVVL, LOW);
    BIT_CLEAR(alphaVars.alphaBools1, BIT_VVL_ON);
      //  Serial.println("VVL OFF");

  }
}

void readACReq()
{
  if (alphaVars.carSelect == 2) {
    if ((digitalRead(pinAcReq) == HIGH) && (digitalRead(pinACpress) == HIGH)) {
      BIT_SET(alphaVars.alphaBools1, BIT_AC_REQ); //pin 26 is AC Request, pin 28 is a combined pressure/temp signal that is high when the A/C compressor can be activated
    }
    else {
      BIT_CLEAR(alphaVars.alphaBools1, BIT_AC_REQ);
    }
  }
  else if (alphaVars.carSelect == 1) {
    if ((digitalRead(pinAcReq) == HIGH) && (digitalRead(pinACpress) == LOW) && (analogRead(pinACtemp) < 780)) {
      BIT_SET(alphaVars.alphaBools1, BIT_AC_REQ);
    }
    else {
      BIT_CLEAR(alphaVars.alphaBools1, BIT_AC_REQ);
    }
  }
}

//Simple correction if VVL is active
static inline uint8_t correctionVVL()
{
  uint8_t VVLValue = 100;
  if ((BIT_CHECK(alphaVars.alphaBools1, BIT_VVL_ON)) && (alphaVars.carSelect == 1)){
    VVLValue = 102;  //Adds 7% fuel when VVL is active
  }
  return VVLValue;
}
void alpha4hz(){
//alphamods
  if ((alphaVars.carSelect != 255) && (alphaVars.carSelect != 0)){
  readACReq();
  if(digitalRead(pinRollingAL)){
    BIT_SET(alphaVars.alphaBools2, BIT_RLING_TRIG);
  }
  else{ BIT_CLEAR(alphaVars.alphaBools2, BIT_RLING_TRIG);}
  }
//alphaMods
}

static inline uint8_t correctionAlphaN() {
  uint8_t alphaNvalue = 100;
  if ((configPage2.fuelAlgorithm == LOAD_SOURCE_TPS) && (currentStatus.MAP > 100) && (alphaVars.carSelect == 7)){
    static uint8_t startMAP = 100;
    static uint8_t endMAP = 280;
    static uint8_t startCorr = 100;
    static uint8_t endCorr = 128;
    alphaNvalue = map(currentStatus.MAP, startMAP, endMAP, startCorr, endCorr);
    alphaNvalue = constrain(alphaNvalue, startCorr, endCorr);
  }
  return alphaNvalue;
}


/*
   Returns true if decelleration fuel cutoff should be on, false if its off
*/
static inline bool correctionDFCO2()
{
  bool DFCOValue = false;
  if ( configPage2.dfcoEnabled == 1 )
  {
    if ( bitRead(currentStatus.status1, BIT_STATUS1_DFCO) == 1 ) {
      DFCOValue = ( currentStatus.RPM > ( configPage4.dfcoRPM * 10) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ) && (BIT_CHECK(alphaVars.alphaBools1, BIT_DFCO_WAIT))  && (currentStatus.coolant > 60);
    }
    else {
      DFCOValue = ( currentStatus.RPM > (unsigned int)( (configPage4.dfcoRPM * 10) + configPage4.dfcoHyster) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ) && (BIT_CHECK(alphaVars.alphaBools1, BIT_DFCO_WAIT)) && (currentStatus.coolant > 60);
    }
  }
  return DFCOValue;
}

static inline int8_t correctionAtUpshift(int8_t advance)
{
  int8_t upshiftAdvance = advance;
  if ((currentStatus.rpmDOT < -1700) && (currentStatus.TPS > 90)) {
    upshiftAdvance = advance - 10;
  }
  return upshiftAdvance;
}

static inline int8_t correctionZeroThrottleTiming(int8_t advance)
{
  static uint16_t idleRPMtrg = configPage4.idleRPMtarget * 10;
  static uint16_t idleRPMmin = idleRPMtrg - (configPage4.idleRPMNegHyst * 10);
  static uint16_t idleRPMmax = idleRPMtrg + (configPage4.idleRPMPosHyst * 10);
  static int8_t popAndBangTiming = -5;
  static uint8_t ASEtiming = 12;

  int8_t ignZeroThrottleValue = advance;
    if(configPage4.idleZTTenabled &&
      (currentStatus.TPS < configPage4.idleTPSlimit) &&
      (currentStatus.MAP < configPage4.idleMAPlimit) &&
      (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_ASE))) //Check whether TPS coorelates to zero value
    {
      if((currentStatus.RPM > idleRPMmin) && (currentStatus.RPM < idleRPMmax)) {
        ignZeroThrottleValue = table2D_getValue(&zttAdvance, currentStatus.RPM / 10); // Divide by 10 because values are stored that way in EEPROM
      }
      // ignZeroThrottleValue = constrain(ignZeroThrottleValue , configPage4.idleAdvMin, configPage4.idleAdvMax);
      
      if ((currentStatus.RPM > 3000) && (currentStatus.RPM < 5500)) {
        ignZeroThrottleValue = popAndBangTiming;
      }
       if ((BIT_CHECK(alphaVars.alphaBools1, BIT_AC_ON)) && (currentStatus.RPM < 3000) && (currentStatus.TPS < 30)) {
      ignZeroThrottleValue = ignZeroThrottleValue + 2;
    }
    else if ((currentStatus.TPS < 2) && (BIT_CHECK(currentStatus.engine, BIT_ENGINE_ASE))) {
      ignZeroThrottleValue = ASEtiming;
    }
  }
   
  return ignZeroThrottleValue;
}

static inline int8_t correctionTimingAlphaN(int8_t advance){
  int8_t timingAlphaN = advance;
  if ((configPage2.ignAlgorithm == LOAD_SOURCE_TPS) && (currentStatus.MAP > 100)){
    uint8_t timingCorr = 0;
    static uint8_t startMAP = 100;
    static uint8_t endMAP = 280;
    static uint8_t startTiming = 1;
    static uint8_t endTiming = 8;
    timingCorr = map(currentStatus.MAP, startMAP, endMAP, startTiming, endTiming);
    timingCorr = constrain(timingCorr, startTiming, endTiming);
    timingAlphaN = timingAlphaN - timingCorr;
  }
  return timingAlphaN;
}

void highIdleFunc() {
  //high idle function
  if (( currentStatus.RPM > 1150 ) && ( currentStatus.TPS > 2 )&& (currentStatus.rpmDOT < -200)) 
  {
    alphaVars.highIdleCount++;
    if (alphaVars.highIdleCount >= 2 ) {
      BIT_SET(alphaVars.alphaBools1, BIT_HIGH_IDLE);
    }
  }
  else {
    if (alphaVars.highIdleCount > 0) {
      alphaVars.highIdleCount--;
    }
    else if (alphaVars.highIdleCount == 0)
    {
      BIT_CLEAR(alphaVars.alphaBools1, BIT_HIGH_IDLE);
    }
  }
  alphaVars.highIdleCount = constrain(alphaVars.highIdleCount, 0, 12);
}

void DFCOwaitFunc() {
  //DFCO wait time
  if ( ( currentStatus.RPM > ( configPage4.dfcoRPM * 10) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ) )
  {
    static uint8_t dfcoWaitTime = 2;
    alphaVars.DFCOcounter++;
    if (alphaVars.DFCOcounter > dfcoWaitTime ) {
      BIT_SET(alphaVars.alphaBools1, BIT_DFCO_WAIT);
    }
  }
  else {
    BIT_CLEAR(alphaVars.alphaBools1, BIT_DFCO_WAIT);
    alphaVars.DFCOcounter = 0;
  }
}


void XRSgaugeCLT() {
  //Coolant gauge control
  if (currentStatus.coolant < 50) {
    if (loopCLT < 320) {
      digitalWrite(pinCLTgauge, LOW);
    }
    else {
      digitalWrite(pinCLTgauge, HIGH);
    }
  }
  else if ((currentStatus.coolant >= 50) && (currentStatus.coolant < 70)) {
    if (loopCLT < 225) {
      digitalWrite(pinCLTgauge, LOW);
    }
    else {
      digitalWrite(pinCLTgauge, HIGH);
    }
  }
  else if ((currentStatus.coolant >= 70) && (currentStatus.coolant < 95)) {
    if (loopCLT < 100) {
      digitalWrite(pinCLTgauge, LOW);
    }
    else {
      digitalWrite(pinCLTgauge, HIGH);
    }
  }
  else if ((currentStatus.coolant >= 95) && (currentStatus.coolant < 105)) {
    if (loopCLT <= 75) {
      digitalWrite(pinCLTgauge, LOW);
    }
    else {
      digitalWrite(pinCLTgauge, HIGH);
    }
  }
  else if ((currentStatus.coolant >= 105) && (currentStatus.coolant < 115)) {
    if (loopCLT <= 55) {
      digitalWrite(pinCLTgauge, LOW);
    }
    else {
      digitalWrite(pinCLTgauge, HIGH);
    }
  }
  else if (currentStatus.coolant >= 115)  {
    if (loopCLT <= 35) {
      digitalWrite(pinCLTgauge, LOW);
    }
    else {
      digitalWrite(pinCLTgauge, HIGH);
    }
  }
}

void alphaIdleMods() {
  if ((BIT_CHECK(currentStatus.engine, BIT_ENGINE_ASE))) {
    currentStatus.idleDuty = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  }
  if ((BIT_CHECK(alphaVars.alphaBools1, BIT_HIGH_IDLE)) && (currentStatus.idleDuty < 60)) {
    currentStatus.idleDuty = currentStatus.idleDuty + alphaVars.highIdleCount;
  }
  if (BIT_CHECK(alphaVars.alphaBools1, BIT_AC_REQ)) {
    currentStatus.idleDuty = currentStatus.idleDuty + 10;
  }
  if ((currentStatus.RPM > 1600) && (currentStatus.TPS < 3) && (currentStatus.coolant > 60)){
    currentStatus.idleDuty = 0;
  }
  if (currentStatus.fanOn){
    currentStatus.idleDuty = currentStatus.idleDuty + 2;
  }
}

void RPMdance() {
  //sweep tacho gauge for cool points
  if ((mainLoopCount > 30) && (mainLoopCount < 300))
  {
    tone(pinTachOut, mainLoopCount);
    analogWrite(pinTachOut,  138);
  }
  else if (mainLoopCount == 4999)
  {
    noTone(pinTachOut);
    digitalWrite(pinTachOut, LOW);
    BIT_CLEAR(alphaVars.alphaBools1, BIT_GAUGE_SWEEP);
  }
}

void initialiseAlphaPins(){
	if ((alphaVars.carSelect != 255) && (alphaVars.carSelect != 0)){ // alphamods
    alphaPinSetup();
  }
}


uint16_t WOTdwellCorrection(uint16_t tempDwell) {
  if ((currentStatus.TPS > 80) && (currentStatus.RPM > 3500)) {
    static uint8_t minDwellCorr = 200;
    static uint16_t maxDwellCorr = 500;
    static uint16_t startRPM = 3000;
    static uint16_t endRPM = 5500;
    
    uint16_t dwellCorr = map(currentStatus.RPM, startRPM, endRPM, minDwellCorr, maxDwellCorr);
    dwellCorr = constrain(dwellCorr, minDwellCorr, maxDwellCorr);
    tempDwell = tempDwell - dwellCorr;
  }
  return tempDwell;
}

uint16_t boostAssist(uint16_t tempDuty) {
  if ((currentStatus.TPS > 90) && (currentStatus.MAP < 120)) {
    tempDuty = 9000;
  }
  return tempDuty;
}

static inline int8_t correctionRollingAntiLag(int8_t advance)
{
  uint8_t ignRollingALValue = advance;
  //SoftCut rev limit for 2-step launch control.
  //if (configPage6.launchEnabled && alphaVars.rollingALtrigger && (currentStatus.RPM > 2500) /*&& (currentStatus.TPS >= configPage10.lnchCtrlTPS)*/ )
  /*{
    uint16_t rollingALrpm = currentStatus.RPM + 300;
    alphaVars.rollingALsoft = true;
    ignRollingALValue = map(currentStatus.RPM, rollingALrpm -400, rollingALrpm-100, configPage6.lnchRetard + 15, configPage6.lnchRetard);
    constrain(ignRollingALValue, configPage6.lnchRetard, configPage6.lnchRetard + 15);
    if (currentStatus.RPM > rollingALrpm){ alphaVars.rollingALhard = true;}
    }
    else
    {
    alphaVars.rollingALsoft = false;
    }
  */
  return ignRollingALValue;
}

void ghostCam(){
  if ((currentStatus.coolant > 55) && (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_ASE)) && (currentStatus.TPS < 25) && (currentStatus.RPM > 1000) && (currentStatus.RPM < 4000) && (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) && (alphaVars.carSelect == 5)) {
    if(BIT_CHECK(alphaVars.alphaBools2, BIT_GCAM_STATE)){
      BIT_SET(currentStatus.spark, BIT_SPARK_HRDLIM);
    }
    else{
      BIT_CLEAR(currentStatus.spark, BIT_SPARK_HRDLIM);
    }
  }
}
