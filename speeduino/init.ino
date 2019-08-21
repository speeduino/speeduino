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
#include BOARD_H //Note that this is not a real file, it is defined in globals.h. 

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
    doUpdates(); //Check if any data items need updating (Occurs with firmware updates)

    //Always start with a clean slate on the bootloader capabilities level
    //This should be 0 until we hear otherwise from the 16u2
    configPage4.bootloaderCaps = 0;

    initBoard(); //This calls the current individual boards init function. See the board_xxx.ino files for these.
    initialiseTimers();

    Serial.begin(115200);
    if (configPage9.enable_secondarySerial == 1) { CANSerial.begin(115200); }

    #if defined(CORE_STM32) || defined(CORE_TEENSY)
    configPage9.intcan_available = 1;   // device has internal canbus
    //Teensy onboard CAN not used currently
    //enable local can interface
    //setup can interface to 250k
    //FlexCAN CANbus0(2500000, 0);
    //static CAN_message_t txmsg,rxmsg;
    //CANbus0.begin();
    #endif

    //Repoint the 2D table structs to the config pages that were just loaded
    taeTable.valueSize = SIZE_BYTE; //Set this table to use byte values
    taeTable.xSize = 4;
    taeTable.values = configPage4.taeValues;
    taeTable.axisX = configPage4.taeBins;
    maeTable.valueSize = SIZE_BYTE; //Set this table to use byte values
    maeTable.xSize = 4;
    maeTable.values = configPage4.maeRates;
    maeTable.axisX = configPage4.maeBins;
    WUETable.valueSize = SIZE_BYTE; //Set this table to use byte values
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
    CLTAdvanceTable.values = (byte*)configPage4.cltAdvValues;
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

    //Setup the calibration tables
    loadCalibration();

    //Set the pin mappings
    if(configPage2.pinMapping == 255)
    {
    //First time running on this board
    setPinMapping(3); //Force board to v0.4
    configPage2.flexEnabled = false; //Have to disable flex. If this isn't done and the wrong flex pin is interrupt attached below, system can hang.
    }
    else { setPinMapping(configPage2.pinMapping); }

    //Need to check early on whether the coil charging is inverted. If this is not set straight away it can cause an unwanted spark at bootup
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

    //Similar for injectors, make sure they're turned off
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

    //Set the tacho output default state
    digitalWrite(pinTachOut, HIGH);
    //Perform all initialisations
    initialiseSchedulers();
    //initialiseDisplay();
    initialiseIdle();
    initialiseFan();
    initialiseAuxPWM();
    initialiseCorrections();
    initialiseADC();

    //Lookup the current MAP reading for barometric pressure
    instanteneousMAPReading();
    //barometric reading can be taken from either an external sensor if enabled, or simply by using the initial MAP value
    if ( configPage6.useExtBaro != 0 )
    {
      readBaro();
      //EEPROM.update(EEPROM_LAST_BARO, currentStatus.baro);
      storeLastBaro(currentStatus.baro);
    }
    else
    {
    /*
        * The highest sea-level pressure on Earth occurs in Siberia, where the Siberian High often attains a sea-level pressure above 105 kPa;
        * with record highs close to 108.5 kPa.
        * The lowest measurable sea-level pressure is found at the centers of tropical cyclones and tornadoes, with a record low of 87 kPa;
        */
    if ((currentStatus.MAP >= BARO_MIN) && (currentStatus.MAP <= BARO_MAX)) //Check if engine isn't running
    {
        currentStatus.baro = currentStatus.MAP;
        //EEPROM.update(EEPROM_LAST_BARO, currentStatus.baro);
        storeLastBaro(currentStatus.baro);
    }
    else
    {
        //Attempt to use the last known good baro reading from EEPROM
        if ((readLastBaro() >= BARO_MIN) && (readLastBaro() <= BARO_MAX)) //Make sure it's not invalid (Possible on first run etc)
        { currentStatus.baro = readLastBaro(); } //last baro correction
        else { currentStatus.baro = 100; } //Final fall back position.
    }
    }

    //Check whether the flex sensor is enabled and if so, attach an interupt for it
    if(configPage2.flexEnabled > 0)
    {
    attachInterrupt(digitalPinToInterrupt(pinFlex), flexPulse, RISING);
    currentStatus.ethanolPct = 0;
    }

    //Once the configs have been loaded, a number of one time calculations can be completed
    req_fuel_uS = configPage2.reqFuel * 100; //Convert to uS and an int. This is the only variable to be used in calculations
    inj_opentime_uS = configPage2.injOpen * 100; //Injector open time. Comes through as ms*10 (Eg 15.5ms = 155).

    if(configPage10.stagingEnabled == true)
    {
    uint32_t totalInjector = configPage10.stagedInjSizePri + configPage10.stagedInjSizeSec;
    /*
        These values are a percentage of the req_fuel value that would be required for each injector channel to deliver that much fuel.
        Eg:
        Pri injectors are 250cc
        Sec injectors are 500cc
        Total injector capacity = 750cc

        staged_req_fuel_mult_pri = 300% (The primary injectors would have to run 3x the overall PW in order to be the equivalent of the full 750cc capacity
        staged_req_fuel_mult_sec = 150% (The secondary injectors would have to run 1.5x the overall PW in order to be the equivalent of the full 750cc capacity
    */
    staged_req_fuel_mult_pri = (100 * totalInjector) / configPage10.stagedInjSizePri;
    staged_req_fuel_mult_sec = (100 * totalInjector) / configPage10.stagedInjSizeSec;
    }

    //Begin the main crank trigger interrupt pin setup
    //The interrupt numbering is a bit odd - See here for reference: arduino.cc/en/Reference/AttachInterrupt
    //These assignments are based on the Arduino Mega AND VARY BETWEEN BOARDS. Please confirm the board you are using and update acordingly.
    currentStatus.RPM = 0;
    currentStatus.hasSync = false;
    currentStatus.runSecs = 0;
    currentStatus.secl = 0;
    currentStatus.startRevolutions = 0;
    currentStatus.syncLossCounter = 0;
    currentStatus.flatShiftingHard = false;
    currentStatus.launchingHard = false;
    currentStatus.crankRPM = ((unsigned int)configPage4.crankRPM * 10); //Crank RPM limit (Saves us calculating this over and over again. It's updated once per second in timers.ino)
    currentStatus.fuelPumpOn = false;
    triggerFilterTime = 0; //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be disgarded as noise. This is simply a default value, the actual values are set in the setup() functinos of each decoder
    dwellLimit_uS = (1000 * configPage4.dwellLimit);
    currentStatus.nChannels = (INJ_CHANNELS << 4) + IGN_CHANNELS; //First 4 bits store the number of injection channels, 2nd 4 store the number of ignition channels
    fpPrimeTime = 0;

    noInterrupts();
    initialiseTriggers();

    //End crank triger interrupt attachment
    if(configPage2.strokes == FOUR_STROKE)
    {
      //Default is 1 squirt per revolution, so we halve the given req-fuel figure (Which would be over 2 revolutions)
      req_fuel_uS = req_fuel_uS / 2; //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle. If we're doing more than 1 squirt per cycle then we need to split the amount accordingly. (Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the stroke to make the single squirt on)
    }

    //Initial values for loop times
    previousLoopTime = 0;
    currentLoopTime = micros_safe();

    mainLoopCount = 0;

    currentStatus.nSquirts = configPage2.nCylinders / configPage2.divider; //The number of squirts being requested. This is manaully overriden below for sequential setups (Due to TS req_fuel calc limitations)
    if(currentStatus.nSquirts == 0) { currentStatus.nSquirts = 1; } //Safety check. Should never happen as TS will give an error, but leave incase tune is manually altered etc. 
    if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = 720 / currentStatus.nSquirts; }
    else { CRANK_ANGLE_MAX_INJ = 360 / currentStatus.nSquirts; }

    //Calculate the number of degrees between cylinders
    switch (configPage2.nCylinders) {
    case 1:
        channel1IgnDegrees = 0;
        channel1InjDegrees = 0;
        maxIgnOutputs = 1;

        //Sequential ignition works identically on a 1 cylinder whether it's odd or even fire. 
        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) ) { CRANK_ANGLE_MAX_IGN = 720; }

        if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
        {
          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
          req_fuel_uS = req_fuel_uS * 2;
        }

        channel1InjEnabled = true;

        //Check if injector staging is enabled
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

        //Sequential ignition works identically on a 2 cylinder whether it's odd or even fire (With the default being a 180 degree second cylinder). 
        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) ) { CRANK_ANGLE_MAX_IGN = 720; }

        if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
        {
          CRANK_ANGLE_MAX_INJ = 720;
          currentStatus.nSquirts = 1;
          req_fuel_uS = req_fuel_uS * 2;
        }
        //The below are true regardless of whether this is running sequential or not
        if (configPage2.engineType == EVEN_FIRE ) { channel2InjDegrees = 180; }
        else { channel2InjDegrees = configPage2.oddfire2; }
        if (!configPage2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          channel1InjDegrees = 0;
          channel2InjDegrees = 0; 
        }

        channel1InjEnabled = true;
        channel2InjEnabled = true;

        //Check if injector staging is enabled
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
        //Sequential and Single channel modes both run over 720 crank degrees, but only on 4 stroke engines. 
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

        //For alternatiing injection, the squirt occurs at different times for each channel
        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) || (configPage2.strokes == TWO_STROKE) )
        {
          channel1InjDegrees = 0;
          channel2InjDegrees = 120;
          channel3InjDegrees = 240;

          //Adjust the injection angles based on the number of squirts
          if (currentStatus.nSquirts > 2)
          {
            channel2InjDegrees = (channel2InjDegrees * 2) / currentStatus.nSquirts;
            channel3InjDegrees = (channel3InjDegrees * 2) / currentStatus.nSquirts;
          }

          if (!configPage2.injTiming) 
          { 
            //For simultaneous, all squirts happen at the same time
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
        maxIgnOutputs = 2; //Default value for 4 cylinder, may be changed below
        if (configPage2.engineType == EVEN_FIRE )
        {
          channel2IgnDegrees = 180;
          //Adjust the injection angles based on the number of squirts
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
            //Rotary uses the ign 3 and 4 schedules for the trailing spark. They are offset from the ign 1 and 2 channels respectively and so use the same degrees as them
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

        //For alternatiing injection, the squirt occurs at different times for each channel
        if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) || (configPage2.strokes == TWO_STROKE) )
        {
          channel2InjDegrees = 180;

          if (!configPage2.injTiming) 
          { 
            //For simultaneous, all squirts happen at the same time
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

        //Check if injector staging is enabled
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
        maxIgnOutputs = 4; //Only 4 actual outputs, so that's all that can be cut

        if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
        {
          channel2IgnDegrees = 144;
          channel3IgnDegrees = 288;
          channel4IgnDegrees = 432;
          channel5IgnDegrees = 576;

          CRANK_ANGLE_MAX_IGN = 720;
        }

        //For alternatiing injection, the squirt occurs at different times for each channel
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
          //For simultaneous, all squirts happen at the same time
          channel1InjDegrees = 0;
          channel2InjDegrees = 0;
          channel3InjDegrees = 0;
          channel4InjDegrees = 0;
          channel5InjDegrees = 0; 
        }

        channel1InjEnabled = true;
        channel2InjEnabled = true;
        channel3InjEnabled = false; //this is disabled as injector 5 function calls 3 & 5 together
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

        //Adjust the injection angles based on the number of squirts
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
          //For simultaneous, all squirts happen at the same time
          channel1InjDegrees = 0;
          channel2InjDegrees = 0;
          channel3InjDegrees = 0; 
        } 

        configPage2.injLayout = 0; //This is a failsafe. We can never run semi-sequential with more than 4 cylinders

        channel1InjEnabled = true;
        channel2InjEnabled = true;
        channel3InjEnabled = true;
        break;
    case 8:
        channel1IgnDegrees = 0;
        channel2IgnDegrees = channel2InjDegrees = 90;
        channel3IgnDegrees = channel3InjDegrees = 180;
        channel4IgnDegrees = channel4InjDegrees = 270;

        //Adjust the injection angles based on the number of squirts
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
          //For simultaneous, all squirts happen at the same time
          channel1InjDegrees = 0;
          channel2InjDegrees = 0;
          channel3InjDegrees = 0;
          channel4InjDegrees = 0; 
        }

        configPage2.injLayout = 0; //This is a failsafe. We can never run semi-sequential with more than 4 cylinders

        channel1InjEnabled = true;
        channel2InjEnabled = true;
        channel3InjEnabled = true;
        channel4InjEnabled = true;
        break;
    default: //Handle this better!!!
        channel1InjDegrees = 0;
        channel2InjDegrees = 180;
        break;
    }

    if(CRANK_ANGLE_MAX_IGN == CRANK_ANGLE_MAX_INJ) { CRANK_ANGLE_MAX = CRANK_ANGLE_MAX_IGN; } //If both the injector max and ignition max angles are the same, make the overall system max this value
    else if (CRANK_ANGLE_MAX_IGN > CRANK_ANGLE_MAX_INJ) { CRANK_ANGLE_MAX = CRANK_ANGLE_MAX_IGN; }
    else { CRANK_ANGLE_MAX = CRANK_ANGLE_MAX_INJ; }
    currentStatus.status3 = currentStatus.nSquirts << BIT_STATUS3_NSQUIRTS1; //Top 3 bits of the status3 variable are the number of squirts. This must be done after the above section due to nSquirts being forced to 1 for sequential
    
    //Special case:
    //3 or 5 squirts per cycle MUST be tracked over 720 degrees. This is because the angles for them (Eg 720/3=240) are not evenly divisible into 360
    //This is ONLY the case on 4 stroke systems
    if( (currentStatus.nSquirts == 3) || (currentStatus.nSquirts == 5) )
    {
      if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX = 720; }
    }
    

    switch(configPage4.sparkMode)
    {
    case IGN_MODE_WASTED:
        //Wasted Spark (Normal mode)
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
        //Single channel mode. All ignition pulses are on channel 1
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
        //Wasted COP mode. Ignition channels 1&3 and 2&4 are paired together
        //This is not a valid mode for >4 cylinders
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
          //If the person has inadvertantly selected this when running more than 4 cylinders, just use standard Wasted spark mode
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
          //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
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
          //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
          ign1StartFunction = beginCoil1Charge;
          ign1EndFunction = endCoil1Charge;
          ign2StartFunction = beginCoil1Charge;
          ign2EndFunction = endCoil1Charge;

          //Trailing coils have their own channel each
          //IGN2 = front rotor trailing spark
          ign3StartFunction = beginCoil2Charge;
          ign3EndFunction = endCoil2Charge;
          //IGN3 = rear rotor trailing spark
          ign4StartFunction = beginCoil3Charge;
          ign4EndFunction = endCoil3Charge;

          //IGN4 not used
        }
        else if(configPage10.rotaryType == ROTARY_IGN_RX8)
        {
          //RX8 outputs are simply 1 coil and 1 output per plug

          //IGN1 is front rotor, leading spark
          ign1StartFunction = beginCoil1Charge;
          ign1EndFunction = endCoil1Charge;
          //IGN2 is rear rotor, leading spark
          ign2StartFunction = beginCoil2Charge;
          ign2EndFunction = endCoil2Charge;
          //IGN3 = front rotor trailing spark
          ign3StartFunction = beginCoil3Charge;
          ign3EndFunction = endCoil3Charge;
          //IGN4 = rear rotor trailing spark
          ign4StartFunction = beginCoil4Charge;
          ign4EndFunction = endCoil4Charge;
        }
        break;



    default:
        //Wasted spark (Shouldn't ever happen anyway)
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

    //Begin priming the fuel pump. This is turned off in the low resolution, 1s interrupt in timers.ino
    //First check that the priming time is not 0
    if(configPage2.fpPrime > 0)
    {
      FUEL_PUMP_ON();
      currentStatus.fuelPumpOn = true;
    }
    else { fpPrimed = true; } //If the user has set 0 for the pump priming, immediately mark the priming as being completed

    interrupts();
    //Perform the priming pulses. Set these to run at an arbitrary time in the future (100us). The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
    readCLT(false); // Need to read coolant temp to make priming pulsewidth work correctly. The false here disables use of the filter
    unsigned long primingValue = table2D_getValue(&PrimingPulseTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
    if(primingValue > 0)
    {
      setFuelSchedule1(100, (primingValue * 100 * 5)); //to acheive long enough priming pulses, the values in tuner studio are divided by 0.5 instead of 0.1, so multiplier of 5 is required.
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
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.1 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 11; //Output pin injector 3 is on
      pinInjector4 = 10; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 6; //Pin for coil 1
      pinCoil2 = 7; //Pin for coil 2
      pinCoil3 = 12; //Pin for coil 3
      pinCoil4 = 13; //Pin for coil 4
      pinCoil5 = 14; //Pin for coil 5
      pinTrigger = 2; //The CAS pin
      pinTrigger2 = 3; //The CAS pin
      pinTPS = A0; //TPS input pin
      pinMAP = A1; //MAP sensor pin
      pinIAT = A2; //IAT sensor pin
      pinCLT = A3; //CLS sensor pin
      pinO2 = A4; //O2 Sensor pin
      pinIdle1 = 46; //Single wire idle control
      pinIdle2 = 47; //2 wire idle control
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinFan = 47; //Pin for the fan output
      pinFuelPump = 4; //Fuel pump output
      pinTachOut = 49; //Tacho output pin
      pinFlex = 19; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 43; //Reset control output
    #endif
      break;
    case 1:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.2 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 28; //Pin for coil 1
      pinCoil2 = 24; //Pin for coil 2
      pinCoil3 = 40; //Pin for coil 3
      pinCoil4 = 36; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 20; //The CAS pin
      pinTrigger2 = 21; //The Cam Sensor pin
      pinTPS = A2; //TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A8; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin
      pinIdle1 = 30; //Single wire idle control
      pinIdle2 = 31; //2 wire idle control
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinFan = 47; //Pin for the fan output
      pinFuelPump = 4; //Fuel pump output
      pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 43; //Reset control output
      break;
    #endif
    case 2:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the v0.3 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 28; //Pin for coil 1
      pinCoil2 = 24; //Pin for coil 2
      pinCoil3 = 40; //Pin for coil 3
      pinCoil4 = 36; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A8; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      pinIdle2 = 53; //2 wire idle control
      pinBoost = 7; //Boost control
      pinVVT_1 = 6; //Default VVT output
      pinFuelPump = 4; //Fuel pump output
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinStepperEnable = 26; //Enable pin for DRV8825
      pinFan = A13; //Pin for the fan output
      pinLaunch = 51; //Can be overwritten below
      pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 50; //Reset control output

      #if defined(CORE_TEENSY35)
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
      //Pin mappings as per the v0.4 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinInjector6 = 50; //CAUTION: Uses the same as Coil 4 below. 
      pinCoil1 = 40; //Pin for coil 1
      pinCoil2 = 38; //Pin for coil 2
      pinCoil3 = 52; //Pin for coil 3
      pinCoil4 = 50; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A8; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
      pinIdle1 = 5; //Single wire idle control
      pinIdle2 = 6; //2 wire idle control
      pinBoost = 7; //Boost control
      pinVVT_1 = 4; //Default VVT output
      pinFuelPump = 45; //Fuel pump output  (Goes to ULN2803)
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinStepperEnable = 24; //Enable pin for DRV8825
      pinFan = 47; //Pin for the fan output (Goes to ULN2803)
      pinLaunch = 51; //Can be overwritten below
      pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 43; //Reset control output

      #if defined(CORE_TEENSY35)
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
        //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407
        //PC8~PC12 SDio
        //PA13~PA15 & PB4 SWD(debug) pins
        //PB0 EEPROM CS pin
        //PA9 & PD10 Serial1
        //PD5 & PD6 Serial2
        pinInjector1 = PE7; //Output pin injector 1 is on
        pinInjector2 = PE8; //Output pin injector 2 is on
        pinInjector3 = PE9; //Output pin injector 3 is on
        pinInjector4 = PE10; //Output pin injector 4 is on
        pinInjector5 = PE11; //Output pin injector 5 is on
        pinInjector6 = PE12; //Output pin injector 6 is on
        pinCoil1 = PD0; //Pin for coil 1
        pinCoil2 = PD1; //Pin for coil 2
        pinCoil3 = PD2; //Pin for coil 3
        pinCoil4 = PD3; //Pin for coil 4
        pinCoil5 = PD4; //Pin for coil 5
        pinTPS = A0; //TPS input pin
        pinMAP = A1; //MAP sensor pin
        pinIAT = A2; //IAT sensor pin
        pinCLT = A3; //CLT sensor pin
        pinO2 = A4; //O2 Sensor pin
        pinBat = A5; //Battery reference voltage pin
        pinBaro = A9;
        pinIdle1 = PB8; //Single wire idle control
        pinIdle2 = PB9; //2 wire idle control
        pinBoost = PE0; //Boost control
        pinVVT_1 = PE1; //Default VVT output
        pinStepperDir = PD8; //Direction pin  for DRV8825 driver
        pinStepperStep = PB15; //Step pin for DRV8825 driver
        pinStepperEnable = PD9; //Enable pin for DRV8825
        pinDisplayReset = PE1; // OLED reset pin
        pinFan = PE2; //Pin for the fan output
        pinFuelPump = PC0; //Fuel pump output
        pinTachOut = PC1; //Tacho output pin
        //external interrupt enabled pins
        //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
        pinFlex = PE2; // Flex sensor (Must be external interrupt enabled)
        pinTrigger = PE3; //The CAS pin
        pinTrigger2 = PE4; //The Cam Sensor pin
      #elif defined(CORE_STM32)
        //blue pill wiki.stm32duino.com/index.php?title=Blue_Pill
        //Maple mini wiki.stm32duino.com/index.php?title=Maple_Mini
        //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
        pinInjector1 = PB7; //Output pin injector 1 is on
        pinInjector2 = PB6; //Output pin injector 2 is on
        pinInjector3 = PB5; //Output pin injector 3 is on
        pinInjector4 = PB4; //Output pin injector 4 is on
        pinCoil1 = PB3; //Pin for coil 1
        pinCoil2 = PA15; //Pin for coil 2
        pinCoil3 = PA14; //Pin for coil 3
        pinCoil4 = PA9; //Pin for coil 4
        pinCoil5 = PA8; //Pin for coil 5
        pinTPS = A0; //TPS input pin
        pinMAP = A1; //MAP sensor pin
        pinIAT = A2; //IAT sensor pin
        pinCLT = A3; //CLS sensor pin
        pinO2 = A4; //O2 Sensor pin
        pinBat = A5; //Battery reference voltage pin
        pinBaro = pinMAP;
        pinIdle1 = PB2; //Single wire idle control
        pinIdle2 = PA2; //2 wire idle control
        pinBoost = PA1; //Boost control
        pinVVT_1 = PA0; //Default VVT output
        pinStepperDir = PC15; //Direction pin  for DRV8825 driver
        pinStepperStep = PC14; //Step pin for DRV8825 driver
        pinStepperEnable = PC13; //Enable pin for DRV8825
        pinDisplayReset = PB2; // OLED reset pin
        pinFan = PB1; //Pin for the fan output
        pinFuelPump = PB11; //Fuel pump output
        pinTachOut = PB10; //Tacho output pin
        //external interrupt enabled pins
        pinFlex = PB8; // Flex sensor (Must be external interrupt enabled)
        pinTrigger = PA10; //The CAS pin
        pinTrigger2 = PA13; //The Cam Sensor pin
      #endif
      break;

    case 6:
      //Pin mappings as per the 2001-05 MX5 PNP shield
      pinInjector1 = 44; //Output pin injector 1 is on
      pinInjector2 = 46; //Output pin injector 2 is on
      pinInjector3 = 47; //Output pin injector 3 is on
      pinInjector4 = 45; //Output pin injector 4 is on
      pinInjector5 = 14; //Output pin injector 5 is on
      pinCoil1 = 42; //Pin for coil 1
      pinCoil2 = 43; //Pin for coil 2
      pinCoil3 = 32; //Pin for coil 3
      pinCoil4 = 33; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A5; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A3; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 23; //Tacho output pin  (Goes to ULN2803)
      pinIdle1 = 5; //Single wire idle control
      pinBoost = 4;
      pinVVT_1 = 11; //Default VVT output
      pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pinFuelPump = 40; //Fuel pump output
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinStepperEnable = 24;
      pinFan = 41; //Pin for the fan output
      pinLaunch = 12; //Can be overwritten below
      pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 39; //Reset control output

      //This is NOT correct. It has not yet been tested with this board
      #if defined(CORE_TEENSY35)
        pinTrigger = 23;
        pinTrigger2 = 36;
        pinStepperDir = 34;
        pinStepperStep = 35;
        pinCoil1 = 33; //Done
        pinCoil2 = 24; //Done
        pinCoil3 = 51; //Won't work (No mapping for pin 32)
        pinCoil4 = 52; //Won't work (No mapping for pin 33)
        pinFuelPump = 26; //Requires PVT4 adapter or above
        pinFan = 50; //Won't work (No mapping for pin 35)
        pinTachOut = 28; //Done
      #endif
      break;

    case 8:
      //Pin mappings as per the 1996-97 MX5 PNP shield
      pinInjector1 = 11; //Output pin injector 1 is on
      pinInjector2 = 10; //Output pin injector 2 is on
      pinInjector3 = 9; //Output pin injector 3 is on
      pinInjector4 = 8; //Output pin injector 4 is on
      pinInjector5 = 14; //Output pin injector 5 is on
      pinCoil1 = 39; //Pin for coil 1
      pinCoil2 = 41; //Pin for coil 2
      pinCoil3 = 32; //Pin for coil 3
      pinCoil4 = 33; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A5; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A3; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = A9; //Tacho output pin  (Goes to ULN2803)
      pinIdle1 = 2; //Single wire idle control
      pinBoost = 4;
      pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pinFuelPump = 49; //Fuel pump output
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinStepperEnable = 24;
      pinFan = 35; //Pin for the fan output
      pinLaunch = 37; //Can be overwritten below
      pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 44; //Reset control output

      //This is NOT correct. It has not yet been tested with this board
      #if defined(CORE_TEENSY35)
        pinTrigger = 23;
        pinTrigger2 = 36;
        pinStepperDir = 34;
        pinStepperStep = 35;
        pinCoil1 = 33; //Done
        pinCoil2 = 24; //Done
        pinCoil3 = 51; //Won't work (No mapping for pin 32)
        pinCoil4 = 52; //Won't work (No mapping for pin 33)
        pinFuelPump = 26; //Requires PVT4 adapter or above
        pinFan = 50; //Won't work (No mapping for pin 35)
        pinTachOut = 28; //Done
      #endif
      break;

    case 9:
      //Pin mappings as per the 89-95 MX5 PNP shield
      pinInjector1 = 11; //Output pin injector 1 is on
      pinInjector2 = 10; //Output pin injector 2 is on
      pinInjector3 = 9; //Output pin injector 3 is on
      pinInjector4 = 8; //Output pin injector 4 is on
      pinInjector5 = 14; //Output pin injector 5 is on
      pinCoil1 = 39; //Pin for coil 1
      pinCoil2 = 41; //Pin for coil 2
      pinCoil3 = 32; //Pin for coil 3
      pinCoil4 = 33; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A5; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A3; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
      pinIdle1 = 2; //Single wire idle control
      pinBoost = 4;
      pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pinFuelPump = 37; //Fuel pump output
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinFan = 35; //Pin for the fan output
      pinLaunch = 12; //Can be overwritten below
      pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 44; //Reset control output

      #if defined(CORE_TEENSY35)
        pinTrigger = 23;
        pinTrigger2 = 36;
        pinStepperDir = 34;
        pinStepperStep = 35;
        pinCoil1 = 33; //Done
        pinCoil2 = 24; //Done
        pinCoil3 = 51; //Won't work (No mapping for pin 32)
        pinCoil4 = 52; //Won't work (No mapping for pin 33)
        pinFuelPump = 26; //Requires PVT4 adapter or above
        pinFan = 50; //Won't work (No mapping for pin 35)
        pinTachOut = 28; //Done
      #endif
      break;

    case 10:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings for user turtanas PCB
      pinInjector1 = 4; //Output pin injector 1 is on
      pinInjector2 = 5; //Output pin injector 2 is on
      pinInjector3 = 6; //Output pin injector 3 is on
      pinInjector4 = 7; //Output pin injector 4 is on
      pinInjector5 = 8; //Placeholder only - NOT USED
      pinInjector6 = 9; //Placeholder only - NOT USED
      pinInjector7 = 10; //Placeholder only - NOT USED
      pinInjector8 = 11; //Placeholder only - NOT USED
      pinCoil1 = 24; //Pin for coil 1
      pinCoil2 = 28; //Pin for coil 2
      pinCoil3 = 36; //Pin for coil 3
      pinCoil4 = 40; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 18; //The CAS pin
      pinTrigger2 = 19; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinMAP2 = A8; //MAP2 sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A4; //O2 Sensor pin
      pinBat = A7; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinSpareTemp1 = A6;
      pinSpareTemp2 = A5;
      pinTachOut = 41; //Tacho output pin transistori puuttuu 2n2222 tähän ja 1k 12v
      pinFuelPump = 42; //Fuel pump output 2n2222
      pinFan = 47; //Pin for the fan output
      pinTachOut = 49; //Tacho output pin
      pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 26; //Reset control output

    #endif
      break;

    case 20:
    #if defined(CORE_AVR) && !defined(SMALL_FLASH_MODE) //No support for bluepill here anyway
      //Pin mappings as per the Plazomat In/Out shields Rev 0.1
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 28; //Pin for coil 1
      pinCoil2 = 24; //Pin for coil 2
      pinCoil3 = 40; //Pin for coil 3
      pinCoil4 = 36; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinSpareOut1 = 4; //Spare LSD Output 1(PWM)
      pinSpareOut2 = 5; //Spare LSD Output 2(PWM)
      pinSpareOut3 = 6; //Spare LSD Output 3(PWM)
      pinSpareOut4 = 7; //Spare LSD Output 4(PWM)
      pinSpareOut5 = 50; //Spare LSD Output 5(digital)
      pinSpareOut6 = 52; //Spare LSD Output 6(digital)
      pinTrigger = 20; //The CAS pin
      pinTrigger2 = 21; //The Cam Sensor pin
      pinSpareTemp2 = A15; //spare Analog input 2
      pinSpareTemp1 = A14; //spare Analog input 1
      pinO2 = A8; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinMAP = A3; //MAP sensor pin
      pinTPS = A2;//TPS input pin
      pinCLT = A1; //CLS sensor pin
      pinIAT = A0; //IAT sensor pin
      pinFan = 47; //Pin for the fan output
      pinFuelPump = 4; //Fuel pump output
      pinTachOut = 49; //Tacho output pin
      pinResetControl = 26; //Reset control output
    #endif
      break;

    case 30:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the dazv6 shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 10; //Output pin injector 3 is on
      pinInjector4 = 11; //Output pin injector 4 is on
      pinInjector5 = 12; //Output pin injector 5 is on
      pinCoil1 = 40; //Pin for coil 1
      pinCoil2 = 38; //Pin for coil 2
      pinCoil3 = 50; //Pin for coil 3
      pinCoil4 = 52; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTrigger3 = 17; // cam sensor 2 pin
      pinTPS = A2;//TPS input pin
      pinMAP = A3; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A8; //O2 Sensor pin
      pinO2_2 = A9; //O2 sensor pin (second sensor)
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      pinFuelPump = 45; //Fuel pump output
      pinStepperDir = 20; //Direction pin  for DRV8825 driver
      pinStepperStep = 21; //Step pin for DRV8825 driver
      pinSpareHOut1 = 4; // high current output spare1
      pinSpareHOut2 = 6; // high current output spare2
      pinBoost = 7;
      pinSpareLOut1 = 43; //low current output spare1
      pinSpareLOut2 = 47;
      pinSpareLOut3 = 49;
      pinSpareLOut4 = 51;
      pinSpareLOut5 = 53;
      pinFan = 47; //Pin for the fan output
    #endif
      break;

    case 40:
      //Pin mappings as per the NO2C shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 9; //Output pin injector 2 is on
      pinInjector3 = 11; //Output pin injector 3 is on - NOT USED
      pinInjector4 = 12; //Output pin injector 4 is on - NOT USED
      pinInjector5 = 13; //Placeholder only - NOT USED
      pinCoil1 = 23; //Pin for coil 1
      pinCoil2 = 22; //Pin for coil 2
      pinCoil3 = 2; //Pin for coil 3 - ONLY WITH DB2
      pinCoil4 = 3; //Pin for coil 4 - ONLY WITH DB2
      pinCoil5 = 46; //Placeholder only - NOT USED
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A3; //TPS input pin
      pinMAP = A0; //MAP sensor pin
      pinIAT = A5; //IAT sensor pin
      pinCLT = A4; //CLT sensor pin
      pinO2 = A2; //O2 sensor pin
      pinBat = A1; //Battery reference voltage pin
      pinBaro = A6; //Baro sensor pin - ONLY WITH DB
      pinSpareTemp1 = A7; //spare Analog input 1 - ONLY WITH DB
      pinDisplayReset = 48; // OLED reset pin - NOT USED
      pinTachOut = 38; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      pinIdle2 = 47; //2 wire idle control - NOT USED
      pinBoost = 7; //Boost control
      pinVVT_1 = 6; //Default VVT output
      pinFuelPump = 4; //Fuel pump output
      pinStepperDir = 25; //Direction pin for DRV8825 driver
      pinStepperStep = 24; //Step pin for DRV8825 driver
      pinStepperEnable = 27; //Enable pin for DRV8825 driver
      pinLaunch = 10; //Can be overwritten below
      pinFlex = 20; // Flex sensor (Must be external interrupt enabled) - ONLY WITH DB
      pinFan = 30; //Pin for the fan output - ONLY WITH DB
      pinSpareLOut1 = 32; //low current output spare1 - ONLY WITH DB
      pinSpareLOut2 = 34; //low current output spare2 - ONLY WITH DB
      pinSpareLOut3 = 36; //low current output spare3 - ONLY WITH DB
      pinResetControl = 26; //Reset control output
      break;

    case 41:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings as per the UA4C shield
      pinInjector1 = 8; //Output pin injector 1 is on
      pinInjector2 = 7; //Output pin injector 2 is on
      pinInjector3 = 6; //Output pin injector 3 is on
      pinInjector4 = 5; //Output pin injector 4 is on
      pinInjector5 = 45; //Output pin injector 5 is on PLACEHOLDER value for now
      pinCoil1 = 35; //Pin for coil 1
      pinCoil2 = 36; //Pin for coil 2
      pinCoil3 = 33; //Pin for coil 3
      pinCoil4 = 34; //Pin for coil 4
      pinCoil5 = 44; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinFlex = 20; // Flex sensor
      pinTPS = A3; //TPS input pin
      pinMAP = A0; //MAP sensor pin
      pinBaro = A7; //Baro sensor pin
      pinIAT = A5; //IAT sensor pin
      pinCLT = A4; //CLS sensor pin
      pinO2 = A1; //O2 Sensor pin
      pinO2_2 = A9; //O2 sensor pin (second sensor)
      pinBat = A2; //Battery reference voltage pin
      pinSpareTemp1 = A8; //spare Analog input 1
      pinLaunch = 37; //Can be overwritten below
      pinDisplayReset = 48; // OLED reset pin PLACEHOLDER value for now
      pinTachOut = 22; //Tacho output pin
      pinIdle1 = 9; //Single wire idle control
      pinIdle2 = 10; //2 wire idle control
      pinFuelPump = 23; //Fuel pump output
      pinVVT_1 = 11; //Default VVT output
      pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pinStepperStep = 31; //Step pin for DRV8825 driver
      pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pinBoost = 12; //Boost control
      pinSpareLOut1 = 26; //low current output spare1
      pinSpareLOut2 = 27; //low current output spare2
      pinSpareLOut3 = 28; //low current output spare3
      pinSpareLOut4 = 29; //low current output spare4
      pinFan = 24; //Pin for the fan output
      pinResetControl = 46; //Reset control output PLACEHOLDER value for now
    #endif
      break;

    case 45:
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
      //Pin mappings for the DIY-EFI CORE4 Module. This is an AVR only module
      #if defined(CORE_AVR)
      pinInjector1 = 10; //Output pin injector 1 is on
      pinInjector2 = 11; //Output pin injector 2 is on
      pinInjector3 = 12; //Output pin injector 3 is on
      pinInjector4 = 9; //Output pin injector 4 is on
      pinCoil1 = 39; //Pin for coil 1
      pinCoil2 = 29; //Pin for coil 2
      pinCoil3 = 28; //Pin for coil 3
      pinCoil4 = 27; //Pin for coil 4
      pinCoil4 = 26; //Placeholder  for coil 5
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinFlex = 20; // Flex sensor
      pinTPS = A3; //TPS input pin
      pinMAP = A2; //MAP sensor pin
      pinBaro = A15; //Baro sensor pin
      pinIAT = A11; //IAT sensor pin
      pinCLT = A4; //CLS sensor pin
      pinO2 = A12; //O2 Sensor pin
      pinO2_2 = A13; //O2 sensor pin (second sensor)
      pinBat = A1; //Battery reference voltage pin
      pinSpareTemp1 = A14; //spare Analog input 1
      pinLaunch = 24; //Can be overwritten below
      pinDisplayReset = 48; // OLED reset pin PLACEHOLDER value for now
      pinTachOut = 38; //Tacho output pin
      pinIdle1 = 42; //Single wire idle control
      pinIdle2 = 43; //2 wire idle control
      pinFuelPump = 41; //Fuel pump output
      pinVVT_1 = 44; //Default VVT output
      pinStepperDir = 32; //Direction pin  for DRV8825 driver
      pinStepperStep = 31; //Step pin for DRV8825 driver
      pinStepperEnable = 30; //Enable pin for DRV8825 driver
      pinBoost = 45; //Boost control
      pinSpareLOut1 = 37; //low current output spare1
      pinSpareLOut2 = 36; //low current output spare2
      pinSpareLOut3 = 35; //low current output spare3
      pinSpareLOut4 = 34; //low current output spare4
      pinSpareLOut5 = 33; //low current output spare4
      pinFan = 40; //Pin for the fan output
      pinResetControl = 46; //Reset control output PLACEHOLDER value for now
      #endif
    #endif
      break;

    #if defined(CORE_TEENSY35)
    case 50:
      //Pin mappings as per the teensy rev A shield
      pinInjector1 = 2; //Output pin injector 1 is on
      pinInjector2 = 10; //Output pin injector 2 is on
      pinInjector3 = 6; //Output pin injector 3 is on
      pinInjector4 = 9; //Output pin injector 4 is on
      //Placeholder only - NOT USED:
      //pinInjector5 = 13;
      pinCoil1 = 29; //Pin for coil 1
      pinCoil2 = 30; //Pin for coil 2
      pinCoil3 = 31; //Pin for coil 3 - ONLY WITH DB2
      pinCoil4 = 32; //Pin for coil 4 - ONLY WITH DB2
      //Placeholder only - NOT USED:
      //pinCoil5 = 46; 
      pinTrigger = 23; //The CAS pin
      pinTrigger2 = 36; //The Cam Sensor pin
      pinTPS = 16; //TPS input pin
      pinMAP = 17; //MAP sensor pin
      pinIAT = 14; //IAT sensor pin
      pinCLT = 15; //CLT sensor pin
      pinO2 = A22; //O2 sensor pin
      pinO2_2 = A21; //O2 sensor pin (second sensor)
      pinBat = 18; //Battery reference voltage pin
      pinTachOut = 20; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      pinBoost = 11; //Boost control
      pinFuelPump = 38; //Fuel pump output
      pinStepperDir = 34; //Direction pin for DRV8825 driver
      pinStepperStep = 35; //Step pin for DRV8825 driver
      pinStepperEnable = 33; //Enable pin for DRV8825 driver
      pinLaunch = 26; //Can be overwritten below
      pinFan = 37; //Pin for the fan output - ONLY WITH DB
      pinSpareHOut1 = 8; // high current output spare1
      pinSpareHOut2 = 7; // high current output spare2
      pinSpareLOut1 = 21; //low current output spare1
      break;

    case 51:
      //Pin mappings as per the teensy revB board shield
      pinInjector1 = 2; //Output pin injector 1 is on
      pinInjector2 = 10; //Output pin injector 2 is on
      pinInjector3 = 6; //Output pin injector 3 is on - NOT USED
      pinInjector4 = 9; //Output pin injector 4 is on - NOT USED
      pinCoil1 = 29; //Pin for coil 1
      pinCoil2 = 30; //Pin for coil 2
      pinCoil3 = 31; //Pin for coil 3 - ONLY WITH DB2
      pinCoil4 = 32; //Pin for coil 4 - ONLY WITH DB2
      pinTrigger = 23; //The CAS pin
      pinTrigger2 = 36; //The Cam Sensor pin
      pinTPS = 16; //TPS input pin
      pinMAP = 17; //MAP sensor pin
      pinIAT = 14; //IAT sensor pin
      pinCLT = 15; //CLT sensor pin
      pinO2 = A22; //O2 sensor pin
      pinO2_2 = A21; //O2 sensor pin (second sensor)
      pinBat = 18; //Battery reference voltage pin
      pinTachOut = 20; //Tacho output pin
      pinIdle1 = 5; //Single wire idle control
      pinBoost = 11; //Boost control
      pinFuelPump = 38; //Fuel pump output
      pinStepperDir = 34; //Direction pin for DRV8825 driver
      pinStepperStep = 35; //Step pin for DRV8825 driver
      pinStepperEnable = 33; //Enable pin for DRV8825 driver
      pinLaunch = 26; //Can be overwritten below
      pinFan = 37; //Pin for the fan output - ONLY WITH DB
      pinSpareHOut1 = 8; // high current output spare1
      pinSpareHOut2 = 7; // high current output spare2
      pinSpareLOut1 = 21; //low current output spare1
      break;
    #endif

    default:
      #if defined(STM32F4)
        //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407
        //PC8~PC12 SDio
        //PA13~PA15 & PB4 SWD(debug) pins
        //PB0 EEPROM CS pin
        //PA9 & PD10 Serial1
        //PD5 & PD6 Serial2
        pinInjector1 = PE7; //Output pin injector 1 is on
        pinInjector2 = PE8; //Output pin injector 2 is on
        pinInjector3 = PE9; //Output pin injector 3 is on
        pinInjector4 = PE10; //Output pin injector 4 is on
        pinInjector5 = PE11; //Output pin injector 5 is on
        pinInjector6 = PE12; //Output pin injector 6 is on
        pinCoil1 = PD0; //Pin for coil 1
        pinCoil2 = PD1; //Pin for coil 2
        pinCoil3 = PD2; //Pin for coil 3
        pinCoil4 = PD3; //Pin for coil 4
        pinCoil5 = PD4; //Pin for coil 5
        pinTPS = A0; //TPS input pin
        pinMAP = A1; //MAP sensor pin
        pinIAT = A2; //IAT sensor pin
        pinCLT = A3; //CLT sensor pin
        pinO2 = A4; //O2 Sensor pin
        pinBat = A5; //Battery reference voltage pin
        pinBaro = A9;
        pinIdle1 = PB8; //Single wire idle control
        pinIdle2 = PB9; //2 wire idle control
        pinBoost = PE0; //Boost control
        pinVVT_1 = PE1; //Default VVT output
        pinStepperDir = PD8; //Direction pin  for DRV8825 driver
        pinStepperStep = PB15; //Step pin for DRV8825 driver
        pinStepperEnable = PD9; //Enable pin for DRV8825
        pinDisplayReset = PE1; // OLED reset pin
        pinFan = PE2; //Pin for the fan output
        pinFuelPump = PC0; //Fuel pump output
        pinTachOut = PC1; //Tacho output pin
        //external interrupt enabled pins
        //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
        pinFlex = PE2; // Flex sensor (Must be external interrupt enabled)
        pinTrigger = PE3; //The CAS pin
        pinTrigger2 = PE4; //The Cam Sensor pin
      #else
        #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
        //Pin mappings as per the v0.2 shield
        pinInjector1 = 8; //Output pin injector 1 is on
        pinInjector2 = 9; //Output pin injector 2 is on
        pinInjector3 = 10; //Output pin injector 3 is on
        pinInjector4 = 11; //Output pin injector 4 is on
        pinInjector5 = 12; //Output pin injector 5 is on
        pinCoil1 = 28; //Pin for coil 1
        pinCoil2 = 24; //Pin for coil 2
        pinCoil3 = 40; //Pin for coil 3
        pinCoil4 = 36; //Pin for coil 4
        pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
        pinTrigger = 20; //The CAS pin
        pinTrigger2 = 21; //The Cam Sensor pin
        pinTPS = A2; //TPS input pin
        pinMAP = A3; //MAP sensor pin
        pinIAT = A0; //IAT sensor pin
        pinCLT = A1; //CLS sensor pin
        pinO2 = A8; //O2 Sensor pin
        pinBat = A4; //Battery reference voltage pin
        pinStepperDir = 16; //Direction pin  for DRV8825 driver
        pinStepperStep = 17; //Step pin for DRV8825 driver
        pinDisplayReset = 48; // OLED reset pin
        pinFan = 47; //Pin for the fan output
        pinFuelPump = 4; //Fuel pump output
        pinTachOut = 49; //Tacho output pin
        pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
        pinBoost = 5;
        pinIdle1 = 6;
        pinResetControl = 43; //Reset control output
        #endif
      #endif  
      break;
  }

  //Setup any devices that are using selectable pins

  if ( (configPage6.launchPin != 0) && (configPage6.launchPin < BOARD_NR_GPIO_PINS) ) { pinLaunch = pinTranslate(configPage6.launchPin); }
  if ( (configPage4.ignBypassPin != 0) && (configPage4.ignBypassPin < BOARD_NR_GPIO_PINS) ) { pinIgnBypass = pinTranslate(configPage4.ignBypassPin); }
  if ( (configPage2.tachoPin != 0) && (configPage2.tachoPin < BOARD_NR_GPIO_PINS) ) { pinTachOut = pinTranslate(configPage2.tachoPin); }
  if ( (configPage4.fuelPumpPin != 0) && (configPage4.fuelPumpPin < BOARD_NR_GPIO_PINS) ) { pinFuelPump = pinTranslate(configPage4.fuelPumpPin); }
  if ( (configPage6.fanPin != 0) && (configPage6.fanPin < BOARD_NR_GPIO_PINS) ) { pinFan = pinTranslate(configPage6.fanPin); }
  if ( (configPage6.boostPin != 0) && (configPage6.boostPin < BOARD_NR_GPIO_PINS) ) { pinBoost = pinTranslate(configPage6.boostPin); }
  if ( (configPage6.vvtPin != 0) && (configPage6.vvtPin < BOARD_NR_GPIO_PINS) ) { pinVVT_1 = pinTranslate(configPage6.vvtPin); }
  if ( (configPage6.useExtBaro != 0) && (configPage6.baroPin < BOARD_NR_GPIO_PINS) ) { pinBaro = configPage6.baroPin + A0; }
  if ( (configPage6.useEMAP != 0) && (configPage10.EMAPPin < BOARD_NR_GPIO_PINS) ) { pinEMAP = configPage10.EMAPPin + A0; }
  if ( (configPage10.fuel2InputPin != 0) && (configPage10.fuel2InputPin < BOARD_NR_GPIO_PINS) ) { pinFuel2Input = pinTranslate(configPage10.fuel2InputPin); }

  //Currently there's no default pin for Idle Up
  pinIdleUp = pinTranslate(configPage2.idleUpPin);

  /* Reset control is a special case. If reset control is enabled, it needs its initial state set BEFORE its pinMode.
     If that doesn't happen and reset control is in "Serial Command" mode, the Arduino will end up in a reset loop
     because the control pin will go low as soon as the pinMode is set to OUTPUT. */
  if ( (configPage4.resetControl != 0) && (configPage4.resetControlPin < BOARD_NR_GPIO_PINS) )
  {
    resetControl = configPage4.resetControl;
    pinResetControl = pinTranslate(configPage4.resetControlPin);
    setResetControlPinState();
    pinMode(pinResetControl, OUTPUT);
  }

  //Finally, set the relevant pin modes for outputs
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

  //This is a legacy mode option to revert the MAP reading behaviour to match what was in place prior to the 201905 firmware
  if(configPage2.legacyMAP > 0) { digitalWrite(pinMAP, HIGH); }

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

  //And for inputs
  #if defined(CORE_STM32)
    #ifndef ARDUINO_ARCH_STM32 //libmaple core aka STM32DUINO
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

  //Each of the below are only set when their relevant function is enabled. This can help prevent pin conflicts that users aren't aware of with unused functions
  if(configPage2.flexEnabled > 0)
  {
    pinMode(pinFlex, INPUT); //Standard GM / Continental flex sensor requires pullup, but this should be onboard. The internal pullup will not work (Requires ~3.3k)!
  }
  if(configPage6.launchEnabled > 0)
  {
    if (configPage6.lnchPullRes == true) { pinMode(pinLaunch, INPUT_PULLUP); }
    else { pinMode(pinLaunch, INPUT); } //If Launch Pull Resistor is not set make input float.
  }
  if(configPage2.idleUpEnabled > 0)
  {
    if (configPage2.idleUpPolarity == 0) { pinMode(pinIdleUp, INPUT_PULLUP); } //Normal setting
    else { pinMode(pinIdleUp, INPUT); } //inverted setting
  }
  if(configPage10.fuel2Mode == FUEL2_MODE_INPUT_SWITCH)
  {
    if (configPage10.fuel2InputPullup == true) { pinMode(pinFuel2Input, INPUT_PULLUP); } //With pullup
    else { pinMode(pinFuel2Input, INPUT); } //Normal input
  }
  

  //These must come after the above pinMode statements
  triggerPri_pin_port = portInputRegister(digitalPinToPort(pinTrigger));
  triggerPri_pin_mask = digitalPinToBitMask(pinTrigger);
  triggerSec_pin_port = portInputRegister(digitalPinToPort(pinTrigger2));
  triggerSec_pin_mask = digitalPinToBitMask(pinTrigger2);

}

void initialiseTriggers()
{
  byte triggerInterrupt = 0; // By default, use the first interrupt
  byte triggerInterrupt2 = 1;

  #if defined(CORE_AVR)
    switch (pinTrigger) {
      //Arduino Mega 2560 mapping
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
        triggerInterrupt = 0; break; //This should NEVER happen
    }
  #else
    triggerInterrupt = pinTrigger;
  #endif

  #if defined(CORE_AVR)
    switch (pinTrigger2) {
      //Arduino Mega 2560 mapping
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
        triggerInterrupt2 = 0; break; //This should NEVER happen
    }
  #else
    triggerInterrupt2 = pinTrigger2;
  #endif

  pinMode(pinTrigger, INPUT);
  pinMode(pinTrigger2, INPUT);
  pinMode(pinTrigger3, INPUT);
  //digitalWrite(pinTrigger, HIGH);
  detachInterrupt(triggerInterrupt);
  detachInterrupt(triggerInterrupt2);
  //The default values for edges
  primaryTriggerEdge = 0; //This should ALWAYS be changed below
  secondaryTriggerEdge = 0; //This is optional and may not be changed below, depending on the decoder in use

  //Set the trigger function based on the decoder in the config
  switch (configPage4.TrigPattern)
  {
    case 0:
      //Missing tooth decoder
      triggerSetup_missingTooth();
      triggerHandler = triggerPri_missingTooth;
      triggerSecondaryHandler = triggerSec_missingTooth;
      decoderHasSecondary = true;
      getRPM = getRPM_missingTooth;
      getCrankAngle = getCrankAngle_missingTooth;
      triggerSetEndTeeth = triggerSetEndTeeth_missingTooth;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      if(configPage4.TrigEdgeSec == 0) { secondaryTriggerEdge = RISING; }
      else { secondaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);

      /*
      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, triggerHandler, RISING); }
      else { attachInterrupt(triggerInterrupt, triggerHandler, FALLING); }
      if(configPage4.TrigEdgeSec == 0) { attachInterrupt(triggerInterrupt2, triggerSec_missingTooth, RISING); }
      else { attachInterrupt(triggerInterrupt2, triggerSec_missingTooth, FALLING); }
      */
      break;

    case 1:
      // Basic distributor
      triggerSetup_BasicDistributor();
      triggerHandler = triggerPri_BasicDistributor;
      getRPM = getRPM_BasicDistributor;
      getCrankAngle = getCrankAngle_BasicDistributor;
      triggerSetEndTeeth = triggerSetEndTeeth_BasicDistributor;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, triggerHandler, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, triggerHandler, FALLING); }

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = CHANGE; //Secondary is always on every change

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

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = RISING; //always rising for this trigger

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

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      //These may both need to change, not sure
      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }
      secondaryTriggerEdge = FALLING;

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      attachInterrupt(triggerInterrupt2, triggerSecondaryHandler, secondaryTriggerEdge);
      break;

    case 11:
      triggerSetup_non360();
      triggerHandler = triggerPri_DualWheel; //Is identical to the dual wheel decoder, so that is used. Same goes for the secondary below
      triggerSecondaryHandler = triggerSec_DualWheel; //Note the use of the Dual Wheel trigger function here. No point in having the same code in twice.
      decoderHasSecondary = true;
      getRPM = getRPM_non360;
      getCrankAngle = getCrankAngle_non360;
      triggerSetEndTeeth = triggerSetEndTeeth_non360;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      //No secondary input required for this pattern
      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { primaryTriggerEdge = FALLING; }

      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case 15:
      triggerSetup_Harley();
      triggerHandler = triggerPri_Harley;
      //triggerSecondaryHandler = triggerSec_Harley;
      getRPM = getRPM_Harley;
      getCrankAngle = getCrankAngle_Harley;
      triggerSetEndTeeth = triggerSetEndTeeth_Harley;

      primaryTriggerEdge = RISING; //Always rising
      attachInterrupt(triggerInterrupt, triggerHandler, primaryTriggerEdge);
      break;

    case 16:
      //36-2-2-2
      triggerSetup_ThirtySixMinus222();
      triggerHandler = triggerPri_ThirtySixMinus222;
      triggerSecondaryHandler = triggerSec_ThirtySixMinus222;
      decoderHasSecondary = true;
      getRPM = getRPM_missingTooth; //This uses the same function as the missing tooth decoder, so no need to duplicate code
      getCrankAngle = getCrankAngle_missingTooth; //This uses the same function as the missing tooth decoder, so no need to duplicate code
      triggerSetEndTeeth = triggerSetEndTeeth_ThirtySixMinus222;

      if(configPage4.TrigEdge == 0) { primaryTriggerEdge = RISING; } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
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

      if(configPage4.TrigEdge == 0) { attachInterrupt(triggerInterrupt, triggerHandler, RISING); } // Attach the crank trigger wheel interrupt (Hall sensor drags to ground when triggering)
      else { attachInterrupt(triggerInterrupt, triggerHandler, FALLING); }
      break;
  }
}
