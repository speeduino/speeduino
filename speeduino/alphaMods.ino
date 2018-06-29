//#include "alphaMods.h"

//pin setup
void alphaPinSetup(){
  switch (carSelect){
    case 0:
      pinAC; // pin for AC clutch
      pinAcReq;
      pinFan2;
      pinCEL;
      pinVVL;
      pinACpress;
      pinACtemp;
  
    case 1:
      pinAC = 45; // pin for AC clutch
      pinAcReq = 26;
      //pinFan2;
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
      break;
    case 4:
      pinAC = 45; // pin for AC clutch
      pinAcReq = 26;
      pinFan2 = 35;
      pinCEL = 53;
      //pinVVL = 6;
      pinACpress = 28;
      pinACtemp = A5;
      pinOilPress = 30;
      //pinCLTgauge = 2;
      pinMode(pinAC, OUTPUT);
      pinMode(pinAcReq, INPUT);
      pinMode(pinCEL, OUTPUT);
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
  ACOn = false;
}

void fanControl2()
{
  //fan2
    int onTemp = (int)configPage6.fanSP - CALIBRATION_TEMPERATURE_OFFSET;
    int offTemp = onTemp - configPage6.fanHyster;
    if ( ((currentStatus.fanOn) && (currentStatus.coolant >= onTemp+7) && (currentStatus.RPM > 500)) || (AcReq== true) ) { digitalWrite(pinFan2,fanHIGH); currentStatus.fanOn = true; }
    if ( ((!currentStatus.fanOn) && (currentStatus.coolant <= offTemp+7) && (AcReq== false)) || (currentStatus.RPM == 0) ) { digitalWrite(pinFan2, fanLOW); currentStatus.fanOn = false; }
}

void audiFanControl()
{
  if (currentStatus.coolant < 80){
    if (loopCLT < 10){
      digitalWrite(pinFan2,LOW);
    }
    else{
      digitalWrite(pinFan2,HIGH);
    }
  }
  else if ((currentStatus.coolant >= 80) && (currentStatus.coolant < 90)){
    if (loopCLT < 100){
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
  }
}

void ACControl()
{
  if ((AcReq) && (currentStatus.TPS < 60) && (currentStatus.RPM > 600) && (currentStatus.RPM < 3600)){digitalWrite(pinAC, HIGH); ACOn = true;}// turn on AC compressor
  else{ digitalWrite(pinAC, LOW); ACOn = false;} // shut down AC compressor
}

void CELcontrol()
{
  if ((currentStatus.RPM == 0) || (currentStatus.tpsADC > configPage2.tpsMax + 1) || (currentStatus.tpsADC < configPage2.tpsMin - 1)/* || (currentStatus.mapADC > configPage1.mapMax + 1) || (currentStatus.mapADC < configPage1.mapMin - 1)*/ || (currentStatus.RPM > 6400))
  {
    digitalWrite(pinCEL, HIGH);
  }
  else {digitalWrite(pinCEL, LOW);}
}


void vvlControl()
{
  if ((currentStatus.RPM >= 5800) && (currentStatus.TPS > 80) && (currentStatus.coolant > 50))
  {
    if (!vvlOn)
    {
      vvlOn = true;
      digitalWrite(pinVVL, HIGH);
    }
  }
  else if ((currentStatus.RPM <= 5600) && (currentStatus.TPS < 80)) { digitalWrite(pinVVL, LOW);  vvlOn = false;}
}

 void readACReq()
 {
  if (carSelect == 2){
    if ((digitalRead(pinAcReq) == HIGH) && (digitalRead(pinACpress) == LOW)) {AcReq= true;} //pin 26 is AC Request, pin 28 is a combined pressure/temp signal that is high when the A/C compressor can be activated
      else {AcReq= false;}
  }
  else if (carSelect == 1){
    if ((digitalRead(pinAcReq) == HIGH) && (digitalRead(pinACpress) == LOW) && (analogRead(pinACtemp) < 860)) {AcReq= true;} //pin 26 is AC Request, pin 28 is a combined pressure/temp signal that is high when the A/C compressor can be activated
      else {AcReq= false;}
  }
 }

 //Simple correction if VVL is active
 static inline byte correctionVVL()
{
  byte VVLValue = 100;
  if (vvlOn) { VVLValue = 107; } //Adds 7% fuel when VVL is active
  return VVLValue;
}


/*
 * Returns true if decelleration fuel cutoff should be on, false if its off
 */
static inline bool correctionDFCO2()
{
  bool DFCOValue = false;
  if ( configPage2.dfcoEnabled == 1 )
  {
    if ( bitRead(currentStatus.status1, BIT_STATUS1_DFCO) == 1 ) { DFCOValue = ( currentStatus.RPM > ( configPage4.dfcoRPM * 10) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh )&& (DFCOwait)  && (currentStatus.coolant > 60);  }
    else { DFCOValue = ( currentStatus.RPM > (unsigned int)( (configPage4.dfcoRPM * 10) + configPage4.dfcoHyster) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh )&& (DFCOwait)&& (currentStatus.coolant > 60); }  }
  return DFCOValue;
}

static inline int8_t correctionAtUpshift(int8_t advance)
{
  int8_t upshiftAdvance = advance;
  if ((currentStatus.rpmDOT < -500) && (currentStatus.TPS > 90)){
   upshiftAdvance = advance - 10;
  }
  return upshiftAdvance;
}

static inline int8_t correctionZeroThrottleTiming(int8_t advance)
{
  int8_t ignZeroThrottleValue = advance;
  if ((currentStatus.TPS < 2) && !(BIT_CHECK(currentStatus.engine, BIT_ENGINE_ASE))) //Check whether TPS coorelates to zero value
  {
     if ((currentStatus.RPM > 500) && (currentStatus.RPM <= 800)) { 
      ignZeroThrottleValue = map(currentStatus.RPM, 500, 800, 25, 9);
     }
     else if ((currentStatus.RPM > 800) && (currentStatus.RPM < 1600)) { 
      ignZeroThrottleValue = map(currentStatus.RPM, 800, 1200, 9, 0);
     }
     else{ignZeroThrottleValue = advance;}
    ignZeroThrottleValue = constrain(ignZeroThrottleValue , 0, 25);
     if ((currentStatus.RPM > 3000) && (currentStatus.RPM < 5500)){ ignZeroThrottleValue = -5;}
     
  }
  else if ((currentStatus.TPS < 2) && (BIT_CHECK(currentStatus.engine, BIT_ENGINE_ASE))){
    ignZeroThrottleValue = 10;
  }
  if ((ACOn == true) && (currentStatus.RPM < 3000) && (currentStatus.TPS < 30)) {ignZeroThrottleValue = ignZeroThrottleValue + 2;}
  return ignZeroThrottleValue;
}

void highIdleFunc(){
  //high idle function
    if ( (( currentStatus.RPM > 950 ) && ( currentStatus.TPS > 7 )) || ((currentStatus.RPM > 1150) && (currentStatus.rpmDOT < -50)) ) 
    {
      highIdleCount++;
      if (highIdleCount <= 1 ) {highIdleReq = true;}
      if (highIdleCount > 3) { highIdleReq = 3;}
    }
    else { 
      if (highIdleCount > 0){
        highIdleCount--;
      }
      else if(highIdleCount == 0)
      {
       highIdleReq = false; 
      }
     }
}

void DFCOwaitFunc(){
      //DFCO wait time
    if ( ( currentStatus.RPM > ( configPage4.dfcoRPM * 10) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ) ) 
    {
      DFCOcounter++;
      if (DFCOcounter > 2 ) {DFCOwait = true;}
    }
    else {DFCOwait = false; DFCOcounter = 0;}
}


void XRSgaugeCLT(){
//Coolant gauge control
  if (currentStatus.coolant < 40){
    if (loopCLT < 220){
      digitalWrite(pinCLTgauge, LOW);
    }
    else {digitalWrite(pinCLTgauge, HIGH);}
   }
  else if ((currentStatus.coolant >= 40) && (currentStatus.coolant < 60)){
      if (loopCLT < 165){
        digitalWrite(pinCLTgauge, LOW);
      }
      else {digitalWrite(pinCLTgauge, HIGH);}
  }
  else if ((currentStatus.coolant >= 60) && (currentStatus.coolant < 90)){ 
    if (loopCLT < 100){
        digitalWrite(pinCLTgauge, LOW);
      }
      else {digitalWrite(pinCLTgauge, HIGH);}
    }
  else if ((currentStatus.coolant >= 90) && (currentStatus.coolant < 140)){
      if (loopCLT <= 35){
        digitalWrite(pinCLTgauge, LOW);
      }
      else {digitalWrite(pinCLTgauge, HIGH);}
  }
}
    

