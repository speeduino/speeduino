//# 1 "/tmp/tmpT4476z"
#include <Arduino.h>
#ifndef UNIT_TEST

#include <stdint.h>

#include "globals.h"
#include "speeduino.h"
#include "table.h"
#include "scheduler.h"
#include "comms.h"
#include "cancomms.h"
#include "maths.h"
#include "corrections.h"
#include "timers.h"
#include "decoders.h"
#include "idle.h"
#include "auxiliaries.h"
#include "sensors.h"
#include "storage.h"
#include "crankMaths.h"
#include "init.h"
#include BOARD_H
#if defined (CORE_TEENSY)
#include <FlexCAN.h>
#endif
void setup();
void loop();
uint16_t PW(int REQ_FUEL, byte VE, long MAP, int corrections, int injOpen);
byte getVE();
byte getVE2();
byte getAdvance();
uint16_t calculateInjector2StartAngle(unsigned int PWdivTimerPerDegree);
uint16_t calculateInjector3StartAngle(unsigned int PWdivTimerPerDegree);
uint16_t calculateInjector4StartAngle(unsigned int PWdivTimerPerDegree);
uint16_t calculateInjector5StartAngle(unsigned int PWdivTimerPerDegree);
void initialiseFan();
void fanControl();
void initialiseAuxPWM();
void boostControl();
void vvtControl();
void nitrousControl();
void boostDisable();
void ftm1_isr(void);
void initBoard();
uint16_t freeRam();
void initBoard();
uint16_t freeRam();
void initBoard();
uint16_t freeRam();
void initBoard();
uint16_t freeRam();
void canCommand();
void sendcanValues(uint16_t offset, uint16_t packetLength, byte cmd, byte portType);
void sendCancommand(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress);
void command();
void sendValues(uint16_t offset, uint16_t packetLength, byte cmd, byte portNum);
void sendValuesLegacy();
void receiveValue(uint16_t valueOffset, byte newValue);
void sendPage();
void sendPageASCII();
byte getPageValue(byte page, uint16_t valueAddress);
void receiveCalibration(byte tableID);
void sendToothLog(bool useChar);
void sendCompositeLog();
void testComm();
void commandButtons(int buttonCommand);
void initialiseCorrections();
int8_t correctionsIgn(int8_t base_advance);
uint16_t correctionsDwell(uint16_t dwell);
unsigned long angleToTime(int16_t angle, byte method);
uint16_t timeToAngle(unsigned long time, byte method);
void doCrankSpeedCalcs();
void loggerPrimaryISR();
void loggerSecondaryISR();
void triggerSetup_missingTooth();
void triggerPri_missingTooth();
void triggerSec_missingTooth();
uint16_t getRPM_missingTooth();
int getCrankAngle_missingTooth();
void triggerSetEndTeeth_missingTooth();
void triggerSetup_DualWheel();
void triggerPri_DualWheel();
void triggerSec_DualWheel();
uint16_t getRPM_DualWheel();
int getCrankAngle_DualWheel();
void triggerSetEndTeeth_DualWheel();
void triggerSetup_BasicDistributor();
void triggerPri_BasicDistributor();
void triggerSec_BasicDistributor();
uint16_t getRPM_BasicDistributor();
int getCrankAngle_BasicDistributor();
void triggerSetEndTeeth_BasicDistributor();
void triggerSetup_GM7X();
void triggerPri_GM7X();
void triggerSec_GM7X();
uint16_t getRPM_GM7X();
int getCrankAngle_GM7X();
void triggerSetEndTeeth_GM7X();
void triggerSetup_4G63();
void triggerPri_4G63();
void triggerSec_4G63();
uint16_t getRPM_4G63();
int getCrankAngle_4G63();
void triggerSetEndTeeth_4G63();
void triggerSetup_24X();
void triggerPri_24X();
void triggerSec_24X();
uint16_t getRPM_24X();
int getCrankAngle_24X();
void triggerSetEndTeeth_24X();
void triggerSetup_Jeep2000();
void triggerPri_Jeep2000();
void triggerSec_Jeep2000();
uint16_t getRPM_Jeep2000();
int getCrankAngle_Jeep2000();
void triggerSetEndTeeth_Jeep2000();
void triggerSetup_Audi135();
void triggerPri_Audi135();
void triggerSec_Audi135();
uint16_t getRPM_Audi135();
int getCrankAngle_Audi135();
void triggerSetEndTeeth_Audi135();
void triggerSetup_HondaD17();
void triggerPri_HondaD17();
void triggerSec_HondaD17();
uint16_t getRPM_HondaD17();
int getCrankAngle_HondaD17();
void triggerSetEndTeeth_HondaD17();
void triggerSetup_Miata9905();
void triggerPri_Miata9905();
void triggerSec_Miata9905();
uint16_t getRPM_Miata9905();
int getCrankAngle_Miata9905();
void triggerSetEndTeeth_Miata9905();
void triggerSetup_MazdaAU();
void triggerPri_MazdaAU();
void triggerSec_MazdaAU();
uint16_t getRPM_MazdaAU();
int getCrankAngle_MazdaAU();
void triggerSetEndTeeth_MazdaAU();
void triggerSetup_non360();
void triggerPri_non360();
void triggerSec_non360();
uint16_t getRPM_non360();
int getCrankAngle_non360();
void triggerSetEndTeeth_non360();
void triggerSetup_Nissan360();
void triggerPri_Nissan360();
void triggerSec_Nissan360();
uint16_t getRPM_Nissan360();
int getCrankAngle_Nissan360();
void triggerSetEndTeeth_Nissan360();
void triggerSetup_Subaru67();
void triggerPri_Subaru67();
void triggerSec_Subaru67();
uint16_t getRPM_Subaru67();
int getCrankAngle_Subaru67();
void triggerSetEndTeeth_Subaru67();
void triggerSetup_Daihatsu();
void triggerPri_Daihatsu();
void triggerSec_Daihatsu();
uint16_t getRPM_Daihatsu();
int getCrankAngle_Daihatsu();
void triggerSetEndTeeth_Daihatsu();
void triggerSetup_Harley();
void triggerPri_Harley();
void triggerSec_Harley();
uint16_t getRPM_Harley();
int getCrankAngle_Harley();
void triggerSetEndTeeth_Harley();
void triggerSetup_ThirtySixMinus222();
void triggerPri_ThirtySixMinus222();
void triggerSec_ThirtySixMinus222();
int getCrankAngle_ThirtySixMinus222();
void triggerSetEndTeeth_ThirtySixMinus222();
void initialiseDisplay();
void updateDisplay();
byte setError(byte errorID);
void clearError(byte errorID);
byte getNextError();
void initialiseIdle();
void idleControl();
void ftm2_isr(void);
void initialiseAll();
void setPinMapping(byte boardID);
void initialiseTriggers();
int fastMap(unsigned long x, int in_min, int in_max, int out_min, int out_max);
unsigned int divu10(unsigned int n);
int divs10(long n);
int divs100(long n);
unsigned long divu100(unsigned long n);
unsigned long percentage(byte x, unsigned long y);
inline long powint(int factor, unsigned int exponent);
inline void beginCoil1Charge();
inline void endCoil1Charge();
inline void beginCoil2Charge();
inline void endCoil2Charge();
inline void beginCoil3Charge();
inline void endCoil3Charge();
inline void beginCoil4Charge();
inline void endCoil4Charge();
inline void beginCoil5Charge();
inline void endCoil5Charge();
inline void beginCoil6Charge();
inline void endCoil6Charge();
inline void beginCoil7Charge();
inline void endCoil7Charge();
inline void beginCoil8Charge();
inline void endCoil8Charge();
inline void beginTrailingCoilCharge();
inline void endTrailingCoilCharge1();
inline void endTrailingCoilCharge2();
void beginCoil1and3Charge();
void endCoil1and3Charge();
void beginCoil2and4Charge();
void endCoil2and4Charge();
void nullCallback();
void initialiseSchedulers();
void setFuelSchedule(struct Schedule *targetSchedule, unsigned long timeout, unsigned long duration);
void setFuelSchedule1(unsigned long timeout, unsigned long duration);
void setFuelSchedule2(unsigned long timeout, unsigned long duration);
void setFuelSchedule3(unsigned long timeout, unsigned long duration);
void setFuelSchedule4(unsigned long timeout, unsigned long duration);
void setFuelSchedule6(unsigned long timeout, unsigned long duration);
void setFuelSchedule7(unsigned long timeout, unsigned long duration);
void setFuelSchedule8(unsigned long timeout, unsigned long duration);
void ftm0_isr(void);
void ftm3_isr(void);
void initialiseADC();
void readTPS();
void readCLT(bool useFilter);
void readIAT();
void readBaro();
void readO2();
void readO2_2();
void readBat();
void flexPulse();
void knockPulse();
uint16_t readAuxanalog(uint8_t analogPin);
uint16_t readAuxdigital(uint8_t digitalPin);
void writeAllConfig();
void writeConfig(byte tableNum);
void loadConfig();
void loadCalibration();
void writeCalibration();
void storePageCRC32(byte pageNo, uint32_t crc32_val);
uint32_t readPageCRC32(byte pageNo);
byte readLastBaro();
void storeLastBaro(byte newValue);
void storeCalibrationValue(uint16_t location, byte value);
byte readEEPROMVersion();
void storeEEPROMVersion(byte newVersion);
void table2D_setSize(struct table2D* targetTable, byte newSize);
void table3D_setSize(struct table3D *targetTable, byte newSize);
int table2D_getValue(struct table2D *fromTable, int X_in);
int get3DTableValue(struct table3D *fromTable, int Y_in, int X_in);
void initialiseTimers();
void doUpdates();
byte pinTranslate(byte rawPin);
void setResetControlPinState();
uint32_t calculateCRC32(byte pageNo);
//#line 45 "/home/developper/speeduino/speeduino/speeduino.ino"
void setup()
{
  initialiseAll();
}

void loop()
{
      mainLoopCount++;
      LOOP_TIMER = TIMER_mask;




      if ( ((mainLoopCount & 31) == 1) or (Serial.available() > SERIAL_BUFFER_THRESHOLD) )
      {
        if (Serial.available() > 0) { command(); }
        else if(cmdPending == true)
        {

          if (currentCommand == 'T') { command(); }
        }

      }

      if (configPage9.enable_secondarySerial == 1)
          {
            if ( ((mainLoopCount & 31) == 1) or (CANSerial.available() > SERIAL_BUFFER_THRESHOLD) )
                {
                  if (CANSerial.available() > 0) { canCommand(); }
                }
          }
      #if defined(CORE_TEENSY) || defined(CORE_STM32)
          else if (configPage9.enable_secondarySerial == 2)
          {





          }
      #endif




    previousLoopTime = currentLoopTime;
    currentLoopTime = micros_safe();
    unsigned long timeToLastTooth = (currentLoopTime - toothLastToothTime);
    if ( (timeToLastTooth < MAX_STALL_TIME) || (toothLastToothTime > currentLoopTime) )
    {
      currentStatus.longRPM = getRPM();
      currentStatus.RPM = currentStatus.longRPM;
      FUEL_PUMP_ON();
      currentStatus.fuelPumpOn = true;
    }
    else
    {

      currentStatus.RPM = 0;
      currentStatus.PW1 = 0;
      currentStatus.VE = 0;
      currentStatus.VE2 = 0;
      toothLastToothTime = 0;
      toothLastSecToothTime = 0;

      currentStatus.hasSync = false;
      currentStatus.runSecs = 0;
      currentStatus.startRevolutions = 0;
      toothSystemCount = 0;
      secondaryToothCount = 0;
      MAPcurRev = 0;
      MAPcount = 0;
      currentStatus.rpmDOT = 0;
      AFRnextCycle = 0;
      ignitionCount = 0;
      ignitionOn = false;
      fuelOn = false;
      if (fpPrimed == true) { FUEL_PUMP_OFF(); currentStatus.fuelPumpOn = false; }
      disableIdle();
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_WARMUP);
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN);
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ASE);


      initialiseTriggers();

      VVT_PIN_LOW();
      DISABLE_VVT_TIMER();
      boostDisable();
    }



    readMAP();

    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_15HZ);
      readTPS();


      previousClutchTrigger = clutchTrigger;
      if(configPage6.launchHiLo > 0) { clutchTrigger = digitalRead(pinLaunch); }
      else { clutchTrigger = !digitalRead(pinLaunch); }

      if(previousClutchTrigger != clutchTrigger) { currentStatus.clutchEngagedRPM = currentStatus.RPM; }

      if (configPage6.launchEnabled && clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > ((unsigned int)(configPage6.lnchHardLim) * 100)) && (currentStatus.TPS >= configPage10.lnchCtrlTPS) )
      {

        currentStatus.launchingHard = true;
        BIT_SET(currentStatus.spark, BIT_SPARK_HLAUNCH);
      }
      else { currentStatus.launchingHard = false; BIT_CLEAR(currentStatus.spark, BIT_SPARK_HLAUNCH); }

      if(configPage6.flatSEnable && clutchTrigger && (currentStatus.RPM > ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > currentStatus.clutchEngagedRPM) ) { currentStatus.flatShiftingHard = true; }
      else { currentStatus.flatShiftingHard = false; }


      if( (configPage6.boostCutType > 0) && (currentStatus.MAP > (configPage6.boostLimit * 2)) )
      {
        switch(configPage6.boostCutType)
        {
          case 1:
            BIT_SET(currentStatus.spark, BIT_SPARK_BOOSTCUT);
            BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
            break;
          case 2:
            BIT_SET(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
            BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
            break;
          case 3:
            BIT_SET(currentStatus.spark, BIT_SPARK_BOOSTCUT);
            BIT_SET(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
            break;
          default:

            BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
            BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
        }
      }
      else
      {
        BIT_CLEAR(currentStatus.spark, BIT_SPARK_BOOSTCUT);
        BIT_CLEAR(currentStatus.status1, BIT_STATUS1_BOOSTCUT);
      }


      if(toothHistoryIndex > TOOTH_LOG_SIZE) { BIT_SET(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY); }

    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_30HZ);

      boostControl();
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_4HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_4HZ);

      readCLT();
      readIAT();
      readO2();
      readO2_2();
      readBat();
      nitrousControl();

      if(eepromWritesPending == true) { writeAllConfig(); }

      if(auxIsEnabled == true)
      {


        for (byte AuxinChan = 0; AuxinChan <16 ; AuxinChan++)
        {
          currentStatus.current_caninchannel = AuxinChan;

          if (((configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 4)
              && (((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 0)&&(configPage9.intcan_available == 1)))
              || ((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 1))&&
              ((configPage9.caninput_sel[currentStatus.current_caninchannel]&64) == 0))
              || ((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 0)))))
          {


            if (configPage9.enable_secondarySerial == 1)
            {
              sendCancommand(2,0,currentStatus.current_caninchannel,0,((configPage9.caninput_source_can_address[currentStatus.current_caninchannel]&2047)+0x100));

            }
          }
          else if (((configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 4)
              && (((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 1))&&
              ((configPage9.caninput_sel[currentStatus.current_caninchannel]&64) == 64))
              || ((configPage9.enable_secondarySerial == 0) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 1))&&
              ((configPage9.caninput_sel[currentStatus.current_caninchannel]&128) == 128))))
          {


          #if defined(CORE_STM32) || defined(CORE_TEENSY)
           if (configPage9.enable_intcan == 1)
           {
              sendCancommand(3,configPage9.speeduino_tsCanId,currentStatus.current_caninchannel,0,((configPage9.caninput_source_can_address[currentStatus.current_caninchannel]&2047)+0x100));

           }
          #endif
          }
          else if ((((configPage9.enable_secondarySerial == 1) || ((configPage9.enable_intcan == 1) && (configPage9.intcan_available == 1))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 8)
                  || (((configPage9.enable_secondarySerial == 0) && ( (configPage9.enable_intcan == 1) && (configPage9.intcan_available == 0) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 2)
                  || (((configPage9.enable_secondarySerial == 0) && (configPage9.enable_intcan == 0)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 2)))
          {


            currentStatus.canin[currentStatus.current_caninchannel] = readAuxanalog(configPage9.Auxinpina[currentStatus.current_caninchannel]&63);
          }
          else if ((((configPage9.enable_secondarySerial == 1) || ((configPage9.enable_intcan == 1) && (configPage9.intcan_available == 1))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 12)
                  || (((configPage9.enable_secondarySerial == 0) && ( (configPage9.enable_intcan == 1) && (configPage9.intcan_available == 0) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 3)
                  || (((configPage9.enable_secondarySerial == 0) && (configPage9.enable_intcan == 0)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 3)))
          {


            currentStatus.canin[currentStatus.current_caninchannel] = readAuxdigital((configPage9.Auxinpinb[currentStatus.current_caninchannel]&63)+1);
          }
        }
      }

       vvtControl();
       idleControl();
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_1HZ);
      readBaro();
    }

    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL) ) { idleControl(); }

    byte totalVE = 0;

    currentStatus.VE = getVE();


    if(configPage10.fuel2Mode > 0)
    {
      currentStatus.VE2 = getVE2();

      if(configPage10.fuel2Mode == FUEL2_MODE_MULTIPLY)
      {

        totalVE = ((uint16_t)currentStatus.VE * (uint16_t)currentStatus.VE2) / 100;
      }
      else if(configPage10.fuel2Mode == FUEL2_MODE_ADD)
      {

        uint16_t combinedVE = (uint16_t)currentStatus.VE + (uint16_t)currentStatus.VE2;
        if(combinedVE <= 255) { totalVE = combinedVE; }
        else { totalVE = 255; }
      }
      else if(configPage10.fuel2Mode == FUEL2_MODE_SWITCH)
      {

      }
    }
    else { totalVE = currentStatus.VE; }



    if (currentStatus.hasSync && (currentStatus.RPM > 0))
    {
        if(currentStatus.startRevolutions >= configPage4.StgCycles) { ignitionOn = true; fuelOn = true; }

        if(currentStatus.RPM > currentStatus.crankRPM)
        {
          BIT_SET(currentStatus.engine, BIT_ENGINE_RUN);

          if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
          {
            BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
            if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, HIGH); }
          }
        }
        else
        {

          BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
          BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN);
          currentStatus.runSecs = 0;
          if(configPage4.ignBypassEnabled > 0) { digitalWrite(pinIgnBypass, LOW); }
        }





      currentStatus.corrections = correctionsFuel();

      currentStatus.advance = getAdvance();

      currentStatus.PW1 = PW(req_fuel_uS, totalVE, currentStatus.MAP, currentStatus.corrections, inj_opentime_uS);


      if(currentStatus.nitrous_status == NITROUS_STAGE1)
      {
        int16_t adderRange = (configPage10.n2o_stage1_maxRPM - configPage10.n2o_stage1_minRPM) * 100;
        int16_t adderPercent = ((currentStatus.RPM - (configPage10.n2o_stage1_minRPM * 100)) * 100) / adderRange;
        adderPercent = 100 - adderPercent;
        currentStatus.PW1 = currentStatus.PW1 + (configPage10.n2o_stage1_adderMax + percentage(adderPercent, (configPage10.n2o_stage1_adderMin - configPage10.n2o_stage1_adderMax))) * 100;
      }
      if(currentStatus.nitrous_status == NITROUS_STAGE2)
      {
        int16_t adderRange = (configPage10.n2o_stage2_maxRPM - configPage10.n2o_stage2_minRPM) * 100;
        int16_t adderPercent = ((currentStatus.RPM - (configPage10.n2o_stage2_minRPM * 100)) * 100) / adderRange;
        adderPercent = 100 - adderPercent;
        currentStatus.PW1 = currentStatus.PW1 + (configPage10.n2o_stage2_adderMax + percentage(adderPercent, (configPage10.n2o_stage2_adderMin - configPage10.n2o_stage2_adderMax))) * 100;
      }

      int injector1StartAngle = 0;
      uint16_t injector2StartAngle = 0;
      uint16_t injector3StartAngle = 0;
      uint16_t injector4StartAngle = 0;
      #if INJ_CHANNELS >= 5
      uint16_t injector5StartAngle = 0;
      #endif
      #if INJ_CHANNELS >= 6
      int injector6StartAngle = 0;
      #endif
      #if INJ_CHANNELS >= 7
      int injector7StartAngle = 0;
      #endif
      #if INJ_CHANNELS >= 8
      int injector8StartAngle = 0;
      #endif
      int ignition1StartAngle = 0;
      int ignition2StartAngle = 0;
      int ignition3StartAngle = 0;
      int ignition4StartAngle = 0;
      int ignition5StartAngle = 0;
      #if IGN_CHANNELS >= 6
      int ignition6StartAngle = 0;
      #endif
      #if IGN_CHANNELS >= 7
      int ignition7StartAngle = 0;
      #endif
      #if IGN_CHANNELS >= 8
      int ignition8StartAngle = 0;
      #endif


      int tempCrankAngle;
      int tempStartAngle;

      doCrankSpeedCalcs();


      unsigned long pwLimit = percentage(configPage2.dutyLim, revolutionTime);

      if (configPage2.strokes == FOUR_STROKE) { pwLimit = pwLimit * 2 / currentStatus.nSquirts; }
      else { pwLimit = pwLimit / currentStatus.nSquirts; }

      if( (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) && (configPage10.stagingEnabled == false) ) { if (currentStatus.PW1 > pwLimit) { currentStatus.PW1 = pwLimit; } }



      if( (configPage10.stagingEnabled == true) && (configPage2.nCylinders <= INJ_CHANNELS) )
      {

        currentStatus.PW1 -= inj_opentime_uS;
        uint32_t tempPW1 = (((unsigned long)currentStatus.PW1 * staged_req_fuel_mult_pri) / 100) + inj_opentime_uS;

        if(configPage10.stagingMode == STAGING_MODE_TABLE)
        {
          uint32_t tempPW3 = (((unsigned long)currentStatus.PW1 * staged_req_fuel_mult_sec) / 100) + inj_opentime_uS;

          byte stagingSplit = get3DTableValue(&stagingTable, currentStatus.MAP, currentStatus.RPM);
          currentStatus.PW1 = ((100 - stagingSplit) * tempPW1) / 100;

          if(stagingSplit > 0) { currentStatus.PW3 = (stagingSplit * tempPW3) / 100; }
          else { currentStatus.PW3 = 0; }
        }
        else if(configPage10.stagingMode == STAGING_MODE_AUTO)
        {
          currentStatus.PW1 = tempPW1;


          if(tempPW1 > pwLimit)
          {
            uint32_t extraPW = tempPW1 - pwLimit;
            currentStatus.PW1 = pwLimit;
            currentStatus.PW3 = ((extraPW * staged_req_fuel_mult_sec) / staged_req_fuel_mult_pri) + inj_opentime_uS;
          }
          else { currentStatus.PW3 = 0; }
        }


        currentStatus.PW2 = currentStatus.PW1;
        currentStatus.PW4 = currentStatus.PW3;
      }
      else
      {

        currentStatus.PW2 = currentStatus.PW1;
        currentStatus.PW3 = currentStatus.PW1;
        currentStatus.PW4 = currentStatus.PW1;
        currentStatus.PW5 = currentStatus.PW1;
        currentStatus.PW6 = currentStatus.PW1;
        currentStatus.PW7 = currentStatus.PW1;
      }




      if(!configPage2.indInjAng)
      {

        configPage2.inj2Ang = configPage2.inj1Ang;
        configPage2.inj3Ang = configPage2.inj1Ang;
        configPage2.inj4Ang = configPage2.inj1Ang;
      }
      unsigned int PWdivTimerPerDegree = div(currentStatus.PW1, timePerDegree).quot;

      if(configPage2.inj1Ang > PWdivTimerPerDegree) { injector1StartAngle = configPage2.inj1Ang - ( PWdivTimerPerDegree ); }
      else { injector1StartAngle = configPage2.inj1Ang + CRANK_ANGLE_MAX_INJ - PWdivTimerPerDegree; }
      while(injector1StartAngle > CRANK_ANGLE_MAX_INJ) { injector1StartAngle -= CRANK_ANGLE_MAX_INJ; }


      switch (configPage2.nCylinders)
      {

        case 1:

          if( (configPage10.stagingEnabled == true) && (currentStatus.PW3 > 0) )
          {
            PWdivTimerPerDegree = div(currentStatus.PW3, timePerDegree).quot;
            injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
          }
          break;

        case 2:
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          if( (configPage10.stagingEnabled == true) && (currentStatus.PW3 > 0) )
          {
            PWdivTimerPerDegree = div(currentStatus.PW3, timePerDegree).quot;
            injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);

            injector4StartAngle = injector3StartAngle + (CRANK_ANGLE_MAX_INJ / 2);
            if(injector4StartAngle < 0) {injector4StartAngle += CRANK_ANGLE_MAX_INJ;}
            if(injector4StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { injector4StartAngle -= CRANK_ANGLE_MAX_INJ; }
          }
          break;

        case 3:
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
          break;

        case 4:
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);

          if(configPage2.injLayout == INJ_SEQUENTIAL)
          {
            injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
            injector4StartAngle = calculateInjector4StartAngle(PWdivTimerPerDegree);

            if(configPage6.fuelTrimEnabled > 0)
            {
              unsigned long pw1percent = 100 + (byte)get3DTableValue(&trim1Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;
              unsigned long pw2percent = 100 + (byte)get3DTableValue(&trim2Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;
              unsigned long pw3percent = 100 + (byte)get3DTableValue(&trim3Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;
              unsigned long pw4percent = 100 + (byte)get3DTableValue(&trim4Table, currentStatus.MAP, currentStatus.RPM) - OFFSET_FUELTRIM;

              if (pw1percent != 100) { currentStatus.PW1 = (pw1percent * currentStatus.PW1) / 100; }
              if (pw2percent != 100) { currentStatus.PW2 = (pw2percent * currentStatus.PW2) / 100; }
              if (pw3percent != 100) { currentStatus.PW3 = (pw3percent * currentStatus.PW3) / 100; }
              if (pw4percent != 100) { currentStatus.PW4 = (pw4percent * currentStatus.PW4) / 100; }
            }
          }
          else if( (configPage10.stagingEnabled == true) && (currentStatus.PW3 > 0) )
          {
            PWdivTimerPerDegree = div(currentStatus.PW3, timePerDegree).quot;
            injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);

            injector4StartAngle = injector3StartAngle + (CRANK_ANGLE_MAX_INJ / 2);
            if(injector4StartAngle < 0) {injector4StartAngle += CRANK_ANGLE_MAX_INJ;}
            if(injector4StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { injector4StartAngle -= CRANK_ANGLE_MAX_INJ; }
          }
          break;

        case 5:
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
          injector4StartAngle = calculateInjector4StartAngle(PWdivTimerPerDegree);
          #if INJ_CHANNELS >= 5
            injector5StartAngle = calculateInjector5StartAngle(PWdivTimerPerDegree);
          #endif
          break;

        case 6:
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
          #if INJ_CHANNELS >= 6
            if(configPage2.injLayout == INJ_SEQUENTIAL)
            {
              injector4StartAngle = (configPage2.inj1Ang + channel4InjDegrees - ( PWdivTimerPerDegree ));
              if(injector4StartAngle > CRANK_ANGLE_MAX_INJ) {injector4StartAngle -= CRANK_ANGLE_MAX_INJ;}
              injector5StartAngle = (configPage2.inj2Ang + channel5InjDegrees - ( PWdivTimerPerDegree ));
              if(injector5StartAngle > CRANK_ANGLE_MAX_INJ) {injector5StartAngle -= CRANK_ANGLE_MAX_INJ;}
              injector6StartAngle = (configPage2.inj3Ang + channel6InjDegrees - ( PWdivTimerPerDegree ));
              if(injector6StartAngle > CRANK_ANGLE_MAX_INJ) {injector6StartAngle -= CRANK_ANGLE_MAX_INJ;}
            }
          #endif
          break;

        case 8:
          injector2StartAngle = calculateInjector2StartAngle(PWdivTimerPerDegree);
          injector3StartAngle = calculateInjector3StartAngle(PWdivTimerPerDegree);
          injector4StartAngle = calculateInjector4StartAngle(PWdivTimerPerDegree);
          break;


        default:
          break;
      }



      if (currentStatus.RPM > ((unsigned int)(configPage4.HardRevLim) * 100) ) { BIT_SET(currentStatus.spark, BIT_SPARK_HRDLIM); }
      else { BIT_CLEAR(currentStatus.spark, BIT_SPARK_HRDLIM); }




      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { currentStatus.dwell = (configPage4.dwellCrank * 100); }
      else { currentStatus.dwell = (configPage4.dwellRun * 100); }
      currentStatus.dwell = correctionsDwell(currentStatus.dwell);

      int dwellAngle = timeToAngle(currentStatus.dwell, CRANKMATH_METHOD_INTERVAL_REV);



      ignition1EndAngle = CRANK_ANGLE_MAX_IGN - currentStatus.advance;
      if(ignition1EndAngle > CRANK_ANGLE_MAX_IGN) {ignition1EndAngle -= CRANK_ANGLE_MAX_IGN;}
      ignition1StartAngle = ignition1EndAngle - dwellAngle;
      if(ignition1StartAngle < 0) {ignition1StartAngle += CRANK_ANGLE_MAX_IGN;}


      switch (configPage2.nCylinders)
      {

        case 2:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}
          break;

        case 3:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
          if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition3StartAngle = channel3IgnDegrees - dwellAngle;
          if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}
          break;

        case 4:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
          {
            ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
            if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
            ignition3StartAngle = ignition3EndAngle - dwellAngle;
            if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

            ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
            if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
            ignition4StartAngle = ignition4EndAngle - dwellAngle;
            if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
          }
          else if(configPage4.sparkMode == IGN_MODE_ROTARY)
          {
            byte splitDegrees = 0;
            if (configPage2.fuelAlgorithm == LOAD_SOURCE_MAP) { splitDegrees = table2D_getValue(&rotarySplitTable, currentStatus.MAP/2); }
            else { splitDegrees = table2D_getValue(&rotarySplitTable, currentStatus.TPS/2); }


            ignition3EndAngle = ignition1EndAngle + splitDegrees;
            ignition3StartAngle = ignition3EndAngle - dwellAngle;
            if(ignition3StartAngle > CRANK_ANGLE_MAX_IGN) {ignition3StartAngle -= CRANK_ANGLE_MAX_IGN;}
            if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

            ignition4EndAngle = ignition2EndAngle + splitDegrees;
            ignition4StartAngle = ignition4EndAngle - dwellAngle;
            if(ignition4StartAngle > CRANK_ANGLE_MAX_IGN) {ignition4StartAngle -= CRANK_ANGLE_MAX_IGN;}
            if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
          }
          break;

        case 5:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
          if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition3StartAngle = ignition3EndAngle - dwellAngle;
          if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
          if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition4StartAngle = ignition4EndAngle - dwellAngle;
          if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition5EndAngle = channel5IgnDegrees - currentStatus.advance - dwellAngle;
          if(ignition5EndAngle > CRANK_ANGLE_MAX_IGN) {ignition5EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition5StartAngle = ignition5EndAngle - dwellAngle;
          if(ignition5StartAngle < 0) {ignition5StartAngle += CRANK_ANGLE_MAX_IGN;}

          break;

        case 6:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
          if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition3StartAngle = ignition3EndAngle - dwellAngle;
          if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}
          break;

        case 8:
          ignition2EndAngle = channel2IgnDegrees - currentStatus.advance;
          if(ignition2EndAngle > CRANK_ANGLE_MAX_IGN) {ignition2EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition2StartAngle = ignition2EndAngle - dwellAngle;
          if(ignition2StartAngle < 0) {ignition2StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition3EndAngle = channel3IgnDegrees - currentStatus.advance;
          if(ignition3EndAngle > CRANK_ANGLE_MAX_IGN) {ignition3EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition3StartAngle = ignition3EndAngle - dwellAngle;
          if(ignition3StartAngle < 0) {ignition3StartAngle += CRANK_ANGLE_MAX_IGN;}

          ignition4EndAngle = channel4IgnDegrees - currentStatus.advance;
          if(ignition4EndAngle > CRANK_ANGLE_MAX_IGN) {ignition4EndAngle -= CRANK_ANGLE_MAX_IGN;}
          ignition4StartAngle = ignition4EndAngle - dwellAngle;
          if(ignition4StartAngle < 0) {ignition4StartAngle += CRANK_ANGLE_MAX_IGN;}
          break;


        default:
          break;
      }


      if( (configPage2.perToothIgn == true) && (lastToothCalcAdvance != currentStatus.advance) ) { triggerSetEndTeeth(); }
//# 719 "/home/developper/speeduino/speeduino/speeduino.ino"
      int crankAngle = getCrankAngle();
      while(crankAngle > CRANK_ANGLE_MAX_INJ ) { crankAngle = crankAngle - CRANK_ANGLE_MAX_INJ; }

      if(Serial && false)
      {
        if(ignition1StartAngle > crankAngle)
        {
          noInterrupts();
          Serial.print("Time2LastTooth:"); Serial.println(micros()-toothLastToothTime);
          Serial.print("elapsedTime:"); Serial.println(elapsedTime);
          Serial.print("CurAngle:"); Serial.println(crankAngle);
          Serial.print("RPM:"); Serial.println(currentStatus.RPM);
          Serial.print("Tooth:"); Serial.println(toothCurrentCount);
          Serial.print("timePerDegree:"); Serial.println(timePerDegree);
          Serial.print("IGN1Angle:"); Serial.println(ignition1StartAngle);
          Serial.print("TimeToIGN1:"); Serial.println(angleToTime((ignition1StartAngle - crankAngle), CRANKMATH_METHOD_INTERVAL_REV));
          interrupts();
        }
      }

#if INJ_CHANNELS >= 1
      if (fuelOn && !BIT_CHECK(currentStatus.status1, BIT_STATUS1_BOOSTCUT))
      {
        if(currentStatus.PW1 >= inj_opentime_uS)
        {
          if ( (injector1StartAngle <= crankAngle) && (fuelSchedule1.Status == RUNNING) ) { injector1StartAngle += CRANK_ANGLE_MAX_INJ; }
          if (injector1StartAngle > crankAngle)
          {
            setFuelSchedule1(
                      ((injector1StartAngle - crankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW1
                      );
          }
        }
#endif
//# 766 "/home/developper/speeduino/speeduino/speeduino.ino"
#if INJ_CHANNELS >= 2
        if( (channel2InjEnabled) && (currentStatus.PW2 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel2InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector2StartAngle - channel2InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule2.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule2(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW2
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 3
        if( (channel3InjEnabled) && (currentStatus.PW3 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel3InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector3StartAngle - channel3InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule3.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule3(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW3
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 4
        if( (channel4InjEnabled) && (currentStatus.PW4 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel4InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector4StartAngle - channel4InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule4.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule4(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW4
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 5
        if( (channel5InjEnabled) && (currentStatus.PW4 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel5InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector5StartAngle - channel5InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if (tempStartAngle <= tempCrankAngle && fuelSchedule5.schedulesSet == 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {







            setFuelSchedule3(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW1
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 6
        if( (channel6InjEnabled) && (currentStatus.PW6 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel6InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector6StartAngle - channel6InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule6.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule6(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW6
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 7
        if( (channel7InjEnabled) && (currentStatus.PW7 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel7InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector7StartAngle - channel7InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule7.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule7(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW7
                      );
          }
        }
#endif

#if INJ_CHANNELS >= 8
        if( (channel8InjEnabled) && (currentStatus.PW8 >= inj_opentime_uS) )
        {
          tempCrankAngle = crankAngle - channel8InjDegrees;
          if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_INJ; }
          tempStartAngle = injector8StartAngle - channel8InjDegrees;
          if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( (tempStartAngle <= tempCrankAngle) && (fuelSchedule8.Status == RUNNING) ) { tempStartAngle += CRANK_ANGLE_MAX_INJ; }
          if ( tempStartAngle > tempCrankAngle )
          {
            setFuelSchedule8(
                      ((tempStartAngle - tempCrankAngle) * (unsigned long)timePerDegree),
                      (unsigned long)currentStatus.PW8
                      );
          }
        }
#endif
      }





      if ( configPage4.ignCranklock && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && (decoderHasFixedCrankingTiming == true) )
      {
        fixedCrankingOverride = currentStatus.dwell * 3;

        if(currentStatus.RPM < 250)
        {
          ignition1StartAngle -= 5;
          ignition2StartAngle -= 5;
          ignition3StartAngle -= 5;
          ignition4StartAngle -= 5;
        }
      }
      else { fixedCrankingOverride = 0; }



      if(currentStatus.launchingHard || BIT_CHECK(currentStatus.spark, BIT_SPARK_BOOSTCUT) || BIT_CHECK(currentStatus.spark, BIT_SPARK_HRDLIM) || currentStatus.flatShiftingHard)
      {
        if(configPage2.hardCutType == HARD_CUT_FULL) { ignitionOn = false; }
        else
        {
          if(rollingCutCounter >= 2)
          {


            rollingCutCounter = 0;
            ignitionOn = true;
            curRollingCut = 0;
          }
          else
          {
            if(rollingCutLastRev == 0) { rollingCutLastRev = currentStatus.startRevolutions; }
            if (rollingCutLastRev != currentStatus.startRevolutions)
            {
              rollingCutLastRev = currentStatus.startRevolutions;
              rollingCutCounter++;
            }
            ignitionOn = false;
          }
        }
      }
      else { curRollingCut = 0; }


      if(ignitionOn)
      {


        crankAngle = getCrankAngle();
        if (crankAngle > CRANK_ANGLE_MAX_IGN ) { crankAngle -= 360; }

#if IGN_CHANNELS >= 1
        if ( (ignition1StartAngle > crankAngle) && (curRollingCut != 1) )
        {
            if(ignitionSchedule1.Status != RUNNING)
            {
              setIgnitionSchedule1(ign1StartFunction,

                        angleToTime((ignition1StartAngle - crankAngle), CRANKMATH_METHOD_INTERVAL_REV),
                        currentStatus.dwell + fixedCrankingOverride,
                        ign1EndFunction
                        );
            }
        }
#endif

#if defined(USE_IGN_REFRESH)
        if( (ignitionSchedule1.Status == RUNNING) && (ignition1EndAngle > crankAngle) && (configPage4.StgCycles == 0) && (configPage2.perToothIgn != true) )
        {
          unsigned long uSToEnd = 0;

          crankAngle = getCrankAngle();
          if (crankAngle > CRANK_ANGLE_MAX_IGN ) { crankAngle -= 360; }



          if(ignition1EndAngle > crankAngle) { uSToEnd = fastDegreesToUS( (ignition1EndAngle - crankAngle) ); }
          else { uSToEnd = fastDegreesToUS( (360 + ignition1EndAngle - crankAngle) ); }




          refreshIgnitionSchedule1( uSToEnd + fixedCrankingOverride );
        }
  #endif



#if IGN_CHANNELS >= 2
        tempCrankAngle = crankAngle - channel2IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition2StartAngle - channel2IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }

        {
            unsigned long ignition2StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition2StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }

            else { ignition2StartTime = 0; }

            if( (ignition2StartTime > 0) && (curRollingCut != 2) )
            {
              setIgnitionSchedule2(ign2StartFunction,
                        ignition2StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign2EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 3
        tempCrankAngle = crankAngle - channel3IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition3StartAngle - channel3IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }

        {
            long ignition3StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition3StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }

            else { ignition3StartTime = 0; }

            if( (ignition3StartTime > 0) && (curRollingCut != 3) )
            {
              setIgnitionSchedule3(ign3StartFunction,
                        ignition3StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign3EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 4
        tempCrankAngle = crankAngle - channel4IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition4StartAngle - channel4IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }

        {

            long ignition4StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition4StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }

            else { ignition4StartTime = 0; }

            if( (ignition4StartTime > 0) && (curRollingCut != 4) )
            {
              setIgnitionSchedule4(ign4StartFunction,
                        ignition4StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign4EndFunction
                        );
            }
        }
#endif

#if IGN_CHANNELS >= 5
        tempCrankAngle = crankAngle - channel5IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition5StartAngle - channel5IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }

        {

            long ignition5StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition5StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }

            else { ignition5StartTime = 0; }

            if( (ignition5StartTime > 0) && (curRollingCut != 5) ) {
            setIgnitionSchedule5(ign5StartFunction,
                      ignition5StartTime,
                      currentStatus.dwell + fixedCrankingOverride,
                      ign5EndFunction
                      );
            }
        }
#endif

#if IGN_CHANNELS >= 6
        tempCrankAngle = crankAngle - channel6IgnDegrees;
        if( tempCrankAngle < 0) { tempCrankAngle += CRANK_ANGLE_MAX_IGN; }
        tempStartAngle = ignition6StartAngle - channel6IgnDegrees;
        if ( tempStartAngle < 0) { tempStartAngle += CRANK_ANGLE_MAX_IGN; }
        {
            unsigned long ignition6StartTime = 0;
            if(tempStartAngle > tempCrankAngle) { ignition6StartTime = angleToTime((tempStartAngle - tempCrankAngle), CRANKMATH_METHOD_INTERVAL_REV); }
            else { ignition6StartTime = 0; }

            if( (ignition6StartTime > 0) && (curRollingCut != 2) )
            {
              setIgnitionSchedule6(ign6StartFunction,
                        ignition6StartTime,
                        currentStatus.dwell + fixedCrankingOverride,
                        ign6EndFunction
                        );
            }
        }
#endif

      }

      if ( (!BIT_CHECK(currentStatus.status3, BIT_STATUS3_RESET_PREVENT)) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) )
      {

        digitalWrite(pinResetControl, HIGH);
        BIT_SET(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      }
    }
    else if ( (BIT_CHECK(currentStatus.status3, BIT_STATUS3_RESET_PREVENT) > 0) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) )
    {
      digitalWrite(pinResetControl, LOW);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
    }
}
//# 1132 "/home/developper/speeduino/speeduino/speeduino.ino"
uint16_t PW(int REQ_FUEL, byte VE, long MAP, int corrections, int injOpen)
{



  uint16_t iVE, iCorrections;
  uint16_t iMAP = 100;
  uint16_t iAFR = 147;


  iVE = ((unsigned int)VE << 7) / 100;
  if ( configPage2.multiplyMAP == true ) {
    iMAP = ((unsigned int)MAP << 7) / currentStatus.baro;
  }
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == 2)) {
    iAFR = ((unsigned int)currentStatus.O2 << 7) / currentStatus.afrTarget;
  }
  iCorrections = (corrections << 7) / 100;


  unsigned long intermediate = ((long)REQ_FUEL * (long)iVE) >> 7;
  if ( configPage2.multiplyMAP == true ) {
    intermediate = (intermediate * (unsigned long)iMAP) >> 7;
  }
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == 2) ) {
    intermediate = (intermediate * (unsigned long)iAFR) >> 7;
  }
  intermediate = (intermediate * (unsigned long)iCorrections) >> 7;
  if (intermediate != 0)
  {

    intermediate += injOpen;
    if ( intermediate > 65535)
    {
      intermediate = 65535;
    }
  }
  return (unsigned int)(intermediate);
}






byte getVE()
{
  byte tempVE = 100;
  if (configPage2.fuelAlgorithm == LOAD_SOURCE_MAP)
  {

    currentStatus.fuelLoad = currentStatus.MAP;
  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_TPS)
  {

    currentStatus.fuelLoad = currentStatus.TPS;
  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_IMAPEMAP)
  {

    currentStatus.fuelLoad = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  else { currentStatus.fuelLoad = currentStatus.MAP; }
  tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM);

  return tempVE;
}







byte getVE2()
{
  byte tempVE = 100;
  if( configPage10.fuel2Algorithm == LOAD_SOURCE_MAP)
  {

    currentStatus.fuelLoad2 = currentStatus.MAP;
  }
  else if (configPage10.fuel2Algorithm == LOAD_SOURCE_TPS)
  {

    currentStatus.fuelLoad2 = currentStatus.TPS;
  }
  else if (configPage10.fuel2Algorithm == LOAD_SOURCE_IMAPEMAP)
  {

    currentStatus.fuelLoad2 = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  else { currentStatus.fuelLoad2 = currentStatus.MAP; }
  tempVE = get3DTableValue(&fuelTable2, currentStatus.fuelLoad2, currentStatus.RPM);

  return tempVE;
}






byte getAdvance()
{
  byte tempAdvance = 0;
  if (configPage2.ignAlgorithm == LOAD_SOURCE_MAP)
  {

    currentStatus.ignLoad = currentStatus.MAP;
  }
  else if(configPage2.ignAlgorithm == LOAD_SOURCE_TPS)
  {

    currentStatus.ignLoad = currentStatus.TPS;

  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_IMAPEMAP)
  {

    currentStatus.ignLoad = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  tempAdvance = get3DTableValue(&ignitionTable, currentStatus.ignLoad, currentStatus.RPM) - OFFSET_IGNITION;
  tempAdvance = correctionsIgn(tempAdvance);

  return tempAdvance;
}

uint16_t calculateInjector2StartAngle(unsigned int PWdivTimerPerDegree)
{
  uint16_t tempInjector2StartAngle = (configPage2.inj2Ang + channel2InjDegrees);
  if(tempInjector2StartAngle < PWdivTimerPerDegree) { tempInjector2StartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjector2StartAngle -= PWdivTimerPerDegree;
  if(tempInjector2StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjector2StartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjector2StartAngle;
}
uint16_t calculateInjector3StartAngle(unsigned int PWdivTimerPerDegree)
{
  uint16_t tempInjector3StartAngle = (configPage2.inj3Ang + channel3InjDegrees);
  if(tempInjector3StartAngle < PWdivTimerPerDegree) { tempInjector3StartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjector3StartAngle -= PWdivTimerPerDegree;
  if(tempInjector3StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjector3StartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjector3StartAngle;
}
uint16_t calculateInjector4StartAngle(unsigned int PWdivTimerPerDegree)
{
  uint16_t tempInjector4StartAngle = (configPage2.inj4Ang + channel4InjDegrees);
  if(tempInjector4StartAngle < PWdivTimerPerDegree) { tempInjector4StartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjector4StartAngle -= PWdivTimerPerDegree;
  if(tempInjector4StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjector4StartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjector4StartAngle;
}
uint16_t calculateInjector5StartAngle(unsigned int PWdivTimerPerDegree)
{
  uint16_t tempInjector5StartAngle = (configPage2.inj1Ang + channel4InjDegrees);
  if(tempInjector5StartAngle < PWdivTimerPerDegree) { tempInjector5StartAngle += CRANK_ANGLE_MAX_INJ; }
  tempInjector5StartAngle -= PWdivTimerPerDegree;
  if(tempInjector5StartAngle > (uint16_t)CRANK_ANGLE_MAX_INJ) { tempInjector5StartAngle -= CRANK_ANGLE_MAX_INJ; }

  return tempInjector5StartAngle;
}

#endif
//# 1 "/home/developper/speeduino/speeduino/auxiliaries.ino"





#include "globals.h"
#include "auxiliaries.h"
#include "maths.h"
#include "src/PID_v1/PID_v1.h"



integerPID_ideal boostPID(&currentStatus.MAP, &currentStatus.boostDuty , &currentStatus.boostTarget, &configPage10.boostSens, &configPage10.boostIntv, configPage6.boostKP, configPage6.boostKI, configPage6.boostKD, DIRECT);




void initialiseFan()
{
  if( configPage6.fanInv == 1 ) { fanHIGH = LOW; fanLOW = HIGH; }
  else { fanHIGH = HIGH; fanLOW = LOW; }
  digitalWrite(pinFan, fanLOW);
  currentStatus.fanOn = false;

  fan_pin_port = portOutputRegister(digitalPinToPort(pinFan));
  fan_pin_mask = digitalPinToBitMask(pinFan);
}

void fanControl()
{
  if( configPage6.fanEnable == 1 )
  {
    int onTemp = (int)configPage6.fanSP - CALIBRATION_TEMPERATURE_OFFSET;
    int offTemp = onTemp - configPage6.fanHyster;
    bool fanPermit = false;

    if ( configPage2.fanWhenOff ) { fanPermit = true; }
    else { fanPermit = BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN); }

    if ( currentStatus.coolant >= onTemp && fanPermit )
    {

      if( configPage6.fanInv == 0 ) { FAN_PIN_HIGH(); }
      else { FAN_PIN_LOW(); }
      currentStatus.fanOn = true;
    }
    else if ( currentStatus.coolant <= offTemp || !fanPermit )
    {

      if( configPage6.fanInv == 0 ) { FAN_PIN_LOW(); }
      else { FAN_PIN_HIGH(); }
      currentStatus.fanOn = false;
    }
  }
}

void initialiseAuxPWM()
{
  boost_pin_port = portOutputRegister(digitalPinToPort(pinBoost));
  boost_pin_mask = digitalPinToBitMask(pinBoost);
  vvt_pin_port = portOutputRegister(digitalPinToPort(pinVVT_1));
  vvt_pin_mask = digitalPinToBitMask(pinVVT_1);
  n2o_stage1_pin_port = portOutputRegister(digitalPinToPort(configPage10.n2o_stage1_pin));
  n2o_stage1_pin_mask = digitalPinToBitMask(configPage10.n2o_stage1_pin);
  n2o_stage2_pin_port = portOutputRegister(digitalPinToPort(configPage10.n2o_stage2_pin));
  n2o_stage2_pin_mask = digitalPinToBitMask(configPage10.n2o_stage2_pin);
  n2o_arming_pin_port = portInputRegister(digitalPinToPort(configPage10.n2o_arming_pin));
  n2o_arming_pin_mask = digitalPinToBitMask(configPage10.n2o_arming_pin);

  if(configPage10.n2o_enable > 0)
  {

    if(configPage10.n2o_pin_polarity == 1) { pinMode(configPage10.n2o_arming_pin, INPUT_PULLUP); }
    else { pinMode(configPage10.n2o_arming_pin, INPUT); }
  }

  ENABLE_VVT_TIMER();

  boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);
  if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(100, 100, 100); }
  else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }

  currentStatus.boostDuty = 0;
  boostCounter = 0;

  currentStatus.nitrous_status = NITROUS_OFF;

}

#define BOOST_HYSTER 40
void boostControl()
{
  if( configPage6.boostEnabled==1 )
  {
    if(configPage4.boostType == OPEN_LOOP_BOOST)
    {

      currentStatus.boostDuty = get3DTableValue(&boostTable, currentStatus.TPS, currentStatus.RPM) * 2 * 100;

      if(currentStatus.boostDuty > 10000) { currentStatus.boostDuty = 10000; }
      if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); }
      else
      {
        boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000;
        ENABLE_BOOST_TIMER();
      }
    }
    else if (configPage4.boostType == CLOSED_LOOP_BOOST)
    {
      if( (boostCounter & 7) == 1) { currentStatus.boostTarget = get3DTableValue(&boostTable, currentStatus.TPS, currentStatus.RPM) * 2; }
      if(currentStatus.MAP >= 100 )
      {

        if( configPage2.flexEnabled == 1 )
        {
          currentStatus.boostTarget += table2D_getValue(&flexBoostTable, currentStatus.ethanolPct);;
        }
        else
        {
          currentStatus.flexBoostCorrection = 0;
        }

        if(currentStatus.boostTarget > 0)
        {

          if( (boostCounter & 15) == 1)
          {
            boostPID.SetOutputLimits(configPage2.boostMinDuty, configPage2.boostMaxDuty);

            if(configPage6.boostMode == BOOST_MODE_SIMPLE) { boostPID.SetTunings(100, 100, 100); }
            else { boostPID.SetTunings(configPage6.boostKP, configPage6.boostKI, configPage6.boostKD); }
          }

          bool PIDcomputed = boostPID.Compute();
          if(currentStatus.boostDuty == 0) { DISABLE_BOOST_TIMER(); BOOST_PIN_LOW(); }
          else
          {
            if(PIDcomputed == true)
            {
              boost_pwm_target_value = ((unsigned long)(currentStatus.boostDuty) * boost_pwm_max_count) / 10000;
              ENABLE_BOOST_TIMER();
            }
          }

        }
        else
        {

          boostDisable();
        }
      }
      else
      {

        boostDisable();
      }
    }
  }
  else {
    DISABLE_BOOST_TIMER();
    currentStatus.flexBoostCorrection = 0;
  }

  boostCounter++;
}

void vvtControl()
{
  if( configPage6.vvtEnabled == 1 )
  {
    byte vvtDuty = get3DTableValue(&vvtTable, currentStatus.TPS, currentStatus.RPM);


    if( (configPage6.VVTasOnOff == true) && (vvtDuty < 100) ) { vvtDuty = 0; }

    if(vvtDuty == 0)
    {

      VVT_PIN_LOW();
      DISABLE_VVT_TIMER();
    }
    else if (vvtDuty >= 100)
    {

      VVT_PIN_HIGH();
      DISABLE_VVT_TIMER();
    }
    else
    {
      vvt_pwm_target_value = percentage(vvtDuty, vvt_pwm_max_count);
      ENABLE_VVT_TIMER();
    }
  }
  else { DISABLE_VVT_TIMER(); }
}

void nitrousControl()
{
  bool nitrousOn = false;
  if(configPage10.n2o_enable > 0)
  {
    bool isArmed = READ_N2O_ARM_PIN();
    if (configPage10.n2o_pin_polarity == 1) { isArmed = !isArmed; }


    if( (isArmed == true) && (currentStatus.coolant > (configPage10.n2o_minCLT - CALIBRATION_TEMPERATURE_OFFSET)) && (currentStatus.TPS > configPage10.n2o_minTPS) && (currentStatus.O2 < configPage10.n2o_maxAFR) && (currentStatus.MAP < configPage10.n2o_maxMAP) )
    {

      uint16_t realStage1MinRPM = (uint16_t)configPage10.n2o_stage1_minRPM * 100;
      uint16_t realStage1MaxRPM = (uint16_t)configPage10.n2o_stage1_maxRPM * 100;
      uint16_t realStage2MinRPM = (uint16_t)configPage10.n2o_stage2_minRPM * 100;
      uint16_t realStage2MaxRPM = (uint16_t)configPage10.n2o_stage2_maxRPM * 100;

      if( (currentStatus.RPM > realStage1MinRPM) && (currentStatus.RPM < realStage1MaxRPM) )
      {
        currentStatus.nitrous_status = NITROUS_STAGE1;
        BIT_SET(currentStatus.status3, BIT_STATUS3_NITROUS);
        N2O_STAGE1_PIN_HIGH();
        nitrousOn = true;
      }
      if(configPage10.n2o_enable == NITROUS_STAGE2)
      {
        if( (currentStatus.RPM > realStage2MinRPM) && (currentStatus.RPM < realStage2MaxRPM) )
        {
          currentStatus.nitrous_status = NITROUS_STAGE2;
          BIT_SET(currentStatus.status3, BIT_STATUS3_NITROUS);
          N2O_STAGE2_PIN_HIGH();
          nitrousOn = true;
        }
      }
    }
  }

  if (nitrousOn == false)
    {
      currentStatus.nitrous_status = NITROUS_OFF;
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_NITROUS);
      N2O_STAGE1_PIN_LOW();
      N2O_STAGE2_PIN_LOW();
    }
}

void boostDisable()
{
  boostPID.Initialize();
  currentStatus.boostDuty = 0;
  DISABLE_BOOST_TIMER();
  BOOST_PIN_LOW();
}


#if defined(CORE_AVR)
  ISR(TIMER1_COMPA_vect)
#else
  static inline void boostInterrupt()
#endif
{
  if (boost_pwm_state == true)
  {
    BOOST_PIN_LOW();
    BOOST_TIMER_COMPARE = BOOST_TIMER_COUNTER + (boost_pwm_max_count - boost_pwm_cur_value);
    boost_pwm_state = false;
  }
  else
  {
    BOOST_PIN_HIGH();
    BOOST_TIMER_COMPARE = BOOST_TIMER_COUNTER + boost_pwm_target_value;
    boost_pwm_cur_value = boost_pwm_target_value;
    boost_pwm_state = true;
  }
}


#if defined(CORE_AVR)
  ISR(TIMER1_COMPB_vect)
#else
  static inline void vvtInterrupt()
#endif
{
  if (vvt_pwm_state == true)
  {
    VVT_PIN_LOW();
    VVT_TIMER_COMPARE = VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt_pwm_cur_value);
    vvt_pwm_state = false;
  }
  else
  {
    VVT_PIN_HIGH();
    VVT_TIMER_COMPARE = VVT_TIMER_COUNTER + vvt_pwm_target_value;
    vvt_pwm_cur_value = vvt_pwm_target_value;
    vvt_pwm_state = true;
  }
}

#if defined(CORE_TEENSY)
void ftm1_isr(void)
{


  bool interrupt1 = (FTM1_C0SC & FTM_CSC_CHF);
  bool interrupt2 = (FTM1_C1SC & FTM_CSC_CHF);

  if(interrupt1) { FTM1_C0SC &= ~FTM_CSC_CHF; boostInterrupt(); }
  else if(interrupt2) { FTM1_C1SC &= ~FTM_CSC_CHF; vvtInterrupt(); }

}
#endif
//# 1 "/home/developper/speeduino/speeduino/board_avr2560.ino"

#if defined(CORE_AVR)
#include "globals.h"
#include "auxiliaries.h"


void initBoard()
{




    configPage9.intcan_available = 0;






    TCCR1B = 0x00;
    TCNT1 = 0;
    TCCR1A = 0x00;
    TCCR1B = (1 << CS12);
    TIFR1 = (1 << OCF1A) | (1<<OCF1B) | (1<<OCF1C) | (1<<TOV1) | (1<<ICF1);

    boost_pwm_max_count = 1000000L / (16 * configPage6.boostFreq * 2);
    vvt_pwm_max_count = 1000000L / (16 * configPage6.vvtFreq * 2);






    TCCR2B = 0x00;
    TCNT2 = 131;
    TIMSK2 = 0x01;
    TCCR2A = 0x00;

    TCCR2B |= (1<<CS22) | (1<<CS20);
    TCCR2B &= ~(1<<CS21);
    TIFR2 = (1 << OCF2A) | (1<<OCF2B) | (1<<TOV2);
//# 53 "/home/developper/speeduino/speeduino/board_avr2560.ino"
    TCCR3B = 0x00;
    TCNT3 = 0;
    TCCR3A = 0x00;
    TCCR3B = (1 << CS12);
    TIFR3 = (1 << OCF3A) | (1<<OCF3B) | (1<<OCF3C) | (1<<TOV3) | (1<<ICF3);


    TCCR5B = 0x00;
    TCNT5 = 0;
    TCCR5A = 0x00;
    TCCR5B = (1 << CS11) | (1 << CS10);
    TIFR5 = (1 << OCF5A) | (1<<OCF5B) | (1<<OCF5C) | (1<<TOV5) | (1<<ICF5);

    #if defined(TIMER5_MICROS)
      TIMSK5 |= (1 << TOIE5);
      TIMSK0 &= ~_BV(TOIE0);
    #endif


    TCCR4B = 0x00;
    TCNT4 = 0;
    TCCR4A = 0x00;
    TCCR4B = (1 << CS12);
    TIFR4 = (1 << OCF4A) | (1<<OCF4B) | (1<<OCF4C) | (1<<TOV4) | (1<<ICF4);

}

uint16_t freeRam()
{
    extern int __heap_start, *__brkval;
    int currentVal;
    uint16_t v;

    if(__brkval == 0) { currentVal = (int) &__heap_start; }
    else { currentVal = (int) __brkval; }



    return (uint16_t) &v - currentVal;
}

#if defined(TIMER5_MICROS)

ISR(TIMER5_OVF_vect)
{
    ++timer5_overflow_count;
}

static inline unsigned long micros_safe()
{
  unsigned long newMicros;
  noInterrupts();
  newMicros = (((timer5_overflow_count << 16) + TCNT5) * 4);
  interrupts();

  return newMicros;
}
#endif

#endif
//# 1 "/home/developper/speeduino/speeduino/board_stm32_generic.ino"
#if defined(CORE_STM32_GENERIC) && !defined(ARDUINO_BLACK_F407VE)
#include "board_stm32_generic.h"
#include "globals.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"
#include "HardwareTimer.h"
#if defined(ARDUINO_ARCH_STM32) && defined(STM32_CORE_VERSION)

    #include <stm32_TIM_variant_11.h>
    HardwareTimer Timer5(TIM5, chip_tim5, sizeof(chip_tim5) / sizeof(chip_tim5[0]));
    HardwareTimer Timer8(TIM8, chip_tim8, sizeof(chip_tim8) / sizeof(chip_tim8[0]));
#endif

void initBoard()
{




    #ifndef FLASH_LENGTH
      #define FLASH_LENGTH 8192
    #endif
    delay(10);




    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) )
    {
        idle_pwm_max_count = 1000000L / (configPage6.idleFreq * 2);
    }


    Timer1.setMode(4, TIMER_OUTPUT_COMPARE);

    if(idle_pwm_max_count > 0) { Timer1.attachInterrupt(4, idleInterrupt); }






    #if defined(ARDUINO_BLACK_F407VE) || defined(STM32F4) || defined(_STM32F4_)
        Timer8.setPeriod(1000);
        Timer8.setMode(1, TIMER_OUTPUT_COMPARE);
        Timer8.attachInterrupt(1, oneMSInterval);
        Timer8.resume();
    #else
        Timer4.setPeriod(1000);
        Timer4.setMode(1, TIMER_OUTPUT_COMPARE);
        Timer4.attachInterrupt(1, oneMSInterval);
        Timer4.resume();
    #endif
    pinMode(LED_BUILTIN, OUTPUT);






    boost_pwm_max_count = 1000000L / (2 * configPage6.boostFreq * 2);
    vvt_pwm_max_count = 1000000L / (2 * configPage6.vvtFreq * 2);


    Timer1.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer1.setMode(3, TIMER_OUTPUT_COMPARE);
    if(boost_pwm_max_count > 0) { Timer1.attachInterrupt(2, boostInterrupt);}
    if(vvt_pwm_max_count > 0) { Timer1.attachInterrupt(3, vvtInterrupt);}





    #if defined (STM32F1) || defined(__STM32F1__)


        Timer1.setPrescaleFactor((72 * 2)-1);
        Timer2.setPrescaleFactor((36 * 2)-1);
        Timer3.setPrescaleFactor((36 * 2)-1);
    #elif defined(STM32F4)


        Timer1.setPrescaleFactor((168 * 2)-1);
        Timer2.setPrescaleFactor((84 * 2)-1);
        Timer3.setPrescaleFactor((84 * 2)-1);
    #endif
    Timer2.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer2.setMode(4, TIMER_OUTPUT_COMPARE);

    Timer3.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer3.setMode(4, TIMER_OUTPUT_COMPARE);
    Timer1.setMode(1, TIMER_OUTPUT_COMPARE);



    Timer2.attachInterrupt(1, fuelSchedule1Interrupt);
    Timer2.attachInterrupt(2, fuelSchedule2Interrupt);
    Timer2.attachInterrupt(3, fuelSchedule3Interrupt);
    Timer2.attachInterrupt(4, fuelSchedule4Interrupt);
    #if (INJ_CHANNELS >= 5)
    Timer5.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer5.attachInterrupt(1, fuelSchedule5Interrupt);
    #endif
    #if (INJ_CHANNELS >= 6)
    Timer5.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer5.attachInterrupt(2, fuelSchedule6Interrupt);
    #endif
    #if (INJ_CHANNELS >= 7)
    Timer5.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer5.attachInterrupt(3, fuelSchedule7Interrupt);
    #endif
    #if (INJ_CHANNELS >= 8)
    Timer5.setMode(4, TIMER_OUTPUT_COMPARE);
    Timer5.attachInterrupt(4, fuelSchedule8Interrupt);
    #endif


    Timer3.attachInterrupt(1, ignitionSchedule1Interrupt);
    Timer3.attachInterrupt(2, ignitionSchedule2Interrupt);
    Timer3.attachInterrupt(3, ignitionSchedule3Interrupt);
    Timer3.attachInterrupt(4, ignitionSchedule4Interrupt);
    #if (IGN_CHANNELS >= 5)
    Timer4.setMode(1, TIMER_OUTPUT_COMPARE);
    Timer4.attachInterrupt(1, ignitionSchedule5Interrupt);
    #endif
    #if (IGN_CHANNELS >= 6)
    Timer4.setMode(2, TIMER_OUTPUT_COMPARE);
    Timer4.attachInterrupt(2, ignitionSchedule6Interrupt);
    #endif
    #if (IGN_CHANNELS >= 7)
    Timer4.setMode(3, TIMER_OUTPUT_COMPARE);
    Timer4.attachInterrupt(3, ignitionSchedule7Interrupt);
    #endif
    #if (IGN_CHANNELS >= 8)
    Timer4.setMode(4, TIMER_OUTPUT_COMPARE);
    Timer4.attachInterrupt(4, ignitionSchedule8Interrupt);
    #endif

    Timer1.setOverflow(0xFFFF);
    Timer1.resume();
    Timer2.setOverflow(0xFFFF);
    Timer2.resume();
    Timer3.setOverflow(0xFFFF);
    Timer3.resume();
    #if (IGN_CHANNELS >= 5)
    Timer4.setOverflow(0xFFFF);
    Timer4.resume();
    #endif
    #if (INJ_CHANNELS >= 5)
    Timer5.setOverflow(0xFFFF);
    Timer5.resume();
    #endif
}

uint16_t freeRam()
{
    char top = 't';
    return &top - reinterpret_cast<char*>(sbrk(0));
}

#endif
//# 1 "/home/developper/speeduino/speeduino/board_stm32_official.ino"
#if defined(CORE_STM32_OFFICIAL)
#include "board_stm32_official.h"
#include "globals.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"
#include <timer.h>

#if defined(STM32F4)
  #define NR_OFF_TIMERS 9

  stimer_t HardwareTimers_1;
  stimer_t HardwareTimers_2;
  stimer_t HardwareTimers_3;
  stimer_t HardwareTimers_4;
  stimer_t HardwareTimers_5;
  stimer_t HardwareTimers_8;





#else
  #include "HardwareTimer.h"
#endif

  extern void oneMSIntervalIRQ(stimer_t *Timer)
  {
    oneMSInterval();
  }
  extern void EmptyIRQCallback(stimer_t *Timer, uint32_t channel)
  {

  }

  void initBoard()
  {




    HardwareTimers_1.timer = TIM1;
    HardwareTimers_2.timer = TIM2;
    HardwareTimers_3.timer = TIM3;
    HardwareTimers_4.timer = TIM4;

    HardwareTimers_5.timer = TIM5;
    HardwareTimers_8.timer = TIM8;






    #define FLASH_LENGTH 8192





    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) )
    {
        idle_pwm_max_count = 1000000L / (configPage6.idleFreq * 2);
    }


    TimerPulseInit(&HardwareTimers_1, 0xFFFF, 0, EmptyIRQCallback);

    if(idle_pwm_max_count > 0) { attachIntHandleOC(&HardwareTimers_1, idleInterrupt, 4, 0);}
//# 81 "/home/developper/speeduino/speeduino/board_stm32_official.ino"
    #if defined(ARDUINO_BLACK_F407VE) || defined(STM32F4) || defined(_STM32F4_)
      TimerHandleInit(&HardwareTimers_8, 1000, 168);
      attachIntHandle(&HardwareTimers_8, oneMSIntervalIRQ);
    #else

    #endif
    pinMode(LED_BUILTIN, OUTPUT);






    boost_pwm_max_count = 1000000L / (2 * configPage6.boostFreq * 2);
    vvt_pwm_max_count = 1000000L / (2 * configPage6.vvtFreq * 2);






      if(idle_pwm_max_count > 0) { attachIntHandleOC(&HardwareTimers_1, boostInterrupt, 2, 0);}
      if(vvt_pwm_max_count > 0) { attachIntHandleOC(&HardwareTimers_1, vvtInterrupt, 3, 0);}


      TimerPulseInit(&HardwareTimers_3, 0xFFFF, 0, EmptyIRQCallback);
      attachIntHandleOC(&HardwareTimers_3, fuelSchedule1Interrupt, 1, 0);
      attachIntHandleOC(&HardwareTimers_3, fuelSchedule2Interrupt, 2, 0);
      attachIntHandleOC(&HardwareTimers_3, fuelSchedule3Interrupt, 3, 0);
      attachIntHandleOC(&HardwareTimers_3, fuelSchedule4Interrupt, 4, 0);


      TimerPulseInit(&HardwareTimers_2, 0xFFFF, 0, EmptyIRQCallback);
      attachIntHandleOC(&HardwareTimers_2, ignitionSchedule1Interrupt, 1, 0);
      attachIntHandleOC(&HardwareTimers_2, ignitionSchedule2Interrupt, 2, 0);
      attachIntHandleOC(&HardwareTimers_2, ignitionSchedule3Interrupt, 3, 0);
      attachIntHandleOC(&HardwareTimers_2, ignitionSchedule4Interrupt, 4, 0);




      TimerPulseInit(&HardwareTimers_5, 0xFFFF, 0, EmptyIRQCallback);

      #if (INJ_CHANNELS >= 5)
      attachIntHandleOC(&HardwareTimers_5, fuelSchedule5Interrupt, 1, 0);

      #endif
      #if (INJ_CHANNELS >= 6)
      attachIntHandleOC(&HardwareTimers_5, fuelSchedule6Interrupt, 2, 0);

      #endif
      #if (INJ_CHANNELS >= 7)
      attachIntHandleOC(&HardwareTimers_5, fuelSchedule7Interrupt, 3, 0);

      #endif
      #if (INJ_CHANNELS >= 8)
      attachIntHandleOC(&HardwareTimers_5, fuelSchedule8Interrupt, 4, 0);

      #endif

      TimerPulseInit(&HardwareTimers_4, 0xFFFF, 0, EmptyIRQCallback);

      #if (IGN_CHANNELS >= 5)
      attachIntHandleOC(&HardwareTimers_4, ignitionSchedule5Interrupt, 1, 0);

      #endif
      #if (IGN_CHANNELS >= 6)
      attachIntHandleOC(&HardwareTimers_4, ignitionSchedule6Interrupt, 2, 0);

      #endif
      #if (IGN_CHANNELS >= 7)
      attachIntHandleOC(&HardwareTimers_4, ignitionSchedule7Interrupt, 3, 0);

      #endif
      #if (IGN_CHANNELS >= 8)
      attachIntHandleOC(&HardwareTimers_4, ignitionSchedule8Interrupt, 4, 0);

      #endif

      setTimerPrescalerRegister(&HardwareTimers_2, (uint32_t)(getTimerClkFreq(HardwareTimers_2.timer) / (250000)) - 1);
      setTimerPrescalerRegister(&HardwareTimers_3, (uint32_t)(getTimerClkFreq(HardwareTimers_3.timer) / (250000)) - 1);
      setTimerPrescalerRegister(&HardwareTimers_4, (uint32_t)(getTimerClkFreq(HardwareTimers_4.timer) / (250000)) - 1);
      setTimerPrescalerRegister(&HardwareTimers_5, (uint32_t)(getTimerClkFreq(HardwareTimers_5.timer) / (250000)) - 1);
  }

  uint16_t freeRam()
  {
      char top = 't';
      return &top - reinterpret_cast<char*>(sbrk(0));
  }




#endif
//# 1 "/home/developper/speeduino/speeduino/board_teensy35.ino"
#if defined(CORE_TEENSY)
#include "board_teensy35.h"
#include "globals.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"


void initBoard()
{
//# 20 "/home/developper/speeduino/speeduino/board_teensy35.ino"
    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) )
    {

        FTM2_MODE |= FTM_MODE_WPDIS;
        FTM2_MODE |= FTM_MODE_FTMEN;
        FTM2_MODE |= FTM_MODE_INIT;

        FTM2_SC = 0x00;
        FTM2_CNTIN = 0x0000;
        FTM2_CNT = 0x0000;
        FTM2_MOD = 0xFFFF;
//# 39 "/home/developper/speeduino/speeduino/board_teensy35.ino"
        FTM2_SC |= FTM_SC_CLKS(0b10);






        MCG_C3 = 0x9B;
//# 63 "/home/developper/speeduino/speeduino/board_teensy35.ino"
        FTM2_SC |= FTM_SC_PS(0b0);


        FTM2_C0SC &= ~FTM_CSC_MSB;
        FTM2_C0SC |= FTM_CSC_MSA;



        FTM2_C1SC &= ~FTM_CSC_MSB;
        FTM2_C1SC |= FTM_CSC_MSA;



        FTM2_C1SC &= ~FTM_CSC_MSB;
        FTM2_C1SC |= FTM_CSC_MSA;




        NVIC_ENABLE_IRQ(IRQ_FTM2);
    }






    lowResTimer.begin(oneMSInterval, 1000);






    FTM1_MODE |= FTM_MODE_WPDIS;
    FTM1_MODE |= FTM_MODE_FTMEN;
    FTM1_MODE |= FTM_MODE_INIT;
    FTM1_SC |= FTM_SC_CLKS(0b1);
    FTM1_SC |= FTM_SC_PS(0b111);


    FTM1_C0SC &= ~FTM_CSC_MSB;
    FTM1_C0SC |= FTM_CSC_MSA;
    FTM1_C0SC |= FTM_CSC_CHIE;
    FTM1_C1SC &= ~FTM_CSC_MSB;
    FTM1_C1SC |= FTM_CSC_MSA;
    FTM1_C1SC |= FTM_CSC_CHIE;




    boost_pwm_max_count = 1000000L / (2 * configPage6.boostFreq * 2);
    vvt_pwm_max_count = 1000000L / (2 * configPage6.vvtFreq * 2);







    FTM0_MODE |= FTM_MODE_WPDIS;
    FTM0_MODE |= FTM_MODE_FTMEN;
    FTM0_MODE |= FTM_MODE_INIT;

    FTM0_SC = 0x00;
    FTM0_CNTIN = 0x0000;
    FTM0_CNT = 0x0000;
    FTM0_MOD = 0xFFFF;


    FTM3_MODE |= FTM_MODE_WPDIS;
    FTM3_MODE |= FTM_MODE_FTMEN;
    FTM3_MODE |= FTM_MODE_INIT;

    FTM3_SC = 0x00;
    FTM3_CNTIN = 0x0000;
    FTM3_CNT = 0x0000;
    FTM3_MOD = 0xFFFF;
//# 149 "/home/developper/speeduino/speeduino/board_teensy35.ino"
    FTM0_SC |= FTM_SC_CLKS(0b1);
    FTM3_SC |= FTM_SC_CLKS(0b1);
//# 167 "/home/developper/speeduino/speeduino/board_teensy35.ino"
    FTM0_SC |= FTM_SC_PS(0b111);
    FTM3_SC |= FTM_SC_PS(0b111);






    FTM0_C0SC &= ~FTM_CSC_MSB;
    FTM0_C0SC |= FTM_CSC_MSA;
    FTM0_C0SC |= FTM_CSC_CHIE;

    FTM0_C1SC &= ~FTM_CSC_MSB;
    FTM0_C1SC |= FTM_CSC_MSA;
    FTM0_C1SC |= FTM_CSC_CHIE;

    FTM0_C2SC &= ~FTM_CSC_MSB;
    FTM0_C2SC |= FTM_CSC_MSA;
    FTM0_C2SC |= FTM_CSC_CHIE;

    FTM0_C3SC &= ~FTM_CSC_MSB;
    FTM0_C3SC |= FTM_CSC_MSA;
    FTM0_C3SC |= FTM_CSC_CHIE;

    FTM0_C4SC &= ~FTM_CSC_MSB;
    FTM0_C4SC |= FTM_CSC_MSA;
    FTM0_C4SC |= FTM_CSC_CHIE;

    FTM0_C5SC &= ~FTM_CSC_MSB;
    FTM0_C5SC |= FTM_CSC_MSA;
    FTM0_C5SC |= FTM_CSC_CHIE;

    FTM0_C6SC &= ~FTM_CSC_MSB;
    FTM0_C6SC |= FTM_CSC_MSA;
    FTM0_C6SC |= FTM_CSC_CHIE;

    FTM0_C7SC &= ~FTM_CSC_MSB;
    FTM0_C7SC |= FTM_CSC_MSA;
    FTM0_C7SC |= FTM_CSC_CHIE;


    FTM3_C0SC &= ~FTM_CSC_MSB;
    FTM3_C0SC |= FTM_CSC_MSA;
    FTM3_C0SC |= FTM_CSC_CHIE;

    FTM3_C1SC &= ~FTM_CSC_MSB;
    FTM3_C1SC |= FTM_CSC_MSA;
    FTM3_C1SC |= FTM_CSC_CHIE;

    FTM3_C2SC &= ~FTM_CSC_MSB;
    FTM3_C2SC |= FTM_CSC_MSA;
    FTM3_C2SC |= FTM_CSC_CHIE;

    FTM3_C3SC &= ~FTM_CSC_MSB;
    FTM3_C3SC |= FTM_CSC_MSA;
    FTM3_C3SC |= FTM_CSC_CHIE;

    FTM3_C4SC &= ~FTM_CSC_MSB;
    FTM3_C4SC |= FTM_CSC_MSA;
    FTM3_C4SC |= FTM_CSC_CHIE;

    FTM3_C5SC &= ~FTM_CSC_MSB;
    FTM3_C5SC |= FTM_CSC_MSA;
    FTM3_C5SC |= FTM_CSC_CHIE;

    FTM3_C6SC &= ~FTM_CSC_MSB;
    FTM3_C6SC |= FTM_CSC_MSA;
    FTM3_C6SC |= FTM_CSC_CHIE;

    FTM3_C7SC &= ~FTM_CSC_MSB;
    FTM3_C7SC |= FTM_CSC_MSA;
    FTM3_C7SC |= FTM_CSC_CHIE;


    NVIC_ENABLE_IRQ(IRQ_FTM0);
    NVIC_ENABLE_IRQ(IRQ_FTM1);

}

uint16_t freeRam()
{
    uint32_t stackTop;
    uint32_t heapTop;


    stackTop = (uint32_t) &stackTop;


    void* hTop = malloc(1);
    heapTop = (uint32_t) hTop;
    free(hTop);


    return (uint16_t)stackTop - heapTop;
}


#endif
//# 1 "/home/developper/speeduino/speeduino/board_template.ino"
#if defined(CORE_TEMPLATE)
#include "globals.h"

void initBoard()
{
//# 30 "/home/developper/speeduino/speeduino/board_template.ino"
}

uint16_t freeRam()
{

}

#endif
//# 1 "/home/developper/speeduino/speeduino/cancomms.ino"
//# 13 "/home/developper/speeduino/speeduino/cancomms.ino"
#include "globals.h"
#include "cancomms.h"
#include "maths.h"
#include "errors.h"
#include "utils.h"

void canCommand()
{
  currentcanCommand = CANSerial.read();

  switch (currentcanCommand)
  {
    case 'A':
        sendcanValues(0, CAN_PACKET_SIZE, 0x30, 1);
        break;

    case 'G':
       byte destcaninchannel;
      if (CANSerial.available() >= 9)
      {
        cancmdfail = CANSerial.read();
        destcaninchannel = CANSerial.read();
        if (cancmdfail != 0)
           {
            for (byte Gx = 0; Gx < 8; Gx++)
              {
                Gdata[Gx] = CANSerial.read();
              }
            Glow = Gdata[(configPage9.caninput_source_start_byte[destcaninchannel]&7)];
            if ((BIT_CHECK(configPage9.caninput_source_num_bytes,destcaninchannel)))
               {
                if ((configPage9.caninput_source_start_byte[destcaninchannel]&7) < 8)
                   {
                    Ghigh = Gdata[((configPage9.caninput_source_start_byte[destcaninchannel]&7)+1)];
                   }
            else{Ghigh = 0;}
               }
          else
               {
                 Ghigh = 0;
               }

          currentStatus.canin[destcaninchannel] = (Ghigh<<8) | Glow;
        }

        else{}

      }
        break;

    case 'L':
        uint8_t Llength;
        while (CANSerial.available() == 0) { }
        canlisten = CANSerial.read();

        if (canlisten == 0)
         {

          break;
         }

         while (CANSerial.available() == 0) { }
         Llength= CANSerial.read();

         for (uint8_t Lcount = 0; Lcount <Llength ;Lcount++)
         {
           while (CANSerial.available() == 0){}

           Lbuffer[Lcount] = CANSerial.read();
         }
         break;

    case 'r':
      byte Cmd;
      if (CANSerial.available() >= 6)
      {
        CANSerial.read();
        Cmd = CANSerial.read();

        uint16_t offset, length;
        if( (Cmd == 0x30) || ( (Cmd >= 0x40) && (Cmd <0x50) ) )
        {
          byte tmp;
          tmp = CANSerial.read();
          offset = word(CANSerial.read(), tmp);
          tmp = CANSerial.read();
          length = word(CANSerial.read(), tmp);
          sendcanValues(offset, length,Cmd, 1);

        }
        else
        {

        }
      }
      break;

    case 's':
      CANSerial.print(F("Speeduino csx02018.7"));
       break;

    case 'S':
      CANSerial.print(F("Speeduino 2018.7-dev"));
      break;

    case 'Q':
       for (unsigned int revn = 0; revn < sizeof( TSfirmwareVersion) - 1; revn++)
       {
         CANSerial.write( TSfirmwareVersion[revn]);
       }

       break;

    case 'Z':
       break;

    default:
       break;
  }
}
void sendcanValues(uint16_t offset, uint16_t packetLength, byte cmd, byte portType)
{
  byte fullStatus[CAN_PACKET_SIZE];


    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)|| defined(CORE_STM32) || defined (CORE_TEENSY)
      if (offset == 0)
        {
          CANSerial.write("A");
        }
      else
        {
      CANSerial.write("r");
      CANSerial.write(cmd);
        }
    #endif

  currentStatus.spark ^= (-currentStatus.hasSync ^ currentStatus.spark) & (1 << BIT_SPARK_SYNC);

  fullStatus[0] = currentStatus.secl;
  fullStatus[1] = currentStatus.status1;
  fullStatus[2] = currentStatus.engine;
  fullStatus[3] = (byte)(divu100(currentStatus.dwell));
  fullStatus[4] = lowByte(currentStatus.MAP);
  fullStatus[5] = highByte(currentStatus.MAP);
  fullStatus[6] = (byte)(currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET);
  fullStatus[7] = (byte)(currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  fullStatus[8] = currentStatus.batCorrection;
  fullStatus[9] = currentStatus.battery10;
  fullStatus[10] = currentStatus.O2;
  fullStatus[11] = currentStatus.egoCorrection;
  fullStatus[12] = currentStatus.iatCorrection;
  fullStatus[13] = currentStatus.wueCorrection;
  fullStatus[14] = lowByte(currentStatus.RPM);
  fullStatus[15] = highByte(currentStatus.RPM);
  fullStatus[16] = currentStatus.AEamount;
  fullStatus[17] = currentStatus.corrections;
  fullStatus[18] = currentStatus.VE;
  fullStatus[19] = currentStatus.afrTarget;
  fullStatus[20] = lowByte(currentStatus.PW1);
  fullStatus[21] = highByte(currentStatus.PW1);
  fullStatus[22] = currentStatus.tpsDOT;
  fullStatus[23] = currentStatus.advance;
  fullStatus[24] = currentStatus.TPS;

  fullStatus[25] = lowByte(currentStatus.loopsPerSecond);
  fullStatus[26] = highByte(currentStatus.loopsPerSecond);


  currentStatus.freeRAM = freeRam();
  fullStatus[27] = lowByte(currentStatus.freeRAM);
  fullStatus[28] = highByte(currentStatus.freeRAM);

  fullStatus[29] = (byte)(currentStatus.boostTarget >> 1);
  fullStatus[30] = (byte)(currentStatus.boostDuty / 100);
  fullStatus[31] = currentStatus.spark;


  fullStatus[32] = lowByte(currentStatus.rpmDOT);
  fullStatus[33] = highByte(currentStatus.rpmDOT);

  fullStatus[34] = currentStatus.ethanolPct;
  fullStatus[35] = currentStatus.flexCorrection;
  fullStatus[36] = currentStatus.flexIgnCorrection;

  fullStatus[37] = currentStatus.idleLoad;
  fullStatus[38] = currentStatus.testOutputs;

  fullStatus[39] = currentStatus.O2_2;
  fullStatus[40] = currentStatus.baro;

  fullStatus[41] = lowByte(currentStatus.canin[0]);
  fullStatus[42] = highByte(currentStatus.canin[0]);
  fullStatus[43] = lowByte(currentStatus.canin[1]);
  fullStatus[44] = highByte(currentStatus.canin[1]);
  fullStatus[45] = lowByte(currentStatus.canin[2]);
  fullStatus[46] = highByte(currentStatus.canin[2]);
  fullStatus[47] = lowByte(currentStatus.canin[3]);
  fullStatus[48] = highByte(currentStatus.canin[3]);
  fullStatus[49] = lowByte(currentStatus.canin[4]);
  fullStatus[50] = highByte(currentStatus.canin[4]);
  fullStatus[51] = lowByte(currentStatus.canin[5]);
  fullStatus[52] = highByte(currentStatus.canin[5]);
  fullStatus[53] = lowByte(currentStatus.canin[6]);
  fullStatus[54] = highByte(currentStatus.canin[6]);
  fullStatus[55] = lowByte(currentStatus.canin[7]);
  fullStatus[56] = highByte(currentStatus.canin[7]);
  fullStatus[57] = lowByte(currentStatus.canin[8]);
  fullStatus[58] = highByte(currentStatus.canin[8]);
  fullStatus[59] = lowByte(currentStatus.canin[9]);
  fullStatus[60] = highByte(currentStatus.canin[9]);
  fullStatus[61] = lowByte(currentStatus.canin[10]);
  fullStatus[62] = highByte(currentStatus.canin[10]);
  fullStatus[63] = lowByte(currentStatus.canin[11]);
  fullStatus[64] = highByte(currentStatus.canin[11]);
  fullStatus[65] = lowByte(currentStatus.canin[12]);
  fullStatus[66] = highByte(currentStatus.canin[12]);
  fullStatus[67] = lowByte(currentStatus.canin[13]);
  fullStatus[68] = highByte(currentStatus.canin[13]);
  fullStatus[69] = lowByte(currentStatus.canin[14]);
  fullStatus[70] = highByte(currentStatus.canin[14]);
  fullStatus[71] = lowByte(currentStatus.canin[15]);
  fullStatus[72] = highByte(currentStatus.canin[15]);

  fullStatus[73] = currentStatus.tpsADC;
  fullStatus[74] = getNextError();

  for(byte x=0; x<packetLength; x++)
  {
    if (portType == 1){ CANSerial.write(fullStatus[offset+x]); }
    else if (portType == 2)
            {

            }
  }

}



void sendCancommand(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress)
{
    switch (cmdtype)
    {
      case 0:
        CANSerial.print("G");
        CANSerial.write(canaddress);
        CANSerial.write(candata1);
        CANSerial.write(candata2);
        break;

      case 1:
        CANSerial.print("L");
        CANSerial.write(canaddress);
        break;

     case 2:
        CANSerial.print("R");
        CANSerial.write(candata1);
        CANSerial.write(lowByte(sourcecanAddress) );
        CANSerial.write(highByte(sourcecanAddress) );
        break;

     case 3:


        break;

     default:
        break;
    }
}
//# 1 "/home/developper/speeduino/speeduino/comms.ino"





#include "globals.h"
#include "comms.h"
#include "cancomms.h"
#include "errors.h"
#include "storage.h"
#include "maths.h"
#include "utils.h"
#include "decoders.h"
#include "scheduledIO.h"







void command()
{
  if (cmdPending == false) { currentCommand = Serial.read(); }

  switch (currentCommand)
  {

    case 'a':
      cmdPending = true;

      if (Serial.available() >= 2)
      {
        Serial.read();
        Serial.read();
        sendValuesLegacy();
        cmdPending = false;
      }
      break;

    case 'A':
      sendValues(0, SERIAL_PACKET_SIZE, 0x30, 0);
      break;


    case 'B':
      writeAllConfig();
      break;

    case 'b':
      cmdPending = true;

      if (Serial.available() >= 2)
      {
        Serial.read();
        writeConfig(Serial.read());
        cmdPending = false;
      }
      break;

    case 'C':
      testComm();
      break;

    case 'c':
      Serial.write(lowByte(currentStatus.loopsPerSecond));
      Serial.write(highByte(currentStatus.loopsPerSecond));
      break;

    case 'd':
      cmdPending = true;

      if (Serial.available() >= 2)
      {
        Serial.read();
        uint32_t CRC32_val = calculateCRC32( Serial.read() );


        Serial.write( ((CRC32_val >> 24) & 255) );
        Serial.write( ((CRC32_val >> 16) & 255) );
        Serial.write( ((CRC32_val >> 8) & 255) );
        Serial.write( (CRC32_val & 255) );

        cmdPending = false;
      }
      break;



    case 'E':
      cmdPending = true;

      if(Serial.available() >= 2)
      {
        int cmdCombined = word(Serial.read(), Serial.read());
        if (currentStatus.RPM == 0) { commandButtons(cmdCombined); }

        cmdPending = false;
      }
      break;

    case 'F':
      Serial.print(F("001"));
      break;

    case 'H':
      currentStatus.toothLogEnabled = true;
      currentStatus.compositeLogEnabled = false;
      toothHistoryIndex = 0;
      toothHistorySerialIndex = 0;


      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), loggerPrimaryISR, CHANGE );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), loggerSecondaryISR, CHANGE );

      Serial.write(1);
      break;

    case 'h':
      currentStatus.toothLogEnabled = false;


      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), triggerHandler, primaryTriggerEdge );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), triggerSecondaryHandler, secondaryTriggerEdge );
      break;

    case 'J':
      currentStatus.compositeLogEnabled = true;
      currentStatus.toothLogEnabled = false;
      toothHistoryIndex = 0;
      toothHistorySerialIndex = 0;
      compositeLastToothTime = 0;


      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), loggerPrimaryISR, CHANGE );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), loggerSecondaryISR, CHANGE );

      Serial.write(1);
      break;

    case 'j':
      currentStatus.compositeLogEnabled = false;


      detachInterrupt( digitalPinToInterrupt(pinTrigger) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger), triggerHandler, primaryTriggerEdge );

      detachInterrupt( digitalPinToInterrupt(pinTrigger2) );
      attachInterrupt( digitalPinToInterrupt(pinTrigger2), triggerSecondaryHandler, secondaryTriggerEdge );
      break;

    case 'L':
      sendPageASCII();
      break;

    case 'm':
      currentStatus.freeRAM = freeRam();
      Serial.write(lowByte(currentStatus.freeRAM));
      Serial.write(highByte(currentStatus.freeRAM));
      break;

    case 'N':
      Serial.println();
      break;

    case 'P':


      cmdPending = true;

      if (Serial.available() > 0)
      {
        currentPage = Serial.read();

        if ((currentPage >= '0') && (currentPage <= '9'))
        {
          currentPage -= 48;
        }
        else if ((currentPage >= 'a') && (currentPage <= 'f'))
        {
          currentPage -= 87;
        }
        else if ((currentPage >= 'A') && (currentPage <= 'F'))
        {
          currentPage -= 55;
        }


        if ( (currentPage == veMapPage) || (currentPage == ignMapPage) || (currentPage == afrMapPage) ) { isMap = true; }
        else { isMap = false; }
        cmdPending = false;
      }
      break;




    case 'p':
      cmdPending = true;





      if(Serial.available() >= 6)
      {
        byte offset1, offset2, length1, length2;
        int length;
        byte tempPage;

        Serial.read();
        tempPage = Serial.read();

        offset1 = Serial.read();
        offset2 = Serial.read();
        valueOffset = word(offset2, offset1);
        length1 = Serial.read();
        length2 = Serial.read();
        length = word(length2, length1);
        for(int i = 0; i < length; i++)
        {
          Serial.write( getPageValue(tempPage, valueOffset + i) );
        }

        cmdPending = false;
      }
      break;

    case 'Q':
      Serial.print(F("speeduino 201904-dev"));
      break;

    case 'r':
      cmdPending = true;
      byte cmd;
      if (Serial.available() >= 6)
      {
        tsCanId = Serial.read();
        cmd = Serial.read();

        uint16_t offset, length;
        if(cmd == 0x30)
        {
          byte tmp;
          tmp = Serial.read();
          offset = word(Serial.read(), tmp);
          tmp = Serial.read();
          length = word(Serial.read(), tmp);
          sendValues(offset, length,cmd, 0);
        }
        else
        {

        }
        cmdPending = false;
      }
      break;

    case 'S':
      Serial.print(F("Speeduino 2019.04-dev"));
      currentStatus.secl = 0;
      break;

    case 'T':
      if(currentStatus.toothLogEnabled == true) { sendToothLog(false); }
      else if (currentStatus.compositeLogEnabled == true) { sendCompositeLog(); }

      break;

    case 't':
      byte tableID;



      while (Serial.available() == 0) { }
      tableID = Serial.read();

      receiveCalibration(tableID);
      writeCalibration();

      break;

    case 'U':
      if (resetControl != RESET_CONTROL_DISABLED)
      {
      #ifndef SMALL_FLASH_MODE
        if (!cmdPending) { Serial.println(F("Comms halted. Next byte will reset the Arduino.")); }
      #endif

        while (Serial.available() == 0) { }
        digitalWrite(pinResetControl, LOW);
      }
      else
      {
      #ifndef SMALL_FLASH_MODE
        if (!cmdPending) { Serial.println(F("Reset control is currently disabled.")); }
      #endif
      }
      break;

    case 'V':
      sendPage();
      break;

    case 'W':
      cmdPending = true;

      if (isMap)
      {
        if(Serial.available() >= 3)
        {
          byte offset1, offset2;
          offset1 = Serial.read();
          offset2 = Serial.read();
          valueOffset = word(offset2, offset1);
          receiveValue(valueOffset, Serial.read());
          cmdPending = false;
        }
      }
      else
      {
        if(Serial.available() >= 2)
        {
          valueOffset = Serial.read();
          receiveValue(valueOffset, Serial.read());
          cmdPending = false;
        }
      }

      break;

    case 'w':
      cmdPending = true;

      if(chunkPending == false)
      {






        if(Serial.available() >= 7)
        {
          byte offset1, offset2, length1, length2;

          Serial.read();
          currentPage = Serial.read();

          offset1 = Serial.read();
          offset2 = Serial.read();
          valueOffset = word(offset2, offset1);
          length1 = Serial.read();
          length2 = Serial.read();
          chunkSize = word(length2, length1);

          chunkPending = true;
          chunkComplete = 0;
        }
      }

      if(chunkPending == true)
      {
        while( (Serial.available() > 0) && (chunkComplete < chunkSize) )
        {
          receiveValue( (valueOffset + chunkComplete), Serial.read());
          chunkComplete++;
        }
        if(chunkComplete >= chunkSize) { cmdPending = false; chunkPending = false; }
      }
      break;

    case 'Z':
    #ifndef SMALL_FLASH_MODE
      Serial.println(F("Coolant"));
      for (int x = 0; x < CALIBRATION_TABLE_SIZE; x++)
      {
        Serial.print(x);
        Serial.print(", ");
        Serial.println(cltCalibrationTable[x]);
      }
      Serial.println(F("Inlet temp"));
      for (int x = 0; x < CALIBRATION_TABLE_SIZE; x++)
      {
        Serial.print(x);
        Serial.print(", ");
        Serial.println(iatCalibrationTable[x]);
      }
      Serial.println(F("O2"));
      for (int x = 0; x < CALIBRATION_TABLE_SIZE; x++)
      {
        Serial.print(x);
        Serial.print(", ");
        Serial.println(o2CalibrationTable[x]);
      }
      Serial.println(F("WUE"));
      for (int x = 0; x < 10; x++)
      {
        Serial.print(configPage4.wueBins[x]);
        Serial.print(F(", "));
        Serial.println(configPage2.wueValues[x]);
      }
      Serial.flush();
    #endif
      break;

    case 'z':
      sendToothLog(true);
      break;

    case '`':
      cmdPending = true;

      if (Serial.available() >= 1) {
        configPage4.bootloaderCaps = Serial.read();
        cmdPending = false;
      }
      break;


    case '?':
    #ifndef SMALL_FLASH_MODE
      Serial.println
      (F(
         "\n"
         "===Command Help===\n\n"
         "All commands are single character and are concatenated with their parameters \n"
         "without spaces."
         "Syntax:  <command>+<parameter1>+<parameter2>+<parameterN>\n\n"
         "===List of Commands===\n\n"
         "A - Displays 31 bytes of currentStatus values in binary (live data)\n"
         "B - Burn current map and configPage values to eeprom\n"
         "C - Test COM port.  Used by Tunerstudio to see whether an ECU is on a given serial \n"
         "    port. Returns a binary number.\n"
         "N - Print new line.\n"
         "P - Set current page.  Syntax:  P+<pageNumber>\n"
         "R - Same as A command\n"
         "S - Display signature number\n"
         "Q - Same as S command\n"
         "V - Display map or configPage values in binary\n"
         "W - Set one byte in map or configPage.  Expects binary parameters. \n"
         "    Syntax:  W+<offset>+<newbyte>\n"
         "t - Set calibration values.  Expects binary parameters.  Table index is either 0, \n"
         "    1, or 2.  Syntax:  t+<tble_idx>+<newValue1>+<newValue2>+<newValueN>\n"
         "Z - Display calibration values\n"
         "T - Displays 256 tooth log entries in binary\n"
         "r - Displays 256 tooth log entries\n"
         "U - Prepare for firmware update. The next byte received will cause the Arduino to reset.\n"
         "? - Displays this help page"
       ));
     #endif

      break;

    default:
      break;
  }
}





void sendValues(uint16_t offset, uint16_t packetLength, byte cmd, byte portNum)
{
  byte fullStatus[SERIAL_PACKET_SIZE];

  if (portNum == 3)
  {

    #if defined(USE_SERIAL3)
      if (offset == 0)
      {
        CANSerial.write("A");
      }
      else
      {
        CANSerial.write("r");
        CANSerial.write(cmd);
      }
    #endif
  }
  else
  {
    if(requestCount == 0) { currentStatus.secl = 0; }
    requestCount++;
  }

  currentStatus.spark ^= (-currentStatus.hasSync ^ currentStatus.spark) & (1 << BIT_SPARK_SYNC);

  fullStatus[0] = currentStatus.secl;
  fullStatus[1] = currentStatus.status1;
  fullStatus[2] = currentStatus.engine;
  fullStatus[3] = currentStatus.syncLossCounter;
  fullStatus[4] = lowByte(currentStatus.MAP);
  fullStatus[5] = highByte(currentStatus.MAP);
  fullStatus[6] = (byte)(currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET);
  fullStatus[7] = (byte)(currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  fullStatus[8] = currentStatus.batCorrection;
  fullStatus[9] = currentStatus.battery10;
  fullStatus[10] = currentStatus.O2;
  fullStatus[11] = currentStatus.egoCorrection;
  fullStatus[12] = currentStatus.iatCorrection;
  fullStatus[13] = currentStatus.wueCorrection;
  fullStatus[14] = lowByte(currentStatus.RPM);
  fullStatus[15] = highByte(currentStatus.RPM);
  fullStatus[16] = (byte)(currentStatus.AEamount >> 1);
  fullStatus[17] = currentStatus.corrections;
  fullStatus[18] = currentStatus.VE;
  fullStatus[19] = currentStatus.afrTarget;
  fullStatus[20] = lowByte(currentStatus.PW1);
  fullStatus[21] = highByte(currentStatus.PW1);
  fullStatus[22] = currentStatus.tpsDOT;
  fullStatus[23] = currentStatus.advance;
  fullStatus[24] = currentStatus.TPS;

  if(currentStatus.loopsPerSecond > 60000) { currentStatus.loopsPerSecond = 60000;}
  fullStatus[25] = lowByte(currentStatus.loopsPerSecond);
  fullStatus[26] = highByte(currentStatus.loopsPerSecond);


  currentStatus.freeRAM = freeRam();
  fullStatus[27] = lowByte(currentStatus.freeRAM);
  fullStatus[28] = highByte(currentStatus.freeRAM);

  fullStatus[29] = (byte)(currentStatus.boostTarget >> 1);
  fullStatus[30] = (byte)(currentStatus.boostDuty / 100);
  fullStatus[31] = currentStatus.spark;


  fullStatus[32] = lowByte(currentStatus.rpmDOT);
  fullStatus[33] = highByte(currentStatus.rpmDOT);

  fullStatus[34] = currentStatus.ethanolPct;
  fullStatus[35] = currentStatus.flexCorrection;
  fullStatus[36] = currentStatus.flexIgnCorrection;

  fullStatus[37] = currentStatus.idleLoad;
  fullStatus[38] = currentStatus.testOutputs;

  fullStatus[39] = currentStatus.O2_2;
  fullStatus[40] = currentStatus.baro;

  fullStatus[41] = lowByte(currentStatus.canin[0]);
  fullStatus[42] = highByte(currentStatus.canin[0]);
  fullStatus[43] = lowByte(currentStatus.canin[1]);
  fullStatus[44] = highByte(currentStatus.canin[1]);
  fullStatus[45] = lowByte(currentStatus.canin[2]);
  fullStatus[46] = highByte(currentStatus.canin[2]);
  fullStatus[47] = lowByte(currentStatus.canin[3]);
  fullStatus[48] = highByte(currentStatus.canin[3]);
  fullStatus[49] = lowByte(currentStatus.canin[4]);
  fullStatus[50] = highByte(currentStatus.canin[4]);
  fullStatus[51] = lowByte(currentStatus.canin[5]);
  fullStatus[52] = highByte(currentStatus.canin[5]);
  fullStatus[53] = lowByte(currentStatus.canin[6]);
  fullStatus[54] = highByte(currentStatus.canin[6]);
  fullStatus[55] = lowByte(currentStatus.canin[7]);
  fullStatus[56] = highByte(currentStatus.canin[7]);
  fullStatus[57] = lowByte(currentStatus.canin[8]);
  fullStatus[58] = highByte(currentStatus.canin[8]);
  fullStatus[59] = lowByte(currentStatus.canin[9]);
  fullStatus[60] = highByte(currentStatus.canin[9]);
  fullStatus[61] = lowByte(currentStatus.canin[10]);
  fullStatus[62] = highByte(currentStatus.canin[10]);
  fullStatus[63] = lowByte(currentStatus.canin[11]);
  fullStatus[64] = highByte(currentStatus.canin[11]);
  fullStatus[65] = lowByte(currentStatus.canin[12]);
  fullStatus[66] = highByte(currentStatus.canin[12]);
  fullStatus[67] = lowByte(currentStatus.canin[13]);
  fullStatus[68] = highByte(currentStatus.canin[13]);
  fullStatus[69] = lowByte(currentStatus.canin[14]);
  fullStatus[70] = highByte(currentStatus.canin[14]);
  fullStatus[71] = lowByte(currentStatus.canin[15]);
  fullStatus[72] = highByte(currentStatus.canin[15]);

  fullStatus[73] = currentStatus.tpsADC;
  fullStatus[74] = getNextError();

  fullStatus[75] = lowByte(currentStatus.PW2);
  fullStatus[76] = highByte(currentStatus.PW2);
  fullStatus[77] = lowByte(currentStatus.PW3);
  fullStatus[78] = highByte(currentStatus.PW3);
  fullStatus[79] = lowByte(currentStatus.PW4);
  fullStatus[80] = highByte(currentStatus.PW4);

  fullStatus[81] = currentStatus.status3;
  fullStatus[82] = lowByte(currentStatus.flexBoostCorrection);
  fullStatus[83] = highByte(currentStatus.flexBoostCorrection);

  fullStatus[84] = currentStatus.nChannels;
  fullStatus[85] = lowByte(currentStatus.fuelLoad);
  fullStatus[86] = highByte(currentStatus.fuelLoad);
  fullStatus[87] = lowByte(currentStatus.ignLoad);
  fullStatus[88] = highByte(currentStatus.ignLoad);
  fullStatus[89] = lowByte(currentStatus.dwell);
  fullStatus[90] = highByte(currentStatus.dwell);
  fullStatus[91] = currentStatus.CLIdleTarget;
  fullStatus[92] = currentStatus.mapDOT;

  for(byte x=0; x<packetLength; x++)
  {
    if (portNum == 0) { Serial.write(fullStatus[offset+x]); }
    else if (portNum == 3){ CANSerial.write(fullStatus[offset+x]); }
  }

}

void sendValuesLegacy()
{
  uint16_t temp;
  int bytestosend = 112;

  bytestosend -= Serial.write(currentStatus.secl>>8);
  bytestosend -= Serial.write(currentStatus.secl);
  bytestosend -= Serial.write(currentStatus.PW1>>8);
  bytestosend -= Serial.write(currentStatus.PW1);
  bytestosend -= Serial.write(currentStatus.PW2>>8);
  bytestosend -= Serial.write(currentStatus.PW2);
  bytestosend -= Serial.write(currentStatus.RPM>>8);
  bytestosend -= Serial.write(currentStatus.RPM);

  temp = currentStatus.advance * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  bytestosend -= Serial.write(currentStatus.nSquirts);
  bytestosend -= Serial.write(currentStatus.engine);
  bytestosend -= Serial.write(currentStatus.afrTarget);
  bytestosend -= Serial.write(currentStatus.afrTarget);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);

  temp = currentStatus.baro * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.MAP * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.IAT * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.coolant * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.TPS * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  bytestosend -= Serial.write(currentStatus.battery10>>8);
  bytestosend -= Serial.write(currentStatus.battery10);
  bytestosend -= Serial.write(currentStatus.O2>>8);
  bytestosend -= Serial.write(currentStatus.O2);
  bytestosend -= Serial.write(currentStatus.O2_2>>8);
  bytestosend -= Serial.write(currentStatus.O2_2);

  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);

  temp = currentStatus.egoCorrection * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.iatCorrection * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.wueCorrection * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);

  temp = currentStatus.corrections * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.VE * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);
  temp = currentStatus.VE2 * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);

  temp = currentStatus.tpsDOT * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.mapDOT * 10;
  bytestosend -= Serial.write(temp >> 8);
  bytestosend -= Serial.write(temp);

  temp = currentStatus.dwell * 10;
  bytestosend -= Serial.write(temp>>8);
  bytestosend -= Serial.write(temp);

  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(currentStatus.fuelLoad*10);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);
  bytestosend -= Serial.write(99);

  for(int i = 0; i < bytestosend; i++)
  {

    Serial.write(99);
  }
}

void receiveValue(uint16_t valueOffset, byte newValue)
{

  void* pnt_configPage;
  int tempOffset;

  switch (currentPage)
  {
    case veMapPage:
      if (valueOffset < 256)
      {
        fuelTable.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {

        if (valueOffset < 272)
        {

          fuelTable.axisX[(valueOffset - 256)] = ((int)(newValue) * TABLE_RPM_MULTIPLIER);
        }
        else if(valueOffset < 288)
        {

          tempOffset = 15 - (valueOffset - 272);
          fuelTable.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
        else
        {

        }
      }
      break;

    case veSetPage:
      pnt_configPage = &configPage2;

      if (valueOffset < npage_size[veSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case ignMapPage:
      if (valueOffset < 256)
      {
        ignitionTable.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {

        if (valueOffset < 272)
        {

          ignitionTable.axisX[(valueOffset - 256)] = (int)(newValue) * TABLE_RPM_MULTIPLIER;
        }
        else if(valueOffset < 288)
        {

          tempOffset = 15 - (valueOffset - 272);
          ignitionTable.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
      }
      break;

    case ignSetPage:
      pnt_configPage = &configPage4;

      if (valueOffset < npage_size[ignSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case afrMapPage:
      if (valueOffset < 256)
      {
        afrTable.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {

        if (valueOffset < 272)
        {

          afrTable.axisX[(valueOffset - 256)] = int(newValue) * TABLE_RPM_MULTIPLIER;
        }
        else
        {

          tempOffset = 15 - (valueOffset - 272);
          afrTable.axisY[tempOffset] = int(newValue) * TABLE_LOAD_MULTIPLIER;

        }
      }
      break;

    case afrSetPage:
      pnt_configPage = &configPage6;

      if (valueOffset < npage_size[afrSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case boostvvtPage:
      if (valueOffset < 64)
      {
        boostTable.values[7 - (valueOffset / 8)][valueOffset % 8] = newValue;
      }
      else if (valueOffset < 72)
      {
        boostTable.axisX[(valueOffset - 64)] = int(newValue) * TABLE_RPM_MULTIPLIER;
      }
      else if (valueOffset < 80)
      {
        boostTable.axisY[(7 - (valueOffset - 72))] = int(newValue);
      }

      else if (valueOffset < 144)
      {
        tempOffset = valueOffset - 80;
        vvtTable.values[7 - (tempOffset / 8)][tempOffset % 8] = newValue;
      }
      else if (valueOffset < 152)
      {
        tempOffset = valueOffset - 144;
        vvtTable.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER;
      }
      else if (valueOffset < 160)
      {
        tempOffset = valueOffset - 152;
        vvtTable.axisY[(7 - tempOffset)] = int(newValue);
      }

      else if (valueOffset < 224)
      {
        tempOffset = valueOffset - 160;
        stagingTable.values[7 - (tempOffset / 8)][tempOffset % 8] = newValue;
      }
      else if (valueOffset < 232)
      {
        tempOffset = valueOffset - 224;
        stagingTable.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER;
      }
      else if (valueOffset < 240)
      {
        tempOffset = valueOffset - 232;
        stagingTable.axisY[(7 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER;
      }
      break;

    case seqFuelPage:
      if (valueOffset < 36) { trim1Table.values[5 - (valueOffset / 6)][valueOffset % 6] = newValue; }
      else if (valueOffset < 42) { trim1Table.axisX[(valueOffset - 36)] = int(newValue) * TABLE_RPM_MULTIPLIER; }
      else if (valueOffset < 48) { trim1Table.axisY[(5 - (valueOffset - 42))] = int(newValue) * TABLE_LOAD_MULTIPLIER; }

      else if (valueOffset < 84) { tempOffset = valueOffset - 48; trim2Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; }
      else if (valueOffset < 90) { tempOffset = valueOffset - 84; trim2Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; }
      else if (valueOffset < 96) { tempOffset = valueOffset - 90; trim2Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; }

      else if (valueOffset < 132) { tempOffset = valueOffset - 96; trim3Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; }
      else if (valueOffset < 138) { tempOffset = valueOffset - 132; trim3Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; }
      else if (valueOffset < 144) { tempOffset = valueOffset - 138; trim3Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; }

      else if (valueOffset < 180) { tempOffset = valueOffset - 144; trim4Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; }
      else if (valueOffset < 186) { tempOffset = valueOffset - 180; trim4Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; }
      else if (valueOffset < 192) { tempOffset = valueOffset - 186; trim4Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; }

      break;

    case canbusPage:
      pnt_configPage = &configPage9;

      if (valueOffset < npage_size[currentPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case warmupPage:
      pnt_configPage = &configPage10;

      if (valueOffset < npage_size[currentPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    default:
      break;
  }

}







void sendPage()
{
  void* pnt_configPage = &configPage2;
  struct table3D currentTable = fuelTable;
  bool sendComplete = false;

  switch (currentPage)
  {
    case veMapPage:
      currentTable = fuelTable;
      break;

    case veSetPage:
      pnt_configPage = &configPage2;
      break;

    case ignMapPage:
      currentTable = ignitionTable;
      break;

    case ignSetPage:
      pnt_configPage = &configPage4;
      break;

    case afrMapPage:
      currentTable = afrTable;
      break;

    case afrSetPage:
      pnt_configPage = &configPage6;
      break;

    case boostvvtPage:
    {

      byte response[80];


      for (int x = 0; x < 64; x++) { response[x] = boostTable.values[7 - (x / 8)][x % 8]; }
      for (int x = 64; x < 72; x++) { response[x] = byte(boostTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 72; y < 80; y++) { response[y] = byte(boostTable.axisY[7 - (y - 72)]); }
      Serial.write((byte *)&response, 80);

      for (int x = 0; x < 64; x++) { response[x] = vvtTable.values[7 - (x / 8)][x % 8]; }
      for (int x = 64; x < 72; x++) { response[x] = byte(vvtTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 72; y < 80; y++) { response[y] = byte(vvtTable.axisY[7 - (y - 72)]); }
      Serial.write((byte *)&response, 80);

      for (int x = 0; x < 64; x++) { response[x] = stagingTable.values[7 - (x / 8)][x % 8]; }
      for (int x = 64; x < 72; x++) { response[x] = byte(stagingTable.axisX[(x - 64)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 72; y < 80; y++) { response[y] = byte(stagingTable.axisY[7 - (y - 72)] / TABLE_LOAD_MULTIPLIER); }
      Serial.write((byte *)&response, 80);
      sendComplete = true;
      break;
    }
    case seqFuelPage:
    {

      byte response[192];


      for (int x = 0; x < 36; x++) { response[x] = trim1Table.values[5 - (x / 6)][x % 6]; }
      for (int x = 36; x < 42; x++) { response[x] = byte(trim1Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 42; y < 48; y++) { response[y] = byte(trim1Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }

      for (int x = 0; x < 36; x++) { response[x + 48] = trim2Table.values[5 - (x / 6)][x % 6]; }
      for (int x = 36; x < 42; x++) { response[x + 48] = byte(trim2Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 42; y < 48; y++) { response[y + 48] = byte(trim2Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }

      for (int x = 0; x < 36; x++) { response[x + 96] = trim3Table.values[5 - (x / 6)][x % 6]; }
      for (int x = 36; x < 42; x++) { response[x + 96] = byte(trim3Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 42; y < 48; y++) { response[y + 96] = byte(trim3Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }

      for (int x = 0; x < 36; x++) { response[x + 144] = trim4Table.values[5 - (x / 6)][x % 6]; }
      for (int x = 36; x < 42; x++) { response[x + 144] = byte(trim4Table.axisX[(x - 36)] / TABLE_RPM_MULTIPLIER); }
      for (int y = 42; y < 48; y++) { response[y + 144] = byte(trim4Table.axisY[5 - (y - 42)] / TABLE_LOAD_MULTIPLIER); }
      Serial.write((byte *)&response, sizeof(response));
      sendComplete = true;
      break;
    }
    case canbusPage:
      pnt_configPage = &configPage9;
      break;

    case warmupPage:
      pnt_configPage = &configPage10;
      break;

    default:
    #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
    #endif

        pnt_configPage = &configPage10;
        currentTable = fuelTable;
        sendComplete = true;
        break;
  }
  if(!sendComplete)
  {
    if (isMap)
    {


        byte response[MAP_PAGE_SIZE];

        for (int x = 0; x < 256; x++) { response[x] = currentTable.values[15 - (x / 16)][x % 16]; }

        for (int x = 256; x < 272; x++) { response[x] = byte(currentTable.axisX[(x - 256)] / TABLE_RPM_MULTIPLIER); }

        for (int y = 272; y < 288; y++) { response[y] = byte(currentTable.axisY[15 - (y - 272)] / TABLE_LOAD_MULTIPLIER); }

        Serial.write((byte *)&response, sizeof(response));
    }
    else
    {
      for (byte x = 0; x < npage_size[currentPage]; x++)
      {

        Serial.write(*((byte *)pnt_configPage + x));
      }



    }
  }
}






void sendPageASCII()
{
  void* pnt_configPage = &configPage2;
  struct table3D currentTable = fuelTable;
  byte currentTitleIndex = 0;
  bool sendComplete = false;

  switch (currentPage)
  {
    case veMapPage:
      currentTitleIndex = 0;
      currentTable = fuelTable;
      break;

    case veSetPage:
      uint16_t* pnt16_configPage;


      Serial.println((const __FlashStringHelper *)&pageTitles[27]);


      for (pnt_configPage = &configPage2; pnt_configPage < &configPage2.wueValues[0]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      for (byte x = 10; x; x--)
      {
        Serial.print(configPage2.wueValues[10 - x]);
        Serial.print(F(" "));
      }
      Serial.println();
      for (pnt_configPage = (byte *)&configPage2.wueValues[9] + 1; pnt_configPage < &configPage2.inj1Ang; pnt_configPage = (byte *)pnt_configPage + 1) {
        Serial.println(*((byte *)pnt_configPage));
      }

      for (pnt16_configPage = (uint16_t *)&configPage2.inj1Ang; pnt16_configPage < (uint16_t*)&configPage2.inj4Ang + 1; pnt16_configPage = (uint16_t*)pnt16_configPage + 1)
      { Serial.println(*((uint16_t *)pnt16_configPage)); }

      for (pnt_configPage = (uint16_t *)&configPage2.inj4Ang + 1; pnt_configPage < &configPage2.mapMax; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      Serial.println(configPage2.mapMax);

      for (pnt_configPage = (uint16_t *)&configPage2.mapMax + 1; pnt_configPage < (byte *)&configPage2 + npage_size[veSetPage]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      sendComplete = true;
      break;

    case ignMapPage:
      currentTitleIndex = 42;
      currentTable = ignitionTable;
      break;

    case ignSetPage:

      Serial.println((const __FlashStringHelper *)&pageTitles[56]);
      Serial.println(configPage4.triggerAngle);

      for (pnt_configPage = (int *)&configPage4 + 1; pnt_configPage < &configPage4.taeBins[0]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      for (byte y = 2; y; y--)
      {
        byte * currentVar;
        if (y == 2) {
          currentVar = configPage4.taeBins;
        }
        else {
          currentVar = configPage4.taeValues;
        }

        for (byte j = 4; j; j--)
        {
          Serial.print(currentVar[4 - j]);
          Serial.print(' ');
        }
        Serial.println();
      }
      for (byte x = 10; x ; x--)
      {
        Serial.print(configPage4.wueBins[10 - x]);
        Serial.print(' ');
      }
      Serial.println();
      Serial.println(configPage4.dwellLimit);
      for (byte x = 6; x; x--)
      {
        Serial.print(configPage4.dwellCorrectionValues[6 - x]);
        Serial.print(' ');
      }
      Serial.println();
      for (pnt_configPage = (byte *)&configPage4.dwellCorrectionValues[5] + 1; pnt_configPage < (byte *)&configPage4 + npage_size[ignSetPage]; pnt_configPage = (byte *)pnt_configPage + 1)
      {
        Serial.println(*((byte *)pnt_configPage));
      }
      sendComplete = true;
      break;

    case afrMapPage:
      currentTitleIndex = 71;
      currentTable = afrTable;
      break;

    case afrSetPage:


      Serial.println((const __FlashStringHelper *)&pageTitles[91]);
      for (pnt_configPage = &configPage6; pnt_configPage < &configPage6.voltageCorrectionBins[0]; pnt_configPage = (byte *)pnt_configPage + 1)
      {
        Serial.println(*((byte *)pnt_configPage));
      }
      for (byte y = 2; y; y--)
      {
        byte * currentVar;
        if (y == 2) { currentVar = configPage6.voltageCorrectionBins; }
        else { currentVar = configPage6.injVoltageCorrectionValues; }

        for (byte x = 6; x; x--)
        {
          Serial.print(currentVar[6 - x]);
          Serial.print(' ');
        }
        Serial.println();
      }
      for (byte y = 2; y; y--)
      {
        byte* currentVar;
        if (y == 2) { currentVar = configPage6.airDenBins; }
        else { currentVar = configPage6.airDenRates; }

        for (byte x = 9; x; x--)
        {
          Serial.print(currentVar[9 - x]);
          Serial.print(' ');
        }
        Serial.println();
      }

      for (pnt_configPage = (byte *)&configPage6.airDenRates[8] + 1; pnt_configPage < (byte *)&configPage6 + npage_size[afrSetPage]; pnt_configPage = (byte *)pnt_configPage + 1)
      {
        Serial.println(*((byte *)pnt_configPage));
      }
      sendComplete = true;



      Serial.println((const __FlashStringHelper *)&pageTitles[106]);
      for (byte y = 4; y; y--)
      {
        byte * currentVar;
        switch (y)
        {
          case 1: currentVar = configPage6.iacBins; break;
          case 2: currentVar = configPage6.iacOLPWMVal; break;
          case 3: currentVar = configPage6.iacOLStepVal; break;
          case 4: currentVar = configPage6.iacCLValues; break;
          default: break;
        }
        for (byte x = 10; x; x--)
        {
          Serial.print(currentVar[10 - x]);
          Serial.print(' ');
        }
        Serial.println();
      }
      for (byte y = 3; y; y--)
      {
        byte * currentVar;
        switch (y)
        {
          case 1: currentVar = configPage6.iacCrankBins; break;
          case 2: currentVar = configPage6.iacCrankDuty; break;
          case 3: currentVar = configPage6.iacCrankSteps; break;
          default: break;
        }
        for (byte x = 4; x; x--)
        {
          Serial.print(currentVar[4 - x]);
          Serial.print(' ');
        }
        Serial.println();
      }

      for (pnt_configPage = (byte *)&configPage6.iacCrankBins[3] + 1; pnt_configPage < (byte *)&configPage6 + npage_size[afrSetPage]; pnt_configPage = (byte *)pnt_configPage + 1) { Serial.println(*((byte *)pnt_configPage)); }
      sendComplete = true;
      break;

    case boostvvtPage:
      currentTable = boostTable;
      currentTitleIndex = 121;
      break;

    case seqFuelPage:
      currentTable = trim1Table;
      for (int y = 0; y < currentTable.ySize; y++)
      {
        byte axisY = byte(currentTable.axisY[y]);
        if (axisY < 100)
        {
          Serial.write(" ");
          if (axisY < 10)
          {
            Serial.write(" ");
          }
        }
        Serial.print(axisY);
        Serial.write(" ");
        for (int x = 0; x < currentTable.xSize; x++)
        {
          byte value = currentTable.values[y][x];
          if (value < 100)
          {
            Serial.write(" ");
            if (value < 10)
            {
              Serial.write(" ");
            }
          }
          Serial.print(value);
          Serial.write(" ");
        }
        Serial.println("");
      }
      sendComplete = true;
      break;

    case canbusPage:


      Serial.println((const __FlashStringHelper *)&pageTitles[103]);
      for (pnt_configPage = &configPage9; pnt_configPage < ( (byte *)&configPage9 + npage_size[canbusPage]); pnt_configPage = (byte *)pnt_configPage + 1)
      {
        Serial.println(*((byte *)pnt_configPage));
      }
      sendComplete = true;
      break;

    case warmupPage:

      #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
      #endif
      sendComplete = true;
      break;

    default:
    #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
    #endif

        pnt_configPage = &configPage10;
        currentTable = fuelTable;
        sendComplete = true;
        break;
  }
  if(!sendComplete)
  {
    if (isMap)
    {
      do
      {
        const char spaceChar = ' ';

        Serial.println((const __FlashStringHelper *)&pageTitles[currentTitleIndex]);
        Serial.println();
        for (int y = 0; y < currentTable.ySize; y++)
        {
          byte axisY = byte(currentTable.axisY[y]);
          if (axisY < 100)
          {
            Serial.write(spaceChar);
            if (axisY < 10)
            {
              Serial.write(spaceChar);
            }
          }
          Serial.print(axisY);
          Serial.write(spaceChar);
          for (int i = 0; i < currentTable.xSize; i++)
          {
            byte value = currentTable.values[y][i];
            if (value < 100)
            {
              Serial.write(spaceChar);
              if (value < 10)
              {
                Serial.write(spaceChar);
              }
            }
            Serial.print(value);
            Serial.write(spaceChar);
          }
          Serial.println();
        }
        Serial.print(F("    "));
        for (int x = 0; x < currentTable.xSize; x++)
        {
          byte axisX = byte(currentTable.axisX[x] / 100);
          if (axisX < 100)
          {
            Serial.write(spaceChar);
            if (axisX < 10)
            {
              Serial.write(spaceChar);
            }
          }
          Serial.print(axisX);
          Serial.write(spaceChar);
        }
        Serial.println();
        if(currentTitleIndex == 121)
        {
          currentTitleIndex = 132;
          currentTable = vvtTable;
        }
        else currentTitleIndex = 0;
      }while(currentTitleIndex == 132);
    }
    else
    {
//# 1397 "/home/developper/speeduino/speeduino/comms.ino"
      for (byte x = 0; x < npage_size[currentPage]; x++)
      {

        Serial.write(*((byte *)pnt_configPage + x));
      }
    }
  }
}
//# 1413 "/home/developper/speeduino/speeduino/comms.ino"
byte getPageValue(byte page, uint16_t valueAddress)
{
  void* pnt_configPage = &configPage2;
  uint16_t tempAddress;
  byte returnValue = 0;

  switch (page)
  {
    case veMapPage:
        if( valueAddress < 256) { returnValue = fuelTable.values[15 - (valueAddress / 16)][valueAddress % 16]; }
        else if(valueAddress < 272) { returnValue = byte(fuelTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }
        else if (valueAddress < 288) { returnValue = byte(fuelTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); }
        break;

    case veSetPage:
        pnt_configPage = &configPage2;
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case ignMapPage:
        if( valueAddress < 256) { returnValue = ignitionTable.values[15 - (valueAddress / 16)][valueAddress % 16]; }
        else if(valueAddress < 272) { returnValue = byte(ignitionTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }
        else if (valueAddress < 288) { returnValue = byte(ignitionTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); }
        break;

    case ignSetPage:
        pnt_configPage = &configPage4;
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case afrMapPage:
        if( valueAddress < 256) { returnValue = afrTable.values[15 - (valueAddress / 16)][valueAddress % 16]; }
        else if(valueAddress < 272) { returnValue = byte(afrTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }
        else if (valueAddress < 288) { returnValue = byte(afrTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); }
        break;

    case afrSetPage:
        pnt_configPage = &configPage6;
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case boostvvtPage:

        {

          if(valueAddress < 80)
          {

            if(valueAddress < 64) { returnValue = boostTable.values[7 - (valueAddress / 8)][valueAddress % 8]; }
            else if(valueAddress < 72) { returnValue = byte(boostTable.axisX[(valueAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(valueAddress < 80) { returnValue = byte(boostTable.axisY[7 - (valueAddress - 72)]); }
          }
          else if(valueAddress < 160)
          {
            tempAddress = valueAddress - 80;

            if(tempAddress < 64) { returnValue = vvtTable.values[7 - (tempAddress / 8)][tempAddress % 8]; }
            else if(tempAddress < 72) { returnValue = byte(vvtTable.axisX[(tempAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 80) { returnValue = byte(vvtTable.axisY[7 - (tempAddress - 72)]); }
          }
          else
          {
            tempAddress = valueAddress - 160;

            if(tempAddress < 64) { returnValue = stagingTable.values[7 - (tempAddress / 8)][tempAddress % 8]; }
            else if(tempAddress < 72) { returnValue = byte(stagingTable.axisX[(tempAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 80) { returnValue = byte(stagingTable.axisY[7 - (tempAddress - 72)] / TABLE_LOAD_MULTIPLIER); }
          }
        }
        break;

    case seqFuelPage:

        {

          if(valueAddress < 48)
          {

            if(valueAddress < 36) { returnValue = trim1Table.values[5 - (valueAddress / 6)][valueAddress % 6]; }
            else if(valueAddress < 42) { returnValue = byte(trim1Table.axisX[(valueAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(valueAddress < 48) { returnValue = byte(trim1Table.axisY[5 - (valueAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 96)
          {
            tempAddress = valueAddress - 48;

            if(tempAddress < 36) { returnValue = trim2Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim2Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim2Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 144)
          {
            tempAddress = valueAddress - 96;

            if(tempAddress < 36) { returnValue = trim3Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim3Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim3Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 192)
          {
            tempAddress = valueAddress - 144;

            if(tempAddress < 36) { returnValue = trim4Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim4Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim4Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
        }
        break;

    case canbusPage:
        pnt_configPage = &configPage9;
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case warmupPage:
        pnt_configPage = &configPage10;
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case fuelMap2Page:
        if( valueAddress < 256) { returnValue = fuelTable2.values[15 - (valueAddress / 16)][valueAddress % 16]; }
        else if(valueAddress < 272) { returnValue = byte(fuelTable2.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }
        else if (valueAddress < 288) { returnValue = byte(fuelTable2.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); }
        break;

    default:
    #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
    #endif

        pnt_configPage = &configPage10;
        break;
  }
  return returnValue;
}






void receiveCalibration(byte tableID)
{
  byte* pnt_TargetTable;
  int OFFSET, DIVISION_FACTOR, BYTES_PER_VALUE, EEPROM_START;

  switch (tableID)
  {
    case 0:

      pnt_TargetTable = (byte *)&cltCalibrationTable;
      OFFSET = CALIBRATION_TEMPERATURE_OFFSET;
      DIVISION_FACTOR = 10;
      BYTES_PER_VALUE = 2;
      EEPROM_START = EEPROM_CALIBRATION_CLT;
      break;
    case 1:

      pnt_TargetTable = (byte *)&iatCalibrationTable;
      OFFSET = CALIBRATION_TEMPERATURE_OFFSET;
      DIVISION_FACTOR = 10;
      BYTES_PER_VALUE = 2;
      EEPROM_START = EEPROM_CALIBRATION_IAT;
      break;
    case 2:

      pnt_TargetTable = (byte *)&o2CalibrationTable;
      OFFSET = 0;
      DIVISION_FACTOR = 1;
      BYTES_PER_VALUE = 1;
      EEPROM_START = EEPROM_CALIBRATION_O2;
      break;

    default:
      OFFSET = 0;
      pnt_TargetTable = (byte *)&o2CalibrationTable;
      DIVISION_FACTOR = 1;
      BYTES_PER_VALUE = 1;
      EEPROM_START = EEPROM_CALIBRATION_O2;
      break;
  }



  int tempValue;
  byte tempBuffer[2];
  bool every2nd = true;
  int x;
  int counter = 0;
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  for (x = 0; x < 1024; x++)
  {

    if (BYTES_PER_VALUE == 1)
    {
      while ( Serial.available() < 1 ) {}
      tempValue = Serial.read();
    }
    else
    {
      while ( Serial.available() < 2 ) {}
      tempBuffer[0] = Serial.read();
      tempBuffer[1] = Serial.read();

      tempValue = div(int(word(tempBuffer[1], tempBuffer[0])), DIVISION_FACTOR).quot;
      tempValue = ((tempValue - 32) * 5) / 9;
    }
    tempValue = tempValue + OFFSET;

    if (every2nd)
    {
      if (tempValue > 255) {
        tempValue = 255;
      }
      if (tempValue < 0) {
        tempValue = 0;
      }

      pnt_TargetTable[(x / 2)] = (byte)tempValue;


      int y = EEPROM_START + (x / 2);

      storeCalibrationValue(y, (byte)tempValue);

      every2nd = false;
      #if defined(CORE_STM32)
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      #else
        analogWrite(LED_BUILTIN, (counter % 50) );
      #endif
      counter++;
    }
    else {
      every2nd = true;
    }

  }

}






void sendToothLog(bool useChar)
{

  if (BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY))
  {
      for (int x = 0; x < TOOTH_LOG_SIZE; x++)
      {


        Serial.write(toothHistory[toothHistorySerialIndex] >> 24);
        Serial.write(toothHistory[toothHistorySerialIndex] >> 16);
        Serial.write(toothHistory[toothHistorySerialIndex] >> 8);
        Serial.write(toothHistory[toothHistorySerialIndex]);

        if(toothHistorySerialIndex == (TOOTH_LOG_BUFFER-1)) { toothHistorySerialIndex = 0; }
        else { toothHistorySerialIndex++; }
      }
      BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
      cmdPending = false;
  }
  else { cmdPending = true; }
}

void sendCompositeLog()
{
  if (BIT_CHECK(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY))
  {
      uint32_t runTime = 0;
      for (int x = 0; x < TOOTH_LOG_SIZE; x++)
      {
        runTime += toothHistory[toothHistorySerialIndex];



        Serial.write(runTime >> 24);
        Serial.write(runTime >> 16);
        Serial.write(runTime >> 8);
        Serial.write(runTime);




        Serial.write(compositeLogHistory[toothHistorySerialIndex]);



        if(toothHistorySerialIndex == (TOOTH_LOG_BUFFER-1)) { toothHistorySerialIndex = 0; }
        else { toothHistorySerialIndex++; }
      }
      BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
      cmdPending = false;
  }
  else { cmdPending = true; }
}

void testComm()
{
  Serial.write(1);
  return;
}

void commandButtons(int buttonCommand)
{
  switch (buttonCommand)
  {
    case 256:
      BIT_CLEAR(currentStatus.testOutputs, 1);
      endCoil1Charge();
      endCoil2Charge();
      endCoil3Charge();
      endCoil4Charge();
      closeInjector1();
      closeInjector2();
      closeInjector3();
      closeInjector4();
      break;

    case 257:

      BIT_SET(currentStatus.testOutputs, 1);
      break;

    case 513:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector1(); }
      break;

    case 514:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector1(); }
      break;

    case 515:







      break;

    case 516:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector2(); }
      break;

    case 517:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector2(); }
      break;

    case 518:

      break;

    case 519:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector3(); }
      break;

    case 520:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector3(); }
      break;

    case 521:

      break;

    case 522:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ openInjector4(); }
      break;

    case 523:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ){ closeInjector4(); }
      break;

    case 524:

      break;

    case 769:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil1, coilHIGH); }
      break;

    case 770:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil1, coilLOW); }
      break;

    case 771:

      break;

    case 772:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil2, coilHIGH); }
      break;

    case 773:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil2, coilLOW); }
      break;

    case 774:

      break;

    case 775:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil3, coilHIGH); }
      break;

    case 776:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil3, coilLOW); }
      break;

    case 777:

      break;

    case 778:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil4, coilHIGH); }
      break;

    case 779:
        if( BIT_CHECK(currentStatus.testOutputs, 1) ) { digitalWrite(pinCoil4, coilLOW); }
      break;

    case 780:

      break;

    default:
      break;
  }
}
//# 1 "/home/developper/speeduino/speeduino/corrections.ino"
//# 16 "/home/developper/speeduino/speeduino/corrections.ino"
#include "globals.h"
#include "corrections.h"
#include "timers.h"
#include "maths.h"
#include "sensors.h"
#include "src/PID_v1/PID_v1.h"

long PID_O2, PID_output, PID_AFRTarget;
PID egoPID(&PID_O2, &PID_output, &PID_AFRTarget, configPage6.egoKP, configPage6.egoKI, configPage6.egoKD, REVERSE);

void initialiseCorrections()
{
  egoPID.SetMode(AUTOMATIC);
  currentStatus.flexIgnCorrection = 0;
  currentStatus.egoCorrection = 100;
  AFRnextCycle = 0;
  currentStatus.knockActive = false;
}





static inline byte correctionsFuel()
{
  unsigned long sumCorrections = 100;
  byte activeCorrections = 0;
  byte result;


  currentStatus.wueCorrection = correctionWUE();
  if (currentStatus.wueCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.wueCorrection); activeCorrections++; }

  result = correctionASE();
  if (result != 100) { sumCorrections = (sumCorrections * result); activeCorrections++; }

  result = correctionCranking();
  if (result != 100) { sumCorrections = (sumCorrections * result); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.AEamount = correctionAccel();
  if (currentStatus.AEamount != 100) { sumCorrections = (sumCorrections * currentStatus.AEamount); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  result = correctionFloodClear();
  if (result != 100) { sumCorrections = (sumCorrections * result); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.egoCorrection = correctionAFRClosedLoop();
  if (currentStatus.egoCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.egoCorrection); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.batCorrection = correctionBatVoltage();
  if (currentStatus.batCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.batCorrection); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.iatCorrection = correctionIATDensity();
  if (currentStatus.iatCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.iatCorrection); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.flexCorrection = correctionFlex();
  if (currentStatus.flexCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.flexCorrection); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.launchCorrection = correctionLaunch();
  if (currentStatus.launchCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.launchCorrection); activeCorrections++; }

  bitWrite(currentStatus.status1, BIT_STATUS1_DFCO, correctionDFCO());
  if ( bitRead(currentStatus.status1, BIT_STATUS1_DFCO) == 1 ) { sumCorrections = 0; }

  sumCorrections = sumCorrections / powint(100,activeCorrections);

  if(sumCorrections > 255) { sumCorrections = 255; }
  return (byte)sumCorrections;
}





static inline byte correctionWUE()
{
  byte WUEValue;

  if (currentStatus.coolant > (WUETable.axisX[9] - CALIBRATION_TEMPERATURE_OFFSET))
  {

    BIT_CLEAR(currentStatus.engine, BIT_ENGINE_WARMUP);
    WUEValue = WUETable.values[9];
  }
  else
  {
    BIT_SET(currentStatus.engine, BIT_ENGINE_WARMUP);
    WUEValue = table2D_getValue(&WUETable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  }

  return WUEValue;
}





static inline byte correctionCranking()
{
  byte crankingValue = 100;

  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
  {
    crankingValue = table2D_getValue(&crankingEnrichTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  }
  return crankingValue;
}






static inline byte correctionASE()
{
  byte ASEValue;



  if ( (currentStatus.runSecs < (table2D_getValue(&ASECountTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET))) && !(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) )
  {
    BIT_SET(currentStatus.engine, BIT_ENGINE_ASE);
    ASEValue = 100 + table2D_getValue(&ASETable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  }
  else
  {
    BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ASE);
    ASEValue = 100;
  }
  return ASEValue;
}







static inline int16_t correctionAccel()
{
  int16_t accelValue = 100;

  if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC) )
  {

    if( micros_safe() >= currentStatus.AEEndTime )
    {

      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC);
      currentStatus.AEamount = 0;
      accelValue = 100;
      currentStatus.tpsDOT = 0;
    }
    else
    {

      accelValue = currentStatus.AEamount;
    }
  }
  else
  {
    int8_t TPS_change = (currentStatus.TPS - TPSlast);


    if (TPS_change <= 2)
    {
      accelValue = 100;
      currentStatus.tpsDOT = 0;
    }
    else
    {

      int rateOfChange = ldiv(1000000, (TPS_time - TPSlast_time)).quot * TPS_change;
      currentStatus.tpsDOT = rateOfChange / 10;

      if (rateOfChange > configPage2.taeThresh)
      {
        BIT_SET(currentStatus.engine, BIT_ENGINE_ACC);
        currentStatus.AEEndTime = micros_safe() + ((unsigned long)configPage2.aeTime * 10000);
        accelValue = table2D_getValue(&taeTable, currentStatus.tpsDOT);



        uint16_t trueTaperMin = configPage2.taeTaperMin * 100;
        uint16_t trueTaperMax = configPage2.taeTaperMax * 100;
        if (currentStatus.RPM > trueTaperMin)
        {
          if(currentStatus.RPM > trueTaperMax) { accelValue = 0; }
          else
          {
            int16_t taperRange = trueTaperMax - trueTaperMin;
            int16_t taperPercent = ((currentStatus.RPM - trueTaperMin) * 100) / taperRange;
            accelValue = percentage((100-taperPercent), accelValue);
          }
        }
        accelValue = 100 + accelValue;
      }
    }
  }

  return accelValue;
}






static inline byte correctionFloodClear()
{
  byte floodValue = 100;
  if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
  {

    if(currentStatus.TPS >= configPage4.floodClear)
    {

      floodValue = 0;
    }
  }
  return floodValue;
}





static inline byte correctionBatVoltage()
{
  byte batValue = 100;
  if (currentStatus.battery10 > (injectorVCorrectionTable.axisX[5])) { batValue = injectorVCorrectionTable.values[injectorVCorrectionTable.xSize-1]; }
  else { batValue = table2D_getValue(&injectorVCorrectionTable, currentStatus.battery10); }

  return batValue;
}





static inline byte correctionIATDensity()
{
  byte IATValue = 100;
  if ( (currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET) > (IATDensityCorrectionTable.axisX[8])) { IATValue = IATDensityCorrectionTable.values[IATDensityCorrectionTable.xSize-1]; }
  else { IATValue = table2D_getValue(&IATDensityCorrectionTable, currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET); }

  return IATValue;
}





static inline byte correctionLaunch()
{
  byte launchValue = 100;
  if(currentStatus.launchingHard || currentStatus.launchingSoft) { launchValue = (100 + configPage6.lnchFuelAdd); }

  return launchValue;
}




static inline bool correctionDFCO()
{
  bool DFCOValue = false;
  if ( configPage2.dfcoEnabled == 1 )
  {
    if ( bitRead(currentStatus.status1, BIT_STATUS1_DFCO) == 1 ) { DFCOValue = ( currentStatus.RPM > ( configPage4.dfcoRPM * 10) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ); }
    else { DFCOValue = ( currentStatus.RPM > (unsigned int)( (configPage4.dfcoRPM * 10) + configPage4.dfcoHyster) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ); }
  }
  return DFCOValue;
}





static inline byte correctionFlex()
{
  byte flexValue = 100;

  if (configPage2.flexEnabled == 1)
  {
    flexValue = table2D_getValue(&flexFuelTable, currentStatus.ethanolPct);
  }
  return flexValue;
}
//# 326 "/home/developper/speeduino/speeduino/corrections.ino"
static inline byte correctionAFRClosedLoop()
{
  byte AFRValue = 100;
  if( configPage6.egoType > 0 )
  {
    currentStatus.afrTarget = currentStatus.O2;



    if(currentStatus.runSecs > configPage6.ego_sdelay) { currentStatus.afrTarget = get3DTableValue(&afrTable, currentStatus.fuelLoad, currentStatus.RPM); }


    if( (currentStatus.coolant > (int)(configPage6.egoTemp - CALIBRATION_TEMPERATURE_OFFSET)) && (currentStatus.RPM > (unsigned int)(configPage6.egoRPM * 100)) && (currentStatus.TPS < configPage6.egoTPSMax) && (currentStatus.O2 < configPage6.ego_max) && (currentStatus.O2 > configPage6.ego_min) && (currentStatus.runSecs > configPage6.ego_sdelay) )
    {
      AFRValue = currentStatus.egoCorrection;

      if(ignitionCount >= AFRnextCycle)
      {
        AFRnextCycle = ignitionCount + configPage6.egoCount;


        if (configPage6.egoAlgorithm == EGO_ALGORITHM_SIMPLE)
        {


          if(currentStatus.O2 > currentStatus.afrTarget)
          {

            if(currentStatus.egoCorrection < (100 + configPage6.egoLimit) )
            {
              AFRValue = (currentStatus.egoCorrection + 1);
            }
            else { AFRValue = currentStatus.egoCorrection; }
          }
          else if(currentStatus.O2 < currentStatus.afrTarget)
          {

            if(currentStatus.egoCorrection > (100 - configPage6.egoLimit) )
            {
              AFRValue = (currentStatus.egoCorrection - 1);
            }
            else { AFRValue = currentStatus.egoCorrection; }
          }
          else { AFRValue = currentStatus.egoCorrection; }

        }
        else if(configPage6.egoAlgorithm == EGO_ALGORITHM_PID)
        {


          egoPID.SetOutputLimits((long)(-configPage6.egoLimit), (long)(configPage6.egoLimit));
          egoPID.SetTunings(configPage6.egoKP, configPage6.egoKI, configPage6.egoKD);
          PID_O2 = (long)(currentStatus.O2);
          PID_AFRTarget = (long)(currentStatus.afrTarget);

          egoPID.Compute();

          AFRValue = 100 + PID_output;
        }
        else { AFRValue = 100; }
      }
    }
  }

  return AFRValue;
}



int8_t correctionsIgn(int8_t base_advance)
{
  int8_t advance;
  advance = correctionFlexTiming(base_advance);
  advance = correctionIATretard(advance);
  advance = correctionCLTadvance(advance);
  advance = correctionSoftRevLimit(advance);
  advance = correctionNitrous(advance);
  advance = correctionSoftLaunch(advance);
  advance = correctionSoftFlatShift(advance);
  advance = correctionKnock(advance);


  advance = correctionFixedTiming(advance);
  advance = correctionCrankingFixedTiming(advance);

  return advance;
}

static inline int8_t correctionFixedTiming(int8_t advance)
{
  int8_t ignFixValue = advance;
  if (configPage2.fixAngEnable == 1) { ignFixValue = configPage4.FixAng; }
  return ignFixValue;
}

static inline int8_t correctionCrankingFixedTiming(int8_t advance)
{
  byte ignCrankFixValue = advance;
  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { ignCrankFixValue = configPage4.CrankAng; }
  return ignCrankFixValue;
}

static inline int8_t correctionFlexTiming(int8_t advance)
{
  byte ignFlexValue = advance;
  if( configPage2.flexEnabled == 1 )
  {
    currentStatus.flexIgnCorrection = (int8_t)table2D_getValue(&flexAdvTable, currentStatus.ethanolPct);
    ignFlexValue = advance + currentStatus.flexIgnCorrection;
  }
  return ignFlexValue;
}

static inline int8_t correctionIATretard(int8_t advance)
{
  byte ignIATValue = advance;

  int8_t advanceIATadjust = table2D_getValue(&IATRetardTable, currentStatus.IAT);
  int tempAdvance = (advance - advanceIATadjust);
  if (tempAdvance >= -OFFSET_IGNITION) { ignIATValue = tempAdvance; }
  else { ignIATValue = -OFFSET_IGNITION; }

  return ignIATValue;
}

static inline int8_t correctionCLTadvance(int8_t advance)
{
  byte ignCLTValue = advance;

  int8_t advanceCLTadjust = table2D_getValue(&CLTAdvanceTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  ignCLTValue = (advance + advanceCLTadjust/10);

  return ignCLTValue;
}

static inline int8_t correctionSoftRevLimit(int8_t advance)
{
  byte ignSoftRevValue = advance;
  BIT_CLEAR(currentStatus.spark, BIT_SPARK_SFTLIM);
  if (currentStatus.RPM > ((unsigned int)(configPage4.SoftRevLim) * 100) ) { BIT_SET(currentStatus.spark, BIT_SPARK_SFTLIM); ignSoftRevValue = configPage4.SoftLimRetard; }

  return ignSoftRevValue;
}

static inline int8_t correctionNitrous(int8_t advance)
{
  byte ignNitrous = advance;

  if(configPage10.n2o_enable > 0)
  {

    if( currentStatus.nitrous_status == NITROUS_STAGE1 )
    {
      ignNitrous -= configPage10.n2o_stage1_retard;
    }
    if( currentStatus.nitrous_status == NITROUS_STAGE2 )
    {
      ignNitrous -= configPage10.n2o_stage2_retard;
    }
  }

  return ignNitrous;
}

static inline int8_t correctionSoftLaunch(int8_t advance)
{
  byte ignSoftLaunchValue = advance;

  if (configPage6.launchEnabled && clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > ((unsigned int)(configPage6.lnchSoftLim) * 100)) && (currentStatus.TPS >= configPage10.lnchCtrlTPS) )
  {
    currentStatus.launchingSoft = true;
    BIT_SET(currentStatus.spark, BIT_SPARK_SLAUNCH);
    ignSoftLaunchValue = configPage6.lnchRetard;
  }
  else
  {
    currentStatus.launchingSoft = false;
    BIT_CLEAR(currentStatus.spark, BIT_SPARK_SLAUNCH);
  }

  return ignSoftLaunchValue;
}

static inline int8_t correctionSoftFlatShift(int8_t advance)
{
  byte ignSoftFlatValue = advance;

  if(configPage6.flatSEnable && clutchTrigger && (currentStatus.clutchEngagedRPM > ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > (currentStatus.clutchEngagedRPM-configPage6.flatSSoftWin) ) )
  {
    BIT_SET(currentStatus.spark2, BIT_SPARK2_FLATSS);
    ignSoftFlatValue = configPage6.flatSRetard;
  }
  else { BIT_CLEAR(currentStatus.spark2, BIT_SPARK2_FLATSS); }

  return ignSoftFlatValue;
}

static inline int8_t correctionKnock(int8_t advance)
{
  byte knockRetard = 0;


  if( configPage10.knock_mode != KNOCK_MODE_OFF )
  {
    knockWindowMin = table2D_getValue(&knockWindowStartTable, currentStatus.RPM);
    knockWindowMax = knockWindowMin + table2D_getValue(&knockWindowDurationTable, currentStatus.RPM);
  }


  if( (configPage10.knock_mode == KNOCK_MODE_DIGITAL) )
  {

    if(knockCounter > configPage10.knock_count)
    {
      if(currentStatus.knockActive == true)
      {

      }
      else
      {

        lastKnockCount = knockCounter;
        knockStartTime = micros();
        knockRetard = configPage10.knock_firstStep;
      }
    }

  }

  return advance - knockRetard;
}


uint16_t correctionsDwell(uint16_t dwell)
{
  uint16_t tempDwell = dwell;

  currentStatus.dwellCorrection = table2D_getValue(&dwellVCorrectionTable, currentStatus.battery10);
  if (currentStatus.dwellCorrection != 100) { tempDwell = divs100(dwell) * currentStatus.dwellCorrection; }


  uint16_t dwellPerRevolution = tempDwell + (uint16_t)(configPage4.sparkDur * 100);
  int8_t pulsesPerRevolution = 1;

  if( ( (configPage4.sparkMode == IGN_MODE_SINGLE) || (configPage4.sparkMode == IGN_MODE_ROTARY) ) && (configPage2.nCylinders > 1) )
  {
    pulsesPerRevolution = (configPage2.nCylinders >> 1);
    dwellPerRevolution = dwellPerRevolution * pulsesPerRevolution;
  }

  if(dwellPerRevolution > revolutionTime)
  {

    tempDwell = (revolutionTime / pulsesPerRevolution) - (configPage4.sparkDur * 100);
  }
  return tempDwell;
}
//# 1 "/home/developper/speeduino/speeduino/crankMaths.ino"
#include "globals.h"
#include "crankMaths.h"
#include "decoders.h"
#include "timers.h"
//# 18 "/home/developper/speeduino/speeduino/crankMaths.ino"
unsigned long angleToTime(int16_t angle, byte method)
{
    unsigned long returnTime = 0;

    if( (method == CRANKMATH_METHOD_INTERVAL_REV) || (method == CRANKMATH_METHOD_INTERVAL_DEFAULT) )
    {
        returnTime = ((angle * revolutionTime) / 360);

    }
    else if (method == CRANKMATH_METHOD_INTERVAL_TOOTH)
    {

        if(triggerToothAngleIsCorrect == true)
        {
            returnTime = ( ((toothLastToothTime - toothLastMinusOneToothTime) / triggerToothAngle) * angle );
        }
        else { returnTime = angleToTime(angle, CRANKMATH_METHOD_INTERVAL_REV); }
    }

    return returnTime;
}
//# 48 "/home/developper/speeduino/speeduino/crankMaths.ino"
uint16_t timeToAngle(unsigned long time, byte method)
{
    uint16_t returnAngle = 0;

    if( (method == CRANKMATH_METHOD_INTERVAL_REV) || (method == CRANKMATH_METHOD_INTERVAL_DEFAULT) )
    {


        returnAngle = fastTimeToAngle(time);
    }
    else if (method == CRANKMATH_METHOD_INTERVAL_TOOTH)
    {

        if(triggerToothAngleIsCorrect == true)
        {
            returnAngle = ( (unsigned long)(time * triggerToothAngle) / (toothLastToothTime - toothLastMinusOneToothTime) );
        }
        else { returnAngle = timeToAngle(time, CRANKMATH_METHOD_INTERVAL_REV); }
    }
    else if (method == CRANKMATH_METHOD_ALPHA_BETA)
    {

        returnAngle = timeToAngle(time, CRANKMATH_METHOD_INTERVAL_REV);
    }
    else if (method == CRANKMATH_METHOD_2ND_DERIVATIVE)
    {

        returnAngle = timeToAngle(time, CRANKMATH_METHOD_INTERVAL_REV);
    }

   return returnAngle;

}

void doCrankSpeedCalcs()
{





      if( (secondDerivEnabled > 0) && (toothHistoryIndex >= 3) && (currentStatus.RPM < 2000) )
      {


        {
          deltaToothCount = toothCurrentCount;
          int angle1, angle2;
          if(configPage4.TrigPattern == 4)
          {

            angle2 = triggerToothAngle;
            if (angle2 == 70) { angle1 = 110; }
            else { angle1 = 70; }
          }
          else if(configPage4.TrigPattern == 0)
          {

            if(toothCurrentCount == 1) { angle2 = 2*triggerToothAngle; angle1 = triggerToothAngle; }
            else if(toothCurrentCount == 2) { angle1 = 2*triggerToothAngle; angle2 = triggerToothAngle; }
            else { angle1 = triggerToothAngle; angle2 = triggerToothAngle; }
          }
          else { angle1 = triggerToothAngle; angle2 = triggerToothAngle; }

          long toothDeltaV = (1000000L * angle2 / toothHistory[toothHistoryIndex]) - (1000000L * angle1 / toothHistory[toothHistoryIndex-1]);
          long toothDeltaT = toothHistory[toothHistoryIndex];


          rpmDelta = (toothDeltaV << 10) / (6 * toothDeltaT);
        }

          timePerDegreex16 = ldiv( 2666656L, currentStatus.RPM + rpmDelta).quot;
          timePerDegree = timePerDegreex16 / 16;
      }
      else
      {

        noInterrupts();
        if( (triggerToothAngleIsCorrect == true) && (toothLastToothTime > toothLastMinusOneToothTime) && (abs(currentStatus.rpmDOT) > 30) )
        {

          unsigned long tempToothLastToothTime = toothLastToothTime;
          unsigned long tempToothLastMinusOneToothTime = toothLastMinusOneToothTime;
          uint16_t tempTriggerToothAngle = triggerToothAngle;
          interrupts();
          timePerDegreex16 = (unsigned long)( (tempToothLastToothTime - tempToothLastMinusOneToothTime)*16) / tempTriggerToothAngle;
          timePerDegree = timePerDegreex16 / 16;
        }
        else
        {

          interrupts();


          long rpm_adjust = 0;
          timePerDegreex16 = ldiv( 2666656L, currentStatus.RPM + rpm_adjust).quot;
          timePerDegree = timePerDegreex16 / 16;
        }
      }
      degreesPeruSx2048 = 2048 / timePerDegree;
      degreesPeruSx32768 = 524288 / timePerDegreex16;
}
//# 1 "/home/developper/speeduino/speeduino/decoders.ino"
//# 23 "/home/developper/speeduino/speeduino/decoders.ino"
#include <limits.h>
#include "globals.h"
#include "decoders.h"
#include "scheduledIO.h"
#include "scheduler.h"
#include "crankMaths.h"





static inline void addToothLogEntry(unsigned long toothTime, bool whichTooth)
{

  if( (currentStatus.toothLogEnabled == true) || (currentStatus.compositeLogEnabled == true) )
  {
    bool valueLogged = false;
    if(currentStatus.toothLogEnabled == true)
    {

      if(whichTooth == TOOTH_CRANK)
      {
        toothHistory[toothHistoryIndex] = toothTime;
        valueLogged = true;
      }
    }
    else if(currentStatus.compositeLogEnabled == true)
    {
      compositeLogHistory[toothHistoryIndex] = 0;
      if(READ_PRI_TRIGGER() == true) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_PRI); }
      if(READ_SEC_TRIGGER() == true) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_SEC); }
      if(whichTooth == TOOTH_CAM) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_TRIG); }
      if(currentStatus.hasSync == true) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_SYNC); }

      toothHistory[toothHistoryIndex] = micros() - compositeLastToothTime;
      compositeLastToothTime = micros();
      valueLogged = true;
    }


    if(valueLogged == true)
    {
      if(toothHistoryIndex == (TOOTH_LOG_BUFFER-1)) { toothHistoryIndex = 0; }
      else { toothHistoryIndex++; }

      uint16_t absoluteToothHistoryIndex = toothHistoryIndex;
      if(toothHistoryIndex < toothHistorySerialIndex)
      {

        absoluteToothHistoryIndex += TOOTH_LOG_BUFFER;
      }

      if( (absoluteToothHistoryIndex - toothHistorySerialIndex) >= TOOTH_LOG_SIZE ) { BIT_SET(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY); }
      else { BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY); }
    }


  }
}





void loggerPrimaryISR()
{
  validTrigger = false;
  bool validEdge = false;







  if( ( (primaryTriggerEdge == RISING) && (READ_PRI_TRIGGER() == HIGH) ) || ( (primaryTriggerEdge == FALLING) && (READ_PRI_TRIGGER() == LOW) ) || (primaryTriggerEdge == CHANGE) )
  {
    triggerHandler();
    validEdge = true;
  }
  if( (currentStatus.toothLogEnabled == true) && (validTrigger == true) )
  {

    if(validEdge == true) { addToothLogEntry(curGap, TOOTH_CRANK); }
  }

  else if( (currentStatus.compositeLogEnabled == true) )
  {

    addToothLogEntry(curGap, TOOTH_CRANK);
  }
}




void loggerSecondaryISR()
{
  validTrigger = false;
  validTrigger = true;






  if( ( (secondaryTriggerEdge == RISING) && (READ_SEC_TRIGGER() == HIGH) ) || ( (secondaryTriggerEdge == FALLING) && (READ_SEC_TRIGGER() == LOW) ) || (secondaryTriggerEdge == CHANGE) )
  {
    triggerSecondaryHandler();
  }

  if( (currentStatus.compositeLogEnabled == true) && (validTrigger == true) )
  {

    addToothLogEntry(curGap2, TOOTH_CAM);
  }
}






static inline uint16_t stdGetRPM(uint16_t degreesOver)
{
  uint16_t tempRPM = 0;

  if( currentStatus.hasSync == true )
  {
    if( (currentStatus.RPM < currentStatus.crankRPM) && (currentStatus.startRevolutions == 0) ) { tempRPM = 0; }
    else if( (toothOneTime == 0) || (toothOneMinusOneTime == 0) ) { tempRPM = 0; }
    else
    {
      noInterrupts();
      revolutionTime = (toothOneTime - toothOneMinusOneTime);
      interrupts();
      if(degreesOver == 720) { revolutionTime = revolutionTime / 2; }
      tempRPM = (US_IN_MINUTE / revolutionTime);
      if(tempRPM >= MAX_RPM) { tempRPM = currentStatus.RPM; }
    }
  }
  else { tempRPM = 0; }

  return tempRPM;
}





static inline void setFilter(unsigned long curGap)
{
   if(configPage4.triggerFilter == 1) { triggerFilterTime = curGap >> 2; }
   else if(configPage4.triggerFilter == 2) { triggerFilterTime = curGap >> 1; }
   else if (configPage4.triggerFilter == 3) { triggerFilterTime = (curGap * 3) >> 2; }
   else { triggerFilterTime = 0; }
}







static inline int crankingGetRPM(byte totalTeeth)
{
  uint16_t tempRPM = 0;
  if( (currentStatus.startRevolutions >= configPage4.StgCycles) && (currentStatus.hasSync == true) )
  {
    if( (toothLastToothTime > 0) && (toothLastMinusOneToothTime > 0) && (toothLastToothTime > toothLastMinusOneToothTime) )
    {
      noInterrupts();
      revolutionTime = (toothLastToothTime - toothLastMinusOneToothTime) * totalTeeth;
      interrupts();
      tempRPM = (US_IN_MINUTE / revolutionTime);
      if( tempRPM >= MAX_RPM ) { tempRPM = currentStatus.RPM; }
    }
  }

  return tempRPM;
}







#define MIN_CYCLES_FOR_ENDCOMPARE 6
#define checkPerToothTiming(crankAngle,currentTooth) \
{ \
  if ( (fixedCrankingOverride == 0) && (currentStatus.RPM > 0) ) \
  { \
    if ( (currentTooth == ignition1EndTooth) ) \
    { \
      if( (ignitionSchedule1.Status == RUNNING) ) { IGN1_COMPARE = IGN1_COUNTER + uS_TO_TIMER_COMPARE( fastDegreesToUS( ignitionLimits( (ignition1EndAngle - crankAngle) ) ) ); } \
      else if(currentStatus.startRevolutions > MIN_CYCLES_FOR_ENDCOMPARE) { ignitionSchedule1.endCompare = IGN1_COUNTER + uS_TO_TIMER_COMPARE( fastDegreesToUS( ignitionLimits( (ignition1EndAngle - crankAngle) ) ) ); ignitionSchedule1.endScheduleSetByDecoder = true; } \
    } \
  \
    else if ( (currentTooth == ignition2EndTooth) ) \
    { \
      if( (ignitionSchedule2.Status == RUNNING) ) { IGN2_COMPARE = IGN2_COUNTER + uS_TO_TIMER_COMPARE( fastDegreesToUS( ignitionLimits( (ignition2EndAngle - crankAngle) ) ) ); } \
      else if(currentStatus.startRevolutions > MIN_CYCLES_FOR_ENDCOMPARE) { ignitionSchedule2.endCompare = IGN2_COUNTER + uS_TO_TIMER_COMPARE( fastDegreesToUS( ignitionLimits( (ignition2EndAngle - crankAngle) ) ) ); ignitionSchedule2.endScheduleSetByDecoder = true; } \
    } \
  \
    else if ( (currentTooth == ignition3EndTooth) ) \
    { \
      if( (ignitionSchedule3.Status == RUNNING) ) { IGN3_COMPARE = IGN3_COUNTER + uS_TO_TIMER_COMPARE( fastDegreesToUS( ignitionLimits( (ignition3EndAngle - crankAngle) ) ) ); } \
      else if(currentStatus.startRevolutions > MIN_CYCLES_FOR_ENDCOMPARE) { ignitionSchedule3.endCompare = IGN3_COUNTER + uS_TO_TIMER_COMPARE( fastDegreesToUS( ignitionLimits( (ignition3EndAngle - crankAngle) ) ) ); ignitionSchedule3.endScheduleSetByDecoder = true; } \
    } \
    else if ( (currentTooth == ignition4EndTooth) ) \
    { \
      if( (ignitionSchedule4.Status == RUNNING) ) { IGN4_COMPARE = IGN4_COUNTER + uS_TO_TIMER_COMPARE_SLOW( fastDegreesToUS( ignitionLimits( (ignition4EndAngle - crankAngle) ) ) ); } \
      else if(currentStatus.startRevolutions > MIN_CYCLES_FOR_ENDCOMPARE) { ignitionSchedule4.endCompare = IGN4_COUNTER + uS_TO_TIMER_COMPARE_SLOW( fastDegreesToUS( ignitionLimits( (ignition4EndAngle - crankAngle) ) ) ); ignitionSchedule4.endScheduleSetByDecoder = true; } \
    } \
  } \
}






void triggerSetup_missingTooth()
{
  triggerToothAngle = 360 / configPage4.triggerTeeth;
  if(configPage4.TrigSpeed == CAM_SPEED) { triggerToothAngle = 720 / configPage4.triggerTeeth; }
  triggerActualTeeth = configPage4.triggerTeeth - configPage4.triggerMissingTeeth;
  triggerFilterTime = (int)(1000000 / (MAX_RPM / 60 * configPage4.triggerTeeth));
  if (configPage4.trigPatternSec == SEC_TRIGGER_4_1)
  {
    triggerSecFilterTime = 1000000 * 60 / MAX_RPM / 4 / 2;
  }
  else
  {
    triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60));
  }
  secondDerivEnabled = false;
  decoderIsSequential = false;
  checkSyncToothCount = (configPage4.triggerTeeth) >> 1;
  toothLastMinusOneToothTime = 0;
  toothCurrentCount = 0;
  toothOneTime = 0;
  toothOneMinusOneTime = 0;
  MAX_STALL_TIME = (3333UL * triggerToothAngle * (configPage4.triggerMissingTeeth + 1));
}

void triggerPri_missingTooth()
{
   curTime = micros();
   curGap = curTime - toothLastToothTime;
   if ( curGap >= triggerFilterTime )
   {
     toothCurrentCount++;
     validTrigger = true;


     if( (toothLastToothTime > 0) && (toothLastMinusOneToothTime > 0) )
     {


       if(configPage4.triggerMissingTeeth == 1) { targetGap = (3 * (toothLastToothTime - toothLastMinusOneToothTime)) >> 1; }
       else { targetGap = ((toothLastToothTime - toothLastMinusOneToothTime)) * configPage4.triggerMissingTeeth; }

       if( (toothLastToothTime == 0) || (toothLastMinusOneToothTime == 0) ) { curGap = 0; }

       if ( (curGap > targetGap) || (toothCurrentCount > triggerActualTeeth) )
       {

         if( (toothCurrentCount < triggerActualTeeth) && (currentStatus.hasSync == true) )
         {

            currentStatus.hasSync = false;
            currentStatus.syncLossCounter++;
         }


         else
         {
           if(currentStatus.hasSync == true)
           {
             currentStatus.startRevolutions++;
             if ( configPage4.TrigSpeed == CAM_SPEED ) { currentStatus.startRevolutions++; }
           }
           else { currentStatus.startRevolutions = 0; }

           toothCurrentCount = 1;
           revolutionOne = !revolutionOne;
           toothOneMinusOneTime = toothOneTime;
           toothOneTime = curTime;
           currentStatus.hasSync = true;

           triggerFilterTime = 0;
           toothLastMinusOneToothTime = toothLastToothTime;
           toothLastToothTime = curTime;
           triggerToothAngleIsCorrect = false;
         }
       }
       else
       {

         setFilter(curGap);
         toothLastMinusOneToothTime = toothLastToothTime;
         toothLastToothTime = curTime;
         triggerToothAngleIsCorrect = true;
       }

     }
     else
     {
       toothLastMinusOneToothTime = toothLastToothTime;
       toothLastToothTime = curTime;
     }



     if( (configPage2.perToothIgn == true) && (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) )
     {


        int16_t crankAngle = ( (toothCurrentCount-1) * triggerToothAngle ) + configPage4.triggerAngle;



        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (revolutionOne == true) && (configPage4.TrigSpeed == CRANK_SPEED) )
        {
          crankAngle += 360;
          checkPerToothTiming(crankAngle, (configPage4.triggerTeeth + toothCurrentCount));
        }
        else{ checkPerToothTiming(crankAngle, toothCurrentCount); }
     }
   }
}

void triggerSec_missingTooth()
{
  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;


  if( (toothLastSecToothTime == 0) )
  {
    curGap2 = 0;
    toothLastSecToothTime = curTime2;
  }

  if ( curGap2 >= triggerSecFilterTime )
  {
    if ( configPage4.trigPatternSec == SEC_TRIGGER_4_1 )
    {
      targetGap2 = (3 * (toothLastSecToothTime - toothLastMinusOneSecToothTime)) >> 1;
      toothLastMinusOneSecToothTime = toothLastSecToothTime;
      if ( (curGap2 >= targetGap2) || (secondaryToothCount > 3) )
      {
        secondaryToothCount = 1;
        revolutionOne = 1;
        triggerSecFilterTime = 0;
      }
      else
      {
        triggerSecFilterTime = curGap2 >> 2;
        secondaryToothCount++;
      }
    }
    else
    {

      revolutionOne = 1;
      triggerSecFilterTime = curGap2 >> 1;
    }
    toothLastSecToothTime = curTime2;
  }
}

uint16_t getRPM_missingTooth()
{
  uint16_t tempRPM = 0;
  if( currentStatus.RPM < currentStatus.crankRPM )
  {
    if(toothCurrentCount != 1)
    {
      if(configPage4.TrigSpeed == CAM_SPEED) { tempRPM = crankingGetRPM(configPage4.triggerTeeth/2); }
      else { tempRPM = crankingGetRPM(configPage4.triggerTeeth); }
    }
    else { tempRPM = currentStatus.RPM; }
  }
  else
  {
    if(configPage4.TrigSpeed == CAM_SPEED) { tempRPM = stdGetRPM(720); }
    else { tempRPM = stdGetRPM(360); }
  }
  return tempRPM;
}

int getCrankAngle_missingTooth()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    bool tempRevolutionOne;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempRevolutionOne = revolutionOne;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();

    int crankAngle = ((tempToothCurrentCount - 1) * triggerToothAngle) + configPage4.triggerAngle;


    if ( (tempRevolutionOne == true) && (configPage4.TrigSpeed == CRANK_SPEED) ) { crankAngle += 360; }

    lastCrankAngleCalc = micros();
    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);

    if (crankAngle >= 720) { crankAngle -= 720; }
    else if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += CRANK_ANGLE_MAX; }

    return crankAngle;
}

void triggerSetEndTeeth_missingTooth()
{
  byte toothAdder = 0;
  if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage4.TrigSpeed == CRANK_SPEED) ) { toothAdder = configPage4.triggerTeeth; }

  ignition1EndTooth = ( (ignition1EndAngle - configPage4.triggerAngle) / (int16_t)(triggerToothAngle) ) - 1;
  if(ignition1EndTooth > (configPage4.triggerTeeth + toothAdder)) { ignition1EndTooth -= (configPage4.triggerTeeth + toothAdder); }
  if(ignition1EndTooth <= 0) { ignition1EndTooth += (configPage4.triggerTeeth + toothAdder); }
  if(ignition1EndTooth > (triggerActualTeeth + toothAdder)) { ignition1EndTooth = (triggerActualTeeth + toothAdder); }

  ignition2EndTooth = ( (ignition2EndAngle - configPage4.triggerAngle) / (int16_t)(triggerToothAngle) ) - 1;
  if(ignition2EndTooth > (configPage4.triggerTeeth + toothAdder)) { ignition2EndTooth -= (configPage4.triggerTeeth + toothAdder); }
  if(ignition2EndTooth <= 0) { ignition2EndTooth += (configPage4.triggerTeeth + toothAdder); }
  if(ignition2EndTooth > (triggerActualTeeth + toothAdder)) { ignition2EndTooth = (triggerActualTeeth + toothAdder); }

  ignition3EndTooth = ( (ignition3EndAngle - configPage4.triggerAngle) / (int16_t)(triggerToothAngle) ) - 1;
  if(ignition3EndTooth > (configPage4.triggerTeeth + toothAdder)) { ignition3EndTooth -= (configPage4.triggerTeeth + toothAdder); }
  if(ignition3EndTooth <= 0) { ignition3EndTooth += (configPage4.triggerTeeth + toothAdder); }
  if(ignition3EndTooth > (triggerActualTeeth + toothAdder)) { ignition3EndTooth = (triggerActualTeeth + toothAdder); }

  ignition4EndTooth = ( (ignition4EndAngle - configPage4.triggerAngle) / (int16_t)(triggerToothAngle) ) - 1;
  if(ignition4EndTooth > (configPage4.triggerTeeth + toothAdder)) { ignition4EndTooth -= (configPage4.triggerTeeth + toothAdder); }
  if(ignition4EndTooth <= 0) { ignition4EndTooth += (configPage4.triggerTeeth + toothAdder); }
  if(ignition4EndTooth > (triggerActualTeeth + toothAdder)) { ignition4EndTooth = (triggerActualTeeth + toothAdder); }

  lastToothCalcAdvance = currentStatus.advance;
}






void triggerSetup_DualWheel()
{
  triggerToothAngle = 360 / configPage4.triggerTeeth;
  if(configPage4.TrigSpeed == 1) { triggerToothAngle = 720 / configPage4.triggerTeeth; }
  toothCurrentCount = 255;
  triggerFilterTime = (int)(1000000 / (MAX_RPM / 60 * configPage4.triggerTeeth));
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2;
  secondDerivEnabled = false;
  decoderIsSequential = true;
  triggerToothAngleIsCorrect = true;
  MAX_STALL_TIME = (3333UL * triggerToothAngle);
}


void triggerPri_DualWheel()
{
    curTime = micros();
    curGap = curTime - toothLastToothTime;
    if ( curGap >= triggerFilterTime )
    {
      toothCurrentCount++;
      validTrigger = true;

      toothLastMinusOneToothTime = toothLastToothTime;
      toothLastToothTime = curTime;

      if ( currentStatus.hasSync == true )
      {
        if ( (toothCurrentCount == 1) || (toothCurrentCount > configPage4.triggerTeeth) )
        {
          toothCurrentCount = 1;
          revolutionOne = !revolutionOne;
          toothOneMinusOneTime = toothOneTime;
          toothOneTime = curTime;
          currentStatus.startRevolutions++;
        }

        setFilter(curGap);
      }


      if( (configPage2.perToothIgn == true) && (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) )
      {
        int16_t crankAngle = ( (toothCurrentCount-1) * triggerToothAngle ) + configPage4.triggerAngle;
        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (revolutionOne == true) && (configPage4.TrigSpeed == CRANK_SPEED) )
        {
          crankAngle += 360;
          checkPerToothTiming(crankAngle, (configPage4.triggerTeeth + toothCurrentCount));
        }
        else{ checkPerToothTiming(crankAngle, toothCurrentCount); }
      }
   }
}

void triggerSec_DualWheel()
{
  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;
  if ( curGap2 >= triggerSecFilterTime )
  {
    toothLastSecToothTime = curTime2;
    triggerSecFilterTime = curGap2 >> 2;

    if(currentStatus.hasSync == false)
    {
      toothLastToothTime = micros();
      toothLastMinusOneToothTime = micros() - (6000000 / configPage4.triggerTeeth);
      toothCurrentCount = configPage4.triggerTeeth;

      currentStatus.hasSync = true;
    }
    else
    {
      if (toothCurrentCount != configPage4.triggerTeeth) { currentStatus.syncLossCounter++; }
      if (configPage4.useResync == 1) { toothCurrentCount = configPage4.triggerTeeth; }
    }

    revolutionOne = 1;
  }
}

uint16_t getRPM_DualWheel()
{
  uint16_t tempRPM = 0;
  if( currentStatus.hasSync == true )
  {
    if(currentStatus.RPM < currentStatus.crankRPM) { tempRPM = crankingGetRPM(configPage4.triggerTeeth); }
    else { tempRPM = stdGetRPM(360); }
  }
  return tempRPM;
}

int getCrankAngle_DualWheel()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    bool tempRevolutionOne;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    tempRevolutionOne = revolutionOne;
    lastCrankAngleCalc = micros();
    interrupts();


    if(tempToothCurrentCount == 0) { tempToothCurrentCount = configPage4.triggerTeeth; }

    int crankAngle = ((tempToothCurrentCount - 1) * triggerToothAngle) + configPage4.triggerAngle;

    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);


    if (tempRevolutionOne) { crankAngle += 360; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += CRANK_ANGLE_MAX; }

    return crankAngle;
}

void triggerSetEndTeeth_DualWheel()
{

  byte toothAdder = 0;
  if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage4.TrigSpeed == CRANK_SPEED) ) { toothAdder = configPage4.triggerTeeth; }

  ignition1EndTooth = ( (ignition1EndAngle - configPage4.triggerAngle) / (int16_t)(triggerToothAngle) ) - 1;
  if(ignition1EndTooth > (configPage4.triggerTeeth + toothAdder)) { ignition1EndTooth -= (configPage4.triggerTeeth + toothAdder); }
  if(ignition1EndTooth <= 0) { ignition1EndTooth += configPage4.triggerTeeth; }
  if(ignition1EndTooth > (triggerActualTeeth + toothAdder)) { ignition1EndTooth = (triggerActualTeeth + toothAdder); }

  ignition2EndTooth = ( (ignition2EndAngle - configPage4.triggerAngle) / (int16_t)(triggerToothAngle) ) - 1;
  if(ignition2EndTooth > (configPage4.triggerTeeth + toothAdder)) { ignition2EndTooth -= (configPage4.triggerTeeth + toothAdder); }
  if(ignition2EndTooth <= 0) { ignition2EndTooth += configPage4.triggerTeeth; }
  if(ignition1EndTooth > (triggerActualTeeth + toothAdder)) { ignition3EndTooth = (triggerActualTeeth + toothAdder); }

  ignition3EndTooth = ( (ignition3EndAngle - configPage4.triggerAngle) / (int16_t)(triggerToothAngle) ) - 1;
  if(ignition3EndTooth > (configPage4.triggerTeeth + toothAdder)) { ignition3EndTooth -= (configPage4.triggerTeeth + toothAdder); }
  if(ignition3EndTooth <= 0) { ignition3EndTooth += configPage4.triggerTeeth; }
  if(ignition3EndTooth > (triggerActualTeeth + toothAdder)) { ignition3EndTooth = (triggerActualTeeth + toothAdder); }

  ignition4EndTooth = ( (ignition4EndAngle - configPage4.triggerAngle) / (int16_t)(triggerToothAngle) ) - 1;
  if(ignition4EndTooth > (configPage4.triggerTeeth + toothAdder)) { ignition4EndTooth -= (configPage4.triggerTeeth + toothAdder); }
  if(ignition4EndTooth <= 0) { ignition4EndTooth += configPage4.triggerTeeth; }
  if(ignition4EndTooth > (triggerActualTeeth + toothAdder)) { ignition4EndTooth = (triggerActualTeeth + toothAdder); }

  lastToothCalcAdvance = currentStatus.advance;

}







void triggerSetup_BasicDistributor()
{
  triggerActualTeeth = configPage2.nCylinders;
  if(triggerActualTeeth == 0) { triggerActualTeeth = 1; }
  triggerToothAngle = 720 / triggerActualTeeth;
  triggerFilterTime = 60000000L / MAX_RPM / configPage2.nCylinders;
  triggerFilterTime = triggerFilterTime / 2;
  triggerFilterTime = 0;
  secondDerivEnabled = false;
  decoderIsSequential = false;
  toothCurrentCount = 0;
  decoderHasFixedCrankingTiming = true;
  triggerToothAngleIsCorrect = true;
  if(configPage2.nCylinders <= 4) { MAX_STALL_TIME = (1851UL * triggerToothAngle); }
  else { MAX_STALL_TIME = (3200UL * triggerToothAngle); }

}

void triggerPri_BasicDistributor()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( (curGap >= triggerFilterTime) )
  {
    if( (toothCurrentCount == triggerActualTeeth) || (currentStatus.hasSync == false) )
    {
       toothCurrentCount = 1;
       toothOneMinusOneTime = toothOneTime;
       toothOneTime = curTime;
       currentStatus.hasSync = true;
       currentStatus.startRevolutions++;
    }
    else
    {
      if( (toothCurrentCount < triggerActualTeeth) ) { toothCurrentCount++; }
      else
      {


        if( currentStatus.hasSync == true )
        {
          currentStatus.syncLossCounter++;
          currentStatus.hasSync = false;
        }
      }

    }

    setFilter(curGap);
    validTrigger = true;

    if ( configPage4.ignCranklock && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
    {
      endCoil1Charge();
      endCoil2Charge();
      endCoil3Charge();
      endCoil4Charge();
    }

    if(configPage2.perToothIgn == true)
    {
      uint16_t crankAngle = ( (toothCurrentCount-1) * triggerToothAngle ) + configPage4.triggerAngle;
      crankAngle = ignitionLimits((crankAngle));
      if(toothCurrentCount > (triggerActualTeeth/2) ) { checkPerToothTiming(crankAngle, (toothCurrentCount - (triggerActualTeeth/2))); }
      else { checkPerToothTiming(crankAngle, toothCurrentCount); }
    }

    toothLastMinusOneToothTime = toothLastToothTime;
    toothLastToothTime = curTime;
  }
}
void triggerSec_BasicDistributor() { return; }
uint16_t getRPM_BasicDistributor()
{
  uint16_t tempRPM;
  if( currentStatus.RPM < currentStatus.crankRPM)
  {
    tempRPM = crankingGetRPM(triggerActualTeeth) << 1;
    revolutionTime = revolutionTime >> 1;
  }
  else { tempRPM = stdGetRPM(720); }

  MAX_STALL_TIME = revolutionTime << 1;
  if(triggerActualTeeth == 1) { MAX_STALL_TIME = revolutionTime << 1; }
  if(MAX_STALL_TIME < 366667UL) { MAX_STALL_TIME = 366667UL; }

  return tempRPM;

}
int getCrankAngle_BasicDistributor()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    lastCrankAngleCalc = micros();
    interrupts();

    int crankAngle = ((tempToothCurrentCount - 1) * triggerToothAngle) + configPage4.triggerAngle;


    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);


    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_TOOTH);


    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += CRANK_ANGLE_MAX; }

    return crankAngle;
}

void triggerSetEndTeeth_BasicDistributor()
{

  int tempEndAngle = (ignition1EndAngle - configPage4.triggerAngle);
  tempEndAngle = ignitionLimits((tempEndAngle));


  if( (tempEndAngle > 180) || (tempEndAngle <= 0) )
  {
    ignition1EndTooth = 2;
    ignition2EndTooth = 1;
  }
  else
  {
    ignition1EndTooth = 1;
    ignition2EndTooth = 2;
  }


  lastToothCalcAdvance = currentStatus.advance;
}







void triggerSetup_GM7X()
{
  triggerToothAngle = 360 / 6;
  secondDerivEnabled = false;
  decoderIsSequential = false;
  MAX_STALL_TIME = (3333UL * triggerToothAngle);
}

void triggerPri_GM7X()
{
    lastGap = curGap;
    curTime = micros();
    curGap = curTime - toothLastToothTime;
    toothCurrentCount++;
    validTrigger = true;

    if( (toothLastToothTime > 0) && (toothLastMinusOneToothTime > 0) )
    {
      if( toothCurrentCount > 7 )
      {
        toothCurrentCount = 1;
        toothOneMinusOneTime = toothOneTime;
        toothOneTime = curTime;

        triggerToothAngleIsCorrect = true;
      }
      else
      {
        targetGap = (lastGap) >> 1;
        if ( curGap < targetGap )
        {
          toothCurrentCount = 3;
          currentStatus.hasSync = true;
          triggerToothAngleIsCorrect = false;
          currentStatus.startRevolutions++;
        }
        else
        {
          triggerToothAngleIsCorrect = true;
        }
      }
    }


     if(configPage2.perToothIgn == true)
     {
       if(toothCurrentCount != 3)
       {
          uint16_t crankAngle;
          if( toothCurrentCount < 3 )
          {
            crankAngle = ((toothCurrentCount - 1) * triggerToothAngle) + 42;
          }
          else
          {
            crankAngle = ((toothCurrentCount - 2) * triggerToothAngle) + 42;
          }
          checkPerToothTiming(crankAngle, toothCurrentCount);
       }

     }

    toothLastMinusOneToothTime = toothLastToothTime;
    toothLastToothTime = curTime;


}
void triggerSec_GM7X() { return; }
uint16_t getRPM_GM7X()
{
   return stdGetRPM(360);
}
int getCrankAngle_GM7X()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    lastCrankAngleCalc = micros();
    interrupts();


    int crankAngle;
    if( tempToothCurrentCount < 3 )
    {
      crankAngle = ((tempToothCurrentCount - 1) * triggerToothAngle) + 42;
    }
    else if( tempToothCurrentCount == 3 )
    {
      crankAngle = 112;
    }
    else
    {
      crankAngle = ((tempToothCurrentCount - 2) * triggerToothAngle) + 42;
    }


    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

void triggerSetEndTeeth_GM7X()
{

  lastToothCalcAdvance = currentStatus.advance;

  if(currentStatus.advance < 18 )
  {
    ignition1EndTooth = 7;
    ignition2EndTooth = 2;
    ignition3EndTooth = 5;
  }
  else
  {
    ignition1EndTooth = 6;
    ignition2EndTooth = 1;
    ignition3EndTooth = 4;
  }
}
//# 921 "/home/developper/speeduino/speeduino/decoders.ino"
void triggerSetup_4G63()
{
  triggerToothAngle = 180;
  toothCurrentCount = 99;
  secondDerivEnabled = false;
  decoderIsSequential = true;
  decoderHasFixedCrankingTiming = true;
  triggerToothAngleIsCorrect = true;
  MAX_STALL_TIME = 366667UL;
  if(initialisationComplete == false) { toothLastToothTime = micros(); }



  if(configPage2.nCylinders == 6)
  {

    toothAngles[0] = 715;
    toothAngles[1] = 45;
    toothAngles[2] = 115;
    toothAngles[3] = 165;
    toothAngles[4] = 235;
    toothAngles[5] = 285;

    toothAngles[6] = 355;
    toothAngles[7] = 405;
    toothAngles[8] = 475;
    toothAngles[9] = 525;
    toothAngles[10] = 595;
    toothAngles[11] = 645;

    triggerActualTeeth = 12;
  }
  else
  {

    toothAngles[0] = 715;
    toothAngles[1] = 105;
    toothAngles[2] = 175;
    toothAngles[3] = 285;

    toothAngles[4] = 355;
    toothAngles[5] = 465;
    toothAngles[6] = 535;
    toothAngles[7] = 645;

    triggerActualTeeth = 8;
  }
//# 984 "/home/developper/speeduino/speeduino/decoders.ino"
  triggerFilterTime = 1500;
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2;
  triggerSecFilterTime_duration = 4000;
  secondaryLastToothTime = 0;
}

void triggerPri_4G63()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( (curGap >= triggerFilterTime) || (currentStatus.startRevolutions == 0) )
  {
    validTrigger = true;
    triggerFilterTime = curGap >> 2;

    toothLastMinusOneToothTime = toothLastToothTime;
    toothLastToothTime = curTime;

    toothCurrentCount++;

    if( (toothCurrentCount == 1) || (toothCurrentCount > triggerActualTeeth) )
    {
       toothCurrentCount = 1;
       toothOneMinusOneTime = toothOneTime;
       toothOneTime = curTime;
       currentStatus.startRevolutions++;
    }

    if (currentStatus.hasSync == true)
    {
      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && configPage4.ignCranklock && (currentStatus.startRevolutions >= configPage4.StgCycles))
      {
        if(configPage2.nCylinders == 4)
        {

          if( (toothCurrentCount == 1) || (toothCurrentCount == 5) ) { endCoil1Charge(); endCoil3Charge(); }
          else if( (toothCurrentCount == 3) || (toothCurrentCount == 7) ) { endCoil2Charge(); endCoil4Charge(); }
        }
        else if(configPage2.nCylinders == 6)
        {
          if( (toothCurrentCount == 1) || (toothCurrentCount == 7) ) { endCoil1Charge(); }
          else if( (toothCurrentCount == 3) || (toothCurrentCount == 9) ) { endCoil2Charge(); }
          else if( (toothCurrentCount == 5) || (toothCurrentCount == 11) ) { endCoil3Charge(); }
        }
      }


      if( (configPage4.triggerFilter == 1) || (currentStatus.RPM < 1400) )
      {

        if( (toothCurrentCount == 1) || (toothCurrentCount == 3) || (toothCurrentCount == 5) || (toothCurrentCount == 7) || (toothCurrentCount == 9) || (toothCurrentCount == 11) )
        {
          if(configPage2.nCylinders == 4)
          {
            triggerToothAngle = 70;
            triggerFilterTime = curGap;
          }
          else if(configPage2.nCylinders == 6)
          {
            triggerToothAngle = 70;
            triggerFilterTime = (curGap >> 2);
          }
        }
        else
        {
          if(configPage2.nCylinders == 4)
          {
            triggerToothAngle = 110;
            triggerFilterTime = (curGap * 3) >> 3;
          }
          else if(configPage2.nCylinders == 6)
          {
            triggerToothAngle = 50;
            triggerFilterTime = curGap >> 1;
          }
        }
      }
      else if(configPage4.triggerFilter == 2)
      {

        if( (toothCurrentCount == 1) || (toothCurrentCount == 3) || (toothCurrentCount == 5) || (toothCurrentCount == 7) || (toothCurrentCount == 9) || (toothCurrentCount == 11) )
        {
          triggerToothAngle = 70;
          if(configPage2.nCylinders == 4)
          {
            triggerFilterTime = (curGap * 5) >> 2 ;
          }
          else
          {
            triggerFilterTime = curGap >> 1 ;
          }
        }
        else
        {
          if(configPage2.nCylinders == 4)
          {
            triggerToothAngle = 110;
            triggerFilterTime = (curGap >> 1);
          }
          else
          {
            triggerToothAngle = 50;
            triggerFilterTime = (curGap * 3) >> 2;
          }
        }
      }
      else if (configPage4.triggerFilter == 3)
      {

        if( (toothCurrentCount == 1) || (toothCurrentCount == 3) || (toothCurrentCount == 5) || (toothCurrentCount == 7) || (toothCurrentCount == 9) || (toothCurrentCount == 11) )
        {
          triggerToothAngle = 70;
          if(configPage2.nCylinders == 4)
          {
            triggerFilterTime = (curGap * 11) >> 3;
          }
          else
          {
            triggerFilterTime = curGap >> 1 ;
          }
        }
        else
        {
          if(configPage2.nCylinders == 4)
          {
            triggerToothAngle = 110;
            triggerFilterTime = (curGap * 9) >> 5;
          }
          else
          {
            triggerToothAngle = 50;
            triggerFilterTime = curGap;
          }
        }
      }
      else
      {

        triggerFilterTime = 0;
        if( (toothCurrentCount == 1) || (toothCurrentCount == 3) || (toothCurrentCount == 5) || (toothCurrentCount == 7) || (toothCurrentCount == 9) || (toothCurrentCount == 11) )
        {
          if(configPage2.nCylinders == 4) { triggerToothAngle = 70; }
          else { triggerToothAngle = 70; }
        }
        else
        {
          if(configPage2.nCylinders == 4) { triggerToothAngle = 110; }
          else { triggerToothAngle = 50; }
        }
      }



      if( (configPage2.perToothIgn == true) && (configPage4.triggerAngle == 0) )
      {
        if( (configPage2.nCylinders == 4) && (currentStatus.advance > 0) )
        {
          uint16_t crankAngle = ignitionLimits( toothAngles[(toothCurrentCount-1)] );


          if( (configPage4.sparkMode != IGN_MODE_SEQUENTIAL) && (toothCurrentCount > configPage2.nCylinders) ) { checkPerToothTiming(crankAngle, (toothCurrentCount-configPage2.nCylinders) ); }
          else { checkPerToothTiming(crankAngle, toothCurrentCount); }
        }
      }
    }
    else
    {
      triggerSecFilterTime = 0;

      if(READ_PRI_TRIGGER() == true)
      {
        if(READ_SEC_TRIGGER() == true) { revolutionOne = true; }
        else { revolutionOne = false; }
      }
      else
      {
        if( (READ_SEC_TRIGGER() == false) && (revolutionOne == true) )
        {

          if(configPage2.nCylinders == 4) { toothCurrentCount = 1; }

        }

        else if( (READ_SEC_TRIGGER() == true) && (revolutionOne == true) )
        {

          if(configPage2.nCylinders == 4) { toothCurrentCount = 5; }
          else if(configPage2.nCylinders == 6) { toothCurrentCount = 2; currentStatus.hasSync = true; }
        }
      }
    }
  }

}
void triggerSec_4G63()
{



  if(currentStatus.hasSync == true)
  {



  }


  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;
  if ( (curGap2 >= triggerSecFilterTime) )
  {
    toothLastSecToothTime = curTime2;
    validTrigger = true;


    triggerSecFilterTime = curGap2 >> 1;







    if( (currentStatus.hasSync == false) )
    {

      triggerFilterTime = 1500;
      triggerSecFilterTime = triggerSecFilterTime >> 1;
      if(READ_PRI_TRIGGER() == true)
      {
        if(configPage2.nCylinders == 4)
        {
          if(toothCurrentCount == 8) { currentStatus.hasSync = true; }
        }
        else if(configPage2.nCylinders == 6)
        {
          if(toothCurrentCount == 7) { currentStatus.hasSync = true; }
        }

      }
      else
      {
        if(configPage2.nCylinders == 4)
        {
          if(toothCurrentCount == 5) { currentStatus.hasSync = true; }
        }

      }
    }


    if ( (currentStatus.RPM < currentStatus.crankRPM) || (configPage4.useResync == 1) )
    {
      if( (currentStatus.hasSync == true) && (configPage2.nCylinders == 4) )
      {
        triggerSecFilterTime_duration = (micros() - secondaryLastToothTime1) >> 1;
        if(READ_PRI_TRIGGER() == true)
        {

          if(toothCurrentCount != 8)
          {

            currentStatus.hasSync = false;
            currentStatus.syncLossCounter++;
          }
          else { toothCurrentCount = 8; }
        }
      }
    }
  }
}


uint16_t getRPM_4G63()
{
  uint16_t tempRPM = 0;


  if(currentStatus.hasSync == true)
  {
    if( (currentStatus.RPM < currentStatus.crankRPM) )
    {
      int tempToothAngle;
      unsigned long toothTime;
      if( (toothLastToothTime == 0) || (toothLastMinusOneToothTime == 0) ) { tempRPM = 0; }
      else
      {
        noInterrupts();
        tempToothAngle = triggerToothAngle;
        toothTime = (toothLastToothTime - toothLastMinusOneToothTime);
        interrupts();
        toothTime = toothTime * 36;
        tempRPM = ((unsigned long)tempToothAngle * 6000000UL) / toothTime;
        revolutionTime = (10UL * toothTime) / tempToothAngle;
        MAX_STALL_TIME = 366667UL;
      }
    }
    else
    {
      tempRPM = stdGetRPM(720);


      MAX_STALL_TIME = revolutionTime << 1;
      if(MAX_STALL_TIME < 366667UL) { MAX_STALL_TIME = 366667UL; }
    }
  }

  return tempRPM;
}

int getCrankAngle_4G63()
{
    int crankAngle = 0;
    if(currentStatus.hasSync == true)
    {

      unsigned long tempToothLastToothTime;
      int tempToothCurrentCount;

      noInterrupts();
      tempToothCurrentCount = toothCurrentCount;
      tempToothLastToothTime = toothLastToothTime;
      lastCrankAngleCalc = micros();
      interrupts();

      crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage4.triggerAngle;


      elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
      crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_TOOTH);

      if (crankAngle >= 720) { crankAngle -= 720; }
      if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
      if (crankAngle < 0) { crankAngle += 360; }
    }
    return crankAngle;
}

void triggerSetEndTeeth_4G63()
{
  if(configPage2.nCylinders == 4)
  {
    if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
    {
      ignition1EndTooth = 8;
      ignition2EndTooth = 2;
      ignition3EndTooth = 4;
      ignition4EndTooth = 6;
    }
    else
    {
      ignition1EndTooth = 4;
      ignition2EndTooth = 2;
      ignition3EndTooth = 4;
      ignition4EndTooth = 2;
    }
  }
  if(configPage2.nCylinders == 6)
  {
    if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
    {

      ignition1EndTooth = 8;
      ignition2EndTooth = 2;
      ignition3EndTooth = 4;
      ignition4EndTooth = 6;
    }
    else
    {
      ignition1EndTooth = 6;
      ignition2EndTooth = 2;
      ignition3EndTooth = 4;
      ignition4EndTooth = 2;
    }
  }


  lastToothCalcAdvance = currentStatus.advance;
}
//# 1371 "/home/developper/speeduino/speeduino/decoders.ino"
void triggerSetup_24X()
{
  triggerToothAngle = 15;
  toothAngles[0] = 12;
  toothAngles[1] = 18;
  toothAngles[2] = 33;
  toothAngles[3] = 48;
  toothAngles[4] = 63;
  toothAngles[5] = 78;
  toothAngles[6] = 102;
  toothAngles[7] = 108;
  toothAngles[8] = 123;
  toothAngles[9] = 138;
  toothAngles[10] = 162;
  toothAngles[11] = 177;
  toothAngles[12] = 183;
  toothAngles[13] = 198;
  toothAngles[14] = 222;
  toothAngles[15] = 237;
  toothAngles[16] = 252;
  toothAngles[17] = 258;
  toothAngles[18] = 282;
  toothAngles[19] = 288;
  toothAngles[20] = 312;
  toothAngles[21] = 327;
  toothAngles[22] = 342;
  toothAngles[23] = 357;

  MAX_STALL_TIME = (3333UL * triggerToothAngle);
  if(initialisationComplete == false) { toothCurrentCount = 25; toothLastToothTime = micros(); }
  secondDerivEnabled = false;
  decoderIsSequential = true;
  triggerToothAngleIsCorrect = true;
}

void triggerPri_24X()
{
  if(toothCurrentCount == 25) { currentStatus.hasSync = false; }
  else
  {
    curTime = micros();
    curGap = curTime - toothLastToothTime;

    if(toothCurrentCount == 0)
    {
       toothCurrentCount = 1;
       toothOneMinusOneTime = toothOneTime;
       toothOneTime = curTime;
       revolutionOne = !revolutionOne;
       currentStatus.hasSync = true;
       currentStatus.startRevolutions++;
       triggerToothAngle = 15;
    }
    else
    {
      toothCurrentCount++;
      triggerToothAngle = toothAngles[(toothCurrentCount-1)] - toothAngles[(toothCurrentCount-2)];
    }

    validTrigger = true;

    toothLastToothTime = curTime;


  }
}

void triggerSec_24X()
{
  toothCurrentCount = 0;
  revolutionOne = 1;
}

uint16_t getRPM_24X()
{
   return stdGetRPM(360);
}
int getCrankAngle_24X()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount, tempRevolutionOne;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    tempRevolutionOne = revolutionOne;
    lastCrankAngleCalc = micros();
    interrupts();

    int crankAngle;
    if (tempToothCurrentCount == 0) { crankAngle = 0 + configPage4.triggerAngle; }
    else { crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage4.triggerAngle;}


    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);


    if (tempRevolutionOne == 1) { crankAngle += 360; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

void triggerSetEndTeeth_24X()
{


  lastToothCalcAdvance = currentStatus.advance;
}
//# 1493 "/home/developper/speeduino/speeduino/decoders.ino"
void triggerSetup_Jeep2000()
{
  triggerToothAngle = 0;
  toothAngles[0] = 174;
  toothAngles[1] = 194;
  toothAngles[2] = 214;
  toothAngles[3] = 234;
  toothAngles[4] = 294;
  toothAngles[5] = 314;
  toothAngles[6] = 334;
  toothAngles[7] = 354;
  toothAngles[8] = 414;
  toothAngles[9] = 434;
  toothAngles[10] = 454;
  toothAngles[11] = 474;

  MAX_STALL_TIME = (3333UL * 60);
  if(initialisationComplete == false) { toothCurrentCount = 13; toothLastToothTime = micros(); }
  secondDerivEnabled = false;
  decoderIsSequential = false;
  triggerToothAngleIsCorrect = true;
}

void triggerPri_Jeep2000()
{
  if(toothCurrentCount == 13) { currentStatus.hasSync = false; }
  else
  {
    curTime = micros();
    curGap = curTime - toothLastToothTime;
    if ( curGap >= triggerFilterTime )
    {
      if(toothCurrentCount == 0)
      {
         toothCurrentCount = 1;
         toothOneMinusOneTime = toothOneTime;
         toothOneTime = curTime;
         currentStatus.hasSync = true;
         currentStatus.startRevolutions++;
         triggerToothAngle = 60;
      }
      else
      {
        toothCurrentCount++;
        triggerToothAngle = toothAngles[(toothCurrentCount-1)] - toothAngles[(toothCurrentCount-2)];
      }

      setFilter(curGap);

      validTrigger = true;

      toothLastMinusOneToothTime = toothLastToothTime;
      toothLastToothTime = curTime;
    }
  }
}
void triggerSec_Jeep2000()
{
  toothCurrentCount = 0;
  return;
}

uint16_t getRPM_Jeep2000()
{
   return stdGetRPM(360);
}
int getCrankAngle_Jeep2000()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    lastCrankAngleCalc = micros();
    interrupts();

    int crankAngle;
    if (toothCurrentCount == 0) { crankAngle = 146 + configPage4.triggerAngle; }
    else { crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage4.triggerAngle;}


    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

void triggerSetEndTeeth_Jeep2000()
{

  lastToothCalcAdvance = currentStatus.advance;
}






void triggerSetup_Audi135()
{
  triggerToothAngle = 8;
  toothCurrentCount = 255;
  toothSystemCount = 0;
  triggerFilterTime = (unsigned long)(1000000 / (MAX_RPM / 60 * 135UL));
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2;
  MAX_STALL_TIME = (3333UL * triggerToothAngle);
  secondDerivEnabled = false;
  decoderIsSequential = true;
  triggerToothAngleIsCorrect = true;
}

void triggerPri_Audi135()
{
   curTime = micros();
   curGap = curTime - toothSystemLastToothTime;
   if ( (curGap > triggerFilterTime) || (currentStatus.startRevolutions == 0) )
   {
     toothSystemCount++;

     if ( currentStatus.hasSync == false ) { toothLastToothTime = curTime; }
     else
     {
       if ( toothSystemCount >= 3 )
       {


         validTrigger = true;
         toothSystemLastToothTime = curTime;
         toothSystemCount = 0;
         toothCurrentCount++;

         if ( (toothCurrentCount == 1) || (toothCurrentCount > 45) )
         {
           toothCurrentCount = 1;
           toothOneMinusOneTime = toothOneTime;
           toothOneTime = curTime;
           revolutionOne = !revolutionOne;
           currentStatus.startRevolutions++;
         }

         setFilter(curGap);

         toothLastMinusOneToothTime = toothLastToothTime;
         toothLastToothTime = curTime;
       }
     }
   }
}

void triggerSec_Audi135()
{







  if( currentStatus.hasSync == false )
  {
    toothCurrentCount = 0;
    currentStatus.hasSync = true;
    toothSystemCount = 3;
  }
  else if (configPage4.useResync == 1) { toothCurrentCount = 0; toothSystemCount = 3; }
  else if ( (currentStatus.startRevolutions < 100) && (toothCurrentCount != 45) ) { toothCurrentCount = 0; }
  revolutionOne = 1;
}

uint16_t getRPM_Audi135()
{
   return stdGetRPM(360);
}

int getCrankAngle_Audi135()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    bool tempRevolutionOne;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    tempRevolutionOne = revolutionOne;
    lastCrankAngleCalc = micros();
    interrupts();


    if(tempToothCurrentCount == 0) { tempToothCurrentCount = 45; }

    int crankAngle = ((tempToothCurrentCount - 1) * triggerToothAngle) + configPage4.triggerAngle;


    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);


    if (tempRevolutionOne) { crankAngle += 360; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    else if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += CRANK_ANGLE_MAX; }

    return crankAngle;
}

void triggerSetEndTeeth_Audi135()
{
  lastToothCalcAdvance = currentStatus.advance;
}






void triggerSetup_HondaD17()
{
  triggerToothAngle = 360 / 12;
  MAX_STALL_TIME = (3333UL * triggerToothAngle);
  secondDerivEnabled = false;
  decoderIsSequential = false;
}

void triggerPri_HondaD17()
{
   lastGap = curGap;
   curTime = micros();
   curGap = curTime - toothLastToothTime;
   toothCurrentCount++;

   validTrigger = true;


   if( (toothCurrentCount == 13) && (currentStatus.hasSync == true) )
   {
     toothCurrentCount = 0;
   }
   else if( (toothCurrentCount == 1) && (currentStatus.hasSync == true) )
   {
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.startRevolutions++;

     toothLastMinusOneToothTime = toothLastToothTime;
     toothLastToothTime = curTime;
   }
   else
   {

     targetGap = (lastGap) >> 1;
     if ( curGap < targetGap)
     {
       toothCurrentCount = 0;
       currentStatus.hasSync = true;
     }
     else
     {

       toothLastMinusOneToothTime = toothLastToothTime;
       toothLastToothTime = curTime;
     }
   }

}
void triggerSec_HondaD17() { return; }
uint16_t getRPM_HondaD17()
{
   return stdGetRPM(360);
}
int getCrankAngle_HondaD17()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    lastCrankAngleCalc = micros();
    interrupts();


    int crankAngle;
    if( tempToothCurrentCount == 0 )
    {
      crankAngle = (11 * triggerToothAngle) + configPage4.triggerAngle;
    }
    else
    {
      crankAngle = ((tempToothCurrentCount - 1) * triggerToothAngle) + configPage4.triggerAngle;
    }


    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

void triggerSetEndTeeth_HondaD17()
{
  lastToothCalcAdvance = currentStatus.advance;
}
//# 1816 "/home/developper/speeduino/speeduino/decoders.ino"
void triggerSetup_Miata9905()
{
  triggerToothAngle = 90;
  toothCurrentCount = 99;
  secondDerivEnabled = false;
  decoderIsSequential = true;
  triggerActualTeeth = 8;

  if(initialisationComplete == false) { secondaryToothCount = 0; toothLastToothTime = micros(); }
  else { toothLastToothTime = 0; }
  toothLastMinusOneToothTime = 0;
//# 1837 "/home/developper/speeduino/speeduino/decoders.ino"
  toothAngles[0] = 710;
  toothAngles[1] = 100;
  toothAngles[2] = 170;
  toothAngles[3] = 280;
  toothAngles[4] = 350;
  toothAngles[5] = 460;
  toothAngles[6] = 530;
  toothAngles[7] = 640;

  MAX_STALL_TIME = (3333UL * triggerToothAngle);
  triggerFilterTime = 1500;
  triggerSecFilterTime = 0;
  decoderHasFixedCrankingTiming = true;
  triggerToothAngleIsCorrect = true;
}

void triggerPri_Miata9905()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( (curGap >= triggerFilterTime) || (currentStatus.startRevolutions == 0) )
  {
    toothCurrentCount++;
    validTrigger = true;
    if( (toothCurrentCount == (triggerActualTeeth + 1)) )
    {
       toothCurrentCount = 1;
       toothOneMinusOneTime = toothOneTime;
       toothOneTime = curTime;

       currentStatus.startRevolutions++;
    }
    else
    {
      if( (currentStatus.hasSync == false) || (configPage4.useResync == true) )
      {
        if(secondaryToothCount == 2)
        {
          toothCurrentCount = 6;
          currentStatus.hasSync = true;
        }
      }
    }

    if (currentStatus.hasSync == true)
    {


      if( (configPage4.triggerFilter == 1) || (currentStatus.RPM < 1400) )
      {

        if( (toothCurrentCount == 1) || (toothCurrentCount == 3) || (toothCurrentCount == 5) || (toothCurrentCount == 7) ) { triggerToothAngle = 70; triggerFilterTime = curGap; }
        else { triggerToothAngle = 110; triggerFilterTime = (curGap * 3) >> 3; }
      }
      else if(configPage4.triggerFilter == 2)
      {

        if( (toothCurrentCount == 1) || (toothCurrentCount == 3) || (toothCurrentCount == 5) || (toothCurrentCount == 7) ) { triggerToothAngle = 70; triggerFilterTime = (curGap * 5) >> 2 ; }
        else { triggerToothAngle = 110; triggerFilterTime = (curGap >> 1); }
      }
      else if (configPage4.triggerFilter == 3)
      {

        if( (toothCurrentCount == 1) || (toothCurrentCount == 3) || (toothCurrentCount == 5) || (toothCurrentCount == 7) ) { triggerToothAngle = 70; triggerFilterTime = (curGap * 11) >> 3 ; }
        else { triggerToothAngle = 110; triggerFilterTime = (curGap * 9) >> 5; }
      }
      else if (configPage4.triggerFilter == 0)
      {

        triggerFilterTime = 0;
        triggerSecFilterTime = 0;
        if( (toothCurrentCount == 1) || (toothCurrentCount == 3) || (toothCurrentCount == 5) || (toothCurrentCount == 7) ) { triggerToothAngle = 70; }
        else { triggerToothAngle = 110; }
      }



      if( (configPage2.perToothIgn == true) || (configPage4.triggerAngle == 0) )
      {
        if (currentStatus.advance > 0)
        {
          uint16_t crankAngle = ignitionLimits( toothAngles[(toothCurrentCount-1)] );


          if( (configPage4.sparkMode != IGN_MODE_SEQUENTIAL) && (toothCurrentCount > configPage2.nCylinders) ) { checkPerToothTiming(crankAngle, (toothCurrentCount-configPage2.nCylinders) ); }
          else { checkPerToothTiming(crankAngle, toothCurrentCount); }
        }
      }

    }

    toothLastMinusOneToothTime = toothLastToothTime;
    toothLastToothTime = curTime;


    if ( (currentStatus.RPM < (currentStatus.crankRPM + 30)) && (configPage4.ignCranklock) )
    {
      if( (toothCurrentCount == 1) || (toothCurrentCount == 5) ) { endCoil1Charge(); endCoil3Charge(); }
      else if( (toothCurrentCount == 3) || (toothCurrentCount == 7) ) { endCoil2Charge(); endCoil4Charge(); }
    }
    secondaryToothCount = 0;
  }

}

void triggerSec_Miata9905()
{
  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;

  if(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) || (currentStatus.hasSync == false) )
  {
    triggerFilterTime = 1500;
  }

  if ( curGap2 >= triggerSecFilterTime )
  {
    toothLastSecToothTime = curTime2;
    lastGap = curGap2;
    secondaryToothCount++;


  }
}

uint16_t getRPM_Miata9905()
{


  uint16_t tempRPM = 0;
  if( (currentStatus.RPM < currentStatus.crankRPM) && (currentStatus.hasSync == true) )
  {
    if( (toothLastToothTime == 0) || (toothLastMinusOneToothTime == 0) ) { tempRPM = 0; }
    else
    {
      int tempToothAngle;
      unsigned long toothTime;
      noInterrupts();
      tempToothAngle = triggerToothAngle;
      toothTime = (toothLastToothTime - toothLastMinusOneToothTime);
      interrupts();
      toothTime = toothTime * 36;
      tempRPM = ((unsigned long)tempToothAngle * 6000000UL) / toothTime;
      revolutionTime = (10UL * toothTime) / tempToothAngle;
      MAX_STALL_TIME = 366667UL;
    }
  }
  else
  {
    tempRPM = stdGetRPM(720);
    MAX_STALL_TIME = revolutionTime << 1;
    if(MAX_STALL_TIME < 366667UL) { MAX_STALL_TIME = 366667UL; }
  }

  return tempRPM;
}

int getCrankAngle_Miata9905()
{
    int crankAngle = 0;

    {

      unsigned long tempToothLastToothTime;
      int tempToothCurrentCount;

      noInterrupts();
      tempToothCurrentCount = toothCurrentCount;
      tempToothLastToothTime = toothLastToothTime;
      lastCrankAngleCalc = micros();
      interrupts();

      crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage4.triggerAngle;


      elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
      crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);

      if (crankAngle >= 720) { crankAngle -= 720; }
      if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
      if (crankAngle < 0) { crankAngle += 360; }
    }

    return crankAngle;
}

void triggerSetEndTeeth_Miata9905()
{

  if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
  {
    if(currentStatus.advance >= 10)
    {
      ignition1EndTooth = 8;
      ignition2EndTooth = 2;
      ignition3EndTooth = 4;
      ignition4EndTooth = 6;
    }
    else if (currentStatus.advance > 0)
    {
      ignition1EndTooth = 1;
      ignition2EndTooth = 3;
      ignition3EndTooth = 5;
      ignition4EndTooth = 7;
    }

  }
  else
  {
    if(currentStatus.advance >= 10)
    {
      ignition1EndTooth = 4;
      ignition2EndTooth = 2;
      ignition3EndTooth = 4;
      ignition4EndTooth = 2;
    }
    else if(currentStatus.advance > 0)
    {
      ignition1EndTooth = 1;
      ignition2EndTooth = 3;
      ignition3EndTooth = 1;
      ignition4EndTooth = 3;
    }
  }

  lastToothCalcAdvance = currentStatus.advance;
}
//# 2072 "/home/developper/speeduino/speeduino/decoders.ino"
void triggerSetup_MazdaAU()
{
  triggerToothAngle = 108;
  toothCurrentCount = 99;
  secondaryToothCount = 0;
  secondDerivEnabled = false;
  decoderIsSequential = true;

  toothAngles[0] = 348;
  toothAngles[1] = 96;
  toothAngles[2] = 168;
  toothAngles[3] = 276;

  MAX_STALL_TIME = (3333UL * triggerToothAngle);
  triggerFilterTime = 1500;
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2;
  decoderHasFixedCrankingTiming = true;
}

void triggerPri_MazdaAU()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( curGap >= triggerFilterTime )
  {
    validTrigger = true;

    toothCurrentCount++;
    if( (toothCurrentCount == 1) || (toothCurrentCount == 5) )
    {
       toothCurrentCount = 1;
       toothOneMinusOneTime = toothOneTime;
       toothOneTime = curTime;
       currentStatus.hasSync = true;
       currentStatus.startRevolutions++;
    }

    if (currentStatus.hasSync == true)
    {

      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && configPage4.ignCranklock )
      {
        if( toothCurrentCount == 1 ) { endCoil1Charge(); }
        else if( toothCurrentCount == 3 ) { endCoil2Charge(); }
      }


      if( (toothCurrentCount == 1) || (toothCurrentCount == 3) ) { triggerToothAngle = 72; triggerFilterTime = curGap; }
      else { triggerToothAngle = 108; triggerFilterTime = (curGap * 3) >> 3; }

      toothLastMinusOneToothTime = toothLastToothTime;
      toothLastToothTime = curTime;
    }
  }
}

void triggerSec_MazdaAU()
{
  curTime2 = micros();
  lastGap = curGap2;
  curGap2 = curTime2 - toothLastSecToothTime;

  toothLastSecToothTime = curTime2;


  if(currentStatus.hasSync == false)
  {


    if(secondaryToothCount == 2)
    {
      toothCurrentCount = 1;
      currentStatus.hasSync = true;
    }
    else
    {
      triggerFilterTime = 1500;
      targetGap = (lastGap) >> 1;
      if ( curGap2 < targetGap)
      {
        secondaryToothCount = 2;
      }
    }
    secondaryToothCount++;
  }
}


uint16_t getRPM_MazdaAU()
{
  uint16_t tempRPM = 0;

  if (currentStatus.hasSync == true)
  {


    if(currentStatus.RPM < currentStatus.crankRPM)
    {
      int tempToothAngle;
      noInterrupts();
      tempToothAngle = triggerToothAngle;
      revolutionTime = (toothLastToothTime - toothLastMinusOneToothTime);
      interrupts();
      revolutionTime = revolutionTime * 36;
      tempRPM = (tempToothAngle * 60000000L) / revolutionTime;
    }
    else { tempRPM = stdGetRPM(360); }
  }
  return tempRPM;
}

int getCrankAngle_MazdaAU()
{
    int crankAngle = 0;
    if(currentStatus.hasSync == true)
    {

      unsigned long tempToothLastToothTime;
      int tempToothCurrentCount;

      noInterrupts();
      tempToothCurrentCount = toothCurrentCount;
      tempToothLastToothTime = toothLastToothTime;
      lastCrankAngleCalc = micros();
      interrupts();

      crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage4.triggerAngle;


      elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
      crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);

      if (crankAngle >= 720) { crankAngle -= 720; }
      if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
      if (crankAngle < 0) { crankAngle += 360; }
    }

    return crankAngle;
}

void triggerSetEndTeeth_MazdaAU()
{
  lastToothCalcAdvance = currentStatus.advance;
}






void triggerSetup_non360()
{
  triggerToothAngle = (360 * configPage4.TrigAngMul) / configPage4.triggerTeeth;
  toothCurrentCount = 255;
  triggerFilterTime = (int)(1000000 / (MAX_RPM / 60 * configPage4.triggerTeeth));
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2;
  secondDerivEnabled = false;
  decoderIsSequential = true;
  MAX_STALL_TIME = (3333UL * triggerToothAngle);
}


void triggerPri_non360()
{

}

void triggerSec_non360()
{

}

uint16_t getRPM_non360()
{
  uint16_t tempRPM = 0;
  if( (currentStatus.hasSync == true) && (toothCurrentCount != 0) )
  {
    if(currentStatus.RPM < currentStatus.crankRPM) { tempRPM = crankingGetRPM(configPage4.triggerTeeth); }
    else { tempRPM = stdGetRPM(360); }
  }
  return tempRPM;
}

int getCrankAngle_non360()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    lastCrankAngleCalc = micros();
    interrupts();


    if(tempToothCurrentCount == 0) { tempToothCurrentCount = configPage4.triggerTeeth; }


    int crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle;
    crankAngle = (crankAngle / configPage4.TrigAngMul) + configPage4.triggerAngle;


    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

void triggerSetEndTeeth_non360()
{
  lastToothCalcAdvance = currentStatus.advance;
}






void triggerSetup_Nissan360()
{
  triggerFilterTime = (1000000 / (MAX_RPM / 60 * 360UL));
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2;
  secondaryToothCount = 0;
  secondDerivEnabled = false;
  decoderIsSequential = true;
  toothCurrentCount = 1;
  triggerToothAngle = 2;
  MAX_STALL_TIME = (3333UL * triggerToothAngle);
}


void triggerPri_Nissan360()
{
   curTime = micros();
   curGap = curTime - toothLastToothTime;

   toothCurrentCount++;
   validTrigger = true;

   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;

   if ( currentStatus.hasSync == true )
   {
     if ( toothCurrentCount == 361 )
     {
       toothCurrentCount = 1;
       toothOneMinusOneTime = toothOneTime;
       toothOneTime = curTime;
       currentStatus.startRevolutions++;
     }




     if(configPage2.perToothIgn == true)
     {
        int16_t crankAngle = ( (toothCurrentCount-1) * 2 ) + configPage4.triggerAngle;
        if(crankAngle > CRANK_ANGLE_MAX_IGN)
        {
          crankAngle -= CRANK_ANGLE_MAX_IGN;
          checkPerToothTiming(crankAngle, (toothCurrentCount/2) );
        }
        else
        {
          checkPerToothTiming(crankAngle, toothCurrentCount);
        }

     }

     timePerDegree = curGap >> 1;;
   }
}

void triggerSec_Nissan360()
{
  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;

  toothLastSecToothTime = curTime2;





  byte trigEdge;
  if(configPage4.TrigEdgeSec == 0) { trigEdge = LOW; }
  else { trigEdge = HIGH; }

  if( (secondaryToothCount == 0) || (READ_SEC_TRIGGER() == trigEdge) ) { secondaryToothCount = toothCurrentCount; }
  else
  {

    byte secondaryDuration = toothCurrentCount - secondaryToothCount;

    if(currentStatus.hasSync == false)
    {
      if(configPage2.nCylinders == 4)
      {


        if( (secondaryDuration >= 15) && (secondaryDuration <= 17) )
        {
          toothCurrentCount = 16;
          currentStatus.hasSync = true;
        }
        else if( (secondaryDuration >= 11) && (secondaryDuration <= 13) )
        {
          toothCurrentCount = 102;
          currentStatus.hasSync = true;
        }
        else if( (secondaryDuration >= 7) && (secondaryDuration <= 9) )
        {
          toothCurrentCount = 188;
          currentStatus.hasSync = true;
        }
        else if( (secondaryDuration >= 3) && (secondaryDuration <= 5) )
        {
          toothCurrentCount = 274;
          currentStatus.hasSync = true;
        }
        else { currentStatus.hasSync = false; currentStatus.syncLossCounter++; }
      }
      else if(configPage2.nCylinders == 6)
      {

        if( (secondaryDuration >= 3) && (secondaryDuration <= 5) )
        {
          toothCurrentCount = 124;
          currentStatus.hasSync = true;
        }
      }
      else if(configPage2.nCylinders == 8)
      {


        if( (secondaryDuration >= 6) && (secondaryDuration <= 8) )
        {
          toothCurrentCount = 56;
          currentStatus.hasSync = true;
        }
      }
      else { currentStatus.hasSync = false; }
    }
    else
    {
      if (configPage4.useResync == true)
      {

        if(configPage2.nCylinders == 4)
        {
          if( (secondaryDuration >= 15) && (secondaryDuration <= 17) )
          {
            toothCurrentCount = 16;
          }
        }
        else if(configPage2.nCylinders == 6)
        {
          if(secondaryDuration == 4)
          {

          }
        }
      }
    }
  }
}

uint16_t getRPM_Nissan360()
{

  uint16_t tempRPM;
  if( (currentStatus.hasSync == true) && (toothLastToothTime != 0) && (toothLastMinusOneToothTime != 0) )
  {
    if(currentStatus.startRevolutions < 2)
    {
      noInterrupts();
      revolutionTime = (toothLastToothTime - toothLastMinusOneToothTime) * 180;
      interrupts();
    }
    else
    {
      noInterrupts();
      revolutionTime = (toothOneTime - toothOneMinusOneTime) >> 1;
      interrupts();
    }
    tempRPM = (US_IN_MINUTE / revolutionTime);
    if(tempRPM >= MAX_RPM) { tempRPM = currentStatus.RPM; }
    MAX_STALL_TIME = revolutionTime << 1;
  }
  else { tempRPM = 0; }

  return tempRPM;
}

int getCrankAngle_Nissan360()
{

  int crankAngle = 0;
  int tempToothLastToothTime;
  int tempToothLastMinusOneToothTime;
  int tempToothCurrentCount;

  noInterrupts();
  tempToothLastToothTime = toothLastToothTime;
  tempToothLastMinusOneToothTime = toothLastMinusOneToothTime;
  tempToothCurrentCount = toothCurrentCount;
  lastCrankAngleCalc = micros();
  interrupts();

  crankAngle = ( (tempToothCurrentCount - 1) * 2) + configPage4.triggerAngle;
  unsigned long halfTooth = (tempToothLastToothTime - tempToothLastMinusOneToothTime) >> 1;
  if (elapsedTime > halfTooth)
  {

    crankAngle += 1;
  }

  if (crankAngle >= 720) { crankAngle -= 720; }
  if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
  if (crankAngle < 0) { crankAngle += 360; }

  return crankAngle;
}

void triggerSetEndTeeth_Nissan360()
{

  ignition1EndTooth = ( (ignition1EndAngle - configPage4.triggerAngle) / 2 ) - 4;
  ignition2EndTooth = ( (ignition2EndAngle - configPage4.triggerAngle) / 2 ) - 4;
  ignition3EndTooth = ( (ignition3EndAngle - configPage4.triggerAngle) / 2 ) - 4;
  ignition4EndTooth = ( (ignition4EndAngle - configPage4.triggerAngle) / 2 ) - 4;

  if(ignition1EndTooth < 0) { ignition1EndTooth += 360; }
  if(ignition2EndTooth < 0) { ignition2EndTooth += 360; }
  if(ignition3EndTooth < 0) { ignition3EndTooth += 360; }
  if(ignition4EndTooth < 0) { ignition4EndTooth += 360; }

  lastToothCalcAdvance = currentStatus.advance;
}






void triggerSetup_Subaru67()
{
  triggerFilterTime = (1000000 / (MAX_RPM / 60 * 360UL));
  triggerSecFilterTime = 0;
  secondaryToothCount = 0;
  secondDerivEnabled = false;
  decoderIsSequential = true;
  toothCurrentCount = 1;
  triggerToothAngle = 2;
  triggerToothAngleIsCorrect = false;
  toothSystemCount = 0;
  MAX_STALL_TIME = (3333UL * 93);

  toothAngles[0] = 710;
  toothAngles[1] = 83;
  toothAngles[2] = 115;
  toothAngles[3] = 170;
  toothAngles[4] = toothAngles[1] + 180;
  toothAngles[5] = toothAngles[2] + 180;
  toothAngles[6] = toothAngles[3] + 180;
  toothAngles[7] = toothAngles[1] + 360;
  toothAngles[8] = toothAngles[2] + 360;
  toothAngles[9] = toothAngles[3] + 360;
  toothAngles[10] = toothAngles[1] + 540;
  toothAngles[11] = toothAngles[2] + 540;
}


void triggerPri_Subaru67()
{
   curTime = micros();


   toothCurrentCount++;
   toothSystemCount++;
   validTrigger = true;

   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;

   if ( (currentStatus.hasSync == false) || (configPage4.useResync == true) )
   {
     if(toothCurrentCount > 12) { toothCurrentCount = toothCurrentCount % 12; }


     switch(secondaryToothCount)
     {
        case 0:

          break;

        case 1:

          secondaryToothCount = 0;
          break;

        case 2:
          toothCurrentCount = 8;

          secondaryToothCount = 0;
          break;

        case 3:

          if( toothCurrentCount == 2)
          {
            currentStatus.hasSync = true;
          }
          secondaryToothCount = 0;
          break;

        default:

          currentStatus.hasSync = false;
          triggerToothAngleIsCorrect = false;
          currentStatus.syncLossCounter++;
          secondaryToothCount = 0;
          break;

     }
   }


   if ( currentStatus.hasSync == true )
   {

      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && configPage4.ignCranklock)
      {
        if( (toothCurrentCount == 1) || (toothCurrentCount == 7) ) { endCoil1Charge(); endCoil3Charge(); }
        else if( (toothCurrentCount == 4) || (toothCurrentCount == 10) ) { endCoil2Charge(); endCoil4Charge(); }
      }

      if ( toothCurrentCount > 12 )
      {
        toothCurrentCount = 1;
        toothOneMinusOneTime = toothOneTime;
        toothOneTime = curTime;
        currentStatus.startRevolutions++;
      }


      if(toothCurrentCount == 1) { triggerToothAngle = 55; }
      else if(toothCurrentCount == 2) { triggerToothAngle = 93; }
      else { triggerToothAngle = toothAngles[(toothCurrentCount-1)] - toothAngles[(toothCurrentCount-2)]; }
      triggerToothAngleIsCorrect = true;



      if( (configPage2.perToothIgn == true) && (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) )
      {
        int16_t crankAngle = toothAngles[(toothCurrentCount - 1)] + configPage4.triggerAngle;
        if( (configPage4.sparkMode != IGN_MODE_SEQUENTIAL) )
        {
          crankAngle = ignitionLimits( toothAngles[(toothCurrentCount-1)] );


          if( (configPage4.sparkMode != IGN_MODE_SEQUENTIAL) && (toothCurrentCount > 6) ) { checkPerToothTiming(crankAngle, (toothCurrentCount-6) ); }
          else { checkPerToothTiming(crankAngle, toothCurrentCount); }
        }
        else{ checkPerToothTiming(crankAngle, toothCurrentCount); }
      }


   }
 }

void triggerSec_Subaru67()
{
  if( (toothSystemCount == 0) || (toothSystemCount == 3) )
  {
    curTime2 = micros();
    curGap2 = curTime2 - toothLastSecToothTime;

    if ( curGap2 > triggerSecFilterTime )
    {
      toothLastSecToothTime = curTime2;
      secondaryToothCount++;
      toothSystemCount = 0;

      if(secondaryToothCount > 1)
      {


        triggerSecFilterTime = curGap2 >> 2;
      }
      else { triggerSecFilterTime = 0; }

    }
  }
  else
  {

    if(toothSystemCount > 3)
    {
      toothSystemCount = 0;
      secondaryToothCount = 1;
    }
  }

}

uint16_t getRPM_Subaru67()
{


  uint16_t tempRPM = 0;
  if(currentStatus.startRevolutions > 0)
  {

    tempRPM = stdGetRPM(720);
  }
  return tempRPM;
}

int getCrankAngle_Subaru67()
{
  int crankAngle = 0;
  if( currentStatus.hasSync == true )
  {

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    lastCrankAngleCalc = micros();
    interrupts();

    crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage4.triggerAngle;


    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_TOOTH);

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }
  }

  return crankAngle;
}

void triggerSetEndTeeth_Subaru67()
{
  if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
  {

    if(currentStatus.advance >= 10 )
    {
      ignition1EndTooth = 12;
      ignition2EndTooth = 3;
      ignition3EndTooth = 6;
      ignition4EndTooth = 9;
    }
    else
    {
      ignition1EndTooth = 1;
      ignition2EndTooth = 4;
      ignition3EndTooth = 7;
      ignition4EndTooth = 10;
    }
  }
  else
  {
    if(currentStatus.advance >= 10 )
    {
      ignition1EndTooth = 6;
      ignition2EndTooth = 3;


    }
    else
    {
      ignition1EndTooth = 1;
      ignition2EndTooth = 4;


    }
  }

  lastToothCalcAdvance = currentStatus.advance;
}






void triggerSetup_Daihatsu()
{
  triggerActualTeeth = configPage2.nCylinders + 1;
  triggerToothAngle = 720 / triggerActualTeeth;
  triggerFilterTime = 60000000L / MAX_RPM / configPage2.nCylinders;
  triggerFilterTime = triggerFilterTime / 2;
  secondDerivEnabled = false;
  decoderIsSequential = false;

  MAX_STALL_TIME = (1851UL * triggerToothAngle)*4;

  if(configPage2.nCylinders == 3)
  {
    toothAngles[0] = 0;
    toothAngles[1] = 30;
    toothAngles[2] = 240;
    toothAngles[3] = 480;
  }
  else
  {

    toothAngles[0] = 0;
    toothAngles[1] = 30;
    toothAngles[2] = 180;
    toothAngles[3] = 360;
    toothAngles[4] = 540;
  }
}

void triggerPri_Daihatsu()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;


  {
    toothSystemCount++;
    validTrigger = true;

    if (currentStatus.hasSync == true)
    {
      if( (toothCurrentCount == triggerActualTeeth) )
      {
         toothCurrentCount = 1;
         toothOneMinusOneTime = toothOneTime;
         toothOneTime = curTime;
         currentStatus.hasSync = true;
         currentStatus.startRevolutions++;


         triggerFilterTime = 20;
      }
      else
      {
        toothCurrentCount++;
        setFilter(curGap);
      }

      if ( configPage4.ignCranklock && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {

        if(toothCurrentCount == 1) { endCoil1Charge(); }
        else if(toothCurrentCount == 2) { endCoil2Charge(); }
        else if(toothCurrentCount == 3) { endCoil3Charge(); }
        else if(toothCurrentCount == 4) { endCoil4Charge(); }
      }
    }
    else
    {

      if(toothSystemCount >= 3)
      {
        unsigned long targetTime;


        if(configPage2.nCylinders == 3)
        {
          targetTime = (toothLastToothTime - toothLastMinusOneToothTime) / 4;
        }
        else
        {
          targetTime = ((toothLastToothTime - toothLastMinusOneToothTime) * 3) / 8;
        }
        if(curGap < targetTime)
        {

          toothCurrentCount = 2;
          currentStatus.hasSync = true;
          triggerFilterTime = targetTime;
        }
      }
    }

    toothLastMinusOneToothTime = toothLastToothTime;
    toothLastToothTime = curTime;
  }
}
void triggerSec_Daihatsu() { return; }

uint16_t getRPM_Daihatsu()
{
  uint16_t tempRPM = 0;
  if( (currentStatus.RPM < currentStatus.crankRPM) && false)
  {

    if( currentStatus.hasSync == true )
    {
      if(toothCurrentCount == 2) { tempRPM = currentStatus.RPM; }
      else if (toothCurrentCount == 3) { tempRPM = currentStatus.RPM; }
      else
      {
        noInterrupts();
        revolutionTime = (toothLastToothTime - toothLastMinusOneToothTime) * (triggerActualTeeth-1);
        interrupts();
        tempRPM = (US_IN_MINUTE / revolutionTime);
        if(tempRPM >= MAX_RPM) { tempRPM = currentStatus.RPM; }
      }
    }
    else { tempRPM = 0; }
  }
  else
  { tempRPM = stdGetRPM(720); }

  return tempRPM;

}
int getCrankAngle_Daihatsu()
{

    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    int crankAngle;

    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    lastCrankAngleCalc = micros();
    interrupts();

    crankAngle = toothAngles[tempToothCurrentCount-1] + configPage4.triggerAngle;


    elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
    crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += CRANK_ANGLE_MAX; }

    return crankAngle;
}

void triggerSetEndTeeth_Daihatsu()
{
  lastToothCalcAdvance = currentStatus.advance;
}
//# 2937 "/home/developper/speeduino/speeduino/decoders.ino"
void triggerSetup_Harley()
{
  triggerToothAngle = 0;
  secondDerivEnabled = false;
  decoderIsSequential = false;
  MAX_STALL_TIME = (3333UL * 60);
  if(initialisationComplete == false) { toothLastToothTime = micros(); }
  triggerFilterTime = 1500;
}

void triggerPri_Harley()
{
  lastGap = curGap;
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  setFilter(curGap);
  if (curGap > triggerFilterTime)
  {
    if ( READ_PRI_TRIGGER() == HIGH)
    {
        validTrigger = true;
        targetGap = lastGap ;
        toothCurrentCount++;
        if (curGap > targetGap)
        {
          toothCurrentCount = 1;
          triggerToothAngle = 0;
          toothOneMinusOneTime = toothOneTime;
          toothOneTime = curTime;
          currentStatus.hasSync = true;
        }
        else
        {
          toothCurrentCount = 2;
          triggerToothAngle = 157;


        }
        toothLastMinusOneToothTime = toothLastToothTime;
        toothLastToothTime = curTime;
        currentStatus.startRevolutions++;
    }
    else
    {
      if (currentStatus.hasSync == true) { currentStatus.syncLossCounter++; }
      currentStatus.hasSync = false;
      toothCurrentCount = 0;
    }
  }
}


void triggerSec_Harley()

{
  return;
}


uint16_t getRPM_Harley()
{
  uint16_t tempRPM = 0;
  if (currentStatus.hasSync == true)
  {
    if ( currentStatus.RPM < (unsigned int)(configPage4.crankRPM * 100) )
    {

      int tempToothAngle;
      unsigned long toothTime;
      if ( (toothLastToothTime == 0) || (toothLastMinusOneToothTime == 0) ) { tempRPM = 0; }
      else
      {
        noInterrupts();
        tempToothAngle = triggerToothAngle;




        revolutionTime = (toothOneTime - toothOneMinusOneTime);
        toothTime = (toothLastToothTime - toothLastMinusOneToothTime);
        interrupts();
        toothTime = toothTime * 36;
        tempRPM = ((unsigned long)tempToothAngle * 6000000UL) / toothTime;
      }
    }
    else {
      tempRPM = stdGetRPM(360);
    }
  }
  return tempRPM;
}


int getCrankAngle_Harley()
{

  unsigned long tempToothLastToothTime;
  int tempToothCurrentCount;

  noInterrupts();
  tempToothCurrentCount = toothCurrentCount;
  tempToothLastToothTime = toothLastToothTime;
  lastCrankAngleCalc = micros();
  interrupts();


  int crankAngle;
  if ( (tempToothCurrentCount == 1) || (tempToothCurrentCount == 3) )
  {
    crankAngle = 0 + configPage4.triggerAngle;
  }
  else {
    crankAngle = 157 + configPage4.triggerAngle;
  }


  elapsedTime = (lastCrankAngleCalc - tempToothLastToothTime);
  crankAngle += timeToAngle(elapsedTime, CRANKMATH_METHOD_INTERVAL_REV);

  if (crankAngle >= 720) { crankAngle -= 720; }
  if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
  if (crankAngle < 0) { crankAngle += 360; }

  return crankAngle;
}

void triggerSetEndTeeth_Harley()
{
  lastToothCalcAdvance = currentStatus.advance;
}
//# 3075 "/home/developper/speeduino/speeduino/decoders.ino"
void triggerSetup_ThirtySixMinus222()
{
  triggerToothAngle = 10;
  triggerActualTeeth = 30;
  triggerFilterTime = (int)(1000000 / (MAX_RPM / 60 * configPage4.triggerTeeth));
  secondDerivEnabled = false;
  decoderIsSequential = false;
  checkSyncToothCount = (configPage4.triggerTeeth) >> 1;
  toothLastMinusOneToothTime = 0;
  toothCurrentCount = 0;
  toothOneTime = 0;
  toothOneMinusOneTime = 0;
  MAX_STALL_TIME = (3333UL * triggerToothAngle * 2 );
}

void triggerPri_ThirtySixMinus222()
{
   curTime = micros();
   curGap = curTime - toothLastToothTime;
   if ( curGap >= triggerFilterTime )
   {
     toothCurrentCount++;
     validTrigger = true;




     if(toothSystemCount == 0) { targetGap = ((toothLastToothTime - toothLastMinusOneToothTime)) * 2; }


     if( (toothLastToothTime == 0) || (toothLastMinusOneToothTime == 0) ) { curGap = 0; }

     if ( (curGap > targetGap) )
     {
       {
         if(toothSystemCount == 1)
         {

           toothCurrentCount = 19;
           toothSystemCount = 0;
           currentStatus.hasSync = true;
         }
         else
         {

           toothSystemCount = 1;
           toothCurrentCount++;
           toothCurrentCount++;
         }
         triggerToothAngleIsCorrect = false;
         triggerFilterTime = 0;
       }
     }
     else
     {
       if(toothCurrentCount > 36)
       {

         toothCurrentCount = 1;
         revolutionOne = !revolutionOne;
         toothOneMinusOneTime = toothOneTime;
         toothOneTime = curTime;
         currentStatus.startRevolutions++;

       }
       else if(toothSystemCount == 1)
       {

         toothCurrentCount = 35;
         currentStatus.hasSync = true;
       }


       setFilter(curGap);

       triggerToothAngleIsCorrect = true;
       toothSystemCount = 0;
     }

     toothLastMinusOneToothTime = toothLastToothTime;
     toothLastToothTime = curTime;


     if(configPage2.perToothIgn == true)
     {
       uint16_t crankAngle = ( (toothCurrentCount-1) * triggerToothAngle ) + configPage4.triggerAngle;
       checkPerToothTiming(crankAngle, toothCurrentCount);
     }

   }
}

void triggerSec_ThirtySixMinus222()
{

}

int getCrankAngle_ThirtySixMinus222()
{

    return 0;
}

void triggerSetEndTeeth_ThirtySixMinus222()
{
  if(currentStatus.advance < 10) { ignition1EndTooth = 36; }
  else if(currentStatus.advance < 20) { ignition1EndTooth = 35; }
  else if(currentStatus.advance < 30) { ignition1EndTooth = 34; }
  else { ignition1EndTooth = 31; }

  if(currentStatus.advance < 30) { ignition2EndTooth = 16; }
  else { ignition2EndTooth = 13; }

  lastToothCalcAdvance = currentStatus.advance;
}
//# 1 "/home/developper/speeduino/speeduino/display.ino"






#ifdef USE_DISPLAY

#include <SPI.h>
#include <Wire.h>
#include "src/Adafruit_SSD1306/Adafruit_GFX.h"
#include "src/Adafruit_SSD1306/Adafruit_SSD1306.h"

Adafruit_SSD1306 display(pinDisplayReset);

void initialiseDisplay()
{

  if(pinTrigger == 20 || pinTrigger == 21 || pinTrigger2 == 20 || pinTrigger2 == 21) { return; }

   switch(configPage1.displayType)
   {
     case 1:
       display.SSD1306_SETCOMPINS_V = 0x02;
      break;
    case 2:
       display.SSD1306_SETCOMPINS_V = 0x12;
      break;
     case 3:
       display.SSD1306_SETCOMPINS_V = 0x12;
      break;
     case 4:
       display.SSD1306_SETCOMPINS_V = 0x12;
      break;
   }

   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0,0);
   display.print("RPM: ");
   display.setCursor(0,16);
   display.print("CPU: ");
}

void updateDisplay()
{
  display.clearDisplay();
  display.setCursor(0,0);
  switch(configPage1.display1)
  {
    case 0:
      display.print("RPM: ");
      display.setCursor(28,0);
      display.print(currentStatus.RPM);
      break;
    case 1:
      display.print("PW: ");
      display.setCursor(28,0);
      display.print(currentStatus.PW1);
      break;
    case 2:
      display.print("Adv: ");
      display.setCursor(28,0);
      display.print(currentStatus.advance);
      break;
    case 3:
      display.print("VE: ");
      display.setCursor(28,0);
      display.print(currentStatus.VE);
      break;
    case 4:
      display.print("GamE: ");
      display.setCursor(28,0);
      display.print(currentStatus.corrections);
      break;
    case 5:
      display.print("TPS: ");
      display.setCursor(28,0);
      display.print(currentStatus.TPS);
      break;
    case 6:
      display.print("IAT: ");
      display.setCursor(28,0);
      display.print(currentStatus.IAT);
      break;
    case 7:
      display.print("CLT: ");
      display.setCursor(28,0);
      display.print(currentStatus.coolant);
      break;
  }

  display.setCursor(0,11);
  switch(configPage1.display3)
  {
    case 0:
      display.print("RPM: ");
      display.setCursor(28,11);
      display.print(currentStatus.RPM);
      break;
    case 1:
      display.print("PW: ");
      display.setCursor(28,11);
      display.print(currentStatus.PW1);
      break;
    case 2:
      display.print("Adv: ");
      display.setCursor(28,11);
      display.print(currentStatus.advance);
      break;
    case 3:
      display.print("VE: ");
      display.setCursor(28,11);
      display.print(currentStatus.VE);
      break;
    case 4:
      display.print("GamE: ");
      display.setCursor(28,11);
      display.print(currentStatus.corrections);
      break;
    case 5:
      display.print("TPS: ");
      display.setCursor(28,11);
      display.print(currentStatus.TPS);
      break;
    case 6:
      display.print("IAT: ");
      display.setCursor(28,11);
      display.print(currentStatus.IAT);
      break;
    case 7:
      display.print("CLT: ");
      display.setCursor(28,11);
      display.print(currentStatus.coolant);
      break;
  }

  display.setCursor(64,0);
  switch(configPage1.display2)
  {
    case 0:
      display.print("O2: ");
      display.setCursor(92,0);
      display.print(currentStatus.O2);
      break;
    case 1:
      display.print("Vdc: ");
      display.setCursor(92,0);
      display.print(currentStatus.battery10);
      break;
    case 2:
      display.print("CPU: ");
      display.setCursor(92,0);
      display.print(currentStatus.loopsPerSecond);
      break;
    case 3:
      display.print("Mem: ");
      display.setCursor(92,0);
      display.print(currentStatus.freeRAM);
      break;
  }

  display.setCursor(64,11);
  switch(configPage1.display4)
  {
    case 0:
      display.print("O2: ");
      display.setCursor(92,11);
      display.print(currentStatus.O2);
      break;
    case 1:
      display.print("Vdc: ");
      display.setCursor(92,11);
      display.print(currentStatus.battery10);
      break;
    case 2:
      display.print("CPU: ");
      display.setCursor(92,11);
      display.print(currentStatus.loopsPerSecond);
      break;
    case 3:
      display.print("Mem: ");
      display.setCursor(92,11);
      display.print(currentStatus.freeRAM);
      break;
  }

  int barWidth = ldiv(((unsigned long)currentStatus.RPM * 128), 9000).quot;

  display.fillRect(0, 20, barWidth, 10, 1);

  display.display();
}
#endif
//# 1 "/home/developper/speeduino/speeduino/errors.ino"
//# 11 "/home/developper/speeduino/speeduino/errors.ino"
#include "globals.h"
#include "errors.h"

byte setError(byte errorID)
{
  if(errorCount < MAX_ERRORS)
  {
    errorCodes[errorCount] = errorID;
    errorCount++;
    if(errorCount == 1) { BIT_SET(currentStatus.spark, BIT_SPARK_ERROR); }
  }
  return errorCount;
}

void clearError(byte errorID)
{
  byte clearedError = 255;

  if (errorID == errorCodes[0]) { clearedError = 0; }
  else if(errorID == errorCodes[1]) { clearedError = 1; }
  else if(errorID == errorCodes[2]) { clearedError = 2; }
  else if(errorID == errorCodes[3]) { clearedError = 3; }

  if(clearedError < MAX_ERRORS)
  {
    errorCodes[clearedError] = ERR_NONE;

    for (byte x=clearedError; x < (errorCount-1); x++)
    {
      errorCodes[x] = errorCodes[x+1];
      errorCodes[x+1] = ERR_NONE;
    }

    errorCount--;
    if(errorCount == 0) { BIT_CLEAR(currentStatus.spark, BIT_SPARK_ERROR); }
  }
}

byte getNextError()
{
  packedError currentError;


  byte currentErrorNum = currentStatus.secl % MAX_ERRORS;

  currentError.errorNum = currentErrorNum;
  currentError.errorID = errorCodes[currentErrorNum];

  return *(byte*)&currentError;
}
//# 1 "/home/developper/speeduino/speeduino/idle.ino"





#include "idle.h"
#include "maths.h"
#include "timers.h"
#include "src/PID_v1/PID_v1.h"
//# 19 "/home/developper/speeduino/speeduino/idle.ino"
integerPID idlePID(&currentStatus.longRPM, &idle_pid_target_value, &idle_cl_target_rpm, configPage6.idleKP, configPage6.idleKI, configPage6.idleKD, DIRECT);

void initialiseIdle()
{

  IDLE_TIMER_DISABLE();


  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:

      break;

    case IAC_ALGORITHM_ONOFF:

      if ((currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < configPage6.iacFastTemp)
      {
        digitalWrite(pinIdle1, HIGH);
        idleOn = true;
      }
      break;

    case IAC_ALGORITHM_PWM_OL:

      iacPWMTable.xSize = 10;
      iacPWMTable.valueSize = SIZE_BYTE;
      iacPWMTable.values = configPage6.iacOLPWMVal;
      iacPWMTable.axisX = configPage6.iacBins;


      iacCrankDutyTable.xSize = 4;
      iacCrankDutyTable.valueSize = SIZE_BYTE;
      iacCrankDutyTable.values = configPage6.iacCrankDuty;
      iacCrankDutyTable.axisX = configPage6.iacCrankBins;

      idle_pin_port = portOutputRegister(digitalPinToPort(pinIdle1));
      idle_pin_mask = digitalPinToBitMask(pinIdle1);
      idle2_pin_port = portOutputRegister(digitalPinToPort(pinIdle2));
      idle2_pin_mask = digitalPinToBitMask(pinIdle2);
      #if defined(CORE_AVR)
        idle_pwm_max_count = 1000000L / (16 * configPage6.idleFreq * 2);
      #elif defined(CORE_TEENSY)
        idle_pwm_max_count = 1000000L / (32 * configPage6.idleFreq * 2);
      #endif
      enableIdle();
      break;

    case IAC_ALGORITHM_PWM_CL:

      iacClosedLoopTable.xSize = 10;
      iacClosedLoopTable.valueSize = SIZE_BYTE;
      iacClosedLoopTable.values = configPage6.iacCLValues;
      iacClosedLoopTable.axisX = configPage6.iacBins;

      iacCrankDutyTable.xSize = 4;
      iacCrankDutyTable.valueSize = SIZE_BYTE;
      iacCrankDutyTable.values = configPage6.iacCrankDuty;
      iacCrankDutyTable.axisX = configPage6.iacCrankBins;

      idle_pin_port = portOutputRegister(digitalPinToPort(pinIdle1));
      idle_pin_mask = digitalPinToBitMask(pinIdle1);
      idle2_pin_port = portOutputRegister(digitalPinToPort(pinIdle2));
      idle2_pin_mask = digitalPinToBitMask(pinIdle2);
      #if defined(CORE_AVR)
        idle_pwm_max_count = 1000000L / (16 * configPage6.idleFreq * 2);
      #elif defined(CORE_TEENSY)
        idle_pwm_max_count = 1000000L / (32 * configPage6.idleFreq * 2);
      #endif
      idlePID.SetOutputLimits(percentage(configPage2.iacCLminDuty, idle_pwm_max_count), percentage(configPage2.iacCLmaxDuty, idle_pwm_max_count));
      idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
      idlePID.SetMode(AUTOMATIC);

      idleCounter = 0;
      break;

    case IAC_ALGORITHM_STEP_OL:

      iacStepTable.xSize = 10;
      iacStepTable.valueSize = SIZE_BYTE;
      iacStepTable.values = configPage6.iacOLStepVal;
      iacStepTable.axisX = configPage6.iacBins;

      iacCrankStepsTable.xSize = 4;
      iacCrankStepsTable.valueSize = SIZE_BYTE;
      iacCrankStepsTable.values = configPage6.iacCrankSteps;
      iacCrankStepsTable.axisX = configPage6.iacCrankBins;
      iacStepTime = configPage6.iacStepTime * 1000;
      iacCoolTime = configPage9.iacCoolTime * 1000;

      completedHomeSteps = 0;
      idleStepper.curIdleStep = 0;
      idleStepper.stepperStatus = SOFF;
      if (! configPage9.iacStepperInv)
      {
        idleStepper.lessAirDirection = STEPPER_BACKWARD;
        idleStepper.moreAirDirection = STEPPER_FORWARD;
      }
      else
      {
        idleStepper.lessAirDirection = STEPPER_FORWARD;
        idleStepper.moreAirDirection = STEPPER_BACKWARD;
      }

      break;

    case IAC_ALGORITHM_STEP_CL:

      iacClosedLoopTable.xSize = 10;
      iacClosedLoopTable.valueSize = SIZE_BYTE;
      iacClosedLoopTable.values = configPage6.iacCLValues;
      iacClosedLoopTable.axisX = configPage6.iacBins;

      iacCrankStepsTable.xSize = 4;
      iacCrankStepsTable.valueSize = SIZE_BYTE;
      iacCrankStepsTable.values = configPage6.iacCrankSteps;
      iacCrankStepsTable.axisX = configPage6.iacCrankBins;
      iacStepTime = configPage6.iacStepTime * 1000;
      iacCoolTime = configPage9.iacCoolTime * 1000;

      completedHomeSteps = 0;
      idleCounter = 0;
      idleStepper.curIdleStep = 0;
      idleStepper.stepperStatus = SOFF;

      if (! configPage9.iacStepperInv)
      {
        idleStepper.lessAirDirection = STEPPER_BACKWARD;
        idleStepper.moreAirDirection = STEPPER_FORWARD;
      }
      else
      {
        idleStepper.lessAirDirection = STEPPER_FORWARD;
        idleStepper.moreAirDirection = STEPPER_BACKWARD;
      }

      idlePID.SetOutputLimits(0, (configPage6.iacStepHome * 3));
      idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
      idlePID.SetMode(AUTOMATIC);
      break;

    default:

      break;
  }
  idleInitComplete = configPage6.iacAlgorithm;
  currentStatus.idleLoad = 0;
}

void idleControl()
{
  if(idleInitComplete != configPage6.iacAlgorithm) { initialiseIdle(); }
  if(currentStatus.RPM > 0) { enableIdle(); }


  if(configPage2.idleUpEnabled == true)
  {
    if(configPage2.idleUpPolarity == 0) { currentStatus.idleUpActive = !digitalRead(pinIdleUp); }
    else { currentStatus.idleUpActive = digitalRead(pinIdleUp); }
  }
  else { currentStatus.idleUpActive = false; }

  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:
      break;

    case IAC_ALGORITHM_ONOFF:
      if ( (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < configPage6.iacFastTemp)
      {
        digitalWrite(pinIdle1, HIGH);
        idleOn = true;
        BIT_SET(currentStatus.spark, BIT_SPARK_IDLE);
      }
      else if (idleOn)
      {
        digitalWrite(pinIdle1, LOW);
        idleOn = false;
        BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE);
      }
      break;

    case IAC_ALGORITHM_PWM_OL:

      if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {

        currentStatus.idleDuty = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
      }
      else
      {

        currentStatus.idleDuty = table2D_getValue(&iacPWMTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
      }

      if(currentStatus.idleUpActive == true) { currentStatus.idleDuty += configPage2.idleUpAdder; }
      if( currentStatus.idleDuty == 0 )
      {
        disableIdle();
        BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE);
        break;
      }
      BIT_SET(currentStatus.spark, BIT_SPARK_IDLE);
      idle_pwm_target_value = percentage(currentStatus.idleDuty, idle_pwm_max_count);
      currentStatus.idleLoad = currentStatus.idleDuty >> 1;
      idleOn = true;

      break;

    case IAC_ALGORITHM_PWM_CL:

        currentStatus.CLIdleTarget = (byte)table2D_getValue(&iacClosedLoopTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
        idle_cl_target_rpm = (uint16_t)currentStatus.CLIdleTarget * 10;
        if( (idleCounter & 31) == 1) { idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD); }

        idlePID.Compute();
        idle_pwm_target_value = idle_pid_target_value;
        if( idle_pwm_target_value == 0 )
        {
          disableIdle();
          BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE);
          break;
        }
        BIT_SET(currentStatus.spark, BIT_SPARK_IDLE);
        currentStatus.idleLoad = ((unsigned long)(idle_pwm_target_value * 100UL) / idle_pwm_max_count) >> 1;
        if(currentStatus.idleUpActive == true) { currentStatus.idleDuty += configPage2.idleUpAdder; }

        idleCounter++;
      break;

    case IAC_ALGORITHM_STEP_OL:

      if( (checkForStepping() == false) && (isStepperHomed() == true) )
      {

        if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
        {

          idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3;
          if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; }
          doStep();
        }
        else if( (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < iacStepTable.axisX[IDLE_TABLE_SIZE-1])
        {


          if (((mainLoopCount & 255) == 1) && (currentStatus.RPM > 0))
          {

            idleStepper.targetIdleStep = table2D_getValue(&iacStepTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3;
            if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; }
            iacStepTime = configPage6.iacStepTime * 1000;
            iacCoolTime = configPage9.iacCoolTime * 1000;
          }
          doStep();
        }
        currentStatus.idleLoad = idleStepper.curIdleStep >> 1;
      }

      if(idleStepper.targetIdleStep != idleStepper.curIdleStep) { BIT_SET(currentStatus.spark, BIT_SPARK_IDLE); }
      else { BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE); }
      break;

    case IAC_ALGORITHM_STEP_CL:

      if( (checkForStepping() == false) && (isStepperHomed() == true) )
      {
        if( (idleCounter & 31) == 1)
        {

          idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
          iacStepTime = configPage6.iacStepTime * 1000;
          iacCoolTime = configPage9.iacCoolTime * 1000;
        }

        currentStatus.CLIdleTarget = (byte)table2D_getValue(&iacClosedLoopTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
        idle_cl_target_rpm = (uint16_t)currentStatus.CLIdleTarget * 10;
        if(currentStatus.idleUpActive == true) { idle_pid_target_value += configPage2.idleUpAdder; }
        idlePID.Compute();
        idleStepper.targetIdleStep = idle_pid_target_value;

        doStep();
        currentStatus.idleLoad = idleStepper.curIdleStep >> 1;
        idleCounter++;
      }

      if(idleStepper.targetIdleStep != idleStepper.curIdleStep) { BIT_SET(currentStatus.spark, BIT_SPARK_IDLE); }
      else { BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE); }
      break;

    default:

      break;
  }
}







static inline byte isStepperHomed()
{
  bool isHomed = true;
  if( completedHomeSteps < (configPage6.iacStepHome * 3) )
  {
    digitalWrite(pinStepperDir, idleStepper.lessAirDirection);
    digitalWrite(pinStepperEnable, LOW);
    digitalWrite(pinStepperStep, HIGH);
    idleStepper.stepStartTime = micros_safe();
    idleStepper.stepperStatus = STEPPING;
    completedHomeSteps++;
    idleOn = true;
    isHomed = false;
  }
  return isHomed;
}







static inline byte checkForStepping()
{
  bool isStepping = false;
  unsigned int timeCheck;

  if( (idleStepper.stepperStatus == STEPPING) || (idleStepper.stepperStatus == COOLING) )
  {
    if (idleStepper.stepperStatus == STEPPING)
    {
      timeCheck = iacStepTime;
    }
    else
    {
      timeCheck = iacCoolTime;
    }

    if(micros_safe() > (idleStepper.stepStartTime + timeCheck) )
    {
      if(idleStepper.stepperStatus == STEPPING)
      {

        digitalWrite(pinStepperStep, LOW);
        idleStepper.stepStartTime = micros_safe();


        if (iacCoolTime > 0)
        {
          idleStepper.stepperStatus = COOLING;
        }
        else
        {
          idleStepper.stepperStatus = SOFF;
        }

        isStepping = true;
      }
      else
      {

        idleStepper.stepperStatus = SOFF;
        digitalWrite(pinStepperEnable, HIGH);
      }
    }
    else
    {

      isStepping = true;
    }
  }
  return isStepping;
}




static inline void doStep()
{
  if ( (idleStepper.targetIdleStep <= (idleStepper.curIdleStep - configPage6.iacStepHyster)) || (idleStepper.targetIdleStep >= (idleStepper.curIdleStep + configPage6.iacStepHyster)) )
  {

    if(idleStepper.targetIdleStep < idleStepper.curIdleStep)
    {

      digitalWrite(pinStepperDir, idleStepper.lessAirDirection);
      idleStepper.curIdleStep--;
    }
    else
    if (idleStepper.targetIdleStep > idleStepper.curIdleStep)
    {

      digitalWrite(pinStepperDir, idleStepper.moreAirDirection);
      idleStepper.curIdleStep++;
    }

    digitalWrite(pinStepperEnable, LOW);
    digitalWrite(pinStepperStep, HIGH);
    idleStepper.stepStartTime = micros_safe();
    idleStepper.stepperStatus = STEPPING;
    idleOn = true;
  }
}


static inline void disableIdle()
{
  if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) )
  {
    IDLE_TIMER_DISABLE();
    digitalWrite(pinIdle1, LOW);
  }
  else if ((configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL) )
  {

    if( (checkForStepping() == false) && (isStepperHomed() == true) )
    {




        idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3;
        if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; }
    }
  }
  BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE);
  currentStatus.idleLoad = 0;
}



static inline void enableIdle()
{
  if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) )
  {
    IDLE_TIMER_ENABLE();
  }
  else if ( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL) )
  {

  }
}

#if defined(CORE_AVR)
ISR(TIMER4_COMPC_vect)
#else
static inline void idleInterrupt()
#endif
{
  if (idle_pwm_state)
  {
    if (configPage6.iacPWMdir == 0)
    {

      *idle_pin_port &= ~(idle_pin_mask);
      if(configPage6.iacChannels == 1) { *idle2_pin_port |= (idle2_pin_mask); }
    }
    else
    {

      *idle_pin_port |= (idle_pin_mask);
      if(configPage6.iacChannels == 1) { *idle2_pin_port &= ~(idle2_pin_mask); }
    }
    IDLE_COMPARE = IDLE_COUNTER + (idle_pwm_max_count - idle_pwm_cur_value);
    idle_pwm_state = false;
  }
  else
  {
    if (configPage6.iacPWMdir == 0)
    {

      *idle_pin_port |= (idle_pin_mask);
      if(configPage6.iacChannels == 1) { *idle2_pin_port &= ~(idle2_pin_mask); }
    }
    else
    {

      *idle_pin_port &= ~(idle_pin_mask);
      if(configPage6.iacChannels == 1) { *idle2_pin_port |= (idle2_pin_mask); }
    }
    IDLE_COMPARE = IDLE_COUNTER + idle_pwm_target_value;
    idle_pwm_cur_value = idle_pwm_target_value;
    idle_pwm_state = true;
  }
}

#if defined(CORE_TEENSY)
void ftm2_isr(void)
{


  bool interrupt1 = (FTM2_C0SC & FTM_CSC_CHF);
  bool interrupt2 = (FTM2_C1SC & FTM_CSC_CHF);

  if(interrupt1) { FTM2_C0SC &= ~FTM_CSC_CHF; idleInterrupt(); }
  else if(interrupt2) { FTM2_C1SC &= ~FTM_CSC_CHF; }
}
#endif
//# 1 "/home/developper/speeduino/speeduino/init.ino"
#include "globals.h"
#include "init.h"
#include "storage.h"
#include "updates.h"
#include "speeduino.h"
#include "timers.h"
#include "cancomms.h"
#include "utils.h"
#include "scheduledIO.h"
#include "scheduler.h"
#include "auxiliaries.h"
#include "sensors.h"
#include "decoders.h"
#include "corrections.h"
#include "idle.h"
#include "table.h"
#include BOARD_H

void initialiseAll()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    table3D_setSize(&fuelTable, 16);
    table3D_setSize(&fuelTable2, 16);
    table3D_setSize(&ignitionTable, 16);
    table3D_setSize(&afrTable, 16);
    table3D_setSize(&stagingTable, 8);
    table3D_setSize(&boostTable, 8);
    table3D_setSize(&vvtTable, 8);
    table3D_setSize(&trim1Table, 6);
    table3D_setSize(&trim2Table, 6);
    table3D_setSize(&trim3Table, 6);
    table3D_setSize(&trim4Table, 6);

    loadConfig();
    doUpdates();



    configPage4.bootloaderCaps = 0;

    initBoard();
    initialiseTimers();

    Serial.begin(115200);
    if (configPage9.enable_secondarySerial == 1) { CANSerial.begin(115200); }

    #if defined(CORE_STM32) || defined(CORE_TEENSY)
    configPage9.intcan_available = 1;






    #endif


    taeTable.valueSize = SIZE_BYTE;
    taeTable.xSize = 4;
    taeTable.values = configPage4.taeValues;
    taeTable.axisX = configPage4.taeBins;
    maeTable.valueSize = SIZE_BYTE;
    maeTable.xSize = 4;
    maeTable.values = configPage4.maeRates;
    maeTable.axisX = configPage4.maeBins;
    WUETable.valueSize = SIZE_BYTE;
    WUETable.xSize = 10;
    WUETable.values = configPage2.wueValues;
    WUETable.axisX = configPage4.wueBins;
    ASETable.valueSize = SIZE_BYTE;
    ASETable.xSize = 4;
    ASETable.values = configPage2.asePct;
    ASETable.axisX = configPage2.aseBins;
    ASECountTable.valueSize = SIZE_BYTE;
    ASECountTable.xSize = 4;
    ASECountTable.values = configPage2.aseCount;
    ASECountTable.axisX = configPage2.aseBins;
    PrimingPulseTable.valueSize = SIZE_BYTE;
    PrimingPulseTable.xSize = 4;
    PrimingPulseTable.values = configPage2.primePulse;
    PrimingPulseTable.axisX = configPage2.primeBins;
    crankingEnrichTable.valueSize = SIZE_BYTE;
    crankingEnrichTable.valueSize = SIZE_BYTE;
    crankingEnrichTable.xSize = 4;
    crankingEnrichTable.values = configPage10.crankingEnrichValues;
    crankingEnrichTable.axisX = configPage10.crankingEnrichBins;

    dwellVCorrectionTable.valueSize = SIZE_BYTE;
    dwellVCorrectionTable.xSize = 6;
    dwellVCorrectionTable.values = configPage4.dwellCorrectionValues;
    dwellVCorrectionTable.axisX = configPage6.voltageCorrectionBins;
    injectorVCorrectionTable.valueSize = SIZE_BYTE;
    injectorVCorrectionTable.xSize = 6;
    injectorVCorrectionTable.values = configPage6.injVoltageCorrectionValues;
    injectorVCorrectionTable.axisX = configPage6.voltageCorrectionBins;
    IATDensityCorrectionTable.valueSize = SIZE_BYTE;
    IATDensityCorrectionTable.xSize = 9;
    IATDensityCorrectionTable.values = configPage6.airDenRates;
    IATDensityCorrectionTable.axisX = configPage6.airDenBins;
    IATRetardTable.valueSize = SIZE_BYTE;
    IATRetardTable.xSize = 6;
    IATRetardTable.values = configPage4.iatRetValues;
    IATRetardTable.axisX = configPage4.iatRetBins;
    CLTAdvanceTable.valueSize = SIZE_BYTE;
    CLTAdvanceTable.xSize = 6;
    CLTAdvanceTable.values = configPage4.cltAdvValues;
    CLTAdvanceTable.axisX = configPage4.cltAdvBins;
    rotarySplitTable.valueSize = SIZE_BYTE;
    rotarySplitTable.xSize = 8;
    rotarySplitTable.values = configPage10.rotarySplitValues;
    rotarySplitTable.axisX = configPage10.rotarySplitBins;

    flexFuelTable.valueSize = SIZE_BYTE;
    flexFuelTable.xSize = 6;
    flexFuelTable.values = configPage10.flexFuelAdj;
    flexFuelTable.axisX = configPage10.flexFuelBins;
    flexAdvTable.valueSize = SIZE_BYTE;
    flexAdvTable.xSize = 6;
    flexAdvTable.values = configPage10.flexAdvAdj;
    flexAdvTable.axisX = configPage10.flexAdvBins;
    flexBoostTable.valueSize = SIZE_INT;
    flexBoostTable.xSize = 6;
    flexBoostTable.values16 = configPage10.flexBoostAdj;
    flexBoostTable.axisX = configPage10.flexBoostBins;

    knockWindowStartTable.valueSize = SIZE_BYTE;
    knockWindowStartTable.xSize = 6;
    knockWindowStartTable.values = configPage10.knock_window_angle;
    knockWindowStartTable.axisX = configPage10.knock_window_rpms;
    knockWindowDurationTable.valueSize = SIZE_BYTE;
    knockWindowDurationTable.xSize = 6;
    knockWindowDurationTable.values = configPage10.knock_window_dur;
    knockWindowDurationTable.axisX = configPage10.knock_window_rpms;


    loadCalibration();


    if(configPage2.pinMapping == 255)
    {

    setPinMapping(3);
    configPage2.flexEnabled = false;
    }
    else { setPinMapping(configPage2.pinMapping); }


    if(configPage4.IgInv == 1) { coilHIGH = LOW; coilLOW = HIGH; }
    else { coilHIGH = HIGH; coilLOW = LOW; }
    endCoil1Charge();
    endCoil2Charge();
    endCoil3Charge();
    endCoil4Charge();
    endCoil5Charge();
    #if (INJ_CHANNELS >= 6)
    endCoil6Charge();
    #endif
    #if (INJ_CHANNELS >= 7)
    endCoil7Charge();
    #endif
    #if (INJ_CHANNELS >= 8)
    endCoil8Charge();
    #endif


    closeInjector1();
    closeInjector2();
    closeInjector3();
    closeInjector4();
    closeInjector5();
    #if (IGN_CHANNELS >= 6)
    closeInjector6();
    #endif
    #if (IGN_CHANNELS >= 7)
    closeInjector7();
    #endif
    #if (IGN_CHANNELS >= 8)
    closeInjector8();
    #endif


    digitalWrite(pinTachOut, HIGH);

    initialiseSchedulers();

    initialiseIdle();
    initialiseFan();
    initialiseAuxPWM();
    initialiseCorrections();
    initialiseADC();


    instanteneousMAPReading();

    if ( configPage6.useExtBaro != 0 )
    {
      readBaro();

      storeLastBaro(currentStatus.baro);
    }
    else
    {





    if ((currentStatus.MAP >= BARO_MIN) && (currentStatus.MAP <= BARO_MAX))
    {
        currentStatus.baro = currentStatus.MAP;

        storeLastBaro(currentStatus.baro);
    }
    else
    {

        if ((readLastBaro() >= BARO_MIN) && (readLastBaro() <= BARO_MAX))
        { currentStatus.baro = readLastBaro(); }
        else { currentStatus.baro = 100; }
    }
    }


    if(configPage2.flexEnabled > 0)
    {
    attachInterrupt(digitalPinToInterrupt(pinFlex), flexPulse, RISING);
    currentStatus.ethanolPct = 0;
    }


    req_fuel_uS = configPage2.reqFuel * 100;
    inj_opentime_uS = configPage2.injOpen * 100;

    if(configPage10.stagingEnabled == true)
    {
    uint32_t totalInjector = configPage10.stagedInjSizePri + configPage10.stagedInjSizeSec;
//# 248 "/home/developper/speeduino/speeduino/init.ino"
    staged_req_fuel_mult_pri = (100 * totalInjector) / configPage10.stagedInjSizePri;
    staged_req_fuel_mult_sec = (100 * totalInjector) / configPage10.stagedInjSizeSec;
    }




    currentStatus.RPM = 0;
    currentStatus.hasSync = false;
    currentStatus.runSecs = 0;
    currentStatus.secl = 0;
    currentStatus.startRevolutions = 0;
    currentStatus.syncLossCounter = 0;
    currentStatus.flatShiftingHard = false;
    currentStatus.launchingHard = false;
    currentStatus.crankRPM = ((unsigned int)configPage4.crankRPM * 10);
    currentStatus.fuelPumpOn = false;
    triggerFilterTime = 0;
    dwellLimit_uS = (1000 * configPage4.dwellLimit);
    currentStatus.nChannels = (INJ_CHANNELS << 4) + IGN_CHANNELS;
    fpPrimeTime = 0;

    noInterrupts();
    initialiseTriggers();


    if(configPage2.strokes == FOUR_STROKE)
    {

      req_fuel_uS = req_fuel_uS / 2;
    }


    previousLoopTime = 0;
    currentLoopTime = micros_safe();

    mainLoopCount = 0;

    currentStatus.nSquirts = configPage2.nCylinders / configPage2.divider;
    if(currentStatus.nSquirts == 0) { currentStatus.nSquirts = 1; }
    if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = 720 / currentStatus.nSquirts; }
    else { CRANK_ANGLE_MAX_INJ = 360 / currentStatus.nSquirts; }


    switch (configPage2.nCylinders) {
    case 1:
        channel1IgnDegrees = 0;
        channel1InjDegrees = 0;
        maxIgnOutputs = 1;


        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) ) { CRANK_ANGLE_MAX_IGN = 720; }

        if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
        {
          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
          req_fuel_uS = req_fuel_uS * 2;
        }

        channel1InjEnabled = true;


        if(configPage10.stagingEnabled == true)
        {
          channel3InjEnabled = true;
          channel3InjDegrees = channel1InjDegrees;
        }
        break;

    case 2:
        channel1IgnDegrees = 0;
        channel1InjDegrees = 0;
        maxIgnOutputs = 2;
        if (configPage2.engineType == EVEN_FIRE ) { channel2IgnDegrees = 180; }
        else { channel2IgnDegrees = configPage2.oddfire2; }


        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) ) { CRANK_ANGLE_MAX_IGN = 720; }

        if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
        {
          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
          req_fuel_uS = req_fuel_uS * 2;
        }

        if (configPage2.engineType == EVEN_FIRE ) { channel2InjDegrees = 180; }
        else { channel2InjDegrees = configPage2.oddfire2; }
        if (!configPage2.injTiming)
        {

          channel1InjDegrees = 0;
          channel2InjDegrees = 0;
        }

        channel1InjEnabled = true;
        channel2InjEnabled = true;


        if(configPage10.stagingEnabled == true)
        {
          channel3InjEnabled = true;
          channel4InjEnabled = true;

          channel3InjDegrees = channel1InjDegrees;
          channel4InjDegrees = channel2InjDegrees;
        }

        break;

    case 3:
        channel1IgnDegrees = 0;
        maxIgnOutputs = 3;
        if (configPage2.engineType == EVEN_FIRE )
        {

        if( ( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) || (configPage4.sparkMode == IGN_MODE_SINGLE) ) && (configPage2.strokes == FOUR_STROKE) )
        {
          channel2IgnDegrees = 240;
          channel3IgnDegrees = 480;

          CRANK_ANGLE_MAX_IGN = 720;
        }
        else
        {
          channel2IgnDegrees = 120;
          channel3IgnDegrees = 240;
        }
        }
        else
        {
        channel2IgnDegrees = configPage2.oddfire2;
        channel3IgnDegrees = configPage2.oddfire3;
        }


        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) || (configPage2.strokes == TWO_STROKE) )
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 120;
          channel3InjDegrees = 240;


          if (currentStatus.nSquirts > 2)
          {
            channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
            channel3InjDegrees = (channel3InjDegrees * 2) / currentStatus.nSquirts;
          }

          if (!configPage2.injTiming)
          {

            channel1InjDegrees = 0;
            channel2InjDegrees = 0;
            channel3InjDegrees = 0;
          }
        }
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 240;
          channel3InjDegrees = 480;
          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
          req_fuel_uS = req_fuel_uS * 2;
        }

        channel1InjEnabled = true;
        channel2InjEnabled = true;
        channel3InjEnabled = true;
        break;
    case 4:
        channel1IgnDegrees = 0;
        channel1InjDegrees = 0;
        maxIgnOutputs = 2;
        if (configPage2.engineType == EVEN_FIRE )
        {
          channel2IgnDegrees = 180;

          if (currentStatus.nSquirts > 2)
          {
            channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
          }

          if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
          {
            channel3IgnDegrees = 360;
            channel4IgnDegrees = 540;

            CRANK_ANGLE_MAX_IGN = 720;
            maxIgnOutputs = 4;
          }
          else if(configPage4.sparkMode == IGN_MODE_ROTARY)
          {

            channel3IgnDegrees = 0;
            channel4IgnDegrees = 180;
          }
        }
        else
        {
          channel2IgnDegrees = configPage2.oddfire2;
          channel3IgnDegrees = configPage2.oddfire3;
          channel4IgnDegrees = configPage2.oddfire4;
          maxIgnOutputs = 4;
        }


        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) || (configPage2.strokes == TWO_STROKE) )
        {
          channel2InjDegrees = 180;

          if (!configPage2.injTiming)
          {

            channel1InjDegrees = 0;
            channel2InjDegrees = 0;
          }
        }
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          channel2InjDegrees = 180;
          channel3InjDegrees = 360;
          channel4InjDegrees = 540;

          channel3InjEnabled = true;
          channel4InjEnabled = true;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
          req_fuel_uS = req_fuel_uS * 2;
        }


        if(configPage10.stagingEnabled == true)
        {
          channel3InjEnabled = true;
          channel4InjEnabled = true;

          channel3InjDegrees = channel1InjDegrees;
          channel4InjDegrees = channel2InjDegrees;
        }

        channel1InjEnabled = true;
        channel2InjEnabled = true;
        break;
    case 5:
        channel1IgnDegrees = 0;
        channel2IgnDegrees = 72;
        channel3IgnDegrees = 144;
        channel4IgnDegrees = 216;
        channel5IgnDegrees = 288;
        maxIgnOutputs = 4;

        if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
        {
          channel2IgnDegrees = 144;
          channel3IgnDegrees = 288;
          channel4IgnDegrees = 432;
          channel5IgnDegrees = 576;

          CRANK_ANGLE_MAX_IGN = 720;
        }


        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) || (configPage2.strokes == TWO_STROKE) )
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 72;
          channel3InjDegrees = 144;
          channel4InjDegrees = 216;
          channel5InjDegrees = 288;
        }
        else if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 144;
          channel3InjDegrees = 288;
          channel4InjDegrees = 432;
          channel5InjDegrees = 576;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
        }
        if (!configPage2.injTiming)
        {

          channel1InjDegrees = 0;
          channel2InjDegrees = 0;
          channel3InjDegrees = 0;
          channel4InjDegrees = 0;
          channel5InjDegrees = 0;
        }

        channel1InjEnabled = true;
        channel2InjEnabled = true;
        channel3InjEnabled = false;
        channel4InjEnabled = true;
        channel5InjEnabled = true;
        break;
    case 6:
        channel1IgnDegrees = 0;
        channel1InjDegrees = 0;
        channel2IgnDegrees = 120;
        channel2InjDegrees = 120;
        channel3IgnDegrees = 240;
        channel3InjDegrees = 240;
        maxIgnOutputs = 3;


        if (currentStatus.nSquirts > 2)
        {
          channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
          channel3InjDegrees = (channel3InjDegrees * 2) / currentStatus.nSquirts;
        }

    #if INJ_CHANNELS >= 6
        if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 120;
          channel3InjDegrees = 240;
          channel4InjDegrees = 360;
          channel5InjDegrees = 480;
          channel6InjDegrees = 600;

          channel4InjEnabled = true;
          channel5InjEnabled = true;
          channel6InjEnabled = true;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
          req_fuel_uS = req_fuel_uS * 2;
        }
    #endif

        if (!configPage2.injTiming)
        {

          channel1InjDegrees = 0;
          channel2InjDegrees = 0;
          channel3InjDegrees = 0;
        }

        configPage2.injLayout = 0;

        channel1InjEnabled = true;
        channel2InjEnabled = true;
        channel3InjEnabled = true;
        break;
    case 8:
        channel1IgnDegrees = 0;
        channel2IgnDegrees = channel2InjDegrees = 90;
        channel3IgnDegrees = channel3InjDegrees = 180;
        channel4IgnDegrees = channel4InjDegrees = 270;


        if (currentStatus.nSquirts > 2)
        {
          channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
          channel3InjDegrees = (channel3InjDegrees * 2) / currentStatus.nSquirts;
          channel4InjDegrees = (channel4InjDegrees * 2) / currentStatus.nSquirts;
        }

    #if INJ_CHANNELS >= 8
        if (configPage2.injLayout == INJ_SEQUENTIAL)
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 90;
          channel3InjDegrees = 180;
          channel4InjDegrees = 270;
          channel5InjDegrees = 360;
          channel6InjDegrees = 450;
          channel7InjDegrees = 540;
          channel8InjDegrees = 630;

          channel5InjEnabled = true;
          channel6InjEnabled = true;
          channel7InjEnabled = true;
          channel8InjEnabled = true;

          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
          req_fuel_uS = req_fuel_uS * 2;
        }
    #endif

        maxIgnOutputs = 4;

        if (!configPage2.injTiming)
        {

          channel1InjDegrees = 0;
          channel2InjDegrees = 0;
          channel3InjDegrees = 0;
          channel4InjDegrees = 0;
        }

        configPage2.injLayout = 0;

        channel1InjEnabled = true;
        channel2InjEnabled = true;
        channel3InjEnabled = true;
        channel4InjEnabled = true;
        break;
    default:
        channel1InjDegrees = 0;
        channel2InjDegrees = 180;
        break;
    }

    if(CRANK_ANGLE_MAX_IGN == CRANK_ANGLE_MAX_INJ) { CRANK_ANGLE_MAX = CRANK_ANGLE_MAX_IGN; }
    else if (CRANK_ANGLE_MAX_IGN > CRANK_ANGLE_MAX_INJ) { CRANK_ANGLE_MAX = CRANK_ANGLE_MAX_IGN; }
    else { CRANK_ANGLE_MAX = CRANK_ANGLE_MAX_INJ; }
    currentStatus.status3 = currentStatus.nSquirts << BIT_STATUS3_NSQUIRTS1;




    if( (currentStatus.nSquirts == 3) || (currentStatus.nSquirts == 5) )
    {
      if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX = 720; }
    }


    switch(configPage4.sparkMode)
    {
    case IGN_MODE_WASTED:

        ign1StartFunction = beginCoil1Charge;
        ign1EndFunction = endCoil1Charge;
        ign2StartFunction = beginCoil2Charge;
        ign2EndFunction = endCoil2Charge;
        ign3StartFunction = beginCoil3Charge;
        ign3EndFunction = endCoil3Charge;
        ign4StartFunction = beginCoil4Charge;
        ign4EndFunction = endCoil4Charge;
        ign5StartFunction = beginCoil5Charge;
        ign5EndFunction = endCoil5Charge;
        break;

    case IGN_MODE_SINGLE:

        ign1StartFunction = beginCoil1Charge;
        ign1EndFunction = endCoil1Charge;
        ign2StartFunction = beginCoil1Charge;
        ign2EndFunction = endCoil1Charge;
        ign3StartFunction = beginCoil1Charge;
        ign3EndFunction = endCoil1Charge;
        ign4StartFunction = beginCoil1Charge;
        ign4EndFunction = endCoil1Charge;
        ign5StartFunction = beginCoil1Charge;
        ign5EndFunction = endCoil1Charge;
        break;

    case IGN_MODE_WASTEDCOP:


        if( configPage2.nCylinders <= 4 )
        {
          ign1StartFunction = beginCoil1and3Charge;
          ign1EndFunction = endCoil1and3Charge;
          ign2StartFunction = beginCoil2and4Charge;
          ign2EndFunction = endCoil2and4Charge;

          ign3StartFunction = nullCallback;
          ign3EndFunction = nullCallback;
          ign4StartFunction = nullCallback;
          ign4EndFunction = nullCallback;
        }
        else
        {

          ign1StartFunction = beginCoil1Charge;
          ign1EndFunction = endCoil1Charge;
          ign2StartFunction = beginCoil2Charge;
          ign2EndFunction = endCoil2Charge;
          ign3StartFunction = beginCoil3Charge;
          ign3EndFunction = endCoil3Charge;
          ign4StartFunction = beginCoil4Charge;
          ign4EndFunction = endCoil4Charge;
          ign5StartFunction = beginCoil5Charge;
          ign5EndFunction = endCoil5Charge;
        }
        break;

    case IGN_MODE_SEQUENTIAL:
        ign1StartFunction = beginCoil1Charge;
        ign1EndFunction = endCoil1Charge;
        ign2StartFunction = beginCoil2Charge;
        ign2EndFunction = endCoil2Charge;
        ign3StartFunction = beginCoil3Charge;
        ign3EndFunction = endCoil3Charge;
        ign4StartFunction = beginCoil4Charge;
        ign4EndFunction = endCoil4Charge;
        ign5StartFunction = beginCoil5Charge;
        ign5EndFunction = endCoil5Charge;
        ign6StartFunction = beginCoil6Charge;
        ign6EndFunction = endCoil6Charge;
        ign7StartFunction = beginCoil7Charge;
        ign7EndFunction = endCoil7Charge;
        ign8StartFunction = beginCoil8Charge;
        ign8EndFunction = endCoil8Charge;
        break;

    case IGN_MODE_ROTARY:
        if(configPage10.rotaryType == ROTARY_IGN_FC)
        {

          ign1StartFunction = beginCoil1Charge;
          ign1EndFunction = endCoil1Charge;
          ign2StartFunction = beginCoil1Charge;
          ign2EndFunction = endCoil1Charge;

          ign3StartFunction = beginTrailingCoilCharge;
          ign3EndFunction = endTrailingCoilCharge1;
          ign4StartFunction = beginTrailingCoilCharge;
          ign4EndFunction = endTrailingCoilCharge2;
        }
        else if(configPage10.rotaryType == ROTARY_IGN_FD)
        {

          ign1StartFunction = beginCoil1Charge;
          ign1EndFunction = endCoil1Charge;
          ign2StartFunction = beginCoil1Charge;
          ign2EndFunction = endCoil1Charge;



          ign3StartFunction = beginCoil2Charge;
          ign3EndFunction = endCoil2Charge;

          ign4StartFunction = beginCoil3Charge;
          ign4EndFunction = endCoil3Charge;


        }
        else if(configPage10.rotaryType == ROTARY_IGN_RX8)
        {



          ign1StartFunction = beginCoil1Charge;
          ign1EndFunction = endCoil1Charge;

          ign2StartFunction = beginCoil2Charge;
          ign2EndFunction = endCoil2Charge;

          ign3StartFunction = beginCoil3Charge;
          ign3EndFunction = endCoil3Charge;

          ign4StartFunction = beginCoil4Charge;
          ign4EndFunction = endCoil4Charge;
        }
        break;



    default:

        ign1StartFunction = beginCoil1Charge;
        ign1EndFunction = endCoil1Charge;
        ign2StartFunction = beginCoil2Charge;
        ign2EndFunction = endCoil2Charge;
        ign3StartFunction = beginCoil3Charge;
        ign3EndFunction = endCoil3Charge;
        ign4StartFunction = beginCoil4Charge;
        ign4EndFunction = endCoil4Charge;
        ign5StartFunction = beginCoil5Charge;
        ign5EndFunction = endCoil5Charge;
        break;
    }



    if(configPage2.fpPrime > 0)
    {
      FUEL_PUMP_ON();
      currentStatus.fuelPumpOn = true;
    }
    else { fpPrimed = true; }

    interrupts();

    readCLT(false);
    unsigned long primingValue = table2D_getValue(&PrimingPulseTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
    if(primingValue > 0)
    {
      setFuelSchedule1(100, (primingValue * 100 * 5));
      setFuelSchedule2(100, (primingValue * 100 * 5));
      setFuelSchedule3(100, (primingValue * 100 * 5));
      setFuelSchedule4(100, (primingValue * 100 * 5));
    }


    initialisationComplete = true;
    digitalWrite(LED_BUILTIN, HIGH);
}

void setPinMapping(byte boardID)
{
  switch (boardID)
  {
    case 0:
    #ifndef SMALL_FLASH_MODE

      pinInjector1 = 8;
      pinInjector2 = 9;
      pinInjector3 = 11;
      pinInjector4 = 10;
      pinInjector5 = 12;
      pinCoil1 = 6;
      pinCoil2 = 7;
      pinCoil3 = 12;
      pinCoil4 = 13;
      pinCoil5 = 14;
      pinTrigger = 2;
      pinTrigger2 = 3;
      pinTPS = A0;
      pinMAP = A1;
      pinIAT = A2;
      pinCLT = A3;
      pinO2 = A4;
      pinIdle1 = 46;
      pinIdle2 = 47;
      pinStepperDir = 16;
      pinStepperStep = 17;
      pinFan = 47;
      pinFuelPump = 4;
      pinTachOut = 49;
      pinFlex = 19;
      pinResetControl = 43;
    #endif
      break;
    case 1:
    #ifndef SMALL_FLASH_MODE

      pinInjector1 = 8;
      pinInjector2 = 9;
      pinInjector3 = 10;
      pinInjector4 = 11;
      pinInjector5 = 12;
      pinCoil1 = 28;
      pinCoil2 = 24;
      pinCoil3 = 40;
      pinCoil4 = 36;
      pinCoil5 = 34;
      pinTrigger = 20;
      pinTrigger2 = 21;
      pinTPS = A2;
      pinMAP = A3;
      pinIAT = A0;
      pinCLT = A1;
      pinO2 = A8;
      pinBat = A4;
      pinDisplayReset = 48;
      pinTachOut = 49;
      pinIdle1 = 30;
      pinIdle2 = 31;
      pinStepperDir = 16;
      pinStepperStep = 17;
      pinFan = 47;
      pinFuelPump = 4;
      pinFlex = 2;
      pinResetControl = 43;
      break;
    #endif
    case 2:
    #ifndef SMALL_FLASH_MODE

      pinInjector1 = 8;
      pinInjector2 = 9;
      pinInjector3 = 10;
      pinInjector4 = 11;
      pinInjector5 = 12;
      pinCoil1 = 28;
      pinCoil2 = 24;
      pinCoil3 = 40;
      pinCoil4 = 36;
      pinCoil5 = 34;
      pinTrigger = 19;
      pinTrigger2 = 18;
      pinTPS = A2;
      pinMAP = A3;
      pinIAT = A0;
      pinCLT = A1;
      pinO2 = A8;
      pinBat = A4;
      pinDisplayReset = 48;
      pinTachOut = 49;
      pinIdle1 = 5;
      pinIdle2 = 53;
      pinBoost = 7;
      pinVVT_1 = 6;
      pinFuelPump = 4;
      pinStepperDir = 16;
      pinStepperStep = 17;
      pinStepperEnable = 26;
      pinFan = A13;
      pinLaunch = 51;
      pinFlex = 2;
      pinResetControl = 50;

      #if defined(CORE_TEENSY)
        pinTrigger = 23;
        pinStepperDir = 33;
        pinStepperStep = 34;
        pinCoil1 = 31;
        pinTachOut = 28;
        pinFan = 27;
        pinCoil4 = 21;
        pinCoil3 = 30;
        pinO2 = A22;
      #endif
    #endif
      break;

    case 3:

      pinInjector1 = 8;
      pinInjector2 = 9;
      pinInjector3 = 10;
      pinInjector4 = 11;
      pinInjector5 = 12;
      pinInjector6 = 50;
      pinCoil1 = 40;
      pinCoil2 = 38;
      pinCoil3 = 52;
      pinCoil4 = 50;
      pinCoil5 = 34;
      pinTrigger = 19;
      pinTrigger2 = 18;
      pinTPS = A2;
      pinMAP = A3;
      pinIAT = A0;
      pinCLT = A1;
      pinO2 = A8;
      pinBat = A4;
      pinDisplayReset = 48;
      pinTachOut = 49;
      pinIdle1 = 5;
      pinIdle2 = 6;
      pinBoost = 7;
      pinVVT_1 = 4;
      pinFuelPump = 45;
      pinStepperDir = 16;
      pinStepperStep = 17;
      pinStepperEnable = 24;
      pinFan = 47;
      pinLaunch = 51;
      pinFlex = 2;
      pinResetControl = 43;

      #if defined(CORE_TEENSY)
        pinInjector6 = 51;

        pinTrigger = 23;
        pinTrigger2 = 36;
        pinStepperDir = 34;
        pinStepperStep = 35;
        pinCoil1 = 31;
        pinCoil2 = 32;
        pinTachOut = 28;
        pinFan = 27;
        pinCoil4 = 29;
        pinCoil3 = 30;
        pinO2 = A22;
      #elif defined(STM32F4)






        pinInjector1 = PE7;
        pinInjector2 = PE8;
        pinInjector3 = PE9;
        pinInjector4 = PE10;
        pinInjector5 = PE11;
        pinInjector6 = PE12;
        pinCoil1 = PD0;
        pinCoil2 = PD1;
        pinCoil3 = PD2;
        pinCoil4 = PD3;
        pinCoil5 = PD4;
        pinTPS = A0;
        pinMAP = A1;
        pinIAT = A2;
        pinCLT = A3;
        pinO2 = A4;
        pinBat = A5;
        pinBaro = A9;
        pinIdle1 = PB8;
        pinIdle2 = PB9;
        pinBoost = PE0;
        pinVVT_1 = PE1;
        pinStepperDir = PD8;
        pinStepperStep = PB15;
        pinStepperEnable = PD9;
        pinDisplayReset = PE1;
        pinFan = PE2;
        pinFuelPump = PC0;
        pinTachOut = PC1;


        pinFlex = PE2;
        pinTrigger = PE3;
        pinTrigger2 = PE4;
      #elif defined(CORE_STM32)



        pinInjector1 = PB7;
        pinInjector2 = PB6;
        pinInjector3 = PB5;
        pinInjector4 = PB4;
        pinCoil1 = PB3;
        pinCoil2 = PA15;
        pinCoil3 = PA14;
        pinCoil4 = PA9;
        pinCoil5 = PA8;
        pinTPS = A0;
        pinMAP = A1;
        pinIAT = A2;
        pinCLT = A3;
        pinO2 = A4;
        pinBat = A5;
        pinBaro = pinMAP;
        pinIdle1 = PB2;
        pinIdle2 = PA2;
        pinBoost = PA1;
        pinVVT_1 = PA0;
        pinStepperDir = PC15;
        pinStepperStep = PC14;
        pinStepperEnable = PC13;
        pinDisplayReset = PB2;
        pinFan = PB1;
        pinFuelPump = PB11;
        pinTachOut = PB10;

        pinFlex = PB8;
        pinTrigger = PA10;
        pinTrigger2 = PA13;
      #endif
      break;

    case 9:

      pinInjector1 = 11;
      pinInjector2 = 10;
      pinInjector3 = 9;
      pinInjector4 = 8;
      pinInjector5 = 14;
      pinCoil1 = 39;
      pinCoil2 = 41;
      pinCoil3 = 32;
      pinCoil4 = 33;
      pinCoil5 = 34;
      pinTrigger = 19;
      pinTrigger2 = 18;
      pinTPS = A2;
      pinMAP = A5;
      pinIAT = A0;
      pinCLT = A1;
      pinO2 = A3;
      pinBat = A4;
      pinDisplayReset = 48;
      pinTachOut = 49;
      pinIdle1 = 2;
      pinBoost = 4;
      pinIdle2 = 4;
      pinFuelPump = 37;
      pinStepperDir = 16;
      pinStepperStep = 17;
      pinFan = 35;
      pinLaunch = 12;
      pinFlex = 3;
      pinResetControl = 44;

      #if defined(CORE_TEENSY)
        pinTrigger = 23;
        pinTrigger2 = 36;
        pinStepperDir = 34;
        pinStepperStep = 35;
        pinCoil1 = 33;
        pinCoil2 = 24;
        pinCoil3 = 51;
        pinCoil4 = 52;
        pinFuelPump = 26;
        pinFan = 50;
        pinTachOut = 28;
      #endif
      break;

    case 10:
    #ifndef SMALL_FLASH_MODE

      pinInjector1 = 4;
      pinInjector2 = 5;
      pinInjector3 = 6;
      pinInjector4 = 7;
      pinInjector5 = 8;
      pinInjector6 = 9;
      pinInjector7 = 10;
      pinInjector8 = 11;
      pinCoil1 = 24;
      pinCoil2 = 28;
      pinCoil3 = 36;
      pinCoil4 = 40;
      pinCoil5 = 34;
      pinTrigger = 18;
      pinTrigger2 = 19;
      pinTPS = A2;
      pinMAP = A3;
      pinMAP2 = A8;
      pinIAT = A0;
      pinCLT = A1;
      pinO2 = A4;
      pinBat = A7;
      pinDisplayReset = 48;
      pinSpareTemp1 = A6;
      pinSpareTemp2 = A5;
      pinTachOut = 41;
      pinFuelPump = 42;
      pinFan = 47;
      pinTachOut = 49;
      pinFlex = 2;
      pinResetControl = 26;

    #endif
      break;

    case 20:
    #if defined(CORE_AVR) && !defined(SMALL_FLASH_MODE)

      pinInjector1 = 8;
      pinInjector2 = 9;
      pinInjector3 = 10;
      pinInjector4 = 11;
      pinInjector5 = 12;
      pinCoil1 = 28;
      pinCoil2 = 24;
      pinCoil3 = 40;
      pinCoil4 = 36;
      pinCoil5 = 34;
      pinSpareOut1 = 4;
      pinSpareOut2 = 5;
      pinSpareOut3 = 6;
      pinSpareOut4 = 7;
      pinSpareOut5 = 50;
      pinSpareOut6 = 52;
      pinTrigger = 20;
      pinTrigger2 = 21;
      pinSpareTemp2 = A15;
      pinSpareTemp1 = A14;
      pinO2 = A8;
      pinBat = A4;
      pinMAP = A3;
      pinTPS = A2;
      pinCLT = A1;
      pinIAT = A0;
      pinFan = 47;
      pinFuelPump = 4;
      pinTachOut = 49;
      pinResetControl = 26;
    #endif
      break;

    case 30:
    #ifndef SMALL_FLASH_MODE

      pinInjector1 = 8;
      pinInjector2 = 9;
      pinInjector3 = 10;
      pinInjector4 = 11;
      pinInjector5 = 12;
      pinCoil1 = 40;
      pinCoil2 = 38;
      pinCoil3 = 50;
      pinCoil4 = 52;
      pinCoil5 = 34;
      pinTrigger = 19;
      pinTrigger2 = 18;
      pinTrigger3 = 17;
      pinTPS = A2;
      pinMAP = A3;
      pinIAT = A0;
      pinCLT = A1;
      pinO2 = A8;
      pinO2_2 = A9;
      pinBat = A4;
      pinDisplayReset = 48;
      pinTachOut = 49;
      pinIdle1 = 5;
      pinFuelPump = 45;
      pinStepperDir = 20;
      pinStepperStep = 21;
      pinSpareHOut1 = 4;
      pinSpareHOut2 = 6;
      pinBoost = 7;
      pinSpareLOut1 = 43;
      pinSpareLOut2 = 47;
      pinSpareLOut3 = 49;
      pinSpareLOut4 = 51;
      pinSpareLOut5 = 53;
      pinFan = 47;
    #endif
      break;

    case 40:

      pinInjector1 = 8;
      pinInjector2 = 9;
      pinInjector3 = 11;
      pinInjector4 = 12;
      pinInjector5 = 13;
      pinCoil1 = 23;
      pinCoil2 = 22;
      pinCoil3 = 2;
      pinCoil4 = 3;
      pinCoil5 = 46;
      pinTrigger = 19;
      pinTrigger2 = 18;
      pinTPS = A3;
      pinMAP = A0;
      pinIAT = A5;
      pinCLT = A4;
      pinO2 = A2;
      pinBat = A1;
      pinBaro = A6;
      pinSpareTemp1 = A7;
      pinDisplayReset = 48;
      pinTachOut = 38;
      pinIdle1 = 5;
      pinIdle2 = 47;
      pinBoost = 7;
      pinVVT_1 = 6;
      pinFuelPump = 4;
      pinStepperDir = 25;
      pinStepperStep = 24;
      pinStepperEnable = 27;
      pinLaunch = 10;
      pinFlex = 20;
      pinFan = 30;
      pinSpareLOut1 = 32;
      pinSpareLOut2 = 34;
      pinSpareLOut3 = 36;
      pinResetControl = 26;
      break;

    case 41:
    #ifndef SMALL_FLASH_MODE

      pinInjector1 = 8;
      pinInjector2 = 7;
      pinInjector3 = 6;
      pinInjector4 = 5;
      pinInjector5 = 45;
      pinCoil1 = 35;
      pinCoil2 = 36;
      pinCoil3 = 33;
      pinCoil4 = 34;
      pinCoil5 = 44;
      pinTrigger = 19;
      pinTrigger2 = 18;
      pinFlex = 20;
      pinTPS = A3;
      pinMAP = A0;
      pinBaro = A7;
      pinIAT = A5;
      pinCLT = A4;
      pinO2 = A1;
      pinO2_2 = A9;
      pinBat = A2;
      pinSpareTemp1 = A8;
      pinLaunch = 37;
      pinDisplayReset = 48;
      pinTachOut = 22;
      pinIdle1 = 9;
      pinIdle2 = 10;
      pinFuelPump = 23;
      pinVVT_1 = 11;
      pinStepperDir = 32;
      pinStepperStep = 31;
      pinStepperEnable = 30;
      pinBoost = 12;
      pinSpareLOut1 = 26;
      pinSpareLOut2 = 27;
      pinSpareLOut3 = 28;
      pinSpareLOut4 = 29;
      pinFan = 24;
      pinResetControl = 46;
    #endif
      break;

    #if defined(CORE_TEENSY)
    case 50:

      pinInjector1 = 2;
      pinInjector2 = 10;
      pinInjector3 = 6;
      pinInjector4 = 9;


      pinCoil1 = 29;
      pinCoil2 = 30;
      pinCoil3 = 31;
      pinCoil4 = 32;


      pinTrigger = 23;
      pinTrigger2 = 36;
      pinTPS = 16;
      pinMAP = 17;
      pinIAT = 14;
      pinCLT = 15;
      pinO2 = A22;
      pinO2_2 = A21;
      pinBat = 18;
      pinTachOut = 20;
      pinIdle1 = 5;
      pinBoost = 11;
      pinFuelPump = 38;
      pinStepperDir = 34;
      pinStepperStep = 35;
      pinStepperEnable = 33;
      pinLaunch = 26;
      pinFan = 37;
      pinSpareHOut1 = 8;
      pinSpareHOut2 = 7;
      pinSpareLOut1 = 21;
      break;

    case 51:

      pinInjector1 = 2;
      pinInjector2 = 10;
      pinInjector3 = 6;
      pinInjector4 = 9;
      pinCoil1 = 29;
      pinCoil2 = 30;
      pinCoil3 = 31;
      pinCoil4 = 32;
      pinTrigger = 23;
      pinTrigger2 = 36;
      pinTPS = 16;
      pinMAP = 17;
      pinIAT = 14;
      pinCLT = 15;
      pinO2 = A22;
      pinO2_2 = A21;
      pinBat = 18;
      pinTachOut = 20;
      pinIdle1 = 5;
      pinBoost = 11;
      pinFuelPump = 38;
      pinStepperDir = 34;
      pinStepperStep = 35;
      pinStepperEnable = 33;
      pinLaunch = 26;
      pinFan = 37;
      pinSpareHOut1 = 8;
      pinSpareHOut2 = 7;
      pinSpareLOut1 = 21;
      break;
    #endif

    default:
      #if defined(STM32F4)






        pinInjector1 = PE7;
        pinInjector2 = PE8;
        pinInjector3 = PE9;
        pinInjector4 = PE10;
        pinInjector5 = PE11;
        pinInjector6 = PE12;
        pinCoil1 = PD0;
        pinCoil2 = PD1;
        pinCoil3 = PD2;
        pinCoil4 = PD3;
        pinCoil5 = PD4;
        pinTPS = A0;
        pinMAP = A1;
        pinIAT = A2;
        pinCLT = A3;
        pinO2 = A4;
        pinBat = A5;
        pinBaro = A9;
        pinIdle1 = PB8;
        pinIdle2 = PB9;
        pinBoost = PE0;
        pinVVT_1 = PE1;
        pinStepperDir = PD8;
        pinStepperStep = PB15;
        pinStepperEnable = PD9;
        pinDisplayReset = PE1;
        pinFan = PE2;
        pinFuelPump = PC0;
        pinTachOut = PC1;


        pinFlex = PE2;
        pinTrigger = PE3;
        pinTrigger2 = PE4;
      #else
        #ifndef SMALL_FLASH_MODE

        pinInjector1 = 8;
        pinInjector2 = 9;
        pinInjector3 = 10;
        pinInjector4 = 11;
        pinInjector5 = 12;
        pinCoil1 = 28;
        pinCoil2 = 24;
        pinCoil3 = 40;
        pinCoil4 = 36;
        pinCoil5 = 34;
        pinTrigger = 20;
        pinTrigger2 = 21;
        pinTPS = A2;
        pinMAP = A3;
        pinIAT = A0;
        pinCLT = A1;
        pinO2 = A8;
        pinBat = A4;
        pinStepperDir = 16;
        pinStepperStep = 17;
        pinDisplayReset = 48;
        pinFan = 47;
        pinFuelPump = 4;
        pinTachOut = 49;
        pinFlex = 3;
        pinBoost = 5;
        pinIdle1 = 6;
        pinResetControl = 43;
        #endif
      #endif
      break;
  }



  if ( (configPage6.launchPin != 0) && (configPage6.launchPin < BOARD_NR_GPIO_PINS) ) { pinLaunch = pinTranslate(configPage6.launchPin); }
  if ( (configPage4.ignBypassPin != 0) && (configPage4.ignBypassPin < BOARD_NR_GPIO_PINS) ) { pinIgnBypass = pinTranslate(configPage4.ignBypassPin); }
  if ( (configPage2.tachoPin != 0) && (configPage2.tachoPin < BOARD_NR_GPIO_PINS) ) { pinTachOut = pinTranslate(configPage2.tachoPin); }
  if ( (configPage4.fuelPumpPin != 0) && (configPage4.fuelPumpPin < BOARD_NR_GPIO_PINS) ) { pinFuelPump = pinTranslate(configPage4.fuelPumpPin); }
  if ( (configPage6.fanPin != 0) && (configPage6.fanPin < BOARD_NR_GPIO_PINS) ) { pinFan = pinTranslate(configPage6.fanPin); }
  if ( (configPage6.boostPin != 0) && (configPage6.boostPin < BOARD_NR_GPIO_PINS) ) { pinBoost = pinTranslate(configPage6.boostPin); }
  if ( (configPage6.vvtPin != 0) && (configPage6.vvtPin < BOARD_NR_GPIO_PINS) ) { pinVVT_1 = pinTranslate(configPage6.vvtPin); }
  if ( (configPage6.useExtBaro != 0) && (configPage6.baroPin < BOARD_NR_GPIO_PINS) ) { pinBaro = configPage6.baroPin + A0; }
  if ( (configPage6.useEMAP != 0) && (configPage10.EMAPPin < BOARD_NR_GPIO_PINS) ) { pinEMAP = configPage10.EMAPPin + A0; }


  pinIdleUp = pinTranslate(configPage2.idleUpPin);




  if ( (configPage4.resetControl != 0) && (configPage4.resetControlPin < BOARD_NR_GPIO_PINS) )
  {
    resetControl = configPage4.resetControl;
    pinResetControl = pinTranslate(configPage4.resetControlPin);
    setResetControlPinState();
    pinMode(pinResetControl, OUTPUT);
  }


  pinMode(pinCoil1, OUTPUT);
  pinMode(pinCoil2, OUTPUT);
  pinMode(pinCoil3, OUTPUT);
  pinMode(pinCoil4, OUTPUT);
  pinMode(pinCoil5, OUTPUT);
  pinMode(pinInjector1, OUTPUT);
  pinMode(pinInjector2, OUTPUT);
  pinMode(pinInjector3, OUTPUT);
  pinMode(pinInjector4, OUTPUT);
  pinMode(pinInjector5, OUTPUT);
  pinMode(pinTachOut, OUTPUT);
  pinMode(pinIdle1, OUTPUT);
  pinMode(pinIdle2, OUTPUT);
  pinMode(pinFuelPump, OUTPUT);
  pinMode(pinIgnBypass, OUTPUT);
  pinMode(pinFan, OUTPUT);
  pinMode(pinStepperDir, OUTPUT);
  pinMode(pinStepperStep, OUTPUT);
  pinMode(pinStepperEnable, OUTPUT);
  pinMode(pinBoost, OUTPUT);
  pinMode(pinVVT_1, OUTPUT);

  inj1_pin_port = portOutputRegister(digitalPinToPort(pinInjector1));
  inj1_pin_mask = digitalPinToBitMask(pinInjector1);
  inj2_pin_port = portOutputRegister(digitalPinToPort(pinInjector2));
  inj2_pin_mask = digitalPinToBitMask(pinInjector2);
  inj3_pin_port = portOutputRegister(digitalPinToPort(pinInjector3));
  inj3_pin_mask = digitalPinToBitMask(pinInjector3);
  inj4_pin_port = portOutputRegister(digitalPinToPort(pinInjector4));
  inj4_pin_mask = digitalPinToBitMask(pinInjector4);
  inj5_pin_port = portOutputRegister(digitalPinToPort(pinInjector5));
  inj5_pin_mask = digitalPinToBitMask(pinInjector5);
  inj6_pin_port = portOutputRegister(digitalPinToPort(pinInjector6));
  inj6_pin_mask = digitalPinToBitMask(pinInjector6);
  inj7_pin_port = portOutputRegister(digitalPinToPort(pinInjector7));
  inj7_pin_mask = digitalPinToBitMask(pinInjector7);
  inj8_pin_port = portOutputRegister(digitalPinToPort(pinInjector8));
  inj8_pin_mask = digitalPinToBitMask(pinInjector8);

  ign1_pin_port = portOutputRegister(digitalPinToPort(pinCoil1));
  ign1_pin_mask = digitalPinToBitMask(pinCoil1);
  ign2_pin_port = portOutputRegister(digitalPinToPort(pinCoil2));
  ign2_pin_mask = digitalPinToBitMask(pinCoil2);
  ign3_pin_port = portOutputRegister(digitalPinToPort(pinCoil3));
  ign3_pin_mask = digitalPinToBitMask(pinCoil3);
  ign4_pin_port = portOutputRegister(digitalPinToPort(pinCoil4));
  ign4_pin_mask = digitalPinToBitMask(pinCoil4);
  ign5_pin_port = portOutputRegister(digitalPinToPort(pinCoil5));
  ign5_pin_mask = digitalPinToBitMask(pinCoil5);
  ign6_pin_port = portOutputRegister(digitalPinToPort(pinCoil6));
  ign6_pin_mask = digitalPinToBitMask(pinCoil6);
  ign7_pin_port = portOutputRegister(digitalPinToPort(pinCoil7));
  ign7_pin_mask = digitalPinToBitMask(pinCoil7);
  ign8_pin_port = portOutputRegister(digitalPinToPort(pinCoil8));
  ign8_pin_mask = digitalPinToBitMask(pinCoil8);

  tach_pin_port = portOutputRegister(digitalPinToPort(pinTachOut));
  tach_pin_mask = digitalPinToBitMask(pinTachOut);
  pump_pin_port = portOutputRegister(digitalPinToPort(pinFuelPump));
  pump_pin_mask = digitalPinToBitMask(pinFuelPump);


  #if defined(CORE_STM32)
    #ifndef ARDUINO_ARCH_STM32
      pinMode(pinMAP, INPUT_ANALOG);
      pinMode(pinO2, INPUT_ANALOG);
      pinMode(pinO2_2, INPUT_ANALOG);
      pinMode(pinTPS, INPUT_ANALOG);
      pinMode(pinIAT, INPUT_ANALOG);
      pinMode(pinCLT, INPUT_ANALOG);
      pinMode(pinBat, INPUT_ANALOG);
      pinMode(pinBaro, INPUT_ANALOG);
    #else
      pinMode(pinMAP, INPUT);
      pinMode(pinO2, INPUT);
      pinMode(pinO2_2, INPUT);
      pinMode(pinTPS, INPUT);
      pinMode(pinIAT, INPUT);
      pinMode(pinCLT, INPUT);
      pinMode(pinBat, INPUT);
      pinMode(pinBaro, INPUT);
    #endif
  #endif
  pinMode(pinTrigger, INPUT);
  pinMode(pinTrigger2, INPUT);
  pinMode(pinTrigger3, INPUT);


  if(configPage2.flexEnabled > 0)
  {
    pinMode(pinFlex, INPUT);
  }
  if(configPage6.launchEnabled > 0)
  {
    if (configPage6.lnchPullRes == true) { pinMode(pinLaunch, INPUT_PULLUP); }
    else { pinMode(pinLaunch, INPUT); }
  }
  if(configPage2.idleUpEnabled > 0)
  {
    if (configPage2.idleUpPolarity == 0) { pinMode(pinIdleUp, INPUT_PULLUP); }
    else { pinMode(pinIdleUp, INPUT); }
  }



  triggerPri_pin_port = portInputRegister(digitalPinToPort(pinTrigger));
  triggerPri_pin_mask = digitalPinToBitMask(pinTrigger);
  triggerSec_pin_port = portInputRegister(digitalPinToPort(pinTrigger2));
  triggerSec_pin_mask = digitalPinToBitMask(pinTrigger2);

}

void initialiseTriggers()
{
  byte triggerInterrupt = 0;
  byte triggerInterrupt2 = 1;

  #if defined(CORE_AVR)
    switch (pinTrigger) {

      case 2:
        triggerInterrupt = 0; break;
      case 3:
        triggerInterrupt = 1; break;
      case 18:
        triggerInterrupt = 5; break;
      case 19:
        triggerInterrupt = 4; break;
      case 20:
        triggerInterrupt = 3; break;
      case 21:
        triggerInterrupt = 2; break;
      default:
        triggerInterrupt = 0; break;
    }
  #else
    triggerInterrupt = pinTrigger;
  #endif

  #if defined(CORE_AVR)
    switch (pinTrigger2) {

      case 2:
        triggerInterrupt2 = 0; break;
      case 3:
        triggerInterrupt2 = 1; break;
      case 18:
        triggerInterrupt2 = 5; break;
      case 19:
        triggerInterrupt2 = 4; break;
      case 20:
        triggerInterrupt2 = 3; break;
      case 21:
        triggerInterrupt2 = 2; break;
      default:
        triggerInterrupt2 = 0; break;
    }
  #else
    triggerInterrupt2 = pinTrigger2;
  #endif

  pinMode(pinTrigger, INPUT);
  pinMode(pinTrigger2, INPUT);
  pinMode(pinTrigger3, INPUT);

  detachInterrupt(triggerInterrupt);
  detachInterrupt(triggerInterrupt2);

  primaryTriggerEdge = 0;
  secondaryTriggerEdge = 0;


  switch (configPage4.TrigPattern)
  {
    case 0:

      triggerSetup_missingTooth();
      triggerHandler = triggerPri_missingTooth;
      triggerSecondaryHandler = triggerSec_missingTooth;
      decoderHasSecondary = true;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;
      triggerSetEndTeeth = triggerSetEndTeeth_missingTooth;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);







      break;

    case 1:

      triggerSetup_BasicDistributor();
      triggerHandler = triggerPri_BasicDistributor;
      getRPM = getRPM_BasicDistributor;
      getCrankAngle = getCrankAngle_BasicDistributor;
      triggerSetEndTeeth = triggerSetEndTeeth_BasicDistributor;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case 2:
      triggerSetup_DualWheel();
      triggerHandler = triggerPri_DualWheel;
      triggerSecondaryHandler = triggerSec_DualWheel;
      decoderHasSecondary = true;
      getRPM = getRPM_DualWheel;
      getCrankAngle = getCrankAngle_DualWheel;
      triggerSetEndTeeth = triggerSetEndTeeth_DualWheel;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 3:
      triggerSetup_GM7X();
      triggerHandler = triggerPri_GM7X;
      getRPM = getRPM_GM7X;
      getCrankAngle = getCrankAngle_GM7X;
      triggerSetEndTeeth = triggerSetEndTeeth_GM7X;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, triggerHandler, RISING); }
      else { attachInterrupt(triggerInterrupt, triggerHandler, FALLING); }

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case 4:
      triggerSetup_4G63();
      triggerHandler = triggerPri_4G63;
      triggerSecondaryHandler = triggerSec_4G63;
      decoderHasSecondary = true;
      getRPM = getRPM_4G63;
      getCrankAngle = getCrankAngle_4G63;
      triggerSetEndTeeth = triggerSetEndTeeth_4G63;

      primaryTriggerEdge = CHANGE;
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 5:
      triggerSetup_24X();
      triggerHandler = triggerPri_24X;
      triggerSecondaryHandler = triggerSec_24X;
      decoderHasSecondary = true;
      getRPM = getRPM_24X;
      getCrankAngle = getCrankAngle_24X;
      triggerSetEndTeeth = triggerSetEndTeeth_24X;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = CHANGE;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 6:
      triggerSetup_Jeep2000();
      triggerHandler = triggerPri_Jeep2000;
      triggerSecondaryHandler = triggerSec_Jeep2000;
      decoderHasSecondary = true;
      getRPM = getRPM_Jeep2000;
      getCrankAngle = getCrankAngle_Jeep2000;
      triggerSetEndTeeth = triggerSetEndTeeth_Jeep2000;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = CHANGE;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 7:
      triggerSetup_Audi135();
      triggerHandler = triggerPri_Audi135;
      triggerSecondaryHandler = triggerSec_Audi135;
      decoderHasSecondary = true;
      getRPM = getRPM_Audi135;
      getCrankAngle = getCrankAngle_Audi135;
      triggerSetEndTeeth = triggerSetEndTeeth_Audi135;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = RISING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 8:
      triggerSetup_HondaD17();
      triggerHandler = triggerPri_HondaD17;
      triggerSecondaryHandler = triggerSec_HondaD17;
      decoderHasSecondary = true;
      getRPM = getRPM_HondaD17;
      getCrankAngle = getCrankAngle_HondaD17;
      triggerSetEndTeeth = triggerSetEndTeeth_HondaD17;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = CHANGE;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 9:
      triggerSetup_Miata9905();
      triggerHandler = triggerPri_Miata9905;
      triggerSecondaryHandler = triggerSec_Miata9905;
      decoderHasSecondary = true;
      getRPM = getRPM_Miata9905;
      getCrankAngle = getCrankAngle_Miata9905;
      triggerSetEndTeeth = triggerSetEndTeeth_Miata9905;


      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 10:
      triggerSetup_MazdaAU();
      triggerHandler = triggerPri_MazdaAU;
      triggerSecondaryHandler = triggerSec_MazdaAU;
      decoderHasSecondary = true;
      getRPM = getRPM_MazdaAU;
      getCrankAngle = getCrankAngle_MazdaAU;
      triggerSetEndTeeth = triggerSetEndTeeth_MazdaAU;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 11:
      triggerSetup_non360();
      triggerHandler = triggerPri_DualWheel;
      triggerSecondaryHandler = triggerSec_DualWheel;
      decoderHasSecondary = true;
      getRPM = getRPM_non360;
      getCrankAngle = getCrankAngle_non360;
      triggerSetEndTeeth = triggerSetEndTeeth_non360;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 12:
      triggerSetup_Nissan360();
      triggerHandler = triggerPri_Nissan360;
      triggerSecondaryHandler = triggerSec_Nissan360;
      decoderHasSecondary = true;
      getRPM = getRPM_Nissan360;
      getCrankAngle = getCrankAngle_Nissan360;
      triggerSetEndTeeth = triggerSetEndTeeth_Nissan360;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = CHANGE;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 13:
      triggerSetup_Subaru67();
      triggerHandler = triggerPri_Subaru67;
      triggerSecondaryHandler = triggerSec_Subaru67;
      decoderHasSecondary = true;
      getRPM = getRPM_Subaru67;
      getCrankAngle = getCrankAngle_Subaru67;
      triggerSetEndTeeth = triggerSetEndTeeth_Subaru67;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 14:
      triggerSetup_Daihatsu();
      triggerHandler = triggerPri_Daihatsu;
      getRPM = getRPM_Daihatsu;
      getCrankAngle = getCrankAngle_Daihatsu;
      triggerSetEndTeeth = triggerSetEndTeeth_Daihatsu;


      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case 15:
      triggerSetup_Harley();
      triggerHandler = triggerPri_Harley;

      getRPM = getRPM_Harley;
      getCrankAngle = getCrankAngle_Harley;
      triggerSetEndTeeth = triggerSetEndTeeth_Harley;

      primaryTriggerEdge = RISING;
      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case 16:

      triggerSetup_ThirtySixMinus222();
      triggerHandler = triggerPri_ThirtySixMinus222;
      triggerSecondaryHandler = triggerSec_ThirtySixMinus222;
      decoderHasSecondary = true;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;
      triggerSetEndTeeth = triggerSetEndTeeth_ThirtySixMinus222;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; }
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    default:
      triggerHandler = triggerPri_missingTooth;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, triggerHandler, RISING); }
      else { attachInterrupt(triggerInterrupt, triggerHandler, FALLING); }
      break;
  }
}
//# 1 "/home/developper/speeduino/speeduino/maths.ino"
#include "maths.h"
#include "globals.h"


int fastMap(unsigned long x, int in_min, int in_max, int out_min, int out_max)
{
  unsigned long a = (x - (unsigned long)in_min);
  int b = (out_max - out_min);
  int c = (in_max - in_min);
  int d = (ldiv( (a * (long)b) , (long)c ).quot);
  return d + out_min;


}







unsigned int divu10(unsigned int n)
{
  unsigned long q, r;
  q = (n >> 1) + (n >> 2);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q + (q >> 16);
  q = q >> 3;
  r = n - (q * 10);
  return q + ((r + 6) >> 4);
}


int divs10(long n)
{
  long q, r, p;
  p = n + ( (n>>31) & 9);
  q = (p >> 1) + (p >> 2);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q + (q >> 16);
  q = q >> 3;
  r = p - (q * 10);
  return q + ((r + 6) >> 4);
}


int divs100(long n)
{
  return (n / 100);
//# 62 "/home/developper/speeduino/speeduino/maths.ino"
}


unsigned long divu100(unsigned long n)
{

  unsigned long q, r;
  q = (n >> 1) + (n >> 3) + (n >> 6) - (n >> 10) +
  (n >> 12) + (n >> 13) - (n >> 16);
  q = q + (q >> 20);
  q = q >> 6;
  r = n - (q * 100);
  return q + ((r + 28) >> 7);
}



unsigned long percentage(byte x, unsigned long y)
{
  return (y * x) / 100;

}




inline long powint(int factor, unsigned int exponent)
{
   long product = 1;
   unsigned int counter = exponent;
   while ( (counter--) > 0) { product *= factor; }
   return product;
}
//# 1 "/home/developper/speeduino/speeduino/scheduledIO.ino"
#include "scheduledIO.h"
#include "scheduler.h"
#include "globals.h"
#include "timers.h"

inline void beginCoil1Charge() { digitalWrite(pinCoil1, coilHIGH); tachoOutputFlag = READY; }
inline void endCoil1Charge() { digitalWrite(pinCoil1, coilLOW); }

inline void beginCoil2Charge() { digitalWrite(pinCoil2, coilHIGH); tachoOutputFlag = READY; }
inline void endCoil2Charge() { digitalWrite(pinCoil2, coilLOW); }

inline void beginCoil3Charge() { digitalWrite(pinCoil3, coilHIGH); tachoOutputFlag = READY; }
inline void endCoil3Charge() { digitalWrite(pinCoil3, coilLOW); }

inline void beginCoil4Charge() { digitalWrite(pinCoil4, coilHIGH); tachoOutputFlag = READY; }
inline void endCoil4Charge() { digitalWrite(pinCoil4, coilLOW); }

inline void beginCoil5Charge() { digitalWrite(pinCoil5, coilHIGH); tachoOutputFlag = READY; }
inline void endCoil5Charge() { digitalWrite(pinCoil5, coilLOW); }

inline void beginCoil6Charge() { digitalWrite(pinCoil6, coilHIGH); tachoOutputFlag = READY; }
inline void endCoil6Charge() { digitalWrite(pinCoil6, coilLOW); }

inline void beginCoil7Charge() { digitalWrite(pinCoil7, coilHIGH); tachoOutputFlag = READY; }
inline void endCoil7Charge() { digitalWrite(pinCoil7, coilLOW); }

inline void beginCoil8Charge() { digitalWrite(pinCoil8, coilHIGH); tachoOutputFlag = READY; }
inline void endCoil8Charge() { digitalWrite(pinCoil8, coilLOW); }


inline void beginTrailingCoilCharge() { digitalWrite(pinCoil2, coilHIGH); }
inline void endTrailingCoilCharge1() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port |= ign3_pin_mask; }
inline void endTrailingCoilCharge2() { digitalWrite(pinCoil2, coilLOW); *ign3_pin_port &= ~(ign3_pin_mask); }


void beginCoil1and3Charge() { digitalWrite(pinCoil1, coilHIGH); digitalWrite(pinCoil3, coilHIGH); tachoOutputFlag = READY; }
void endCoil1and3Charge() { digitalWrite(pinCoil1, coilLOW); digitalWrite(pinCoil3, coilLOW); }
void beginCoil2and4Charge() { digitalWrite(pinCoil2, coilHIGH); digitalWrite(pinCoil4, coilHIGH); tachoOutputFlag = READY; }
void endCoil2and4Charge() { digitalWrite(pinCoil2, coilLOW); digitalWrite(pinCoil4, coilLOW); }

void nullCallback() { return; }
//# 1 "/home/developper/speeduino/speeduino/scheduler.ino"






#include "globals.h"
#include "scheduler.h"
#include "scheduledIO.h"


void initialiseSchedulers()
{
    nullSchedule.Status = OFF;

    fuelSchedule1.Status = OFF;
    fuelSchedule2.Status = OFF;
    fuelSchedule3.Status = OFF;
    fuelSchedule4.Status = OFF;
    fuelSchedule5.Status = OFF;
    fuelSchedule6.Status = OFF;
    fuelSchedule7.Status = OFF;
    fuelSchedule8.Status = OFF;

    fuelSchedule1.schedulesSet = 0;
    fuelSchedule2.schedulesSet = 0;
    fuelSchedule3.schedulesSet = 0;
    fuelSchedule4.schedulesSet = 0;
    fuelSchedule5.schedulesSet = 0;
    fuelSchedule6.schedulesSet = 0;
    fuelSchedule7.schedulesSet = 0;
    fuelSchedule8.schedulesSet = 0;

    fuelSchedule1.counter = &FUEL1_COUNTER;
    fuelSchedule1.compare = &FUEL1_COMPARE;
    fuelSchedule2.counter = &FUEL2_COUNTER;
    fuelSchedule2.compare = &FUEL2_COMPARE;
    fuelSchedule3.counter = &FUEL3_COUNTER;
    fuelSchedule3.compare = &FUEL3_COMPARE;
    fuelSchedule4.counter = &FUEL4_COUNTER;
    fuelSchedule4.compare = &FUEL4_COMPARE;
    #if (INJ_CHANNELS >= 5)
    fuelSchedule5.counter = &FUEL5_COUNTER;
    fuelSchedule5.compare = &FUEL5_COMPARE;
    #endif
    #if (INJ_CHANNELS >= 6)
    fuelSchedule6.counter = &FUEL6_COUNTER;
    fuelSchedule6.compare = &FUEL6_COMPARE;
    #endif
    #if (INJ_CHANNELS >= 7)
    fuelSchedule7.counter = &FUEL7_COUNTER;
    fuelSchedule7.compare = &FUEL7_COMPARE;
    #endif
    #if (INJ_CHANNELS >= 8)
    fuelSchedule8.counter = &FUEL8_COUNTER;
    fuelSchedule8.compare = &FUEL8_COMPARE;
    #endif

    ignitionSchedule1.Status = OFF;
    ignitionSchedule2.Status = OFF;
    ignitionSchedule3.Status = OFF;
    ignitionSchedule4.Status = OFF;
    ignitionSchedule5.Status = OFF;
    ignitionSchedule6.Status = OFF;
    ignitionSchedule7.Status = OFF;
    ignitionSchedule8.Status = OFF;

    IGN1_TIMER_ENABLE();
    IGN2_TIMER_ENABLE();
    IGN3_TIMER_ENABLE();
    IGN4_TIMER_ENABLE();

    ignitionSchedule1.schedulesSet = 0;
    ignitionSchedule2.schedulesSet = 0;
    ignitionSchedule3.schedulesSet = 0;
    ignitionSchedule4.schedulesSet = 0;
    ignitionSchedule5.schedulesSet = 0;
    ignitionSchedule6.schedulesSet = 0;
    ignitionSchedule7.schedulesSet = 0;
    ignitionSchedule8.schedulesSet = 0;

    ignitionSchedule1.counter = &IGN1_COUNTER;
    ignitionSchedule1.compare = &IGN1_COMPARE;
    ignitionSchedule2.counter = &IGN2_COUNTER;
    ignitionSchedule2.compare = &IGN2_COMPARE;
    ignitionSchedule3.counter = &IGN3_COUNTER;
    ignitionSchedule3.compare = &IGN3_COMPARE;
    ignitionSchedule4.counter = &IGN4_COUNTER;
    ignitionSchedule4.compare = &IGN4_COMPARE;
    #if (INJ_CHANNELS >= 5)
    ignitionSchedule5.counter = &IGN5_COUNTER;
    ignitionSchedule5.compare = &IGN5_COMPARE;
    #endif
    #if (INJ_CHANNELS >= 6)
    ignitionSchedule6.counter = &IGN6_COUNTER;
    ignitionSchedule6.compare = &IGN6_COMPARE;
    #endif
    #if (INJ_CHANNELS >= 7)
    ignitionSchedule7.counter = &IGN7_COUNTER;
    ignitionSchedule7.compare = &IGN7_COMPARE;
    #endif
    #if (INJ_CHANNELS >= 8)
    ignitionSchedule8.counter = &IGN8_COUNTER;
    ignitionSchedule8.compare = &IGN8_COMPARE;
    #endif

}
//# 120 "/home/developper/speeduino/speeduino/scheduler.ino"
void setFuelSchedule(struct Schedule *targetSchedule, unsigned long timeout, unsigned long duration)
{
  if(targetSchedule->Status != RUNNING)
  {



    targetSchedule->duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); }


    noInterrupts();
    targetSchedule->startCompare = *targetSchedule->counter + timeout_timer_compare;
    targetSchedule->endCompare = targetSchedule->startCompare + uS_TO_TIMER_COMPARE(duration);
    targetSchedule->Status = PENDING;
    targetSchedule->schedulesSet++;

    *targetSchedule->compare = targetSchedule->startCompare;
    interrupts();
    FUEL1_TIMER_ENABLE();
  }
  else
  {


    targetSchedule->nextStartCompare = *targetSchedule->counter + uS_TO_TIMER_COMPARE(timeout);
    targetSchedule->nextEndCompare = targetSchedule->nextStartCompare + uS_TO_TIMER_COMPARE(duration);
    targetSchedule->hasNextSchedule = true;
  }
}



void setFuelSchedule1(unsigned long timeout, unsigned long duration)
{

  if(timeout < MAX_TIMER_PERIOD_SLOW)
  {
    if(fuelSchedule1.Status != RUNNING)
    {



      fuelSchedule1.duration = duration;


      uint16_t timeout_timer_compare;
      if ((timeout+duration) > MAX_TIMER_PERIOD_SLOW) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD_SLOW - 1 - duration) ); }
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }


      noInterrupts();
      fuelSchedule1.startCompare = FUEL1_COUNTER + timeout_timer_compare;
      fuelSchedule1.endCompare = fuelSchedule1.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
      fuelSchedule1.Status = PENDING;
      fuelSchedule1.schedulesSet++;




      FUEL1_COMPARE = fuelSchedule1.startCompare;
      interrupts();
      FUEL1_TIMER_ENABLE();
    }
    else
    {


      noInterrupts();
      fuelSchedule1.nextStartCompare = FUEL1_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
      fuelSchedule1.nextEndCompare = fuelSchedule1.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
      fuelSchedule1.duration = duration;
      fuelSchedule1.hasNextSchedule = true;
      interrupts();
    }
  }
}

void setFuelSchedule2(unsigned long timeout, unsigned long duration)
{

  if(timeout < MAX_TIMER_PERIOD_SLOW)
  {
    if(fuelSchedule2.Status != RUNNING)
    {



      fuelSchedule2.duration = duration;


      uint16_t timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD_SLOW) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD - 1) ); }
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }


      noInterrupts();
      fuelSchedule2.startCompare = FUEL2_COUNTER + timeout_timer_compare;
      fuelSchedule2.endCompare = fuelSchedule2.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
      FUEL2_COMPARE = fuelSchedule2.startCompare;
      fuelSchedule2.Status = PENDING;
      fuelSchedule2.schedulesSet++;
      interrupts();
      FUEL2_TIMER_ENABLE();
    }
    else
    {


      fuelSchedule2.nextStartCompare = FUEL2_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
      fuelSchedule2.nextEndCompare = fuelSchedule2.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
      fuelSchedule2.hasNextSchedule = true;
    }
  }
}

void setFuelSchedule3(unsigned long timeout, unsigned long duration)
{

  if(timeout < MAX_TIMER_PERIOD_SLOW)
  {
    if(fuelSchedule3.Status != RUNNING)
    {



      fuelSchedule3.duration = duration;


      uint16_t timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD_SLOW) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD - 1) ); }
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }


      noInterrupts();
      fuelSchedule3.startCompare = FUEL3_COUNTER + timeout_timer_compare;
      fuelSchedule3.endCompare = fuelSchedule3.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
      FUEL3_COMPARE = fuelSchedule3.startCompare;
      fuelSchedule3.Status = PENDING;
      fuelSchedule3.schedulesSet++;
      interrupts();
      FUEL3_TIMER_ENABLE();
    }
    else
    {


      fuelSchedule3.nextStartCompare = FUEL3_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
      fuelSchedule3.nextEndCompare = fuelSchedule3.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
      fuelSchedule3.hasNextSchedule = true;
    }
  }
}

void setFuelSchedule4(unsigned long timeout, unsigned long duration)
{

  if(timeout < MAX_TIMER_PERIOD_SLOW)
  {
    if(fuelSchedule4.Status != RUNNING)
    {



      fuelSchedule4.duration = duration;


      uint16_t timeout_timer_compare;
      if (timeout > MAX_TIMER_PERIOD_SLOW) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD - 1) ); }
      else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }


      noInterrupts();
      fuelSchedule4.startCompare = FUEL4_COUNTER + timeout_timer_compare;
      fuelSchedule4.endCompare = fuelSchedule4.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
      FUEL4_COMPARE = fuelSchedule4.startCompare;
      fuelSchedule4.Status = PENDING;
      fuelSchedule4.schedulesSet++;
      interrupts();
      FUEL4_TIMER_ENABLE();
    }
    else
    {


      fuelSchedule4.nextStartCompare = FUEL4_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
      fuelSchedule4.nextEndCompare = fuelSchedule4.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
      fuelSchedule4.hasNextSchedule = true;
    }
  }
}

#if INJ_CHANNELS >= 5
void setFuelSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(fuelSchedule5.Status != RUNNING)
  {
    fuelSchedule5.StartCallback = startCallback;
    fuelSchedule5.EndCallback = endCallback;
    fuelSchedule5.duration = duration;




#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
    noInterrupts();
    fuelSchedule5.startCompare = TCNT3 + (timeout >> 4);
    fuelSchedule5.endCompare = fuelSchedule5.startCompare + (duration >> 4);
    fuelSchedule5.Status = PENDING;
    fuelSchedule5.schedulesSet++;
    OCR3A = setQueue(timer3Aqueue, &fuelSchedule1, &fuelSchedule5, TCNT3);
    interrupts();
    TIMSK3 |= (1 << OCIE3A);
#endif
  }
  else
  {


    fuelSchedule5.nextStartCompare = FUEL5_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
    fuelSchedule5.nextEndCompare = fuelSchedule5.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
    fuelSchedule5.hasNextSchedule = true;
  }
}
#endif

#if INJ_CHANNELS >= 6

void setFuelSchedule6(unsigned long timeout, unsigned long duration)
{
  if(fuelSchedule6.Status != RUNNING)
  {



    fuelSchedule6.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD_SLOW) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }


    noInterrupts();
    fuelSchedule6.startCompare = FUEL6_COUNTER + timeout_timer_compare;
    fuelSchedule6.endCompare = fuelSchedule6.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
    FUEL6_COMPARE = fuelSchedule6.startCompare;
    fuelSchedule6.Status = PENDING;
    fuelSchedule6.schedulesSet++;
    interrupts();
    FUEL6_TIMER_ENABLE();
  }
  else
  {


    fuelSchedule6.nextStartCompare = FUEL6_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
    fuelSchedule6.nextEndCompare = fuelSchedule6.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
    fuelSchedule6.hasNextSchedule = true;
  }
}
#endif

#if INJ_CHANNELS >= 7

void setFuelSchedule7(unsigned long timeout, unsigned long duration)
{
  if(fuelSchedule7.Status != RUNNING)
  {



    fuelSchedule7.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); }


    noInterrupts();
    fuelSchedule7.startCompare = FUEL7_COUNTER + timeout_timer_compare;
    fuelSchedule7.endCompare = fuelSchedule7.startCompare + uS_TO_TIMER_COMPARE(duration);
    FUEL7_COMPARE = fuelSchedule7.startCompare;
    fuelSchedule7.Status = PENDING;
    fuelSchedule7.schedulesSet++;
    interrupts();
    FUEL7_TIMER_ENABLE();
  }
  else
  {


    fuelSchedule7.nextStartCompare = FUEL7_COUNTER + uS_TO_TIMER_COMPARE(timeout);
    fuelSchedule7.nextEndCompare = fuelSchedule7.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
    fuelSchedule7.hasNextSchedule = true;
  }
}
#endif

#if INJ_CHANNELS >= 8

void setFuelSchedule8(unsigned long timeout, unsigned long duration)
{
  if(fuelSchedule8.Status != RUNNING)
  {



    fuelSchedule8.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); }


    noInterrupts();
    fuelSchedule8.startCompare = FUEL8_COUNTER + timeout_timer_compare;
    fuelSchedule8.endCompare = fuelSchedule8.startCompare + uS_TO_TIMER_COMPARE(duration);
    FUEL8_COMPARE = fuelSchedule8.startCompare;
    fuelSchedule8.Status = PENDING;
    fuelSchedule8.schedulesSet++;
    interrupts();
    FUEL8_TIMER_ENABLE();
  }
  else
  {


    fuelSchedule8.nextStartCompare = FUEL8_COUNTER + uS_TO_TIMER_COMPARE(timeout);
    fuelSchedule8.nextEndCompare = fuelSchedule8.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
    fuelSchedule8.hasNextSchedule = true;
  }
}
#endif


void setIgnitionSchedule1(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule1.Status != RUNNING)
  {
    ignitionSchedule1.StartCallback = startCallback;
    ignitionSchedule1.EndCallback = endCallback;
    ignitionSchedule1.duration = duration;


    uint16_t timeout_timer_compare;

    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); }

    noInterrupts();
    ignitionSchedule1.startCompare = IGN1_COUNTER + timeout_timer_compare;
    if(ignitionSchedule1.endScheduleSetByDecoder == false) { ignitionSchedule1.endCompare = ignitionSchedule1.startCompare + uS_TO_TIMER_COMPARE(duration); }
    IGN1_COMPARE = ignitionSchedule1.startCompare;
    ignitionSchedule1.Status = PENDING;
    ignitionSchedule1.schedulesSet++;
    interrupts();
    IGN1_TIMER_ENABLE();
  }
}

static inline void refreshIgnitionSchedule1(unsigned long timeToEnd)
{
  if( (ignitionSchedule1.Status == RUNNING) && (timeToEnd < ignitionSchedule1.duration) )


  {
    noInterrupts();
    ignitionSchedule1.endCompare = IGN1_COUNTER + uS_TO_TIMER_COMPARE(timeToEnd);
    IGN1_COMPARE = ignitionSchedule1.endCompare;
    interrupts();
  }
}

void setIgnitionSchedule2(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule2.Status != RUNNING)
  {
    ignitionSchedule2.StartCallback = startCallback;
    ignitionSchedule2.EndCallback = endCallback;
    ignitionSchedule2.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); }

    noInterrupts();
    ignitionSchedule2.startCompare = IGN2_COUNTER + timeout_timer_compare;
    if(ignitionSchedule2.endScheduleSetByDecoder == false) { ignitionSchedule2.endCompare = ignitionSchedule2.startCompare + uS_TO_TIMER_COMPARE(duration); }
    IGN2_COMPARE = ignitionSchedule2.startCompare;
    ignitionSchedule2.Status = PENDING;
    ignitionSchedule2.schedulesSet++;
    interrupts();
    IGN2_TIMER_ENABLE();
  }
}
void setIgnitionSchedule3(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule3.Status != RUNNING)
  {

    ignitionSchedule3.StartCallback = startCallback;
    ignitionSchedule3.EndCallback = endCallback;
    ignitionSchedule3.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE(timeout); }

    noInterrupts();
    ignitionSchedule3.startCompare = IGN3_COUNTER + timeout_timer_compare;
    if(ignitionSchedule3.endScheduleSetByDecoder == false) { ignitionSchedule3.endCompare = ignitionSchedule3.startCompare + uS_TO_TIMER_COMPARE(duration); }
    IGN3_COMPARE = ignitionSchedule3.startCompare;
    ignitionSchedule3.Status = PENDING;
    ignitionSchedule3.schedulesSet++;
    interrupts();
    IGN3_TIMER_ENABLE();
  }
  else
  {


    ignitionSchedule3.nextStartCompare = IGN3_COUNTER + uS_TO_TIMER_COMPARE(timeout);
    ignitionSchedule3.nextEndCompare = ignitionSchedule3.nextStartCompare + uS_TO_TIMER_COMPARE(duration);
    ignitionSchedule3.hasNextSchedule = true;
  }
}
void setIgnitionSchedule4(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule4.Status != RUNNING)
  {

    ignitionSchedule4.StartCallback = startCallback;
    ignitionSchedule4.EndCallback = endCallback;
    ignitionSchedule4.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }

    noInterrupts();
    ignitionSchedule4.startCompare = IGN4_COUNTER + timeout_timer_compare;
    if(ignitionSchedule4.endScheduleSetByDecoder == false) { ignitionSchedule4.endCompare = ignitionSchedule4.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration); }
    IGN4_COMPARE = ignitionSchedule4.startCompare;
    ignitionSchedule4.Status = PENDING;
    ignitionSchedule4.schedulesSet++;
    interrupts();
    IGN4_TIMER_ENABLE();
  }
  else
  {


    ignitionSchedule4.nextStartCompare = IGN4_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
    ignitionSchedule4.nextEndCompare = ignitionSchedule4.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
    ignitionSchedule4.hasNextSchedule = true;
  }
}
void setIgnitionSchedule5(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule5.Status != RUNNING)
  {

    ignitionSchedule5.StartCallback = startCallback;
    ignitionSchedule5.EndCallback = endCallback;
    ignitionSchedule5.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }

    noInterrupts();
    ignitionSchedule5.startCompare = IGN5_COUNTER + timeout_timer_compare;
    if(ignitionSchedule5.endScheduleSetByDecoder == false) { ignitionSchedule5.endCompare = ignitionSchedule5.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration); }
    IGN5_COMPARE = ignitionSchedule5.startCompare;
    ignitionSchedule5.Status = PENDING;
    ignitionSchedule5.schedulesSet++;
    interrupts();
    IGN5_TIMER_ENABLE();
  }
}
void setIgnitionSchedule6(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule6.Status != RUNNING)
  {

    ignitionSchedule6.StartCallback = startCallback;
    ignitionSchedule6.EndCallback = endCallback;
    ignitionSchedule6.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }

    noInterrupts();
    ignitionSchedule6.startCompare = IGN6_COUNTER + timeout_timer_compare;
    if(ignitionSchedule6.endScheduleSetByDecoder == false) { ignitionSchedule6.endCompare = ignitionSchedule6.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration); }
    IGN6_COMPARE = ignitionSchedule6.startCompare;
    ignitionSchedule6.Status = PENDING;
    ignitionSchedule6.schedulesSet++;
    interrupts();
    IGN6_TIMER_ENABLE();
  }
  else
  {


    ignitionSchedule6.nextStartCompare = IGN6_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
    ignitionSchedule6.nextEndCompare = ignitionSchedule6.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
    ignitionSchedule6.hasNextSchedule = true;
  }
}
void setIgnitionSchedule7(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule7.Status != RUNNING)
  {

    ignitionSchedule7.StartCallback = startCallback;
    ignitionSchedule7.EndCallback = endCallback;
    ignitionSchedule7.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }

    noInterrupts();
    ignitionSchedule7.startCompare = IGN4_COUNTER + timeout_timer_compare;
    if(ignitionSchedule7.endScheduleSetByDecoder == false) { ignitionSchedule7.endCompare = ignitionSchedule7.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration); }
    IGN7_COMPARE = ignitionSchedule7.startCompare;
    ignitionSchedule7.Status = PENDING;
    ignitionSchedule7.schedulesSet++;
    interrupts();
    IGN7_TIMER_ENABLE();
  }
  else
  {


    ignitionSchedule7.nextStartCompare = IGN7_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
    ignitionSchedule7.nextEndCompare = ignitionSchedule7.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
    ignitionSchedule7.hasNextSchedule = true;
  }
}
void setIgnitionSchedule8(void (*startCallback)(), unsigned long timeout, unsigned long duration, void(*endCallback)())
{
  if(ignitionSchedule8.Status != RUNNING)
  {

    ignitionSchedule8.StartCallback = startCallback;
    ignitionSchedule8.EndCallback = endCallback;
    ignitionSchedule8.duration = duration;


    uint16_t timeout_timer_compare;
    if (timeout > MAX_TIMER_PERIOD) { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW( (MAX_TIMER_PERIOD - 1) ); }
    else { timeout_timer_compare = uS_TO_TIMER_COMPARE_SLOW(timeout); }

    noInterrupts();
    ignitionSchedule8.startCompare = IGN8_COUNTER + timeout_timer_compare;
    if(ignitionSchedule8.endScheduleSetByDecoder == false) { ignitionSchedule8.endCompare = ignitionSchedule8.startCompare + uS_TO_TIMER_COMPARE_SLOW(duration); }
    IGN8_COMPARE = ignitionSchedule8.startCompare;
    ignitionSchedule8.Status = PENDING;
    ignitionSchedule8.schedulesSet++;
    interrupts();
    IGN8_TIMER_ENABLE();
  }
  else
  {


    ignitionSchedule8.nextStartCompare = IGN8_COUNTER + uS_TO_TIMER_COMPARE_SLOW(timeout);
    ignitionSchedule8.nextEndCompare = ignitionSchedule8.nextStartCompare + uS_TO_TIMER_COMPARE_SLOW(duration);
    ignitionSchedule8.hasNextSchedule = true;
  }
}






#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
ISR(TIMER3_COMPA_vect)
#else
static inline void fuelSchedule1Interrupt()
#endif
  {
    if (fuelSchedule1.Status == PENDING)
    {

      if (configPage2.injLayout == INJ_SEMISEQUENTIAL) { openInjector1and4(); }
      else { openInjector1(); }
      fuelSchedule1.Status = RUNNING;
      FUEL1_COMPARE = FUEL1_COUNTER + uS_TO_TIMER_COMPARE_SLOW(fuelSchedule1.duration);
    }
    else if (fuelSchedule1.Status == RUNNING)
    {

       if (configPage2.injLayout == INJ_SEMISEQUENTIAL) { closeInjector1and4(); }
       else { closeInjector1(); }
       fuelSchedule1.Status = OFF;
       fuelSchedule1.schedulesSet = 0;



       if(fuelSchedule1.hasNextSchedule == true)
       {
         FUEL1_COMPARE = fuelSchedule1.nextStartCompare;
         fuelSchedule1.endCompare = fuelSchedule1.nextEndCompare;
         fuelSchedule1.Status = PENDING;
         fuelSchedule1.schedulesSet = 1;
         fuelSchedule1.hasNextSchedule = false;
       }
       else { FUEL1_TIMER_DISABLE(); }
    }
    else if (fuelSchedule1.Status == OFF) { FUEL1_TIMER_DISABLE(); }
  }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
ISR(TIMER3_COMPB_vect)
#else
static inline void fuelSchedule2Interrupt()
#endif
  {
    if (fuelSchedule2.Status == PENDING)
    {

      if (configPage2.injLayout == INJ_SEMISEQUENTIAL) { openInjector2and3(); }
      else { openInjector2(); }
      fuelSchedule2.Status = RUNNING;
      FUEL2_COMPARE = FUEL2_COUNTER + uS_TO_TIMER_COMPARE_SLOW(fuelSchedule2.duration);
    }
    else if (fuelSchedule2.Status == RUNNING)
    {

       if (configPage2.injLayout == INJ_SEMISEQUENTIAL) { closeInjector2and3(); }
       else { closeInjector2(); }
       fuelSchedule2.Status = OFF;
       fuelSchedule2.schedulesSet = 0;


       if(fuelSchedule2.hasNextSchedule == true)
       {
         FUEL2_COMPARE = fuelSchedule2.nextStartCompare;
         fuelSchedule2.endCompare = fuelSchedule2.nextEndCompare;
         fuelSchedule2.Status = PENDING;
         fuelSchedule2.schedulesSet = 1;
         fuelSchedule2.hasNextSchedule = false;
       }
       else { FUEL2_TIMER_DISABLE(); }
    }
  }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
ISR(TIMER3_COMPC_vect)
#else
static inline void fuelSchedule3Interrupt()
#endif
  {
    if (fuelSchedule3.Status == PENDING)
    {


      if(channel5InjEnabled) { openInjector3and5(); }
      else { openInjector3(); }
      fuelSchedule3.Status = RUNNING;
      FUEL3_COMPARE = FUEL3_COUNTER + uS_TO_TIMER_COMPARE_SLOW(fuelSchedule3.duration);
    }
    else if (fuelSchedule3.Status == RUNNING)
    {


       if(channel5InjEnabled) { closeInjector3and5(); }
       else { closeInjector3and5(); }
       fuelSchedule3.Status = OFF;
       fuelSchedule3.schedulesSet = 0;


       if(fuelSchedule3.hasNextSchedule == true)
       {
         FUEL3_COMPARE = fuelSchedule3.nextStartCompare;
         fuelSchedule3.endCompare = fuelSchedule3.nextEndCompare;
         fuelSchedule3.Status = PENDING;
         fuelSchedule3.schedulesSet = 1;
         fuelSchedule3.hasNextSchedule = false;
       }
       else { FUEL3_TIMER_DISABLE(); }
    }
  }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
ISR(TIMER4_COMPB_vect)
#else
static inline void fuelSchedule4Interrupt()
#endif
  {
    if (fuelSchedule4.Status == PENDING)
    {

      openInjector4();
      fuelSchedule4.Status = RUNNING;
      FUEL4_COMPARE = FUEL4_COUNTER + uS_TO_TIMER_COMPARE_SLOW(fuelSchedule4.duration);
    }
    else if (fuelSchedule4.Status == RUNNING)
    {

       closeInjector4();
       fuelSchedule4.Status = OFF;
       fuelSchedule4.schedulesSet = 0;


       if(fuelSchedule4.hasNextSchedule == true)
       {
         FUEL4_COMPARE = fuelSchedule4.nextStartCompare;
         fuelSchedule4.endCompare = fuelSchedule4.nextEndCompare;
         fuelSchedule4.Status = PENDING;
         fuelSchedule4.schedulesSet = 1;
         fuelSchedule4.hasNextSchedule = false;
       }
       else { FUEL4_TIMER_DISABLE(); }
    }
  }

#if (INJ_CHANNELS >= 5)
#if defined(CORE_AVR)
ISR(TIMER1_COMPC_vect)
#else
static inline void fuelSchedule5Interrupt()
#endif
{
  if (fuelSchedule5.Status == PENDING)
  {
    openInjector5();
    fuelSchedule5.Status = RUNNING;
    FUEL5_COMPARE = fuelSchedule5.endCompare;
  }
  else if (fuelSchedule5.Status == RUNNING)
  {
     closeInjector5();
     fuelSchedule5.Status = OFF;
     fuelSchedule5.schedulesSet = 0;


     if(fuelSchedule5.hasNextSchedule == true)
     {
       FUEL5_COMPARE = fuelSchedule5.nextStartCompare;
       fuelSchedule5.endCompare = fuelSchedule5.nextEndCompare;
       fuelSchedule5.Status = PENDING;
       fuelSchedule5.schedulesSet = 1;
       fuelSchedule5.hasNextSchedule = false;
     }
     else { FUEL5_TIMER_DISABLE(); }
  }
}
#endif

#if (INJ_CHANNELS >= 6)
#if defined(CORE_AVR)
ISR(TIMER4_COMPA_vect)
#else
static inline void fuelSchedule6Interrupt()
#endif
{
  if (fuelSchedule6.Status == PENDING)
  {

    openInjector6();
    fuelSchedule6.Status = RUNNING;
    FUEL6_COMPARE = fuelSchedule6.endCompare;
  }
  else if (fuelSchedule6.Status == RUNNING)
  {

     closeInjector6();
     fuelSchedule6.Status = OFF;
     fuelSchedule6.schedulesSet = 0;


     if(fuelSchedule6.hasNextSchedule == true)
     {
       FUEL6_COMPARE = fuelSchedule6.nextStartCompare;
       fuelSchedule6.endCompare = fuelSchedule6.nextEndCompare;
       fuelSchedule6.Status = PENDING;
       fuelSchedule6.schedulesSet = 1;
       fuelSchedule6.hasNextSchedule = false;
     }
     else { FUEL6_TIMER_DISABLE(); }
  }
}
#endif

#if (INJ_CHANNELS >= 7)
#if defined(CORE_AVR)
ISR(TIMER5_COMPC_vect)
#else
static inline void fuelSchedule7Interrupt()
#endif
{
  if (fuelSchedule7.Status == PENDING)
  {
    openInjector7();
    fuelSchedule7.Status = RUNNING;
    FUEL7_COMPARE = fuelSchedule7.endCompare;
  }
  else if (fuelSchedule7.Status == RUNNING)
  {
     closeInjector7();
     fuelSchedule7.Status = OFF;
     fuelSchedule7.schedulesSet = 0;


     if(fuelSchedule7.hasNextSchedule == true)
     {
       FUEL7_COMPARE = fuelSchedule7.nextStartCompare;
       fuelSchedule7.endCompare = fuelSchedule7.nextEndCompare;
       fuelSchedule7.Status = PENDING;
       fuelSchedule7.schedulesSet = 1;
       fuelSchedule7.hasNextSchedule = false;
     }
     else { FUEL7_TIMER_DISABLE(); }
  }
}
#endif

#if (INJ_CHANNELS >= 8)
#if defined(CORE_AVR)
ISR(TIMER5_COMPB_vect)
#else
static inline void fuelSchedule8Interrupt()
#endif
{
  if (fuelSchedule8.Status == PENDING)
  {

    openInjector8();
    fuelSchedule8.Status = RUNNING;
    FUEL8_COMPARE = fuelSchedule8.endCompare;
  }
  else if (fuelSchedule8.Status == RUNNING)
  {

     closeInjector8();
     fuelSchedule8.Status = OFF;
     fuelSchedule8.schedulesSet = 0;


     if(fuelSchedule8.hasNextSchedule == true)
     {
       FUEL8_COMPARE = fuelSchedule8.nextStartCompare;
       fuelSchedule8.endCompare = fuelSchedule8.nextEndCompare;
       fuelSchedule8.Status = PENDING;
       fuelSchedule8.schedulesSet = 1;
       fuelSchedule8.hasNextSchedule = false;
     }
     else { FUEL8_TIMER_DISABLE(); }
  }
}
#endif

#if IGN_CHANNELS >= 1
#if defined(CORE_AVR)
ISR(TIMER5_COMPA_vect)
#else
static inline void ignitionSchedule1Interrupt()
#endif
  {
    if (ignitionSchedule1.Status == PENDING)
    {
      ignitionSchedule1.StartCallback();
      ignitionSchedule1.Status = RUNNING;
      ignitionSchedule1.startTime = micros();
      if(ignitionSchedule1.endScheduleSetByDecoder == true) { IGN1_COMPARE = ignitionSchedule1.endCompare; }
      else { IGN1_COMPARE = IGN1_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule1.duration); }
    }
    else if (ignitionSchedule1.Status == RUNNING)
    {
      ignitionSchedule1.EndCallback();

      ignitionSchedule1.Status = OFF;
      ignitionSchedule1.schedulesSet = 0;
      ignitionSchedule1.hasNextSchedule = false;
      ignitionSchedule1.endScheduleSetByDecoder = false;
      ignitionCount += 1;
      IGN1_TIMER_DISABLE();
    }
    else if (ignitionSchedule1.Status == OFF)
    {

      IGN1_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 2
#if defined(CORE_AVR)
ISR(TIMER5_COMPB_vect)
#else
static inline void ignitionSchedule2Interrupt()
#endif
  {
    if (ignitionSchedule2.Status == PENDING)
    {
      ignitionSchedule2.StartCallback();
      ignitionSchedule2.Status = RUNNING;
      ignitionSchedule2.startTime = micros();
      if(ignitionSchedule2.endScheduleSetByDecoder == true) { IGN2_COMPARE = ignitionSchedule2.endCompare; }
      else { IGN2_COMPARE = IGN2_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule2.duration); }
    }
    else if (ignitionSchedule2.Status == RUNNING)
    {
      ignitionSchedule2.Status = OFF;
      ignitionSchedule2.EndCallback();
      ignitionSchedule2.schedulesSet = 0;
      ignitionSchedule2.endScheduleSetByDecoder = false;
      ignitionCount += 1;
      IGN2_TIMER_DISABLE();
    }
    else if (ignitionSchedule2.Status == OFF)
    {

      IGN2_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 3
#if defined(CORE_AVR)
ISR(TIMER5_COMPC_vect)
#else
static inline void ignitionSchedule3Interrupt()
#endif
  {
    if (ignitionSchedule3.Status == PENDING)
    {
      ignitionSchedule3.StartCallback();
      ignitionSchedule3.Status = RUNNING;
      ignitionSchedule3.startTime = micros();
      if(ignitionSchedule3.endScheduleSetByDecoder == true) { IGN3_COMPARE = ignitionSchedule3.endCompare; }
      else { IGN3_COMPARE = IGN3_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule3.duration); }
    }
    else if (ignitionSchedule3.Status == RUNNING)
    {
       ignitionSchedule3.Status = OFF;
       ignitionSchedule3.EndCallback();
       ignitionSchedule3.schedulesSet = 0;
       ignitionSchedule3.endScheduleSetByDecoder = false;
       ignitionCount += 1;


       if(ignitionSchedule3.hasNextSchedule == true)
       {
         IGN3_COMPARE = ignitionSchedule3.nextStartCompare;
         ignitionSchedule3.endCompare = ignitionSchedule3.nextEndCompare;
         ignitionSchedule3.Status = PENDING;
         ignitionSchedule3.schedulesSet = 1;
         ignitionSchedule3.hasNextSchedule = false;
       }
       else { IGN3_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule3.Status == OFF)
    {

      IGN3_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 4
#if defined(CORE_AVR)
ISR(TIMER4_COMPA_vect)
#else
static inline void ignitionSchedule4Interrupt()
#endif
  {
    if (ignitionSchedule4.Status == PENDING)
    {
      ignitionSchedule4.StartCallback();
      ignitionSchedule4.Status = RUNNING;
      ignitionSchedule4.startTime = micros();
      IGN4_COMPARE = IGN4_COUNTER + uS_TO_TIMER_COMPARE_SLOW(ignitionSchedule4.duration);
    }
    else if (ignitionSchedule4.Status == RUNNING)
    {
       ignitionSchedule4.Status = OFF;
       ignitionSchedule4.EndCallback();
       ignitionSchedule4.schedulesSet = 0;
       ignitionSchedule4.endScheduleSetByDecoder = false;
       ignitionCount += 1;


       if(ignitionSchedule4.hasNextSchedule == true)
       {
         IGN4_COMPARE = ignitionSchedule4.nextStartCompare;
         ignitionSchedule4.endCompare = ignitionSchedule4.nextEndCompare;
         ignitionSchedule4.Status = PENDING;
         ignitionSchedule4.schedulesSet = 1;
         ignitionSchedule4.hasNextSchedule = false;
       }
       else { IGN4_TIMER_DISABLE(); }
    }
    else if (ignitionSchedule4.Status == OFF)
    {

      IGN4_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 5
#if defined(CORE_AVR)
ISR(TIMER1_COMPC_vect)
#else
static inline void ignitionSchedule5Interrupt()
#endif
  {
    if (ignitionSchedule5.Status == PENDING)
    {
      ignitionSchedule5.StartCallback();
      ignitionSchedule5.Status = RUNNING;
      ignitionSchedule5.startTime = micros();
      IGN5_COMPARE = IGN5_COUNTER + uS_TO_TIMER_COMPARE_SLOW(ignitionSchedule5.duration);
    }
    else if (ignitionSchedule5.Status == RUNNING)
    {
       ignitionSchedule5.Status = OFF;
       ignitionSchedule5.EndCallback();
       ignitionSchedule5.schedulesSet = 0;
       ignitionSchedule5.endScheduleSetByDecoder = false;
       ignitionCount += 1;
       IGN5_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 6
#if defined(CORE_AVR)
ISR(TIMER1_COMPC_vect)
#else
static inline void ignitionSchedule6Interrupt()
#endif
  {
    if (ignitionSchedule6.Status == PENDING)
    {
      ignitionSchedule6.StartCallback();
      ignitionSchedule6.Status = RUNNING;
      ignitionSchedule6.startTime = micros();
      IGN6_COMPARE = IGN6_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule6.duration);
    }
    else if (ignitionSchedule6.Status == RUNNING)
    {
       ignitionSchedule6.Status = OFF;
       ignitionSchedule6.EndCallback();
       ignitionSchedule6.schedulesSet = 0;
       ignitionSchedule6.endScheduleSetByDecoder = false;
       ignitionCount += 1;
       IGN6_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 7
#if defined(CORE_AVR)
ISR(TIMER1_COMPC_vect)
#else
static inline void ignitionSchedule7Interrupt()
#endif
  {
    if (ignitionSchedule7.Status == PENDING)
    {
      ignitionSchedule7.StartCallback();
      ignitionSchedule7.Status = RUNNING;
      ignitionSchedule7.startTime = micros();
      IGN7_COMPARE = IGN7_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule7.duration);
    }
    else if (ignitionSchedule7.Status == RUNNING)
    {
       ignitionSchedule7.Status = OFF;
       ignitionSchedule7.EndCallback();
       ignitionSchedule7.schedulesSet = 0;
       ignitionSchedule7.endScheduleSetByDecoder = false;
       ignitionCount += 1;
       IGN7_TIMER_DISABLE();
    }
  }
#endif

#if IGN_CHANNELS >= 8
#if defined(CORE_AVR)
ISR(TIMER1_COMPC_vect)
#else
static inline void ignitionSchedule8Interrupt()
#endif
  {
    if (ignitionSchedule8.Status == PENDING)
    {
      ignitionSchedule8.StartCallback();
      ignitionSchedule8.Status = RUNNING;
      ignitionSchedule8.startTime = micros();
      IGN8_COMPARE = IGN8_COUNTER + uS_TO_TIMER_COMPARE(ignitionSchedule8.duration);
    }
    else if (ignitionSchedule8.Status == RUNNING)
    {
       ignitionSchedule8.Status = OFF;
       ignitionSchedule8.EndCallback();
       ignitionSchedule8.schedulesSet = 0;
       ignitionSchedule8.endScheduleSetByDecoder = false;
       ignitionCount += 1;
       IGN8_TIMER_DISABLE();
    }
  }
#endif


#if defined(CORE_TEENSY)
void ftm0_isr(void)
{

  bool interrupt1 = (FTM0_C0SC & FTM_CSC_CHF);
  bool interrupt2 = (FTM0_C1SC & FTM_CSC_CHF);
  bool interrupt3 = (FTM0_C2SC & FTM_CSC_CHF);
  bool interrupt4 = (FTM0_C3SC & FTM_CSC_CHF);
  bool interrupt5 = (FTM0_C4SC & FTM_CSC_CHF);
  bool interrupt6 = (FTM0_C5SC & FTM_CSC_CHF);
  bool interrupt7 = (FTM0_C6SC & FTM_CSC_CHF);
  bool interrupt8 = (FTM0_C7SC & FTM_CSC_CHF);

  if(interrupt1) { FTM0_C0SC &= ~FTM_CSC_CHF; fuelSchedule1Interrupt(); }
  else if(interrupt2) { FTM0_C1SC &= ~FTM_CSC_CHF; fuelSchedule2Interrupt(); }
  else if(interrupt3) { FTM0_C2SC &= ~FTM_CSC_CHF; fuelSchedule3Interrupt(); }
  else if(interrupt4) { FTM0_C3SC &= ~FTM_CSC_CHF; fuelSchedule4Interrupt(); }
  else if(interrupt5) { FTM0_C4SC &= ~FTM_CSC_CHF; ignitionSchedule1Interrupt(); }
  else if(interrupt6) { FTM0_C5SC &= ~FTM_CSC_CHF; ignitionSchedule2Interrupt(); }
  else if(interrupt7) { FTM0_C6SC &= ~FTM_CSC_CHF; ignitionSchedule3Interrupt(); }
  else if(interrupt8) { FTM0_C7SC &= ~FTM_CSC_CHF; ignitionSchedule4Interrupt(); }

}
void ftm3_isr(void)
{

#if (INJ_CHANNELS >= 5)
  bool interrupt1 = (FTM3_C0SC & FTM_CSC_CHF);
  if(interrupt1) { FTM3_C0SC &= ~FTM_CSC_CHF; fuelSchedule5Interrupt(); }
#endif
#if (INJ_CHANNELS >= 6)
  bool interrupt2 = (FTM3_C1SC & FTM_CSC_CHF);
  if(interrupt2) { FTM3_C1SC &= ~FTM_CSC_CHF; fuelSchedule6Interrupt(); }
#endif
#if (INJ_CHANNELS >= 7)
  bool interrupt3 = (FTM3_C2SC & FTM_CSC_CHF);
  if(interrupt3) { FTM3_C2SC &= ~FTM_CSC_CHF; fuelSchedule7Interrupt(); }
#endif
#if (INJ_CHANNELS >= 8)
  bool interrupt4 = (FTM3_C3SC & FTM_CSC_CHF);
  if(interrupt4) { FTM3_C3SC &= ~FTM_CSC_CHF; fuelSchedule8Interrupt(); }
#endif
#if (IGN_CHANNELS >= 5)
  bool interrupt5 = (FTM3_C4SC & FTM_CSC_CHF);
  if(interrupt5) { FTM3_C4SC &= ~FTM_CSC_CHF; ignitionSchedule5Interrupt(); }
#endif
#if (IGN_CHANNELS >= 6)
  bool interrupt6 = (FTM3_C5SC & FTM_CSC_CHF);
  if(interrupt6) { FTM3_C5SC &= ~FTM_CSC_CHF; ignitionSchedule6Interrupt(); }
#endif
#if (IGN_CHANNELS >= 7)
  bool interrupt7 = (FTM3_C6SC & FTM_CSC_CHF);
  if(interrupt7) { FTM3_C6SC &= ~FTM_CSC_CHF; ignitionSchedule7Interrupt(); }
#endif
#if (IGN_CHANNELS >= 8)
  bool interrupt8 = (FTM3_C7SC & FTM_CSC_CHF);
  if(interrupt8) { FTM3_C7SC &= ~FTM_CSC_CHF; ignitionSchedule8Interrupt(); }
#endif

}
#endif
//# 1 "/home/developper/speeduino/speeduino/sensors.ino"





#include "sensors.h"
#include "crankMaths.h"
#include "globals.h"
#include "maths.h"
#include "storage.h"
#include "comms.h"
#include "idle.h"

void initialiseADC()
{
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)

  #if defined(ANALOG_ISR)



    noInterrupts();

    ADCSRB = 0x00;
    ADMUX = 0x40;


    #define ADFR 5
    BIT_SET(ADCSRA,ADFR);
    BIT_SET(ADCSRA,ADIE);
    BIT_CLEAR(ADCSRA,ADIF);


    BIT_SET(ADCSRA,ADPS2);
    BIT_SET(ADCSRA,ADPS1);
    BIT_SET(ADCSRA,ADPS0);

    BIT_SET(ADCSRA,ADEN);

    interrupts();
    BIT_SET(ADCSRA,ADSC);

  #else



     BIT_SET(ADCSRA,ADPS2);
     BIT_CLEAR(ADCSRA,ADPS1);
     BIT_CLEAR(ADCSRA,ADPS0);
  #endif
#elif defined(ARDUINO_ARCH_STM32)
  analogReadResolution(12);
#endif
  MAPcurRev = 0;
  MAPcount = 0;
  MAPrunningValue = 0;


  auxIsEnabled = false;
  for (byte AuxinChan = 0; AuxinChan <16 ; AuxinChan++)
  {
    currentStatus.current_caninchannel = AuxinChan;
    if (((configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 4)
    && ((configPage9.enable_secondarySerial == 1) || ((configPage9.enable_intcan == 1) && (configPage9.intcan_available == 1))))
    {

      auxIsEnabled = true;
    }
    else if ((((configPage9.enable_secondarySerial == 1) || ((configPage9.enable_intcan == 1) && (configPage9.intcan_available == 1))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 8)
            || (((configPage9.enable_secondarySerial == 0) && ( (configPage9.enable_intcan == 1) && (configPage9.intcan_available == 0) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 2)
            || (((configPage9.enable_secondarySerial == 0) && (configPage9.enable_intcan == 0)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 2)))
    {
      byte pinNumber = (configPage9.Auxinpina[currentStatus.current_caninchannel]&127);
      if( pinIsUsed(pinNumber) )
      {


      }
      else
      {

        pinMode( pinNumber, INPUT);

        auxIsEnabled = true;
      }
    }
    else if ((((configPage9.enable_secondarySerial == 1) || ((configPage9.enable_intcan == 1) && (configPage9.intcan_available == 1))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 12)
            || (((configPage9.enable_secondarySerial == 0) && ( (configPage9.enable_intcan == 1) && (configPage9.intcan_available == 0) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 3)
            || (((configPage9.enable_secondarySerial == 0) && (configPage9.enable_intcan == 0)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 3)))
    {
       byte pinNumber = (configPage9.Auxinpinb[currentStatus.current_caninchannel]&127);
       if( pinIsUsed(pinNumber) )
       {


       }
       else
       {

         pinMode( pinNumber, INPUT);

         auxIsEnabled = true;
       }

    }
  }




  if(configPage4.ADCFILTER_TPS > 240) { configPage4.ADCFILTER_TPS = 50; writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_CLT > 240) { configPage4.ADCFILTER_CLT = 180; writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_IAT > 240) { configPage4.ADCFILTER_IAT = 180; writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_O2 > 240) { configPage4.ADCFILTER_O2 = 100; writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_BAT > 240) { configPage4.ADCFILTER_BAT = 128; writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_MAP > 240) { configPage4.ADCFILTER_MAP = 20; writeConfig(ignSetPage); }
  if(configPage4.ADCFILTER_BARO > 240) { configPage4.ADCFILTER_BARO = 64; writeConfig(ignSetPage); }

}

static inline void instanteneousMAPReading()
{

  MAPlast = currentStatus.MAP;
  MAPlast_time = MAP_time;
  MAP_time = micros();

  unsigned int tempReading;

  #if defined(ANALOG_ISR_MAP)
    tempReading = AnChannel[pinMAP-A0];
  #else
    tempReading = analogRead(pinMAP);
    tempReading = analogRead(pinMAP);
  #endif

  if( (tempReading >= VALID_MAP_MAX) || (tempReading <= VALID_MAP_MIN) ) { mapErrorCount += 1; }
  else { mapErrorCount = 0; }


  if(initialisationComplete == true) { currentStatus.mapADC = ADC_FILTER(tempReading, configPage4.ADCFILTER_MAP, currentStatus.mapADC); }
  else { currentStatus.mapADC = tempReading; }

  currentStatus.MAP = fastMap10Bit(currentStatus.mapADC, configPage2.mapMin, configPage2.mapMax);
  if(currentStatus.MAP < 0) { currentStatus.MAP = 0; }

}

static inline void readMAP()
{
  unsigned int tempReading;

  switch(configPage2.mapSample)
  {
    case 0:

      instanteneousMAPReading();
      break;

    case 1:


      if ( (currentStatus.RPM > 0) && (currentStatus.hasSync == true) )
      {
        if( (MAPcurRev == currentStatus.startRevolutions) || (MAPcurRev == (currentStatus.startRevolutions+1)) )
        {
          #if defined(ANALOG_ISR_MAP)
            tempReading = AnChannel[pinMAP-A0];
          #else
            tempReading = analogRead(pinMAP);
            tempReading = analogRead(pinMAP);
          #endif


          if( (tempReading < VALID_MAP_MAX) && (tempReading > VALID_MAP_MIN) )
          {
            currentStatus.mapADC = ADC_FILTER(tempReading, configPage4.ADCFILTER_MAP, currentStatus.mapADC);
            MAPrunningValue += currentStatus.mapADC;
            MAPcount++;
          }
          else { mapErrorCount += 1; }


          if(configPage6.useEMAP == true)
          {
            tempReading = analogRead(pinEMAP);
            tempReading = analogRead(pinEMAP);


            if( (tempReading < VALID_MAP_MAX) && (tempReading > VALID_MAP_MIN) )
            {
              currentStatus.EMAPADC = ADC_FILTER(tempReading, configPage4.ADCFILTER_MAP, currentStatus.EMAPADC);
              EMAPrunningValue += currentStatus.EMAPADC;
            }
            else { mapErrorCount += 1; }
          }
        }
        else
        {


          if( (MAPrunningValue != 0) && (MAPcount != 0) )
          {

            MAPlast = currentStatus.MAP;
            MAPlast_time = MAP_time;
            MAP_time = micros();

            currentStatus.mapADC = ldiv(MAPrunningValue, MAPcount).quot;
            currentStatus.MAP = fastMap10Bit(currentStatus.mapADC, configPage2.mapMin, configPage2.mapMax);
            if(currentStatus.MAP < 0) { currentStatus.MAP = 0; }


            if(configPage6.useEMAP == true)
            {
              currentStatus.EMAPADC = ldiv(EMAPrunningValue, MAPcount).quot;
              currentStatus.EMAP = fastMap10Bit(currentStatus.EMAPADC, configPage2.EMAPMin, configPage2.EMAPMax);
              if(currentStatus.EMAP < 0) { currentStatus.EMAP = 0; }
            }
          }
          else { instanteneousMAPReading(); }
          MAPcurRev = currentStatus.startRevolutions;
          MAPrunningValue = 0;
          EMAPrunningValue = 0;
          MAPcount = 0;
        }
      }
      else { instanteneousMAPReading(); }
      break;

    case 2:

      if (currentStatus.RPM > 0 )
      {
        if( (MAPcurRev == currentStatus.startRevolutions) || (MAPcurRev == (currentStatus.startRevolutions+1)) )
        {
          #if defined(ANALOG_ISR_MAP)
            tempReading = AnChannel[pinMAP-A0];
          #else
            tempReading = analogRead(pinMAP);
            tempReading = analogRead(pinMAP);
          #endif

          if( (tempReading < VALID_MAP_MAX) && (tempReading > VALID_MAP_MIN) )
          {
            if( (unsigned long)tempReading < MAPrunningValue ) { MAPrunningValue = (unsigned long)tempReading; }
          }
          else { mapErrorCount += 1; }
        }
        else
        {



          MAPlast = currentStatus.MAP;
          MAPlast_time = MAP_time;
          MAP_time = micros();

          currentStatus.mapADC = MAPrunningValue;
          currentStatus.MAP = fastMap10Bit(currentStatus.mapADC, configPage2.mapMin, configPage2.mapMax);
          if(currentStatus.MAP < 0) { currentStatus.MAP = 0; }
          MAPcurRev = currentStatus.startRevolutions;
          MAPrunningValue = 1023;
        }
      }
      else { instanteneousMAPReading(); }
      break;

    default:

    instanteneousMAPReading();
    break;
  }
}

void readTPS()
{
  TPSlast = currentStatus.TPS;
  TPSlast_time = TPS_time;
  #if defined(ANALOG_ISR)
    byte tempTPS = fastMap1023toX(AnChannel[pinTPS-A0], 255);
  #else
    analogRead(pinTPS);
    byte tempTPS = fastMap1023toX(analogRead(pinTPS), 255);
  #endif
  currentStatus.tpsADC = ADC_FILTER(tempTPS, configPage4.ADCFILTER_TPS, currentStatus.tpsADC);

  byte tempADC = currentStatus.tpsADC;

  if(configPage2.tpsMax > configPage2.tpsMin)
  {

    if (currentStatus.tpsADC < configPage2.tpsMin) { tempADC = configPage2.tpsMin; }
    else if(currentStatus.tpsADC > configPage2.tpsMax) { tempADC = configPage2.tpsMax; }
    currentStatus.TPS = map(tempADC, configPage2.tpsMin, configPage2.tpsMax, 0, 100);
  }
  else
  {



    tempADC = 255 - currentStatus.tpsADC;


    if (tempADC > configPage2.tpsMin) { tempADC = configPage2.tpsMin; }
    else if(tempADC < configPage2.tpsMax) { tempADC = configPage2.tpsMax; }
    currentStatus.TPS = map(tempADC, configPage2.tpsMax, configPage2.tpsMin, 0, 100);
  }

  TPS_time = micros();
}

void readCLT(bool useFilter)
{
  unsigned int tempReading;
  #if defined(ANALOG_ISR)
    tempReading = fastMap1023toX(AnChannel[pinCLT-A0], 511);
  #else
    tempReading = analogRead(pinCLT);
    tempReading = fastMap1023toX(analogRead(pinCLT), 511);
  #endif

  if(useFilter == true) { currentStatus.cltADC = ADC_FILTER(tempReading, configPage4.ADCFILTER_CLT, currentStatus.cltADC); }
  else { currentStatus.cltADC = tempReading; }

  currentStatus.coolant = cltCalibrationTable[currentStatus.cltADC] - CALIBRATION_TEMPERATURE_OFFSET;
}

void readIAT()
{
  unsigned int tempReading;
  #if defined(ANALOG_ISR)
    tempReading = fastMap1023toX(AnChannel[pinIAT-A0], 511);
  #else
    tempReading = analogRead(pinIAT);
    tempReading = fastMap1023toX(analogRead(pinIAT), 511);
  #endif
  currentStatus.iatADC = ADC_FILTER(tempReading, configPage4.ADCFILTER_IAT, currentStatus.iatADC);
  currentStatus.IAT = iatCalibrationTable[currentStatus.iatADC] - CALIBRATION_TEMPERATURE_OFFSET;
}

void readBaro()
{
  if ( configPage6.useExtBaro != 0 )
  {
    int tempReading;

    #if defined(ANALOG_ISR_MAP)
      tempReading = AnChannel[pinBaro-A0];
    #else
      tempReading = analogRead(pinBaro);
      tempReading = analogRead(pinBaro);
    #endif

    currentStatus.baroADC = ADC_FILTER(tempReading, configPage4.ADCFILTER_BARO, currentStatus.baroADC);

    currentStatus.baro = fastMap10Bit(currentStatus.baroADC, configPage2.baroMin, configPage2.baroMax);
  }
}

void readO2()
{
  unsigned int tempReading;
  #if defined(ANALOG_ISR)
    tempReading = fastMap1023toX(AnChannel[pinO2-A0], 511);
  #else
    tempReading = analogRead(pinO2);
    tempReading = fastMap1023toX(analogRead(pinO2), 511);
  #endif
  currentStatus.O2ADC = ADC_FILTER(tempReading, configPage4.ADCFILTER_O2, currentStatus.O2ADC);
  currentStatus.O2 = o2CalibrationTable[currentStatus.O2ADC];
}

void readO2_2()
{


  unsigned int tempReading;
  #if defined(ANALOG_ISR)
    tempReading = fastMap1023toX(AnChannel[pinO2_2-A0], 511);
  #else
    tempReading = analogRead(pinO2_2);
    tempReading = fastMap1023toX(analogRead(pinO2_2), 511);
  #endif
  currentStatus.O2_2ADC = ADC_FILTER(tempReading, configPage4.ADCFILTER_O2, currentStatus.O2_2ADC);
  currentStatus.O2_2 = o2CalibrationTable[currentStatus.O2_2ADC];
}

void readBat()
{
  unsigned int tempReading;
  #if defined(ANALOG_ISR)
    tempReading = fastMap1023toX(AnChannel[pinBat-A0], 245);
  #else
    tempReading = analogRead(pinBat);
    tempReading = fastMap1023toX(analogRead(pinBat), 245);
  #endif




  if( (currentStatus.battery10 < 55) && (tempReading > 70) && (currentStatus.RPM == 0) )
  {

    fpPrimeTime = currentStatus.secl;
    fpPrimed = false;
    FUEL_PUMP_ON();


    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL) )
    {
      initialiseIdle();
    }
  }

  currentStatus.battery10 = ADC_FILTER(tempReading, configPage4.ADCFILTER_BAT, currentStatus.battery10);
}





void flexPulse()
{
  ++flexCounter;
}





void knockPulse()
{

  if(knockCounter == 0)
  {

    knockStartTime = micros();
    knockCounter = 1;
  }
  else { ++knockCounter; }

}

uint16_t readAuxanalog(uint8_t analogPin)
{

  unsigned int tempReading;
  #if defined(ANALOG_ISR)
    tempReading = fastMap1023toX(AnChannel[analogPin-A0], 1023);
  #else
    tempReading = analogRead(analogPin);
    tempReading = analogRead(analogPin);

  #endif
  return tempReading;
}

uint16_t readAuxdigital(uint8_t digitalPin)
{

  unsigned int tempReading;
  tempReading = digitalRead(digitalPin);
  return tempReading;
}
//# 1 "/home/developper/speeduino/speeduino/storage.ino"







#include "globals.h"
#include "table.h"
#include "comms.h"
#include EEPROM_LIB_H
#include "storage.h"

void writeAllConfig()
{
  writeConfig(veSetPage);
  if (eepromWritesPending == false) { writeConfig(veMapPage); }
  if (eepromWritesPending == false) { writeConfig(ignMapPage); }
  if (eepromWritesPending == false) { writeConfig(ignSetPage); }
  if (eepromWritesPending == false) { writeConfig(afrMapPage); }
  if (eepromWritesPending == false) { writeConfig(afrSetPage); }
  if (eepromWritesPending == false) { writeConfig(boostvvtPage); }
  if (eepromWritesPending == false) { writeConfig(seqFuelPage); }
  if (eepromWritesPending == false) { writeConfig(canbusPage); }
  if (eepromWritesPending == false) { writeConfig(warmupPage); }
}






void writeConfig(byte tableNum)
{





  int offset;
  int i, z, y;
  int writeCounter = 0;
  byte newVal;

  byte* pnt_configPage;

  switch(tableNum)
  {
    case veMapPage:




      if(EEPROM.read(EEPROM_CONFIG1_XSIZE) != fuelTable.xSize) { EEPROM.write(EEPROM_CONFIG1_XSIZE, fuelTable.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG1_YSIZE) != fuelTable.ySize) { EEPROM.write(EEPROM_CONFIG1_YSIZE, fuelTable.ySize); writeCounter++; }
      for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG1_MAP;
        if( EEPROM.read(x) != (fuelTable.values[15-(offset/16)][offset%16]) ) { EEPROM.write(x, fuelTable.values[15-(offset/16)][offset%16]); writeCounter++; }
      }


      for(int x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG1_XBINS;
        if( EEPROM.read(x) != (byte(fuelTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) ) { EEPROM.write(x, byte(fuelTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; }
      }

      for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG1_YBINS;
        EEPROM.update(x, fuelTable.axisY[offset] / TABLE_LOAD_MULTIPLIER);
      }
      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }
      break;


    case veSetPage:




      pnt_configPage = (byte *)&configPage2;
      for(int x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case ignMapPage:





      if(EEPROM.read(EEPROM_CONFIG3_XSIZE) != ignitionTable.xSize) { EEPROM.write(EEPROM_CONFIG3_XSIZE,ignitionTable.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG3_YSIZE) != ignitionTable.ySize) { EEPROM.write(EEPROM_CONFIG3_YSIZE,ignitionTable.ySize); writeCounter++; }

      for(int x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG3_MAP;
        newVal = ignitionTable.values[15-(offset/16)][offset%16];
        if(EEPROM.read(x) != newVal) { EEPROM.write(x, newVal); writeCounter++; }
      }

      for(int x=EEPROM_CONFIG3_XBINS; x<EEPROM_CONFIG3_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG3_XBINS;
        newVal = ignitionTable.axisX[offset]/TABLE_RPM_MULTIPLIER;
        if(EEPROM.read(x) != newVal) { EEPROM.write(x, newVal); writeCounter++; }
      }

      for(int x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG3_YBINS;
        newVal = ignitionTable.axisY[offset]/TABLE_LOAD_MULTIPLIER;
        if(EEPROM.read(x) != newVal) { EEPROM.write(x, newVal); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case ignSetPage:




      pnt_configPage = (byte *)&configPage4;
      for(int x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG4_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG4_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case afrMapPage:





      if(EEPROM.read(EEPROM_CONFIG5_XSIZE) != afrTable.xSize) { EEPROM.write(EEPROM_CONFIG5_XSIZE,afrTable.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG5_YSIZE) != afrTable.ySize) { EEPROM.write(EEPROM_CONFIG5_YSIZE,afrTable.ySize); writeCounter++; }

      for(int x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG5_MAP;
        if(EEPROM.read(x) != (afrTable.values[15-(offset/16)][offset%16]) ) { EEPROM.write(x, afrTable.values[15-(offset/16)][offset%16]); writeCounter++; }
      }

      for(int x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG5_XBINS;
        if(EEPROM.read(x) != byte(afrTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(x, byte(afrTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; }
      }

      for(int x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG5_YBINS;
        EEPROM.update(x, afrTable.axisY[offset]/TABLE_LOAD_MULTIPLIER);
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case afrSetPage:




      pnt_configPage = (byte *)&configPage6;
      for(int x=EEPROM_CONFIG6_START; x<EEPROM_CONFIG6_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG6_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG6_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case boostvvtPage:





      if(EEPROM.read(EEPROM_CONFIG7_XSIZE1) != boostTable.xSize) { EEPROM.write(EEPROM_CONFIG7_XSIZE1,boostTable.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG7_YSIZE1) != boostTable.ySize) { EEPROM.write(EEPROM_CONFIG7_YSIZE1,boostTable.ySize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG7_XSIZE2) != vvtTable.xSize) { EEPROM.write(EEPROM_CONFIG7_XSIZE2,vvtTable.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG7_YSIZE2) != vvtTable.ySize) { EEPROM.write(EEPROM_CONFIG7_YSIZE2,vvtTable.ySize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG7_XSIZE3) != stagingTable.xSize) { EEPROM.write(EEPROM_CONFIG7_XSIZE3,stagingTable.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG7_YSIZE3) != stagingTable.ySize) { EEPROM.write(EEPROM_CONFIG7_YSIZE3,stagingTable.ySize); writeCounter++; }

      y = EEPROM_CONFIG7_MAP2;
      z = EEPROM_CONFIG7_MAP3;
      for(int x=EEPROM_CONFIG7_MAP1; x<EEPROM_CONFIG7_XBINS1; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG7_MAP1;
        if(EEPROM.read(x) != (boostTable.values[7-(offset/8)][offset%8]) ) { EEPROM.write(x, boostTable.values[7-(offset/8)][offset%8]); writeCounter++; }
        offset = y - EEPROM_CONFIG7_MAP2;
        if(EEPROM.read(y) != (vvtTable.values[7-(offset/8)][offset%8]) ) { EEPROM.write(y, vvtTable.values[7-(offset/8)][offset%8]); writeCounter++; }
        offset = z - EEPROM_CONFIG7_MAP3;
        if(EEPROM.read(z) != (stagingTable.values[7-(offset/8)][offset%8]) ) { EEPROM.write(z, stagingTable.values[7-(offset/8)][offset%8]); writeCounter++; }
        y++;
        z++;
      }

      y = EEPROM_CONFIG7_XBINS2;
      z = EEPROM_CONFIG7_XBINS3;
      for(int x=EEPROM_CONFIG7_XBINS1; x<EEPROM_CONFIG7_YBINS1; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG7_XBINS1;
        if(EEPROM.read(x) != byte(boostTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(x, byte(boostTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; }
        offset = y - EEPROM_CONFIG7_XBINS2;
        if(EEPROM.read(y) != byte(vvtTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(y, byte(vvtTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; }
        offset = z - EEPROM_CONFIG7_XBINS3;
        if(EEPROM.read(z) != byte(stagingTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(z, byte(stagingTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; }
        y++;
        z++;
      }

      y=EEPROM_CONFIG7_YBINS2;
      z=EEPROM_CONFIG7_YBINS3;
      for(int x=EEPROM_CONFIG7_YBINS1; x<EEPROM_CONFIG7_XSIZE2; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG7_YBINS1;
        if(EEPROM.read(x) != boostTable.axisY[offset]) { EEPROM.write(x, boostTable.axisY[offset]); writeCounter++; }
        offset = y - EEPROM_CONFIG7_YBINS2;
        if(EEPROM.read(y) != vvtTable.axisY[offset]) { EEPROM.write(y, vvtTable.axisY[offset]); writeCounter++; }
        offset = z - EEPROM_CONFIG7_YBINS3;
        if(EEPROM.read(z) != byte(stagingTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)) { EEPROM.write(z, byte(stagingTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; }
        y++;
        z++;
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case seqFuelPage:





      if(EEPROM.read(EEPROM_CONFIG8_XSIZE1) != trim1Table.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE1,trim1Table.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG8_YSIZE1) != trim1Table.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE1,trim1Table.ySize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG8_XSIZE2) != trim2Table.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE2,trim2Table.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG8_YSIZE2) != trim2Table.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE2,trim2Table.ySize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG8_XSIZE3) != trim3Table.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE3,trim3Table.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG8_YSIZE3) != trim3Table.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE3,trim3Table.ySize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG8_XSIZE4) != trim4Table.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE4,trim4Table.xSize); writeCounter++; }
      if(EEPROM.read(EEPROM_CONFIG8_YSIZE4) != trim4Table.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE4,trim4Table.ySize); writeCounter++; }

      y = EEPROM_CONFIG8_MAP2;
      z = EEPROM_CONFIG8_MAP3;
      i = EEPROM_CONFIG8_MAP4;

      for(int x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG8_MAP1;
        newVal = trim1Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(x) != newVal ) { EEPROM.update(x, newVal ); writeCounter++; }

        offset = y - EEPROM_CONFIG8_MAP2;
        newVal = trim2Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(y) != newVal ) { EEPROM.update(y, newVal); writeCounter++; }

        offset = z - EEPROM_CONFIG8_MAP3;
        newVal = trim3Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(z) != newVal ) { EEPROM.update(z, newVal); writeCounter++; }

        offset = i - EEPROM_CONFIG8_MAP4;
        newVal = trim4Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(i) != newVal ) { EEPROM.update(i, newVal); writeCounter++; }

        y++;
        z++;
        i++;
      }

      y = EEPROM_CONFIG8_XBINS2;
      z = EEPROM_CONFIG8_XBINS3;
      i = EEPROM_CONFIG8_XBINS4;
      for(int x=EEPROM_CONFIG8_XBINS1; x<EEPROM_CONFIG8_YBINS1; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { eepromWritesPending = true; break; }
        offset = x - EEPROM_CONFIG8_XBINS1;
        EEPROM.update(x, byte(trim1Table.axisX[offset]/TABLE_RPM_MULTIPLIER));
        offset = y - EEPROM_CONFIG8_XBINS2;
        EEPROM.update(y, byte(trim2Table.axisX[offset]/TABLE_RPM_MULTIPLIER));
        offset = z - EEPROM_CONFIG8_XBINS3;
        EEPROM.update(z, byte(trim3Table.axisX[offset]/TABLE_RPM_MULTIPLIER));
        offset = i - EEPROM_CONFIG8_XBINS4;
        EEPROM.update(i, byte(trim4Table.axisX[offset]/TABLE_RPM_MULTIPLIER));
        y++;
        z++;
        i++;
      }

      y=EEPROM_CONFIG8_YBINS2;
      z=EEPROM_CONFIG8_YBINS3;
      i=EEPROM_CONFIG8_YBINS4;
      for(int x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { eepromWritesPending = true; break; }
        offset = x - EEPROM_CONFIG8_YBINS1;
        EEPROM.update(x, trim1Table.axisY[offset]/TABLE_LOAD_MULTIPLIER);
        offset = y - EEPROM_CONFIG8_YBINS2;
        EEPROM.update(y, trim2Table.axisY[offset]/TABLE_LOAD_MULTIPLIER);
        offset = z - EEPROM_CONFIG8_YBINS3;
        EEPROM.update(z, trim3Table.axisY[offset]/TABLE_LOAD_MULTIPLIER);
        offset = i - EEPROM_CONFIG8_YBINS4;
        EEPROM.update(i, trim4Table.axisY[offset]/TABLE_LOAD_MULTIPLIER);
        y++;
        z++;
        i++;
      }
      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case canbusPage:




      pnt_configPage = (byte *)&configPage9;
      for(int x=EEPROM_CONFIG9_START; x<EEPROM_CONFIG9_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG9_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG9_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case warmupPage:




      pnt_configPage = (byte *)&configPage10;

      for(int x=EEPROM_CONFIG10_START; x<EEPROM_CONFIG10_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG10_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG10_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case fuelMap2Page:




      EEPROM.update(EEPROM_CONFIG11_XSIZE, fuelTable2.xSize); writeCounter++;
      EEPROM.update(EEPROM_CONFIG11_YSIZE, fuelTable2.ySize); writeCounter++;
      for(int x=EEPROM_CONFIG11_MAP; x<EEPROM_CONFIG11_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG11_MAP;
        EEPROM.update(x, fuelTable2.values[15-(offset/16)][offset%16]); writeCounter++;
      }


      for(int x=EEPROM_CONFIG11_XBINS; x<EEPROM_CONFIG11_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG1_XBINS;
        EEPROM.update(x, byte(fuelTable2.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++;
      }

      for(int x=EEPROM_CONFIG11_YBINS; x<EEPROM_CONFIG11_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG11_YBINS;
        EEPROM.update(x, fuelTable2.axisY[offset] / TABLE_LOAD_MULTIPLIER);
      }
      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }
      break;


    default:
      break;
  }
}

void loadConfig()
{
  int offset;

  byte* pnt_configPage;



  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG1_MAP;
    fuelTable.values[15-(offset/16)][offset%16] = EEPROM.read(x);
  }

  for(int x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG1_XBINS;
    fuelTable.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER);
  }

  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++)
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    fuelTable.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER;
  }

  pnt_configPage = (byte *)&configPage2;
  for(int x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG2_START)) = EEPROM.read(x);
  }






  for(int x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG3_MAP;
    ignitionTable.values[15-(offset/16)][offset%16] = EEPROM.read(x);
  }

  for(int x=EEPROM_CONFIG3_XBINS; x<EEPROM_CONFIG3_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG3_XBINS;
    ignitionTable.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER);
  }

  for(int x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++)
  {
    offset = x - EEPROM_CONFIG3_YBINS;
    ignitionTable.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER;
  }

  pnt_configPage = (byte *)&configPage4;
  for(int x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG4_START)) = EEPROM.read(x);
  }





  for(int x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG5_MAP;
    afrTable.values[15-(offset/16)][offset%16] = EEPROM.read(x);
  }

  for(int x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG5_XBINS;
    afrTable.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER);
  }

  for(int x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++)
  {
    offset = x - EEPROM_CONFIG5_YBINS;
    afrTable.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER;
  }

  pnt_configPage = (byte *)&configPage6;
  for(int x=EEPROM_CONFIG6_START; x<EEPROM_CONFIG6_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG6_START)) = EEPROM.read(x);
  }



  int y = EEPROM_CONFIG7_MAP2;
  int z = EEPROM_CONFIG7_MAP3;
  for(int x=EEPROM_CONFIG7_MAP1; x<EEPROM_CONFIG7_XBINS1; x++)
  {
    offset = x - EEPROM_CONFIG7_MAP1;
    boostTable.values[7-(offset/8)][offset%8] = EEPROM.read(x);
    offset = y - EEPROM_CONFIG7_MAP2;
    vvtTable.values[7-(offset/8)][offset%8] = EEPROM.read(y);
    offset = z - EEPROM_CONFIG7_MAP3;
    stagingTable.values[7-(offset/8)][offset%8] = EEPROM.read(z);
    y++;
    z++;
  }


  y = EEPROM_CONFIG7_XBINS2;
  z = EEPROM_CONFIG7_XBINS3;
  for(int x=EEPROM_CONFIG7_XBINS1; x<EEPROM_CONFIG7_YBINS1; x++)
  {
    offset = x - EEPROM_CONFIG7_XBINS1;
    boostTable.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER);
    offset = y - EEPROM_CONFIG7_XBINS2;
    vvtTable.axisX[offset] = (EEPROM.read(y) * TABLE_RPM_MULTIPLIER);
    offset = z - EEPROM_CONFIG7_XBINS3;
    stagingTable.axisX[offset] = (EEPROM.read(z) * TABLE_RPM_MULTIPLIER);
    y++;
    z++;
  }


  y = EEPROM_CONFIG7_YBINS2;
  z = EEPROM_CONFIG7_YBINS3;
  for(int x=EEPROM_CONFIG7_YBINS1; x<EEPROM_CONFIG7_XSIZE2; x++)
  {
    offset = x - EEPROM_CONFIG7_YBINS1;
    boostTable.axisY[offset] = EEPROM.read(x);
    offset = y - EEPROM_CONFIG7_YBINS2;
    vvtTable.axisY[offset] = EEPROM.read(y);
    offset = z - EEPROM_CONFIG7_YBINS3;
    stagingTable.axisY[offset] = EEPROM.read(z) * TABLE_LOAD_MULTIPLIER;
    y++;
    z++;
  }



  y = EEPROM_CONFIG8_MAP2;
  z = EEPROM_CONFIG8_MAP3;
  int i = EEPROM_CONFIG8_MAP4;
  for(int x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++)
  {
    offset = x - EEPROM_CONFIG8_MAP1;
    trim1Table.values[5-(offset/6)][offset%6] = EEPROM.read(x);
    offset = y - EEPROM_CONFIG8_MAP2;
    trim2Table.values[5-(offset/6)][offset%6] = EEPROM.read(y);
    offset = z - EEPROM_CONFIG8_MAP3;
    trim3Table.values[5-(offset/6)][offset%6] = EEPROM.read(z);
    offset = i - EEPROM_CONFIG8_MAP4;
    trim4Table.values[5-(offset/6)][offset%6] = EEPROM.read(i);
    y++;
    z++;
    i++;
  }


  y = EEPROM_CONFIG8_XBINS2;
  z = EEPROM_CONFIG8_XBINS3;
  i = EEPROM_CONFIG8_XBINS4;
  for(int x=EEPROM_CONFIG8_XBINS1; x<EEPROM_CONFIG8_YBINS1; x++)
  {
    offset = x - EEPROM_CONFIG8_XBINS1;
    trim1Table.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER);
    offset = y - EEPROM_CONFIG8_XBINS2;
    trim2Table.axisX[offset] = (EEPROM.read(y) * TABLE_RPM_MULTIPLIER);
    offset = z - EEPROM_CONFIG8_XBINS3;
    trim3Table.axisX[offset] = (EEPROM.read(z) * TABLE_RPM_MULTIPLIER);
    offset = i - EEPROM_CONFIG8_XBINS4;
    trim4Table.axisX[offset] = (EEPROM.read(i) * TABLE_RPM_MULTIPLIER);
    y++;
    z++;
    i++;
  }


  y = EEPROM_CONFIG8_YBINS2;
  z = EEPROM_CONFIG8_YBINS3;
  i = EEPROM_CONFIG8_YBINS4;
  for(int x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++)
  {
    offset = x - EEPROM_CONFIG8_YBINS1;
    trim1Table.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER;
    offset = y - EEPROM_CONFIG8_YBINS2;
    trim2Table.axisY[offset] = EEPROM.read(y) * TABLE_LOAD_MULTIPLIER;
    offset = z - EEPROM_CONFIG8_YBINS3;
    trim3Table.axisY[offset] = EEPROM.read(z) * TABLE_LOAD_MULTIPLIER;
    offset = i - EEPROM_CONFIG8_YBINS4;
    trim4Table.axisY[offset] = EEPROM.read(i) * TABLE_LOAD_MULTIPLIER;
    y++;
    z++;
    i++;
  }


    pnt_configPage = (byte *)&configPage9;
  for(int x=EEPROM_CONFIG9_START; x<EEPROM_CONFIG9_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG9_START)) = EEPROM.read(x);
  }




  pnt_configPage = (byte *)&configPage10;

  for(int x=EEPROM_CONFIG10_START; x<EEPROM_CONFIG10_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG10_START)) = EEPROM.read(x);
  }



  for(int x=EEPROM_CONFIG11_MAP; x<EEPROM_CONFIG11_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG11_MAP;
    fuelTable2.values[15-(offset/16)][offset%16] = EEPROM.read(x);
  }

  for(int x=EEPROM_CONFIG11_XBINS; x<EEPROM_CONFIG11_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG11_XBINS;
    fuelTable2.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER);
  }

  for(int x=EEPROM_CONFIG11_YBINS; x<EEPROM_CONFIG11_END; x++)
  {
    offset = x - EEPROM_CONFIG11_YBINS;
    fuelTable2.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER;
  }

}





void loadCalibration()
{

  for(int x=0; x<CALIBRATION_TABLE_SIZE; x++)
  {
    int y = EEPROM_CALIBRATION_CLT + x;
    cltCalibrationTable[x] = EEPROM.read(y);

    y = EEPROM_CALIBRATION_IAT + x;
    iatCalibrationTable[x] = EEPROM.read(y);

    y = EEPROM_CALIBRATION_O2 + x;
    o2CalibrationTable[x] = EEPROM.read(y);
  }

}





void writeCalibration()
{

  for(int x=0; x<CALIBRATION_TABLE_SIZE; x++)
  {
    int y = EEPROM_CALIBRATION_CLT + x;
    if(EEPROM.read(y) != cltCalibrationTable[x]) { EEPROM.write(y, cltCalibrationTable[x]); }

    y = EEPROM_CALIBRATION_IAT + x;
    if(EEPROM.read(y) != iatCalibrationTable[x]) { EEPROM.write(y, iatCalibrationTable[x]); }

    y = EEPROM_CALIBRATION_O2 + x;
    if(EEPROM.read(y) != o2CalibrationTable[x]) { EEPROM.write(y, o2CalibrationTable[x]); }
  }

}





void storePageCRC32(byte pageNo, uint32_t crc32_val)
{
  uint16_t address;
  address = EEPROM_PAGE_CRC32 + ((NUM_PAGES - pageNo) * 4);


  byte four = (crc32_val & 0xFF);
  byte three = ((crc32_val >> 8) & 0xFF);
  byte two = ((crc32_val >> 16) & 0xFF);
  byte one = ((crc32_val >> 24) & 0xFF);


  EEPROM.update(address, four);
  EEPROM.update(address + 1, three);
  EEPROM.update(address + 2, two);
  EEPROM.update(address + 3, one);
}




uint32_t readPageCRC32(byte pageNo)
{
  uint16_t address;
  address = EEPROM_PAGE_CRC32 + ((NUM_PAGES - pageNo) * 4);


  uint32_t four = EEPROM.read(address);
  uint32_t three = EEPROM.read(address + 1);
  uint32_t two = EEPROM.read(address + 2);
  uint32_t one = EEPROM.read(address + 3);


  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}



byte readLastBaro() { return EEPROM.read(EEPROM_LAST_BARO); }
void storeLastBaro(byte newValue) { EEPROM.update(EEPROM_LAST_BARO, newValue); }
void storeCalibrationValue(uint16_t location, byte value) { EEPROM.update(location, value); }
byte readEEPROMVersion() { return EEPROM.read(EEPROM_DATA_VERSION); }
void storeEEPROMVersion(byte newVersion) { EEPROM.update(EEPROM_DATA_VERSION, newVersion); }
//# 1 "/home/developper/speeduino/speeduino/table.ino"
//# 11 "/home/developper/speeduino/speeduino/table.ino"
#include "globals.h"
#include "table.h"

void table2D_setSize(struct table2D* targetTable, byte newSize)
{


  {

    if(targetTable->valueSize == SIZE_BYTE)
    {
      targetTable->values = (byte *)realloc(targetTable->values, newSize * sizeof(byte));
      targetTable->axisX = (byte *)realloc(targetTable->axisX, newSize * sizeof(byte));
      targetTable->xSize = newSize;
    }
    else
    {
      targetTable->values16 = (int16_t *)realloc(targetTable->values16, newSize * sizeof(int16_t));
      targetTable->axisX16 = (int16_t *)realloc(targetTable->axisX16, newSize * sizeof(int16_t));
      targetTable->xSize = newSize;
    }
  }
}


void table3D_setSize(struct table3D *targetTable, byte newSize)
{
  if(initialisationComplete == false)
  {
    targetTable->values = (byte **)malloc(newSize * sizeof(byte*));
    for(byte i = 0; i < newSize; i++) { targetTable->values[i] = (byte *)malloc(newSize * sizeof(byte)); }

    targetTable->axisX = (int16_t *)malloc(newSize * sizeof(int16_t));
    targetTable->axisY = (int16_t *)malloc(newSize * sizeof(int16_t));
    targetTable->xSize = newSize;
    targetTable->ySize = newSize;
  }
}
//# 57 "/home/developper/speeduino/speeduino/table.ino"
int table2D_getValue(struct table2D *fromTable, int X_in)
{
    int returnValue;
    bool valueFound = false;

    int X = X_in;
    int xMinValue, xMaxValue;
    int xMin = 0;
    int xMax = 0;


    if( (X_in == fromTable->lastInput) && (fromTable->cacheTime == currentStatus.secl) )
    {
      returnValue = fromTable->lastOutput;
      valueFound = true;
    }
    else
    {
      fromTable->cacheTime = currentStatus.secl;

      if (fromTable->valueSize == SIZE_BYTE)
      {

        xMinValue = fromTable->axisX[0];
        xMaxValue = fromTable->axisX[fromTable->xSize-1];
      }
      else
      {

        xMinValue = fromTable->axisX16[0];
        xMaxValue = fromTable->axisX16[fromTable->xSize-1];
      }


      if(X > xMaxValue) { X = xMaxValue; }
      if(X < xMinValue) { X = xMinValue; }


      if (fromTable->valueSize == SIZE_BYTE)
      {



         if ( (X <= fromTable->axisX[fromTable->lastXMax]) && (X > fromTable->axisX[fromTable->lastXMin]) )
         {
           xMaxValue = fromTable->axisX[fromTable->lastXMax];
           xMinValue = fromTable->axisX[fromTable->lastXMin];
           xMax = fromTable->lastXMax;
           xMin = fromTable->lastXMin;
         }
         else
         {

            for (int x = fromTable->xSize-1; x >= 0; x--)
            {

                if ( (X == fromTable->axisX[x]) || (x == 0) )
                {
                  returnValue = fromTable->values[x];
                  valueFound = true;
                  break;
                }
                else
                {

                  if ( (X <= fromTable->axisX[x]) && (X > fromTable->axisX[x-1]) )
                  {
                    xMaxValue = fromTable->axisX[x];
                    xMinValue = fromTable->axisX[x-1];
                    xMax = x;
                    fromTable->lastXMax = xMax;
                    xMin = x-1;
                    fromTable->lastXMin = xMin;
                    break;
                  }
                }
            }
         }
      }
      else
      {



         if ( (X <= fromTable->axisX16[fromTable->lastXMax]) && (X > fromTable->axisX16[fromTable->lastXMin]) )
         {
           xMaxValue = fromTable->axisX16[fromTable->lastXMax];
           xMinValue = fromTable->axisX16[fromTable->lastXMin];
           xMax = fromTable->lastXMax;
           xMin = fromTable->lastXMin;
         }
         else
         {

            for (int x = fromTable->xSize-1; x >= 0; x--)
            {

                if ( (X == fromTable->axisX16[x]) || (x == 0) )
                {
                  returnValue = fromTable->values16[x];
                  valueFound = true;
                  break;
                }
                else
                {

                  if ( (X <= fromTable->axisX16[x]) && (X > fromTable->axisX16[x-1]) )
                  {
                    xMaxValue = fromTable->axisX16[x];
                    xMinValue = fromTable->axisX16[x-1];
                    xMax = x;
                    fromTable->lastXMax = xMax;
                    xMin = x-1;
                    fromTable->lastXMin = xMin;
                    break;
                  }
                }
            }
         }
      }
    }

    if (valueFound == false)
    {
      unsigned int m = X - xMinValue;
      unsigned int n = xMaxValue - xMinValue;







      int yVal;
      if (fromTable->valueSize == SIZE_BYTE)
      {

         yVal = ((long)(m << 6) / n) * (abs(fromTable->values[xMax] - fromTable->values[xMin]));
         yVal = (yVal >> 6);

         if (fromTable->values[xMax] > fromTable->values[xMin]) { yVal = fromTable->values[xMin] + yVal; }
         else { yVal = fromTable->values[xMin] - yVal; }
      }
      else
      {

         yVal = ((long)(m << 6) / n) * (abs(fromTable->values16[xMax] - fromTable->values16[xMin]));
         yVal = (yVal >> 6);

         if (fromTable->values[xMax] > fromTable->values16[xMin]) { yVal = fromTable->values16[xMin] + yVal; }
         else { yVal = fromTable->values16[xMin] - yVal; }
      }
      returnValue = yVal;
    }

    fromTable->lastInput = X_in;
    fromTable->lastOutput = returnValue;

    return returnValue;
}




int get3DTableValue(struct table3D *fromTable, int Y_in, int X_in)
  {
    int X = X_in;
    int Y = Y_in;

    int tableResult = 0;



    int xMinValue = fromTable->axisX[0];
    int xMaxValue = fromTable->axisX[fromTable->xSize-1];
    byte xMin = 0;
    byte xMax = 0;


    if(X > xMaxValue) { X = xMaxValue; }
    if(X < xMinValue) { X = xMinValue; }


    if ( (X <= fromTable->axisX[fromTable->lastXMax]) && (X > fromTable->axisX[fromTable->lastXMin]) )
    {
      xMaxValue = fromTable->axisX[fromTable->lastXMax];
      xMinValue = fromTable->axisX[fromTable->lastXMin];
      xMax = fromTable->lastXMax;
      xMin = fromTable->lastXMin;
    }

    else if ( ((fromTable->lastXMax + 1) < fromTable->xSize ) && (X <= fromTable->axisX[fromTable->lastXMax +1 ]) && (X > fromTable->axisX[fromTable->lastXMin + 1]) )
    {
      xMax = fromTable->lastXMax + 1;
      fromTable->lastXMax = xMax;
      xMin = fromTable->lastXMin + 1;
      fromTable->lastXMin = xMin;
      xMaxValue = fromTable->axisX[fromTable->lastXMax];
      xMinValue = fromTable->axisX[fromTable->lastXMin];
    }

    else if ( (fromTable->lastXMin > 0 ) && (X <= fromTable->axisX[fromTable->lastXMax - 1]) && (X > fromTable->axisX[fromTable->lastXMin - 1]) )
    {
      xMax = fromTable->lastXMax - 1;
      fromTable->lastXMax = xMax;
      xMin = fromTable->lastXMin - 1;
      fromTable->lastXMin = xMin;
      xMaxValue = fromTable->axisX[fromTable->lastXMax];
      xMinValue = fromTable->axisX[fromTable->lastXMin];
    }
    else

    {
      for (byte x = fromTable->xSize-1; x >= 0; x--)
      {

        if ( (X == fromTable->axisX[x]) || (x == 0) )
        {
          xMaxValue = fromTable->axisX[x];
          xMinValue = fromTable->axisX[x];
          xMax = x;
          fromTable->lastXMax = xMax;
          xMin = x;
          fromTable->lastXMin = xMin;
          break;
        }

        if ( (X <= fromTable->axisX[x]) && (X > fromTable->axisX[x-1]) )
        {
          xMaxValue = fromTable->axisX[x];
          xMinValue = fromTable->axisX[x-1];
          xMax = x;
          fromTable->lastXMax = xMax;
          xMin = x-1;
          fromTable->lastXMin = xMin;
          break;
        }
      }
    }


    int yMaxValue = fromTable->axisY[0];
    int yMinValue = fromTable->axisY[fromTable->ySize-1];
    byte yMin = 0;
    byte yMax = 0;


    if(Y > yMaxValue) { Y = yMaxValue; }
    if(Y < yMinValue) { Y = yMinValue; }


    if ( (Y >= fromTable->axisY[fromTable->lastYMax]) && (Y < fromTable->axisY[fromTable->lastYMin]) )
    {
      yMaxValue = fromTable->axisY[fromTable->lastYMax];
      yMinValue = fromTable->axisY[fromTable->lastYMin];
      yMax = fromTable->lastYMax;
      yMin = fromTable->lastYMin;
    }

    else if ( (fromTable->lastYMin > 0 ) && (Y <= fromTable->axisY[fromTable->lastYMin - 1 ]) && (Y > fromTable->axisY[fromTable->lastYMax - 1]) )
    {
      yMax = fromTable->lastYMax - 1;
      fromTable->lastYMax = yMax;
      yMin = fromTable->lastYMin - 1;
      fromTable->lastYMin = yMin;
      yMaxValue = fromTable->axisY[fromTable->lastYMax];
      yMinValue = fromTable->axisY[fromTable->lastYMin];
    }

    else if ( ((fromTable->lastYMax + 1) < fromTable->ySize) && (Y <= fromTable->axisY[fromTable->lastYMin + 1]) && (Y > fromTable->axisY[fromTable->lastYMax + 1]) )
    {
      yMax = fromTable->lastYMax + 1;
      fromTable->lastYMax = yMax;
      yMin = fromTable->lastYMin + 1;
      fromTable->lastYMin = yMin;
      yMaxValue = fromTable->axisY[fromTable->lastYMax];
      yMinValue = fromTable->axisY[fromTable->lastYMin];
    }
    else

    {

      for (byte y = fromTable->ySize-1; y >= 0; y--)
      {

        if ( (Y == fromTable->axisY[y]) || (y==0) )
        {
          yMaxValue = fromTable->axisY[y];
          yMinValue = fromTable->axisY[y];
          yMax = y;
          fromTable->lastYMax = yMax;
          yMin = y;
          fromTable->lastYMin = yMin;
          break;
        }

        if ( (Y >= fromTable->axisY[y]) && (Y < fromTable->axisY[y-1]) )
        {
          yMaxValue = fromTable->axisY[y];
          yMinValue = fromTable->axisY[y-1];
          yMax = y;
          fromTable->lastYMax = yMax;
          yMin = y-1;
          fromTable->lastYMin = yMin;
          break;
        }
      }
    }
//# 379 "/home/developper/speeduino/speeduino/table.ino"
    int A = fromTable->values[yMin][xMin];
    int B = fromTable->values[yMin][xMax];
    int C = fromTable->values[yMax][xMin];
    int D = fromTable->values[yMax][xMax];


    if( (A == B) && (A == C) && (A == D) ) { tableResult = A; }
    else
    {






      long p = (long)X - xMinValue;
      if (xMaxValue == xMinValue) { p = (p << 8); }
      else { p = ( (p << 8) / (xMaxValue - xMinValue) ); }

      long q;
      if (yMaxValue == yMinValue)
      {
        q = (long)Y - yMinValue;
        q = (q << 8);
      }

      else
      {
        q = long(Y) - yMaxValue;
        q = 256 - ( (q << 8) / (yMinValue - yMaxValue) );
      }







      int m = ((256-p) * (256-q)) >> 8;
      int n = (p * (256-q)) >> 8;
      int o = ((256-p) * q) >> 8;
      int r = (p * q) >> 8;
      tableResult = ( (A * m) + (B * n) + (C * o) + (D * r) ) >> 8;
    }
    return tableResult;
}
//# 1 "/home/developper/speeduino/speeduino/timers.ino"
//# 13 "/home/developper/speeduino/speeduino/timers.ino"
#include "timers.h"
#include "globals.h"
#include "sensors.h"
#include "scheduler.h"
#include "scheduledIO.h"
#include "auxiliaries.h"

#if defined(CORE_AVR)
  #include <avr/wdt.h>
#endif

void initialiseTimers()
{
  lastRPM_100ms = 0;
  loop33ms = 0;
  loop66ms = 0;
  loop100ms = 0;
  loop250ms = 0;
  loopSec = 0;
}




#if defined(CORE_AVR)
ISR(TIMER2_OVF_vect, ISR_NOBLOCK)
#else
void oneMSInterval()
#endif
{
  ms_counter++;


  loop33ms++;
  loop66ms++;
  loop100ms++;
  loop250ms++;
  loopSec++;

  unsigned long targetOverdwellTime;


  targetOverdwellTime = micros() - dwellLimit_uS;
  bool isCrankLocked = configPage4.ignCranklock && (currentStatus.RPM < currentStatus.crankRPM);


  if(ignitionSchedule1.Status == RUNNING) { if( (ignitionSchedule1.startTime < targetOverdwellTime) && (configPage4.useDwellLim) && (isCrankLocked != true) ) { endCoil1Charge(); ignitionSchedule1.Status = OFF; } }
  if(ignitionSchedule2.Status == RUNNING) { if( (ignitionSchedule2.startTime < targetOverdwellTime) && (configPage4.useDwellLim) && (isCrankLocked != true) ) { endCoil2Charge(); ignitionSchedule2.Status = OFF; } }
  if(ignitionSchedule3.Status == RUNNING) { if( (ignitionSchedule3.startTime < targetOverdwellTime) && (configPage4.useDwellLim) && (isCrankLocked != true) ) { endCoil3Charge(); ignitionSchedule3.Status = OFF; } }
  if(ignitionSchedule4.Status == RUNNING) { if( (ignitionSchedule4.startTime < targetOverdwellTime) && (configPage4.useDwellLim) && (isCrankLocked != true) ) { endCoil4Charge(); ignitionSchedule4.Status = OFF; } }
  if(ignitionSchedule5.Status == RUNNING) { if( (ignitionSchedule5.startTime < targetOverdwellTime) && (configPage4.useDwellLim) && (isCrankLocked != true) ) { endCoil5Charge(); ignitionSchedule5.Status = OFF; } }



  if(tachoOutputFlag == READY)
  {

    if( (configPage2.tachoDiv == 0) || (tachoAlt == true) )
    {
      TACHO_PULSE_LOW();

      tachoEndTime = (uint8_t)ms_counter + configPage2.tachoDuration;
      tachoOutputFlag = ACTIVE;
    }
    else
    {

      tachoOutputFlag = DEACTIVE;
    }
    tachoAlt = !tachoAlt;
  }
  else if(tachoOutputFlag == ACTIVE)
  {

    if((uint8_t)ms_counter >= tachoEndTime)
    {
      TACHO_PULSE_HIGH();
      tachoOutputFlag = DEACTIVE;
    }
  }




  if (loop33ms == 33)
  {
    loop33ms = 0;
    BIT_SET(TIMER_mask, BIT_TIMER_30HZ);
  }


  if (loop66ms == 66)
  {
    loop66ms = 0;
    BIT_SET(TIMER_mask, BIT_TIMER_15HZ);
  }



  if (loop100ms == 100)
  {
    loop100ms = 0;
    BIT_SET(TIMER_mask, BIT_TIMER_10HZ);

    currentStatus.rpmDOT = (currentStatus.RPM - lastRPM_100ms) * 10;
    lastRPM_100ms = currentStatus.RPM;
  }



  if (loop250ms == 250)
  {
    loop250ms = 0;
    BIT_SET(TIMER_mask, BIT_TIMER_4HZ);
    #if defined(CORE_STM32)
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    #endif

    #if defined(CORE_AVR)






    #endif
  }


  if (loopSec == 1000)
  {
    loopSec = 0;
    BIT_SET(TIMER_mask, BIT_TIMER_1HZ);

    dwellLimit_uS = (1000 * configPage4.dwellLimit);
    currentStatus.crankRPM = ((unsigned int)configPage4.crankRPM * 10);




    if (BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN))
    {
      if (currentStatus.runSecs <= 254)
        { currentStatus.runSecs++; }
    }


    currentStatus.loopsPerSecond = mainLoopCount;
    mainLoopCount = 0;


    currentStatus.secl++;


    if (configPage6.fanEnable == 1)
    {
       fanControl();
    }


    if(fpPrimed == false)
    {

      if( (currentStatus.secl - fpPrimeTime) >= configPage2.fpPrime)
      {
        fpPrimed = true;
        if(currentStatus.RPM == 0)
        {

          digitalWrite(pinFuelPump, LOW);
          currentStatus.fuelPumpOn = false;
        }
      }
    }


    if(configPage2.flexEnabled == true)
    {
      if(flexCounter < 50)
      {
        currentStatus.ethanolPct = 0;
        flexCounter = 0;
      }
      else if (flexCounter > 151)
      {

        if(flexCounter < 169)
        {
          currentStatus.ethanolPct = 100;
          flexCounter = 0;
        }
        else
        {

          currentStatus.ethanolPct = 0;
          flexCounter = 0;
        }
      }
      else
      {
        currentStatus.ethanolPct = flexCounter - 50;
        flexCounter = 0;
      }


      if (currentStatus.ethanolPct == 1) { currentStatus.ethanolPct = 0; }

    }

  }
#if defined(CORE_AVR)

    TCNT2 = 131;
#endif
}
//# 1 "/home/developper/speeduino/speeduino/updates.ino"






#include "globals.h"
#include "storage.h"
#include EEPROM_LIB_H

void doUpdates()
{
  #define CURRENT_DATA_VERSION 11


  if(EEPROM.read(EEPROM_DATA_VERSION) == 2)
  {
    for(int x=0; x<16; x++)
    {
      for(int y=0; y<16; y++)
      {
        ignitionTable.values[x][y] = ignitionTable.values[x][y] + 40;
      }
    }
    writeAllConfig();

    storeEEPROMVersion(3);
  }

  if(EEPROM.read(EEPROM_DATA_VERSION) == 3)
  {
    configPage9.speeduino_tsCanId = 0;
    configPage9.true_address = 256;
    configPage9.realtime_base_address = 336;


    if(configPage4.sparkDur == 255) { configPage4.sparkDur = 10; }

    writeAllConfig();

    storeEEPROMVersion(4);
  }

  if(EEPROM.read(EEPROM_DATA_VERSION) == 4)
  {

    configPage10.crankingEnrichBins[0] = 0;
    configPage10.crankingEnrichBins[1] = 40;
    configPage10.crankingEnrichBins[2] = 70;
    configPage10.crankingEnrichBins[3] = 100;

    configPage10.crankingEnrichValues[0] = 100 + configPage2.crankingPct;
    configPage10.crankingEnrichValues[1] = 100 + configPage2.crankingPct;
    configPage10.crankingEnrichValues[2] = 100 + configPage2.crankingPct;
    configPage10.crankingEnrichValues[3] = 100 + configPage2.crankingPct;

    writeAllConfig();

    storeEEPROMVersion(5);
  }

  if(EEPROM.read(EEPROM_DATA_VERSION) == 5)
  {

    for(int x=0; x < 1152; x++)
    {
      int endMem = EEPROM_CONFIG10_END - x;
      int startMem = endMem - 128;
      byte currentVal = EEPROM.read(startMem);
      EEPROM.update(endMem, currentVal);
    }

    for(int x=0; x < 352; x++)
    {
      int endMem = EEPROM_CONFIG10_END - 1152 - x;
      int startMem = endMem - 64;
      byte currentVal = EEPROM.read(startMem);
      EEPROM.update(endMem, currentVal);
    }

    EEPROM.write(EEPROM_DATA_VERSION, 6);
    loadConfig();
  }

  if(EEPROM.read(EEPROM_DATA_VERSION) == 6)
  {

    for(int x=0; x < 529; x++)
    {
      int endMem = EEPROM_CONFIG10_END - x;
      int startMem = endMem - 82;
      byte currentVal = EEPROM.read(startMem);
      EEPROM.update(endMem, currentVal);
    }

    EEPROM.write(EEPROM_DATA_VERSION, 7);
    loadConfig();
  }

  if (EEPROM.read(EEPROM_DATA_VERSION) == 7) {


    configPage10.flexBoostBins[0] = 0;
    configPage10.flexBoostAdj[0] = (int8_t)configPage2.unused2_1;

    configPage10.flexFuelBins[0] = 0;
    configPage10.flexFuelAdj[0] = configPage2.idleUpPin;

    configPage10.flexAdvBins[0] = 0;
    configPage10.flexAdvAdj[0] = configPage2.taeTaperMin;

    for (uint8_t x = 1; x < 6; x++)
    {
      uint8_t pct = x * 20;
      configPage10.flexBoostBins[x] = pct;
      configPage10.flexFuelBins[x] = pct;
      configPage10.flexAdvBins[x] = pct;

      int16_t boostAdder = (((configPage2.unused2_2 - (int8_t)configPage2.unused2_1) * pct) / 100) + (int8_t)configPage2.unused2_1;
      configPage10.flexBoostAdj[x] = boostAdder;

      uint8_t fuelAdder = (((configPage2.idleUpAdder - configPage2.idleUpPin) * pct) / 100) + configPage2.idleUpPin;
      configPage10.flexFuelAdj[x] = fuelAdder;

      uint8_t advanceAdder = (((configPage2.taeTaperMax - configPage2.taeTaperMin) * pct) / 100) + configPage2.taeTaperMin;
      configPage10.flexAdvAdj[x] = advanceAdder;
    }

    writeAllConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 8);
  }

  if (EEPROM.read(EEPROM_DATA_VERSION) == 8)
  {

    configPage2.fuelAlgorithm = configPage2.unused2_38c;
    configPage2.ignAlgorithm = configPage2.unused2_38c;


    configPage4.boostType = 1;

    writeAllConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 9);
  }

  if(EEPROM.read(EEPROM_DATA_VERSION) == 9)
  {


    for (byte AuxinChan = 0; AuxinChan <16 ; AuxinChan++)
    {
      configPage9.caninput_sel[AuxinChan] = 0;
    }


    configPage4.ADCFILTER_TPS = 50;
    configPage4.ADCFILTER_CLT = 180;
    configPage4.ADCFILTER_IAT = 180;
    configPage4.ADCFILTER_O2 = 128;
    configPage4.ADCFILTER_BAT = 128;
    configPage4.ADCFILTER_MAP = 20;
    configPage4.ADCFILTER_BARO= 64;

    writeAllConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 10);
  }

  if(EEPROM.read(EEPROM_DATA_VERSION) == 10)
  {


    configPage2.primePulse[0] = configPage2.unused2_39 / 5;
    configPage2.primePulse[1] = configPage2.unused2_39 / 5;
    configPage2.primePulse[2] = configPage2.unused2_39 / 5;
    configPage2.primePulse[3] = configPage2.unused2_39 / 5;

    configPage2.primeBins[0] = 0;
    configPage2.primeBins[1] = 40;
    configPage2.primeBins[2] = 70;
    configPage2.primeBins[3] = 100;


    if(configPage2.tachoDuration > 6) { configPage2.tachoDuration = 3; }


    configPage2.aeMode = AE_MODE_TPS;

    configPage4.maeRates[0] = 75;
    configPage4.maeRates[2] = 75;
    configPage4.maeRates[3] = 75;
    configPage4.maeRates[4] = 75;
    configPage4.maeBins[0] = 7;
    configPage4.maeBins[1] = 12;
    configPage4.maeBins[2] = 20;
    configPage4.maeBins[3] = 40;


    writeAllConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 11);
  }


  if( (EEPROM.read(EEPROM_DATA_VERSION) == 0) || (EEPROM.read(EEPROM_DATA_VERSION) == 255) )
  {
    configPage9.true_address = 0x200;
    EEPROM.write(EEPROM_DATA_VERSION, CURRENT_DATA_VERSION);
  }


  if( EEPROM.read(EEPROM_DATA_VERSION) > CURRENT_DATA_VERSION ) { EEPROM.write(EEPROM_DATA_VERSION, CURRENT_DATA_VERSION); }
}
//# 1 "/home/developper/speeduino/speeduino/utils.ino"
//# 12 "/home/developper/speeduino/speeduino/utils.ino"
#include <avr/pgmspace.h>
#include "globals.h"
#include "utils.h"
#include "decoders.h"
#include "comms.h"
#include "src/FastCRC/FastCRC.h"

FastCRC32 CRC32;




byte pinTranslate(byte rawPin)
{
  byte outputPin = rawPin;
  if(rawPin > BOARD_DIGITAL_GPIO_PINS) { outputPin = A8 + (outputPin - BOARD_DIGITAL_GPIO_PINS - 1); }

  return outputPin;
}


void setResetControlPinState()
{
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);


  switch (resetControl)
  {
    case RESET_CONTROL_PREVENT_WHEN_RUNNING:

      digitalWrite(pinResetControl, LOW);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      break;
    case RESET_CONTROL_PREVENT_ALWAYS:

      digitalWrite(pinResetControl, HIGH);
      BIT_SET(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      break;
    case RESET_CONTROL_SERIAL_COMMAND:


      digitalWrite(pinResetControl, HIGH);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      break;
  }
}




uint32_t calculateCRC32(byte pageNo)
{
  uint32_t CRC32_val;
  byte raw_value;
  void* pnt_configPage;


  switch(pageNo)
  {
    case veMapPage:

      raw_value = getPageValue(veMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[veMapPage]; x++)

      {
        raw_value = getPageValue(veMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }

      CRC32_val = ~CRC32_val;
      break;

    case veSetPage:

      pnt_configPage = &configPage2;
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage2) );
      break;

    case ignMapPage:

      raw_value = getPageValue(ignMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[ignMapPage]; x++)
      {
        raw_value = getPageValue(ignMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }

      CRC32_val = ~CRC32_val;
      break;

    case ignSetPage:

      pnt_configPage = &configPage4;
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage4) );
      break;

    case afrMapPage:

      raw_value = getPageValue(afrMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[afrMapPage]; x++)
      {
        raw_value = getPageValue(afrMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }

      CRC32_val = ~CRC32_val;
      break;

    case afrSetPage:

      pnt_configPage = &configPage6;
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage6) );
      break;

    case boostvvtPage:

      raw_value = getPageValue(boostvvtPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[boostvvtPage]; x++)
      {
        raw_value = getPageValue(boostvvtPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }

      CRC32_val = ~CRC32_val;
      break;

    case seqFuelPage:

      raw_value = getPageValue(seqFuelPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[seqFuelPage]; x++)
      {
        raw_value = getPageValue(seqFuelPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }

      CRC32_val = ~CRC32_val;
      break;

    case canbusPage:

      pnt_configPage = &configPage9;
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage9) );
      break;

    case warmupPage:

      pnt_configPage = &configPage10;
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage10) );
      break;

    case fuelMap2Page:

      raw_value = getPageValue(fuelMap2Page, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[fuelMap2Page]; x++)

      {
        raw_value = getPageValue(fuelMap2Page, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }

      CRC32_val = ~CRC32_val;
      break;

    default:
      CRC32_val = 0;
      break;
  }

  return CRC32_val;
}